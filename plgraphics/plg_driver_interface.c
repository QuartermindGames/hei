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
/*===========================
	DRIVER INTERFACE
===========================*/

#include <plgraphics/plg_driver_interface.h>
#include "plg_private.h"

static PLGDriverExportTable exportTable = {
        .version = { PLG_INTERFACE_VERSION_MAJOR, PLG_INTERFACE_VERSION_MINOR },
        .CreateTexture = PlgCreateTexture,
        .DestroyTexture = PlgDestroyTexture,
        .GetMaxTextureAnistropy = PlgGetMaxTextureAnistropy,
        .GetMaxTextureSize = PlgGetMaxTextureSize,
        .GetMaxTextureUnits = PlgGetMaxTextureUnits,
        .SetTexture = PlgSetTexture,
        .SetTextureAnisotropy = PlgSetTextureAnisotropy,
        .SetTextureEnvironmentMode = PlgSetTextureEnvironmentMode,
        .SetTextureFlags = PlgSetTextureFlags,
};

#define DRIVER_EXTENSION "_driver"

#define MAX_GRAPHICS_MODES 32

typedef struct GraphicsMode {
	const char *description;
	const PLGDriverImportTable *interface;
} GraphicsMode;
static GraphicsMode graphicsModes[ MAX_GRAPHICS_MODES ];
static unsigned int numGraphicsModes = 0;

bool PlgRegisterDriverPlugin( const char *path ) {
	PLLibrary *library = PlLoadLibrary( path, false );
	if ( library == NULL ) {
		return false;
	}


}

void PlgRegisterDriver( const char *description, const PLGDriverImportTable *interface ) {
	if ( numGraphicsModes >= MAX_GRAPHICS_MODES ) {
		PlReportBasicError( PL_RESULT_MEMORY_EOA );
		return;
	}

	graphicsModes[ numGraphicsModes ].description = description;
	graphicsModes[ numGraphicsModes ].interface = interface;
	numGraphicsModes++;
}

/**
 * Private callback when scanning for plugins.
 */
static void RegisterScannedPlugin( const char *path, void *unused ) {
	plUnused( unused );

	/* validate it's actually a plugin */
	size_t length = strlen( path );
	const char *c = strrchr( path, '_' );
	if ( c == NULL ) {
		return;
	}

	/* remaining length */
	length -= c - path;
	if ( pl_strncasecmp( c, DRIVER_EXTENSION PL_SYSTEM_LIBRARY_EXTENSION, length ) != 0 ) {
		return;
	}

	PlRegisterPlugin( path );
}

void PlgScanForDrivers( const char *path ) {
	PlScanDirectory( path, NULL, RegisterScannedPlugin, false, NULL );
}

/**
 * Query what available graphics modes there are so that the
 * host can either choose their preference or attempt each.
 */
const char **PlgGetAvailableDriverInterfaces( unsigned int *numModes ) {
	static unsigned int cachedModes = 0;
	static const char *descriptors[ MAX_GRAPHICS_MODES ];
	if ( cachedModes != numGraphicsModes ) {
		for ( unsigned int i = 0; i < numGraphicsModes; ++i ) {
			descriptors[ i ] = graphicsModes[ i ].description;
		}
	}
	*numModes = numGraphicsModes;
	cachedModes = *numModes;

	return descriptors;
}

void PlgSetDriver( const char *mode ) {
	GfxLog( "Initializing graphics abstraction layer...\n" );

	const PLGDriverImportTable *interface = NULL;
	for ( unsigned int i = 0; i < numGraphicsModes; ++i ) {
		if ( pl_strcasecmp( mode, graphicsModes[ i ].description ) != 0 ) {
			continue;
		}

		interface = graphicsModes[ i ].interface;
		break;
	}

	if ( interface == NULL ) {
		PlReportErrorF( PL_RESULT_GRAPHICSINIT, "invalid graphics interface \"%s\" selected", mode );
		return;
	}

	if ( gfx_state.interface != NULL && interface == gfx_state.interface ) {
		PlReportErrorF( PL_RESULT_GRAPHICSINIT, "chosen interface \"%s\" is already active", mode );
		return;
	}

	gfx_state.interface = interface;

	CallGfxFunction( Initialize );
}
