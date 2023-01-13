// SPDX-License-Identifier: MIT
// Copyright © 2023 Mark E Sowden <hogsy@oldtimes-software.com>
// Loader for 3D Realms' MDX format - per-vertex, not skeletal

#include "../plm_private.h"

//////////////////////////////////////////////////////////////////
// ASCF Handler (might come in use again)

typedef struct ASCFHeader {
	uint32_t magic;     // 'ASCF'
	uint32_t subMagic;  // denotes type
	uint16_t version;   // should be 3 (as of DNF98/99)
	uint16_t subVersion;// type version
	uint32_t totalSize; // total size of the file
	uint32_t dirOffset; // offset of chunks
	uint32_t dirNum;    // number of chunks
	uint32_t unused[ 2 ];
} ASCFHeader;
PL_STATIC_ASSERT( sizeof( ASCFHeader ) == 32, "" );

static ASCFHeader *ParseASCFHeader( PLFile *file, ASCFHeader *out ) {
	PL_ZERO_( *out );
	out->magic = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( out->magic != PL_MAGIC_TO_NUM( 'A', 'S', 'C', 'F' ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic" );
		return NULL;
	}
	out->subMagic = ( uint32_t ) PlReadInt32( file, false, NULL );

	static const uint16_t ASCF_VERSION = 3;
	out->version = ( uint16_t ) PlReadInt16( file, false, NULL );
	if ( out->version > ASCF_VERSION ) {
		PlReportErrorF( PL_RESULT_FILEVERSION, "unsupported version (%u > %u)", out->version, ASCF_VERSION );
		return NULL;
	}
	out->subVersion = ( uint16_t ) PlReadInt16( file, false, NULL );

	size_t fileSize = PlGetFileSize( file );
	out->totalSize = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( out->totalSize != fileSize ) {
		PlReportErrorF( PL_RESULT_FILESIZE, "size mismatch (%u != %u)", fileSize, out->totalSize );
		return NULL;
	}

	out->dirOffset = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( out->dirOffset >= fileSize ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid dir offset (%u >= %u)", out->dirOffset, fileSize );
		return NULL;
	}
	out->dirNum = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( out->dirNum == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid dir number (%u == 0)", out->dirNum );
		return NULL;
	}

	return out;
}

typedef struct ASCFChunkIndex {
	uint32_t magic;
	uint32_t offset;
	uint32_t length;
	uint8_t version;
	PL_UNUSED char unused[ 3 ];
	char description[ 32 ];
} ASCFChunkIndex;
PL_STATIC_ASSERT( sizeof( ASCFChunkIndex ) == 48, "" );

static ASCFChunkIndex *ParseASCFChunkIndex( PLFile *file, ASCFChunkIndex *out ) {
	PL_ZERO_( *out );

	bool status;
	out->magic = ( uint32_t ) PlReadInt32( file, false, &status );
	out->offset = ( uint32_t ) PlReadInt32( file, false, &status );
	out->length = ( uint32_t ) PlReadInt32( file, false, &status );
	out->version = ( uint8_t ) PlReadInt8( file, &status );
	if ( !status ) {
		return NULL;
	}

	// skip unused data
	if ( !PlFileSeek( file, 3, PL_SEEK_CUR ) ) {
		return NULL;
	}

	if ( PlReadFile( file, out->description, sizeof( char ), sizeof( out->description ) ) != sizeof( out->description ) ) {
		return NULL;
	}

	return out;
}

static ASCFChunkIndex *SeekChunk( PLFile *file, const ASCFHeader *header, unsigned int startIndex, unsigned int magic, const char *description, ASCFChunkIndex *out ) {
	if ( startIndex >= header->dirNum ) {
		return NULL;
	}

	for ( uint32_t i = startIndex; i < header->dirNum; ++i ) {
		PlFileSeek( file, ( PLFileOffset ) ( header->dirOffset + ( sizeof( ASCFChunkIndex ) * i ) ), PL_SEEK_SET );

		// parse in the chunk header and check whether it matches
		if ( ParseASCFChunkIndex( file, out ) == NULL ) {
			break;
		}
		if ( out->magic != magic ) {
			continue;
		}

		// if a description is provided, check if it matches
		if ( description != NULL && strcmp( out->description, description ) != 0 ) {
			continue;
		}

		// automatically seek to the data before we return
		PlFileSeek( file, out->offset, PL_SEEK_SET );

		return out;
	}

	return NULL;
}

/////////////////////////////////////////////////////////////////////
// Below is the actual MDX loader itself...

typedef char MDXSkinName[ 64 ];

#define MDX_MAX_GROUPS 16

typedef struct MDXFrameInfo {
	PLVector3 scales[ MDX_MAX_GROUPS ];
	PLVector3 translations[ MDX_MAX_GROUPS ];
	PLVector3 volumes[ 2 ][ MDX_MAX_GROUPS ];
} MDXFrameInfo;
static MDXFrameInfo *ParseMDXFrameInfo( PLFile *file, MDXFrameInfo *out ) {
	bool status;

	for ( uint8_t i = 0; i < MDX_MAX_GROUPS; ++i ) {
		out->scales[ i ].x = PlReadFloat32( file, false, &status );
		out->scales[ i ].y = PlReadFloat32( file, false, &status );
		out->scales[ i ].z = PlReadFloat32( file, false, &status );
	}
	if ( !status ) {
		return NULL;
	}

	for ( uint8_t i = 0; i < MDX_MAX_GROUPS; ++i ) {
		out->translations[ i ].x = PlReadFloat32( file, false, &status );
		out->translations[ i ].y = PlReadFloat32( file, false, &status );
		out->translations[ i ].z = PlReadFloat32( file, false, &status );
	}
	if ( !status ) {
		return NULL;
	}

	for ( uint8_t i = 0; i < 2; ++i ) {
		for ( uint8_t j = 0; j < MDX_MAX_GROUPS; ++j ) {
			out->volumes[ i ][ j ].x = PlReadFloat32( file, false, &status );
			out->volumes[ i ][ j ].y = PlReadFloat32( file, false, &status );
			out->volumes[ i ][ j ].z = PlReadFloat32( file, false, &status );
		}
	}
	if ( !status ) {
		return NULL;
	}

	return out;
}

static PLGMesh *ParseMDXReferenceChunk( PLFile *file, const ASCFChunkIndex *chunkIndex, PLMemoryGroup *memoryGroup ) {
	MDXFrameInfo frameInfo;
	if ( ParseMDXFrameInfo( file, &frameInfo ) == NULL ) {
		return NULL;
	}

	// fetch the number of verts and triangles, validate them

	uint32_t numVertices = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( numVertices == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid vertices" );
		return NULL;
	}

	uint32_t numTriangles = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( numTriangles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid triangles" );
		return NULL;
	}

	// load in the vertex data
	typedef struct MDXVertex {
		uint8_t group;
		uint8_t positions[ 3 ];
		uint8_t normals[ 3 ];
		uint8_t mount;
	} MDXVertex;
	MDXVertex *vertices = PL_GNEW_( memoryGroup, MDXVertex, numVertices );
	for ( uint32_t i = 0; i < numVertices; ++i ) {
		vertices[ i ].group = ( uint8_t ) PlReadInt8( file, NULL );
		for ( uint8_t j = 0; j < 3; ++j ) {
			vertices[ i ].positions[ j ] = ( uint8_t ) PlReadInt8( file, NULL );
		}
		for ( uint8_t j = 0; j < 3; ++j ) {
			vertices[ i ].normals[ j ] = ( uint8_t ) PlReadInt8( file, NULL );
		}
		vertices[ i ].mount = ( uint8_t ) PlReadInt8( file, NULL );
	}

	// triangle texture coords
	typedef struct MDXTriangleSTCoord {
		int16_t s;
		int16_t t;
	} MDXTriangleSTCoord;
	MDXTriangleSTCoord *stCoords = PL_GNEW_( memoryGroup, MDXTriangleSTCoord, numTriangles );
	for ( uint32_t i = 0; i < numTriangles; ++i ) {
		stCoords[ i ].s = PlReadInt16( file, false, NULL );
		stCoords[ i ].t = PlReadInt16( file, false, NULL );
	}

	uint8_t *skins = PL_GNEW_( memoryGroup, uint8_t, numTriangles );
	for ( uint32_t i = 0; i < numTriangles; ++i ) {
		skins[ i ] = ( uint8_t ) PlReadInt8( file, NULL );
	}

	return NULL;
}

static MDXSkinName *ParseMDXSkinChunk( PLFile *file, const ASCFChunkIndex *chunkIndex, PLMemoryGroup *memoryGroup, uint32_t *outSkins ) {
	PlFileSeek( file, chunkIndex->offset, PL_SEEK_SET );
	uint32_t numSkins = ( uint32_t ) PlReadInt32( file, false, NULL );
	MDXSkinName *skins = PL_GNEW_( memoryGroup, MDXSkinName, numSkins );
	for ( uint32_t i = 0; i < numSkins; ++i ) {
		PlReadInt32( file, false, NULL );// width
		PlReadInt32( file, false, NULL );// height
		PlReadInt32( file, false, NULL );// depth
		PlReadFile( file, skins[ i ], sizeof( char ), sizeof( MDXSkinName ) );
	}
	return skins;
}

PLMModel *ParseMDXChunks( PLFile *file, const ASCFHeader *header, PLMemoryGroup *memoryGroup ) {
	// ensure the required chunks are there first
	ASCFChunkIndex skinChunk;
	if ( SeekChunk( file, header, 0, PL_MAGIC_TO_NUM( 'S', 'K', 'I', 'N' ), NULL, &skinChunk ) == NULL ) {
		PlReportErrorF( PL_RESULT_FILEERR, "failed to fetch SKIN chunk: %s", PlGetError() );
		return NULL;
	}
	ASCFChunkIndex triangleChunk;
	if ( SeekChunk( file, header, 0, PL_MAGIC_TO_NUM( 'T', 'R', 'I', 'S' ), NULL, &triangleChunk ) == NULL ) {
		PlReportErrorF( PL_RESULT_FILEERR, "failed to fetch TRIS chunk: %s", PlGetError() );
		return NULL;
	}
	ASCFChunkIndex referenceChunk;
	if ( SeekChunk( file, header, 0, PL_MAGIC_TO_NUM( 'R', 'F', 'R', 'M' ), NULL, &referenceChunk ) == NULL ) {
		PlReportErrorF( PL_RESULT_FILEERR, "failed to fetch RFRM chunk: %s", PlGetError() );
		return NULL;
	}

	// read in the skins
	uint32_t numSkins;
	MDXSkinName *skins = ParseMDXSkinChunk( file, &skinChunk, memoryGroup, &numSkins );
	if ( skins == NULL ) {
		return NULL;
	}

	PLGMesh *mesh = PlgCreateMesh( PLG_MESH_TRIANGLES, PLG_DRAW_DYNAMIC, 0, 0 );

}

PLMModel *PlmParseMDX( PLFile *file ) {
	ASCFHeader header;
	if ( ParseASCFHeader( file, &header ) == NULL ) {
		return NULL;
	}

	// validate that the subtype is the correct type
	if ( header.subMagic != PL_MAGIC_TO_NUM( 'D', 'N', 'X', 'M' ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid sub magic" );
		return NULL;
	}
	static const unsigned int DNXM_VERSION = 5;
	if ( header.subVersion > 5 ) {
		PlReportErrorF( PL_RESULT_FILEVERSION, "unsupported DNXM version (%u > %u)", header.subVersion, DNXM_VERSION );
		return NULL;
	}

	PLMemoryGroup *memoryGroup = PlCreateMemoryGroup();
	PLMModel *model = ParseMDXChunks( file, &header, memoryGroup );
	PlDestroyMemoryGroup( memoryGroup );

	return model;
}
