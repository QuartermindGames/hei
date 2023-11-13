// SPDX-License-Identifier: MIT
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_linkedlist.h>

#include "package_private.h"

/*
 * todo:
 * 	- zip64 support is currently missing
 */

#define ZIP_FILE_MAGIC PL_MAGIC_TO_NUM( 'P', 'K', '\3', '\4' )

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

static bool ParseZipFileHeader( PLFile *file, ZipFileHeader *header ) {
	header->magic = PlReadInt32( file, false, NULL );
	if ( header->magic != ZIP_FILE_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: %X", header->magic );
		return false;
	}

	header->version = PlReadInt16( file, false, NULL );
	header->flags = PlReadInt16( file, false, NULL );
	header->compression = PlReadInt16( file, false, NULL );
	header->modificationTime = PlReadInt16( file, false, NULL );
	header->modificationDate = PlReadInt16( file, false, NULL );
	header->checksum = PlReadInt32( file, false, NULL );
	header->compressedSize = PlReadInt32( file, false, NULL );
	header->uncompressedSize = PlReadInt32( file, false, NULL );

	header->nameSize = PlReadInt16( file, false, NULL );
	header->name = PL_NEW_( char, header->nameSize + 1 );

	header->extraSize = PlReadInt16( file, false, NULL );
	header->extra = PL_NEW_( char, header->extraSize );

	PlReadFile( file, header->name, sizeof( char ), header->nameSize );
	PlReadFile( file, header->extra, sizeof( char ), header->extraSize );

	header->offset = PlGetFileOffset( file );

	PlFileSeek( file, header->compressedSize, PL_SEEK_CUR );

	return true;
}

PLPackage *PlParseZipPackage( PLFile *file ) {
	/* check this first, just to be sure it's indicated
	 * that it's actually a zip file */
	uint32_t magic = PlReadInt32( file, false, NULL );
	if ( magic != ZIP_FILE_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: %X", magic );
		return NULL;
	}

	PlRewindFile( file );

	PLLinkedList *files = PlCreateLinkedList();

	while ( true ) {
		ZipFileHeader *store = PL_NEW( ZipFileHeader );
		if ( !ParseZipFileHeader( file, store ) ) {
			PL_DELETE( store );
			break;
		}

#if defined( ZIP_EXCLUDE_DIRS )
		/* couldn't see any defined way to determine type for a zip
		 * 'file' - and some zips store indices representing directories
		 * which is great, but we're not specifically interested in these */
		const char *c = &store->name[ store->nameSize - 1 ];
		if ( *c == '/' || *c == '\\' ) {
			PL_DELETE( store );
			continue;
		}
#endif

		PlInsertLinkedListNode( files, store );
	}

	unsigned int numFiles = PlGetNumLinkedListNodes( files );
	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );

	PLLinkedListNode *node = PlGetFirstNode( files );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		ZipFileHeader *store = PlGetLinkedListNodeUserData( node );
		package->table[ i ].compressedSize = store->compressedSize;
		package->table[ i ].fileSize = store->uncompressedSize;
		package->table[ i ].offset = store->offset;

		snprintf( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ), "%s", store->name );

		switch ( store->compression ) {
			case ZIP_COMPRESSION_DEFLATED:
				package->table[ i ].compressionType = PL_COMPRESSION_DEFLATE;
				break;
			case ZIP_COMPRESSION_IMPLODED:
				package->table[ i ].compressionType = PL_COMPRESSION_IMPLODE;
				break;
			case ZIP_COMPRESSION_NONE:
				package->table[ i ].compressionType = PL_COMPRESSION_NONE;
				break;
			default:
				package->table[ i ].compressionType = PL_COMPRESSION_UNKNOWN;
				break;
		}

		PL_DELETE( store->name );
		PL_DELETE( store->extra );
		PL_DELETE( store );

		node = PlGetNextLinkedListNode( node );
	}

	PlDestroyLinkedList( files );

	return package;
}

PLPackage *PlLoadZipPackage( const char *path ) {
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLPackage *package = PlParseZipPackage( file );
	PlCloseFile( file );
	return package;
}
