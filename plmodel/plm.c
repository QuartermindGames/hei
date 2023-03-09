// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "plm_private.h"

/* PLATFORM MODEL LOADER */

typedef struct ModelLoader {
	const char *ext;
	PLMModel *( *LoadFunction )( const char *path );
	PLMModel *( *Deserialize )( PLFile *file );
	void *( *Serialize )( PLMModel *model );
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

#define StaticModelData( a )   ( a )->internal.static_data
#define VertexModelData( a )   ( a )->internal.vertex_data
#define SkeletalModelData( a ) ( a )->internal.skeletal_data

void PlmGenerateModelNormals( PLMModel *model, bool perFace ) {
	for ( unsigned int j = 0; j < model->numMeshes; ++j ) {
		PlgGenerateMeshNormals( model->meshes[ j ], perFace );
	}
}

void PlmGenerateModelBounds( PLMModel *model ) {
	PLCollisionAABB b1;
	memset( &b1, 0, sizeof( PLCollisionAABB ) );

	for ( unsigned int i = 0; i < model->numMeshes; ++i ) {
		PLCollisionAABB b2 = PlgGenerateAabbFromMesh( model->meshes[ i ], false );
		if ( b2.mins.x < b1.mins.x ) b1.mins.x = b2.mins.x;
		if ( b2.mins.y < b1.mins.y ) b1.mins.y = b2.mins.y;
		if ( b2.mins.z < b1.mins.z ) b1.mins.z = b2.mins.z;
		if ( b2.maxs.x > b1.maxs.x ) b1.maxs.x = b2.maxs.x;
		if ( b2.maxs.y > b1.maxs.y ) b1.maxs.y = b2.maxs.y;
		if ( b2.maxs.z > b1.maxs.z ) b1.maxs.z = b2.maxs.z;
	}

	/* abs origin is the middle of the bounding volume (wherever it is) and origin is the transformative point */
	b1.absOrigin = PLVector3( ( b1.mins.x + b1.maxs.x ) / 2, ( b1.mins.y + b1.maxs.y ) / 2, ( b1.mins.z + b1.maxs.z ) / 2 );
	b1.origin = pl_vecOrigin3;

	model->bounds = b1;
}

bool PlmWriteModel( const char *path, PLMModel *model, PLMModelOutputType type ) {
	if ( path == NULL || *path == '\0' ) {
		PlReportBasicError( PL_RESULT_FILEPATH );
		return false;
	}

	if ( type == PLM_MODEL_OUTPUT_DEFAULT ) {
		const char *ext = PlGetFileExtension( path );
		if ( ext == NULL ) {
			PlReportErrorF( PL_RESULT_FILEPATH, "no extension for file" );
			return false;
		}

		if ( strcmp( ext, "smd" ) == 0 ) {
			return PlmWriteSmdModel( model, path );
		} else if ( strcmp( ext, "obj" ) == 0 ) {
			return PlmWriteObjModel( model, path );
		}

		PlReportErrorF( PL_RESULT_UNSUPPORTED, "unsupported output type for %s (%u)", path, type );
		return false;
	}

	switch ( type ) {
		case PLM_MODEL_OUTPUT_SMD:
			return PlmWriteSmdModel( model, path );
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
		PLMModel *( *Deserialize )( PLFile *file );
	} SModelLoader;
	static const SModelLoader loaderList[] = {
	        {PLM_MODEL_FILEFORMAT_CYCLONE, "mdl", PlmLoadRequiemModel},
	        { PLM_MODEL_FILEFORMAT_HDV,    "hdv", PlmLoadHdvModel    },
	        { PLM_MODEL_FILEFORMAT_U3D,    "3d",  PlmLoadU3DModel    },
	        { PLM_MODEL_FILEFORMAT_OBJ,    "obj", PlmLoadObjModel    },
	        { PLM_MODEL_FILEFORMAT_CPJ,    "cpj", PlmLoadCpjModel    },
	};

	for ( unsigned int i = 0; i < PL_ARRAY_ELEMENTS( loaderList ); ++i ) {
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

	PLMModel *model = NULL;

	const char *extension = PlGetFileExtension( path );
	size_t ext_len = strlen( extension );
	for ( unsigned int i = 0; i < num_model_loaders; ++i ) {
		if ( pl_strcasecmp( extension, model_interfaces[ i ].ext ) != 0 ) {
			continue;
		}

		// first attempt to load it in using ye old method
		if ( model_interfaces[ i ].LoadFunction != NULL ) {
			model = model_interfaces[ i ].LoadFunction( path );
			if ( model != NULL ) {
				break;
			}
		}

		if ( model_interfaces[ i ].Deserialize != NULL ) {
			// eventually this should only get loaded the once, once LoadFunction is gone,
			// and then just rewound each time
			PLFile *file = PlOpenFile( path, false );
			if ( file == NULL ) {
				// break in this case, as if this fails it's likely every other attempt will
				break;
			}

			model = model_interfaces[ i ].Deserialize( file );

			// again, we'll eventually only need to do this once, once LoadFunction is out of the picture
			PlCloseFile( file );

			if ( model != NULL ) {
				break;
			}
		}
	}

	if ( model != NULL ) {
		const char *name = PlGetFileName( path );
		if ( !PL_INVALID_STRING( name ) ) {
			size_t nme_len = strlen( name );
			strncpy( model->name, name, nme_len - ( ext_len + 1 ) );
		} else {
			snprintf( model->name, sizeof( model->name ), "null" );
		}

		strncpy( model->path, path, sizeof( model->path ) );
	}

	return model;
}

static PLMModel *CreateModel( PLMModelType type, PLGMesh **meshes, unsigned int numMeshes ) {
	PLMModel *model = PlMAllocA( sizeof( PLMModel ) );
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
	PLGMesh **meshes = PlMAllocA( sizeof( PLGMesh * ) );
	meshes[ 0 ] = mesh;

	PLMModel *model = CreateModel( type, meshes, 1 );
	if ( model == NULL ) {
		PlFree( meshes );
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

static PLMModel *CreateSkeletalModel( PLMModel *model, PLMBone *bones, unsigned int numBones, PLMBoneWeight *weights, unsigned int numWeights ) {
	model->internal.skeletal_data.bones = bones;
	model->internal.skeletal_data.numBones = numBones;
	model->internal.skeletal_data.weights = weights;
	model->internal.skeletal_data.numBoneWeights = numWeights;

	model->internal.skeletal_data.rootIndex = 0;

	return model;
}

PLMModel *PlmCreateBasicSkeletalModel( PLGMesh *mesh, PLMBone *bones, unsigned int numBones, PLMBoneWeight *weights, unsigned int numWeights ) {
	return CreateSkeletalModel( CreateBasicModel( PLM_MODELTYPE_SKELETAL, mesh ), bones, numBones, weights, numWeights );
}

PLMModel *PlmCreateSkeletalModel( PLGMesh **meshes, unsigned int numMeshes, PLMBone *bones, unsigned int numBones, PLMBoneWeight *weights, unsigned int numWeights ) {
	return CreateSkeletalModel( CreateModel( PLM_MODELTYPE_SKELETAL, meshes, numMeshes ), bones, numBones, weights, numWeights );
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

	PlFree( model->meshes );
	PlFree( model->materials );

	if ( model->type == PLM_MODELTYPE_SKELETAL ) {
		PlFree( model->internal.skeletal_data.bones );
	}

	PlFree( model );
}

void PlmDrawModel( PLMModel *model ) {
	PL_ASSERT( model );

	/* todo: currently only deals with static... */

	PLGShaderProgram *old_program = PlgGetCurrentShaderProgram();
	for ( unsigned int i = 0; i < model->numMeshes; ++i ) {
		if ( model->meshes[ i ]->shader_program != NULL ) {
			PlgSetShaderProgram( model->meshes[ i ]->shader_program );
		}

		PlgSetTexture( model->meshes[ i ]->texture, 0 );

		PlgSetShaderUniformValue( PlgGetCurrentShaderProgram(), "pl_model", &model->modelMatrix, true );

		PlgUploadMesh( model->meshes[ i ] );
		PlgDrawMesh( model->meshes[ i ] );
	}

	PlgSetShaderProgram( old_program );
	PlgSetTexture( NULL, 0 );
}
