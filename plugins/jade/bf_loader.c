/* Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com> */

#include "plugin.h"

/* BF Loader
 * Only checked against Rayman Raving Rabbids, as
 * that's what I originally implemented this for.
 * Might work for other games, unsure.
 */

PLPackage *BFLoader_ReadFile( PLFile *file ) {
	char magic[ 4 ];
	gInterface->ReadString( file, magic, sizeof( magic ) );
	if ( strcmp( "BUG", magic ) != 0 ) {
		gInterface->ReportError( );
		return NULL;
	}

	return NULL;
}

PLPackage *BFLoader_OpenFile( const char *path ) {
	PLFile *file = gInterface->OpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLPackage *package = BFLoader_ReadFile( file );

	gInterface->CloseFile( file );

	return package;
}
