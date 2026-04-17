// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "../package_private.h"
#include "qmos/public/qm_os_memory.h"

/* id Software's WAD package format */

/////////////////////////////////////////////////////////////////////////////////////
// Doom WAD support
/////////////////////////////////////////////////////////////////////////////////////

#define WAD_MAGIC   QM_OS_MAGIC_TO_NUM( 'I', 'W', 'A', 'D' )
#define WAD_MAGIC_2 QM_OS_MAGIC_TO_NUM( 'P', 'W', 'A', 'D' )

typedef struct WadIndex {
	int32_t offset;
	int32_t size;
	char name[ 8 ];
} WadIndex;

QmFsPackage *PlParseWadPackage_( QmFsFile *file ) {
	int32_t magic = qm_fs_file_read_int32( file, false, NULL );
	if ( magic != WAD_MAGIC && magic != WAD_MAGIC_2 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: \"%s\"", magic );
		return NULL;
	}

	int32_t numLumps = qm_fs_file_read_int32( file, false, NULL );
	if ( numLumps <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid number of lumps: %d", numLumps );
		return NULL;
	}

	int32_t tableOffset = qm_fs_file_read_int32( file, false, NULL );
	if ( tableOffset <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid table offset: %d", tableOffset );
		return NULL;
	}

	unsigned int tableSize = sizeof( WadIndex ) * numLumps;
	if ( tableOffset + tableSize > qm_fs_file_get_size( file ) ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid table offset location: %u", tableSize );
		return NULL;
	}

	qm_fs_file_seek( file, tableOffset, QM_FS_SEEK_SET );

	WadIndex *indices = QM_OS_MEMORY_MALLOC_( tableSize );
	for ( int i = 0; i < numLumps; ++i ) {
		indices[ i ].offset = qm_fs_file_read_int32( file, false, NULL );
		if ( indices[ i ].offset == 0 || indices[ i ].offset >= tableOffset ) {
			qm_os_memory_free( indices );
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid file offset for index %d", i );
			return NULL;
		}

		indices[ i ].size = qm_fs_file_read_int32( file, false, NULL );
		if ( indices[ i ].size >= ( int ) qm_fs_file_get_size( file ) ) {
			qm_os_memory_free( indices );
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid file size for index %d", i );
			return NULL;
		}

		if ( PlReadFile( file, indices[ i ].name, 1, 8 ) != 8 ) {
			qm_os_memory_free( indices );
			return NULL;
		}
	}

	const char *path = qm_fs_file_get_path( file );
	QmFsPackage *package = PlCreatePackageHandle( path, numLumps, NULL );
	for ( unsigned int i = 0; i < package->maxFiles; ++i ) {
		QmFsPackageFile *index = &package->files[ i ];
		index->offset = indices[ i ].offset;
		index->size = indices[ i ].size;
		strncpy( index->name, indices[ i ].name, sizeof( index->name ) );
		index->name[ 8 ] = '\0';
	}

	return package;
}

/////////////////////////////////////////////////////////////////////////////////////
// Quake/Half-Life WAD support
/////////////////////////////////////////////////////////////////////////////////////

#define WAD2_MAGIC       QM_OS_MAGIC_TO_NUM( 'W', 'A', 'D', '2' )
#define WAD2_NAME_LENGTH 16

#define WAD3_MAGIC QM_OS_MAGIC_TO_NUM( 'W', 'A', 'D', '3' )

typedef struct Wad2Index {
	int32_t offset;
	int32_t cSize;
	int32_t size;
	uint8_t type;
	uint8_t compression;
	uint16_t unused;
	char name[ WAD2_NAME_LENGTH ];
} Wad2Index;

QmFsPackage *PlParseQWadPackage_( QmFsFile *file ) {
	int32_t magic = qm_fs_file_read_int32( file, false, NULL );
	if ( magic != WAD2_MAGIC && magic != WAD3_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: \"%u\"", magic );
		return NULL;
	}

	int32_t numLumps = qm_fs_file_read_int32( file, false, NULL );
	if ( numLumps <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid number of lumps: %d", numLumps );
		return NULL;
	}

	int32_t tableOffset = qm_fs_file_read_int32( file, false, NULL );
	if ( tableOffset <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid table offset: %d", tableOffset );
		return NULL;
	}

	unsigned int tableSize = sizeof( Wad2Index ) * numLumps;
	if ( tableOffset + tableSize > qm_fs_file_get_size( file ) ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid table offset location: %u", tableSize );
		return NULL;
	}

	qm_fs_file_seek( file, tableOffset, QM_FS_SEEK_SET );

	const char *path = qm_fs_file_get_path( file );
	QmFsPackage *package = PlCreatePackageHandle( path, numLumps, NULL );
	for ( unsigned int i = 0; i < package->maxFiles; ++i ) {
		QmFsPackageFile *index = &package->files[ i ];
		index->offset = qm_fs_file_read_int32( file, false, NULL );
		index->compressedSize = qm_fs_file_read_int32( file, false, NULL );
		index->size = qm_fs_file_read_int32( file, false, NULL );

		const char *hint;
		uint8_t type = qm_fs_file_read_int8( file, NULL ); /* type */
		switch ( type ) {
			default:
				hint = ".none";
				break;
			case 1:
				hint = ".label";
				break;
			case 64:
				hint = ".palette";
				break;
			case 65:
				hint = ".colormap";
				break;
			case 66:
				hint = ".qpic";
				break;
			case 67:
				hint = ".miptex";
				break;
			case 68:
				hint = ".raw";
				break;
			case 69:
				hint = ".colormap2";
				break;
			case 70:
				hint = ".font";
				break;
		}

		qm_fs_file_read_int8( file, NULL );         /* compression (afaik, never used) */
		qm_fs_file_read_int16( file, false, NULL ); /* unused */

		PlReadFile( file, index->name, sizeof( char ), WAD2_NAME_LENGTH );
		strcat( index->name, hint );
	}

	return package;
}
