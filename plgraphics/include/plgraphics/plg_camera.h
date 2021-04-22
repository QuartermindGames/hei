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

#pragma once

#include <PL/platform_physics.h>
#include <PL/platform_image.h>

enum {
	PLG_CAMERA_MODE_PERSPECTIVE,
	PLG_CAMERA_MODE_ORTHOGRAPHIC,
	PLG_CAMERA_MODE_ISOMETRIC
};

typedef struct PLGViewport {
	bool auto_scale;//If true, viewport will update to match
	                // height/width of currently bound FBO when
	                // SetupCamera() is called
	int x, y;
	int w, h;
	int oldW, oldH;
	uint8_t *buffer;
	unsigned int bufferSize;
} PLGViewport;

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

	// Viewport
	PLGViewport viewport;

	struct {
		PLMatrix4 proj;
		PLMatrix4 view;
	} internal;
} PLGCamera;

PL_EXTERN_C

PL_EXTERN PLGCamera *PlgCreateCamera( void );
PL_EXTERN void PlgDestroyCamera( PLGCamera *camera );

PL_EXTERN void PlgSetCameraFieldOfView( PLGCamera *camera, float fieldOfView );
PL_EXTERN float PlgGetCameraFieldOfView( const PLGCamera *camera );

PL_EXTERN void PlgSetupCamera( PLGCamera *camera );

PL_EXTERN bool PlgIsBoxInsideView( const PLGCamera *camera, const PLCollisionAABB *bounds );
PL_EXTERN bool PlgIsSphereInsideView( const PLGCamera *camera, const PLCollisionSphere *sphere );

PL_EXTERN const PLGViewport *PlgGetCurrentViewport( void );

PL_EXTERN_C_END
