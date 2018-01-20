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
#define CAMERA_DEFAULT_FOV        90
#define CAMERA_DEFAULT_NEAR       0.1
#define CAMERA_DEFAULT_FAR        100000

void InitCameras(void) {
    gfx_state.max_cameras   = 1024;
    gfx_state.cameras       = (PLCamera**)malloc(sizeof(PLCamera) * gfx_state.max_cameras);
    gfx_state.num_cameras   = 0;
}

void ShutdownCameras(void) {

}

PLMatrix4x4 LookAt(PLVector3 eye, PLVector3 center, PLVector3 up) {
    PLVector3 z = eye;
    plSubtractVector3(&z, center);
    float mag = plVector3Length(z);
    if(mag > 0) {
        plDivideVector3f(&z, mag);
    }

    PLVector3 y = up;
    PLVector3 x = plVector3CrossProduct(y, z);
    y = plVector3CrossProduct(z, x);

    mag = plVector3Length(x);
    if(mag > 0) {
        plDivideVector3f(&x, mag);
    }

    mag = plVector3Length(y);
    if(mag > 0) {
        plDivideVector3f(&y, mag);
    }

    return (PLMatrix4x4){{
            x.x, y.x, z.x, 0,
            x.y, y.y, z.y, 0,
            x.z, y.z, z.z, 0,
            0  , 0  , 0  , 1
    }};
}

/////////////////////////////////////////////////////////////////////////////////////

PLCamera *plCreateCamera(void) {
    PLCamera *camera = (PLCamera*)calloc(1, sizeof(PLCamera));
    if(camera == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for Camera, %d!\n", sizeof(PLCamera));
        return NULL;
    }

    memset(camera, 0, sizeof(PLCamera));
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
    camera->viewport.r_w    = 0;
    camera->viewport.r_h    = 0;

    CallGfxFunction(CreateCamera, camera);

    camera->bounds.mins = PLVector3(
            -CAMERA_DEFAULT_BOUNDS, -CAMERA_DEFAULT_BOUNDS, -CAMERA_DEFAULT_BOUNDS);
    camera->bounds.maxs = PLVector3(
            CAMERA_DEFAULT_BOUNDS, CAMERA_DEFAULT_BOUNDS, CAMERA_DEFAULT_BOUNDS);

    return camera;
}

void plDeleteCamera(PLCamera *camera) {
    if(camera == NULL) {
        return;
    }

    CallGfxFunction(DeleteCamera, camera);

    free(camera->viewport.v_buffer);
    free(camera);
}

void plSetupCamera(PLCamera *camera) {
    plAssert(camera);

    CallGfxFunction(SetupCamera, camera);
}

void plDrawPerspectivePOST(PLCamera *camera) {
    CallGfxFunction(DrawPerspectivePOST, camera);
}
