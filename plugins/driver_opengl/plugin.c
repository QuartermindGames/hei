/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "plugin.h"

static PLGDriverDescription pluginDesc = {
        .identifier = "opengl",
        .description = "OpenGL Graphics Driver.",
        .driverVersion = { 0, 1, 0 },
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

	glLogLevel = gInterface->core->AddLogLevel( "plugin/opengl", QM_MATH_COLOUR4UB_RGB( 255, 255, 255 ), true );

	extern PLGDriverImportTable graphicsInterface;
	return &graphicsInterface;
}
