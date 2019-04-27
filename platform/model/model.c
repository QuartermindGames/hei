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

typedef struct ModelLoader {
    const char *ext;
    PLModel*(*LoadFunction)(const char *path);
} ModelLoader;

static ModelLoader model_interfaces[MAX_OBJECT_INTERFACES];
static unsigned int num_model_loaders = 0;

void _plInitModelSubSystem(void) {
    plClearModelLoaders();
}

#define StaticModelData(a)      (a)->internal.static_data
#define VertexModelData(a)      (a)->internal.vertex_data
#define SkeletalModelData(a)    (a)->internal.skeletal_data

PLModelLod *plGetModelLodLevel(PLModel *model, unsigned int level) {
    if(level > model->num_levels || level >= PL_MAX_MODEL_LODS) {
        ReportError(PL_RESULT_FAIL, "invalid lod level");
        return NULL;
    }

    if      (model->type == PL_MODELTYPE_SKELETAL)  return &SkeletalModelData(model).levels[level];
    else if (model->type == PL_MODELTYPE_STATIC)    return &StaticModelData(model).levels[level];

    ReportError(PL_RESULT_FAIL, "unknown model type");
    return NULL;
}

///////////////////////////////////////

void plGenerateModelNormals(PLModel *model) {
    plAssert(model);

    PLModelLod *lod;
    for(unsigned int i = 0; (lod = plGetModelLodLevel(model, i)) != NULL; ++i) {
        for(unsigned int j = 0; j < lod->num_meshes; ++j) {
            plGenerateMeshNormals(&(lod->meshes[j]));
        }
    }
}

void plGenerateModelBounds(PLModel *model) {

}

//////////////////////////////////////////////////////////////////////////////

bool plWriteModel(const char *path, PLModel *model, PLModelOutputType type) {
    if(plIsEmptyString(path)) {
        ReportError(PL_RESULT_FILEPATH, plGetResultString(PL_RESULT_FILEPATH));
        return false;
    }

    switch(type) {
        case PL_MODEL_OUTPUT_SMD: return plWriteSMDModel(path, model);

        default:{
            ReportError(PL_RESULT_UNSUPPORTED, "unsupported output type for %s (%u)", path, type);
            return false;
        }
    }
}

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
    if(num_model_loaders == (unsigned int)(-1)) {
        for(unsigned int i = 0; i < plArrayElements(model_interfaces); ++i, ++num_model_loaders) {
            if(model_interfaces[i].ext == NULL && model_interfaces[i].LoadFunction == NULL) {
                num_model_loaders++;
                break;
            }
        }
    }

    model_interfaces[num_model_loaders].ext = ext;
    model_interfaces[num_model_loaders].LoadFunction = LoadFunction;
    num_model_loaders++;
}

void plRegisterStandardModelLoaders(void) {
    plRegisterModelLoader("hdv", plLoadHDVModel);
    plRegisterModelLoader("mdl", plLoadRequiemModel);
}

void plClearModelLoaders(void) {
    memset(model_interfaces, 0, sizeof(ModelLoader) * MAX_OBJECT_INTERFACES);
    num_model_loaders = 0;
}

PLModel *plLoadModel(const char *path) {
    if(!plFileExists(path)) {
        ReportError(PL_RESULT_FILEREAD, "failed to load model, %s", path);
        return NULL;
    }

    const char *extension = plGetFileExtension(path);
    for(unsigned int i = 0; i < num_model_loaders; ++i) {
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

#ifdef PL_USE_GRAPHICS

/* todo: move the following into sw pipeline */
void plApplyMeshLighting(PLMesh *mesh, const PLLight *light, PLVector3 position);
void plApplyModelLighting(PLModel *model, PLLight *light, PLVector3 position) {
    for(unsigned int i = 0; i < model->num_levels; ++i) {
        PLModelLod *lod = plGetModelLodLevel(model, i);
        if(lod == NULL) {
            continue;
        }

        for(unsigned int j = 0; j < lod->num_meshes; ++j) {
            plApplyMeshLighting(&lod->meshes[j], light, position);
        }
    }
}

#endif

PLModel *plCreateModel(PLModelType type, unsigned int num_levels, PLModelLod levels[]) {
    PLModel *model = pl_malloc(sizeof(PLModel));
    if(model == NULL) {
        return NULL;
    }

    model->model_matrix = plMatrix4x4Identity();
    model->num_levels = num_levels;
    model->type = type;

    if(num_levels > 0) {
        if (model->type == PL_MODELTYPE_SKELETAL) {
            memcpy(model->internal.skeletal_data.levels, levels, sizeof(PLModelLod) * num_levels);
        } else if (model->type == PL_MODELTYPE_STATIC) {
            memcpy(model->internal.static_data.levels, levels, sizeof(PLModelLod) * num_levels);
        } else if (model->type == PL_MODELTYPE_VERTEX) {
            /* todo */
        } else {
            plDestroyModel(model);
            ReportError(PL_RESULT_FAIL, "invalid model type");
            return NULL;
        }
    }

    return model;
}

void plDestroyModel(PLModel *model) {
    plAssert(model);

    for(unsigned int i = 0; i < model->num_levels; ++i) {
        PLModelLod *lod = plGetModelLodLevel(model, i);
        if(lod == NULL) {
            continue;
        }

        for(unsigned int j = 0; j < lod->num_meshes; ++j) {
            if(&lod->meshes[j] == NULL) {
                continue;
            }

            plDestroyMesh(&lod->meshes[j]);
        }

        free(lod->meshes);
    }

    free(model);
}

#if defined(PL_USE_GRAPHICS) /* todo: move */

#include "graphics/graphics_private.h"

void plDrawModel(PLModel *model) {
    plAssert(model);

    /* todo: currently only deals with static... */

    PLModelLod *lod = plGetModelLodLevel(model, model->internal.current_level);
    if(lod == NULL) {
        return;
    }

    for(unsigned int i = 0; i < lod->num_meshes; ++i) {
        plSetTexture(lod->meshes[i].texture, 0);

        plSetNamedShaderUniformMatrix4x4(NULL, "pl_model", model->model_matrix, true);

        plUploadMesh(&lod->meshes[i]);
        plDrawMesh(&lod->meshes[i]);
    }
}

void plDrawModelRadius(PLModel *model) {
    plAssert(model);
    ///plDrawCube(model->bounds)
}

void plDrawModelSkeleton(PLModel *model) {
    FunctionStart();

    if(model->type != PL_MODELTYPE_SKELETAL) {
        ReportError(PL_RESULT_FAIL, "invalid model type");
        return;
    }

    if(SkeletalModelData(model).num_bones == 0) {
        /* nothing to draw */
        return;
    }

    // todo

    static PLMesh *mesh = NULL;
    if(mesh == NULL) {

    }
}

#endif
