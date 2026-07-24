// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>
// Purpose: Helper drawing methods.
//			This is deprecated!
// Author:  Mark E. Sowden

#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"

/****************************************
 * Immediate-mode style API
 * Possibly consider moving these into
 * some higher-level library?
 ****************************************/

static QmGfxMesh *currentDynamicMesh;
static unsigned int currentVertex;

static unsigned int currentTriangle;

#define MAXIMUM_STORAGE 4096

static QmGfxMesh *meshes[ QM_GFX_MESH_PRIMITIVE_MAX ];
static QmGfxMesh *GetInternalMesh( QmGfxMeshPrimitive primitive ) {
	if ( meshes[ primitive ] == NULL ) {
		return ( meshes[ primitive ] = PlgCreateMesh( primitive, QM_GFX_MESH_DRAW_MODE_STREAM, MAXIMUM_STORAGE, MAXIMUM_STORAGE ) );
	}

	PlgClearMesh( meshes[ primitive ] );
	return meshes[ primitive ];
}

QmGfxMesh *PlgImmBegin( QmGfxMeshPrimitive primitive ) {
	currentDynamicMesh = GetInternalMesh( primitive );
	PlgClearMesh( currentDynamicMesh );
	PlgSetMeshPrimitiveScale( currentDynamicMesh, 1.0f );
	currentVertex = 0;
	return currentDynamicMesh;
}

unsigned int PlgImmPushVertex( float x, float y, float z ) {
	return ( currentVertex = PlgAddMeshVertex( currentDynamicMesh, &QM_MATH_VECTOR3F( x, y, z ), &QM_MATH_VECTOR3F_ZERO, &PL_COLOUR_WHITE, &QM_MATH_VECTOR2F_ZERO ) );
}

void PlgImmNormal( float x, float y, float z ) {
	PlgSetMeshVertexNormal( currentDynamicMesh, currentVertex, &QM_MATH_VECTOR3F( x, y, z ) );
}

void PlgImmColour( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
	PlgSetMeshVertexColour( currentDynamicMesh, currentVertex, &QM_MATH_COLOUR4UB( r, g, b, a ) );
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
	QmGfxShaderProgram *program = PlgGetCurrentShaderProgram();
	if ( program ) {
		int slot;
		if ( ( slot = qm_gfx_shader_program_get_uniform_slot( program, "pl_model" ) ) >= 0 ) {
			qm_gfx_shader_program_set_uniform( program, slot, PlGetMatrix( PL_MODELVIEW_MATRIX ), false );
		}
		if ( ( slot = qm_gfx_shader_program_get_uniform_slot( program, "pl_texture" ) ) >= 0 ) {
			qm_gfx_shader_program_set_uniform( program, slot, PlGetMatrix( PL_TEXTURE_MATRIX ), false );
		}
	}

	PlgUploadMesh( currentDynamicMesh );
	PlgDrawMesh( currentDynamicMesh );
}

unsigned int PlgPushTriangle( QmGfxMesh *mesh, unsigned int x, unsigned int y, unsigned int z ) {
	return PlgAddMeshTriangle( mesh, x, y, z );
}

unsigned int PlgPushVertex3f( QmGfxMesh *mesh, float x, float y, float z ) {
	return PlgAddMeshVertex( mesh, &QM_MATH_VECTOR3F( x, y, z ), &QM_MATH_VECTOR3F_ZERO, &PL_COLOUR_WHITE, &QM_MATH_VECTOR2F_ZERO );
}

void PlgColour4bv( QmGfxMesh *mesh, const QmMathColour4ub *col ) {
	PlgSetMeshVertexColour( mesh, mesh->num_verts - 1, col );
}

/****************************************
 ****************************************/

void PlgClearInternalMeshes( void ) {
	for ( unsigned int i = 0; i < QM_GFX_MESH_PRIMITIVE_MAX; ++i ) {
		if ( meshes[ i ] == NULL ) {
			continue;
		}

		PlgDestroyMesh( meshes[ i ] );
		meshes[ i ] = NULL;
	}
}

static void SetupRectangleMesh( float x, float y, float w, float h, QmMathColour4ub colour ) {
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

void PlgDrawRectangle( float x, float y, float w, float h, QmMathColour4ub colour ) {
	PlgImmBegin( QM_GFX_MESH_PRIMITIVE_TRIANGLE_STRIP );

	SetupRectangleMesh( x, y, w, h, colour );

	PlgImmDraw();
}

void PlgDrawLineRectangle( float x, float y, float w, float h, QmMathColour4ub colour ) {
	PlgImmBegin( QM_GFX_MESH_PRIMITIVE_LINE_LOOP );
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

void PlgDrawLines( const QmMathVector3f *points, unsigned int numPoints, QmMathColour4ub colour, float thickness ) {
	QmGfxMesh *mesh = PlgImmBegin( QM_GFX_MESH_PRIMITIVE_LINES );
	mesh->primitiveScale = thickness;

	for ( unsigned int i = 0; i < numPoints; ++i ) {
		PlgImmPushVertex( points[ i ].x, points[ i ].y, points[ i ].z );
		PlgImmColour( colour.r, colour.g, colour.b, colour.a );
	}

	PlgImmDraw();

	mesh->primitiveScale = 1.0f;
}

void PlgDrawLine( QmMathVector3f startPos, QmMathColour4ub startColour, QmMathVector3f endPos, QmMathColour4ub endColour ) {
	PlgImmBegin( QM_GFX_MESH_PRIMITIVE_LINES );

	PlgImmPushVertex( startPos.x, startPos.y, startPos.z );
	PlgImmColour( startColour.r, startColour.g, startColour.b, startColour.a );

	PlgImmPushVertex( endPos.x, endPos.y, endPos.z );
	PlgImmColour( endColour.r, endColour.g, endColour.b, endColour.a );

	PlgImmDraw();
}

void PlgDrawSimpleLine( QmMathVector3f startPos, QmMathVector3f endPos, QmMathColour4ub colour ) {
	PlgDrawLine( startPos, colour, endPos, colour );
}

void PlgDrawPixel( int x, int y, QmMathColour4ub colour ) {
	int vpW, vpH;
	qm_gfx_get_viewport( NULL, NULL, &vpW, &vpH );

	/* make sure that the pixel is within the viewport */
	if ( x > vpW || x < 0 || y > vpH || y < 0 ) {
		return;
	}

	CallGfxFunction( DrawPixel, x, y, colour );
}
