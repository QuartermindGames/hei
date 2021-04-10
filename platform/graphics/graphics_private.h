/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#pragma once

#include "platform_private.h"

#include <PL/pl_graphics.h>
#include <PL/pl_graphics_texture.h>
#include <PL/pl_graphics_font.h>
#include <PL/pl_graphics_camera.h>

#ifdef _DEBUG
#   define GfxLog( ... ) plLogMessage(LOG_LEVEL_GRAPHICS, __VA_ARGS__)
#else
#   define GfxLog(...)
#endif

typedef struct GfxState {
	const PLGraphicsInterface *interface;

	PLCullMode current_cullmode;

	PLColour current_clearcolour;
	PLColour current_colour;        // Current global colour.

	bool current_capabilities[PL_GFX_MAX_STATES];    // Enabled capabilities.
	unsigned int current_textureunit;

	// Textures

	PLTextureMappingUnit *tmu;
	PLTexture **textures;

	unsigned int num_textures;
	unsigned int max_textures;

	// Shader states

	PLShaderProgram *current_program;

	// Hardware / Driver information

	const char *hw_vendor;
	const char *hw_renderer;
	const char *hw_version;
	const char *hw_extensions;

	unsigned int hw_maxtexturesize;
	unsigned int hw_maxtextureunits;
	unsigned int hw_maxtextureanistropy;

	// Lighting

	unsigned int num_lights;

	// Cameras

	PLCamera **cameras;

	unsigned int num_cameras;
	unsigned int max_cameras;

	PLMatrix4 projection_matrix;
	PLMatrix4 view_matrix;

	////////////////////////////////////////

	PLViewport *current_viewport;

	bool mode_debug;
} GfxState;

#if defined(PL_USE_GRAPHICS)
#   define CallGfxFunction( FUNCTION, ... ) \
    if(gfx_state.interface != NULL && gfx_state.interface->FUNCTION != NULL) { \
        gfx_state.interface->FUNCTION(__VA_ARGS__); \
    } else { \
        GfxLog("Unbound layer function %s was called\n", #FUNCTION); \
    }
#   define CallReturningGfxFunction( FUNCTION, RETURN, ... ) \
	if ( gfx_state.interface != NULL && gfx_state.interface->FUNCTION != NULL ) return gfx_state.interface->FUNCTION( __VA_ARGS__ ); \
    else { GfxLog( "Unbound layer function %s was called\n", #FUNCTION ); return ( RETURN ); }
#else
#   define CallGfxFunction(FUNCTION, ...)
#endif

///////////////////////////////////////////////////////

#if 1
#   define UseBufferScaling( a ) \
    ((a)->viewport.r_w != 0 && (a)->viewport.r_h != 0) && \
    ((a)->viewport.r_w != (a)->viewport.w && (a)->viewport.r_h != (a)->viewport.h)
#else /* for debugging purposes */
#   define UseBufferScaling(a) 0
#endif

///////////////////////////////////////////////////////

#define PL_POLYGON_MAX_SIDES 32

typedef struct PLPolygon {
	PLTexture *texture;
	PLVector2 textureOffset;
	PLVector2 textureScale;
	float textureRotation;

	PLVector3 normal;

	PLVertex vertices[ PL_POLYGON_MAX_SIDES ];
	unsigned int numVertices;
} PLPolygon;

PL_EXTERN_C

PL_EXTERN GfxState gfx_state;

void _plBindTexture( const PLTexture *texture );

#if defined( PL_SUPPORT_VULKAN )
void plInitVulkan( void );
void plShutdownVulkan( void );
#endif

PL_EXTERN_C_END
