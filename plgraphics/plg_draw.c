/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"


/****************************************
 * Immediate-mode style API
 * Possibly consider moving these into
 * some higher-level library?
 ****************************************/

static PLGMesh *currentDynamicMesh;
static unsigned int currentVertex;

static unsigned int currentTriangle;

#define MAXIMUM_STORAGE 4096

static PLGMesh *meshes[ PLG_NUM_PRIMITIVES ];
static PLGMesh *GetInternalMesh( PLGMeshPrimitive primitive ) {
	if ( meshes[ primitive ] == NULL ) {
		return ( meshes[ primitive ] = PlgCreateMesh( primitive, PLG_DRAW_DYNAMIC, MAXIMUM_STORAGE, MAXIMUM_STORAGE ) );
	}

	PlgClearMesh( meshes[ primitive ] );
	return meshes[ primitive ];
}

PLGMesh *PlgImmBegin( PLGMeshPrimitive primitive ) {
	currentDynamicMesh = GetInternalMesh( primitive );
	PlgClearMesh( currentDynamicMesh );
	PlgSetMeshPrimitiveScale( currentDynamicMesh, 1.0f );
	currentVertex = 0;
	return currentDynamicMesh;
}

unsigned int PlgImmPushVertex( float x, float y, float z ) {
	return ( currentVertex = PlgAddMeshVertex( currentDynamicMesh, &QM_MATH_VECTOR3F( x, y, z ), &pl_vecOrigin3, &PL_COLOUR_WHITE, &pl_vecOrigin2 ) );
}

void PlgImmNormal( float x, float y, float z ) {
	PlgSetMeshVertexNormal( currentDynamicMesh, currentVertex, &QM_MATH_VECTOR3F( x, y, z ) );
}

void PlgImmColour( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
	PlgSetMeshVertexColour( currentDynamicMesh, currentVertex, &PL_COLOURU8( r, g, b, a ) );
}

void PlgImmTextureCoord( float s, float t ) {
	PlgSetMeshVertexST( currentDynamicMesh, currentVertex, s, t );
}

unsigned int PlgImmPushTriangle( unsigned int x, unsigned int y, unsigned int z ) {
	return ( currentTriangle = PlgAddMeshTriangle( currentDynamicMesh, x, y, z ) );
}

void PlgImmSetPrimitiveScale( float scale ) {
	PlgSetMeshPrimitiveScale( currentDynamicMesh, scale );
}

void PlgImmDraw( void ) {
	PLGShaderProgram *program = PlgGetCurrentShaderProgram();
	if ( program ) {
		int slot;
		if ( ( slot = PlgGetShaderUniformSlot( program, "pl_model" ) ) >= 0 ) {
			PlgSetShaderUniformValueByIndex( program, slot, PlGetMatrix( PL_MODELVIEW_MATRIX ), false );
		}
		if ( ( slot = PlgGetShaderUniformSlot( program, "pl_texture" ) ) >= 0 ) {
			PlgSetShaderUniformValueByIndex( program, slot, PlGetMatrix( PL_TEXTURE_MATRIX ), false );
		}
	}

	PlgUploadMesh( currentDynamicMesh );
	PlgDrawMesh( currentDynamicMesh );
}

unsigned int PlgPushTriangle( PLGMesh *mesh, unsigned int x, unsigned int y, unsigned int z ) {
	return PlgAddMeshTriangle( mesh, x, y, z );
}

unsigned int PlgPushVertex3f( PLGMesh *mesh, float x, float y, float z ) {
	return PlgAddMeshVertex( mesh, &QM_MATH_VECTOR3F( x, y, z ), &pl_vecOrigin3, &PL_COLOUR_WHITE, &pl_vecOrigin2 );
}

unsigned int PlgPushVertex3fv( PLGMesh *mesh, const PLVector3 *vec ) {
	return PlgAddMeshVertex( mesh, vec, &pl_vecOrigin3, &PL_COLOUR_WHITE, &pl_vecOrigin2 );
}

void PlgColour4bv( PLGMesh *mesh, const PLColour *col ) {
	PlgSetMeshVertexColour( mesh, mesh->num_verts - 1, col );
}

/****************************************
 ****************************************/

void PlgInitializeInternalMeshes( void ) {
	PL_ZERO_( meshes );
}

void PlgClearInternalMeshes( void ) {
	for ( unsigned int i = 0; i < PLG_NUM_PRIMITIVES; ++i ) {
		if ( meshes[ i ] == NULL ) {
			continue;
		}

		PlgDestroyMesh( meshes[ i ] );
		meshes[ i ] = NULL;
	}
}

void PlgDrawEllipse( unsigned int segments, const PLVector2 *position, float w, float h, const PLColour *colour ) {
	PLGMesh *mesh = GetInternalMesh( PLG_MESH_TRIANGLE_FAN );

	for ( unsigned int i = 0, pos = 0; i < 360; i += ( 360 / segments ) ) {
		if ( pos >= segments ) {
			break;
		}

		PLVector3 coord = qm_math_vector3f(
		        ( position->x + w ) + cosf( PL_DEG2RAD( ( float ) i ) ) * w,
		        ( position->y + h ) + sinf( PL_DEG2RAD( ( float ) i ) ) * h,
		        0.0f );

		PlgAddMeshVertex( mesh, &coord, &pl_vecOrigin3, colour, &pl_vecOrigin2 );
	}

	PlMatrixMode( PL_MODELVIEW_MATRIX );
	PlPushMatrix();

	PlLoadIdentityMatrix();

	PLGShaderProgram *program = PlgGetCurrentShaderProgram();
	if ( program ) {
		int slot;
		if ( ( slot = PlgGetShaderUniformSlot( program, "pl_model" ) ) >= 0 ) {
			PlgSetShaderUniformValueByIndex( program, slot, PlGetMatrix( PL_MODELVIEW_MATRIX ), false );
		}
	}

	PlgUploadMesh( mesh );
	PlgDrawMesh( mesh );

	PlPopMatrix();
}

static void SetupRectangleMesh( float x, float y, float w, float h, PLColour colour ) {
	PlgImmPushVertex( x, y, 0.0f );
	PlgImmTextureCoord( 0.0f, 0.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );

	PlgImmPushVertex( x, y + h, 0.0f );
	PlgImmTextureCoord( 0.0f, 1.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );

	PlgImmPushVertex( x + w, y, 0.0f );
	PlgImmTextureCoord( 1.0f, 0.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );

	PlgImmPushVertex( x + w, y + h, 0.0f );
	PlgImmTextureCoord( 1.0f, 1.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );
}

void PlgDrawTexturedRectangle( float x, float y, float w, float h, PLGTexture *texture ) {
	PlgSetTexture( texture, 0 );

	PlgDrawRectangle( x, y, w, h, PLColour( 255, 255, 255, 255 ) );

	PlgSetTexture( NULL, 0 );
}

void PlgDrawRectangle( float x, float y, float w, float h, PLColour colour ) {
	PlgImmBegin( PLG_MESH_TRIANGLE_STRIP );

	SetupRectangleMesh( x, y, w, h, colour );

	PlgImmDraw();
}

void PlgDrawLineRectangle( float x, float y, float w, float h, PLColour colour ) {
	PlgImmBegin( PLG_MESH_LINE_LOOP );
	PlgImmPushVertex( x, y, 0.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );
	PlgImmPushVertex( x + w, y, 0.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );
	PlgImmPushVertex( x + w, y + h, 0.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );
	PlgImmPushVertex( x, y + h, 0.0f );
	PlgImmColour( colour.r, colour.g, colour.b, colour.a );
	PlgImmDraw();
}

void PlgDrawTexturedQuad( const PLVector3 *ul, const PLVector3 *ur, const PLVector3 *ll, const PLVector3 *lr, float hScale, float vScale, PLGTexture *texture ) {
	PLGMesh *mesh = GetInternalMesh( PLG_MESH_TRIANGLES );

	PLVector3 upperDist = qm_math_vector3f_sub( *ul, *ur );
	float quadWidth = qm_math_vector3f_length( upperDist ) / hScale;
	PLVector3 lowerDist = qm_math_vector3f_sub( *ll, *ul );
	float quadHeight = qm_math_vector3f_length( lowerDist ) / vScale;

	PlgAddMeshVertex( mesh, ul, &pl_vecOrigin3, &PL_COLOUR_WHITE, &QM_MATH_VECTOR2F( 0.0f, quadHeight / texture->h ) );
	PlgAddMeshVertex( mesh, ur, &pl_vecOrigin3, &PL_COLOUR_WHITE, &QM_MATH_VECTOR2F( quadWidth / texture->w, quadHeight / texture->h ) );
	PlgAddMeshVertex( mesh, ll, &pl_vecOrigin3, &PL_COLOUR_WHITE, &QM_MATH_VECTOR2F( 0.0f, 0.0f ) );
	PlgAddMeshVertex( mesh, lr, &pl_vecOrigin3, &PL_COLOUR_WHITE, &QM_MATH_VECTOR2F( quadWidth / texture->w, 0.0f ) );

	PlgAddMeshTriangle( mesh, 0, 1, 2 );
	PlgAddMeshTriangle( mesh, 2, 1, 3 );

	PlgGenerateMeshNormals( mesh, true );

	PlgSetTexture( texture, 0 );

	PlMatrixMode( PL_MODELVIEW_MATRIX );
	PlPushMatrix();

	PlLoadIdentityMatrix();

	PLGShaderProgram *program = PlgGetCurrentShaderProgram();
	if ( program ) {
		int slot;
		if ( ( slot = PlgGetShaderUniformSlot( program, "pl_model" ) ) >= 0 ) {
			PlgSetShaderUniformValueByIndex( program, slot, PlGetMatrix( PL_MODELVIEW_MATRIX ), false );
		}
		if ( ( slot = PlgGetShaderUniformSlot( program, "pl_texture" ) ) >= 0 ) {
			PlgSetShaderUniformValueByIndex( program, slot, PlGetMatrix( PL_TEXTURE_MATRIX ), false );
		}
	}

	PlgUploadMesh( mesh );
	PlgDrawMesh( mesh );

	PlPopMatrix();
}

void PlgDrawLines( const PLVector3 *points, unsigned int numPoints, PLColour colour, float thickness ) {
	PLGMesh *mesh = PlgImmBegin( PLG_MESH_LINES );
	mesh->primitiveScale = thickness;

	for ( unsigned int i = 0; i < numPoints; ++i ) {
		PlgImmPushVertex( points[ i ].x, points[ i ].y, points[ i ].z );
		PlgImmColour( colour.r, colour.g, colour.b, colour.a );
	}

	PlgImmDraw();

	mesh->primitiveScale = 1.0f;
}

void PlgDrawLine( PLVector3 startPos, PLColour startColour, PLVector3 endPos, PLColour endColour ) {
	PlgImmBegin( PLG_MESH_LINES );

	PlgImmPushVertex( startPos.x, startPos.y, startPos.z );
	PlgImmColour( startColour.r, startColour.g, startColour.b, startColour.a );

	PlgImmPushVertex( endPos.x, endPos.y, endPos.z );
	PlgImmColour( endColour.r, endColour.g, endColour.b, endColour.a );

	PlgImmDraw();
}

void PlgDrawSimpleLine( PLVector3 startPos, PLVector3 endPos, PLColour colour ) {
	PlgDrawLine( startPos, colour, endPos, colour );
}

void PlgDrawGrid( int x, int y, int w, int h, unsigned int gridSize, const PLColour *colour ) {
	PlgImmBegin( PLG_MESH_LINES );

	int c = 0, r = 0;
	for ( ; r < h + 1; r += ( int ) gridSize ) {
		PlgImmPushVertex( x, y + r, 0.0f );
		PlgImmColour( colour->r, colour->g, colour->b, colour->a );
		PlgImmPushVertex( x + w, r + y, 0.0f );
		PlgImmColour( colour->r, colour->g, colour->b, colour->a );
		for ( ; c < w + 1; c += ( int ) gridSize ) {
			PlgImmPushVertex( c + x, y, 0.0f );
			PlgImmColour( colour->r, colour->g, colour->b, colour->a );
			PlgImmPushVertex( c + x, y + h, 0.0f );
			PlgImmColour( colour->r, colour->g, colour->b, colour->a );
		}
	}

	PlgImmDraw();
}

void PlgDrawDottedGrid( int x, int y, int w, int h, unsigned int gridSize, const PLColour *colour ) {
	if ( gridSize == 0 ) {
		return;
	}

	PlgImmBegin( PLG_MESH_POINTS );

	for ( int r = 0; r < h + 1; r += ( int ) gridSize ) {
		for ( int c = 0; c < w + 1; c += ( int ) gridSize ) {
			PlgImmPushVertex( c + x, y + r, 0.0f );
			PlgImmColour( colour->r, colour->g, colour->b, colour->a );
		}
	}

	PlgImmDraw();
}

void PlgDrawPixel( int x, int y, PLColour colour ) {
	int vpW, vpH;
	PlgGetViewport( NULL, NULL, &vpW, &vpH );

	/* make sure that the pixel is within the viewport */
	if ( x > vpW || x < 0 || y > vpH || y < 0 ) {
		return;
	}

	CallGfxFunction( DrawPixel, x, y, colour );
}

/**
 * Utility function for drawing a bounding volume.
 */
void PlgDrawBoundingVolume( const PLCollisionAABB *bounds, const PLColour *colour ) {
	PlMatrixMode( PL_MODELVIEW_MATRIX );
	PlPushMatrix();

	PlTranslateMatrix( bounds->origin );

	PlgImmBegin( PLG_MESH_LINES );

	PlgImmPushVertex( bounds->mins.x, bounds->mins.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->mins.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->mins.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->maxs.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->maxs.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->maxs.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->maxs.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->mins.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );

	PlgImmPushVertex( bounds->mins.x, bounds->mins.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->mins.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->mins.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->maxs.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->maxs.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->maxs.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->maxs.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->mins.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );

	PlgImmPushVertex( bounds->mins.x, bounds->mins.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->mins.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->mins.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->mins.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->maxs.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->maxs.x, bounds->maxs.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->maxs.y, bounds->mins.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );
	PlgImmPushVertex( bounds->mins.x, bounds->maxs.y, bounds->maxs.z );
	PlgImmColour( colour->r, colour->g, colour->b, colour->a );

	PlgImmDraw();

	PlPopMatrix();
}
