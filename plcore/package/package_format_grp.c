// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"

// GRP package format used by Duke Nukem 3D

PLPackage *PlParseGrpPackage_( PLFile *file ) {
	char buf[ 12 ];
	PlReadFile( file, buf, sizeof( char ), sizeof( buf ) );
	if ( strncmp( buf, "KenSilverman", sizeof( buf ) ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "not a grp package" );
		return NULL;
	}

	unsigned int numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "empty grp package" );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	if ( package == NULL ) {
		return NULL;
	}

	unsigned int baseOffset = PlGetFileOffset( file ) + ( numFiles * 16 );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		PlReadFile( file, package->table[ i ].fileName, sizeof( char ), 12 );
		package->table[ i ].fileSize = PL_READUINT32( file, false, NULL );
		package->table[ i ].offset = baseOffset;
		baseOffset += package->table[ i ].fileSize;
	}

	return package;
}
