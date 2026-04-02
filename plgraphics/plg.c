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
	PlgClearInternalMeshes();
	PlgShutdownTextures();

	CallGfxFunction( Shutdown );
}

/*===========================
	DEBUGGING
===========================*/

void qm_gfx_debug_insert_marker( const char *msg )
{
	CallGfxFunction( InsertDebugMarker, msg );
}

void qm_gfx_debug_push_group_marker( const char *msg )
{
	CallGfxFunction( PushDebugGroupMarker, msg );
}

void qm_gfx_debug_pop_group_marker( void )
{
	CallGfxFunction( PopDebugGroupMarker );
}

/*===========================
	HARDWARE INFORMATION
===========================*/

bool PlgSupportsHWShaders( void ) {
	CallReturningGfxFunction( SupportsHWShaders, false );
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

/***** TEMPORARY CRAP START *****/
/* this whole API needs to be revisited... */

void      PlgSetViewMatrix( const PLMatrix4 *viewMatrix ) { gfx_state.view_matrix = *viewMatrix; }
PLMatrix4 PlgGetViewMatrix( void ) { return gfx_state.view_matrix; }

void      PlgSetProjectionMatrix( const PLMatrix4 *projMatrix ) { gfx_state.projection_matrix = *projMatrix; }
PLMatrix4 PlgGetProjectionMatrix( void ) { return gfx_state.projection_matrix; }

/***** TEMPORARY CRAP END 	*****/

void qm_gfx_clip_viewport( int x, int y, int width, int height )
{
	CallGfxFunction( ClipViewport, x, y, width, height );
}

void qm_gfx_set_viewport( int x, int y, int width, int height )
{
	gfx_state.viewport.x = x;
	gfx_state.viewport.y = y;
	gfx_state.viewport.w = width;
	gfx_state.viewport.h = height;

	CallGfxFunction( SetViewport, x, y, width, height );
}

void qm_gfx_get_viewport( int *x, int *y, int *width, int *height )
{
	if ( x != NULL )
	{
		*x = gfx_state.viewport.x;
	}
	if ( y != NULL )
	{
		*y = gfx_state.viewport.y;
	}
	if ( width != NULL )
	{
		*width = gfx_state.viewport.w;
	}
	if ( height != NULL )
	{
		*height = gfx_state.viewport.h;
	}
}
