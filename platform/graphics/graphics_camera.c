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

    pl_free(camera);
}

void plSetupCamera(PLCamera *camera) {
    plAssert(camera);

    camera->internal.proj = plMatrix4Identity();
    camera->internal.view = plMatrix4Identity();

    int w = camera->viewport.w;
    int h = camera->viewport.h;

    switch(camera->mode) {
        case PL_CAMERA_MODE_PERSPECTIVE: {
            camera->internal.proj = plPerspective(camera->fov, (float)w / (float)h, camera->near, camera->far);

            PLVector3 forward = PLVector3(
                    cosf(plToRadians(camera->angles.y)) * cosf(plToRadians(camera->angles.x)),
                    sinf(plToRadians(camera->angles.x)),
                    sinf(plToRadians(camera->angles.y)) * cosf(plToRadians(camera->angles.x))
                    );
            camera->forward = plNormalizeVector3(forward);
            camera->internal.view = plLookAt(camera->position, plAddVector3(camera->position, camera->forward), camera->up);
        } break;

        case PL_CAMERA_MODE_ORTHOGRAPHIC: {
            camera->internal.proj = plOrtho(0, w, h, 0, camera->near, camera->far);
        } break;

        case PL_CAMERA_MODE_ISOMETRIC: {
            camera->internal.proj = plOrtho(-camera->fov, camera->fov, -camera->fov, 5, -5, 40);
        } break;

        default: break;
    }

    // keep the gfx_state up-to-date on the situation
    gfx_state.current_viewport = camera->viewport;

    // Copy camera matrices
    gfx_state.view_matrix = camera->internal.view;
    gfx_state.projection_matrix = camera->internal.proj;

    CallGfxFunction(SetupCamera, camera);
}

const PLViewport *plGetCurrentViewport(void) {
    return &gfx_state.current_viewport;
}
