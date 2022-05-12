/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

/* this is a tool whipped up to quickly dump
 * the timestamps out of DNF2001 CPJ files */

#include <plcore/pl.h>
#include <plcore/pl_filesystem.h>

static void DumpHeaderTimestamp( const char *path, void *user ) {
	FILE *out = ( FILE * ) user;
	fprintf( out, "\"%s\" - ", path );

	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		fprintf( out, "failed to open: %s\n", PlGetError() );
		return;
	}

	static unsigned int cpjMagic = PL_MAGIC_TO_NUM( 'R', 'I', 'F', 'F' );
	unsigned int magic = PlReadInt32( file, false, NULL );
	if ( magic != cpjMagic ) {
		PlCloseFile( file );
		fprintf( out, "unknown filetype\n" );
		return;
	}

	PlFileSeek( file, 24, PL_SEEK_SET );

	time_t timestamp = PlReadInt32( file, false, NULL );
	struct tm *time = gmtime( &timestamp );

	PlCloseFile( file );

	char buf[ 32 ];
	strftime( buf, sizeof( buf ), "%c", time );
	fprintf( out, "%s\n", buf );
}

int main( int argc, char **argv ) {
	PlInitialize( argc, argv );

	FILE *out = fopen( "dump.txt", "w" );
	if ( out == NULL ) {
		printf( "Failed to open destination for writing!\n" );
		return EXIT_FAILURE;
	}

	PlScanDirectory( "Meshes/", "cpj", DumpHeaderTimestamp, true, out );

	fclose( out );

	return EXIT_SUCCESS;
}
