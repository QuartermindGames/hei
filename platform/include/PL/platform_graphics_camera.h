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
    int x, y;
    unsigned int w, h;

    uint8_t *v_buffer;
    unsigned int buffers[32];

    PLImageFormat format;

    unsigned int r_w, r_h;
    unsigned int old_r_w, old_r_h;
} PLViewport;

typedef struct PLCamera {
    float fov;
    float near, far;
    unsigned int mode;

    PLVector3 angles, position;
    PLVector3 forward, up;

    PLAABB bounds;

    // Viewport
    PLViewport viewport;

    PLMatrix4x4 perspective;
    PLMatrix4x4 view;
} PLCamera;

PL_EXTERN_C

PLMatrix4x4 plTranslate(PLVector3 vec);
PLMatrix4x4 plOrtho(float left, float right, float bottom, float top, float near, float far);
PLMatrix4x4 plFrustum(float left, float right, float bottom, float top, float near, float far);
PLMatrix4x4 plPerspective(float fov, float aspect, float near, float far);

PL_EXTERN PLCamera *plCreateCamera(void);
PL_EXTERN void plDeleteCamera(PLCamera *camera);

PL_EXTERN void plSetupCamera(PLCamera *camera);

PL_EXTERN void plDrawPerspectivePOST(PLCamera *camera);

PL_EXTERN_C_END