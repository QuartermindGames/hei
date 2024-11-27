// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"

PLPackage *PlParseIce3DDatPackage_( PLFile *file ) {
	uint16_t versionMajor = PL_READUINT16( file, false, NULL );
	uint16_t versionMinor = PL_READUINT16( file, false, NULL );
	if ( versionMajor != 1 || versionMinor != 4 ) {
		PlReportErrorF( PL_RESULT_FILEVERSION, "invalid version (%u != 1 || %u != 4 )", versionMajor, versionMinor );
		return NULL;
	}

	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "unexpected file count" );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	for ( uint32_t i = 0; i < numFiles; ++i ) {
		PL_READUINT32( file, false, NULL );// index
		PL_READUINT16( file, false, NULL );// ??

		package->table[ i ].fileSize = PL_READUINT32( file, false, NULL );

		uint32_t stringSize = PL_READUINT32( file, false, NULL );
		char *buf = PL_NEW_( char, stringSize + 1 );
		PlReadFile( file, buf, sizeof( char ), stringSize );
		snprintf( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ), "%s", buf );
		PL_DELETE( buf );

		package->table[ i ].offset = PL_READUINT32( file, false, NULL ) + 8;
	}

	return package;
}
