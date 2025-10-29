/**
 * Hei Platform Library
 * Copyright (C) 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "qmos/public/qm_os_memory.h"

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
		PlRewindFile( file );

		size_t size = PlGetFileSize( file );
		char *buf = QM_OS_MEMORY_NEW_( char, size + 1 );
		PlReadFile( file, buf, sizeof( char ), size );

		const char *p = buf;
		PlSkipLine( &p );

		if ( strncmp( "Creating ", p, 9 ) == 0 ) {
			/* argh bloody hell, turns out this line *does* give us the name of a packed
			 * file, so we need to pull that first before just skipping the line */
			p = p + 9;
			if ( *p == 'Y' ) {
				p = p + 18;

				PLPath tmp;
				PL_ZERO_( tmp );
				for ( unsigned int i = 0; i < sizeof( tmp ); ++i ) {
					if ( *p == '\0' || strncmp( p, "Tree file", 9 ) == 0 ) {
						break;
					}

					if ( *p == '\\' ) {
						tmp[ i++ ] = '\\';
					}
					tmp[ i ] = *( p++ );
				}

				pl_strtolower( tmp );
				fprintf( out, "\"%s\",\n", tmp );
			}

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
			printf( "Unexpected second line in file: %s\n", path );
		}

		qm_os_memory_free( buf );
	} else if ( magic == ATR_MAGIC ) {
		uint16_t headerType = PlReadInt16( file, false, NULL );
		if ( headerType == 130 ) {
			PlFileSeek( file, 15, PL_SEEK_SET );
		} else if ( headerType == 194 ) {
			PlFileSeek( file, 17, PL_SEEK_SET );
		} else {
			PlFileSeek( file, 14, PL_SEEK_SET );
		}

		uint8_t length = PlReadInt8( file, NULL ) + 1; /* null-terminated it seems */

		PLPath tmp;
		PL_ZERO_( tmp );
		for ( unsigned int i = 0; i < sizeof( tmp ); ++i ) {
			char c = PlReadInt8( file, NULL );
			if ( c == '\\' ) {
				tmp[ i++ ] = '\\';
			}
			tmp[ i ] = c;
			if ( c == '\0' )
				break;
		}

		if ( *tmp == 'Y' || *tmp == 'y' ) {
			pl_strtolower( tmp );
			const char *c;
			if ( tmp[ 3 ] == '\\' ) {
				c = &tmp[ 4 ];
			} else {
				c = &tmp[ 3 ];
			}
			fprintf( out, "\"%s\",\n", c );

			assert( strrchr( c, '.' ) != NULL );
		} else {
			printf( "Unexpected path in file: %s\n", path );
		}
	}

	PlCloseFile( file );
}

static void command_asa_dump_strings( unsigned int argc, char **argv ) {
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
