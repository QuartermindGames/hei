// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"

PLPackage *PlParseAllPackage_( PLFile *file ) {
	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid number of files in package" );
		return NULL;
	}

	// try to validate...
	for ( unsigned int i = 0; i < 3; ++i ) {
		bool status;
		if ( PL_READUINT32( file, false, &status ) == 0 && status ) {
			continue;
		}

		if ( !status ) {
			return NULL;
		}

		PlReportErrorF( PL_RESULT_FILEERR, "invalid header for package" );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	if ( package == NULL ) {
		return NULL;
	}

	// this seems to be good enough, but not entirely correct?
	static const unsigned int ENTRY_SIZE = 72;
	unsigned int offset = PlGetFileOffset( file ) + ( ENTRY_SIZE * numFiles );
	for ( unsigned int i = 0; i < numFiles; offset += package->table[ i ].fileSize, ++i ) {
		PlReadFile( file, package->table[ i ].fileName, sizeof( char ), 64 );
		package->table[ i ].fileSize = PL_READUINT32( file, false, NULL );
		package->table[ i ].offset = offset;
		PlReadInt32( file, false, NULL );
	}

	return package;
}
