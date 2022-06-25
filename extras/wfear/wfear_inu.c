/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl.h>
#include <plcore/pl_package.h>

#define WFEAR_INU_MAGIC      0xF4EA1072
#define WFEAR_INU_TERMINATOR 0x0A

#define WFEAR_IGNORE_DRIVE /* if enabled, strips the drive from the file path */

static bool ParseINUHeader( PLFile *file, uint32_t *tableOffset, uint32_t *tableIndices ) {
	uint32_t magic = PlReadInt32( file, false, NULL );
	if ( magic != WFEAR_INU_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic" );
		return false;
	}

	if ( PlReadInt32( file, false, NULL ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected clear zone" );
		return false;
	}

	*tableOffset = PlReadInt32( file, false, NULL );
	if ( *tableOffset >= PlGetFileSize( file ) ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid table offset" );
		return false;
	}

	*tableIndices = PlReadInt32( file, false, NULL );
	if ( *tableIndices == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid table indices" );
		return false;
	}

	return true;
}

static PLPackage *ParseINUFile( PLFile *file ) {
	uint32_t tocOffset, tocIndices;
	if ( !ParseINUHeader( file, &tocOffset, &tocIndices ) ) {
		return NULL;
	}

	PlFileSeek( file, tocOffset, PL_SEEK_SET );

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), tocIndices, NULL );
	for ( unsigned int i = 0; i < tocIndices; ++i ) {
		unsigned int p = 0;
		bool status;
		do {
			int c = PlReadInt8( file, &status );
			if ( c == WFEAR_INU_TERMINATOR ) {
				break;
			}

			if ( p < sizeof( package->table[ i ].fileName ) ) {
				package->table[ i ].fileName[ p++ ] = c;
			}
		} while ( status );

		if ( !status ) {
			PlDestroyPackage( package );
			PlReportErrorF( PL_RESULT_FILEERR, "aborted table parse unexpectedly" );
			return NULL;
		}

#ifdef WFEAR_IGNORE_DRIVE
		/* a lot of White Fear's files have paths with specific drive names which makes
		 * it slightly awkward to dump them later, so we need to cut that crap off... */
		size_t s = strlen( package->table[ i ].fileName );
		if ( s > 3 && ( package->table[ i ].fileName[ 1 ] == ':' && ( package->table[ i ].fileName[ 2 ] == '\\' || package->table[ i ].fileName[ 2 ] == '/' ) ) ) {
			char *c = &package->table[ i ].fileName[ 3 ];
			for ( unsigned int j = 0; j < ( s - 3 ); ++j ) {
				package->table[ i ].fileName[ j ] = *c++;
			}
			package->table[ i ].fileName[ s - 3 ] = '\0';
		}
#endif

		package->table[ i ].offset = PlReadInt32( file, false, NULL );
		package->table[ i ].fileSize = PlReadInt32( file, false, NULL );
	}

	return package;
}

PLPackage *WFear_INU_LoadPackage( const char *path ) {
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLPackage *package = ParseINUFile( file );

	PlCloseFile( file );

	return package;
}
