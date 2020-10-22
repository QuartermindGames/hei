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

/**
 * Generate cubic coordinates for the given vertices.
 */
void plGenerateTextureCoordinates( PLVertex *vertices, unsigned int numVertices, PLVector2 textureOffset, PLVector2 textureScale ) {
	/* todo: figure out what projection face we're using */
	unsigned int l = 0, r = 2;
	PLVector3 projSums[2] = { { 0.0f, 0.0f, 0.0f },
	                          { 0.0f, 0.0f, 0.0f } };
	for ( unsigned int i = 0; i < numVertices; ++i ) {
		for ( unsigned int j = 0; j < 3; ++j ) {
			float v = plVector3Index( vertices[ i ].normal, j );
			float max = plVector3Index( projSums[ 0 ], j );
			if ( v > max ) { plVector3Index( projSums[ 0 ], j ) = v; }
			float min = plVector3Index( projSums[ 1 ], j );
			if ( v < min ) { plVector3Index( projSums[ 1 ], j ) = v; }
		}
	}

	/* now get the greater sum and dir */
#if 0
	float sum = 0.0f;
	unsigned int dir = 0;
	for ( unsigned int j = 0; j < 3; ++j ) {
		float max = plVector3Index( projSums[ 0 ], j );
		float min = plVector3Index( projSums[ 1 ], j );
		if ( sum > max ) { plVector3Index( projSums[ 0 ], j ) = sum; dir = j; }
		else if ( sum < min ) { plVector3Index( projSums[ 1 ], j ) = sum; dir = j; }
	}
#endif

	PLCollisionAABB bounds = plGenerateAABB( vertices, numVertices, true );
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
	return plNormalizeVector3( plVector3CrossProduct( x, y ) );
}

void plGenerateMeshNormals( PLMesh *mesh, bool perFace ) {
	plAssert( mesh );

	plGenerateVertexNormals( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles, perFace );
}

void plGenerateMeshTangentBasis( PLMesh *mesh ) {
	plGenerateTangentBasis( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles );
}

/* based on http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#computing-the-tangents-and-bitangents */
void plGenerateTangentBasis( PLVertex *vertices, unsigned int numVertices, const unsigned int *indices, unsigned int numTriangles ) {
	for ( unsigned int i = 0; i < numTriangles; i += 3 ) {
		PLVertex *a = &vertices[ indices[ i ] ];
		PLVertex *b = &vertices[ indices[ i + 1 ] ];
		PLVertex *c = &vertices[ indices[ i + 2 ] ];

		/* edges of the triangle, aka, position delta */
		PLVector3 deltaPos1 = plSubtractVector3( b->position, a->position );
		PLVector3 deltaPos2 = plSubtractVector3( c->position, a->position );

		/* uv delta */
		PLVector2 deltaUV1 = plSubtractVector2( &b->st[ 0 ], &a->st[ 0 ] );
		PLVector2 deltaUV2 = plSubtractVector2( &c->st[ 0 ], &a->st[ 0 ] );

		/* now actually compute the tangent and bitangent */
		float r = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x );
		PLVector3 tangent = plScaleVector3f( plSubtractVector3( plScaleVector3f( deltaPos1, deltaUV2.y ), plScaleVector3f( deltaPos2, deltaUV1.y ) ), r );
		PLVector3 bitangent = plScaleVector3f( plSubtractVector3( plScaleVector3f( deltaPos2, deltaUV1.x ), plScaleVector3f( deltaPos1, deltaUV2.x ) ), r );

		a->tangent = b->tangent = c->tangent = tangent;
		a->bitangent = b->bitangent = c->bitangent = bitangent;
	}
}

/* software implementation of gouraud shading */
void plApplyMeshLighting( PLMesh *mesh, const PLLight *light, PLVector3 position ) {
	PLVector3 distvec = plSubtractVector3( position, light->position );
	float distance = ( plByteToFloat( light->colour.a ) - plVector3Length( distvec ) ) / 100.f;
	for ( unsigned int i = 0; i < mesh->num_verts; i++ ) {
		PLVector3 normal = mesh->vertices[ i ].normal;
		float angle = ( distance * ( ( normal.x * distvec.x ) + ( normal.y * distvec.y ) + ( normal.z * distvec.z ) ) );
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
	return plCreateMeshInit( primitive, mode, num_tris, num_verts, NULL, NULL );
}

PLMesh *plCreateMeshInit( PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int numTriangles, unsigned int numVerts,
                          const unsigned int *indicies, const PLVertex *verticies ) {
	plAssert( numVerts );

	PLMesh *mesh = ( PLMesh * ) pl_calloc( 1, sizeof( PLMesh ) );
	if ( mesh == NULL ) {
		return NULL;
	}

	mesh->primitive = primitive;
	mesh->mode = mode;

	if ( numTriangles > 0 ) {
		mesh->num_triangles = numTriangles;
		if ( mesh->primitive == PL_MESH_TRIANGLES ) {
			mesh->maxIndices = mesh->num_indices = mesh->num_triangles * 3;
			if ( ( mesh->indices = pl_calloc( mesh->maxIndices, sizeof( unsigned int ) ) ) == NULL ) {
				plDestroyMesh( mesh );
				return NULL;
			}

			if ( indicies != NULL ) {
				memcpy( mesh->indices, indicies, mesh->num_indices * sizeof( unsigned int ) );
			}
		}
	}

	mesh->maxVertices = mesh->num_verts = numVerts;
	mesh->vertices = ( PLVertex * ) pl_calloc( mesh->maxVertices, sizeof( PLVertex ) );
	if ( mesh->vertices == NULL ) {
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
	if ( mesh == NULL ) {
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

unsigned int plAddMeshTriangle( PLMesh *mesh, unsigned int x, unsigned int y, unsigned int z ) {
	unsigned int triangleIndex = mesh->num_indices;

	mesh->num_indices += 3;
	if ( mesh->num_indices >= mesh->maxIndices ) {
		mesh->indices = pl_realloc( mesh->indices, ( mesh->maxIndices += 16 ) * sizeof( unsigned int ) );
	}

	mesh->indices[ triangleIndex ] = x;
	mesh->indices[ triangleIndex + 1 ] = y;
	mesh->indices[ triangleIndex + 2 ] = z;

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

#define MAXIMUM_STORAGE 4096

static PLMesh *_plInitLineMesh( void ) {
	static PLMesh *mesh = NULL;
	if ( mesh != NULL ) {
		plClearMesh( mesh );
		return mesh;
	}

	return ( mesh = plCreateMesh( PL_MESH_LINES, PL_DRAW_DYNAMIC, 0, MAXIMUM_STORAGE ) );
}

static PLMesh *_plInitTriangleMesh( void ) {
	static PLMesh *mesh = NULL;
	if ( mesh != NULL ) {
		plClearMesh( mesh );
		return mesh;
	}

	return ( mesh = plCreateMesh( PL_MESH_TRIANGLES, PL_DRAW_DYNAMIC, MAXIMUM_STORAGE, MAXIMUM_STORAGE ) );
}

static PLMesh *_plInitTriangleFanMesh( void ) {
	static PLMesh *mesh = NULL;
	if ( mesh != NULL ) {
		plClearMesh( mesh );
		return mesh;
	}

	return ( mesh = plCreateMesh( PL_MESH_TRIANGLE_FAN, PL_DRAW_DYNAMIC, 0, MAXIMUM_STORAGE ) );
}

static PLMesh *_plInitTriangleStripMesh( void ) {
	static PLMesh *mesh = NULL;
	if ( mesh != NULL ) {
		plClearMesh( mesh );
		return mesh;
	}

	return ( mesh = plCreateMesh( PL_MESH_TRIANGLE_STRIP, PL_DRAW_DYNAMIC, 0, MAXIMUM_STORAGE ) );
}

void plDrawEllipse( unsigned int segments, PLVector2 position, float w, float h, PLColour colour ) {
	PLMesh *mesh = _plInitTriangleFanMesh();
	if ( mesh == NULL ) {
		return;
	}

	for ( unsigned int i = 0, pos = 0; i < 360; i += ( 360 / segments ) ) {
		if ( pos >= segments ) {
			break;
		}

		PLVector3 coord = PLVector3(
				( position.x + w ) + cosf( plDegreesToRadians( ( float ) i ) ) * w,
				( position.y + h ) + sinf( plDegreesToRadians( ( float ) i ) ) * h,
				0.0f );

		plAddMeshVertex( mesh, coord, pl_vecOrigin3, colour, pl_vecOrigin2 );
	}

	plMatrixMode( PL_MODELVIEW_MATRIX );
	plPushMatrix();

	plLoadIdentityMatrix();

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", plGetMatrix( PL_MODELVIEW_MATRIX ), false );

	plUploadMesh( mesh );
	plDrawMesh( mesh );

	plPopMatrix();
}

static void SetupRectangleMesh( PLMesh *mesh, float x, float y, float w, float h, PLColour colour ) {
	plAddMeshVertex( mesh, PLVector3( x, y, 0.0f ), pl_vecOrigin3, colour, PLVector2( 0.0f, 0.0f ) );
	plAddMeshVertex( mesh, PLVector3( x, y + h, 0.0f ), pl_vecOrigin3, colour, PLVector2( 0.0f, 1.0f ) );
	plAddMeshVertex( mesh, PLVector3( x + w, y, 0.0f ), pl_vecOrigin3, colour, PLVector2( 1.0f, 0.0f ) );
	plAddMeshVertex( mesh, PLVector3( x + w, y + h, 0.0f ), pl_vecOrigin3, colour, PLVector2( 1.0f, 1.0f ) );
}

void plDrawTexturedRectangle( const PLMatrix4 *transform, float x, float y, float w, float h, PLTexture *texture ) {
	PLMesh *mesh = _plInitTriangleStripMesh();
	if ( mesh == NULL ) {
		return;
	}

	SetupRectangleMesh( mesh, x, y, w, h, PLColour( 255, 255, 255, 255 ) );

	plSetTexture( texture, 0 );

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", transform, true );

	plUploadMesh( mesh );
	plDrawMesh( mesh );

	plSetTexture( NULL, 0 );
}

PLMesh *plCreateMeshRectangle( float x, float y, float w, float h, PLColour colour ) {
	PLMesh *mesh = plCreateMesh( PL_MESH_TRIANGLE_STRIP, PL_DRAW_DYNAMIC, 0, 4 );
	if ( mesh == NULL ) {
		return NULL;
	}

	SetupRectangleMesh( mesh, x, y, w, h, colour );

	return mesh;
}

void plDrawRectangle( const PLMatrix4 *transform, float x, float y, float w, float h, PLColour colour ) {
	PLMesh *mesh = _plInitTriangleStripMesh();
	if ( mesh == NULL ) {
		return;
	}

	SetupRectangleMesh( mesh, x, y, w, h, colour );

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", transform, true );

	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

void plDrawFilledRectangle( const PLRectangle2D *rectangle ) {
	PLMesh *mesh = _plInitTriangleStripMesh();
	if ( mesh == NULL ) {
		return;
	}

	SetupRectangleMesh( mesh, rectangle->xy.x, rectangle->xy.y, rectangle->wh.x, rectangle->wh.y, PLColour( 255, 255, 255, 255 ) );

	plSetMeshVertexColour( mesh, 0, rectangle->ll );
	plSetMeshVertexColour( mesh, 1, rectangle->ul );
	plSetMeshVertexColour( mesh, 2, rectangle->lr );
	plSetMeshVertexColour( mesh, 3, rectangle->ur );

	plMatrixMode( PL_MODELVIEW_MATRIX );
	plPushMatrix();

	plLoadIdentityMatrix();

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", plGetMatrix( PL_MODELVIEW_MATRIX ), false );

	plUploadMesh( mesh );
	plDrawMesh( mesh );

	plPopMatrix();
}

void plDrawTexturedQuad( const PLVector3 *ul, const PLVector3 *ur, const PLVector3 *ll, const PLVector3 *lr, float hScale, float vScale, PLTexture *texture ) {
	PLMesh *mesh = _plInitTriangleMesh();
	if ( mesh == NULL ) {
		return;
	}

	PLVector3 upperDist = plSubtractVector3( *ul, *ur );
	float quadWidth = plVector3Length( upperDist ) / hScale;
	PLVector3 lowerDist = plSubtractVector3( *ll, *ul );
	float quadHeight = plVector3Length( lowerDist ) / vScale;

	plAddMeshVertex( mesh, *ul, pl_vecOrigin3, PL_COLOUR_WHITE, PLVector2( 0.0f, quadHeight / texture->h ) );
	plAddMeshVertex( mesh, *ur, pl_vecOrigin3, PL_COLOUR_WHITE, PLVector2( quadWidth / texture->w, quadHeight / texture->h ) );
	plAddMeshVertex( mesh, *ll, pl_vecOrigin3, PL_COLOUR_WHITE, PLVector2( 0.0f, 0.0f ) );
	plAddMeshVertex( mesh, *lr, pl_vecOrigin3, PL_COLOUR_WHITE, PLVector2( quadWidth / texture->w, 0.0f ) );

	plAddMeshTriangle( mesh, 0, 1, 2 );
	plAddMeshTriangle( mesh, 2, 1, 3 );

	plGenerateMeshNormals( mesh, true );

	plSetTexture( texture, 0 );

	plMatrixMode( PL_MODELVIEW_MATRIX );
	plPushMatrix();

	plLoadIdentityMatrix();

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", plGetMatrix( PL_MODELVIEW_MATRIX ), false );

	plUploadMesh( mesh );
	plDrawMesh( mesh );

	plPopMatrix();
}

void plDrawTriangle( int x, int y, unsigned int w, unsigned int h ) {
	PLMesh *mesh = _plInitTriangleFanMesh();
	if ( mesh == NULL ) {
		return;
	}

	plClearMesh( mesh );

	plAddMeshVertex( mesh, PLVector3( x, y + h, 0.0f ), pl_vecOrigin3, PLColour( 255, 0, 0, 255 ), pl_vecOrigin2 );
	plAddMeshVertex( mesh, PLVector3( x + w / 2, x, 0.0f ), pl_vecOrigin3, PLColour( 0, 255, 0, 255 ), pl_vecOrigin2 );
	plAddMeshVertex( mesh, PLVector3( x + w, y + h, 0.0f ), pl_vecOrigin3, PLColour( 0, 0, 255, 255 ), pl_vecOrigin2 );

	//plSetMeshUniformColour(mesh, PLColour(255, 0, 0, 255));

	plMatrixMode( PL_MODELVIEW_MATRIX );
	plPushMatrix();

	plLoadIdentityMatrix();

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", plGetMatrix( PL_MODELVIEW_MATRIX ), false );

	plUploadMesh( mesh );
	plDrawMesh( mesh );

	plPopMatrix();
}

void plDrawLine( PLMatrix4 transform, PLVector3 startPos, PLColour startColour, PLVector3 endPos, PLColour endColour ) {
	PLMesh *mesh = _plInitLineMesh();
	if ( mesh == NULL ) {
		return;
	}

	plAddMeshVertex( mesh, startPos, pl_vecOrigin3, startColour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, endPos, pl_vecOrigin3, endColour, pl_vecOrigin2 );

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", &transform, true );

	plUploadMesh( mesh );
	plDrawMesh( mesh );
}

void plDrawSimpleLine( PLMatrix4 transform, PLVector3 startPos, PLVector3 endPos, PLColour colour ) {
	plDrawLine( transform, startPos, colour, endPos, colour );
}

void plDrawGrid( PLMatrix4 transform, int x, int y, int w, int h, unsigned int gridSize ) {
	int c = 0, r = 0;
	for ( ; r < h + 1; r += gridSize ) {
		plDrawSimpleLine( transform, PLVector3( x, r + y, 0 ), PLVector3( x + w, r + y, 0 ), PLColour( 255, 255, 255, 255 ) );

		for ( ; c < w + 1; c += gridSize ) {
			plDrawSimpleLine( transform, PLVector3( c + x, y, 0 ), PLVector3( c + x, y + h, 0 ), PLColour( 255, 255, 255, 255 ) );
		}
	}
}

/**
 * Draw lines at each vertex point representing the direction of the normal. This is very slow!
 */
void plDrawMeshNormals( const PLMatrix4 *transform, const PLMesh *mesh ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		PLVector3 linePos = mesh->vertices[ i ].position;
		PLVector3 lineEndPos = plAddVector3( linePos, plScaleVector3f( mesh->vertices[ i ].normal, 64.0f ) );
		plDrawSimpleLine( *transform, linePos, lineEndPos, PLColour( 255, 0, 0, 255 ) );
	}
}

/**
 * Utility function for drawing a bounding volume.
 */
void plDrawBoundingVolume( const PLCollisionAABB *bounds, PLColour colour ) {
	PLMesh *mesh = _plInitLineMesh();
	if ( mesh == NULL ) {
		return;
	}

	plMatrixMode( PL_MODELVIEW_MATRIX );
	plPushMatrix();

	plLoadIdentityMatrix();
	plTranslateMatrix( bounds->origin );

	PLVector3 boxPoints[8] = {
			{ bounds->maxs.x, bounds->maxs.y, bounds->maxs.z },
			{ bounds->maxs.x, bounds->mins.y, bounds->maxs.z },
			{ bounds->maxs.x, bounds->maxs.y, bounds->mins.z },
			{ bounds->mins.x, bounds->maxs.y, bounds->maxs.z },

			{ bounds->mins.x, bounds->mins.y, bounds->mins.z },
			{ bounds->mins.x, bounds->maxs.y, bounds->mins.z },
			{ bounds->mins.x, bounds->mins.y, bounds->maxs.z },
			{ bounds->maxs.x, bounds->mins.y, bounds->mins.z },
	};

	plAddMeshVertex( mesh, boxPoints[ 0 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 1 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 0 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 2 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 0 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 3 ], pl_vecOrigin3, colour, pl_vecOrigin2 );

	plAddMeshVertex( mesh, boxPoints[ 4 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 5 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 4 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 6 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 4 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 7 ], pl_vecOrigin3, colour, pl_vecOrigin2 );

	plAddMeshVertex( mesh, boxPoints[ 2 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 5 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 2 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 7 ], pl_vecOrigin3, colour, pl_vecOrigin2 );

	plAddMeshVertex( mesh, boxPoints[ 1 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 6 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 1 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 7 ], pl_vecOrigin3, colour, pl_vecOrigin2 );

	plAddMeshVertex( mesh, boxPoints[ 3 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 5 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 3 ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	plAddMeshVertex( mesh, boxPoints[ 6 ], pl_vecOrigin3, colour, pl_vecOrigin2 );

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", plGetMatrix( PL_MODELVIEW_MATRIX ), true );

	plUploadMesh( mesh );
	plDrawMesh( mesh );

	plPopMatrix();
}

#endif
