/**
 * Hei Platform Library
 * Copyright (C) 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "asa.h"

static const unsigned int ATR_MAGIC = 0x72746101;
static const unsigned int LOG_MAGIC = 0x646e6946;

static void dump_file_strings( const char *path, void *user ) {
	FILE *out = ( FILE * ) user;

	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		printf( "Failed to open file: %s\n", PlGetError() );
		return;
	}

	uint32_t magic = PlReadInt32( file, false, NULL );
	if ( magic == ATR_MAGIC ) {
		printf( "ATR file: %s\n", path );
	} else if ( magic == LOG_MAGIC ) {
		printf( "LOG file: %s\n", path );
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
	printf( "Done!" );
}

void asa_register_commands( void ) {
	PlRegisterConsoleCommand( "asa_dump_strings",
	                          "Scrape through and dump identifiable paths.",
	                          0, command_asa_dump_strings );
}
