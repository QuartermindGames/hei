// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"

// Package format as used by FreshEngine
/*	- Little endian
 *	- Fixed-length filenames (12 bytes)
 */

PLPackage *PlParseFreshBinPackage_( PLFile *file ) {
	// First check the funky identifier at the start
	char ident[ 8 ];
	PlReadFile( file, ident, sizeof( char ), sizeof( ident ) );
	if ( strncmp( ident, "DATA    ", 8 ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "not a freshengine bin package" );
		return NULL;
	}

	// First two bytes seem to just be zero, so we'll assume that's normal
	if ( PlReadInt32( file, false, NULL ) != 0 || PlReadInt32( file, false, NULL ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid freshengine bin package" );
		return NULL;
	}

	unsigned int numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "empty freshengine bin package" );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	if ( package == NULL ) {
		return NULL;
	}

	for ( unsigned int i = 0; i < numFiles; ++i ) {
		char filename[ 12 ];
		PlReadFile( file, filename, sizeof( char ), sizeof( filename ) );
		// For some reason, so null-terminator but spaces instead??
		// Let's convert it...
		for ( unsigned int j = 0; j < sizeof( filename ); ++j ) {
			if ( filename[ j ] != ' ' )
				continue;

			filename[ j ] = '\0';
			break;
		}

		strncpy( package->table[ i ].fileName, filename, 12 );

		unsigned int blockOffs = PL_READUINT32( file, false, NULL );
		package->table[ i ].offset = 2048 * blockOffs;
		unsigned int size = PL_READUINT32( file, false, NULL );
		package->table[ i ].fileSize = size;
	}

	return package;
}
