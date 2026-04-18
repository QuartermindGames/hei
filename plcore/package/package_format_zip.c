// SPDX-License-Identifier: MIT
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_linkedlist.h>

#include "package_private.h"
#include "qmos/public/qm_os_memory.h"

/*
 * todo:
 * 	- zip64 support is currently missing
 */

#define ZIP_FILE_MAGIC QM_OS_MAGIC_TO_NUM( 'P', 'K', '\3', '\4' )

#define ZIP_EXCLUDE_DIRS

typedef enum ZipCompression {
	ZIP_COMPRESSION_NONE = 0,
	ZIP_COMPRESSION_IMPLODED = 6,
	ZIP_COMPRESSION_DEFLATED = 8,
	ZIP_COMPRESSION_LZMA = 14,
} ZipCompression;

typedef struct ZipFileHeader {
	uint32_t magic;
	uint16_t version;
	uint16_t flags;
	uint16_t compression;
	uint16_t modificationTime;
	uint16_t modificationDate;
	uint32_t checksum;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t nameSize;
	uint16_t extraSize;

	PLFileOffset offset;

	char *name;
	void *extra;
} ZipFileHeader;

static bool ParseZipFileHeader( QmFsFile *file, ZipFileHeader *header ) {
	header->magic = qm_fs_file_read_int32( file, false, NULL );
	if ( header->magic != ZIP_FILE_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: %X", header->magic );
		return false;
	}

	header->version = qm_fs_file_read_int16( file, false, NULL );
	header->flags = qm_fs_file_read_int16( file, false, NULL );
	header->compression = qm_fs_file_read_int16( file, false, NULL );
	header->modificationTime = qm_fs_file_read_int16( file, false, NULL );
	header->modificationDate = qm_fs_file_read_int16( file, false, NULL );
	header->checksum = qm_fs_file_read_int32( file, false, NULL );
	header->compressedSize = qm_fs_file_read_int32( file, false, NULL );
	header->uncompressedSize = qm_fs_file_read_int32( file, false, NULL );

	header->nameSize = qm_fs_file_read_int16( file, false, NULL );
	header->name = QM_OS_MEMORY_NEW_( char, header->nameSize + 1 );

	header->extraSize = qm_fs_file_read_int16( file, false, NULL );
	header->extra = QM_OS_MEMORY_NEW_( char, header->extraSize );

	qm_file_read( file, header->name, sizeof( char ), header->nameSize );
	qm_file_read( file, header->extra, sizeof( char ), header->extraSize );

	header->offset = qm_fs_file_get_offset( file );

	qm_fs_file_seek( file, header->compressedSize, QM_FS_SEEK_CUR );

	return true;
}

QmFsPackage *PlParseZipPackage( QmFsFile *file ) {
	/* check this first, just to be sure it's indicated
	 * that it's actually a zip file */
	uint32_t magic = qm_fs_file_read_int32( file, false, NULL );
	if ( magic != ZIP_FILE_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: %X", magic );
		return NULL;
	}

	qm_fs_file_rewind( file );

	PLLinkedList *files = PlCreateLinkedList();

	while ( true ) {
		ZipFileHeader *store = QM_OS_MEMORY_NEW( ZipFileHeader );
		if ( !ParseZipFileHeader( file, store ) ) {
			qm_os_memory_free( store );
			break;
		}

#if defined( ZIP_EXCLUDE_DIRS )
		/* couldn't see any defined way to determine type for a zip
		 * 'file' - and some zips store indices representing directories
		 * which is great, but we're not specifically interested in these */
		const char *c = &store->name[ store->nameSize - 1 ];
		if ( *c == '/' || *c == '\\' ) {
			qm_os_memory_free( store );
			continue;
		}
#endif

		PlInsertLinkedListNode( files, store );
	}

	unsigned int numFiles = PlGetNumLinkedListNodes( files );
	QmFsPackage *package = PlCreatePackageHandle( qm_fs_file_get_path( file ), numFiles, NULL );

	PLLinkedListNode *node = PlGetFirstNode( files );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		ZipFileHeader *store = PlGetLinkedListNodeUserData( node );
		package->files[ i ].compressedSize = store->compressedSize;
		package->files[ i ].size = store->uncompressedSize;
		package->files[ i ].offset = store->offset;

		snprintf( package->files[ i ].name, sizeof( package->files[ i ].name ), "%s", store->name );

		switch ( store->compression ) {
			case ZIP_COMPRESSION_DEFLATED:
				package->files[ i ].compressionType = PL_COMPRESSION_DEFLATE;
				break;
			case ZIP_COMPRESSION_IMPLODED:
				package->files[ i ].compressionType = PL_COMPRESSION_IMPLODE;
				break;
			case ZIP_COMPRESSION_NONE:
				package->files[ i ].compressionType = PL_COMPRESSION_NONE;
				break;
			default:
				package->files[ i ].compressionType = PL_COMPRESSION_UNKNOWN;
				break;
		}

		qm_os_memory_free( store->name );
		qm_os_memory_free( store->extra );
		qm_os_memory_free( store );

		node = PlGetNextLinkedListNode( node );
	}

	PlDestroyLinkedList( files );

	return package;
}

QmFsPackage *PlLoadZipPackage( const char *path ) {
	QmFsFile *file = qm_fs_file_open( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	QmFsPackage *package = PlParseZipPackage( file );
	PlCloseFile( file );
	return package;
}
