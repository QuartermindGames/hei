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
#include "graphics_private.h"

#define CAMERA_DEFAULT_WIDTH      640
#define CAMERA_DEFAULT_HEIGHT     480
#define CAMERA_DEFAULT_BOUNDS     5
#define CAMERA_DEFAULT_FOV        90
#define CAMERA_DEFAULT_NEAR       0.1
#define CAMERA_DEFAULT_FAR        100000

void _plInitCameras(void) {
    gfx_state.max_cameras   = 1024;
    gfx_state.cameras       = (PLCamera**)malloc(sizeof(PLCamera) * gfx_state.max_cameras);
    gfx_state.num_cameras   = 0;
}

void _plShutdownCameras(void) {

}

/////////////////////////////////////////////////////////////////////////////////////

PLCamera *plCreateCamera(void) {
    PLCamera *camera = (PLCamera*)calloc(1, sizeof(PLCamera));
    if(camera == NULL) {
        ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for Camera, %d!\n", sizeof(PLCamera));
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
    camera->viewport.w          = CAMERA_DEFAULT_WIDTH;
    camera->viewport.h          = CAMERA_DEFAULT_HEIGHT;
    camera->viewport.r_width    = 0;
    camera->viewport.r_height   = 0;

    if(gfx_layer.CreateCamera) {
        gfx_layer.CreateCamera(camera);
    }

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

    if(gfx_layer.DeleteCamera) {
        gfx_layer.DeleteCamera(camera);
    }

    if(camera->viewport.v_buffer != NULL) {
        free(camera->viewport.v_buffer);
        camera->viewport.v_buffer = NULL;
    }

    free(camera);
}

void plSetupCamera(PLCamera *camera) {
    plAssert(camera);

    if(gfx_layer.SetupCamera) {
        gfx_layer.SetupCamera(camera);
    }

#if defined(PL_MODE_OPENGL)
    // todo, modernize start
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // modernize end

    switch(camera->mode) {
        case PL_CAMERA_MODE_PERSPECTIVE: {
            plPerspective(camera->fov, camera->viewport.w / camera->viewport.h, 0.1, 100000);

            // todo, modernize start
            glRotatef(camera->angles.y, 1, 0, 0);
            glRotatef(camera->angles.x, 0, 1, 0);
            glRotatef(camera->angles.z, 0, 0, 1);
            glTranslatef(camera->position.x, camera->position.y, camera->position.z);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            // modernize end

            break;
        }
            
        case PL_CAMERA_MODE_ORTHOGRAPHIC: {
            glOrtho(0, camera->viewport.w, camera->viewport.h, 0, 0, 1000);
            break;
        }
        
        case PL_CAMERA_MODE_ISOMETRIC: {
            glOrtho(-camera->fov, camera->fov, -camera->fov, 5, -5, 40);

            // todo, modernize start
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glRotatef(35.264f, 1, 0, 0);
            glRotatef(camera->angles.x, 0, 1, 0);

            glTranslatef(camera->position.x, camera->position.y, camera->position.z);
            // modernize end
            break;
        }

        default: break;
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////

void plDrawPerspective(void) {
    for(PLCamera **camera = gfx_state.cameras;
        camera < gfx_state.cameras + gfx_state.num_cameras; ++camera) {
        plAssert(camera);

        plSetupCamera((*camera));

        // todo, draw stuff...

        if(gfx_layer.DrawPerspectivePOST) {
            gfx_layer.DrawPerspectivePOST((*camera));
        }
    }
}
