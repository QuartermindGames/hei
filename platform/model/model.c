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
#include <PL/platform_mesh.h>

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

    return &model->levels[level];
}

///////////////////////////////////////

void plGenerateModelNormals(PLModel *model, bool perFace) {
    plAssert(model);

    PLModelLod *lod;
    for(unsigned int i = 0; (lod = plGetModelLodLevel(model, i)) != NULL; ++i) {
        for(unsigned int j = 0; j < lod->num_meshes; ++j) {
            plGenerateMeshNormals(lod->meshes[j], perFace);
        }
    }
}

void plGenerateModelBounds(PLModel* model) {
    plAssert(model != NULL);

    PLModelLod* lod = plGetModelLodLevel(model, 0);
    if(lod == NULL) {
        ReportError(PL_RESULT_FAIL, "failed to get lod level 0, unable to proceed with bound generation");
        return;
    }

    PLAABB bounds = {
            PLVector3(99999, 99999, 99999),     /* max */
            PLVector3(-99999, -99999, -99999)   /* min */
    };
    for(unsigned int i = 0; i < lod->num_meshes; ++i) {
        PLMesh* mesh = lod->meshes[i];
        for(unsigned int j = 0; j < mesh->num_verts; ++j) {
            PLVertex* vertex = &mesh->vertices[j];
            if(vertex->position.x < bounds.mins.x) bounds.mins.x = vertex->position.x;
            if(vertex->position.x > bounds.maxs.x) bounds.maxs.x = vertex->position.x;
            if(vertex->position.y < bounds.mins.y) bounds.mins.y = vertex->position.y;
            if(vertex->position.y > bounds.maxs.y) bounds.maxs.y = vertex->position.y;
            if(vertex->position.z < bounds.mins.z) bounds.mins.z = vertex->position.z;
            if(vertex->position.z > bounds.maxs.z) bounds.maxs.z = vertex->position.z;
        }
    }
    model->bounds = bounds;
}

//////////////////////////////////////////////////////////////////////////////

bool plWriteModel(const char *path, PLModel *model, PLModelOutputType type) {
    if(plIsEmptyString(path)) {
        ReportError(PL_RESULT_FILEPATH, plGetResultString(PL_RESULT_FILEPATH));
        return false;
    }

    switch(type) {
        case PL_MODEL_OUTPUT_SMD: return plWriteSmdModel(model, path);
        default:
            ReportError(PL_RESULT_UNSUPPORTED, "unsupported output type for %s (%u)", path, type);
            return false;
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
        for(size_t i = 0; i < plArrayElements(model_interfaces); ++i, ++num_model_loaders) {
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

void plRegisterStandardModelLoaders( unsigned int flags ) {
	typedef struct SModelLoader {
		unsigned int flag;
		const char *extension;
		PLModel *( *LoadFunction )( const char *path );
	} SModelLoader;

	static const SModelLoader loaderList[]={
			{ PL_MODEL_FILEFORMAT_CYCLONE, "mdl", plLoadRequiemModel },
			{ PL_MODEL_FILEFORMAT_HDV, "hdv", plLoadHDVModel },
			{ PL_MODEL_FILEFORMAT_U3D, "3d", plLoadU3DModel },
			{ PL_MODEL_FILEFORMAT_OBJ, "obj", plLoadObjModel },
	};

	for ( unsigned int i = 0; i < plArrayElements( loaderList ); ++i ) {
		if ( flags != PL_MODEL_FILEFORMAT_ALL && !( flags & loaderList[ i ].flag ) ) {
			continue;
		}

		plRegisterModelLoader( loaderList[ i ].extension, loaderList[ i ].LoadFunction );
	}
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

                    strncpy( model->path, path, sizeof( model->path ) );
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
            plApplyMeshLighting(lod->meshes[j], light, position);
        }
    }
}

#endif

static PLModel* CreateModel(PLModelType type, PLModelLod *levels, uint8_t num_levels) {
    PLModel* model = pl_malloc(sizeof(PLModel));
    if(model == NULL) {
        return NULL;
    }

    model->model_matrix = plMatrix4Identity();
    model->num_levels = num_levels;
    model->type = type;

    if(num_levels > 0 && levels != NULL) {
        memcpy(model->levels, levels, sizeof(PLModelLod) * num_levels);
    }

    return model;
}

static PLModel* CreateBasicModel(PLModelType type, PLMesh *mesh) {
    PLMesh** meshes = pl_malloc(sizeof(PLMesh*));
    if(meshes == NULL) return NULL;
    meshes[0] = mesh;
    PLModel* model = CreateModel(type, &(PLModelLod) {meshes, 1}, 1);
    if(model == NULL) {
        pl_free(meshes);
    }
    return model;
}

PLModel* plCreateBasicStaticModel(PLMesh *mesh) {
    return CreateBasicModel(PL_MODELTYPE_STATIC, mesh);
}

PLModel* plCreateStaticModel(PLModelLod *levels, uint8_t num_levels) {
    return CreateModel(PL_MODELTYPE_STATIC, levels, num_levels);
}

static PLModel* CreateSkeletalModel(PLModel *model, PLModelBone *skeleton, uint32_t num_bones, uint32_t root_index) {
    if(model == NULL) {
        return NULL;
    }

    if(skeleton != NULL) {
        model->internal.skeletal_data.bones = skeleton;
        model->internal.skeletal_data.num_bones = num_bones;
    }

    model->internal.skeletal_data.root_index = root_index;

    return model;
}

PLModel* plCreateBasicSkeletalModel(PLMesh *mesh, PLModelBone *skeleton, uint32_t num_bones, uint32_t root_index) {
    return CreateSkeletalModel(CreateBasicModel(PL_MODELTYPE_SKELETAL, mesh), skeleton, num_bones, root_index);
}

PLModel* plCreateSkeletalModel(PLModelLod *levels, uint8_t num_levels, PLModelBone *skeleton, uint32_t num_bones,
                               uint32_t root_index) {
    return CreateSkeletalModel(CreateModel(PL_MODELTYPE_SKELETAL, levels, num_levels), skeleton, num_bones, root_index);
}

void plDestroyModel(PLModel *model) {
    if(model == NULL) {
        return;
    }

    for(unsigned int i = 0; i < model->num_levels; ++i) {
        for(unsigned int j = 0; j < model->levels[i].num_meshes; ++j) {
            if(model->levels[i].meshes[j] == NULL) {
                continue;
            }

            plDestroyMesh(model->levels[i].meshes[j]);
        }

        pl_free(model->levels[i].meshes);
    }

    if(model->type == PL_MODELTYPE_SKELETAL) {
        pl_free(model->internal.skeletal_data.bones);
    }

    pl_free(model);
}

#if defined(PL_USE_GRAPHICS) /* todo: move */

#include "graphics/graphics_private.h"

void plDrawModel(PLModel *model) {
    plAssert(model);

    /* todo: currently only deals with static... */

    PLModelLod *lod = plGetModelLodLevel(model, model->current_level);
    if(lod == NULL) {
        return;
    }

    PLShaderProgram* old_program = plGetCurrentShaderProgram();
    for(unsigned int i = 0; i < lod->num_meshes; ++i) {
        if(lod->meshes[i]->shader_program != NULL) {
            plSetShaderProgram(lod->meshes[i]->shader_program);
        }

        plSetTexture(lod->meshes[i]->texture, 0);

        plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", &model->model_matrix, true );

        plUploadMesh(lod->meshes[i]);
        plDrawMesh(lod->meshes[i]);
    }

    plSetShaderProgram(old_program);
    plSetTexture(NULL, 0);
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
