/*
DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
Version 2, December 2004

Copyright (C) 2011-2016 Mark E Sowden <markelswo@gmail.com>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

0. You just DO WHAT THE FUCK YOU WANT TO.
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

typedef struct PLModelFrame {
    PLTriangle  *triangles;
    PLVertex    *vertices;

    PLVector3D mins, maxs; // Bounds
} PLModelFrame;

/*	Static animated mesh.
*/
typedef struct PLStaticModel {
    unsigned int num_triangles;
    unsigned int num_vertices;

    PLPrimitive primitive;

    PLModelFrame frame;
} PLStaticModel;

/*	Per-vertex animated mesh.
*/
typedef struct PLAnimatedModel {
    unsigned int num_triangles;
    unsigned int num_vertices;
    unsigned int num_frames;

    PLPrimitive primitive;

    PLModelFrame *frames;
} PLAnimatedModel;

/*	Mesh with bone structure.
*/
typedef struct PLSkeletalModel {
    unsigned int num_triangles;
    unsigned int num_vertices;

    PLPrimitive primitive;

    // Unfinished...
} PLSkeletalModel;

#include "platform_model_u3d.h"
#include "platform_model_obj.h"

PL_EXTERN_C

// Static
PLStaticModel *plCreateStaticModel(void);
PLStaticModel *plLoadStaticModel(const char *path);
void plDeleteStaticModel(PLStaticModel *model);

// Animated
PLAnimatedModel *plCreateAnimatedModel(void);
PLAnimatedModel *plLoadAnimatedModel(const char *path);
void plDeleteAnimatedModel(PLAnimatedModel *model);

PLAnimatedModel *plLoadU3DModel(const char *path);

// Utility
void plGenerateStaticModelNormals(PLStaticModel *model);
void plGenerateAnimatedModelNormals(PLAnimatedModel *model);
void plGenerateSkeletalModelNormals(PLSkeletalModel *model);

PL_EXTERN_C_END
