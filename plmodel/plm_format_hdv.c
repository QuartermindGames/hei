/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "plm_private.h"

#define HDV_MAX_VERTICES 2048
#define HDV_MAX_FACES    2048

PL_PACKED_STRUCT_START( HDVHeader )
char identity[ 32 ]; /* includes start indicator before text string */

uint32_t face_offset;
uint32_t vert_offset;

uint32_t file_size[ 2 ]; /* provided twice, for some reason */

int32_t unknown[ 2 ];

uint16_t num_vertices;
uint16_t version;
uint16_t num_faces; /* -2, due to some left-over data */

/* the rest of this is unknown - skip to the face offsets once done here! */
PL_PACKED_STRUCT_END( HDVHeader )

PL_PACKED_STRUCT_START( HDVFace )
uint8_t u0[ 2 ];
uint8_t c_flag;

uint8_t u1[ 8 ];

uint8_t u2;
uint16_t vertex_offsets[ 4 ];

int8_t unknown1[ 16 ];
PL_PACKED_STRUCT_END( HDVFace )

PL_PACKED_STRUCT_START( HDVVertex )
int32_t x;
int32_t y;
int32_t z;
PL_PACKED_STRUCT_END( HDVVertex )

PLMModel *PlmParseHdvModel( PLFile *file ) {
	HDVHeader header;
	if ( PlReadFile( file, &header, sizeof( HDVHeader ), 1 ) != 1 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "failed to read in header" );
		return NULL;
	}

	if ( header.identity[ 0 ] != '\017' || strncmp( "TRITON Vec.Obj", header.identity + 1, 14 ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid HDV header" );
		return NULL;
	}

	const size_t size = PlGetFileSize( file );
	if ( header.file_size[ 0 ] != size ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid file size in HDV header" );
		return NULL;
	}

	if ( header.num_vertices >= HDV_MAX_VERTICES ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "model exceeds max vertex limit (%u >= %u)", header.num_vertices, HDV_MAX_VERTICES );
		return NULL;
	}
	if ( header.num_faces >= HDV_MAX_FACES ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "model exceeds max face limit (%u >= %u)", header.num_faces, HDV_MAX_FACES );
		return NULL;
	}

	if ( header.face_offset < sizeof( HDVHeader ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid face offset in HDV header" );
		return NULL;
	}

	if ( header.vert_offset < sizeof( HDVHeader ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid vertex offset in HDV header" );
		return NULL;
	}

	if ( !PlFileSeek( file, header.face_offset, PL_SEEK_SET ) ) {
		return NULL;
	}

	HDVFace faces[ HDV_MAX_FACES ];
	if ( PlReadFile( file, faces, sizeof( HDVFace ), header.num_faces ) != header.num_faces ) {
		return NULL;
	}

	if ( !PlFileSeek( file, header.vert_offset, PL_SEEK_SET ) ) {
		return NULL;
	}

	HDVVertex vertices[ HDV_MAX_VERTICES ];
	if ( PlReadFile( file, vertices, sizeof( HDVVertex ), header.num_vertices ) != header.num_vertices ) {
		return NULL;
	}

	PLGMesh *mesh = PlgCreateMesh( PLG_MESH_TRIANGLES, PLG_DRAW_DYNAMIC,
	                               ( header.num_faces - 2U ) * 2, header.num_vertices );
	if ( mesh == NULL ) {
		return NULL;
	}

#if 1 /* debug */
	srand( mesh->num_verts );
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		uint8_t r = ( uint8_t ) ( rand() % 255 );
		uint8_t g = ( uint8_t ) ( rand() % 255 );
		uint8_t b = ( uint8_t ) ( rand() % 255 );
		PlgSetMeshVertexPosition( mesh, i, &PL_VECTOR3( -vertices[ i ].x / 100, -vertices[ i ].y / 100, vertices[ i ].z / 100 ) );
		PlgSetMeshVertexColour( mesh, i, &PLColour( r, g, b, 255 ) );
	}
#endif

	unsigned int cur_index = 0;
	for ( unsigned int i = 0; i < ( header.num_faces - 2U ); ++i ) {
		//ModelLog(" num_verts %u\n", faces[i].u0[0]);

		// first triangle
		PlgSetMeshTrianglePosition( mesh, &cur_index,
		                            ( uint16_t ) ( faces[ i ].vertex_offsets[ 0 ] / 12 ),
		                            ( uint16_t ) ( faces[ i ].vertex_offsets[ 1 ] / 12 ),
		                            ( uint16_t ) ( faces[ i ].vertex_offsets[ 2 ] / 12 ) );

		if ( faces[ i ].u0[ 0 ] == 4 ) {
			// second triangle
			PlgSetMeshTrianglePosition( mesh, &cur_index,
			                            ( uint16_t ) ( faces[ i ].vertex_offsets[ 3 ] / 12 ),
			                            ( uint16_t ) ( faces[ i ].vertex_offsets[ 0 ] / 12 ),
			                            ( uint16_t ) ( faces[ i ].vertex_offsets[ 2 ] / 12 ) );
		}
	}

	PlgUploadMesh( mesh );

	PLMModel *model = PlmCreateBasicStaticModel( mesh );
	if ( model == NULL ) {
		PlgDestroyMesh( mesh );
		return NULL;
	}

	PlmGenerateModelNormals( model, false );
	PlmGenerateModelBounds( model );

	return model;
}
