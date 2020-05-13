/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include <PL/platform_console.h>
#include <PL/platform_mesh.h>

#include "../graphics/graphics_private.h"

/* todo: move into physics.c */
PLCollisionAABB plGenerateAABB( const PLVertex *vertices, unsigned int numVertices ) {
	PLCollisionAABB bounds;
	bounds.maxs = ( PLVector3 ) { -999999.0f, -999999.0f, -999999.0f };
	bounds.mins = ( PLVector3 ) { 999999.0f, 999999.0f, 999999.0f };

	for ( unsigned int i = 0; i < numVertices; ++i ) {
		if ( bounds.maxs.x < vertices[ i ].position.x ) { bounds.maxs.x = vertices[ i ].position.x; }
		if ( bounds.maxs.y < vertices[ i ].position.y ) { bounds.maxs.y = vertices[ i ].position.y; }
		if ( bounds.maxs.z < vertices[ i ].position.z ) { bounds.maxs.z = vertices[ i ].position.z; }
		if ( bounds.mins.x > vertices[ i ].position.x ) { bounds.mins.x = vertices[ i ].position.x; }
		if ( bounds.mins.y > vertices[ i ].position.y ) { bounds.mins.y = vertices[ i ].position.y; }
		if ( bounds.mins.z > vertices[ i ].position.z ) { bounds.mins.z = vertices[ i ].position.z; }
	}

	return bounds;
}

/**
 * Generate cubic coordinates for the given vertices.
 */
void plGenerateTextureCoordinates( PLVertex *vertices, unsigned int numVertices, PLVector2 textureOffset, PLVector2 textureScale ) {
	/* todo: figure out what projection face we're using */
	unsigned int l = 0, r = 2;
	PLVector3 projSums[ 2 ] = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
	for( unsigned int i = 0; i < numVertices; ++i ) {
		for ( unsigned int j = 0; j < 3; ++j ) {
			float v = plVector3Index( vertices[ i ].normal, j );
			float max = plVector3Index( projSums[ 0 ], j );
			if ( v > max ) { plVector3Index( projSums[ 0 ], j ) = v; }
			float min = plVector3Index( projSums[ 1 ], j );
			if ( v < min ) { plVector3Index( projSums[ 1 ], j ) = v; }
		}
	}

	/* now get the greater sum and dir */
	float sum = 0.0f;
	unsigned int dir = 0;
	for ( unsigned int j = 0; j < 3; ++j ) {
		float max = plVector3Index( projSums[ 0 ], j );
		float min = plVector3Index( projSums[ 1 ], j );
		if ( sum > max ) { plVector3Index( projSums[ 0 ], j ) = sum; dir = j; }
		else if ( sum < min ) { plVector3Index( projSums[ 1 ], j ) = sum; dir = j; }
	}

	PLCollisionAABB bounds = plGenerateAABB( vertices, numVertices );
	for ( unsigned int i = 0; i < numVertices; ++i ) {
		vertices[ i ].st[ 0 ].x = ( plVector3Index( vertices[ i ].position, l ) / plVector3Index( bounds.maxs, l ) + textureOffset.x ) * textureScale.x;
		vertices[ i ].st[ 0 ].y = ( plVector3Index( vertices[ i ].position, r ) / plVector3Index( bounds.maxs, r ) + textureOffset.y ) * textureScale.y;
	}
}

void plGenerateVertexNormals( PLVertex *vertices, unsigned int numVertices, unsigned int *indices, unsigned int numTriangles, bool perFace ) {
	if ( perFace ) {
		for ( unsigned int i = 0, idx = 0; i < numTriangles; ++i, idx += 3 ) {
			unsigned int a = indices[ idx ];
			unsigned int b = indices[ idx + 1 ];
			unsigned int c = indices[ idx + 2 ];

			PLVector3 normal = plGenerateVertexNormal(
					vertices[ a ].position,
					vertices[ b ].position,
					vertices[ c ].position
			);

			vertices[ a ].normal = plAddVector3( vertices[ a ].normal, normal );
			vertices[ b ].normal = plAddVector3( vertices[ b ].normal, normal );
			vertices[ c ].normal = plAddVector3( vertices[ c ].normal, normal );
		}

		return;
	}

	/* todo: normal generation per vertex */
}

PLVector3 plGenerateVertexNormal( PLVector3 a, PLVector3 b, PLVector3 c ) {
	PLVector3 x = PLVector3( c.x - b.x, c.y - b.y, c.z - b.z );
	PLVector3 y = PLVector3( a.x - b.x, a.y - b.y, a.z - b.z );
	return plNormalizeVector3( plVector3CrossProduct( x, y ));
}

void plGenerateMeshNormals( PLMesh *mesh, bool perFace ) {
	plAssert( mesh );

	plGenerateVertexNormals( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles, perFace );
}

/* software implementation of gouraud shading */
void plApplyMeshLighting( PLMesh *mesh, const PLLight *light, PLVector3 position ) {
	PLVector3 distvec = plSubtractVector3( position, light->position );
	float distance = (plByteToFloat( light->colour.a ) - plVector3Length( &distvec )) / 100.f;
	for ( unsigned int i = 0; i < mesh->num_verts; i++ ) {
		PLVector3 normal = mesh->vertices[ i ].normal;
		float angle = ( distance * (( normal.x * distvec.x ) + ( normal.y * distvec.y ) + ( normal.z * distvec.z )));
		if ( angle < 0 ) {
			plClearColour( &mesh->vertices[ i ].colour );
		} else {
			mesh->vertices[ i ].colour.r = light->colour.r * plFloatToByte( angle );
			mesh->vertices[ i ].colour.g = light->colour.g * plFloatToByte( angle );
			mesh->vertices[ i ].colour.b = light->colour.b * plFloatToByte( angle );
		}
		//GfxLog("light angle is %f\n", angle);
	}

#if 0
	/*
	x = Object->Vertices_normalStat[count].x;
	y = Object->Vertices_normalStat[count].y;
	z = Object->Vertices_normalStat[count].z;

	angle = (LightDist*((x * Object->Spotlight.x) + (y * Object->Spotlight.y) + (z * Object->Spotlight.z) ));
	if (angle<0 )
	{
	Object->Vertices_screen[count].r = 0;
	Object->Vertices_screen[count].b = 0;
	Object->Vertices_screen[count].g = 0;
	}
	else
	{
	Object->Vertices_screen[count].r = Object->Vertices_local[count].r * angle;
	Object->Vertices_screen[count].b = Object->Vertices_local[count].b * angle;
	Object->Vertices_screen[count].g = Object->Vertices_local[count].g * angle;
	}
	*/
#endif
}

PLMesh *plCreateMesh( PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts ) {
	return plCreateMeshInit( primitive, mode, num_tris, num_verts, NULL, NULL);
}

PLMesh *plCreateMeshInit( PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int numTriangles, unsigned int numVerts,
	const unsigned int *indicies, const PLVertex *verticies ) {
	plAssert( numVerts );

	PLMesh *mesh = ( PLMesh * ) pl_calloc( 1, sizeof( PLMesh ));
	if ( mesh == NULL) {
		return NULL;
	}

	mesh->primitive = primitive;
	mesh->mode = mode;

	mesh->internal.old_primitive = mesh->primitive;

	if ( numTriangles > 0 ) {
		mesh->num_triangles = numTriangles;
		if ( mesh->primitive == PL_MESH_TRIANGLES ) {
			mesh->maxIndices = mesh->num_indices = mesh->num_triangles * 3;
			if (( mesh->indices = pl_calloc( mesh->maxIndices, sizeof( unsigned int ))) == NULL) {
				plDestroyMesh( mesh );
				return NULL;
			}

			if ( indicies != NULL) {
				memcpy( mesh->indices, indicies, mesh->num_indices * sizeof( unsigned int ));
			}
		}
	}

	mesh->maxVertices = mesh->num_verts = numVerts;
	mesh->vertices = ( PLVertex * ) pl_calloc( mesh->maxVertices, sizeof( PLVertex ));
	if ( mesh->vertices == NULL) {
		plDestroyMesh( mesh );
		return NULL;
	}

	if ( verticies != NULL ) {
		memcpy( mesh->vertices, verticies, sizeof( PLVertex ) * mesh->num_verts );
	}

	CallGfxFunction( CreateMesh, mesh );

	return mesh;
}

void plDestroyMesh( PLMesh *mesh ) {
	if ( mesh == NULL) {
		return;
	}

	CallGfxFunction( DeleteMesh, mesh );

	pl_free( mesh->vertices );
	pl_free( mesh->indices );
	pl_free( mesh );
}

void plClearMesh( PLMesh *mesh ) {
	plClearMeshVertices( mesh );
	plClearMeshTriangles( mesh );
}

void plClearMeshVertices( PLMesh *mesh ) {
	mesh->num_verts = 0;
}

void plClearMeshTriangles( PLMesh *mesh ) {
	mesh->num_triangles = mesh->num_indices = 0;
}

void plScaleMesh( PLMesh *mesh, PLVector3 scale ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		mesh->vertices[ i ].position = plScaleVector3( mesh->vertices[ i ].position, scale );
	}
}

void plSetMeshTrianglePosition( PLMesh *mesh, unsigned int *index, unsigned int x, unsigned int y, unsigned int z ) {
	plAssert( *index < mesh->maxIndices );
	mesh->indices[ ( *index )++ ] = x;
	mesh->indices[ ( *index )++ ] = y;
	mesh->indices[ ( *index )++ ] = z;
}

void plSetMeshVertexPosition( PLMesh *mesh, unsigned int index, PLVector3 vector ) {
	plAssert( index < mesh->maxVertices );
	mesh->vertices[ index ].position = vector;
}

void plSetMeshVertexNormal( PLMesh *mesh, unsigned int index, PLVector3 vector ) {
	plAssert( index < mesh->maxVertices );
	mesh->vertices[ index ].normal = vector;
}

void plSetMeshVertexST( PLMesh *mesh, unsigned int index, float s, float t ) {
	plAssert( index < mesh->maxVertices );
	mesh->vertices[ index ].st[ 0 ] = PLVector2( s, t );
}

void plSetMeshVertexSTv( PLMesh *mesh, uint8_t unit, unsigned int index, unsigned int size, const float *st ) {
	size += index;
	if ( size > mesh->num_verts ) {
		size -= ( size - mesh->num_verts );
	}

	for ( unsigned int i = index; i < size; i++ ) {
		mesh->vertices[ i ].st[ unit ].x = st[ 0 ];
		mesh->vertices[ i ].st[ unit ].y = st[ 1 ];
	}
}

void plSetMeshVertexColour( PLMesh *mesh, unsigned int index, PLColour colour ) {
	plAssert( index < mesh->maxVertices );
	mesh->vertices[ index ].colour = colour;
}

void plSetMeshUniformColour( PLMesh *mesh, PLColour colour ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		mesh->vertices[ i ].colour = colour;
	}
}

void plSetMeshShaderProgram( PLMesh *mesh, PLShaderProgram *program ) {
	mesh->shader_program = program;
}

unsigned int plAddMeshVertex( PLMesh *mesh, PLVector3 position, PLVector3 normal, PLColour colour, PLVector2 st ) {
	unsigned int vertexIndex = mesh->num_verts++;
	if ( vertexIndex >= mesh->maxVertices ) {
		mesh->vertices = pl_realloc( mesh->vertices, ( mesh->maxVertices += 16 ) * sizeof( PLVertex ) );
	}

	plSetMeshVertexPosition( mesh, vertexIndex, position );
	plSetMeshVertexNormal( mesh, vertexIndex, normal );
	plSetMeshVertexColour( mesh, vertexIndex, colour );
	plSetMeshVertexST( mesh, vertexIndex, st.x, st.y );

	return vertexIndex;
}

unsigned int plAddMeshTriangle( PLMesh *mesh, unsigned int x, unsigned int y, unsigned int z  ) {
	unsigned int triangleIndex = mesh->num_indices;

	mesh->num_indices += 3;
	if ( mesh->num_indices >= mesh->maxIndices ) {
		mesh->indices = pl_realloc( mesh->indices, ( mesh->maxIndices += 16 ) * sizeof( unsigned int ) );
	}

	mesh->indices[ triangleIndex ]      = x;
	mesh->indices[ triangleIndex + 1 ]  = y;
	mesh->indices[ triangleIndex + 2 ]  = z;

	mesh->num_triangles++;

	return triangleIndex;
}

void plUploadMesh( PLMesh *mesh ) {
	CallGfxFunction( UploadMesh, mesh );
}

void plDrawMesh( PLMesh *mesh ) {
	CallGfxFunction( DrawMesh, mesh );
}

/* primitive types */

#if defined(PL_USE_GRAPHICS)    /* todo: move these... */

void plDrawRaisedBox( int x, int y, unsigned int w, unsigned int h ) {
	static PLMesh *mesh = NULL;
	if ( mesh == NULL) {
		if (( mesh = plCreateMesh(
				PL_MESH_LINES,
				PL_DRAW_DYNAMIC,
				0, 4
		)) == NULL) {
			return;
		}
	}
}

void plDrawBevelledBorder( int x, int y, unsigned int w, unsigned int h ) {
	static PLMesh *mesh = NULL;
	if ( mesh == NULL) {
		if (( mesh = plCreateMesh(
				PL_MESH_LINES,
				PL_DRAW_DYNAMIC,
				0, 16
		)) == NULL) {
			return;
		}
	}

	plSetMeshVertexPosition( mesh, 0, PLVector3( x, y, 0 ) );
	plSetMeshVertexPosition( mesh, 1, PLVector3( x + w, y, 0 ) );
	plSetMeshVertexPosition( mesh, 2, PLVector3( x, y, 0 ) );
	plSetMeshVertexPosition( mesh, 3, PLVector3( x, y + h, 0 ) );

	plSetMeshVertexColour( mesh, 0, PLColourRGB( 127, 127, 127 ) );
	plSetMeshVertexColour( mesh, 1, PLColourRGB( 127, 127, 127 ) );
	plSetMeshVertexColour( mesh, 2, PLColourRGB( 127, 127, 127 ) );
	plSetMeshVertexColour( mesh, 3, PLColourRGB( 127, 127, 127 ) );

	/************************/

	plSetMeshVertexPosition( mesh, 4, PLVector3( x, y + h, 0 ) );
	plSetMeshVertexPosition( mesh, 5, PLVector3( x + w, y + h, 0 ) );
	plSetMeshVertexPosition( mesh, 6, PLVector3( x + w, y + h, 0 ) );
	plSetMeshVertexPosition( mesh, 7, PLVector3( x + w, y, 0 ) );

	plSetMeshVertexColour( mesh, 4, PLColourRGB( 255, 255, 255 ) );
	plSetMeshVertexColour( mesh, 5, PLColourRGB( 255, 255, 255 ) );
	plSetMeshVertexColour( mesh, 6, PLColourRGB( 255, 255, 255 ) );
	plSetMeshVertexColour( mesh, 7, PLColourRGB( 255, 255, 255 ) );

	/************************/

	plSetMeshVertexPosition( mesh, 8, PLVector3( x + 1, y + 1, 0 ) );
	plSetMeshVertexPosition( mesh, 9, PLVector3( x + w - 1, y + 1, 0 ) );
	plSetMeshVertexPosition( mesh, 10, PLVector3( x + 1, y + 1, 0 ) );
	plSetMeshVertexPosition( mesh, 11, PLVector3( x + 1, y + h - 1, 0 ) );

	plSetMeshVertexColour( mesh, 8, PLColourRGB( 63, 63, 63 ) );
	plSetMeshVertexColour( mesh, 9, PLColourRGB( 63, 63, 63 ) );
	plSetMeshVertexColour( mesh, 10, PLColourRGB( 63, 63, 63 ) );
	plSetMeshVertexColour( mesh, 11, PLColourRGB( 63, 63, 63 ) );

	/************************/

	plSetMeshVertexPosition( mesh, 12, PLVector3( x + 1, y + h - 1, 0 ) );
	plSetMeshVertexPosition( mesh, 13, PLVector3( x + w - 1, y + h - 1, 0 ) );
	plSetMeshVertexPosition( mesh, 14, PLVector3( x + w - 1, y + h - 1, 0 ) );
	plSetMeshVertexPosition( mesh, 15, PLVector3( x + w - 1, y + 1, 0 ) );

	plSetMeshVertexColour( mesh, 12, PLColourRGB( 63, 63, 63 ) );
	plSetMeshVertexColour( mesh, 13, PLColourRGB( 63, 63, 63 ) );
	plSetMeshVertexColour( mesh, 14, PLColourRGB( 63, 63, 63 ) );
	plSetMeshVertexColour( mesh, 15, PLColourRGB( 63, 63, 63 ) );

	plSetNamedShaderUniformMatrix4( NULL, "pl_model", plMatrix4Identity(), false );
	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

void plDrawCube( PLVector3 position, float size ) {
	/* todo */
}

void plDrawSphere() {}  // todo

void plDrawEllipse( unsigned int segments, PLVector2 position, float w, float h, PLColour colour ) {
	static unsigned int last_num_segments = 0;
	static PLMesh *mesh = NULL;
	if ( last_num_segments != segments ) {
		plDestroyMesh( mesh );
		mesh = NULL;
	}

	if ( mesh == NULL) {
		if (( mesh = plCreateMesh(
				PL_MESH_TRIANGLE_FAN,
				PL_DRAW_DYNAMIC,
				0, segments
		)) == NULL) {
			return;
		}
	}

	plSetMeshUniformColour( mesh, colour );
	for ( unsigned int i = 0, pos = 0; i < 360; i += ( 360 / segments )) {
		if ( pos >= segments ) {
			break;
		}

		plSetMeshVertexPosition( mesh, pos++, PLVector3(
				( position.x + w ) + cosf( plDegreesToRadians( ( float ) i )) * w,
				( position.y + h ) + sinf( plDegreesToRadians( ( float ) i )) * h, 0 ) );
	}

	plSetNamedShaderUniformMatrix4( NULL, "pl_model", plMatrix4Identity(), false );
	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

static void SetupRectangleMesh( PLMesh *mesh, float x, float y, float w, float h, PLColour colour ) {
	plSetMeshVertexPosition( mesh, 0, PLVector3( x, y, 0 ) );
	plSetMeshVertexPosition( mesh, 1, PLVector3( x, y + h, 0 ) );
	plSetMeshVertexPosition( mesh, 2, PLVector3( x + w, y, 0 ) );
	plSetMeshVertexPosition( mesh, 3, PLVector3( x + w, y + h, 0 ) );
	plSetMeshUniformColour( mesh, colour );
	plSetMeshVertexST( mesh, 0, 0, 0 );
	plSetMeshVertexST( mesh, 1, 0, 1 );
	plSetMeshVertexST( mesh, 2, 1, 0 );
	plSetMeshVertexST( mesh, 3, 1, 1 );
}

void plDrawTexturedRectangle( const PLMatrix4 *transform, float x, float y, float w, float h, PLTexture *texture ) {
	static PLMesh *mesh = NULL;
	if ( mesh == NULL) {
		mesh = plCreateMesh( PL_MESH_TRIANGLE_STRIP, PL_DRAW_DYNAMIC, 2, 4 );
		if ( mesh == NULL) {
			return;
		}
	}

	SetupRectangleMesh( mesh, x, y, w, h, PLColour( 255, 255, 255, 255 ) );

	plSetTexture( texture, 0 );

	plSetNamedShaderUniformMatrix4( NULL, "pl_model", *transform, true );
	plUploadMesh( mesh );
	plDrawMesh( mesh );

	plSetTexture( NULL, 0 );
}

PLMesh *plCreateMeshRectangle( float x, float y, float w, float h, PLColour colour ) {
	PLMesh *mesh = plCreateMesh( PL_MESH_TRIANGLE_STRIP, PL_DRAW_DYNAMIC, 2, 4 );
	if ( mesh == NULL) {
		return NULL;
	}

	SetupRectangleMesh( mesh, x, y, w, h, colour );

	return mesh;
}

void plDrawRectangle( const PLMatrix4 *transform, float x, float y, float w, float h, PLColour colour ) {
	static PLMesh *mesh = NULL;
	if ( mesh == NULL) {
		if (( mesh = plCreateMesh( PL_MESH_TRIANGLE_STRIP, PL_DRAW_DYNAMIC, 0, 4 )) == NULL) {
			return;
		}
	}

	SetupRectangleMesh( mesh, x, y, w, h, colour );

	plSetNamedShaderUniformMatrix4( NULL, "pl_model", *transform, true );

	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

void plDrawFilledRectangle( const PLRectangle2D *rectangle ) {
	static PLMesh *mesh = NULL;
	if ( mesh == NULL) {
		if (( mesh = plCreateMesh(
				PL_MESH_TRIANGLE_STRIP,
				PL_DRAW_DYNAMIC,
				2, 4
		)) == NULL) {
			return;
		}
	}

	SetupRectangleMesh( mesh, rectangle->xy.x, rectangle->xy.y, rectangle->wh.x, rectangle->wh.y, PLColour( 255, 255, 255, 255 ) );
	plSetMeshVertexColour( mesh, 0, rectangle->ll );
	plSetMeshVertexColour( mesh, 1, rectangle->ul );
	plSetMeshVertexColour( mesh, 2, rectangle->lr );
	plSetMeshVertexColour( mesh, 3, rectangle->ur );

	plSetNamedShaderUniformMatrix4( NULL, "pl_model", plMatrix4Identity(), false );
	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

void plDrawTexturedQuad(
		const PLVector3 *ul, const PLVector3 *ur,
		const PLVector3 *ll, const PLVector3 *lr,
		float hScale, float vScale,
		PLTexture *texture ) {
	static PLMesh *mesh = NULL;
	if ( mesh == NULL) {
		mesh = plCreateMesh( PL_MESH_TRIANGLES, PL_DRAW_DYNAMIC, 2, 4 );
		if ( mesh == NULL) {
			return;
		}
	}

	plSetMeshVertexPosition( mesh, 0, *ul );
	plSetMeshVertexPosition( mesh, 1, *ur );
	plSetMeshVertexPosition( mesh, 2, *ll );
	plSetMeshVertexPosition( mesh, 3, *lr );

	PLVector3 upperDist = plSubtractVector3( *ul, *ur );
	float quadWidth = plVector3Length( &upperDist ) / hScale;
	PLVector3 lowerDist = plSubtractVector3( *ll, *ul );
	float quadHeight = plVector3Length( &lowerDist ) / vScale;

	plSetMeshVertexST( mesh, 3, 0.0f, quadHeight / texture->h );
	plSetMeshVertexST( mesh, 2, quadWidth / texture->w, quadHeight / texture->h );
	plSetMeshVertexST( mesh, 1, 0.0f, 0.0f  );
	plSetMeshVertexST( mesh, 0, quadWidth / texture->w, 0.0f );

	plGenerateMeshNormals( mesh, true );

	unsigned int pos = 0;
	plSetMeshTrianglePosition( mesh, &pos, 0, 1, 2 );
	plSetMeshTrianglePosition( mesh, &pos, 2, 1, 3 );

	plSetMeshUniformColour( mesh, PLColour( 255, 255, 255, 255 ) );

	plSetTexture( texture, 0 );

	plSetNamedShaderUniformMatrix4( NULL, "pl_model", plMatrix4Identity(), true );

	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

void plDrawTriangle( int x, int y, unsigned int w, unsigned int h ) {
	static PLMesh *mesh = NULL;
	if ( mesh == NULL) {
		if (( mesh = plCreateMesh(
				PL_MESH_TRIANGLE_FAN,
				PL_DRAW_DYNAMIC,
				1, 3
		)) == NULL) {
			return;
		}
	}

	plSetMeshVertexPosition( mesh, 0, PLVector3( x, y + h, 0 ) );
	plSetMeshVertexPosition( mesh, 1, PLVector3( x + w / 2, x, 0 ) );
	plSetMeshVertexPosition( mesh, 2, PLVector3( x + w, y + h, 0 ) );

	plSetMeshVertexColour( mesh, 0, PLColour( 255, 0, 0, 255 ) );
	plSetMeshVertexColour( mesh, 1, PLColour( 0, 255, 0, 255 ) );
	plSetMeshVertexColour( mesh, 2, PLColour( 0, 0, 255, 255 ) );

	//plSetMeshUniformColour(mesh, PLColour(255, 0, 0, 255));

	plSetNamedShaderUniformMatrix4( NULL, "pl_model", plMatrix4Identity(), false );
	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

void
plDrawLine( const PLMatrix4 *transform, const PLVector3 *startPos, const PLColour *startColour, const PLVector3 *endPos,
			const PLColour *endColour ) {
	static PLMesh *mesh = NULL;
	if ( mesh == NULL) {
		mesh = plCreateMesh( PL_MESH_LINES, PL_DRAW_DYNAMIC, 0, 2 );
		if ( mesh == NULL) {
			return;
		}
	}

	plSetMeshVertexPosition( mesh, 0, *startPos );
	plSetMeshVertexColour( mesh, 0, *startColour );
	plSetMeshVertexPosition( mesh, 1, *endPos );
	plSetMeshVertexColour( mesh, 1, *endColour );

	plSetNamedShaderUniformMatrix4( NULL, "pl_model", *transform, true );

	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

void plDrawSimpleLine( const PLMatrix4 *transform, const PLVector3 *startPos, const PLVector3 *endPos, const PLColour *colour ) {
	plDrawLine( transform, startPos, colour, endPos, colour );
}

void plDrawGrid( const PLMatrix4 *transform, int x, int y, int w, int h, unsigned int gridSize ) {
	int c = 0, r = 0;
	for ( ; r < h + 1; r += gridSize ) {
		plDrawSimpleLine( transform, &PLVector3( x, r + y, 0 ), &PLVector3( x + w, r + y, 0 ), &PLColour( 255, 255, 255, 255 ) );

		for ( ; c < w + 1; c += gridSize ) {
			plDrawSimpleLine( transform, &PLVector3( c + x, y, 0 ), &PLVector3( c + x, y + h, 0 ), &PLColour( 255, 255, 255, 255 ) );
		}
	}
}

void plDrawMeshNormals( const PLMatrix4 *transform, const PLMesh *mesh ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		PLVector3 linePos = mesh->vertices[ i ].position;
		PLVector3 lineEndPos = plAddVector3( linePos, plScaleVector3f( mesh->vertices[ i ].normal, 64.0f ) );
		plDrawSimpleLine( transform, &linePos, &lineEndPos, &PLColour( 255, 0, 0, 255 ) );
	}
}

#endif
