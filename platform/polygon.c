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

#include "../graphics/graphics_private.h"

PLPolygon *plCreatePolygon( PLTexture *texture, PLVector2 textureOffset, PLVector2 textureScale, float textureRotation ) {
	PLPolygon *polygon = pl_calloc( 1, sizeof( PLPolygon ) );
	polygon->texture = texture;
	polygon->textureOffset = textureOffset;
	polygon->textureScale = textureScale;
	polygon->textureRotation = textureRotation;
	return polygon;
}

void plDestroyPolygon( PLPolygon *polygon ) {
	if( polygon == NULL ) {
		return;
	}

	pl_free( polygon );
}

/**
 * Generate vertex normals for the given polygon.
 */
void plGeneratePolygonNormals( PLPolygon *polygon ) {
	unsigned int numTriangles;
	unsigned int *indices = plConvertPolygonToTriangles( polygon, &numTriangles );

	plGenerateVertexNormals( polygon->vertices, polygon->numVertices, indices, numTriangles, true );

	pl_free( indices );
}

/**
 * Generate face normal for the given polygon.
 */
void plGeneratePolygonFaceNormal( PLPolygon *polygon ) {
	unsigned int numTriangles;
	unsigned int *indices = plConvertPolygonToTriangles( polygon, &numTriangles );

	PLVector3 *normals = pl_malloc( sizeof( PLVector3 ) * polygon->numVertices );
	for ( unsigned int i = 0, idx = 0; i < numTriangles; ++i, idx += 3 ) {
		unsigned int a = indices[ idx ];
		unsigned int b = indices[ idx + 1 ];
		unsigned int c = indices[ idx + 2 ];

		PLVector3 normal = plGenerateVertexNormal(
				polygon->vertices[ a ].position,
				polygon->vertices[ b ].position,
				polygon->vertices[ c ].position
		);

		normals[ a ] = plAddVector3( normals[ a ], normal );
		normals[ b ] = plAddVector3( normals[ b ], normal );
		normals[ c ] = plAddVector3( normals[ c ], normal );
	}

	polygon->normal = normals[ 0 ];

	pl_free( normals );
	pl_free( indices );
}

void plAddPolygonVertex( PLPolygon *polygon, const PLVertex *vertex ) {
	if( polygon->numVertices >= PL_POLYGON_MAX_SIDES ) {
		ReportError( PL_RESULT_INVALID_PARM2, "reached maximum number of polygon sides (%d)", PL_POLYGON_MAX_SIDES );
		return;
	}

	polygon->vertices[ polygon->numVertices ] = *vertex;
	polygon->numVertices++;

	plGeneratePolygonFaceNormal( polygon );

#if 0
	if( polygon->texture != NULL ) {
		plGenerateTextureCoordinates( polygon->vertices, polygon->numVertices, polygon->textureOffset, polygon->textureScale );
	}
#endif
}

void plRemovePolygonVertex( PLPolygon *polygon, unsigned int vertIndex ) {
	if( vertIndex >= polygon->numVertices ) {
		ReportError( PL_RESULT_INVALID_PARM2, "invalid vertex index (%d)", polygon->numVertices );
		return;
	}

	/* reshuffle the list */
	memmove( polygon->vertices + vertIndex, polygon->vertices + vertIndex + 1, ( polygon->numVertices - vertIndex ) - 1);
	--( polygon->numVertices );

	plGeneratePolygonFaceNormal( polygon );

#if 0
	if( polygon->texture != NULL ) {
		plGenerateTextureCoordinates( polygon->vertices, polygon->numVertices, polygon->textureOffset, polygon->textureScale );
	}
#endif
}

unsigned int plGetNumOfPolygonVertices( const PLPolygon *polygon ) {
	return polygon->numVertices;
}

PLVertex *plGetPolygonVertex( PLPolygon *polygon, unsigned int vertIndex ) {
	if( vertIndex >= polygon->numVertices ) {
		ReportError( PL_RESULT_INVALID_PARM2, "invalid vertex index, %d", vertIndex );
		return NULL;
	}

	return &polygon->vertices[ vertIndex ];
}

PLVertex *plGetPolygonVertices( PLPolygon *polygon, unsigned int *numVertices ) {
	*numVertices = polygon->numVertices;
	return polygon->vertices;
}

PLTexture *plGetPolygonTexture( PLPolygon *polygon ) {
	return polygon->texture;
}

PLVector3 plGetPolygonFaceNormal( const PLPolygon *polygon ) {
	return polygon->normal;
}

/**
 * Return the number of triangles in this polygon.
 */
unsigned int plGetNumOfPolygonTriangles( const PLPolygon *polygon ) {
	if ( polygon->numVertices < 3 ) {
		return 0;
	}

	return polygon->numVertices - 2;
}

unsigned int *plConvertPolygonToTriangles( const PLPolygon *polygon, unsigned int *numTriangles ) {
	*numTriangles = plGetNumOfPolygonTriangles( polygon );
	if ( *numTriangles == 0 ) {
		ReportError( PL_RESULT_INVALID_PARM1, "invalid polygon" );
		return NULL;
	}

	unsigned int *indices = pl_malloc( sizeof( unsigned int ) * ( *numTriangles * 3 ) );
	unsigned int *index = indices;
	for ( unsigned int i = 1; i + 1 < polygon->numVertices; ++i ) {
		index[ 0 ] = 0;
		index[ 1 ] = i;
		index[ 2 ] = i + 1;
		index += 3;
	}

	return indices;
}

PLMesh *plConvertPolygonToMesh( const PLPolygon *polygon ) {
	unsigned int numTriangles;
	unsigned int *indices = plConvertPolygonToTriangles( polygon, &numTriangles );
	if ( indices == NULL ) {
		return NULL;
	}

	PLMesh *mesh = plCreateMeshInit( PL_MESH_TRIANGLES, PL_DRAW_STATIC, numTriangles, polygon->numVertices, indices, polygon->vertices );

	pl_free( indices );

	return mesh;
}
