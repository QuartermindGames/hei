/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl_math.h>
#include <plcore/pl_physics.h>

PL_EXTERN_C

typedef enum PLGMeshPrimitive {
	PLG_MESH_LINES,
	PLG_MESH_LINE_LOOP,
	PLG_MESH_POINTS,
	PLG_MESH_TRIANGLES,
	PLG_MESH_TRIANGLE_STRIP,
	PLG_MESH_TRIANGLE_FAN,
	PLG_MESH_TRIANGLE_FAN_LINE,
	PLG_MESH_QUADS,      /* todo */
	PLG_MESH_QUAD_STRIP, /* todo */

	PLG_NUM_PRIMITIVES
} PLGMeshPrimitive;

typedef enum PLGMeshDrawMode {
	PLG_DRAW_STREAM,
	PLG_DRAW_STATIC,
	PLG_DRAW_DYNAMIC,

	PLG_NUM_DRAWMODES
} PLGMeshDrawMode;

typedef struct PLGVertex {
	PLVector3 position, normal;
	PLVector3 tangent, bitangent;
	PLVector2 st[ 16 ];
	PLColour colour;
} PLGVertex;

typedef struct PLGTexture PLGTexture;

#define PLG_MAX_MESH_BUFFERS 32

typedef struct PLGMesh {
	PLGVertex *vertices;
	unsigned int num_verts;
	unsigned int maxVertices;

	uint32_t numSubMeshes;
	uint32_t maxSubMeshes;
	int32_t *subMeshes;
	int32_t *firstSubMeshes;

	unsigned int *indices;
	unsigned int num_indices;
	unsigned int maxIndices;
	unsigned int start;
	unsigned int range;

	unsigned int num_triangles;

	/* todo: consider throwing out the below */
	struct PLGShaderProgram *shader_program;
	PLGTexture *texture;
	unsigned int materialIndex;

	PLGMeshPrimitive primitive;
	PLGMeshDrawMode mode;

	bool isDirty;

	unsigned int buffers[ PLG_MAX_MESH_BUFFERS ];

	float primitiveScale; /* only matters for points/lines */
} PLGMesh;

typedef struct PLCollisionAABB PLCollisionAABB;

#if !defined( PL_COMPILE_PLUGIN )

PLGMesh *PlgCreateMesh( PLGMeshPrimitive primitive, PLGMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts );
PLGMesh *PlgCreateMeshInit( PLGMeshPrimitive primitive, PLGMeshDrawMode mode, unsigned int numTriangles, unsigned int numVerts,
                            const unsigned int *indicies, const PLGVertex *vertices );
PLGMesh *PlgCreateMeshRectangle( float x, float y, float w, float h, const PLColour *colour );
void PlgDestroyMesh( PLGMesh *mesh );

void PlgDrawEllipse( unsigned int segments, const PLVector2 *position, float w, float h, const PLColour *colour );
void PlgDrawRectangle( float x, float y, float w, float h, PLColour colour );
void PlgDrawLineRectangle( float x, float y, float w, float h, PLColour colour );
void PlgDrawTexturedRectangle( float x, float y, float w, float h, PLGTexture *texture );
void PlgDrawTexturedQuad( const PLVector3 *ul, const PLVector3 *ur, const PLVector3 *ll, const PLVector3 *lr,
                          float hScale, float vScale, PLGTexture *texture );
void PlgDrawLines( const PLVector3 *points, unsigned int numPoints, PLColour colour, float thickness );
void PlgDrawLine( PLVector3 startPos, PLColour startColour, PLVector3 endPos, PLColour endColour );
void PlgDrawSimpleLine( PLVector3 startPos, PLVector3 endPos, PLColour colour );
void PlgDrawGrid( int x, int y, int w, int h, unsigned int gridSize, const PLColour *colour );
void PlgDrawDottedGrid( int x, int y, int w, int h, unsigned int gridSize, const PLColour *colour );
void PlgDrawPixel( int x, int y, PLColour colour );
void PlgDrawBoundingVolume( const PLCollisionAABB *bounds, const PLColour *colour );

void PlgClearMesh( PLGMesh *mesh );
void PlgClearMeshVertices( PLGMesh *mesh );
void PlgClearMeshTriangles( PLGMesh *mesh );

void PlgScaleMesh( PLGMesh *mesh, PLVector3 scale );
void PlgSetMeshTrianglePosition( PLGMesh *mesh, unsigned int *index, unsigned int x, unsigned int y, unsigned int z );
void PlgSetMeshVertexPosition( PLGMesh *mesh, unsigned int index, const PLVector3 *vector );
void PlgSetMeshVertexNormal( PLGMesh *mesh, unsigned int index, const PLVector3 *vector );
void PlgSetMeshVertexST( PLGMesh *mesh, unsigned int index, float s, float t );
void PlgSetMeshVertexSTv( PLGMesh *mesh, uint8_t unit, unsigned int index, unsigned int size, const float *st );
void PlgSetMeshVertexColour( PLGMesh *mesh, unsigned int index, const PLColour *colour );
void PlgSetMeshUniformColour( PLGMesh *mesh, const PLColour *colour );
void PlgSetMeshShaderProgram( PLGMesh *mesh, struct PLGShaderProgram *program );
void PlgSetMeshPrimitiveScale( PLGMesh *mesh, float scale );

unsigned int PlgAddMeshVertex( PLGMesh *mesh, const PLVector3 *position, const PLVector3 *normal, const PLColour *colour, const PLVector2 *st );
unsigned int PlgAddMeshTriangle( PLGMesh *mesh, unsigned int x, unsigned int y, unsigned int z );

void PlgUploadMesh( PLGMesh *mesh );
void PlgDrawMesh( PLGMesh *mesh );
void PlgDrawSubMeshes( PLGMesh *mesh, int32_t *firstSubMeshes, int32_t *subMeshes, uint32_t numSubMeshes );

PLCollisionAABB PlgGenerateAabbFromVertices( const PLGVertex *vertices, unsigned int numVertices, bool absolute );
PLCollisionAABB PlgGenerateAabbFromMesh( const PLGMesh *mesh, bool absolute );

unsigned int PlgGetNumTrianglesForPolygon( unsigned int numVertices );

void PlgGenerateMeshNormals( PLGMesh *mesh, bool perFace );
void PlgGenerateMeshTangentBasis( PLGMesh *mesh );

void PlgGenerateVertexTangentBasis( PLGVertex *vertices, unsigned int numVertices );
void PlgGenerateTangentBasis( PLGVertex *vertices, unsigned int numVertices, const unsigned int *indices, unsigned int numTriangles );
void PlgGenerateTextureCoordinates( PLGVertex *vertices, unsigned int numVertices, PLVector2 textureOffset, PLVector2 textureScale );
void PlgGenerateVertexNormals( PLGVertex *vertices, unsigned int numVertices, unsigned int *indices, unsigned int numTriangles, bool perFace );

PLVector3 PlgGenerateVertexNormal( PLVector3 a, PLVector3 b, PLVector3 c );

/* immediate mode style api */
PLGMesh *PlgImmBegin( PLGMeshPrimitive primitive );
unsigned int PlgImmPushVertex( float x, float y, float z );
void PlgImmNormal( float x, float y, float z );
void PlgImmColour( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
void PlgImmTextureCoord( float s, float t );
unsigned int PlgImmPushTriangle( unsigned int x, unsigned int y, unsigned int z );
void PlgImmSetPrimitiveScale( float scale );
void PlgImmDraw( void );

unsigned int PlgPushTriangle( PLGMesh *mesh, unsigned int x, unsigned int y, unsigned int z );
unsigned int PlgPushVertex3f( PLGMesh *mesh, float x, float y, float z );
unsigned int PlgPushVertex3fv( PLGMesh *mesh, const PLVector3 *vec );
void PlgColour4bv( PLGMesh *mesh, const PLColour *col );

#endif

PL_EXTERN_C_END
