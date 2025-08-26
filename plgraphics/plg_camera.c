/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"
#include "qmos/public/qm_os_memory.h"

#define CAMERA_DEFAULT_BOUNDS 5
#define CAMERA_DEFAULT_FOV    75.f
#define CAMERA_DEFAULT_NEAR   0.1f
#define CAMERA_DEFAULT_FAR    1000.f

PLGCamera *PlgCreateCamera( void ) {
	PLGCamera *camera = QM_OS_MEMORY_CALLOC( 1, sizeof( PLGCamera ) );
	if ( camera == NULL ) {
		return NULL;
	}

	camera->fov = CAMERA_DEFAULT_FOV;
	camera->near = CAMERA_DEFAULT_NEAR;
	camera->far = CAMERA_DEFAULT_FAR;
	camera->mode = PLG_CAMERA_MODE_PERSPECTIVE;

	camera->bounds.mins = qm_math_vector3f(
	        -CAMERA_DEFAULT_BOUNDS, -CAMERA_DEFAULT_BOUNDS, -CAMERA_DEFAULT_BOUNDS );
	camera->bounds.maxs = qm_math_vector3f(
	        CAMERA_DEFAULT_BOUNDS, CAMERA_DEFAULT_BOUNDS, CAMERA_DEFAULT_BOUNDS );

	return camera;
}

void PlgDestroyCamera( PLGCamera *camera ) {
	if ( camera == NULL ) {
		return;
	}

	qm_os_memory_free( camera );
}

/**
 * Set the camera's field of view, carry out some basic validation.
 * (does not allow fov in excess of 1 or 179)
 */
void PlgSetCameraFieldOfView( PLGCamera *camera, float fieldOfView ) {
	if ( fieldOfView < 1.0f ) {
		fieldOfView = 1.0f;
	} else if ( fieldOfView > 179.0f ) {
		fieldOfView = 179.0f;
	}

	camera->fov = fieldOfView;
}

/**
 * Return the given camera's field of view.
 */
float PlgGetCameraFieldOfView( const PLGCamera *camera ) {
	return camera->fov;
}

void PlgMakeFrustumPlanes( const PLMatrix4 *matrix, PLGViewFrustum outFrustum ) {
	// Right
	outFrustum[ PLG_FRUSTUM_PLANE_RIGHT ].x = matrix->m[ 3 ] - matrix->m[ 0 ];
	outFrustum[ PLG_FRUSTUM_PLANE_RIGHT ].y = matrix->m[ 7 ] - matrix->m[ 4 ];
	outFrustum[ PLG_FRUSTUM_PLANE_RIGHT ].z = matrix->m[ 11 ] - matrix->m[ 8 ];
	outFrustum[ PLG_FRUSTUM_PLANE_RIGHT ].w = matrix->m[ 15 ] - matrix->m[ 12 ];
	outFrustum[ PLG_FRUSTUM_PLANE_RIGHT ] = PlNormalizePlane( outFrustum[ PLG_FRUSTUM_PLANE_RIGHT ] );
	// Left
	outFrustum[ PLG_FRUSTUM_PLANE_LEFT ].x = matrix->m[ 3 ] + matrix->m[ 0 ];
	outFrustum[ PLG_FRUSTUM_PLANE_LEFT ].y = matrix->m[ 7 ] + matrix->m[ 4 ];
	outFrustum[ PLG_FRUSTUM_PLANE_LEFT ].z = matrix->m[ 11 ] + matrix->m[ 8 ];
	outFrustum[ PLG_FRUSTUM_PLANE_LEFT ].w = matrix->m[ 15 ] + matrix->m[ 12 ];
	outFrustum[ PLG_FRUSTUM_PLANE_LEFT ] = PlNormalizePlane( outFrustum[ PLG_FRUSTUM_PLANE_LEFT ] );
	// Bottom
	outFrustum[ PLG_FRUSTUM_PLANE_BOTTOM ].x = matrix->m[ 3 ] - matrix->m[ 1 ];
	outFrustum[ PLG_FRUSTUM_PLANE_BOTTOM ].y = matrix->m[ 7 ] - matrix->m[ 5 ];
	outFrustum[ PLG_FRUSTUM_PLANE_BOTTOM ].z = matrix->m[ 11 ] - matrix->m[ 9 ];
	outFrustum[ PLG_FRUSTUM_PLANE_BOTTOM ].w = matrix->m[ 15 ] - matrix->m[ 13 ];
	outFrustum[ PLG_FRUSTUM_PLANE_BOTTOM ] = PlNormalizePlane( outFrustum[ PLG_FRUSTUM_PLANE_BOTTOM ] );
	// Top
	outFrustum[ PLG_FRUSTUM_PLANE_TOP ].x = matrix->m[ 3 ] + matrix->m[ 1 ];
	outFrustum[ PLG_FRUSTUM_PLANE_TOP ].y = matrix->m[ 7 ] + matrix->m[ 5 ];
	outFrustum[ PLG_FRUSTUM_PLANE_TOP ].z = matrix->m[ 11 ] + matrix->m[ 9 ];
	outFrustum[ PLG_FRUSTUM_PLANE_TOP ].w = matrix->m[ 15 ] + matrix->m[ 13 ];
	outFrustum[ PLG_FRUSTUM_PLANE_TOP ] = PlNormalizePlane( outFrustum[ PLG_FRUSTUM_PLANE_TOP ] );
	// Far
	outFrustum[ PLG_FRUSTUM_PLANE_FAR ].x = matrix->m[ 3 ] - matrix->m[ 2 ];
	outFrustum[ PLG_FRUSTUM_PLANE_FAR ].y = matrix->m[ 7 ] - matrix->m[ 6 ];
	outFrustum[ PLG_FRUSTUM_PLANE_FAR ].z = matrix->m[ 11 ] - matrix->m[ 10 ];
	outFrustum[ PLG_FRUSTUM_PLANE_FAR ].w = matrix->m[ 15 ] - matrix->m[ 14 ];
	outFrustum[ PLG_FRUSTUM_PLANE_FAR ] = PlNormalizePlane( outFrustum[ PLG_FRUSTUM_PLANE_FAR ] );
	// Near
	outFrustum[ PLG_FRUSTUM_PLANE_NEAR ].x = matrix->m[ 3 ] + matrix->m[ 2 ];
	outFrustum[ PLG_FRUSTUM_PLANE_NEAR ].y = matrix->m[ 7 ] + matrix->m[ 6 ];
	outFrustum[ PLG_FRUSTUM_PLANE_NEAR ].z = matrix->m[ 11 ] + matrix->m[ 10 ];
	outFrustum[ PLG_FRUSTUM_PLANE_NEAR ].w = matrix->m[ 15 ] + matrix->m[ 14 ];
	outFrustum[ PLG_FRUSTUM_PLANE_NEAR ] = PlNormalizePlane( outFrustum[ PLG_FRUSTUM_PLANE_NEAR ] );
}

void PlgSetupCameraFrustumFromMatrix( PLGCamera *camera, const PLMatrix4 *projMatrix, const PLMatrix4 *viewMatrix ) {
	PLMatrix4 viewProj = PlMultiplyMatrix4( projMatrix, viewMatrix );
	PlgMakeFrustumPlanes( &viewProj, camera->frustum );
}

void PlgSetupCameraFrustum( PLGCamera *camera ) {
	PlgSetupCameraFrustumFromMatrix( camera, &camera->internal.proj, &camera->internal.view );
}

static void SetupCameraAngles( PLGCamera *camera ) {
	PlMatrixMode( PL_VIEW_MATRIX );
	PlLoadIdentityMatrix();

	PlRotateMatrix3f( PL_DEG2RAD( -camera->angles.x ), 1.0f, 0.0f, 0.0f );
	PlRotateMatrix3f( PL_DEG2RAD( -camera->angles.y ), 0.0f, 1.0f, 0.0f );
	PlRotateMatrix3f( PL_DEG2RAD( -camera->angles.z ), 0.0f, 0.0f, 1.0f );

	PlTranslateMatrix( ( QmMathVector3f ) { -camera->position.x, -camera->position.y, -camera->position.z } );

	camera->internal.view = *PlGetMatrix( PL_VIEW_MATRIX );
}

static void SetupCameraPerspective( PLGCamera *camera, int width, int height ) {
	camera->internal.proj = PlPerspective( camera->fov, ( float ) width / ( float ) height, camera->near, camera->far );
}

/***** TEMPORARY CRAP START *****/
/* this whole API needs to be revisited... */

void PlgSetViewMatrix( const PLMatrix4 *viewMatrix ) { gfx_state.view_matrix = *viewMatrix; }
PLMatrix4 PlgGetViewMatrix( void ) { return gfx_state.view_matrix; }

void PlgSetProjectionMatrix( const PLMatrix4 *projMatrix ) { gfx_state.projection_matrix = *projMatrix; }
PLMatrix4 PlgGetProjectionMatrix( void ) { return gfx_state.projection_matrix; }

/***** TEMPORARY CRAP END 	*****/

void PlgGetViewport( int *x, int *y, int *width, int *height ) {
	if ( x != NULL ) {
		*x = gfx_state.viewport.x;
	}
	if ( y != NULL ) {
		*y = gfx_state.viewport.y;
	}
	if ( width != NULL ) {
		*width = gfx_state.viewport.w;
	}
	if ( height != NULL ) {
		*height = gfx_state.viewport.h;
	}
}

void PlgClipViewport( int x, int y, int width, int height ) {
	CallGfxFunction( ClipViewport, x, y, width, height );
}

void PlgSetViewport( int x, int y, int width, int height ) {
	gfx_state.viewport.x = x;
	gfx_state.viewport.y = y;
	gfx_state.viewport.w = width;
	gfx_state.viewport.h = height;

	CallGfxFunction( SetViewport, x, y, width, height );
}

void PlgSetupCamera( PLGCamera *camera ) {
	assert( camera );

	switch ( camera->mode ) {
		case PLG_CAMERA_MODE_PERSPECTIVE: {
			SetupCameraPerspective( camera, gfx_state.viewport.w, gfx_state.viewport.h );
			break;
		}
		case PLG_CAMERA_MODE_ORTHOGRAPHIC: {
			camera->internal.proj = PlOrtho( 0, ( float ) gfx_state.viewport.w, ( float ) gfx_state.viewport.h, 0, camera->near, camera->far );
			camera->internal.view = PlMatrix4Identity();
			break;
		}
		case PLG_CAMERA_MODE_ISOMETRIC: {
			camera->internal.proj = PlOrtho( -camera->fov, camera->fov, -camera->fov, 5, -5, 40 );
			camera->internal.view = PlMatrix4Identity();
			break;
		}
		default:
			break;
	}

	if ( camera->mode != PLG_CAMERA_MODE_ORTHOGRAPHIC ) {
		SetupCameraAngles( camera );
	}

	/* setup the camera frustum */
	PlgSetupCameraFrustum( camera );

	// Copy camera matrices
	PlgSetViewMatrix( &camera->internal.view );
	PlgSetProjectionMatrix( &camera->internal.proj );
}

/**
 * Checks that the given bounding box is within the view space.
 */
bool PlgIsBoxInsideView( const PLGCamera *camera, const PLCollisionAABB *bounds ) {
	QmMathVector3f mins = qm_math_vector3f_add( bounds->mins, bounds->origin );
	QmMathVector3f maxs = qm_math_vector3f_add( bounds->maxs, bounds->origin );
	for ( unsigned int i = 0; i < 5; ++i ) {
		if ( PlGetPlaneDotProduct( &camera->frustum[ i ], &QM_MATH_VECTOR3F( mins.x, mins.y, mins.z ) ) >= 0.0f ) {
			continue;
		}
		if ( PlGetPlaneDotProduct( &camera->frustum[ i ], &QM_MATH_VECTOR3F( maxs.x, mins.y, mins.z ) ) >= 0.0f ) {
			continue;
		}
		if ( PlGetPlaneDotProduct( &camera->frustum[ i ], &QM_MATH_VECTOR3F( mins.x, maxs.y, mins.z ) ) >= 0.0f ) {
			continue;
		}
		if ( PlGetPlaneDotProduct( &camera->frustum[ i ], &QM_MATH_VECTOR3F( maxs.x, maxs.y, mins.z ) ) >= 0.0f ) {
			continue;
		}
		if ( PlGetPlaneDotProduct( &camera->frustum[ i ], &QM_MATH_VECTOR3F( mins.x, mins.y, maxs.z ) ) >= 0.0f ) {
			continue;
		}
		if ( PlGetPlaneDotProduct( &camera->frustum[ i ], &QM_MATH_VECTOR3F( mins.x, maxs.y, maxs.z ) ) >= 0.0f ) {
			continue;
		}
		if ( PlGetPlaneDotProduct( &camera->frustum[ i ], &QM_MATH_VECTOR3F( maxs.x, maxs.y, maxs.z ) ) >= 0.0f ) {
			continue;
		}

		return false;
	}

	return true;
}

/**
 * Checks that the given sphere is within the view space.
 */
bool PlgIsSphereInsideView( const PLGCamera *camera, const PLCollisionSphere *sphere ) {
	for ( unsigned int i = 0; i < 5; ++i ) {
		if ( PlGetPlaneDotProduct( &camera->frustum[ i ], &sphere->origin ) < -sphere->radius ) {
			return false;
		}
	}

	return true;
}
