/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"
#include "qmos/public/qm_os_memory.h"

/**
 * Generate cubic coordinates for the given vertices.
 */
void PlgGenerateTextureCoordinates( PLGVertex *vertices, unsigned int numVertices, QmMathVector2f textureOffset, QmMathVector2f textureScale ) {
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
		vertices[ i ].st[ 0 ].x = ( PL_VECTOR3_I( vertices[ i ].position, x ) + textureOffset.x ) / textureScale.x;
		vertices[ i ].st[ 0 ].y = ( PL_VECTOR3_I( vertices[ i ].position, y ) + textureOffset.y ) / textureScale.y;
	}
}

void PlgGenerateVertexNormals( PLGVertex *vertices, unsigned int numVertices, unsigned int *indices, unsigned int numTriangles, bool perFace ) {
	if ( perFace ) {
		for ( unsigned int i = 0, idx = 0; i < numTriangles; ++i, idx += 3 ) {
			unsigned int a = indices[ idx ];
			unsigned int b = indices[ idx + 1 ];
			unsigned int c = indices[ idx + 2 ];

			QmMathVector3f normal = PlgGenerateVertexNormal(
			        vertices[ a ].position,
			        vertices[ b ].position,
			        vertices[ c ].position );

			vertices[ a ].normal = qm_math_vector3f_add( vertices[ a ].normal, normal );
			vertices[ b ].normal = qm_math_vector3f_add( vertices[ b ].normal, normal );
			vertices[ c ].normal = qm_math_vector3f_add( vertices[ c ].normal, normal );
		}

		return;
	}

	/* todo: normal generation per vertex */
}

QmMathVector3f PlgGenerateVertexNormal( QmMathVector3f a, QmMathVector3f b, QmMathVector3f c ) {
	QmMathVector3f x = qm_math_vector3f( c.x - b.x, c.y - b.y, c.z - b.z );
	QmMathVector3f y = qm_math_vector3f( a.x - b.x, a.y - b.y, a.z - b.z );
	return qm_math_vector3f_normalize( qm_math_vector3f_cross_product( x, y ) );
}

void PlgGenerateMeshNormals( PLGMesh *mesh, bool perFace ) {
	assert( mesh );

	PlgGenerateVertexNormals( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles, perFace );
}

void PlgGenerateMeshTangentBasis( PLGMesh *mesh ) {
	PlgGenerateTangentBasis( mesh->vertices, mesh->num_verts, mesh->indices, mesh->num_triangles );
}

void PlgGenerateVertexTangentBasis( PLGVertex *vertices, unsigned int numVertices ) {
	for ( unsigned int i = 0; i < numVertices; ++i ) {
		PLGVertex *v = &vertices[ i ];

		QmMathVector3f up, forward;
		PlAnglesAxes( v->normal, NULL, &up, &forward );

		v->tangent = qm_math_vector3f_cross_product( v->normal, forward );
		if ( qm_math_vector3f_length( v->tangent ) == 0 ) {
			v->tangent = qm_math_vector3f_cross_product( v->normal, up );
		}

		v->tangent = qm_math_vector3f_normalize( v->tangent );
		v->bitangent = qm_math_vector3f_normalize( qm_math_vector3f_cross_product( v->normal, v->tangent ) );
	}
}

/* based on http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#computing-the-tangents-and-bitangents */
void PlgGenerateTangentBasis( PLGVertex *vertices, unsigned int numVertices, const unsigned int *indices, unsigned int numTriangles ) {
	for ( unsigned int i = 0; i < numTriangles; i++, indices += 3 ) {
		PLGVertex *a = &vertices[ indices[ 0 ] ];
		PLGVertex *b = &vertices[ indices[ 1 ] ];
		PLGVertex *c = &vertices[ indices[ 2 ] ];

		/* edges of the triangle, aka, position delta */
		QmMathVector3f deltaPos1 = qm_math_vector3f_sub( b->position, a->position );
		QmMathVector3f deltaPos2 = qm_math_vector3f_sub( c->position, a->position );

		/* uv delta */
		QmMathVector2f deltaUV1 = qm_math_vector2f_sub( b->st[ 0 ], a->st[ 0 ] );
		QmMathVector2f deltaUV2 = qm_math_vector2f_sub( c->st[ 0 ], a->st[ 0 ] );

		/* now actually compute the tangent and bitangent */
		float r = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x );

		QmMathVector3f tangent = qm_math_vector3f_scale_float( qm_math_vector3f_sub( qm_math_vector3f_scale_float( deltaPos1, deltaUV2.y ), qm_math_vector3f_scale_float( deltaPos2, deltaUV1.y ) ), r );
		QmMathVector3f bitangent = qm_math_vector3f_scale_float( qm_math_vector3f_add( qm_math_vector3f_scale_float( deltaPos1, -deltaUV2.x ), qm_math_vector3f_scale_float( deltaPos2, deltaUV1.x ) ), r );

		a->tangent = b->tangent = c->tangent = tangent;
		a->bitangent = b->bitangent = c->bitangent = bitangent;
	}
}

/* software implementation of gouraud shading */
void PlgApplyMeshLighting( PLGMesh *mesh, const PLGLight *light, QmMathVector3f position ) {
	QmMathVector3f distvec = qm_math_vector3f_sub( position, light->position );
	float distance = ( QM_OS_BYTE_TO_FLOAT( light->colour.a ) - qm_math_vector3f_length( distvec ) ) / 100.f;
	for ( unsigned int i = 0; i < mesh->num_verts; i++ ) {
		QmMathVector3f normal = mesh->vertices[ i ].normal;
		float angle = ( distance * ( ( normal.x * distvec.x ) + ( normal.y * distvec.y ) + ( normal.z * distvec.z ) ) );
		if ( angle < 0 ) {
			mesh->vertices[ i ].colour = ( QmMathColour4ub ){};
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
	PLGMesh *mesh = ( PLGMesh * ) QM_OS_MEMORY_CALLOC( 1, sizeof( PLGMesh ) );
	mesh->primitive = primitive;
	mesh->mode = mode;

	if ( numTriangles > 0 ) {
		if ( mesh->primitive == PLG_MESH_TRIANGLES ) {
			unsigned int numIndices = numTriangles * 3; /* todo: this is too assumptious... */
			mesh->maxIndices = numIndices;
			mesh->indices = QM_OS_MEMORY_CALLOC( mesh->maxIndices, sizeof( unsigned int ) );
			if ( indicies != NULL ) {
				memcpy( mesh->indices, indicies, sizeof( unsigned int ) * numIndices );
				mesh->num_indices = numIndices;
				mesh->num_triangles = numTriangles;
				mesh->range = mesh->num_indices;
			}
		}
	}

	mesh->maxVertices = numVerts;
	mesh->vertices = QM_OS_MEMORY_NEW_( PLGVertex, mesh->maxVertices );

	// If the vertices passed in aren't null, copy them into our vertex list
	if ( vertices != NULL && mesh->num_verts > 0 ) {
		memcpy( mesh->vertices, vertices, sizeof( PLGVertex ) * numVerts );
		mesh->num_verts = numVerts;
	}

	mesh->isDirty = true;

	CallGfxFunction( CreateMesh, mesh );

	return mesh;
}

PLGMesh *PlgCreateMeshRectangle( float x, float y, float w, float h, const QmMathColour4ub *colour ) {
	PLGMesh *mesh = PlgCreateMesh( PLG_MESH_TRIANGLE_STRIP, PLG_DRAW_DYNAMIC, 0, 4 );
	if ( mesh == NULL ) {
		return NULL;
	}

	PlgAddMeshVertex( mesh, &QM_MATH_VECTOR3F( x, y, 0.0f ), &pl_vecOrigin3, colour, &QM_MATH_VECTOR2F( 0.0f, 0.0f ) );
	PlgAddMeshVertex( mesh, &QM_MATH_VECTOR3F( x, y + h, 0.0f ), &pl_vecOrigin3, colour, &QM_MATH_VECTOR2F( 0.0f, 1.0f ) );
	PlgAddMeshVertex( mesh, &QM_MATH_VECTOR3F( x + w, y, 0.0f ), &pl_vecOrigin3, colour, &QM_MATH_VECTOR2F( 1.0f, 0.0f ) );
	PlgAddMeshVertex( mesh, &QM_MATH_VECTOR3F( x + w, y + h, 0.0f ), &pl_vecOrigin3, colour, &QM_MATH_VECTOR2F( 1.0f, 1.0f ) );

	return mesh;
}

void PlgDestroyMesh( PLGMesh *mesh ) {
	if ( mesh == NULL ) {
		return;
	}

	CallGfxFunction( DeleteMesh, mesh );

	//qm_os_memory_free( mesh->subMeshes );
	//qm_os_memory_free( mesh->firstSubMeshes );

	qm_os_memory_free( mesh->vertices );
	qm_os_memory_free( mesh->indices );
	qm_os_memory_free( mesh );
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
	mesh->range = mesh->start = 0;
	mesh->num_triangles = mesh->num_indices = 0;
	mesh->isDirty = true;
}

void PlgScaleMesh( PLGMesh *mesh, QmMathVector3f scale ) {
	for ( unsigned int i = 0; i < mesh->num_verts; ++i ) {
		mesh->vertices[ i ].position = qm_math_vector3f_scale( mesh->vertices[ i ].position, scale );
	}
}

void PlgSetMeshTrianglePosition( PLGMesh *mesh, unsigned int *index, unsigned int x, unsigned int y, unsigned int z ) {
	assert( *index < mesh->maxIndices );
	mesh->indices[ ( *index )++ ] = x;
	mesh->indices[ ( *index )++ ] = y;
	mesh->indices[ ( *index )++ ] = z;
}

void PlgSetMeshVertexPosition( PLGMesh *mesh, unsigned int index, const QmMathVector3f *vector ) {
	assert( index < mesh->maxVertices );
	mesh->vertices[ index ].position = *vector;
}

void PlgSetMeshVertexNormal( PLGMesh *mesh, unsigned int index, const QmMathVector3f *vector ) {
	assert( index < mesh->maxVertices );
	mesh->vertices[ index ].normal = *vector;
}

void PlgSetMeshVertexST( PLGMesh *mesh, unsigned int index, float s, float t ) {
	assert( index < mesh->maxVertices );
	mesh->vertices[ index ].st[ 0 ] = qm_math_vector2f( s, t );
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

void PlgSetMeshVertexColour( PLGMesh *mesh, unsigned int index, const QmMathColour4ub *colour ) {
	assert( index < mesh->maxVertices );
	mesh->vertices[ index ].colour = *colour;
}

void PlgSetMeshUniformColour( PLGMesh *mesh, const QmMathColour4ub *colour ) {
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

unsigned int PlgAddMeshVertex( PLGMesh *mesh, const QmMathVector3f *position, const QmMathVector3f *normal, const QmMathColour4ub *colour, const QmMathVector2f *st ) {
	unsigned int vertexIndex = mesh->num_verts++;
	if ( vertexIndex >= mesh->maxVertices ) {
		mesh->vertices = qm_os_memory_realloc( mesh->vertices, ( mesh->maxVertices += 16 ) * sizeof( PLGVertex ) );
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
		mesh->indices = qm_os_memory_realloc( mesh->indices, ( mesh->maxIndices += 16 ) * sizeof( unsigned int ) );
	}

	mesh->range = mesh->num_indices;

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
	QmMathVector3f *vvertices = QM_OS_MEMORY_MALLOC_( sizeof( QmMathVector3f ) * numVertices );
	for ( unsigned int i = 0; i < numVertices; ++i ) {
		vvertices[ i ] = vertices[ i ].position;
	}

	PLCollisionAABB bounds = {};
	qm_math_compute_min_max( vvertices, numVertices, &bounds.mins, &bounds.maxs, absolute );

	bounds.absOrigin = PlGetAabbAbsOrigin( &bounds, pl_vecOrigin3 );
	bounds.origin    = pl_vecOrigin3;

	qm_os_memory_free( vvertices );

	return bounds;
}

PLCollisionAABB PlgGenerateAabbFromMesh( const PLGMesh *mesh, bool absolute ) {
	return PlgGenerateAabbFromVertices( mesh->vertices, mesh->num_verts, absolute );
}

unsigned int PlgGetNumTrianglesForPolygon( unsigned int numVertices ) {
	return ( numVertices < 3 ) ? 0 : numVertices - 2;
}
