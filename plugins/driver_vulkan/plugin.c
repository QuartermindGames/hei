/* Copyright (C) 2021 Mark E Sowden <markelswo@gmail.com> */

#include "plugin.h"

static PLPluginDescription pluginDesc = {
        .description = "Vulkan Graphics Driver.",
        .pluginVersion = { 0, 0, 1 },
        .interfaceVersion = { PL_PLUGIN_INTERFACE_VERSION_MAJOR, PL_PLUGIN_INTERFACE_VERSION_MINOR }
};

const PLPluginExportTable *gInterface = NULL;

PL_EXPORT const PLPluginDescription *PLQueryPlugin( unsigned int interfaceVersion ) {
	return &pluginDesc;
}

int vkLogLevel;

PL_EXPORT void PLInitializePlugin( const PLPluginExportTable *functionTable ) {
	gInterface = functionTable;

	vkLogLevel = gInterface->AddLogLevel( "plugin/vulkan", PLColourRGB( 255, 255, 255 ), true );

	extern PLGraphicsInterface graphicsInterface;
	gInterface->RegisterGraphicsMode( "vulkan", &graphicsInterface );
}
