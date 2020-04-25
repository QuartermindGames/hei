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

PLPolygon *plCreatePolygon( void ) {
	return pl_calloc( 1, sizeof( PLPolygon ) );
}

void plDestroyPolygon( PLPolygon *polygon ) {
	if( polygon == NULL ) {
		return;
	}

	pl_free( polygon );
}

void plAddPolygonVertex( PLPolygon *polygon, const PLVertex *vertex ) {
	if( polygon->numVertices >= PL_POLYGON_MAX_SIDES ) {
		ReportError( PL_RESULT_INVALID_PARM2, "reached maximum number of polygon sides (%d)", PL_POLYGON_MAX_SIDES );
		return;
	}

	polygon->vertices[ polygon->numVertices ] = *vertex;
	polygon->numVertices++;
}

void plRemovePolygonVertex( PLPolygon *polygon, unsigned int vertIndex ) {
	if( vertIndex >= polygon->numVertices ) {
		ReportError( PL_RESULT_INVALID_PARM2, "invalid vertex index (%d)", polygon->numVertices );
		return;
	}

	/* reshuffle the list */
	memmove( polygon->vertices + vertIndex, polygon->vertices + vertIndex + 1, ( polygon->numVertices - vertIndex ) - 1);
	--( polygon->numVertices );
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
	return polygon->vertices;
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
	plGenerateMeshNormals( mesh, true );

	pl_free( indices );

	return mesh;
}
