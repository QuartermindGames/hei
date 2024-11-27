// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "plcore/pl_math.h"

#include "../package_private.h"

/* Headhunter AHF package format
 *
 * Based on the packs found in the build below.
 * https://hiddenpalace.org/Headhunter_(Nov_18,_2001_prototype)
 */

PLPackage *PlParseAhfPackage_( PLFile *file ) {
	static const uint32_t MAGIC = PL_MAGIC_TO_NUM( 'A', 'H', 'F', 'F' );
	uint32_t tag = PL_READUINT32( file, false, NULL );
	if ( tag != MAGIC ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid tag (%u != %u)", tag, MAGIC );
		return NULL;
	}

	uint32_t version = PL_READUINT32( file, false, NULL );
	if ( version != 2 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid version (%u != 2)", version );
		return NULL;
	}

	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid number of files in package" );
		return NULL;
	}

	size_t totalSize = PlGetFileSize( file );
	uint32_t startOffset = PL_READUINT32( file, false, NULL );
	if ( startOffset >= totalSize ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid file size" );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		package->table[ i ].offset = PL_READUINT32( file, false, NULL );
		package->table[ i ].fileSize = PL_READUINT32( file, false, NULL );
		PL_READUINT32( file, false, NULL );// maybe was meant to be string size? seems unused

		unsigned int j;
		for ( j = 0; j < ( sizeof( package->table[ i ].fileName ) - 1 ); ++j ) {
			package->table[ i ].fileName[ j ] = PlReadInt8( file, NULL );
			if ( package->table[ i ].fileName[ j ] == '\0' ) {
				break;
			}
		}

		PLFileOffset padding = PlRoundUp( ( int ) ( j + 1 ), 4 ) - ( j + 1 );
		PlFileSeek( file, padding, SEEK_CUR );

		PlNormalizePath( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ) );
	}

	return package;
}
