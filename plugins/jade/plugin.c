/* Copyright (C) 2020 Mark E Sowden <markelswo@gmail.com> */

#define PL_COMPILE_PLUGIN 1

#include <PL/pl_plugin_interface.h>

static PLPluginDescription pluginDesc = {
        .description = "Jade.",
        .pluginVersion = { 0, 0, 1 },
        .interfaceVersion = PL_PLUGIN_INTERFACE_VERSION,
};

const PLPluginExportTable *gInterface = NULL;

PL_EXPORT const PLPluginDescription *PLQueryPlugin( unsigned int interfaceVersion ) {
	return &pluginDesc;
}

PL_EXPORT void PLInitializePlugin( const PLPluginExportTable *functionTable ) {
	gInterface = functionTable;
}
