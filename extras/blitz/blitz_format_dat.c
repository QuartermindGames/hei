// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "blitz.h"

/* Blitz, DAT format
 *
 * Usages:
 * 	- Titan A.E. (Cancelled, PSX)
 * 	- Chicken Run (2000, PSX)
 *
 * Notes:
 * 	Filenames are all hashed using a relatively common algorithm.
 */

#define BLITZ_DAT_MAGIC 0x12345678

static PLPackage *ParseDATFile( PLFile *file ) {
	int32_t magic = gInterface->ReadInt32( file, false, NULL );
	if ( magic != BLITZ_DAT_MAGIC ) {
		gInterface->ReportError( PL_RESULT_FILETYPE, PL_FUNCTION, "invalid filetype" );
		return NULL;
	}

	int32_t numFiles = gInterface->ReadInt32( file, false, NULL );
	PLPackage *package = gInterface->CreatePackageHandle( gInterface->GetFilePath( file ), numFiles, NULL );
	if ( package == NULL ) {
		return NULL;
	}

	/* see if we have a string list available to match up w/ the hashes */
	const char **strings = NULL;
	unsigned int numStrings = 0;
	const char *filePath = gInterface->GetFilePath( file );
	if ( filePath != NULL ) {
		const char *fileName = strrchr( filePath, '/' );
		if ( fileName == NULL ) {
			fileName = filePath;
		}
		if ( fileName != NULL ) {
			if ( gInterface->strcasecmp( fileName, "TITAN.DAT" ) == 0 ) {
				strings = titanStrings;
				numStrings = numTitanStrings;
			}
		}
	}

	unsigned int numMatches = 0;
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		int32_t hash = gInterface->ReadInt32( file, false, NULL );
		const char *fileName = get_string_for_hash( hash, strings, numStrings );
		if ( fileName == NULL ) {
			snprintf( package->table[ i ].fileName, sizeof( PLPath ), "%X", hash );
		} else {
			numMatches++;
			snprintf( package->table[ i ].fileName, sizeof( PLPath ), "%s", fileName );
			gInterface->NormalizePath( package->table[ i ].fileName, sizeof( PLPath ) );
		}
		package->table[ i ].offset = 16384 + ( gInterface->ReadInt32( file, false, NULL ) * 2048 );
		package->table[ i ].fileSize = gInterface->ReadInt32( file, false, NULL );
	}

#if !defined( NDEBUG )
	const char *fileName = strrchr( filePath, '/' );
	if ( fileName == NULL ) {
		fileName = filePath;
	}
	printf( "%.2lf%% matched (%u/%u) for Blitz package \"%s\"\n", ( ( double ) numMatches / numFiles ) * 100.0,
	        numMatches, numFiles,
	        fileName );
#endif

	return package;
}

PLPackage *Blitz_DAT_LoadPackage( const char *path ) {
	PLFile *file = gInterface->OpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLPackage *package = ParseDATFile( file );

	gInterface->CloseFile( file );

	return package;
}
