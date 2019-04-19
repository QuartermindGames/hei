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
#include "platform_physics.h"

#define PL_MAX_MODEL_LODS       10

typedef struct PLAnimationFrame {
    PLVector3 transform;
} PLAnimationFrame;

typedef struct PLAnimation {
    char name[64];

    PLAnimationFrame *frames;
    unsigned int num_frames;

    float framerate;
} PLAnimation;

////////////////////////////////////////////////////////////////////////////

typedef struct PLBoneWeight {
    unsigned int bone_index;
    unsigned int vertex_index;

    float bone_weight;

    PLVector3 direction;
} PLBoneWeight;

typedef struct PLBone {
    char name[64];
    unsigned int parent;

    PLVector3 position;
    PLQuaternion orientation;
} PLBone;

/* * * * * * * * * * * * * * * * * */

typedef struct PLModelMesh {
    PLMesh *mesh;

    PLBoneWeight *bone_weights;
    unsigned int num_bone_weights;
} PLModelMesh;

typedef struct PLModel {
    char name[64];

    unsigned int flags;

    PLBone *bones;
    unsigned int num_bones;
    unsigned int root_index;

    PLModelMesh *meshes;
    unsigned int num_meshes;

    PLMatrix4x4 model_matrix;

    float radius;
    PLAABB bounds;

    struct {
        //unsigned int current_lod;
        unsigned int current_animation;
        unsigned int current_frame;
    } internal;
} PLModel;

PL_EXTERN_C

PLModel *plLoadModel(const char *path);

typedef enum PLModelOutputType {
    PL_MODEL_OUTPUT_DEFAULT,
    PL_MODEL_OUTPUT_SMD,
    PL_MAX_MODEL_OUTPUT_FORMATS
} PLModelOutputType;
bool plWriteModel(const char *path, const PLModel *model, PLModelOutputType type);

enum {
    PL_SERIALIZE_MODEL_BASE,
    PL_SERIALIZE_MODEL_VERTICES,

    PL_SERIALIZE_MODEL_COMPLETE,

    PL_SERIALIZE_MODEL_END
};

uint8_t *plSerializeModel(PLModel *model, unsigned int type);

///////////////////////////////////////////////////////////////////

void plDeleteModel(PLModel *model);

void plDrawModel(PLModel *model);
void plDrawModelSkeleton(PLModel *model);

void plApplyModelLighting(PLModel *model, PLLight *light, PLVector3 position);

void plRegisterModelLoader(const char *ext, PLModel*(*LoadFunction)(const char *path));

void plGenerateModelNormals(PLModel *model);
void plGenerateModelAABB(PLModel *model);

PL_EXTERN_C_END
