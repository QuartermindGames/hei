// SPDX-License-Identifier: MIT
// Copyright © 2021-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_math.h>
#include <plcore/pl_physics.h>

#include <plgraphics/plg.h>
#include <plgraphics/plg_mesh.h>

typedef enum PLMModelType {
	PLM_MODELTYPE_STATIC,   /* static non-animated */
	PLM_MODELTYPE_VERTEX,   /* per-vertex animated */
	PLM_MODELTYPE_SKELETAL, /* skeletal/bones animated */

	PLM_NUM_MODELTYPES
} PLMModelType;

/**
 * Location per vertex.
 */
typedef struct PLMVertexAnimationTransform {
	unsigned int index;
	PLVector3 position;
	bool relative;
} PLMVertexAnimationTransform;

typedef struct PLMVertexAnimationFrame {
	PLMVertexAnimationTransform *transforms;
	unsigned int numTransforms;
} PLMVertexAnimationFrame;

typedef struct PLMVertexAnimation {
	char name[ 32 ];
	unsigned int startFrame;
	unsigned int endFrame;
	float speed;
} PLMVertexAnimation;

typedef struct PLMVertexAnimationModelData {
	PLMVertexAnimationFrame *frames;
	unsigned int numFrames;
	PLMVertexAnimation *animations;
	unsigned int numAnimations;
} PLMVertexAnimationModelData;

/* * * * * * * * * * * * * * * * * */
/* Skeletal Model Data */

#define PLM_MAX_BONE_WEIGHTS 4

typedef struct PLMBoneWeight {
	struct {
		unsigned int boneIndex;
		float factor;
	} subWeights[ PLM_MAX_BONE_WEIGHTS ];
	unsigned int numSubWeights;
} PLMBoneWeight;

typedef struct PLMModelBone {
	char name[ 64 ];
	unsigned int parent;
	PLVector3 position;
	PLQuaternion orientation;
} PLMBone;

typedef struct PLMSkeletalModelData {
	unsigned int rootIndex; /* root bone */

	PLMBone *bones;        /* list of bones */
	unsigned int numBones; /* number of bones in the array */

	unsigned int numBoneWeights; /* should be the same as the number of vertices */
	PLMBoneWeight *weights;
} PLMSkeletalModelData;

#define PLM_MODEL_MAX_MESHES 8

typedef struct PLMModel {
	char name[ 64 ];
	PLPath path;
	PLMModelType type;
	uint16_t flags;

	/* used for visibility culling */
	float radius;
	PLCollisionAABB bounds;

	/* transformations */
	PLMatrix4 modelMatrix;//TODO: kill this

	PLGMesh *meshes[ PLM_MODEL_MAX_MESHES ];
	unsigned int numMeshes;

	PLPath *materials;
	unsigned int numMaterials;

	void *userData; /* for storing material references etc. */

	struct {
		/* model type data */
		union {
			PLMSkeletalModelData skeletal_data;      /* skeletal animation data */
			PLMVertexAnimationModelData vertex_data; /* per-vertex animation data */
		};
	} internal;
} PLMModel;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PLMModel *PlmCreateStaticModel( PLGMesh **meshes, unsigned int numMeshes );
PLMModel *PlmCreateBasicStaticModel( PLGMesh *mesh );
PLMModel *PlmCreateSkeletalModel( PLGMesh **meshes, unsigned int numMeshes, PLMBone *bones, unsigned int numBones, PLMBoneWeight *weights, unsigned int numWeights );
PLMModel *PlmCreateBasicSkeletalModel( PLGMesh *mesh, PLMBone *bones, unsigned int numBones, PLMBoneWeight *weights, unsigned int numWeights );

PLMModel *PlmCreateVertexModel( PLGMesh **meshes, unsigned int numMeshes, unsigned int numFrames, unsigned int numAnimations );
PLMVertexAnimationFrame *PlmGetVertexAnimationFrames( const PLMModel *model, unsigned int *numFrames );

// Loads a model from the specific path - will automatically determine format from extension.
PLMModel *PlmLoadModel( const char *path );
// Attempts to load a model from the file handle. Will attempt to determine format.
PLMModel *PlmParseModel( PLFile *file );

PLMModel *PlmLoadU3DModel( const char *path );
PLMModel *PlmLoadHdvModel( const char *path );
PLMModel *PlmLoadRequiemModel( const char *path );
PLMModel *PlmLoadObjModel( const char *path );
PLMModel *PlmLoadCpjModel( const char *path );

PLMModel *PlmParseMDX( PLFile *file );
PLMModel *PlmParseGMA( PLFile *file );

void PlmDestroyModel( PLMModel *model );

void PlmDrawModel( PLMModel *model );
void PlmDrawModelBounds( const PLMModel *model );
void PlmDrawModelSkeleton( PLMModel *model );

void PlmGenerateModelNormals( PLMModel *model, bool perFace );
void PlmGenerateModelBounds( PLMModel *model );

#endif

enum {
	PLM_MODEL_FILEFORMAT_ALL = 0,

	PL_BITFLAG( PLM_MODEL_FILEFORMAT_CYCLONE, 0 ),
	PL_BITFLAG( PLM_MODEL_FILEFORMAT_HDV, 1 ),
	PL_BITFLAG( PLM_MODEL_FILEFORMAT_U3D, 2 ),
	PL_BITFLAG( PLM_MODEL_FILEFORMAT_OBJ, 3 ),
	PL_BITFLAG( PLM_MODEL_FILEFORMAT_CPJ, 4 ),
	PL_BITFLAG( PLM_MODEL_FILEFORMAT_MDX, 5 ),
	PL_BITFLAG( PLM_MODEL_FILEFORMAT_GMA, 6 ),
};

#if !defined( PL_COMPILE_PLUGIN )

void PlmRegisterModelLoader( const char *ext, PLMModel *( *LoadFunction )( const char *path ), PLMModel *( *ParseFunction )( PLFile *file ) );
void PlmRegisterStandardModelLoaders( unsigned int flags );
void PlmClearModelLoaders( void );

#endif

typedef enum PLModelOutputType {
	PLM_MODEL_OUTPUT_DEFAULT,
	PLM_MODEL_OUTPUT_SMD,
	PLM_MODEL_OUTPUT_OBJ,

	PLM_MAX_MODEL_OUTPUT_FORMATS
} PLMModelOutputType;

#if !defined( PL_COMPILE_PLUGIN )

bool PlmWriteModel( const char *path, PLMModel *model, PLMModelOutputType type );

#endif

PL_EXTERN_C_END
