/**
 * Hei Platform Library
 * Copyright (C) 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_parse.h>

#include "asa.h"

static const unsigned int ATR_MAGIC = 0x72746101; /* actor attribute file */
static const unsigned int LOG_MAGIC = 0x646e6946; /* log output from dumping tool */

static void dump_file_strings( const char *path, void *user ) {
	FILE *out = ( FILE * ) user;

	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		printf( "Failed to open file: %s\n", PlGetError() );
		return;
	}

	uint32_t magic = PlReadInt32( file, false, NULL );
	if ( magic == LOG_MAGIC ) {
		printf( "LOG file: %s\n", path );

		PlRewindFile( file );

		size_t size = PlGetFileSize( file );
		char *buf = PL_NEW_( char, size + 1 );
		PlReadFile( file, buf, sizeof( char ), size );

		const char *p = buf;
		PlSkipLine( &p );

		if ( strncmp( "Creating ", p, 9 ) == 0 ) {
			PlSkipLine( &p );
			while ( *p == '.' ) p++;

			while ( *p != '\0' ) {
				PLPath tmp;
				PL_ZERO_( tmp );
				for ( unsigned int i = 0; i < sizeof( tmp ); ++i ) {
					if ( *p == ' ' ) {
						break;
					}

					if ( *p == '\\' ) {
						tmp[ i++ ] = '\\';
					}
					tmp[ i ] = *( p++ );
				}

				fprintf( out, "\"%s\",\n", tmp );

				PlSkipLine( &p );
			}
		} else {
			printf( "Unexpected second line in file!\n" );
		}

		PL_DELETE( buf );
	}

	PlCloseFile( file );
}

static void command_asa_dump_strings( int argc, char **argv ) {
	FILE *out = fopen( "dump.txt", "w" );
	if ( out == NULL ) {
		printf( "Failed to open 'dump.txt' for write!\n" );
		return;
	}

	PlScanDirectory( "./", NULL, dump_file_strings, true, out );

	fclose( out );
	printf( "Done!\n" );
}

void asa_register_commands( void ) {
	PlRegisterConsoleCommand( "asa_dump_strings",
	                          "Scrape through and dump identifiable paths.",
	                          0, command_asa_dump_strings );
}
