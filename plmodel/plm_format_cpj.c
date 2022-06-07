/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "plm_private.h"

typedef struct CPJSurface {
	float uv;
} CPJSurface;

typedef struct CPJVertex {
	uint8_t flags;
	PLVector3 position;
} CPJVertex;

typedef uint16_t CPJTriangle[ 3 ];

typedef struct CPJModel {
	unsigned int numVerts;
	CPJVertex *vertices;

	unsigned int numTriangles;
	CPJTriangle *triangles;

	unsigned int numEdges;

	unsigned int numSurfaces;
	CPJSurface *surfaces;
} CPJModel;

static void CPJModel_Free( CPJModel *model ) {
	PL_DELETE( model->vertices );
	PL_DELETE( model->triangles );
	PL_DELETE( model->surfaces );
}

typedef struct CPJChunkInfo {
	PLFileOffset offset;
	uint32_t length;
	uint32_t version;
	uint32_t timestamp;
	uint32_t nameOffset;
} CPJChunkInfo;

static bool SeekChunk( PLFile *file, unsigned int magic, PLFileOffset startOffset, CPJChunkInfo *out ) {
	PlFileSeek( file, startOffset, PL_SEEK_SET );

	while ( !PlIsEndOfFile( file ) ) {
		out->offset = PlGetFileOffset( file );
		unsigned int cm = PlReadInt32( file, false, NULL );
		if ( cm != magic ) {
			unsigned int length = PlReadInt32( file, false, NULL );
			if ( length == 0 || length >= PlGetFileSize( file ) ) {
				return false;
			}

			PLFileOffset offset = PlGetFileOffset( file ) + length;
			if ( ( offset % 2 ) != 0 ) {
				offset += 2 - ( offset % 2 );
			}

			PlFileSeek( file, offset, PL_SEEK_SET );
			continue;
		}

		out->length = PlReadInt32( file, false, NULL );
		out->version = PlReadInt32( file, false, NULL );
		out->timestamp = PlReadInt32( file, false, NULL );
		out->nameOffset = PlReadInt32( file, false, NULL );
		return true;
	}

	PlReportErrorF( PL_RESULT_FILEREAD, "failed to find specified chunk (%d)", magic );
	return false;
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

	unsigned int subMagic = PlReadInt32( file, false, NULL );
	PLFileOffset subOffset = PlGetFileOffset( file );

	CPJChunkInfo chunkInfo;
	PL_ZERO_( chunkInfo );
	CPJModel cpjModel;
	PL_ZERO_( cpjModel );

	/* first fetch the geometry description - there's only one of these it seems per model */
	if ( !SeekChunk( file, PL_MAGIC_TO_NUM( 'G', 'E', 'O', 'B' ), subOffset, &chunkInfo ) ) {
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

		uint32_t numMounts = PlReadInt32( file, false, NULL );
		uint32_t ofsMounts = PlReadInt32( file, false, NULL );

		uint32_t numObjectLinks = PlReadInt32( file, false, NULL );
		uint32_t ofsObjectLinks = PlReadInt32( file, false, NULL );

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
			PlReadInt16( file, false, NULL );                        /* first edge link */
			PlReadInt16( file, false, NULL );                        /* first tri link */

			cpjModel.vertices[ i ].position.x = ( float ) PlReadInt32( file, false, NULL );
			cpjModel.vertices[ i ].position.y = ( float ) PlReadInt32( file, false, NULL );
			cpjModel.vertices[ i ].position.z = ( float ) PlReadInt32( file, false, NULL );
		}
	}

	/* surface chunk - there are multiple of these, per texture */
	while ( SeekChunk( file, PL_MAGIC_TO_NUM( 'S', 'R', 'F', 'B' ), subOffset, &chunkInfo ) ) {
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
	if ( SeekChunk( file, PL_MAGIC_TO_NUM( 'S', 'K', 'L', 'B' ), subOffset, &chunkInfo ) ) {
		static const unsigned int version = 1;
		if ( chunkInfo.version != version ) {
			CPJModel_Free( &cpjModel );
			PlReportErrorF( PL_RESULT_FILEVERSION, "unexpected SKL version (%u != %u)", chunkInfo.version, version );
			return NULL;
		}

		uint32_t numBones = PlReadInt32( file, false, NULL );
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

		uint32_t numMounts = PlReadInt32( file, false, NULL );
		uint32_t ofsMounts = PlReadInt32( file, false, NULL );
	}

	PLMModel *model;

	return model;
}

PLMModel *PlmLoadCpjModel( const char *path ) {
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL )
		return NULL;

	PLMModel *model = ParseCPJModel( file );

	PlCloseFile( file );

	return model;
}
