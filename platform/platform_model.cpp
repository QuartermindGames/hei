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

#include "platform_model.h"

/*	PLATFORM MODEL LOADER	*/

PLVector3D plGenerateNormal(PLVector3D a, PLVector3D b, PLVector3D c) {
#if 1
    PLVector3D x = c - b;
    PLVector3D y = a - b;

    PLVector3D normal = x.CrossProduct(y);
    return normal.Normalize();
#else // legacy
    PLVector3f x, y;
    plVectorSubtract3fv(c, b, x);
    plVectorSubtract3fv(a, b, y);

    PLVector3f normal;
    plVectorClear(normal);
    plCrossProduct(x, y, normal);
    plVectorNormalize(normal);

    std::vector<float> vnorm = { normal[0], normal[1], normal[2] };
    return vnorm;
#endif
}

void plGenerateStaticModelNormals(PLStaticModel *model) {
    PLModelFrame *frame = &model->frame;
#if 0 // per face...
    for (int i = 0; i < model->num_triangles; i++)
    {
        std::vector<float> normal = plGenerateNormal(
            frame->vertices[frame->triangles[i].indices[0]].position,
            frame->vertices[frame->triangles[i].indices[1]].position,
            frame->vertices[frame->triangles[i].indices[2]].position);

        frame->triangles[i].normal[PL_X] = normal[PL_X];
        frame->triangles[i].normal[PL_Y] = normal[PL_Y];
        frame->triangles[i].normal[PL_Z] = normal[PL_Z];
    }
#else // per vertex...
    for (PLVertex *vertex = &frame->vertices[0]; vertex; ++vertex) {
        for (PLTriangle *triangle = &frame->triangles[0]; triangle; ++triangle) {

        }
    }
#endif
}

void plGenerateAnimatedModelNormals(PLAnimatedModel *model) {
    if (!model)
        return;

    // Calculate normals for each frame... Can't we transform these? I guess that's
    // only feasible with skeletal animation formats where we can get the transform
    // but hell, if there's a way to abstractily grab the direction of a face then
    // surely we could figure that out.
    for (PLModelFrame *frame = &model->frames[0]; frame; ++frame) {
        for (PLVertex *vertex = &frame->vertices[0]; vertex; ++vertex) {
            for (PLTriangle *triangle = &frame->triangles[0]; triangle; ++triangle) {

            }
        }
    }
}

void plGenerateSkeletalModelNormals(PLSkeletalModel *model) {
    if (!model)
        return;
}

/*
	Static Model
*/

PLStaticModel *plCreateStaticModel(void) {
    plSetErrorFunction("plCreateStaticModel");

    PLStaticModel *model = new PLStaticModel;
    memset(model, 0, sizeof(PLStaticModel));

    return model;
}

// Less direct implementation to load a model (less efficient too).
PLStaticModel *plLoadStaticModel(const char *path) {
    if (!path || path[0] == ' ')
        return nullptr;

    std::string extension(path);
    if (extension.find(".pskx") != std::string::npos)
        return plLoadOBJModel(path);
    else if (extension.find(".obj") != std::string::npos)
        return plLoadOBJModel(path);

    return nullptr;
}

void plDeleteStaticModel(PLStaticModel *model) {
    if (!model) {
        plSetError("Invalid model!\n");
        return;
    }

    if (model->frame.triangles)
        delete model->frame.triangles;
    if (model->frame.vertices)
        delete model->frame.vertices;

    delete model;
}

/*	
	Animated Model
*/

PLAnimatedModel *plCreateAnimatedModel(void) {
    plSetErrorFunction("plCreateAnimatedModel");

    PLAnimatedModel *model = new PLAnimatedModel;
    memset(model, 0, sizeof(PLAnimatedModel));

    return model;
}

// Less direct implementation to load a model (less efficient too).
PLAnimatedModel *plLoadAnimatedModel(const char *path) {
    plSetErrorFunction("plLoadAnimatedModel");

    if (!path || path[0] == ' ') {
        plSetError("Invalid path!\n");
        return nullptr;
    }

    return nullptr;
}

void plDeleteAnimatedModel(PLAnimatedModel *model) {
    plSetErrorFunction("plDeleteAnimatedModel");

    if (!model) {
        plSetError("Invalid model!\n");
        return;
    }

    for (unsigned int i = 0; i < model->num_frames; i++) {
        if (&model->frames[i]) {
            if (model->frames[i].triangles)
                delete model->frames[i].triangles;
            if (model->frames[i].vertices)
                delete model->frames[i].vertices;
            delete &model->frames[i];
        }
    }

    delete model;
}

/*
	UNREAL PSKX Static Model Format

	Model format introduced in Unreal Engine 2.0, sadly rather
	hard to dig up much information about it.
*/

#define PSKX_EXTENSION "pskx"

