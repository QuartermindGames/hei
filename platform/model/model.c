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
#include <PL/platform_model.h>

/* PLATFORM MODEL LOADER */

typedef struct ModelInterface {
    const char *ext;
    PLModel*(*LoadFunction)(const char *path);
} ModelInterface;

ModelInterface model_interfaces[512]= {
        { "hdv", LoadHDVModel },
        { "mdl", LoadRequiemModel },
        //{ "mdl", LoadSourceModel },
        //{ "mdl", LoadGoldSrcModel },
        //{ "3d", LoadU3DModel },
        //{ "smd", LoadSMDModel },
        //{ "obj", LoadOBJModel },

        { NULL, NULL }
};
unsigned int num_model_interfaces = (unsigned int)(-1);

///////////////////////////////////////

void plGenerateModelNormals(PLModel *model) {
    plAssert(model);

    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        plGenerateMeshNormals(model->meshes[i].mesh);
    }
}

void plGenerateModelAABB(PLModel *model) {
    plAssert(model);

    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        plAddAABB(&model->bounds, plCalculateMeshAABB(model->meshes[i].mesh));
    }
}

//////////////////////////////////////////////////////////////////////////////

uint8_t *plSerializeModel(PLModel *model, unsigned int type) {
    if(type >= PL_SERIALIZE_MODEL_END) {
        // todo, report error
        return NULL;
    }

    switch(type) {
        case PL_SERIALIZE_MODEL_COMPLETE: {

        } break;

        case PL_SERIALIZE_MODEL_BASE: {

        } break;

        case PL_SERIALIZE_MODEL_VERTICES: {

        } break;

        default: {
            // report error
            return NULL;
        }
    }

    // todo, stub
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////

void plRegisterModelLoader(const char *ext, PLModel*(*LoadFunction)(const char *path)) {
    if(num_model_interfaces == (unsigned int)(-1)) {
        for(unsigned int i = 0; i < plArrayElements(model_interfaces); ++i, ++num_model_interfaces) {
            if(model_interfaces[i].ext == NULL && model_interfaces[i].LoadFunction == NULL) {
                num_model_interfaces++;
                break;
            }
        }
    }

    model_interfaces[num_model_interfaces].ext = ext;
    model_interfaces[num_model_interfaces].LoadFunction = LoadFunction;
    num_model_interfaces++;
}

PLModel *plLoadModel(const char *path) {
    if(!plFileExists(path)) {
        ReportError(PL_RESULT_FILEREAD, "failed to load model, %s", path);
        return NULL;
    }

    const char *extension = plGetFileExtension(path);
    for(unsigned int i = 0; i < num_model_interfaces; ++i) {
        if(model_interfaces[i].LoadFunction == NULL) {
            break;
        }

        if(!plIsEmptyString(model_interfaces[i].ext)) {
            if (pl_strncasecmp(extension, model_interfaces[i].ext, sizeof(model_interfaces[i].ext)) == 0) {
                PLModel *model = model_interfaces[i].LoadFunction(path);
                if(model != NULL) {
                    const char *name = plGetFileName(path);
                    if(!plIsEmptyString(name)) {
                        size_t nme_len = strlen(name);
                        size_t ext_len = strlen(extension);
                        strncpy(model->name, name, nme_len - (ext_len + 1));
                    } else {
                        snprintf(model->name, sizeof(model->name), "null");
                    }
                    return model;
                }
            }
        }
    }

    return NULL;
}

void plApplyMeshLighting(PLMesh *mesh, const PLLight *light, PLVector3 position);
void plApplyModelLighting(PLModel *model, PLLight *light, PLVector3 position) {
    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        plApplyMeshLighting(model->meshes[i].mesh, light, position);
    }
}

void plDeleteModel(PLModel *model) {
    plAssert(model);

    for(unsigned int j = 0; j < model->num_meshes; ++j) {
        if(&model->meshes[j] == NULL) {
            continue;
        }

        free(model->meshes[j].bone_weights);

        plDeleteMesh(model->meshes[j].mesh);
    }

    free(model->meshes);
    free(model->bones);
    free(model);
}

void plDrawModel(PLModel *model) {
    plAssert(model);

    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        PLModelMesh *mesh = &model->meshes[i];
        plSetTexture(mesh->texture, 0);
        plDrawMesh(model->meshes[i].mesh);
    }
}

void plDrawModelBounds(PLModel *model) {
    plAssert(model);
    ///plDrawCube(model->bounds)
}

void plDrawModelSkeleton(PLModel *model) {
    plAssert(model);

    if(model->num_bones == 0) {
        return;
    }

    // todo

    static PLMesh *mesh = NULL;
    if(mesh == NULL) {

    }
}
