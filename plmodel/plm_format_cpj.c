/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "plm_private.h"

#define DEBUG_CPJ_LOADER
#if defined( DEBUG_CPJ_LOADER )
#	define dprint( ... ) printf( __VA_ARGS__ )
#else
#	define dprint( ... )
#endif

/* i know, i know, terrible c-isms */
#define CPJ_MAX_NAME_LENGTH  16
#define CPJ_MAX_BONE_WEIGHTS 8

#define CPJ_OFFSET_START 12

typedef struct CPJSurface {
	float uv;
} CPJSurface;

typedef struct CPJBone {
	char name[ CPJ_MAX_NAME_LENGTH ];
	unsigned int parent;
	PLVector3 scale;
	PLQuaternion rotation;
	PLVector3 transform;
	float length;
} CPJBone;

typedef struct CPJBoneWeight {
	unsigned int boneIndex;
	float weightFactor;
} CPJBoneWeight;

typedef struct CPJVertex {
	uint8_t flags;
	PLVector3 position;

	unsigned int numWeights;
	CPJBoneWeight weights[ CPJ_MAX_BONE_WEIGHTS ];
} CPJVertex;

typedef struct CPJTriangle {
	uint16_t x, y, z;
} CPJTriangle;

typedef struct CPJEdge {
	uint16_t x, y;
} CPJEdge;

typedef struct CPJModel {
	unsigned int numVerts;
	CPJVertex *vertices;

	unsigned int numTriangles;
	CPJTriangle *triangles;

	unsigned int numSurfaces;
	CPJSurface *surfaces;

	unsigned int numBones;
	CPJBone *bones;
} CPJModel;

static void CPJModel_Free( CPJModel *model ) {
	PL_DELETE( model->vertices );
	PL_DELETE( model->triangles );
	PL_DELETE( model->surfaces );
	PL_DELETE( model->bones );
}

typedef struct CPJChunkInfo {
	PLFileOffset offset;
	uint32_t magic;
	uint32_t length;
	uint32_t version;
	uint32_t timestamp;
	uint32_t nameOffset;
} CPJChunkInfo;

static PLFileOffset GetPaddedOffset( PLFileOffset offset ) {
	if ( ( offset % 2 ) != 0 ) {
		offset += 2 - ( offset % 2 );
	}
	return offset;
}

static PLFileOffset SeekChunk( PLFile *file, unsigned int magic, PLFileOffset startOffset, CPJChunkInfo *out ) {
	PlFileSeek( file, startOffset, PL_SEEK_SET );

	while ( !PlIsEndOfFile( file ) ) {
		out->offset = PlGetFileOffset( file );
		out->magic = PlReadInt32( file, false, NULL );
		out->length = PlReadInt32( file, false, NULL );

		PLFileOffset nextOffset = GetPaddedOffset( PlGetFileOffset( file ) + out->length );

		if ( out->magic != magic ) {
			if ( out->length == 0 || out->length >= PlGetFileSize( file ) ) {
				PlReportErrorF( PL_RESULT_FILEREAD, "invalid chunk length (%d - %u)", magic, out->length );
				return 0;
			}

			if ( !PlFileSeek( file, nextOffset, PL_SEEK_SET ) ) {
				return 0;
			}

			continue;
		}

		out->version = PlReadInt32( file, false, NULL );
		out->timestamp = PlReadInt32( file, false, NULL );
		out->nameOffset = PlReadInt32( file, false, NULL );
		return nextOffset;
	}

	PlReportErrorF( PL_RESULT_FILEREAD, "failed to find specified chunk (%d)", magic );
	return 0;
}

static const char *ReadName( PLFile *file, PLFileOffset offset, char *dst, size_t dstSize ) {
	/* fetch current position */
	PLFileOffset oldOffset = PlGetFileOffset( file );
	PlFileSeek( file, offset, PL_SEEK_SET );

	for ( unsigned int i = 0; i < dstSize - 1; ++i ) {
		bool status;
		char c = ( char ) PlReadInt8( file, &status );
		if ( c == '\0' || !status ) {
			break;
		}

		dst[ i ] = c;
	}

	/* restore old position */
	PlFileSeek( file, oldOffset, PL_SEEK_SET );

	return dst;
}

static PLMModel *ParseCPJModel( PLFile *file ) {
	unsigned int magic = PlReadInt32( file, false, NULL );
	if ( magic != PL_MAGIC_TO_NUM( 'R', 'I', 'F', 'F' ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected magic" );
		return NULL;
	}

	unsigned int fileSize = PlReadInt32( file, false, NULL );
	if ( fileSize != ( PlGetFileSize( file ) - PlGetFileOffset( file ) ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected header file length" );
		return NULL;
	}

	PlReadInt32( file, false, NULL ); /* sub magic */

	CPJChunkInfo chunkInfo;
	PL_ZERO_( chunkInfo );
	CPJModel cpjModel;
	PL_ZERO_( cpjModel );

	/* first fetch the geometry description - there's only one of these it seems per model */
	if ( SeekChunk( file, PL_MAGIC_TO_NUM( 'G', 'E', 'O', 'B' ), CPJ_OFFSET_START, &chunkInfo ) ) {
		static const unsigned int version = 1;
		if ( chunkInfo.version != version ) {
			CPJModel_Free( &cpjModel );
			PlReportErrorF( PL_RESULT_FILEVERSION, "unexpected GEO version (%u != %u)", chunkInfo.version, version );
			return NULL;
		}

		cpjModel.numVerts = PlReadInt32( file, false, NULL );
		uint32_t ofsVerts = PlReadInt32( file, false, NULL );

		uint32_t numEdges = PlReadInt32( file, false, NULL );
		uint32_t ofsEdges = PlReadInt32( file, false, NULL );

		cpjModel.numTriangles = PlReadInt32( file, false, NULL );
		uint32_t ofsTriangles = PlReadInt32( file, false, NULL );

		/* mounts */
		PlReadInt32( file, false, NULL );
		PlReadInt32( file, false, NULL );

		/* object links */
		PlReadInt32( file, false, NULL );
		PlReadInt32( file, false, NULL );

		PLFileOffset baseOffset = PlGetFileOffset( file );

		/* fetch the vertices */
		PlFileSeek( file, baseOffset + ofsVerts, PL_SEEK_SET );
		cpjModel.vertices = PL_NEW_( CPJVertex, cpjModel.numVerts );
		for ( unsigned int i = 0; i < cpjModel.numVerts; ++i ) {
			cpjModel.vertices[ i ].flags = PlReadInt8( file, NULL ); /* flags */
			PlReadInt8( file, NULL );                                /* group */
			PlReadInt16( file, false, NULL );                        /* unused */
			PlReadInt16( file, false, NULL );                        /* edge links */
			PlReadInt16( file, false, NULL );                        /* tri links */
			PlReadInt32( file, false, NULL );                        /* first edge link */
			PlReadInt32( file, false, NULL );                        /* first tri link */

			cpjModel.vertices[ i ].position.x = PlReadFloat32( file, false, NULL );
			cpjModel.vertices[ i ].position.y = PlReadFloat32( file, false, NULL );
			cpjModel.vertices[ i ].position.z = PlReadFloat32( file, false, NULL );
		}

		/* boy this format is weird... so next, rather than fetching the triangles, we need
		 * to fetch the edges as the triangles are edge-based... and then we can get the true
		 * vertices that each triangle is *actually* using */
		PlFileSeek( file, baseOffset + ofsEdges, PL_SEEK_SET );
		CPJEdge *edges = PL_NEW_( CPJEdge, numEdges );
		for ( unsigned int i = 0; i < numEdges; ++i ) {
			edges[ i ].x = ( uint16_t ) PlReadInt16( file, false, NULL );
			edges[ i ].y = ( uint16_t ) PlReadInt16( file, false, NULL );
			/* don't believe we need these */
			PlReadInt16( file, false, NULL );
			PlReadInt16( file, false, NULL );
			PlReadInt32( file, false, NULL );
		}

		/* and now fetch the triangles */
		PlFileSeek( file, baseOffset + ofsTriangles, PL_SEEK_SET );
		CPJTriangle *triangle = cpjModel.triangles = PL_NEW_( CPJTriangle, cpjModel.numTriangles );
		for ( unsigned int i = 0; i < cpjModel.numTriangles; ++i, ++triangle ) {
			triangle->x = edges[ ( uint16_t ) PlReadInt16( file, false, NULL ) ].y;
			triangle->y = edges[ ( uint16_t ) PlReadInt16( file, false, NULL ) ].y;
			triangle->z = edges[ ( uint16_t ) PlReadInt16( file, false, NULL ) ].y;
			PlReadInt16( file, false, NULL ); /* unused */
		}
	} else {
		CPJModel_Free( &cpjModel );
		return NULL;
	}

	/* surface chunk - there are multiple of these, per texture */
	PLFileOffset subOffset = CPJ_OFFSET_START;
	while ( ( subOffset = SeekChunk( file, PL_MAGIC_TO_NUM( 'S', 'R', 'F', 'B' ), subOffset, &chunkInfo ) ) ) {
		static const unsigned int version = 1;
		if ( chunkInfo.version != version ) {
			CPJModel_Free( &cpjModel );
			PlReportErrorF( PL_RESULT_FILEVERSION, "unexpected SRF version (%u != %u)", chunkInfo.version, version );
			return NULL;
		}

		cpjModel.numSurfaces++;

		uint32_t numTextures = PlReadInt32( file, false, NULL );
		uint32_t ofsTextures = PlReadInt32( file, false, NULL );

		/* according to the spec, this should be the same as the geo's number of tris */
		uint32_t numTriangles = PlReadInt32( file, false, NULL );
		if ( numTriangles != cpjModel.numTriangles ) {
			CPJModel_Free( &cpjModel );
			PlReportErrorF( PL_RESULT_FILEERR, "invalid number of triangles (%u != %u)", numTriangles, cpjModel.numTriangles );
			return NULL;
		}

		uint32_t ofsTriangles = PlReadInt32( file, false, NULL );

		uint32_t numUVCoords = PlReadInt32( file, false, NULL );
		uint32_t ofsUVCoords = PlReadInt32( file, false, NULL );

		PLFileOffset baseOffset = PlGetFileOffset( file );
	}

	/* skeleton chunk, not a problem if it doesn't exist as it likely just means it's static */
	if ( SeekChunk( file, PL_MAGIC_TO_NUM( 'S', 'K', 'L', 'B' ), CPJ_OFFSET_START, &chunkInfo ) ) {
		static const unsigned int version = 1;
		if ( chunkInfo.version != version ) {
			CPJModel_Free( &cpjModel );
			PlReportErrorF( PL_RESULT_FILEVERSION, "unexpected SKL version (%u != %u)", chunkInfo.version, version );
			return NULL;
		}

		cpjModel.numBones = PlReadInt32( file, false, NULL );
		uint32_t ofsBones = PlReadInt32( file, false, NULL );

		uint32_t numVerts = PlReadInt32( file, false, NULL );
		if ( numVerts != cpjModel.numVerts ) {
			CPJModel_Free( &cpjModel );
			PlReportErrorF( PL_RESULT_FILEERR, "invalid number of vertices in SKLB (%u != %u)", numVerts, cpjModel.numVerts );
			return NULL;
		}

		uint32_t ofsVerts = PlReadInt32( file, false, NULL );

		uint32_t numWeights = PlReadInt32( file, false, NULL );
		uint32_t ofsWeights = PlReadInt32( file, false, NULL );

		/* mounts */
		PlReadInt32( file, false, NULL );
		PlReadInt32( file, false, NULL );

		PLFileOffset baseOffset = PlGetFileOffset( file );

		/* and now fetch the booones */
		dprint( "bones...\n" );
		PlFileSeek( file, baseOffset + ofsBones, PL_SEEK_SET );
		cpjModel.bones = PL_NEW_( CPJBone, cpjModel.numBones );
		for ( unsigned int i = 0; i < cpjModel.numBones; ++i ) {
			uint32_t ofsName = PlReadInt32( file, false, NULL );
			ReadName( file, baseOffset + ofsName, cpjModel.bones[ i ].name, sizeof( cpjModel.bones[ i ].name ) );

			cpjModel.bones[ i ].parent = ( unsigned int ) PlReadInt32( file, false, NULL );

			cpjModel.bones[ i ].scale.x = PlReadFloat32( file, false, NULL );
			cpjModel.bones[ i ].scale.y = PlReadFloat32( file, false, NULL );
			cpjModel.bones[ i ].scale.z = PlReadFloat32( file, false, NULL );

			cpjModel.bones[ i ].rotation.x = PlReadFloat32( file, false, NULL );
			cpjModel.bones[ i ].rotation.y = PlReadFloat32( file, false, NULL );
			cpjModel.bones[ i ].rotation.z = PlReadFloat32( file, false, NULL );
			cpjModel.bones[ i ].rotation.w = PlReadFloat32( file, false, NULL );

			cpjModel.bones[ i ].transform.x = PlReadFloat32( file, false, NULL );
			cpjModel.bones[ i ].transform.y = PlReadFloat32( file, false, NULL );
			cpjModel.bones[ i ].transform.z = PlReadFloat32( file, false, NULL );

			cpjModel.bones[ i ].length = PlReadFloat32( file, false, NULL );
		}
	}

	PLGMesh *mesh = PlgCreateMesh( PLG_MESH_TRIANGLES, PLG_DRAW_STATIC, cpjModel.numTriangles, cpjModel.numVerts );

	for ( unsigned int i = 0; i < cpjModel.numVerts; ++i ) {
		PlgAddMeshVertex( mesh, cpjModel.vertices[ i ].position, pl_vecOrigin3, PL_COLOUR_WHITE, pl_vecOrigin2 );
	}

	CPJTriangle *triangle = cpjModel.triangles;
	for ( unsigned int i = 0; i < cpjModel.numTriangles; ++i, ++triangle ) {
		PlgAddMeshTriangle( mesh, triangle->x, triangle->y, triangle->z );
	}

	PLMSkeletalModelData skeletalModelData;
	PL_ZERO_( skeletalModelData );

	skeletalModelData.numBones = cpjModel.numBones;
	skeletalModelData.bones = PL_NEW_( PLMBone, skeletalModelData.numBones );
	for ( unsigned int i = 0; i < skeletalModelData.numBones; ++i ) {
		skeletalModelData.bones[ i ].orientation = cpjModel.bones[ i ].rotation;
		skeletalModelData.bones[ i ].position = cpjModel.bones[ i ].transform;
		skeletalModelData.bones[ i ].parent = cpjModel.bones[ i ].parent;
		snprintf( skeletalModelData.bones[ i ].name, sizeof( skeletalModelData.bones[ i ].name ), "%s", cpjModel.bones[ i ].name );
	}

	skeletalModelData.numBoneWeights = cpjModel.numVerts;
	skeletalModelData.weights = PL_NEW_( PLMBoneWeight, skeletalModelData.numBoneWeights );
	for ( unsigned int i = 0; i < skeletalModelData.numBoneWeights; ++i ) {
		skeletalModelData.weights[ i ].boneIndex = cpjModel.vertices[ i ].weights[ 0 ].boneIndex;
		skeletalModelData.weights[ i ].weightFactor = cpjModel.vertices[ i ].weights[ 0 ].weightFactor;
	}

	CPJModel_Free( &cpjModel );

	return PlmCreateBasicSkeletalModel( mesh, skeletalModelData.bones, skeletalModelData.numBones, skeletalModelData.weights, skeletalModelData.numBoneWeights );
}

PLMModel *PlmLoadCpjModel( const char *path ) {
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL )
		return NULL;

	PLMModel *model = ParseCPJModel( file );

	PlCloseFile( file );

	return model;
}
