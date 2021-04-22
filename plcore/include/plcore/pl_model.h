/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <PL/platform.h>
#include <PL/platform_math.h>
#include <PL/platform_physics.h>

#define PL_MAX_MODEL_LODS       5

typedef struct PLAnimationFrame {
    PLVector3 transform;
} PLAnimationFrame;

typedef struct PLAnimation {
    char                name[64];
    PLAnimationFrame*   frames;
    uint32_t            num_frames;
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

#if 0
typedef struct PLStaticModelData {
    /* nothing to do? */
} PLStaticModelData;
#endif

typedef struct PLVertexAnimationFrame {
    /* todo: store submeshes into PLMesh struct */
    PLMesh**    meshes;
    uint32_t    num_meshes;
} PLVertexAnimationFrame;

typedef struct PLVertexAnimModelData {
    uint32_t                current_animation;  /* current animation index */
    uint32_t                current_frame;      /* current animation frame */
    PLVertexAnimationFrame* animations;
} PLVertexAnimModelData;

/* * * * * * * * * * * * * * * * * */
/* Skeletal Model Data */

typedef struct PLModelBoneWeight {
    PLVector3       direction;
} PLModelBoneWeight;

typedef struct PLModelBone {
    char            name[64];
    uint32_t        parent;
    PLVector3       position;
    PLQuaternion    orientation;
} PLModelBone;

typedef struct PLSkeletalModelData {
    PLModelBone*    bones;                      /* list of bones */
    uint32_t        num_bones;                  /* number of bones in the array */
    uint32_t        root_index;                 /* root bone */
    uint32_t        current_animation;          /* current animation index */
    uint32_t        current_frame;              /* current animation frame */
} PLSkeletalModelData;

/* * * * * * * * * * * * * * * * * */

typedef struct PLModelLod {
    PLMesh**    meshes;
    uint32_t    num_meshes;
} PLModelLod;

typedef struct PLModel {
    char            name[64];
    char            path[PL_SYSTEM_MAX_PATH];
    PLModelType     type;
    uint16_t        flags;
    /* used for visibility culling */
    float           radius;
    PLAABB          bounds;
    /* transformations */
    PLMatrix4       model_matrix;
    /* model lods (todo: kill...) */
    PLModelLod      levels[PL_MAX_MODEL_LODS];  /* different mesh sets for different levels of detail */
    uint8_t         num_levels;                 /* levels of detail provided */
    uint8_t         current_level;              /* current lod level, used for rendering */

    struct {
        /* model type data */
        union {
            PLSkeletalModelData     skeletal_data;  /* skeletal animation data */
            //PLStaticModelData       static_data;    /* static model data */
            PLVertexAnimModelData   vertex_data;    /* per-vertex animation data */
        };
    } internal;
} PLModel;

PL_EXTERN_C

PLModel* plCreateStaticModel(PLModelLod *levels, uint8_t num_levels);
PLModel* plCreateBasicStaticModel(PLMesh *mesh);
PLModel* plCreateSkeletalModel(PLModelLod *levels, uint8_t num_levels, PLModelBone *skeleton, uint32_t num_bones,
                               uint32_t root_index);
PLModel* plCreateBasicSkeletalModel(PLMesh *mesh, PLModelBone *skeleton, uint32_t num_bones, uint32_t root_index);

PLModel* plLoadModel(const char *path);

void plDestroyModel(PLModel *model);

void plDrawModel(PLModel *model);
void plDrawModelBounds(const PLModel* model);
void plDrawModelSkeleton(PLModel *model);

void plGenerateModelNormals(PLModel *model, bool perFace);
void plGenerateModelBounds(PLModel *model);

enum {
	PL_MODEL_FILEFORMAT_ALL = 0,

	PL_BITFLAG( PL_MODEL_FILEFORMAT_CYCLONE, 0 ),
	PL_BITFLAG( PL_MODEL_FILEFORMAT_HDV, 1 ),
	PL_BITFLAG( PL_MODEL_FILEFORMAT_U3D, 2 ),
	PL_BITFLAG( PL_MODEL_FILEFORMAT_OBJ, 3 ),
};

void plRegisterModelLoader(const char *ext, PLModel*(*LoadFunction)(const char *path));
void plRegisterStandardModelLoaders( unsigned int flags );
void plClearModelLoaders(void);

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
