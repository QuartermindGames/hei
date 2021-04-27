/* Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com> */

#include "plugin.h"

static PLPluginDescription pluginDesc = {
        .description = "RenderWare file format support.",
        .pluginVersion = { 0, 0, 1 },
        .interfaceVersion = { PL_PLUGIN_INTERFACE_VERSION_MAJOR, PL_PLUGIN_INTERFACE_VERSION_MINOR },
};

const PLPluginExportTable *gInterface = NULL;

PL_EXPORT const PLPluginDescription *PLQueryPlugin( void ) {
	return &pluginDesc;
}

PLPackage *PAK_LoadFile( const char *path );

PL_EXPORT void PLInitializePlugin( const PLPluginExportTable *functionTable ) {
	gInterface = functionTable;
	gInterface->RegisterPackageLoader( "pak", PAK_LoadFile );
}
