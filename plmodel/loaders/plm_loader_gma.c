// SPDX-License-Identifier: MIT
// Copyright © 2023 Mark E Sowden <hogsy@oldtimes-software.com>
// Loader for 3D Realms' GMA format

#include "../plm_private.h"

static const unsigned int GMA_MAJOR_VERSION = 1;

static uint32_t ParseGMAHeader( PLFile *file, uint16_t *numFrames, uint16_t *numTriangles, uint16_t *numVertices ) {
	// make sure it's a valid GMA file
	uint32_t magic = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( magic != PL_MAGIC_TO_NUM( 'A', 'G', 'M', 'A' ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic" );
		return 0;
	}

	// read in version info
	int8_t major = PlReadInt8( file, NULL );
	if ( major > GMA_MAJOR_VERSION ) {
		PlReportErrorF( PL_RESULT_FILEVERSION, "invalid version (%u > %u)", major, GMA_MAJOR_VERSION );
		return 0;
	}
	// minor version - eh...
	PlReadInt8( file, NULL );

	*numFrames = ( uint16_t ) PlReadInt16( file, false, NULL );
	if ( *numFrames == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid number of frames" );
		return 0;
	}
	*numTriangles = ( uint16_t ) PlReadInt16( file, false, NULL );
	if ( *numTriangles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid number of triangles" );
		return 0;
	}
	*numVertices = ( uint16_t ) PlReadInt16( file, false, NULL );
	if ( *numVertices == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid number of vertices" );
		return 0;
	}

	// and finally read in the offset here
	return ( uint32_t ) PlReadInt32( file, false, NULL );
}

PLMModel *PlmParseGMA( PLFile *file ) {
	uint16_t numFrames, numTriangles, numVertices;
	uint32_t vertexOffset = ParseGMAHeader( file, &numFrames, &numTriangles, &numVertices );
	if ( vertexOffset == 0 ) {
		return NULL;
	}

	PLGMesh *mesh = PlgCreateMesh( PLG_MESH_TRIANGLES, ( numFrames == 1 ) ? PLG_DRAW_STATIC : PLG_DRAW_DYNAMIC, numTriangles, numVertices );
	if ( mesh == NULL ) {
		return NULL;
	}

	// triangles immediately follow the header, so fetch those first
	for ( uint16_t i = 0; i < numTriangles; ++i ) {
		uint16_t x = ( uint16_t ) PlReadInt16( file, false, NULL );
		uint16_t y = ( uint16_t ) PlReadInt16( file, false, NULL );
		uint16_t z = ( uint16_t ) PlReadInt16( file, false, NULL );
		PlgAddMeshTriangle( mesh, x, y, z );
	}

	// fetch the vertices
	PlFileSeek( file, vertexOffset, PL_SEEK_SET );
	for ( uint16_t i = 0; i < numVertices; ++i ) {
		PLVector3 v;
		v.x = PlReadFloat32( file, false, NULL );
		v.y = PlReadFloat32( file, false, NULL );
		v.z = PlReadFloat32( file, false, NULL );
		PlgAddMeshVertex( mesh, v, pl_vecOrigin3, PL_COLOUR_WHITE, pl_vecOrigin2 );
	}

	PlgGenerateMeshNormals( mesh, true );
	PlgGenerateMeshTangentBasis( mesh );

#if 0
	// nice and easy!
	if ( numFrames == 1 ) {
		return PlmCreateBasicStaticModel( mesh );
	}

	// otherwise we're going to have to load in all the other frames (fortunately this is very easy)

	PLMModel *model = PlmCreateVertexModel( &mesh, 1, numFrames, 0 );
	for ( uint16_t i = 1; i < numFrames; ++i ) {
		PLMVertexAnimationFrame *frame = &model->internal.vertex_data.frames[ i ];
		frame->transforms = PL_NEW_( PLMVertexAnimationTransform, numVertices );
		for ( uint16_t j = 0; j < numVertices; ++j ) {
			PLVector3 v;
			v.x = PlReadFloat32( file, false, NULL );
			v.y = PlReadFloat32( file, false, NULL );
			v.z = PlReadFloat32( file, false, NULL );
			if ( PlCompareVector3( &mesh->vertices[ j ].position, &v ) ) {
				continue;
			}

			frame->transforms[ frame->numTransforms ].index = j;
			frame->transforms[ frame->numTransforms ].position = v;
			frame->numTransforms++;
		}
		frame->transforms = PL_REALLOCA( frame->transforms, sizeof( PLMVertexAnimationTransform ) * frame->numTransforms );
	}

	return model;
#else// not entirely sure the above code for frames is right??
	return PlmCreateBasicStaticModel( mesh );
#endif
}
