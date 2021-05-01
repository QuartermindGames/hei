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

#include "plm_private.h"

/* PLATFORM MODEL LOADER */

typedef struct ModelLoader {
	const char *ext;
	PLMModel *( *LoadFunction )( const char *path );
} ModelLoader;

#define MAX_OBJECT_INTERFACES 512
static ModelLoader model_interfaces[ MAX_OBJECT_INTERFACES ];
static unsigned int num_model_loaders = 0;

int LOG_LEVEL_MODEL = -1;

void PlmInitialize( void ) {
	PlmClearModelLoaders();

	LOG_LEVEL_MODEL = PlAddLogLevel( "plmodel", PL_COLOUR_BLUE,
#if !defined( NDEBUG )
	                                 true
#else
	                                 false
#endif
	);
}

#define StaticModelData( a ) ( a )->internal.static_data
#define VertexModelData( a ) ( a )->internal.vertex_data
#define SkeletalModelData( a ) ( a )->internal.skeletal_data

void PlmGenerateModelNormals( PLMModel *model, bool perFace ) {
	for ( unsigned int j = 0; j < model->numMeshes; ++j ) {
		PlgGenerateMeshNormals( model->meshes[ j ], perFace );
	}
}

void PlmGenerateModelBounds( PLMModel *model ) {
	PLAABB bounds = {
	        PLVector3( 99999, 99999, 99999 ),   /* max */
	        PLVector3( -99999, -99999, -99999 ) /* min */
	};
	for ( unsigned int i = 0; i < model->numMeshes; ++i ) {
		PLGMesh *mesh = model->meshes[ i ];
		for ( unsigned int j = 0; j < mesh->num_verts; ++j ) {
			PLGVertex *vertex = &mesh->vertices[ j ];
			if ( vertex->position.x < bounds.mins.x ) bounds.mins.x = vertex->position.x;
			if ( vertex->position.x > bounds.maxs.x ) bounds.maxs.x = vertex->position.x;
			if ( vertex->position.y < bounds.mins.y ) bounds.mins.y = vertex->position.y;
			if ( vertex->position.y > bounds.maxs.y ) bounds.maxs.y = vertex->position.y;
			if ( vertex->position.z < bounds.mins.z ) bounds.mins.z = vertex->position.z;
			if ( vertex->position.z > bounds.maxs.z ) bounds.maxs.z = vertex->position.z;
		}
	}
	model->bounds = bounds;
}

bool PlmWriteModel( const char *path, PLMModel *model, PLMModelOutputType type ) {
	if ( plIsEmptyString( path ) ) {
		PlReportBasicError( PL_RESULT_FILEPATH );
		return false;
	}

	switch ( type ) {
		case PLM_MODEL_OUTPUT_SMD:
			return plWriteSmdModel( model, path );
		default:
			PlReportErrorF( PL_RESULT_UNSUPPORTED, "unsupported output type for %s (%u)", path, type );
			return false;
	}
}

void PlmRegisterModelLoader( const char *ext, PLMModel *( *LoadFunction )( const char *path ) ) {
	if ( num_model_loaders >= MAX_OBJECT_INTERFACES ) {
		PlReportBasicError( PL_RESULT_MEMORY_EOA );
		return;
	}

	model_interfaces[ num_model_loaders ].ext = ext;
	model_interfaces[ num_model_loaders ].LoadFunction = LoadFunction;
	num_model_loaders++;
}

void PlmRegisterStandardModelLoaders( unsigned int flags ) {
	typedef struct SModelLoader {
		unsigned int flag;
		const char *extension;
		PLMModel *( *LoadFunction )( const char *path );
	} SModelLoader;
	static const SModelLoader loaderList[] = {
	        { PLM_MODEL_FILEFORMAT_CYCLONE, "mdl", PlmLoadRequiemModel },
	        { PLM_MODEL_FILEFORMAT_HDV, "hdv", PlmLoadHdvModel },
	        { PLM_MODEL_FILEFORMAT_U3D, "3d", PlmLoadU3DModel },
	        { PLM_MODEL_FILEFORMAT_OBJ, "obj", PlmLoadObjModel },
	};

	for ( unsigned int i = 0; i < plArrayElements( loaderList ); ++i ) {
		if ( flags != PLM_MODEL_FILEFORMAT_ALL && !( flags & loaderList[ i ].flag ) ) {
			continue;
		}

		PlmRegisterModelLoader( loaderList[ i ].extension, loaderList[ i ].LoadFunction );
	}
}

void PlmClearModelLoaders( void ) {
	memset( model_interfaces, 0, sizeof( ModelLoader ) * MAX_OBJECT_INTERFACES );
	num_model_loaders = 0;
}

PLMModel *PlmLoadModel( const char *path ) {
	if ( !PlFileExists( path ) ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "failed to load model, %s", path );
		return NULL;
	}

	const char *extension = PlGetFileExtension( path );
	for ( unsigned int i = 0; i < num_model_loaders; ++i ) {
		if ( model_interfaces[ i ].LoadFunction == NULL ) {
			break;
		}

		if ( !plIsEmptyString( model_interfaces[ i ].ext ) ) {
			if ( pl_strncasecmp( extension, model_interfaces[ i ].ext, sizeof( model_interfaces[ i ].ext ) ) == 0 ) {
				PLMModel *model = model_interfaces[ i ].LoadFunction( path );
				if ( model != NULL ) {
					const char *name = PlGetFileName( path );
					if ( !plIsEmptyString( name ) ) {
						size_t nme_len = strlen( name );
						size_t ext_len = strlen( extension );
						strncpy( model->name, name, nme_len - ( ext_len + 1 ) );
					} else {
						snprintf( model->name, sizeof( model->name ), "null" );
					}

					strncpy( model->path, path, sizeof( model->path ) );
					return model;
				}
			}
		}
	}

	return NULL;
}

static PLMModel *CreateModel( PLMModelType type, PLGMesh **meshes, unsigned int numMeshes ) {
	PLMModel *model = pl_malloc( sizeof( PLMModel ) );
	if ( model == NULL ) {
		return NULL;
	}

	model->modelMatrix = PlMatrix4Identity();
	model->type = type;
	model->meshes = meshes;
	model->numMeshes = numMeshes;

	return model;
}

static PLMModel *CreateBasicModel( PLMModelType type, PLGMesh *mesh ) {
	PLGMesh **meshes = pl_malloc( sizeof( PLGMesh * ) );
	meshes[ 0 ] = mesh;

	PLMModel *model = CreateModel( type, meshes, 1 );
	if ( model == NULL ) {
		pl_free( meshes );
		return NULL;
	}

	return model;
}

PLMModel *PlmCreateBasicStaticModel( PLGMesh *mesh ) {
	return CreateBasicModel( PLM_MODELTYPE_STATIC, mesh );
}

PLMModel *PlmCreateStaticModel( PLGMesh **meshes, unsigned int numMeshes ) {
	return CreateModel( PLM_MODELTYPE_STATIC, meshes, numMeshes );
}

static PLMModel *CreateSkeletalModel( PLMModel *model, PLMModelBone *skeleton, uint32_t num_bones, uint32_t root_index ) {
	if ( model == NULL ) {
		return NULL;
	}

	if ( skeleton != NULL ) {
		model->internal.skeletal_data.bones = skeleton;
		model->internal.skeletal_data.num_bones = num_bones;
	}

	model->internal.skeletal_data.root_index = root_index;

	return model;
}

PLMModel *PlmCreateBasicSkeletalModel( PLGMesh *mesh, PLMModelBone *skeleton, uint32_t num_bones, uint32_t root_index ) {
	return CreateSkeletalModel( CreateBasicModel( PLM_MODELTYPE_SKELETAL, mesh ), skeleton, num_bones, root_index );
}

PLMModel *PlmCreateSkeletalModel( PLGMesh **meshes, unsigned int numMeshes, PLMModelBone *skeleton, uint32_t num_bones,
                                  uint32_t root_index ) {
	return CreateSkeletalModel( CreateModel( PLM_MODELTYPE_SKELETAL, meshes, numMeshes ), skeleton, num_bones, root_index );
}

void PlmDestroyModel( PLMModel *model ) {
	if ( model == NULL ) {
		return;
	}

	for ( unsigned int i = 0; i < model->numMeshes; ++i ) {
		if ( model->meshes[ i ] == NULL ) {
			continue;
		}

		PlgDestroyMesh( model->meshes[ i ] );
	}

	pl_free( model->meshes );

	if ( model->type == PLM_MODELTYPE_SKELETAL ) {
		pl_free( model->internal.skeletal_data.bones );
	}

	pl_free( model );
}

#if 0 /* todo: move */

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

#endif
