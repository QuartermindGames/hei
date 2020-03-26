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
#include <PL/platform_graphics_camera.h>

#include "graphics_private.h"

/* todo:
 * - buffer upscaling and downscaling
 */

#define SWGetDisplayBufferSize(a) \
    plGetImageSize( PL_IMAGEFORMAT_RGBA8, (unsigned int) (a)->w, (unsigned int) (a)->h )
#define SWGetCurrentDisplayBuffer()    gfx_state.current_viewport->buffer

static void SWSetClearColour(PLColour colour) {}

static void SWClearBuffers(unsigned int buffers) {
    if (gfx_state.current_viewport->buffer == NULL) {
        return;
    }

    unsigned int size = SWGetDisplayBufferSize(gfx_state.current_viewport);
    if (buffers & PL_BUFFER_COLOUR) {
        for (unsigned int i = 0; i < size; i += 4) {
            SWGetCurrentDisplayBuffer()[i] = gfx_state.current_clearcolour.r;
            SWGetCurrentDisplayBuffer()[i + 1] = gfx_state.current_clearcolour.g;
            SWGetCurrentDisplayBuffer()[i + 2] = gfx_state.current_clearcolour.b;
            SWGetCurrentDisplayBuffer()[i + 3] = gfx_state.current_clearcolour.a;
        }
    }
}

/**********************************************************/
/** camera **/

static uint8_t *SWCreateDisplayBuffer(PLViewport *viewport) {
    free(viewport->buffer);

    viewport->buffer = (uint8_t *) pl_malloc(SWGetDisplayBufferSize(viewport));
    if (viewport->buffer == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));
    }

    viewport->oldW = viewport->w;
    viewport->oldH = viewport->h;
    return viewport->buffer;
}

static void SWCreateCamera(PLCamera *camera) {
    plAssert(camera);

    PLViewport *viewport = &camera->viewport;
    if (viewport->buffer != NULL) {
        /* only update the display buffer if the target size has changed */
        if (viewport->oldH != viewport->h && viewport->oldW != viewport->w) {
            SWCreateDisplayBuffer(viewport);
        }
        return;
    }

    SWCreateDisplayBuffer(viewport);
}

static void SWDestroyCamera(PLCamera *camera) {
    plAssert(camera);
} /* camera API takes care of deletion, so nothing to do here */

static void SWSetupCamera(PLCamera *camera) {
    plAssert(camera);

    /* only update the display buffer if the target size has changed */
    if (camera->viewport.oldH != camera->viewport.h &&
        camera->viewport.oldW != camera->viewport.w) {
        SWCreateDisplayBuffer(&camera->viewport);
    }
}

/**********************************************************/

static void SWDrawPixel(int x, int y, PLColour colour) {
    PLViewport *viewport = gfx_state.current_viewport;
    if (viewport->buffer == NULL) {
        return;
    }

    unsigned int pos = y * viewport->w + x;
    if (pos >= SWGetDisplayBufferSize(viewport)) {
        return;
    }

    PLColour *buffer = (PLColour *) (viewport->buffer);
    buffer[pos] = colour;
}

void plInitSoftwareGraphicsLayer(void) {
    gfx_layer.CreateCamera = SWCreateCamera;
    gfx_layer.DestroyCamera = SWDestroyCamera;
    gfx_layer.SetupCamera = SWSetupCamera;

    gfx_layer.ClearBuffers = SWClearBuffers;
    gfx_layer.SetClearColour = SWSetClearColour;

    gfx_layer.DrawPixel = SWDrawPixel;
}

void plShutdownSoftwareGraphicsLayer(void) {}