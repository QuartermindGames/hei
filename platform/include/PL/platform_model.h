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

#include "platform.h"
#include "platform_math.h"
#include "platform_graphics.h"

enum {
    PL_MODELTYPE_STATIC,
    PL_MODELTYPE_ANIMATED,
    PL_MODELTYPE_SKELETAL
};

#define PLMODEL_MAX_MESHES  32
#define PLMODEL_MAX_FRAMES  512

// Static mesh.
typedef struct PLStaticModel {
    uint32_t num_triangles;
    uint32_t num_vertices;
    uint32_t num_meshes;

    PLMesh *meshes[PLMODEL_MAX_MESHES];

    PLPhysicsAABB bounds;
} PLStaticModel;

// Per-vertex animated mesh.
typedef struct PLModelFrame {
    unsigned int num_meshes;

    PLMesh *meshes[PLMODEL_MAX_MESHES];

    PLPhysicsAABB bounds;
} PLModelFrame;

typedef struct PLAnimatedModel {
    uint32_t num_triangles;
    uint32_t num_vertices;
    uint32_t num_frames;

    PLMeshPrimitive primitive;

    PLModelFrame frames[PLMODEL_MAX_FRAMES];
} PLAnimatedModel;

// Mesh with bone structure.
typedef struct PLSkeletalModel {
    unsigned int num_triangles;
    unsigned int num_vertices;
    unsigned int num_meshes;

    PLMesh *meshes[PLMODEL_MAX_MESHES];

    PLPhysicsAABB bounds;
} PLSkeletalModel;

PL_EXTERN_C

// Static
PLStaticModel *plLoadStaticModel(const char *path);

void plDeleteStaticModel(PLStaticModel *model);
void plDrawStaticModel(PLStaticModel *model);

// Animated
PLAnimatedModel *plLoadAnimatedModel(const char *path);
void plDeleteAnimatedModel(PLAnimatedModel *model);

PL_EXTERN_C_END
