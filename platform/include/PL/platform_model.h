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
    char                name[64];
    PLAnimationFrame*   frames;
    unsigned int        num_frames;
    float               framerate;
} PLAnimation;

////////////////////////////////////////////////////////////////////////////


/* * * * * * * * * * * * * * * * * */

typedef enum PLModelType {
    PL_MODELTYPE_STATIC,    /* static non-animated */
    PL_MODELTYPE_VERTEX,    /* per-vertex animated */
    PL_MODELTYPE_SKELETAL,  /* skeletal/bones animated */

    PL_NUM_MODELTYPES
} PLModelType;

typedef struct PLStaticModelData {
    /* nothing to do? */
} PLStaticModelData;

typedef struct PLVertexAnimModelData {
    unsigned int    current_animation;  /* current animation index */
    unsigned int    current_frame;      /* current animation frame */
} PLVertexAnimModelData;

/* * * * * * * * * * * * * * * * * */
/* Skeletal Model Data */

typedef struct PLModelBoneWeight {
    PLVector3       direction;
} PLModelBoneWeight;

typedef struct PLModelBone {
    char            name[64];
    unsigned int    parent;
    PLVector3       position;
    PLQuaternion    orientation;
} PLModelBone;

typedef struct PLSkeletalModelData {
    PLModelBone*    bones;                      /* list of bones */
    unsigned int    num_bones;                  /* number of bones in the array */
    unsigned int    root_index;                 /* root bone */
    unsigned int    current_animation;          /* current animation index */
    unsigned int    current_frame;              /* current animation frame */
} PLSkeletalModelData;

/* * * * * * * * * * * * * * * * * */

typedef struct PLModelLod {
    PLMesh*         meshes;
    unsigned int    num_meshes;
} PLModelLod;

typedef struct PLModel {
    char            name[64];
    PLModelType     type;
    unsigned int    flags;
    float           radius;                     /* used for visibility culling */
    /* transformations */
    PLMatrix4x4     model_matrix;
    /* model lods */
    PLModelLod      levels[PL_MAX_MODEL_LODS];  /* different mesh sets for different levels of detail */
    unsigned int    num_levels;                 /* levels of detail provided */
    unsigned int    current_level;              /* current lod level, used for rendering */
    struct {
        /* model type data */
        union {
            PLSkeletalModelData     skeletal_data;  /* skeletal animation data */
            PLStaticModelData       static_data;    /* static model data */
            PLVertexAnimModelData   vertex_data;    /* per-vertex animation data */
        };
    } internal;
} PLModel;

PL_EXTERN_C

PLModel *plCreateModel(PLModelType type, unsigned int num_levels, PLModelLod levels[]);
PLModel *plLoadModel(const char *path);

void plDestroyModel(PLModel *model);

void plDrawModel(PLModel *model);
void plDrawModelSkeleton(PLModel *model);

void plApplyModelLighting(PLModel *model, PLLight *light, PLVector3 position);

void plRegisterModelLoader(const char *ext, PLModel*(*LoadFunction)(const char *path));
void plRegisterStandardModelLoaders(void);
void plClearModelLoaders(void);

void plGenerateModelNormals(PLModel *model);
void plGenerateModelBounds(PLModel *model);

PLModelLod *plGetModelLodLevel(PLModel *model, unsigned int level);

typedef enum PLModelOutputType {
    PL_MODEL_OUTPUT_DEFAULT,
    PL_MODEL_OUTPUT_SMD,

    PL_MAX_MODEL_OUTPUT_FORMATS
} PLModelOutputType;
bool plWriteModel(const char *path, PLModel *model, PLModelOutputType type);

enum {
    PL_SERIALIZE_MODEL_BASE,
    PL_SERIALIZE_MODEL_VERTICES,

    PL_SERIALIZE_MODEL_COMPLETE,

    PL_SERIALIZE_MODEL_END
};

uint8_t *plSerializeModel(PLModel *model, unsigned int type);

PL_EXTERN_C_END
