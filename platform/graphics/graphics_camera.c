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

#include <PL/platform_console.h>

#include "graphics_private.h"

#define CAMERA_DEFAULT_WIDTH      640
#define CAMERA_DEFAULT_HEIGHT     480
#define CAMERA_DEFAULT_BOUNDS     5
#define CAMERA_DEFAULT_FOV        75.f
#define CAMERA_DEFAULT_NEAR       0.1f
#define CAMERA_DEFAULT_FAR        1000.f

PLMatrix4 plLookAtTargetVector(PLVector3 eye, PLVector3 target) {
    return plLookAt(eye, target, PLVector3(0, 1, 0));
}

/////////////////////////////////////////////////////////////////////////////////////

PLCamera *plCreateCamera(void) {
    PLCamera *camera = (PLCamera*)pl_calloc(1, sizeof(PLCamera));
    if(camera == NULL) {
        return NULL;
    }

    camera->fov     = CAMERA_DEFAULT_FOV;
    camera->near    = CAMERA_DEFAULT_NEAR;
    camera->far     = CAMERA_DEFAULT_FAR;
    camera->mode    = PL_CAMERA_MODE_PERSPECTIVE;

    /*  XY * * * * W
     *  *
     *  *
     *  *
     *  *
     *  H
     */
    camera->viewport.w      = CAMERA_DEFAULT_WIDTH;
    camera->viewport.h      = CAMERA_DEFAULT_HEIGHT;

    camera->forward = PLVector3(0, 0, 1);
    camera->up = PLVector3(0, 1, 0);

    CallGfxFunction(CreateCamera, camera);

    camera->bounds.mins = PLVector3(
            -CAMERA_DEFAULT_BOUNDS, -CAMERA_DEFAULT_BOUNDS, -CAMERA_DEFAULT_BOUNDS);
    camera->bounds.maxs = PLVector3(
            CAMERA_DEFAULT_BOUNDS, CAMERA_DEFAULT_BOUNDS, CAMERA_DEFAULT_BOUNDS);

    return camera;
}

void plDestroyCamera(PLCamera *camera) {
    if(camera == NULL) {
        return;
    }

    CallGfxFunction(DestroyCamera, camera);

    pl_free( camera->viewport.buffer );
    pl_free( camera );
}

void plMakeFrustumPlanes( const PLMatrix4 *matrix, PLViewFrustum outFrustum ) {
	// Right
	outFrustum[ PL_FRUSTUM_PLANE_RIGHT ].x = matrix->m[ 3 ] - matrix->m[ 0 ];
	outFrustum[ PL_FRUSTUM_PLANE_RIGHT ].y = matrix->m[ 7 ] - matrix->m[ 4 ];
	outFrustum[ PL_FRUSTUM_PLANE_RIGHT ].z = matrix->m[ 11 ] - matrix->m[ 8 ];
	outFrustum[ PL_FRUSTUM_PLANE_RIGHT ].w = matrix->m[ 15 ] - matrix->m[ 12 ];
	outFrustum[ PL_FRUSTUM_PLANE_RIGHT ] = plNormalizePlane( outFrustum[ PL_FRUSTUM_PLANE_RIGHT ] );
	// Left
	outFrustum[ PL_FRUSTUM_PLANE_LEFT ].x = matrix->m[ 3 ] + matrix->m[ 0 ];
	outFrustum[ PL_FRUSTUM_PLANE_LEFT ].y = matrix->m[ 7 ] + matrix->m[ 4 ];
	outFrustum[ PL_FRUSTUM_PLANE_LEFT ].z = matrix->m[ 11 ] + matrix->m[ 8 ];
	outFrustum[ PL_FRUSTUM_PLANE_LEFT ].w = matrix->m[ 15 ] + matrix->m[ 12 ];
	outFrustum[ PL_FRUSTUM_PLANE_LEFT ] = plNormalizePlane( outFrustum[ PL_FRUSTUM_PLANE_LEFT ] );
	// Bottom
	outFrustum[ PL_FRUSTUM_PLANE_BOTTOM ].x = matrix->m[ 3 ] - matrix->m[ 1 ];
	outFrustum[ PL_FRUSTUM_PLANE_BOTTOM ].y = matrix->m[ 7 ] - matrix->m[ 5 ];
	outFrustum[ PL_FRUSTUM_PLANE_BOTTOM ].z = matrix->m[ 11 ] - matrix->m[ 9 ];
	outFrustum[ PL_FRUSTUM_PLANE_BOTTOM ].w = matrix->m[ 15 ] - matrix->m[ 13 ];
	outFrustum[ PL_FRUSTUM_PLANE_BOTTOM ] = plNormalizePlane( outFrustum[ PL_FRUSTUM_PLANE_BOTTOM ] );
	// Top
	outFrustum[ PL_FRUSTUM_PLANE_TOP ].x = matrix->m[ 3 ] + matrix->m[ 1 ];
	outFrustum[ PL_FRUSTUM_PLANE_TOP ].y = matrix->m[ 7 ] + matrix->m[ 5 ];
	outFrustum[ PL_FRUSTUM_PLANE_TOP ].z = matrix->m[ 11 ] + matrix->m[ 9 ];
	outFrustum[ PL_FRUSTUM_PLANE_TOP ].w = matrix->m[ 15 ] + matrix->m[ 13 ];
	outFrustum[ PL_FRUSTUM_PLANE_TOP ] = plNormalizePlane( outFrustum[ PL_FRUSTUM_PLANE_TOP ] );
	// Far
	outFrustum[ PL_FRUSTUM_PLANE_FAR ].x = matrix->m[ 3 ] - matrix->m[ 2 ];
	outFrustum[ PL_FRUSTUM_PLANE_FAR ].y = matrix->m[ 7 ] - matrix->m[ 6 ];
	outFrustum[ PL_FRUSTUM_PLANE_FAR ].z = matrix->m[ 11 ] - matrix->m[ 10 ];
	outFrustum[ PL_FRUSTUM_PLANE_FAR ].w = matrix->m[ 15 ] - matrix->m[ 14 ];
	outFrustum[ PL_FRUSTUM_PLANE_FAR ] = plNormalizePlane( outFrustum[ PL_FRUSTUM_PLANE_FAR ] );
	// Near
	outFrustum[ PL_FRUSTUM_PLANE_NEAR ].x = matrix->m[ 3 ] + matrix->m[ 2 ];
	outFrustum[ PL_FRUSTUM_PLANE_NEAR ].y = matrix->m[ 7 ] + matrix->m[ 6 ];
	outFrustum[ PL_FRUSTUM_PLANE_NEAR ].z = matrix->m[ 11 ] + matrix->m[ 10 ];
	outFrustum[ PL_FRUSTUM_PLANE_NEAR ].w = matrix->m[ 15 ] + matrix->m[ 14 ];
	outFrustum[ PL_FRUSTUM_PLANE_NEAR ] = plNormalizePlane( outFrustum[ PL_FRUSTUM_PLANE_NEAR ] );
}

void plSetupCamera(PLCamera *camera) {
    plAssert(camera);

    camera->internal.proj = plMatrix4Identity();
    camera->internal.view = plMatrix4Identity();

    float w = (float)camera->viewport.w;
    float h = (float)camera->viewport.h;

	switch ( camera->mode ) {
		case PL_CAMERA_MODE_PERSPECTIVE: {
			camera->internal.proj = plPerspective( camera->fov, w / h, camera->near, camera->far );

			float x = cosf( plDegreesToRadians( camera->angles.y ) ) * cosf( plDegreesToRadians( camera->angles.x ) );
			float y = sinf( plDegreesToRadians( camera->angles.x ) );
			float z = sinf( plDegreesToRadians( camera->angles.y ) ) * cosf( plDegreesToRadians( camera->angles.x ) );

			camera->forward = plNormalizeVector3( PLVector3( x, y, z ) );
			camera->internal.view = plLookAt( camera->position, plAddVector3( camera->position, camera->forward ), camera->up );
			break;
		}

		case PL_CAMERA_MODE_ORTHOGRAPHIC:
			camera->internal.proj = plOrtho( 0, w, h, 0, camera->near, camera->far );
			break;

		case PL_CAMERA_MODE_ISOMETRIC:
			camera->internal.proj = plOrtho( -camera->fov, camera->fov, -camera->fov, 5, -5, 40 );
			break;

		default:
			break;
	}

	/* setup the camera frustum */
	/* todo: this is currently incorrect!! */
	PLMatrix4 mvp = plMultiplyMatrix4( camera->internal.view, camera->internal.proj );
	plMakeFrustumPlanes( &mvp, camera->frustum );

    // keep the gfx_state up-to-date on the situation
    gfx_state.current_viewport = &camera->viewport;

    // Copy camera matrices
    gfx_state.view_matrix = camera->internal.view;
    gfx_state.projection_matrix = camera->internal.proj;

    CallGfxFunction(SetupCamera, camera);
}

/**
 * Checks that the given bounding box is within the view space.
 */
bool plIsBoxInsideView( const PLCamera *camera, const PLCollisionAABB *bounds ) {
	PLVector3 mins = plAddVector3( bounds->mins, bounds->origin );
	PLVector3 maxs = plAddVector3( bounds->maxs, bounds->origin );
	for ( unsigned int i = 0; i < 5; ++i ) {
		if ( plGetPlaneDotProduct( &camera->frustum[ i ], &PLVector3( mins.x, mins.y, mins.z ) ) >= 0.0f ) {
			continue;
		}
		if ( plGetPlaneDotProduct( &camera->frustum[ i ], &PLVector3( maxs.x, mins.y, mins.z ) ) >= 0.0f ) {
			continue;
		}
		if ( plGetPlaneDotProduct( &camera->frustum[ i ], &PLVector3( mins.x, maxs.y, mins.z ) ) >= 0.0f ) {
			continue;
		}
		if ( plGetPlaneDotProduct( &camera->frustum[ i ], &PLVector3( maxs.x, maxs.y, mins.z ) ) >= 0.0f ) {
			continue;
		}
		if ( plGetPlaneDotProduct( &camera->frustum[ i ], &PLVector3( mins.x, mins.y, maxs.z ) ) >= 0.0f ) {
			continue;
		}
		if ( plGetPlaneDotProduct( &camera->frustum[ i ], &PLVector3( mins.x, maxs.y, maxs.z ) ) >= 0.0f ) {
			continue;
		}
		if ( plGetPlaneDotProduct( &camera->frustum[ i ], &PLVector3( maxs.x, maxs.y, maxs.z ) ) >= 0.0f ) {
			continue;
		}

		return false;
	}

	return true;
}

/**
 * Checks that the given sphere is within the view space.
 */
bool plIsSphereInsideView( const PLCamera *camera, const PLCollisionSphere *sphere ) {
	for ( unsigned int i = 0; i < 5; ++i ) {
		if ( plGetPlaneDotProduct( &camera->frustum[ i ], &sphere->origin ) < -sphere->radius ) {
			return false;
		}
	}

	return true;
}

const PLViewport *plGetCurrentViewport(void) {
    return gfx_state.current_viewport;
}
