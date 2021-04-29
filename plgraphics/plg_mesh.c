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

#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"

/**
 * Generate cubic coordinates for the given vertices.
 */
void PlgGenerateTextureCoordinates( PLGVertex *vertices, unsigned int numVertices, PLVector2 textureOffset, PLVector2 textureScale ) {
	if ( textureScale.x == 0.0f || textureScale.y == 0.0f ) {
		return;
	}

	unsigned int x, y;
	for ( unsigned int i = 0; i < numVertices; ++i ) {
        if ( ( fabsf( vertices[ i ].normal.x ) > fabsf( vertices[ i ].normal.y ) ) &&
		     ( fabsf( vertices[ i ].normal.x ) > fabsf( vertices[ i ].normal.z ) ) ) {
			x = ( vertices[ i ].normal.x > 0.0 ) ? 1 : 2;
			y = ( vertices[ i ].normal.x > 0.0 ) ? 2 : 1;
		} else if ( ( fabsf( vertices[ i ].normal.z ) > fabsf( vertices[ i ].normal.x ) ) &&
		            ( fabsf( vertices[ i ].normal.z ) > fabsf( vertices[ i ].normal.y ) ) ) {
			x = ( vertices[ i ].normal.z > 0.0 ) ? 0 : 1;
			y = ( vertices[ i ].normal.z > 0.0 ) ? 1 : 0;
		} else {
            x = ( vertices[ i ].normal.y > 0.0 ) ? 2 : 0;
            y = ( vertices[ i ].normal.y > 0.0 ) ? 0 : 2;
        }

		/* why the weird multiplication at the end here? to roughly match previous scaling values */
        vertices[ i ].st[ 0 ].x = ( PlVector3Index( vertices[ i ].position, x ) + textureOffset.x ) / ( textureScale.x * 500.0f );
        vertices[ i ].st[ 0 ].y = ( PlVector3Index( vertices[ i ].position, y ) + textureOffset.y ) / ( textureScale.y * 500.0f );
	}
}

void PlgGenerateVertexNormals( PLGVertex *vertices, unsigned int numVertices, unsigned int *indices, unsigned int numTriangles, bool perFace ) {
	if ( perFace ) {
		for ( unsigned int i = 0, idx = 0; i < numTriangles; ++i, idx += 3 ) {
			unsigned int a = indices[ idx ];
			unsigned int b = indices[ idx + 1 ];
			unsigned int c = indices[ idx + 2 ];

			PLVector3 normal = PlgGenerateVertexNormal(
			        vertices[ a ].position,
			        vertices[ b ].position,
			        vertices[ c ].position );

			vertices[ a ].normal = PlAddVector3( vertices[ a ].normal, normal );
			vertices[ b ].normal = PlAddVector3( vertices[ b ].normal, normal );
			vertices[ c ].normal = PlAddVector3( vertices[ c ].normal, normal );
		}

		return;
	}

	/* todo: normal generation per vertex */
}

PLVector3 PlgGenerateVertexNormal( PLVector3 a, PLVector3 b, PLVector3 c ) {
	PLVector3 x = PLVector3( c.x - b.x, c.y - b.y, c.z - b.z );
	PLVector3 y = PLVector3( a.x - b.x, a.y - b.y, a.z - b.z );
	return PlNormalizeVector3( PlVector3CrossProduct( x, y ) );
}

void PlgGenerateMeshNormals( PLGMesh *mesh, bool perFace ) {
	plAssert( mesh );

	PlgGenerateVertexNormals( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles, perFace );
}

void PlgGenerateMeshTangentBasis( PLGMesh *mesh ) {
	PlgGenerateTangentBasis( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles );
}

/* based on http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#computing-the-tangents-and-bitangents */
void PlgGenerateTangentBasis( PLGVertex *vertices, unsigned int numVertices, const unsigned int *indices, unsigned int numTriangles ) {
	for ( unsigned int i = 0; i < numTriangles; i++, indices += 3 ) {
		PLGVertex *a = &vertices[ indices[ 0 ] ];
		PLGVertex *b = &vertices[ indices[ 1 ] ];
		PLGVertex *c = &vertices[ indices[ 2 ] ];

		/* edges of the triangle, aka, position delta */
		PLVector3 deltaPos1 = PlSubtractVector3( b->position, a->position );
		PLVector3 deltaPos2 = PlSubtractVector3( c->position, a->position );

		/* uv delta */
		PLVector2 deltaUV1 = PlSubtractVector2( &b->st[ 0 ], &a->st[ 0 ] );
		PLVector2 deltaUV2 = PlSubtractVector2( &c->st[ 0 ], &a->st[ 0 ] );

		/* now actually compute the tangent and bitangent */
		float r = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x );
		PLVector3 tangent = PlScaleVector3F( PlSubtractVector3( PlScaleVector3F( deltaPos1, deltaUV2.y ), PlScaleVector3F( deltaPos2, deltaUV1.y ) ), r );
		PLVector3 bitangent = PlScaleVector3F( PlSubtractVector3( PlScaleVector3F( deltaPos2, deltaUV1.x ), PlScaleVector3F( deltaPos1, deltaUV2.x ) ), r );

		a->tangent = b->tangent = c->tangent = tangent;
		a->bitangent = b->bitangent = c->bitangent = bitangent;
	}
}

/* software implementation of gouraud shading */
void plApplyMeshLighting( PLGMesh *mesh, const PLGLight *light, PLVector3 position ) {
	PLVector3 distvec = PlSubtractVector3( position, light->position );
	float distance = ( PlByteToFloat( light->colour.a ) - PlVector3Length( distvec ) ) / 100.f;
	for ( unsigned int i = 0; i < mesh->num_verts; i++ ) {
		PLVector3 normal = mesh->vertices[ i ].normal;
		float angle = ( distance * ( ( normal.x * distvec.x ) + ( normal.y * distvec.y ) + ( normal.z * distvec.z ) ) );
		if ( angle < 0 ) {
			PlClearColour( &mesh->vertices[ i ].colour );
		} else {
			mesh->vertices[ i ].colour.r = light->colour.r * PlFloatToByte( angle );
			mesh->vertices[ i ].colour.g = light->colour.g * PlFloatToByte( angle );
			mesh->vertices[ i ].colour.b = light->colour.b * PlFloatToByte( angle );
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

PLGMesh *PlgCreateMesh( PLGMeshPrimitive primitive, PLGMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts ) {
	return PlgCreateMeshInit( primitive, mode, num_tris, num_verts, NULL, NULL );
}

PLGMesh *PlgCreateMeshInit( PLGMeshPrimitive primitive, PLGMeshDrawMode mode, unsigned int numTriangles, unsigned int numVerts,
                          const unsigned int *indicies, const PLGVertex *vertices ) {
	plAssert( numVerts );

	PLGMesh *mesh = ( PLGMesh * ) pl_calloc( 1, sizeof( PLGMesh ) );
	if ( mesh == NULL ) {
		return NULL;
	}

	mesh->primitive = primitive;
	mesh->mode = mode;

	if ( numTriangles > 0 ) {
		mesh->num_triangles = numTriangles;
		if ( mesh->primitive == PLG_MESH_TRIANGLES ) {
			mesh->maxIndices = mesh->num_indices = mesh->num_triangles * 3;
			if ( ( mesh->indices = pl_calloc( mesh->maxIndices, sizeof( unsigned int ) ) ) == NULL ) {
				PlgDestroyMesh( mesh );
				return NULL;
			}

			if ( indicies != NULL ) {
				memcpy( mesh->indices, indicies, mesh->num_indices * sizeof( unsigned int ) );
			}
		}
	}

	mesh->maxVertices = mesh->num_verts = numVerts;
	mesh->vertices = ( PLGVertex * ) pl_calloc( mesh->maxVertices, sizeof( PLGVertex ) );
	if ( mesh->vertices == NULL ) {
		PlgDestroyMesh( mesh );
		return NULL;
	}

	if ( vertices != NULL ) {
		memcpy( mesh->vertices, vertices, sizeof( PLGVertex ) * mesh->num_verts );
	}

	CallGfxFunction( CreateMesh, mesh );

	return mesh;
}

void PlgDestroyMesh( PLGMesh *mesh ) {
	if ( mesh == NULL ) {
		return;
	}

	CallGfxFunction( DeleteMesh, mesh );

	pl_free( mesh->vertices );
	pl_free( mesh->indices );
	pl_free( mesh );
}

void PlgClearMesh( PLGMesh *mesh ) {
	PlgClearMeshVertices( mesh );
	PlgClearMeshTriangles( mesh );
}

void PlgClearMeshVertices( PLGMesh *mesh ) {
	mesh->num_verts = 0;
}

void PlgClearMeshTriangles( PLGMesh *mesh ) {
	mesh->num_triangles = mesh->num_indices = 0;
}

void PlgScaleMesh( PLGMesh *mesh, PLVector3 scale ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		mesh->vertices[ i ].position = PlScaleVector3( mesh->vertices[ i ].position, scale );
	}
}

void PlgSetMeshTrianglePosition( PLGMesh *mesh, unsigned int *index, unsigned int x, unsigned int y, unsigned int z ) {
	plAssert( *index < mesh->maxIndices );
	mesh->indices[ ( *index )++ ] = x;
	mesh->indices[ ( *index )++ ] = y;
	mesh->indices[ ( *index )++ ] = z;
}

void PlgSetMeshVertexPosition( PLGMesh *mesh, unsigned int index, PLVector3 vector ) {
	plAssert( index < mesh->maxVertices );
	mesh->vertices[ index ].position = vector;
}

void PlgSetMeshVertexNormal( PLGMesh *mesh, unsigned int index, PLVector3 vector ) {
	plAssert( index < mesh->maxVertices );
	mesh->vertices[ index ].normal = vector;
}

void PlgSetMeshVertexST( PLGMesh *mesh, unsigned int index, float s, float t ) {
	plAssert( index < mesh->maxVertices );
	mesh->vertices[ index ].st[ 0 ] = PLVector2( s, t );
}

void PlgSetMeshVertexSTv( PLGMesh *mesh, uint8_t unit, unsigned int index, unsigned int size, const float *st ) {
	size += index;
	if ( size > mesh->num_verts ) {
		size -= ( size - mesh->num_verts );
	}

	for ( unsigned int i = index; i < size; i++ ) {
		mesh->vertices[ i ].st[ unit ].x = st[ 0 ];
		mesh->vertices[ i ].st[ unit ].y = st[ 1 ];
	}
}

void PlgSetMeshVertexColour( PLGMesh *mesh, unsigned int index, PLColour colour ) {
	plAssert( index < mesh->maxVertices );
	mesh->vertices[ index ].colour = colour;
}

void PlgSetMeshUniformColour( PLGMesh *mesh, PLColour colour ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		mesh->vertices[ i ].colour = colour;
	}
}

void PlgSetMeshShaderProgram( PLGMesh *mesh, PLGShaderProgram *program ) {
	mesh->shader_program = program;
}

unsigned int PlgAddMeshVertex( PLGMesh *mesh, PLVector3 position, PLVector3 normal, PLColour colour, PLVector2 st ) {
	unsigned int vertexIndex = mesh->num_verts++;
	if ( vertexIndex >= mesh->maxVertices ) {
		mesh->vertices = pl_realloc( mesh->vertices, ( mesh->maxVertices += 16 ) * sizeof( PLGVertex ) );
	}

	PlgSetMeshVertexPosition( mesh, vertexIndex, position );
	PlgSetMeshVertexNormal( mesh, vertexIndex, normal );
	PlgSetMeshVertexColour( mesh, vertexIndex, colour );
	PlgSetMeshVertexST( mesh, vertexIndex, st.x, st.y );

	return vertexIndex;
}

unsigned int PlgAddMeshTriangle( PLGMesh *mesh, unsigned int x, unsigned int y, unsigned int z ) {
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

/* todo: combine with Draw? */
void PlgUploadMesh( PLGMesh *mesh ) {
	CallGfxFunction( UploadMesh, mesh, gfx_state.current_program );
}

void PlgDrawMesh( PLGMesh *mesh ) {
	if ( gfx_state.current_program != NULL ) {
		PlgSetShaderUniformValue( gfx_state.current_program, "pl_view", gfx_state.view_matrix.m, false );
		PlgSetShaderUniformValue( gfx_state.current_program, "pl_proj", gfx_state.projection_matrix.m, false );
	}

	CallGfxFunction( DrawMesh, mesh, gfx_state.current_program );
}

void PlgDrawInstancedMesh( PLGMesh *mesh, const PLMatrix4 *transforms, unsigned int instanceCount ) {
	CallGfxFunction( DrawInstancedMesh, mesh, gfx_state.current_program, transforms, instanceCount );
}

PLCollisionAABB PlgGenerateAabbFromVertices( const PLGVertex *vertices, unsigned int numVertices, bool absolute ) {
    PLVector3 *vvertices = pl_malloc( sizeof( PLVector3 ) * numVertices );
    for ( unsigned int i = 0; i < numVertices; ++i ) {
        vvertices[ i ] = vertices[ i ].position;
    }

    PLCollisionAABB bounds = PlGenerateAabbFromCoords( vvertices, numVertices, absolute );

    pl_free( vvertices );

    return bounds;
}

PLCollisionAABB PlgGenerateAabbFromMesh( const PLGMesh *mesh, bool absolute ) {
	return PlgGenerateAabbFromVertices( mesh->vertices, mesh->num_verts, absolute );
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

void plDrawLines( const PLVector3 *points, unsigned int numPoints, PLColour colour ) {
	PLMesh *mesh = _plInitLineMesh();
	if ( mesh == NULL ) {
		return;
	}

	for ( unsigned int i = 0; i < numPoints; ++i ) {
		plAddMeshVertex( mesh, points[ i ], pl_vecOrigin3, colour, pl_vecOrigin2 );
	}

	plSetShaderUniformValue( plGetCurrentShaderProgram(), "pl_model", plGetMatrix( PL_MODELVIEW_MATRIX ), true );

	plUploadMesh( mesh );
	plDrawMesh( mesh );
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
 * Draw lines at each vertex point representing the direction of the normal.
 */
void plDrawMeshNormals( const PLMatrix4 *transform, const PLMesh *mesh ) {
	static PLMesh *linesMesh = NULL;
	if ( linesMesh == NULL ) {
		linesMesh = plCreateMesh( PL_MESH_LINES, PL_DRAW_DYNAMIC, 0, mesh->num_verts * 2 );
		if ( linesMesh == NULL ) {
			return;
		}
	}
#if 0
	else {
		unsigned int numVerts = mesh->num_verts * 2;
		if ( numVerts < linesMesh->num_verts ) {
			linesMesh->vertices = pl_realloc( linesMesh->vertices, sizeof( PLVertex ) * numVerts );
		}
	}
#endif

	plClearMesh( linesMesh );

	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		PLVector3 linePos = mesh->vertices[ i ].position;
		PLVector3 lineEndPos = plAddVector3( linePos, plScaleVector3f( mesh->vertices[ i ].normal, 64.0f ) );
		plAddMeshVertex( linesMesh, linePos, pl_vecOrigin3, PLColour( 255, 0, 0, 255 ), pl_vecOrigin2 );
		plAddMeshVertex( linesMesh, lineEndPos, pl_vecOrigin3, PLColour( 0, 255, 0, 255 ), pl_vecOrigin2 );
	}

	plUploadMesh( linesMesh );
	plDrawMesh( linesMesh );
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
