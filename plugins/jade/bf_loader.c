/* Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com> */

#include "plugin.h"

/* BF Loader
 * Only checked against Rayman Raving Rabbids, as
 * that's what I originally implemented this for.
 * Might work for other games, unsure.
 */

#define BF_LOADER_MAGIC "BUG"

static PLPackage *ReadFile( PLFile *file ) {
	char magic[ 4 ];
	gInterface->ReadString( file, magic, sizeof( magic ) );
	if ( strcmp( BF_LOADER_MAGIC, magic ) != 0 ) {
		gInterface->ReportError( PL_RESULT_FILETYPE, "invalid identifier, received \"%s\" but expected \"" BF_LOADER_MAGIC "\"" );
		return NULL;
	}

	return NULL;
}

PLPackage *BFLoader_OpenFile( const char *path ) {
	PLFile *file = gInterface->OpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLPackage *package = ReadFile( file );

	gInterface->CloseFile( file );

	return package;
}
