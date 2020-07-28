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

#include "platform_physics.h"
#include "platform_image.h"

enum {
    PL_CAMERA_MODE_PERSPECTIVE,
    PL_CAMERA_MODE_ORTHOGRAPHIC,
    PL_CAMERA_MODE_ISOMETRIC
};

typedef struct PLViewport {
    bool         auto_scale; //If true, viewport will update to match height/width of currently bound FBO when SetupCamera() is called
    int          x, y;
    int          w, h;
    int          oldW, oldH;
    uint8_t*     buffer;
    unsigned int bufferSize;
} PLViewport;

enum {
	PL_FRUSTUM_PLANE_RIGHT,
	PL_FRUSTUM_PLANE_LEFT,
	PL_FRUSTUM_PLANE_BOTTOM,
	PL_FRUSTUM_PLANE_TOP,
	PL_FRUSTUM_PLANE_FAR,
	PL_FRUSTUM_PLANE_NEAR,

	PL_MAX_FRUSTUM_PLANES
};

typedef PLVector4 PLViewFrustum[ PL_MAX_FRUSTUM_PLANES ];

typedef struct PLCamera {
    float fov;
    float near, far;
    unsigned int mode;

    PLVector3 angles, position;
    PLVector3 forward, up;

	PLViewFrustum frustum;

    PLCollisionAABB bounds;

    // Viewport
    PLViewport viewport;

    struct {
        PLMatrix4 proj;
        PLMatrix4 view;
    } internal;
} PLCamera;

PL_EXTERN_C

PL_EXTERN PLCamera *plCreateCamera(void);
PL_EXTERN void plDestroyCamera(PLCamera *camera);
PL_EXTERN void plSetupCamera(PLCamera *camera);

PL_EXTERN bool plIsBoxInsideView( const PLCamera *camera, const PLCollisionAABB *bounds );
PL_EXTERN bool plIsSphereInsideView( const PLCamera *camera, const PLCollisionSphere *sphere );

PL_EXTERN const PLViewport *plGetCurrentViewport(void);

PL_EXTERN_C_END
