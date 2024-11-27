// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "../package_private.h"

/* id Software's WAD package format */

/////////////////////////////////////////////////////////////////////////////////////
// Doom WAD support
/////////////////////////////////////////////////////////////////////////////////////

#define WAD_MAGIC   PL_MAGIC_TO_NUM( 'I', 'W', 'A', 'D' )
#define WAD_MAGIC_2 PL_MAGIC_TO_NUM( 'P', 'W', 'A', 'D' )

typedef struct WadIndex {
	int32_t offset;
	int32_t size;
	char name[ 8 ];
} WadIndex;

PLPackage *PlParseWadPackage_( PLFile *file ) {
	int32_t magic = PlReadInt32( file, false, NULL );
	if ( magic != WAD_MAGIC && magic != WAD_MAGIC_2 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: \"%s\"", magic );
		return NULL;
	}

	int32_t numLumps = PlReadInt32( file, false, NULL );
	if ( numLumps <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid number of lumps: %d", numLumps );
		return NULL;
	}

	int32_t tableOffset = PlReadInt32( file, false, NULL );
	if ( tableOffset <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid table offset: %d", tableOffset );
		return NULL;
	}

	unsigned int tableSize = sizeof( WadIndex ) * numLumps;
	if ( tableOffset + tableSize > PlGetFileSize( file ) ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid table offset location: %u", tableSize );
		return NULL;
	}

	PlFileSeek( file, tableOffset, PL_SEEK_SET );

	WadIndex *indices = PlMAllocA( tableSize );
	for ( int i = 0; i < numLumps; ++i ) {
		indices[ i ].offset = PlReadInt32( file, false, NULL );
		if ( indices[ i ].offset == 0 || indices[ i ].offset >= tableOffset ) {
			PlFree( indices );
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid file offset for index %d", i );
			return NULL;
		}

		indices[ i ].size = PlReadInt32( file, false, NULL );
		if ( indices[ i ].size >= ( int ) PlGetFileSize( file ) ) {
			PlFree( indices );
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid file size for index %d", i );
			return NULL;
		}

		if ( PlReadFile( file, indices[ i ].name, 1, 8 ) != 8 ) {
			PlFree( indices );
			return NULL;
		}
	}

	const char *path = PlGetFilePath( file );
	PLPackage *package = PlCreatePackageHandle( path, numLumps, NULL );
	for ( unsigned int i = 0; i < package->maxTableSize; ++i ) {
		PLPackageIndex *index = &package->table[ i ];
		index->offset = indices[ i ].offset;
		index->fileSize = indices[ i ].size;
		strncpy( index->fileName, indices[ i ].name, sizeof( index->fileName ) );
		index->fileName[ 8 ] = '\0';
	}

	return package;
}

/////////////////////////////////////////////////////////////////////////////////////
// Quake/Half-Life WAD support
/////////////////////////////////////////////////////////////////////////////////////

#define WAD2_MAGIC       PL_MAGIC_TO_NUM( 'W', 'A', 'D', '2' )
#define WAD2_NAME_LENGTH 16

#define WAD3_MAGIC PL_MAGIC_TO_NUM( 'W', 'A', 'D', '3' )

typedef struct Wad2Index {
	int32_t offset;
	int32_t cSize;
	int32_t size;
	uint8_t type;
	uint8_t compression;
	uint16_t unused;
	char name[ WAD2_NAME_LENGTH ];
} Wad2Index;

PLPackage *PlParseQWadPackage_( PLFile *file ) {
	int32_t magic = PlReadInt32( file, false, NULL );
	if ( magic != WAD2_MAGIC && magic != WAD3_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: \"%u\"", magic );
		return NULL;
	}

	int32_t numLumps = PlReadInt32( file, false, NULL );
	if ( numLumps <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid number of lumps: %d", numLumps );
		return NULL;
	}

	int32_t tableOffset = PlReadInt32( file, false, NULL );
	if ( tableOffset <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid table offset: %d", tableOffset );
		return NULL;
	}

	unsigned int tableSize = sizeof( Wad2Index ) * numLumps;
	if ( tableOffset + tableSize > PlGetFileSize( file ) ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid table offset location: %u", tableSize );
		return NULL;
	}

	PlFileSeek( file, tableOffset, PL_SEEK_SET );

	const char *path = PlGetFilePath( file );
	PLPackage *package = PlCreatePackageHandle( path, numLumps, NULL );
	for ( unsigned int i = 0; i < package->maxTableSize; ++i ) {
		PLPackageIndex *index = &package->table[ i ];
		index->offset = PlReadInt32( file, false, NULL );
		index->compressedSize = PlReadInt32( file, false, NULL );
		index->fileSize = PlReadInt32( file, false, NULL );

		const char *hint;
		uint8_t type = PlReadInt8( file, NULL ); /* type */
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

		PlReadInt8( file, NULL );         /* compression (afaik, never used) */
		PlReadInt16( file, false, NULL ); /* unused */

		PlReadFile( file, index->fileName, sizeof( char ), WAD2_NAME_LENGTH );
		strcat( index->fileName, hint );
	}

	return package;
}
