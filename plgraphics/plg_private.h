// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_console.h>

#include <plgraphics/plg.h>
#include <plgraphics/plg_texture.h>
#include <plgraphics/plg_camera.h>

PL_EXTERN_C

#if !defined( NDEBUG )
extern int LOG_LEVEL_GRAPHICS;
#	define GfxLog( FORMAT, ... ) PlLogWFunction( LOG_LEVEL_GRAPHICS, FORMAT, ##__VA_ARGS__ )
#else
#	define GfxLog( ... )
#endif

typedef struct GfxState {
	const PLGDriverImportTable *interface;

	PLGCullMode current_cullmode;
	PLColourF32 current_clearcolour;

	bool current_capabilities[ PLG_GFX_MAX_STATES ];// Enabled capabilities.
	unsigned int current_textureunit;

	// Textures

	PLGTextureMappingUnit *tmu;

	// Shader states

	PLGShaderProgram *current_program;
	PLPath shaderCacheLocation; /* where shaders should be cached to, if supported */

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

	PLGCamera **cameras;

	unsigned int num_cameras;
	unsigned int max_cameras;

	PLMatrix4 projection_matrix;
	PLMatrix4 view_matrix;

	////////////////////////////////////////

	struct {
		int x, y;
		int w, h;
	} viewport;

	bool mode_debug;
} GfxState;

#define CallGfxFunction( FUNCTION, ... )                                   \
	if ( gfx_state.interface != NULL ) {                                   \
		if ( gfx_state.interface->FUNCTION != NULL )                       \
			gfx_state.interface->FUNCTION( __VA_ARGS__ );                  \
		else                                                               \
			GfxLog( "Unbound layer function %s was called\n", #FUNCTION ); \
	}
#define CallReturningGfxFunction( FUNCTION, RETURN, ... )                  \
	if ( gfx_state.interface != NULL ) {                                   \
		if ( gfx_state.interface->FUNCTION != NULL ) {                     \
			return gfx_state.interface->FUNCTION( __VA_ARGS__ );           \
		} else {                                                           \
			GfxLog( "Unbound layer function %s was called\n", #FUNCTION ); \
			return ( RETURN );                                             \
		}                                                                  \
	} else {                                                               \
		return ( RETURN );                                                 \
	}

///////////////////////////////////////////////////////

#if 1
#	define UseBufferScaling( a )                                   \
		( ( a )->viewport.r_w != 0 && ( a )->viewport.r_h != 0 ) && \
		        ( ( a )->viewport.r_w != ( a )->viewport.w && ( a )->viewport.r_h != ( a )->viewport.h )
#else /* for debugging purposes */
#	define UseBufferScaling( a ) 0
#endif

///////////////////////////////////////////////////////

PL_EXTERN GfxState gfx_state;

PL_EXTERN_C_END
