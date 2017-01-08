
#pragma once

#include "platform.h"

typedef struct PLCamera {
    PLfloat fov, fovx, fovy;

    PLVector3D angles, position;
} PLCamera;

PL_EXTERN_C

PL_EXTERN PLCamera *plCreateCamera(void);
PL_EXTERN void plDestroyCamera(PLCamera *camera);

PL_EXTERN void plPrintCameraAngles(PLCamera *camera);

PL_EXTERN_C_END
