
#pragma once

#include "platform.h"

typedef struct PLViewport {
    PLint x, y;

    PLuint width, height;

    struct PLViewport *parent, **children;
} PLViewport;

PL_EXTERN_C

PL_EXTERN void plSetViewportSize(PLViewport *viewport, PLuint width, PLuint height);
PL_EXTERN void plSetViewportPosition(PLViewport *viewport, PLint x, PLint y);

PL_EXTERN void plScreenshot(PLViewport *viewport, const PLchar *path);

PL_EXTERN_C_END
