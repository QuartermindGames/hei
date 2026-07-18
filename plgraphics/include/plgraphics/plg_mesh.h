// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>

#pragma once

#include <plcore/pl_math.h>
#include <plcore/pl_physics.h>

PL_EXTERN_C

typedef enum QmGfxMeshPrimitive
{
	QM_GFX_MESH_PRIMITIVE_LINES,
	QM_GFX_MESH_PRIMITIVE_LINE_LOOP,
	PLG_MESH_POINTS,
	PLG_MESH_TRIANGLES,
	PLG_MESH_TRIANGLE_STRIP,
	PLG_MESH_TRIANGLE_FAN,
	PLG_MESH_TRIANGLE_FAN_LINE,

	PLG_NUM_PRIMITIVES
} QmGfxMeshPrimitive;

/* If these confuse you, like they do me, there's a really good write-up here.
 * https://www.reddit.com/r/opengl/comments/57i9cl/comment/d8s8wnq/
 * tldr; stream for update every frame, dynamic for update occasionally and static for no updates.
 */
typedef enum QmGfxMeshDrawMode
{
	QM_GFX_MESH_DRAW_MODE_STREAM,
	QM_GFX_MESH_DRAW_MODE_STATIC,
	QM_GFX_MESH_DRAW_MODE_DYNAMIC,
} QmGfxMeshDrawMode;

typedef struct QmGfxMeshVertex
{
	QmMathVector3f  position, normal;
	QmMathVector3f  tangent, bitangent;
	QmMathVector2f  st[ 16 ];
	QmMathColour4ub colour;
} QmGfxMeshVertex;

typedef struct QmGfxMesh
{
	QmGfxMeshVertex *vertices;
	unsigned int     num_verts;
	unsigned int     maxVertices;

	uint32_t numSubMeshes;
	uint32_t maxSubMeshes;
	int32_t *subMeshes;
	int32_t *firstSubMeshes;

	unsigned int *indices;
	unsigned int  num_indices;
	unsigned int  maxIndices;
	unsigned int  start;
	unsigned int  range;

	unsigned int num_triangles;

	QmGfxMeshPrimitive primitive;
	QmGfxMeshDrawMode  mode;

	bool isDirty;

	float primitiveScale; /* only matters for points/lines */

	void *driver;
} QmGfxMesh;

typedef struct PLCollisionAABB PLCollisionAABB;

#if !defined( PL_COMPILE_PLUGIN )

QmGfxMesh *PlgCreateMesh( QmGfxMeshPrimitive primitive, QmGfxMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts );
QmGfxMesh *PlgCreateMeshInit( QmGfxMeshPrimitive primitive, QmGfxMeshDrawMode mode, unsigned int numTriangles, unsigned int numVerts,
                              const unsigned int *indicies, const QmGfxMeshVertex *vertices );
void       PlgDestroyMesh( QmGfxMesh *mesh );

void PlgDrawRectangle( float x, float y, float w, float h, QmMathColour4ub colour );
void PlgDrawLineRectangle( float x, float y, float w, float h, QmMathColour4ub colour );
void PlgDrawLines( const QmMathVector3f *points, unsigned int numPoints, QmMathColour4ub colour, float thickness );
void PlgDrawLine( QmMathVector3f startPos, QmMathColour4ub startColour, QmMathVector3f endPos, QmMathColour4ub endColour );
void PlgDrawSimpleLine( QmMathVector3f startPos, QmMathVector3f endPos, QmMathColour4ub colour );
void PlgDrawPixel( int x, int y, QmMathColour4ub colour );

void PlgClearMesh( QmGfxMesh *mesh );

void PlgSetMeshVertexPosition( QmGfxMesh *mesh, unsigned int index, const QmMathVector3f *vector );
void PlgSetMeshVertexNormal( QmGfxMesh *mesh, unsigned int index, const QmMathVector3f *vector );
void PlgSetMeshVertexST( QmGfxMesh *mesh, unsigned int index, float s, float t );
void PlgSetMeshVertexSTv( QmGfxMesh *mesh, uint8_t unit, unsigned int index, unsigned int size, const float *st );
void PlgSetMeshVertexColour( QmGfxMesh *mesh, unsigned int index, const QmMathColour4ub *colour );
void PlgSetMeshPrimitiveScale( QmGfxMesh *mesh, float scale );

unsigned int PlgAddMeshVertex( QmGfxMesh *mesh, const QmMathVector3f *position, const QmMathVector3f *normal, const QmMathColour4ub *colour, const QmMathVector2f *st );
unsigned int PlgAddMeshTriangle( QmGfxMesh *mesh, unsigned int x, unsigned int y, unsigned int z );

void PlgUploadMesh( QmGfxMesh *mesh );
void PlgDrawMesh( QmGfxMesh *mesh );
void PlgDrawSubMeshes( QmGfxMesh *mesh, int32_t *firstSubMeshes, int32_t *subMeshes, uint32_t numSubMeshes );

void PlgGenerateVertexTangentBasis( QmGfxMeshVertex *vertices, unsigned int numVertices );
void PlgGenerateTangentBasis( QmGfxMeshVertex *vertices, unsigned int numVertices, const unsigned int *indices, unsigned int numTriangles );

/**
 * Generate cubic coordinates for the given vertices.
 */
void PlgGenerateTextureCoordinates( QmGfxMeshVertex *vertices, unsigned int numVertices, QmMathVector2f textureOffset, QmMathVector2f textureScale );

void PlgGenerateVertexNormals( QmGfxMeshVertex *vertices, unsigned int numVertices, unsigned int *indices, unsigned int numTriangles, bool perFace );

QmMathVector3f PlgGenerateVertexNormal( QmMathVector3f a, QmMathVector3f b, QmMathVector3f c );

/* immediate mode style api */
QmGfxMesh   *PlgImmBegin( QmGfxMeshPrimitive primitive );
unsigned int PlgImmPushVertex( float x, float y, float z );
void         PlgImmNormal( float x, float y, float z );
void         PlgImmColour( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
void         PlgImmTextureCoord( float s, float t );
unsigned int PlgImmPushTriangle( unsigned int x, unsigned int y, unsigned int z );
void         PlgImmSetPrimitiveScale( float scale );
void         PlgImmDraw( void );

unsigned int PlgPushTriangle( QmGfxMesh *mesh, unsigned int x, unsigned int y, unsigned int z );
unsigned int PlgPushVertex3f( QmGfxMesh *mesh, float x, float y, float z );
void         PlgColour4bv( QmGfxMesh *mesh, const QmMathColour4ub *col );

#endif

/////////////////////////////////////////////////////////////////////////////////////
// Vertex Attribute Declarations
// Much of the below is pretty gross and limiting, I know! For the moment, it's just
// to provide the basics and we'll expand on this further down the line so the caller
// can actually set up their own instead rather than using our built-in types.
/////////////////////////////////////////////////////////////////////////////////////

typedef enum QmGfxMeshVertexAttributeType : uint8_t
{
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_POSITION,
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_NORMAL,
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_COLOUR,
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_TANGENT,
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_BITANGENT,

	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_BWEIGHT,
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_BINDICES,

	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_ST0,
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_ST1,
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_ST2,
	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_ST3,

	QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_MAX
} QmGfxMeshVertexAttributeType;

typedef struct QmGfxMeshVertexAttribute
{
	unsigned int location;
	unsigned int stride;
	bool         isEnabled;
	unsigned int offset;
} QmGfxMeshVertexAttribute;

typedef struct QmGfxMeshVertexDescriptor
{
	QmGfxMeshVertexAttribute attributes[ QM_GFX_MESH_VERTEX_ATTRIBUTE_TYPE_MAX ];
	unsigned int             numAttributes;
} QmGfxMeshVertexDescriptor;

void qm_gfx_mesh_set_vertex_layout( QmGfxMesh *mesh );

PL_EXTERN_C_END
