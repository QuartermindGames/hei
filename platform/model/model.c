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

#include "model_private.h"

#include <PL/platform_filesystem.h>

/*	PLATFORM MODEL LOADER	*/

typedef struct PLModelInterface {
    const char *extension;

    PLStaticModel*(*LoadStatic)(const char *path);
    PLAnimatedModel*(*LoadAnimated)(const char *path);
    PLSkeletalModel*(*LoadSkeletal)(const char *path);
} PLModelInterface;

PLModelInterface model_interfaces[]= {
        { "mdl", _plLoadStaticRequiemModel, NULL, /*_plLoadSkeletalRequiemModel*/NULL },
        //{ "mdl", _plLoadStaticSourceModel, NULL, _plLoadSkeletalSourceModel },
        //{ "mdl", _plLoadStaticGoldSrcModel, NULL, _plLoadSkeletalGoldSrcModel },
        //{ "smd", _plLoadStaticSMDModel, NULL, _plLoadSkeletalSMDModel },
        { "obj", _plLoadOBJModel, NULL, NULL },
};

///////////////////////////////////////

void plGenerateStaticModelNormals(PLStaticModel *model) {
    if(model == NULL) {
        return;
    }

    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        PLMesh *mesh = &model->meshes[i];
#if 1 // per face...
        for (unsigned int j = 0; j < model->num_triangles; j++) {
            mesh->triangles[j].normal = plGenerateVertexNormal(
                    mesh->vertices[mesh->triangles[j].indices[0]].position,
                    mesh->vertices[mesh->triangles[j].indices[1]].position,
                    mesh->vertices[mesh->triangles[j].indices[2]].position
            );
        }
#else // per vertex... todo
        for (PLVertex *vertex = &mesh->vertices[0]; vertex; ++vertex) {
            for (PLTriangle *triangle = &mesh->triangles[0]; triangle; ++triangle) {

            }
        }
#endif
    }
}

void plGenerateAnimatedModelNormals(PLAnimatedModel *model) {
    if (model == NULL) {
        return;
    }

    // Calculate normals for each frame... Can't we transform these? I guess that's
    // only feasible with skeletal animation formats where we can get the transform
    // but hell, if there's a way to abstractily grab the direction of a face then
    // surely we could figure that out.
#if 0 // todo
    for (PLModelFrame *frame = &model->frames[0]; frame; ++frame) {
        for (PLVertex *vertex = &frame->vertices[0]; vertex; ++vertex) {
            for (PLTriangle *triangle = &frame->triangles[0]; triangle; ++triangle) {

            }
        }
    }
#endif
}

void plGenerateSkeletalModelNormals(PLSkeletalModel *model) {
    if (model == NULL) {
        return;
    }

    // todo
}

/*	Static Model    */

PLStaticModel *plLoadStaticModel(const char *path) {
    if(!plFileExists(path)) {
        _plReportError(PL_RESULT_FILEREAD, "Failed to load model, %s!", path);
        return NULL;
    }

    const char *extension = plGetFileExtension(path);
    for(unsigned int i = 0; i < plArrayElements(model_interfaces); ++i) {
        if(model_interfaces[i].LoadStatic == NULL) {
            continue;
        }

        if(model_interfaces[i].extension != '\0') {
            if (!strcmp(extension, model_interfaces[i].extension)) {
                PLStaticModel *model = model_interfaces[i].LoadStatic(path);
                if(model != NULL) {
                    return model;
                }
            }
        }
    }

    return NULL;
}

void plDeleteStaticModel(PLStaticModel *model) {
    if (model == NULL) {
        return;
    }

    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        if(&model->meshes[i] == NULL) {
            continue;
        }

        plDeleteMesh(&model->meshes[i]);
    }

    free(model);
}
