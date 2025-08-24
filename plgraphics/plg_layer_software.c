/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"

/* todo:
 * - buffer upscaling and downscaling
 */

static uint8_t *drawBuffer = NULL;

#define SWGetDisplayBufferSize( WIDTH, HEIGHT ) PlGetImageSize( PL_IMAGEFORMAT_RGBA8, WIDTH, HEIGHT )

static void SWSetClearColour( PLColour colour ) {}

static void SWClearBuffers( unsigned int buffers ) {
	if ( drawBuffer == NULL ) {
		return;
	}

	unsigned int size = SWGetDisplayBufferSize( gfx_state.viewport.w, gfx_state.viewport.h );
	if ( buffers & PLG_BUFFER_COLOUR ) {
		for ( unsigned int i = 0; i < size; i += 4 ) {
			drawBuffer[ i ] = gfx_state.current_clearcolour.r;
			drawBuffer[ i + 1 ] = gfx_state.current_clearcolour.g;
			drawBuffer[ i + 2 ] = gfx_state.current_clearcolour.b;
			drawBuffer[ i + 3 ] = gfx_state.current_clearcolour.a;
		}
	}
}

/**********************************************************/

static void SWDrawPixel( int x, int y, PLColour colour ) {
	unsigned int pos = y * gfx_state.viewport.w + x;
	if ( pos >= SWGetDisplayBufferSize( gfx_state.viewport.w, gfx_state.viewport.h ) ) {
		return;
	}

	PLColour *buffer = ( PLColour * ) drawBuffer;
	buffer[ pos ] = colour;
}

static void SWDrawLine( const PLGVertex *start, const PLGVertex *end ) {
}

static void SWDrawMesh( PLGMesh *mesh ) {
	PLVector3 transform = PlGetMatrix4Translation( PlGetMatrix( PL_MODELVIEW_MATRIX ) );
	switch ( mesh->primitive ) {
		case PLG_MESH_LINES: {
			for ( unsigned int i = 0; i < mesh->num_verts; i += 2 ) {
				PLGVertex a = mesh->vertices[ i ];
				a.position = qm_math_vector3f_scale( a.position, transform );
				PLGVertex b = mesh->vertices[ i ];
				b.position = qm_math_vector3f_scale( b.position, transform );
				SWDrawLine( &a, &b );
			}
			break;
		}
		default: /* for now... */
		case PLG_MESH_POINTS: {
			for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
				PLGVertex a = mesh->vertices[ i ];
				a.position = qm_math_vector3f_scale( a.position, transform );
				SWDrawPixel( ( int ) a.position.x, ( int ) a.position.y, a.colour );
			}
			break;
		}
	}
}

static bool SWSupportsHWShaders( void ) {
	return false;
}
