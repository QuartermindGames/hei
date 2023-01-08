/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl.h>
#include <plcore/pl_filesystem.h>
#include <plcore/pl_parse.h>
#include <plcore/pl_hashtable.h>

static PLHashTable *hashTable;

int main( int argc, char **argv ) {
	( void ) ( argc );
	( void ) ( argv );

	PLFile *file = PlOpenFile( "TRFILE.TXT", true );
	if ( file == NULL ) {
		printf( "Failed to open file: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	hashTable = PlCreateHashTable();

	const char *p = PlGetFileData( file );
	while ( *p != '\0' ) {
		char lineBuf[ 512 ];
		if ( PlParseLine( &p, lineBuf, sizeof( lineBuf ) ) == NULL ) {
			break;
		}

		char *c = strchr( &lineBuf[ 30 ], ':' ) + 1;
		assert( c != NULL );

		if ( *c == '-' ) {
			continue;
		}

		char *path = PL_NEW_( char, PL_SYSTEM_MAX_PATH );
		for ( unsigned int i = 0; *c != ':'; ++i ) {
			path[ i ] = *c++;
		}

		if ( !PlInsertHashTableNode( hashTable, path, PL_SYSTEM_MAX_PATH, path ) ) {
			continue;
		}

		printf( "%s\n", path );
	}

	return EXIT_SUCCESS;
}