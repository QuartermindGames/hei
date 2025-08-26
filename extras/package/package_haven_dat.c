// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2025 Mark E Sowden <hogsy@oldtimes-software.com>
// Purpose: Experimental package interface for Haven Call of the King.

#include <plcore/pl_package.h>

#define MAX_DIR_DEPTH 8

typedef struct DatTreeIndex {
	uint32_t offset;
	int16_t l;
	int16_t r;
} DatTreeIndex;

static uint32_t ValidateHeader( PLFile *file ) {
	const size_t fileSize = PlGetFileSize( file );
	if ( fileSize < 12 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "file too small for valid header" );
		return 0;
	}

	const uint32_t tocOffset = PL_READUINT32( file, false, NULL );
	if ( tocOffset == 0 || tocOffset > fileSize ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid table offset" );
		return 0;
	}

	// for validation sake, ensure that the given table size fits or is more than zero
	const uint32_t fileTableSize = PL_READUINT32( file, false, NULL );
	if ( fileTableSize == 0 || tocOffset + fileTableSize > fileSize ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid table size" );
		return 0;
	}
	// for validation sake, make sure there's nothing else
	if ( PlReadInt32( file, false, NULL ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "unexpected header data" );
		return 0;
	}

	return tocOffset;
}

PLPackage *PlxParseHavenPackage( PLFile *file ) {
	uint32_t tocOffset;
	if ( ( tocOffset = ValidateHeader( file ) ) == 0 ) {
		return NULL;
	}

	// + 4 to skip over some crap
	if ( !PlFileSeek( file, tocOffset + 4, PL_SEEK_SET ) ) {
		return NULL;
	}

	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "empty package" );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		package->table[ i ].offset = PL_READUINT32( file, false, NULL );
		package->table[ i ].fileSize = PL_READUINT32( file, false, NULL );
		package->table[ i ].compressedSize = PL_READUINT32( file, false, NULL );// compressed size??? always same as size
		PL_READUINT32( file, false, NULL );                                     // flag, always 0
		PlSetupPath( package->table[ i ].fileName, true, "%u.bin", i );
	}

	const uint32_t numStrings = PL_READUINT32( file, false, NULL );
	if ( numStrings == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "missing string table" );
		PlDestroyPackage( package );
		return NULL;
	}

	PL_READUINT32( file, false, NULL );// unsure...

	DatTreeIndex *tree = QM_OS_MEMORY_NEW_( DatTreeIndex, numStrings );
	for ( unsigned int i = 0; i < numStrings; ++i ) {
		tree[ i ].offset = PL_READUINT32( file, false, NULL );// offset into string table
		tree[ i ].l = PlReadInt16( file, false, NULL );
		tree[ i ].r = PlReadInt16( file, false, NULL );
	}

	// read in the strings - we can just copy out what we need
	const size_t bufSize = PlGetFileSize( file ) - PlGetFileOffset( file );
	char *buf = QM_OS_MEMORY_NEW_( char, bufSize );
	if ( PlReadFile( file, buf, sizeof( char ), bufSize ) != bufSize ) {
		qm_os_memory_free( tree );
		qm_os_memory_free( buf );
		PlDestroyPackage( package );
		return NULL;
	}

	// this is an awful awful thing to do...

	typedef struct DirIndex {
		const char *string;
		unsigned int index;
	} DirIndex;

	PLPackageIndex *currentIndex = &package->table[ 0 ];

	unsigned int dirDepth = 0;
	DirIndex dirStack[ MAX_DIR_DEPTH ] = {};
	for ( unsigned int i = 1; i < numStrings; ++i ) {
		// current string
		const char *c = &buf[ tree[ i ].offset ];
		//printf( "%d %d %d %s\n", i, tree[ i ].l, tree[ i ].r, c );

		if ( i < numStrings - 1 ) {
			if ( tree[ i ].r == 0 ) {
				if ( dirDepth >= MAX_DIR_DEPTH ) {
					PlReportErrorF( PL_RESULT_FILEERR, "directory depth exceeded max (%u >= %u)", dirDepth, MAX_DIR_DEPTH );
					break;
				}

				dirStack[ dirDepth ].string = c;
				dirStack[ dirDepth ].index = i;
				dirDepth++;
				continue;
			}
		}

		// build up the string...
		char filename[ 128 ] = {};
		for ( unsigned int j = 0; j < dirDepth; ++j ) {
			pl_strlcat( filename, dirStack[ j ].string, sizeof( filename ) );
			pl_strlcat( filename, "/", sizeof( filename ) );
		}
		PlSetupPath( currentIndex->fileName, true, "%s%s", filename, c );
		currentIndex++;

		if ( tree[ i ].r != i ) {
			for ( unsigned int j = 0; j < dirDepth; ++j ) {
				if ( dirStack[ j ].index == tree[ i ].r ) {
					dirDepth = j;
					break;
				}
			}
		}
	}

	// cleanup
	qm_os_memory_free( tree );
	qm_os_memory_free( buf );

	return package;
}

void PlxRegisterHavenPackageFormat( void ) {
	PlRegisterPackageLoader( "dat", NULL, PlxParseHavenPackage );
}
