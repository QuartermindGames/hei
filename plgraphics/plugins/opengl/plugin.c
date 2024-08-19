// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "plugin.h"

static PLGDriverDescription pluginDesc = {
        .identifier = "opengl",
        .description = "OpenGL Graphics Driver.",
        .driverVersion = { 1, 0, 0 },
        .coreInterfaceVersion = { PL_PLUGIN_INTERFACE_VERSION_MAJOR, PL_PLUGIN_INTERFACE_VERSION_MINOR },
        .graphicsInterfaceVersion = { PLG_INTERFACE_VERSION_MAJOR, PLG_INTERFACE_VERSION_MINOR },
};

const PLGDriverExportTable *gInterface = NULL;

PL_EXPORT const PLGDriverDescription *QueryGraphicsDriver( void ) {
	return &pluginDesc;
}

int glLogLevel;

PL_EXPORT const PLGDriverImportTable *InitializeGraphicsDriver( const PLGDriverExportTable *functionTable ) {
	gInterface = functionTable;

	glLogLevel = gInterface->core->AddLogLevel( "plugin/opengl", PLColourRGB( 255, 255, 255 ), true );

	return &graphicsInterface;
}
