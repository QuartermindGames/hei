/**
 * Hei Platform Library
 * Copyright (C) 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_console.h>
#include <plcore/pl_image.h>
#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"
#include "qmos/public/qm_os_memory.h"

/*	Graphics	*/

GfxState gfx_state;

/*	TODO:
- Add somewhere we can store tracking
data for each of these functions
- Display that data as an overlay?
*/
#define GRAPHICS_TRACK()                    \
	{                                       \
		static unsigned int num_calls = 0;  \
		if ( gfx_state.mode_debug ) {       \
			GfxLog( " %s\n", PL_FUNCTION ); \
			num_calls++;                    \
		}                                   \
	}

/*===========================
	INITIALIZATION
===========================*/

int LOG_LEVEL_GRAPHICS = 0;

void PlgInitializeInternalMeshes( void ); /* plg_draw.c */

PLFunctionResult PlgInitializeGraphics( void ) {
	memset( &gfx_state, 0, sizeof( GfxState ) );

	LOG_LEVEL_GRAPHICS = PlAddLogLevel( "plgraphics", ( QmMathColour4ub ) { 0, 255, 255, 255 },
#if !defined( NDEBUG )
	                                    true
#else
	                                    false
#endif
	);

	PlgInitializeInternalMeshes();

	return PL_RESULT_SUCCESS;
}

void PlgShutdownTextures( void );    // platform_graphics_texture
void PlgClearInternalMeshes( void ); /* plg_draw.c */

void PlgShutdownGraphics( void ) {
	GRAPHICS_TRACK();

	PlgClearInternalMeshes();
	PlgShutdownTextures();

	CallGfxFunction( Shutdown );
}

/*===========================
	DEBUGGING
===========================*/

void PlgInsertDebugMarker( const char *msg ) {
	CallGfxFunction( InsertDebugMarker, msg );
}

void PlgPushDebugGroupMarker( const char *msg ) {
	CallGfxFunction( PushDebugGroupMarker, msg );
}

void PlgPopDebugGroupMarker( void ) {
	CallGfxFunction( PopDebugGroupMarker );
}

/*===========================
	HARDWARE INFORMATION
===========================*/

bool PlgSupportsHWShaders( void ) {
	GRAPHICS_TRACK();
	CallReturningGfxFunction( SupportsHWShaders, false );
}

/*===========================
	FRAMEBUFFERS
===========================*/

static bool CreateFrameBuffer( PLGFrameBuffer *frameBuffer ) {
	CallReturningGfxFunction( CreateFrameBuffer, false, frameBuffer );
}

PLGFrameBuffer *PlgCreateFrameBuffer( unsigned int w, unsigned int h, unsigned int flags, unsigned int numSamples ) {
	if ( flags == 0 ) {
		return NULL;
	}

	PLGFrameBuffer *buffer = QM_OS_MEMORY_NEW( PLGFrameBuffer );
	buffer->width = w;
	buffer->height = h;
	buffer->flags = flags;
	buffer->numSamples = numSamples;

	if ( !CreateFrameBuffer( buffer ) ) {
		PlgDestroyFrameBuffer( buffer );
		return NULL;
	}

	return buffer;
}

void PlgDestroyFrameBuffer( PLGFrameBuffer *buffer ) {
	if ( !buffer ) {
		return;
	}

	CallGfxFunction( DeleteFrameBuffer, buffer );

	qm_os_memory_free( buffer );
}

PLGTexture *PlgGetFrameBufferTextureAttachment( PLGFrameBuffer *buffer, unsigned int component, PLGTextureFilter filter, PLGTextureWrapMode wrap ) {
	CallReturningGfxFunction( GetFrameBufferTextureAttachment, NULL, buffer, component, filter, wrap );
}

// should really be 'GetFrameBufferSize', urgh...
void PlgGetFrameBufferResolution( const PLGFrameBuffer *buffer, unsigned int *width, unsigned int *height ) {
	*width = buffer->width;
	*height = buffer->height;
}

void PlgBindFrameBuffer( PLGFrameBuffer *buffer, PLGFrameBufferObjectTarget target_binding ) {
	//NOTE: NULL is valid for *buffer, to bind the SDL window default backbuffer
	CallGfxFunction( BindFrameBuffer, buffer, target_binding );
}

void PlgBlitFrameBuffers( PLGFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLGFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, unsigned int mask, bool linear ) {
	//NOTE: NULL is valid for *srcBuffer/*dstBuffer, to bind the SDL window default backbuffer
	//      SRC and DST can be the same buffer, in order to quickly copy a subregion of the buffer to a new location
	CallGfxFunction( BlitFrameBuffers, src_buffer, src_w, src_h, dst_buffer, dst_w, dst_h, mask, linear );
}

void PlgSetClearColour( QmMathColour4ub rgba ) {
	if ( PlCompareColour( rgba, gfx_state.current_clearcolour ) ) {
		return;
	}

	CallGfxFunction( SetClearColour, rgba );

	gfx_state.current_clearcolour = rgba;
}

void PlgClearBuffers( unsigned int buffers ) {
	CallGfxFunction( ClearBuffers, buffers );
}

/**
 * Resizes the given framebuffer to the specified size.
 */
bool PlgSetFrameBufferSize( PLGFrameBuffer *frameBuffer, unsigned int width, unsigned int height ) {
	assert( width != 0 && height != 0 );
	if ( width == 0 ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "invalid width" );
		return false;
	} else if ( height == 0 ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM3, "invalid height" );
		return false;
	}

	if ( frameBuffer->width == width && frameBuffer->height == height ) {
		return true;
	}

	CallGfxFunction( SetFrameBufferSize, frameBuffer, width, height );
	return true;
}

void *PlgReadFrameBufferRegion( PLGFrameBuffer *frameBuffer, uint32_t x, uint32_t y, uint32_t w, uint32_t h, size_t dstSize, void *dstBuf ) {
	CallReturningGfxFunction( ReadFrameBufferRegion, NULL, frameBuffer, x, y, w, h, dstSize, dstBuf );
}

/*===========================
	CAPABILITIES
===========================*/

bool PlgIsGraphicsStateEnabled( PLGDrawState state ) {
	return gfx_state.current_capabilities[ state ];
}

void PlgEnableGraphicsState( PLGDrawState state ) {
	if ( PlgIsGraphicsStateEnabled( state ) ) {
		return;
	}

	CallGfxFunction( EnableState, state );

	gfx_state.current_capabilities[ state ] = true;
}

void PlgDisableGraphicsState( PLGDrawState state ) {
	if ( !PlgIsGraphicsStateEnabled( state ) ) {
		return;
	}

	CallGfxFunction( DisableState, state );

	gfx_state.current_capabilities[ state ] = false;
}

/*===========================
	DRAW
===========================*/

void PlgSetBlendMode( PLGBlend a, PLGBlend b ) {
	CallGfxFunction( SetBlendMode, a, b );
}

void PlgSetCullMode( PLGCullMode mode ) {
	if ( mode == gfx_state.current_cullmode ) {
		return;
	}

	CallGfxFunction( SetCullMode, mode );

	gfx_state.current_cullmode = mode;
}

void PlgSetDepthBufferMode( unsigned int mode ) {
	CallGfxFunction( SetDepthBufferMode, mode );
}

void PlgDepthMask( bool enable ) {
	CallGfxFunction( DepthMask, enable );
}

void PlgColourMask( bool r, bool g, bool b, bool a ) {
	CallGfxFunction( ColourMask, r, g, b, a );
}

void PlgStencilMask( unsigned int mask ) {
	CallGfxFunction( StencilMask, mask );
}

/*===========================
	BUFFER OPERATIONS
===========================*/

void PlgDepthBufferFunction( PLGCompareFunction compareFunction ) {
	CallGfxFunction( DepthBufferFunction, compareFunction );
}

void PlgStencilBufferFunction( PLGCompareFunction function, int reference, unsigned int mask ) {
	CallGfxFunction( StencilBufferFunction, function, reference, mask );
}

void PlgStencilOp( PLGStencilFace face, PLGStencilOp stencilFailOp, PLGStencilOp depthFailOp, PLGStencilOp depthPassOp ) {
	CallGfxFunction( StencilOp, face, stencilFailOp, depthFailOp, depthPassOp );
}

/////////////////////////////////////////////////////////////////////////////////////

void PlgSetClipPlane( const QmMathVector4f *clip, const PLMatrix4 *clipMatrix, bool transpose ) {
	CallGfxFunction( SetClipPlane, clip, clipMatrix, transpose );
}
