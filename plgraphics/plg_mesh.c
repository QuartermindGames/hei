// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"

void PlgInitializeVertexLayout( PLGVertexLayout *layout ) {
	PlgClearVertexLayout( layout );
}

void PlgFreeVertexLayout( PLGVertexLayout *layout ) {
}

void PlgClearVertexLayout( PLGVertexLayout *layout ) {
	layout->numElements = 0;
	layout->dirty = true;
}

void PlgSetVertexLayoutElement( PLGVertexLayout *layout, PLGVertexLayoutElementType type, PLGDataType dataType, intptr_t offset, unsigned int numElements ) {
	if ( layout->numElements >= PLG_MAX_VERTEX_LAYOUT_ELEMENTS ) {
		PlReportErrorF( PL_RESULT_MEMORY_EOA, "hit vertex layout element limit (%u >= %u)",
		                layout->numElements, PLG_MAX_VERTEX_LAYOUT_ELEMENTS );
	}

	layout->elements[ layout->numElements ].numSubElements = numElements;
	layout->elements[ layout->numElements ].offset = offset;
	layout->elements[ layout->numElements ].type = type;
	layout->elements[ layout->numElements ].dataType = dataType;
	layout->numElements++;
	layout->dirty = true;
}

void PlgSetVertexLayoutData( PLGVertexLayout *layout, void *data ) {
	layout->vertexData = data;
	layout->dirty = true;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

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
		vertices[ i ].st[ 0 ].x = ( PlVector3Index( vertices[ i ].position, x ) + textureOffset.x ) / textureScale.x;
		vertices[ i ].st[ 0 ].y = ( PlVector3Index( vertices[ i ].position, y ) + textureOffset.y ) / textureScale.y;
	}
}

void PlgGenerateVertexNormals( PLGVertex *vertices, unsigned int numVertices, const unsigned int *indices, unsigned int numTriangles, bool perFace ) {
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
	PL_ASSERT( mesh );

	PlgGenerateVertexNormals( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles, perFace );
}

void PlgGenerateMeshTangentBasis( PLGMesh *mesh ) {
	PlgGenerateTangentBasis( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles );
}

void PlgGenerateVertexTangentBasis( PLGVertex *vertices, unsigned int numVertices ) {
	for ( unsigned int i = 0; i < numVertices; ++i ) {
		PLGVertex *v = &vertices[ i ];

		PLVector3 up, forward;
		PlAnglesAxes( v->normal, NULL, &up, &forward );

		v->tangent = PlVector3CrossProduct( v->normal, forward );
		if ( PlVector3Length( v->tangent ) == 0 ) {
			v->tangent = PlVector3CrossProduct( v->normal, up );
		}

		v->tangent = PlNormalizeVector3( v->tangent );
		v->bitangent = PlNormalizeVector3( PlVector3CrossProduct( v->normal, v->tangent ) );
	}
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
		PLVector3 bitangent = PlScaleVector3F( PlAddVector3( PlScaleVector3F( deltaPos1, -deltaUV2.x ), PlScaleVector3F( deltaPos2, deltaUV1.x ) ), r );

		a->tangent = b->tangent = c->tangent = tangent;
		a->bitangent = b->bitangent = c->bitangent = bitangent;
	}
}

/* software implementation of gouraud shading */
void PlgApplyMeshLighting( PLGMesh *mesh, const PLGLight *light, PLVector3 position ) {
	PLVector3 distvec = PlSubtractVector3( position, light->position );
	float distance = ( PlByteToFloat( light->colour.a ) - PlVector3Length( distvec ) ) / 100.f;
	for ( unsigned int i = 0; i < mesh->num_verts; i++ ) {
		PLVector3 normal = mesh->vertices[ i ].normal;
		float angle = ( distance * ( ( normal.x * distvec.x ) + ( normal.y * distvec.y ) + ( normal.z * distvec.z ) ) );
		if ( angle < 0 ) {
			mesh->vertices[ i ].light.r = 0.0f;
			mesh->vertices[ i ].light.g = 0.0f;
			mesh->vertices[ i ].light.b = 0.0f;
		} else {
			mesh->vertices[ i ].light.r = light->colour.r * PlFloatToByte( angle );
			mesh->vertices[ i ].light.g = light->colour.g * PlFloatToByte( angle );
			mesh->vertices[ i ].light.b = light->colour.b * PlFloatToByte( angle );
		}
	}
}

PLGMesh *PlgCreateMesh( PLGMeshPrimitive primitive, PLGMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts ) {
	return PlgCreateMeshInit( primitive, mode, num_tris, num_verts, NULL, NULL );
}

PLGMesh *PlgCreateMeshInit( PLGMeshPrimitive primitive, PLGMeshDrawMode mode, unsigned int numTriangles, unsigned int numVerts,
                            const unsigned int *indicies, const PLGVertex *vertices ) {
	PL_ASSERT( numVerts );

	PLGMesh *mesh = PL_NEW( PLGMesh );
	mesh->primitive = primitive;
	mesh->mode = mode;

	if ( numTriangles > 0 ) {
		if ( mesh->primitive == PLG_MESH_TRIANGLES ) {
			unsigned int numIndices = numTriangles * 3; /* todo: this is too assumptious... */
			mesh->maxIndices = numIndices;
			mesh->indices = PlCAllocA( mesh->maxIndices, sizeof( unsigned int ) );
			if ( indicies != NULL ) {
				memcpy( mesh->indices, indicies, sizeof( unsigned int ) * numIndices );
				mesh->num_indices = numIndices;
				mesh->num_triangles = numTriangles;
			}
		}
	}

	mesh->maxVertices = numVerts;
	mesh->vertices = ( PLGVertex * ) PlCAllocA( mesh->maxVertices, sizeof( PLGVertex ) );

	PlgSetVertexLayoutData( &mesh->vertexLayout, mesh->vertices );
	PlgSetVertexLayoutElement( &mesh->vertexLayout, PLG_VERTEX_LAYOUT_ELEMENT_TYPE_POSITION, PLG_DATA_TYPE_FLOAT, PL_OFFSETOF( PLGVertex, position ), 3 );
	PlgSetVertexLayoutElement( &mesh->vertexLayout, PLG_VERTEX_LAYOUT_ELEMENT_TYPE_NORMAL, PLG_DATA_TYPE_FLOAT, PL_OFFSETOF( PLGVertex, normal ), 3 );
	PlgSetVertexLayoutElement( &mesh->vertexLayout, PLG_VERTEX_LAYOUT_ELEMENT_TYPE_TANGENT, PLG_DATA_TYPE_FLOAT, PL_OFFSETOF( PLGVertex, tangent ), 3 );
	PlgSetVertexLayoutElement( &mesh->vertexLayout, PLG_VERTEX_LAYOUT_ELEMENT_TYPE_BITANGENT, PLG_DATA_TYPE_FLOAT, PL_OFFSETOF( PLGVertex, bitangent ), 3 );
	PlgSetVertexLayoutElement( &mesh->vertexLayout, PLG_VERTEX_LAYOUT_ELEMENT_TYPE_UV, PLG_DATA_TYPE_FLOAT, PL_OFFSETOF( PLGVertex, st ), 2 );
	PlgSetVertexLayoutElement( &mesh->vertexLayout, PLG_VERTEX_LAYOUT_ELEMENT_TYPE_COLOUR, PLG_DATA_TYPE_FLOAT, PL_OFFSETOF( PLGVertex, colour ), 4 );

	// If the vertices passed in aren't null, copy them into our vertex list
	if ( vertices != NULL ) {//-V1051
		memcpy( mesh->vertices, vertices, sizeof( PLGVertex ) * numVerts );
		mesh->num_verts = numVerts;
	}

	mesh->isDirty = true;

	CallGfxFunction( CreateMesh, mesh );

	return mesh;
}

PLGMesh *PlgCreateMeshRectangle( float x, float y, float w, float h, const PLColourF32 *colour ) {
	PLGMesh *mesh = PlgCreateMesh( PLG_MESH_TRIANGLE_STRIP, PLG_DRAW_DYNAMIC, 0, 4 );
	if ( mesh == NULL ) {
		return NULL;
	}

	PlgAddMeshVertex( mesh, &PLVector3( x, y, 0.0f ), &pl_vecOrigin3, colour, &PLVector2( 0.0f, 0.0f ) );
	PlgAddMeshVertex( mesh, &PLVector3( x, y + h, 0.0f ), &pl_vecOrigin3, colour, &PLVector2( 0.0f, 1.0f ) );
	PlgAddMeshVertex( mesh, &PLVector3( x + w, y, 0.0f ), &pl_vecOrigin3, colour, &PLVector2( 1.0f, 0.0f ) );
	PlgAddMeshVertex( mesh, &PLVector3( x + w, y + h, 0.0f ), &pl_vecOrigin3, colour, &PLVector2( 1.0f, 1.0f ) );

	return mesh;
}

void PlgDestroyMesh( PLGMesh *mesh ) {
	if ( mesh == NULL ) {
		return;
	}

	CallGfxFunction( DeleteMesh, mesh );

	PL_DELETE( mesh->subMeshes );
	PL_DELETE( mesh->firstSubMeshes );

	PlFree( mesh->vertices );
	PlFree( mesh->indices );
	PlFree( mesh );
}

void PlgClearMesh( PLGMesh *mesh ) {
	PlgClearMeshVertices( mesh );
	PlgClearMeshTriangles( mesh );

	mesh->numSubMeshes = 0;
}

void PlgClearMeshVertices( PLGMesh *mesh ) {
	mesh->num_verts = 0;
	mesh->isDirty = true;
}

void PlgClearMeshTriangles( PLGMesh *mesh ) {
	mesh->num_triangles = mesh->num_indices = 0;
	mesh->isDirty = true;
}

void PlgScaleMesh( PLGMesh *mesh, PLVector3 scale ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		mesh->vertices[ i ].position = PlScaleVector3( mesh->vertices[ i ].position, scale );
	}
}

void PlgSetMeshTrianglePosition( PLGMesh *mesh, unsigned int *index, unsigned int x, unsigned int y, unsigned int z ) {
	PL_ASSERT( *index < mesh->maxIndices );
	mesh->indices[ ( *index )++ ] = x;
	mesh->indices[ ( *index )++ ] = y;
	mesh->indices[ ( *index )++ ] = z;
}

void PlgSetMeshVertexPosition( PLGMesh *mesh, unsigned int index, const PLVector3 *vector ) {
	PL_ASSERT( index < mesh->maxVertices );
	mesh->vertices[ index ].position = *vector;
}

void PlgSetMeshVertexNormal( PLGMesh *mesh, unsigned int index, const PLVector3 *vector ) {
	PL_ASSERT( index < mesh->maxVertices );
	mesh->vertices[ index ].normal = *vector;
}

void PlgSetMeshVertexST( PLGMesh *mesh, unsigned int index, float s, float t ) {
	PL_ASSERT( index < mesh->maxVertices );
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

void PlgSetMeshVertexColour( PLGMesh *mesh, unsigned int index, const PLColourF32 *colour ) {
	PL_ASSERT( index < mesh->maxVertices );
	mesh->vertices[ index ].colour = *colour;
}

void PlgSetMeshUniformColour( PLGMesh *mesh, const PLColourF32 *colour ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		PlgSetMeshVertexColour( mesh, i, colour );
	}
}

void PlgSetMeshShaderProgram( PLGMesh *mesh, PLGShaderProgram *program ) {
	mesh->shader_program = program;
}

/**
 * Sets the draw scale for the particular primitive type.
 * Only applies for POINTS / LINES.
 */
void PlgSetMeshPrimitiveScale( PLGMesh *mesh, float scale ) {
	mesh->primitiveScale = scale;
}

unsigned int PlgAddMeshVertex( PLGMesh *mesh, const PLVector3 *position, const PLVector3 *normal, const PLColourF32 *colour, const PLVector2 *st ) {
	unsigned int vertexIndex = mesh->num_verts++;
	if ( vertexIndex >= mesh->maxVertices ) {
		mesh->vertices = PlReAllocA( mesh->vertices, ( mesh->maxVertices += 16 ) * sizeof( PLGVertex ) );
	}

	PlgSetMeshVertexPosition( mesh, vertexIndex, position );
	PlgSetMeshVertexNormal( mesh, vertexIndex, normal );
	PlgSetMeshVertexColour( mesh, vertexIndex, colour );
	PlgSetMeshVertexST( mesh, vertexIndex, st->x, st->y );

	return vertexIndex;
}

unsigned int PlgAddMeshTriangle( PLGMesh *mesh, unsigned int x, unsigned int y, unsigned int z ) {
	unsigned int triangleIndex = mesh->num_indices;

	mesh->num_indices += 3;
	if ( mesh->num_indices >= mesh->maxIndices ) {
		mesh->indices = PlReAllocA( mesh->indices, ( mesh->maxIndices += 16 ) * sizeof( unsigned int ) );
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

/**
 * Draws a collection of subsets of the given mesh.
 */
void PlgDrawSubMeshes( PLGMesh *mesh, int32_t *firstSubMeshes, int32_t *subMeshes, uint32_t numSubMeshes ) {
	// urgh, this is just a botch for now,
	// eventually we should introduce a proper call for it, probably
	mesh->subMeshes = subMeshes;
	mesh->firstSubMeshes = firstSubMeshes;
	mesh->numSubMeshes = numSubMeshes;

	PlgDrawMesh( mesh );

	// we can just set it to 0 here, and it won't operate any more...
	mesh->numSubMeshes = 0;
}

void PlgDrawInstancedMesh( PLGMesh *mesh, const PLMatrix4 *transforms, unsigned int instanceCount ) {
	CallGfxFunction( DrawInstancedMesh, mesh, gfx_state.current_program, transforms, instanceCount );
}

PLCollisionAABB PlgGenerateAabbFromVertices( const PLGVertex *vertices, unsigned int numVertices, bool absolute ) {
	PLVector3 *vvertices = PlMAllocA( sizeof( PLVector3 ) * numVertices );
	for ( unsigned int i = 0; i < numVertices; ++i ) {
		vvertices[ i ] = vertices[ i ].position;
	}

	PLCollisionAABB bounds = PlGenerateAabbFromCoords( vvertices, numVertices, absolute );

	PlFree( vvertices );

	return bounds;
}

PLCollisionAABB PlgGenerateAabbFromMesh( const PLGMesh *mesh, bool absolute ) {
	return PlgGenerateAabbFromVertices( mesh->vertices, mesh->num_verts, absolute );
}

unsigned int PlgGetNumTrianglesForPolygon( unsigned int numVertices ) {
	return ( numVertices < 3 ) ? 0 : numVertices - 2;
}
