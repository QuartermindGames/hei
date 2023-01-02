/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "core.h"

int coreMessage = PL_LOG_INIT_LEVEL;

PL_EXPORT const PLPluginDescription *PLQueryPlugin( void ) {
	static PLPluginDescription pluginDescription;
	PL_ZERO_( pluginDescription );

	pluginDescription.description = "Core Design file format support.";
	pluginDescription.pluginVersion[ 0 ] = 0;
	pluginDescription.pluginVersion[ 1 ] = 0;
	pluginDescription.pluginVersion[ 2 ] = 1;
	memcpy( pluginDescription.interfaceVersion, PL_PLUGIN_INTERFACE_VERSION, sizeof( PLPluginInterfaceVersion ) );

	return &pluginDescription;
}

PLPackage *Core_CLU_LoadPackage( const char *path );
PLImage *Core_HGT_ParseImage( PLFile *file );

static void DumpObjectFilenames( unsigned int argc, char **argv ) {
	PLFile *file = gInterface->OpenFile( argv[ 1 ], true );
	if ( file == NULL ) {
		gInterface->LogMessage( coreMessage, "Failed to open file: %s\n", gInterface->GetError() );
		return;
	}

	FILE *out = fopen( "object_filenames.txt", "w" );
	if ( out == NULL ) {
		gInterface->CloseFile( file );
		gInterface->LogMessage( coreMessage, "Failed to write output file!\n" );
		return;
	}

	const char *buf = gInterface->GetFileData( file );
	const char *p = buf;
	while ( *p != '\0' ) {
		if ( strncmp( p, "FileName: ", 10 ) != 0 ) {
			gInterface->SkipLine( &p );
			continue;
		}

		p = ( p + 10 );

		char tmp[ 128 ];
		PL_ZERO_( tmp );
		for ( unsigned int i = 0; i < sizeof( tmp ); ++i ) {
			if ( p[ i ] == '\r' || p[ i ] == '\n' ) {
				break;
			}

			tmp[ i ] = ( char ) tolower( p[ i ] );
			if ( tmp[ i ] == '\\' ) {
				tmp[ i ] = '/';
			}
		}

		if ( strcmp( tmp, "no filename (group)" ) != 0 && strcmp( tmp, "deleted" ) != 0 ) {
			fprintf( out, "\"objects/%s\",\n", tmp );
		}
		gInterface->SkipLine( &p );
	}

	gInterface->CloseFile( file );

	fclose( out );
}

const PLPluginExportTable *gInterface = NULL;
PL_EXPORT void PLInitializePlugin( const PLPluginExportTable *functionTable ) {
	gInterface = functionTable;

	coreMessage = gInterface->AddLogLevel( "core", PL_COLOUR_GOLD, true );

	gInterface->RegisterConsoleCommand( "core_dump_filenames",
	                                    "Parses a very specific file to extract object filenames.",
	                                    1, DumpObjectFilenames );

	gInterface->RegisterPackageLoader( "clu", Core_CLU_LoadPackage );
	gInterface->RegisterImageLoader( "hgt", Core_HGT_ParseImage );
}
