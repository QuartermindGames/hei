
#pragma once

#include "platform.h"

static PL_INLINE void *plAllocateMemory(PLuint num, PLuint size) {
    plFunctionStart();

    void *data = calloc(num, size);
    if(!data) {
        plSetError("Failed to allocate memory! (%d * %d)", size, num);
        return NULL;
    }

    return data;
}

static PL_INLINE void plFreeMemory(void *data) {
    plFunctionStart();

    if(!data) {
        return;
    }

    free(data);
}
