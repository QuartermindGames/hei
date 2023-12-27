// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>
// Purpose: Experimental package interface for Haven Call of the King.

#include <plcore/pl_package.h>

/* Could be considered complete, except for the fact that
 * we're not currently handling multi-level directories;
 * I'm not sure if there's a method to the madness or not
 * but for instance, there's a 'chars' folder and then a
 * set of folders that should fall under that, but I can't
 * currently see any obvious ways to determine the association
 * there. *sigh*
 */

PLPackage *PlxParseHavenPackage( PLFile *file ) {
	size_t fileSize = PlGetFileSize( file );
	uint32_t fileTableOffset = PL_READUINT32( file, false, NULL );
	if ( fileTableOffset == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid table offset" );
		return NULL;
	} else if ( fileTableOffset >= fileSize ) {
		PlReportErrorF( PL_RESULT_FILEERR, "out of bounds toc (%u >= %u)", fileTableOffset, fileSize );
		return NULL;
	}

	// + 4 to skip over some crap
	if ( !PlFileSeek( file, fileTableOffset + 4, PL_SEEK_SET ) ) {
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
		assert( package->table[ i ].fileSize != 0 );
		package->table[ i ].compressedSize = PL_READUINT32( file, false, NULL );// compressed size???
		assert( package->table[ i ].compressedSize == package->table[ i ].fileSize );
		assert( PL_READUINT32( file, false, NULL ) == 0 );// unused?
		PlSetupPath( package->table[ i ].fileName, true, "%u.bin", i );
	}

	uint32_t numStrings = PL_READUINT32( file, false, NULL );
	PL_READUINT32( file, false, NULL );// unsure...

	typedef struct SubStringIndex {
		uint32_t offset;
		uint16_t unk;
		uint16_t index;
	} SubStringIndex;
	SubStringIndex *subStrings = PL_NEW_( SubStringIndex, numStrings );

	for ( unsigned int i = 0; i < numStrings; ++i ) {
		subStrings[ i ].offset = PL_READUINT32( file, false, NULL );// offset into string table
#if 0
		subStrings[ i ].unk0[ 0 ] = PL_READUINT8( file, NULL );//
		subStrings[ i ].unk0[ 1 ] = PL_READUINT8( file, NULL );//
#else
		subStrings[ i ].unk = PL_READUINT16( file, false, NULL );
#endif
		subStrings[ i ].index = PL_READUINT16( file, false, NULL );// possibly file id? if 0, it's a dir (probably?)
	}

	PLFileOffset stringTableOffset = PlGetFileOffset( file );
	printf( "ended at %lu\n", stringTableOffset );

	unsigned int numDirs = 0;
	numFiles = 0;

	char dir[ 128 ] = { '\0' };
	for ( unsigned int i = 0; i < numStrings; ++i ) {
		if ( !PlFileSeek( file, stringTableOffset + subStrings[ i ].offset, PL_SEEK_SET ) ) {
			assert( 0 );
			continue;
		}

		char tmp[ 128 ];
		for ( unsigned int j = 0; j < sizeof( tmp ); ++j ) {
			tmp[ j ] = ( char ) PlReadInt8( file, NULL );
			if ( tmp[ j ] == '\0' )
				break;
		}

#if 0
		printf( "%u %u %u %u %s\n",
		        i,
		        subStrings[ i ].offset,
		        subStrings[ i ].unk,
		        subStrings[ i ].index,
		        tmp );
#endif

		if ( subStrings[ i ].index == 0 ) {
			strcpy( dir, tmp );
			numDirs++;
		} else {
			PlSetupPath( package->table[ numFiles ].fileName, true, "%s/%s", dir, tmp );
			numFiles++;
		}
	}

	printf( "num files %u (%u), num dirs %u\n", numFiles, package->table_size, numDirs );

	// cleanup
	PL_DELETE( subStrings );

	return package;
}

void PlxRegisterHavenPackageFormat( void ) {
	PlRegisterPackageLoader( "dat", NULL, PlxParseHavenPackage );
}
