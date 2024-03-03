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

enum {
	MDL_FLAG_FLAT = ( 1 << 0 ),
	MDL_FLAG_UNLIT = ( 1 << 1 ),
};

#define MAX_MODEL_NAME 128
#define MAX_TEXTURE_NAME 64

#define MAX_INDICES_PER_POLYGON 9
#define MIN_INDICES_PER_POLYGON 3

#define MAX_UV_COORDS_PER_FACE 36

#define MAX_VERTICES 4096
#define MAX_POLYGONS 8192

/* There seem to be two seperate model formats
 * used within the game, those that are static
 * and then a completely different type of header
 * for those that aren't.
 *
 * The texture names don't used fixed sizes in either,
 * in the static model the length of these is declared
 * but it doesn't appear this is the case in the animated
 * model format; the animated model format also seems
 * to support multiple textures.
 *
 * The first byte in the model format, is a flag which appears to
 * control how the model will be displayed within the game.
 *
 * Within medkit.mdl
 *  OFFSET ?? is X coord of vertex 1
 *  OFFSET 66 is Y coord of vertex 1
 *  OFFSET 6A is Z coord of vertex 1
 *  OFFSET 6E is X coord of vertex 2
 *  OFFSET 72 is Y coord of vertex 2
 *
 *  3 bytes between each vertex coord
 *  12 bytes for each coord set?
 *
 *  First 32bit int after texture name
 *  appears to be number of vertices
 *
 *  ?? ?? X  ?? ?? ?? Y  ?? ?? ?? Z  ??
 *  7E 71 41 3F 4C 1E 0D BC 9F 3C A8 3F
 *
 *  ????X X  ????Y Y  ????Z Z
 *  7E71413F 4C1E0DBC 9F3CA83F
 */

PL_PACKED_STRUCT_START( MDLVertex )
float x;
float y;
float z;
PL_PACKED_STRUCT_END( MDLVertex )

// 04:00:00:00:B4:BC:79:00:00:00:00:00:00:00:00:00:
// BC:BC:79:00:1C:00:1F:00:1B:00:19:00:57:D0:76:00:
// 66:42:1B:00:B0:21:78:00:70:E4:19:00:5C:35:6C:00:
// A1:D5:1C:00:9C:48:6A:00:36:DF:1E:00
// Quads are 60 bytes long; immediately follow vertices

// 03:00:00:00:14:BD:79:00:00:00:00:00:01:00:00:00:
// 1A:BD:79:00:05:00:01:00:06:00:E6:25:12:00:A1:DE:
// 22:00:92:CB:08:00:09:C3:17:00:B6:65:22:00:D8:E0:
// 09:00
// Triangles are 50 bytes long

typedef struct MDLPolygon {
	uint16_t indices[ MAX_INDICES_PER_POLYGON ];
	uint32_t num_indices;
	int16_t uv[ MAX_UV_COORDS_PER_FACE ];
} MDLPolygon;

/////////////////////////////////////////////////////////////////

/* rough notes for skeletal models in Requiem, which we don't yet support
 * but should do!
 *
 * Aaron.mdl
 * 44 bytes for header
 * this is then followed by the texture name, supposedly unused
 * texture name appears to be typically 11 bytes? followed by null terminator
 * this is then followed by a 16bit int and then another 16bit int?
 * 28 bytes of nothing ... ?
 *
 * fish.mdl
 * 44 bytes for header, again
 * followed by texture name
 * texture name _is_ 11 bytes
 * followed by 16bit integer
 * and another 16bit integer
 * 28 bytes of nothing ... ?
 *
 * 56 bytes for each block following header
 * perhaps these are descriptors for the skeleton?
 *
 * -----
 *
 * Even better!
 *
 * 0000:0000 > ?
 * 0000:0016 > uint8_t / length of texture string
 * 0000:001C > uint32_t / unknown, same for all?
 * 0000:0020 > int32_t / unknown, same for all?
 * 0000:002C > start of texture string, end of header!
 */

typedef struct MDLAniHeader { /* should be 44 bytes */
	uint32_t flag;            /* unconfirmed */
	int32_t u0;               /* appears to be unused? */
	uint32_t u1;              /* ... */
} MDLAniHeader;

static PLMModel *LoadAnimatedRequiemModel( PLFile *fp ) {
	PlRewindFile( fp );

	/* now read in the header */

	MDLAniHeader header;
	if ( PlReadFile( fp, &header, sizeof( MDLAniHeader ), 1 ) != 1 ) {
		return NULL;
	}

	/* todo: the rest */

	return NULL;
}

static PLMModel *LoadStaticRequiemModel( PLFile *fp ) {
	PlRewindFile( fp );

	// check which flags have been set for this particular mesh
	bool status;
	unsigned int flags = PlReadInt8( fp, &status );
	if ( !status ) {
		return NULL;
	}

	if ( flags & MDL_FLAG_FLAT ) {}                                    // flat
	if ( flags & MDL_FLAG_UNLIT ) {}                                   // unlit
	if ( !( flags & MDL_FLAG_FLAT ) && !( flags & MDL_FLAG_UNLIT ) ) {}// shaded

	uint32_t texture_name_length = 0;
	if ( PlReadFile( fp, &texture_name_length, sizeof( uint32_t ), 1 ) != 1 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid file length, failed to get texture name length" );
		return NULL;
	}

	if ( texture_name_length > MAX_TEXTURE_NAME || texture_name_length == 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid texture name length, %d", texture_name_length );
		return NULL;
	}

	char texture_name[ MAX_TEXTURE_NAME ];
	if ( PlReadFile( fp, texture_name, sizeof( char ), texture_name_length ) != texture_name_length ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid file length, failed to get texture name" );
		return NULL;
	}

	uint32_t num_vertices;
	if ( PlReadFile( fp, &num_vertices, sizeof( uint32_t ), 1 ) != 1 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid file length, failed to get number of vertices" );
		return NULL;
	}

	if ( num_vertices == 0 || num_vertices >= MAX_VERTICES ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid number of vertices, %d", num_vertices );
		return NULL;
	}

	uint32_t num_polygons;
	if ( PlReadFile( fp, &num_polygons, sizeof( uint32_t ), 1 ) != 1 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid file length, failed to get number of polygons" );
		return NULL;
	}

	if ( num_polygons == 0 || num_polygons >= MAX_POLYGONS ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid number of faces, %d", num_polygons );
		return NULL;
	}

	MDLVertex vertices[ MAX_VERTICES ];
	if ( PlReadFile( fp, vertices, sizeof( MDLVertex ), num_vertices ) != num_vertices ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid file length, failed to load vertices" );
		return NULL;
	}

	unsigned int num_triangles = 0;
	unsigned int num_indices = 0;
	MDLPolygon polygons[ MAX_POLYGONS ];
	memset( polygons, 0, sizeof( MDLPolygon ) * num_polygons );
	for ( unsigned int i = 0; i < num_polygons; ++i ) {
		/* 0000:00D0 |                           04 00 00 00  F4 9C 79 00 |         ....ô.y.
     * 0000:00E0 | 00 00 00 00  00 00 00 00  FC 9C 79 00  01 00 03 00 | ........ü.y.....
     * 0000:00F0 | 0B 00 09 00  93 65 72 00  00 80 00 00  00 80 7F 00 | .....er.........
     * 0000:0100 | 66 C0 0A 00  00 80 00 00  66 C0 0A 00  6C 9A 0D 00 | fÀ......fÀ..l...
     * 0000:0110 | 00 80 00 00                                        | ....
     *
     * Number of indices per face
     * 04 00 00 00
     *
     * Unsure...
     * F4 9C 79 00
     */

		//long pos = ftell(file);
		if ( PlReadFile( fp, &polygons[ i ].num_indices, sizeof( uint32_t ), 1 ) != 1 ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid file length, failed to load number of indices! (offset: %ld)", PlGetFileOffset( fp ) );
			return NULL;
		}

		if ( polygons[ i ].num_indices < MIN_INDICES_PER_POLYGON || polygons[ i ].num_indices > MAX_INDICES_PER_POLYGON ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid number of indices, %d, required for polygon %d! (offset: %ld)",
			             polygons[ i ].num_indices, i, PlGetFileOffset( fp ) );
			return NULL;
		}

		num_triangles += polygons[ i ].num_indices - 2;
		if ( polygons[ i ].num_indices == 4 ) {
			// conversion of quad to triangles
			num_indices += 6;
		} else {
			num_indices += polygons[ i ].num_indices * 100;
		}

		PlFileSeek( fp, 16, PL_SEEK_CUR );// todo, figure these out
		if ( PlReadFile( fp, polygons[ i ].indices, sizeof( uint16_t ), polygons[ i ].num_indices ) != polygons[ i ].num_indices ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid file length, failed to load indices" );
			return NULL;
		}

		unsigned int num_uv_coords = ( unsigned int ) ( polygons[ i ].num_indices * 4 );
		//ModelLog(" num bytes for UV coords is %lu\n", num_uv_coords * sizeof(int16_t));
		if ( PlReadFile( fp, polygons[ i ].uv, sizeof( int16_t ), num_uv_coords ) != num_uv_coords ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid file length, failed to load UV coords" );
			return NULL;
		}

		//long npos = ftell(file);
		//ModelLog(" Read %ld bytes for polygon %d (indices %d)\n", npos - pos, i, polygons[i].num_indices);
	}

	PLGMesh *mesh = PlgCreateMesh( PLG_MESH_TRIANGLES, PLG_DRAW_DYNAMIC, num_triangles, num_vertices );
	if ( mesh == NULL ) {
		return NULL;
	}

	for ( unsigned int i = 0; i < num_vertices; ++i ) {
		PlgSetMeshVertexPosition( mesh, i, &PLVector3( vertices[ i ].x, vertices[ i ].y, vertices[ i ].z ) );
		PlgSetMeshVertexColour( mesh, i, &PL_COLOURF32RGB( 1.0f, 1.0f, 1.0f ) );
	}

	unsigned int cur_index = 0;
	for ( unsigned int i = 0; i < num_polygons; ++i ) {
		if ( polygons[ i ].num_indices == 4 ) {// quad
			PL_ASSERT( ( cur_index + 6 ) <= mesh->num_indices );
			// first triangle
			mesh->indices[ cur_index++ ] = polygons[ i ].indices[ 0 ];
			mesh->indices[ cur_index++ ] = polygons[ i ].indices[ 1 ];
			mesh->indices[ cur_index++ ] = polygons[ i ].indices[ 2 ];
			// second triangle
			mesh->indices[ cur_index++ ] = polygons[ i ].indices[ 3 ];
			mesh->indices[ cur_index++ ] = polygons[ i ].indices[ 0 ];
			mesh->indices[ cur_index++ ] = polygons[ i ].indices[ 2 ];
		} else if ( polygons[ i ].num_indices == 3 ) {// triangle
			PL_ASSERT( ( cur_index + polygons[ i ].num_indices ) <= mesh->num_indices );
			for ( unsigned int j = 0; j < polygons[ i ].num_indices; ++j ) {
				mesh->indices[ cur_index++ ] = polygons[ i ].indices[ j ];
			}
		} else {
#if 0 /* converts triangle strip into triangles */
      for(unsigned int j = 0; j + 2 < polygons[i].num_indices; ++j) {
          mesh->indices[cur_index++] = polygons[i].indices[j];
          mesh->indices[cur_index++] = polygons[i].indices[j + 1];
          mesh->indices[cur_index++] = polygons[i].indices[j + 2];
      }
#else /* converts triangle fan into triangles */
			for ( unsigned int j = 1; j + 1 < polygons[ i ].num_indices; ++j ) {
				mesh->indices[ cur_index++ ] = polygons[ i ].indices[ 0 ];
				mesh->indices[ cur_index++ ] = polygons[ i ].indices[ j ];
				mesh->indices[ cur_index++ ] = polygons[ i ].indices[ j + 1 ];
			}
#endif
		}
	}

	PLMModel *model = PlmCreateBasicStaticModel( mesh );
	if ( model == NULL ) {
		PlgDestroyMesh( mesh );
		return NULL;
	}

	PlmGenerateModelNormals( model, false );
	PlmGenerateModelBounds( model );

	return model;
}

PLMModel *PlmLoadRequiemModel( const char *path ) {
	PLFile *fp = PlOpenFile( path, false );
	if ( fp == NULL ) {
		return NULL;
	}

	/* attempt to figure out what kind of model it is */

	bool status;
	int len = PlReadInt8( fp, &status );
	if ( !status ) {
		PlCloseFile( fp );
		return NULL;
	}

	PLMModel *model_ptr;
	if ( len == 0 ) {
		/* assume it's an animated model */
		ModelLog( "Texture name length of %d, assuming animated Requiem model...\n", len );
		model_ptr = LoadAnimatedRequiemModel( fp );
	} else {
		model_ptr = LoadStaticRequiemModel( fp );
	}

	PlCloseFile( fp );

	return model_ptr;
}
