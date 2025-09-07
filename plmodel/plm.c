// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "plm_private.h"

/* PLATFORM MODEL LOADER */

typedef struct ModelLoader {
	const char *ext;
	PLMModel *( *parseCallback )( PLFile *file );
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
	b1.absOrigin = PL_VECTOR3( ( b1.mins.x + b1.maxs.x ) / 2, ( b1.mins.y + b1.maxs.y ) / 2, ( b1.mins.z + b1.maxs.z ) / 2 );
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

void PlmRegisterModelLoader( const char *ext, PLMModel *( *Deserialize )( PLFile * ) ) {
	if ( num_model_loaders >= MAX_OBJECT_INTERFACES ) {
		PlReportBasicError( PL_RESULT_MEMORY_EOA );
		return;
	}

	model_interfaces[ num_model_loaders ].ext = ext;
	model_interfaces[ num_model_loaders ].parseCallback = Deserialize;
	num_model_loaders++;
}

void PlmRegisterStandardModelLoaders( unsigned int flags ) {
	typedef struct SModelLoader {
		unsigned int flag;
		const char *extension;
		PLMModel *( *parseCallback )( PLFile *file );
	} SModelLoader;
	static const SModelLoader loaderList[] = {
	        {PLM_MODEL_FILEFORMAT_CYCLONE, "mdl", PlmParseRequiemModel},
	        {PLM_MODEL_FILEFORMAT_HDV,     "hdv", PlmParseHdvModel    },
	        //{PLM_MODEL_FILEFORMAT_U3D,     "3d",  PlmParseU3dModel    },
	        //{PLM_MODEL_FILEFORMAT_OBJ,     "obj", PlmParseObjModel    },
	        {PLM_MODEL_FILEFORMAT_CPJ,     "cpj", PlmParseCpjModel    },
	        //{PLM_MODEL_FILEFORMAT_PLY,     "ply", PlmParsePlyModel    },
	};

	for ( unsigned int i = 0; i < PL_ARRAY_ELEMENTS( loaderList ); ++i ) {
		if ( flags != PLM_MODEL_FILEFORMAT_ALL && !( flags & loaderList[ i ].flag ) ) {
			continue;
		}

		PlmRegisterModelLoader( loaderList[ i ].extension, loaderList[ i ].parseCallback );
	}
}

void PlmClearModelLoaders( void ) {
	memset( model_interfaces, 0, sizeof( ModelLoader ) * MAX_OBJECT_INTERFACES );
	num_model_loaders = 0;
}

PLMModel *PlmLoadModel( const char *path ) {
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	const char *extension = PlGetFileExtension( path );
	const size_t ext_len = strlen( extension );
	for ( unsigned int i = 0; i < num_model_loaders; ++i ) {
		if ( pl_strcasecmp( extension, model_interfaces[ i ].ext ) != 0 ) {
			continue;
		}

		if ( model_interfaces[ i ].parseCallback == NULL ) {
			continue;
		}

		PLMModel *model = model_interfaces[ i ].parseCallback( file );
		if ( model == NULL ) {
			PlRewindFile( file );
			continue;
		}

		PlCloseFile( file );

		const char *name = PlGetFileName( path );
		if ( !PL_INVALID_STRING( name ) ) {
			size_t nme_len = strlen( name );
			strncpy( model->name, name, nme_len - ( ext_len + 1 ) );
		} else {
			*model->name = '\0';
		}

		strncpy( model->path, path, sizeof( model->path ) );

		return model;
	}

	PlCloseFile( file );
	return NULL;
}

static PLMModel *CreateModel( PLMModelType type, PLGMesh **meshes, unsigned int numMeshes ) {
	PLMModel *model = QM_OS_MEMORY_MALLOC_( sizeof( PLMModel ) );
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
	PLGMesh **meshes = QM_OS_MEMORY_MALLOC_( sizeof( PLGMesh * ) );
	meshes[ 0 ] = mesh;

	PLMModel *model = CreateModel( type, meshes, 1 );
	if ( model == NULL ) {
		qm_os_memory_free( meshes );
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

	model->internal.skeletal_data.vertices = QM_OS_MEMORY_NEW_( PLMSkeletalVertex *, model->numMeshes );
	for ( unsigned int i = 0; i < model->numMeshes; ++i ) {
		model->internal.skeletal_data.vertices[ i ] = QM_OS_MEMORY_NEW_( PLMSkeletalVertex, model->meshes[ i ]->num_verts );
	}

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

		if ( model->type == PLM_MODELTYPE_SKELETAL ) {
			qm_os_memory_free( model->internal.skeletal_data.vertices[ i ] );
		}

		PlgDestroyMesh( model->meshes[ i ] );
	}

	qm_os_memory_free( model->meshes );
	qm_os_memory_free( model->materials );

	if ( model->type == PLM_MODELTYPE_SKELETAL ) {
		qm_os_memory_free( model->internal.skeletal_data.bones );
		qm_os_memory_free( model->internal.skeletal_data.vertices );
	}

	qm_os_memory_free( model );
}
