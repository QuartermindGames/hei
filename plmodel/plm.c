// SPDX-License-Identifier: MIT
// Copyright © 2021-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "plm_private.h"

/* PLATFORM MODEL LOADER */

typedef struct ModelLoader {
	const char *ext;
	PLMModel *( *LoadFunction )( const char *path );
	PLMModel *( *ParseFunction )( PLFile *file );
} ModelLoader;

//TODO: switch this over to use a hash table by extension
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

/**
 * Attempts to write the model to the destination. If MODEL_OUTPUT_DEFAULT is
 * specified then it will choose the appropriate output format based on the
 * type of model.
 */
bool PlmWriteModel( const char *path, PLMModel *model, PLMModelOutputType type ) {
	if ( path == NULL || *path == '\0' ) {
		PlReportBasicError( PL_RESULT_FILEPATH );
		return false;
	}

	// if the default, automatically choose based on type
	if ( type == PLM_MODEL_OUTPUT_DEFAULT ) {
		type = ( model->type == PLM_MODELTYPE_SKELETAL ) ? PLM_MODEL_OUTPUT_SMD : PLM_MODEL_OUTPUT_OBJ;
	}

	PLPath tmp;
	if ( *PlGetFileExtension( path ) == '\0' ) {
		const char *extension = NULL;
		switch ( type ) {
			case PLM_MODEL_OUTPUT_OBJ:
				extension = ".obj";
				break;
			case PLM_MODEL_OUTPUT_SMD:
				extension = ".smd";
				break;
			default:
				break;
		}

		if ( extension != NULL ) {
			snprintf( tmp, sizeof( tmp ), "%s%s", path, extension );
		}
		path = tmp;
	}

	switch ( type ) {
		case PLM_MODEL_OUTPUT_SMD:
			return PlmWriteSmdModel( model, path );
		case PLM_MODEL_OUTPUT_OBJ:
			return PlmWriteObjModel( model, path );
		default:
			break;
	}

	return false;
}

void PlmRegisterModelLoader( const char *ext, PLMModel *( *LoadFunction )( const char *path ), PLMModel *( *ParseFunction )( PLFile *file ) ) {
	if ( num_model_loaders >= MAX_OBJECT_INTERFACES ) {
		PlReportBasicError( PL_RESULT_MEMORY_EOA );
		return;
	}

	model_interfaces[ num_model_loaders ].ext = ext;
	model_interfaces[ num_model_loaders ].LoadFunction = LoadFunction;
	model_interfaces[ num_model_loaders ].ParseFunction = ParseFunction;
	num_model_loaders++;
}

void PlmRegisterStandardModelLoaders( unsigned int flags ) {
	typedef struct SModelLoader {
		unsigned int flag;
		const char *extension;
		PLMModel *( *LoadFunction )( const char *path );
		PLMModel *( *ParseFunction )( PLFile *file );
	} SModelLoader;
	static const SModelLoader loaderList[] = {
	        {PLM_MODEL_FILEFORMAT_CYCLONE, "mdl", PlmLoadRequiemModel, NULL       },
	        { PLM_MODEL_FILEFORMAT_HDV,    "hdv", PlmLoadHdvModel,     NULL       },
	        { PLM_MODEL_FILEFORMAT_U3D,    "3d",  PlmLoadU3DModel,     NULL       },
	        { PLM_MODEL_FILEFORMAT_OBJ,    "obj", PlmLoadObjModel,     NULL       },
	        { PLM_MODEL_FILEFORMAT_CPJ,    "cpj", PlmLoadCpjModel,     NULL       },
	        { PLM_MODEL_FILEFORMAT_MDX,    "mdx", NULL,                PlmParseMDX},
	        { PLM_MODEL_FILEFORMAT_GMA,    "gma", NULL,                PlmParseGMA},
	};

	for ( unsigned int i = 0; i < PL_ARRAY_ELEMENTS( loaderList ); ++i ) {
		if ( flags != PLM_MODEL_FILEFORMAT_ALL && !( flags & loaderList[ i ].flag ) ) {
			continue;
		}

		PlmRegisterModelLoader( loaderList[ i ].extension, loaderList[ i ].LoadFunction, loaderList[ i ].ParseFunction );
	}
}

void PlmClearModelLoaders( void ) {
	memset( model_interfaces, 0, sizeof( ModelLoader ) * MAX_OBJECT_INTERFACES );
	num_model_loaders = 0;
}

PLMModel *PlmLoadModel( const char *path ) {
	// preferred route, we load in the file and hand it over to a parse callback -
	// this will be *faster* once we get rid of the LoadFunction method as we'll
	// then only need to fetch the file once!
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	const char *extension = PlGetFileExtension( path );
	size_t extensionLength = strlen( extension );

	PLMModel *model = NULL;
	for ( unsigned int i = 0; i < num_model_loaders; ++i ) {
		if ( model_interfaces[ i ].ParseFunction == NULL ) {
			continue;
		}

		if ( pl_strcasecmp( extension, model_interfaces[ i ].ext ) != 0 ) {
			continue;
		}

		model = model_interfaces[ i ].ParseFunction( file );
		if ( model != NULL ) {
			break;
		}

		PlRewindFile( file );
	}

	PlCloseFile( file );

	//TODO: switch list over to use map - try matched extension first, if that doesn't work, try others?
	//TODO: kill the below

	if ( model == NULL ) {
		for ( unsigned int i = 0; i < num_model_loaders; ++i ) {
			if ( model_interfaces[ i ].LoadFunction == NULL ) {
				continue;
			}

			if ( pl_strcasecmp( extension, model_interfaces[ i ].ext ) != 0 ) {
				continue;
			}

			model = model_interfaces[ i ].LoadFunction( path );
			if ( model != NULL ) {
				break;
			}
		}
	}

	// if we've got a valid model, we need to update the path and name
	if ( model != NULL ) {
		const char *name = PlGetFileName( path );
		if ( !PL_INVALID_STRING( name ) ) {
			size_t nme_len = strlen( name );
			strncpy( model->name, name, nme_len - ( extensionLength + 1 ) );
		} else {
			snprintf( model->name, sizeof( model->name ), "null" );
		}
		strncpy( model->path, path, sizeof( model->path ) );
	}

	return model;
}

PLMModel *PlmParseModel( PLFile *file ) {
	for ( unsigned int i = 0; i < num_model_loaders; ++i ) {
		if ( model_interfaces[ i ].ParseFunction == NULL ) {
			continue;
		}

		PLMModel *model = model_interfaces[ i ].ParseFunction( file );
		if ( model != NULL ) {
			return model;
		}

		PlRewindFile( file );
	}

	PlReportErrorF( PL_RESULT_FILEERR, "failed to parse model, last error: %s", PlGetError() );
	return NULL;
}

static PLMModel *CreateModel( PLMModelType type, PLGMesh **meshes, unsigned int numMeshes ) {
	if ( numMeshes >= PLM_MODEL_MAX_MESHES ) {
		PlReportErrorF( PL_RESULT_MEMORY_EOA, "invalid number of meshes (%u >= %u)", numMeshes, PLM_MODEL_MAX_MESHES );
		return NULL;
	}

	PLMModel *model = PlMAllocA( sizeof( PLMModel ) );
	if ( model == NULL ) {
		return NULL;
	}

	model->modelMatrix = PlMatrix4Identity();
	model->type = type;

	for ( unsigned int i = 0; i < numMeshes; ++i ) {
		model->meshes[ i ] = meshes[ i ];
	}
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

//////////////////////////////////////////////////////////////////
// Vertex animation

PLMModel *PlmCreateVertexModel( PLGMesh **meshes, unsigned int numMeshes, unsigned int numFrames, unsigned int numAnimations ) {
	PLMModel *model = CreateModel( PLM_MODELTYPE_VERTEX, meshes, numMeshes );
	if ( model == NULL ) {
		return NULL;
	}

	model->internal.vertex_data.numFrames = numFrames;
	model->internal.vertex_data.frames = PL_NEW_( PLMVertexAnimationFrame, model->internal.vertex_data.numFrames );
	model->internal.vertex_data.numAnimations = numAnimations;
	model->internal.vertex_data.animations = PL_NEW_( PLMVertexAnimation, model->internal.vertex_data.numAnimations );
	return model;
}

PLMVertexAnimationFrame *PlmGetVertexAnimationFrames( const PLMModel *model, unsigned int *numFrames ) {
	if ( model->type != PLM_MODELTYPE_VERTEX ) {
		return NULL;
	}
	*numFrames = model->internal.vertex_data.numFrames;
	return model->internal.vertex_data.frames;
}

//////////////////////////////////////////////////////////////////

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

	PlFree( model->materials );

	if ( model->type == PLM_MODELTYPE_SKELETAL ) {
		PlFree( model->internal.skeletal_data.bones );
	}

	PlFree( model );
}

void PlmDrawModel( PLMModel *model ) {
	plAssert( model );

	/* todo: currently only deals with static... */

	PLGShaderProgram *old_program = PlgGetCurrentShaderProgram();
	for ( unsigned int i = 0; i < model->numMeshes; ++i ) {
		if ( model->meshes[ i ]->shader_program != NULL ) {
			PlgSetShaderProgram( model->meshes[ i ]->shader_program );
		}

		PlgSetTexture( model->meshes[ i ]->texture, 0 );

		//TODO: use user provided matrix
		PlgSetShaderUniformValue( PlgGetCurrentShaderProgram(), "pl_model", &model->modelMatrix, true );

		PlgUploadMesh( model->meshes[ i ] );
		PlgDrawMesh( model->meshes[ i ] );
	}

	PlgSetShaderProgram( old_program );
	PlgSetTexture( NULL, 0 );
}
