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

/* http://www.songho.ca/opengl/gl_anglestoaxes.html */
void AnglesAxes(PLVector3 angles, PLVector3 *left, PLVector3 *up, PLVector3 *forward) {
    /* pitch */
    float theta = angles.x * PL_PI_DIV_180;
    float sx = sinf(theta);
    float cx = cosf(theta);

    /* yaw */
    theta = angles.y * PL_PI_DIV_180;
    float sy = sinf(theta);
    float cy = cosf(theta);

    /* roll */
    theta = angles.z * PL_PI_DIV_180;
    float sz = sinf(theta);
    float cz = cosf(theta);

    left->x = cy * cz;
    left->y = sx * sy * cz + cx * sz;
    left->z = -cx * sy * cz + sx * sz;

    up->x = -cy * sz;
    up->y = -sx * sy * sz + cx * cz;
    up->z = cx * sy * sz + sx * cz;

    forward->x = sy;
    forward->y = -sx * cy;
    forward->z = cx * cy;
}

PLMatrix4x4 plTranslate(PLVector3 vec) {
    return (PLMatrix4x4) {{
            1, 0, 0, vec.x,
            0 ,1, 0, vec.y,
            0, 0, 1, vec.z,
            0, 0, 0, 1
    }};
}

PLMatrix4x4 plOrtho(float left, float right, float bottom, float top, float near, float far) {
    return (PLMatrix4x4) {{
            2 / (right - left), 0, 0, -(right + left) / (right - left),
            0, 2 / (top - bottom), 0, -(top + bottom) / (top - bottom),
            0, 0, 2 / (far - near), -(far + near) / (far - near),
            0, 0, 0, 1
    }};
}

PLMatrix4x4 plFrustum(float left, float right, float bottom, float top, float near, float far) {
    float m0 = 2.f * near;
    float m1 = right - left;
    float m2 = top - bottom;
    float m3 = far - near;
    return (PLMatrix4x4){{
            m0 / m1, 0, 0, 0,
            0, m0 / m2, 0, 0,
            (right + left) / m1, (top + bottom) / m2, (-far - near) / m3, -1.f,
            0, 0, 0, 1
    }};
}

PLMatrix4x4 plPerspective(float fov, float aspect, float near, float far) {
    float y_max = near * tanf(fov * PL_PI / 360);
    float x_max = y_max * aspect;
    return plFrustum(-x_max, x_max, -y_max, y_max, near, far);
}

PLMatrix4x4 plLookAt(PLVector3 eye, PLVector3 center, PLVector3 up) {
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

PLMatrix4x4 plLookAtTargetVector(PLVector3 eye, PLVector3 target) {
    return plLookAt(eye, target, PLVector3(0, 1, 0));
}

/////////////////////////////////////////////////////////////////////////////////////

PLCamera *plCreateCamera(void) {
    PLCamera *camera = (PLCamera*)calloc(1, sizeof(PLCamera));
    if(camera == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for Camera, %d!\n", sizeof(PLCamera));
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
    camera->viewport.r_w    = 0;
    camera->viewport.r_h    = 0;

    camera->forward = PLVector3(0, 0, 1);
    camera->up = PLVector3(0, 1, 0);

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

PLMatrix4x4 plGetCameraViewProjection(PLCamera *camera) {
    PLMatrix4x4 mat = plLookAt(
            camera->position,
            plVector3Add(camera->position, camera->forward),
            camera->up
    );
}

void plSetupCamera(PLCamera *camera) {
    plAssert(camera);

    CallGfxFunction(SetupCamera, camera);
}

void plDrawPerspectivePOST(PLCamera *camera) {
    CallGfxFunction(DrawPerspectivePOST, camera);
}
