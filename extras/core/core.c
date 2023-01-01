/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "core.h"

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

const PLPluginExportTable *gInterface = NULL;
PL_EXPORT void PLInitializePlugin( const PLPluginExportTable *functionTable ) {
	gInterface = functionTable;

	gInterface->RegisterPackageLoader( "clu", Core_CLU_LoadPackage );
	gInterface->RegisterImageLoader( "hgt", Core_HGT_ParseImage );
}
