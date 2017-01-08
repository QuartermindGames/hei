
#pragma once

typedef enum PLPrimitive {
    PL_PRIMITIVE_IGNORE,

    PL_PRIMITIVE_LINES,
    PL_PRIMITIVE_LINE_STRIP,
    PL_PRIMITIVE_POINTS,
    PL_PRIMITIVE_TRIANGLES,
    PL_PRIMITIVE_TRIANGLE_STRIP,
    PL_PRIMITIVE_TRIANGLE_FAN,
    PL_PRIMITIVE_TRIANGLE_FAN_LINE,
    PL_PRIMITIVE_QUADS,

    PL_NUM_PRIMITIVES
} PLPrimitive;

typedef enum PLDrawMode {
    PL_DRAW_DYNAMIC,
    PL_DRAW_STATIC,

    PL_NUM_DRAWMODES
} PLDrawMode;

typedef struct PLVertex {
    PLVector3D position, normal;
    PLVector2D st[16];

    PLColour colour;
} PLVertex;

typedef struct PLTriangle {
    PLVector3D normal;

    PLuint indices[3];
} PLTriangle;

typedef struct PLMesh {
    PLuint id;

    PLVertex *vertices;
    PLTriangle *triangles;
    PLuint8 *indices;

    PLuint numverts;
    PLuint numtriangles;

    PLPrimitive primitive, primitive_restore;
    PLDrawMode mode;
} PLMesh;

PL_EXTERN_C

PL_EXTERN PLMesh *plCreateMesh(PLPrimitive primitive, PLDrawMode mode, PLuint num_tris, PLuint num_verts);
PL_EXTERN void plDeleteMesh(PLMesh *mesh);

PL_EXTERN_C_END
