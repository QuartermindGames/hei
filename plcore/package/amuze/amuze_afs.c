// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "../package_private.h"

/* Headhunter AFS package format
 *
 * There are two types of AFS packages it seems?
 * One that starts with AHV which doesn't have a TOC and may be compressed,
 * and another which has a TOC and no magic.
 *
 * Based on the packs found in the build below.
 * https://hiddenpalace.org/Headhunter_(Nov_18,_2001_prototype)
 *
 * Will check others some other time.
 */

PLPackage *PlParseAfsPackage_( PLFile *file ) {
	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid number of files in package" );
		return NULL;
	}

	uint32_t version = PL_READUINT32( file, false, NULL );
	if ( version != 2 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid version (%u != 2)", version );
		return NULL;
	}

#define SECTOR_SIZE 2048
	if ( !PlFileSeek( file, SECTOR_SIZE, PL_SEEK_SET ) ) {
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	if ( package == NULL ) {
		return NULL;
	}

	for ( uint32_t i = 0; i < numFiles; ++i ) {
		package->table[ i ].offset = PL_READUINT32( file, false, NULL );
		package->table[ i ].fileSize = PL_READUINT32( file, false, NULL );
		PL_READUINT32( file, false, NULL );//?
		PL_READUINT32( file, false, NULL );//?
		PL_READUINT8( file, NULL );        // maybe this was supposed to be string length?? unused if so
		PlReadFile( file, package->table[ i ].fileName, sizeof( char ), 239 );
		PlNormalizePath( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ) );
	}

	return package;
}
