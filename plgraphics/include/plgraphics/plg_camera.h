// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_physics.h>
#include <plcore/pl_image.h>

enum {
	PLG_CAMERA_MODE_PERSPECTIVE,
	PLG_CAMERA_MODE_ORTHOGRAPHIC,
	PLG_CAMERA_MODE_ISOMETRIC
};

enum {
	PLG_FRUSTUM_PLANE_RIGHT,
	PLG_FRUSTUM_PLANE_LEFT,
	PLG_FRUSTUM_PLANE_BOTTOM,
	PLG_FRUSTUM_PLANE_TOP,
	PLG_FRUSTUM_PLANE_FAR,
	PLG_FRUSTUM_PLANE_NEAR,

	PLG_MAX_FRUSTUM_PLANES
};

typedef PLVector4 PLGViewFrustum[ PLG_MAX_FRUSTUM_PLANES ];

typedef struct PLGCamera {
	float fov;
	float near, far;
	unsigned int mode;

	PLVector3 angles, position;
	PLVector3 forward, up;

	PLGViewFrustum frustum;

	PLCollisionAABB bounds;

	struct {
		PLMatrix4 proj;
		PLMatrix4 view;
	} internal;
} PLGCamera;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PLGCamera *PlgCreateCamera( void );
void PlgDestroyCamera( PLGCamera *camera );

void PlgSetCameraFieldOfView( PLGCamera *camera, float fieldOfView );
float PlgGetCameraFieldOfView( const PLGCamera *camera );

void PlgSetupCamera( PLGCamera *camera );

bool PlgIsBoxInsideView( const PLGCamera *camera, const PLCollisionAABB *bounds );
bool PlgIsSphereInsideView( const PLGCamera *camera, const PLCollisionSphere *sphere );

void PlgGetViewport( int *x, int *y, int *width, int *height );
void PlgClipViewport( int x, int y, int width, int height );
void PlgSetViewport( int x, int y, int width, int height );

/***** TEMPORARY CRAP START *****/

void PlgSetViewMatrix( const PLMatrix4 *viewMatrix );
PLMatrix4 PlgGetViewMatrix( void );
void PlgSetProjectionMatrix( const PLMatrix4 *projMatrix );
PLMatrix4 PlgGetProjectionMatrix( void );

/***** TEMPORARY CRAP END 	*****/

#endif

PL_EXTERN_C_END
