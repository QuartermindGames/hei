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
#pragma once

#include <PL/platform_math.h>
#include <PL/platform_graphics_texture.h>

typedef enum PLMeshPrimitive {
    PL_MESH_LINES,
    PL_MESH_LINE_STIPPLE,
    PL_MESH_LINE_LOOP,
    PL_MESH_LINE_STRIP,
    PL_MESH_POINTS,
    PL_MESH_TRIANGLES,
    PL_MESH_TRIANGLE_STRIP,
    PL_MESH_TRIANGLE_FAN,
    PL_MESH_TRIANGLE_FAN_LINE,
    PL_MESH_QUADS,
    PL_MESH_QUAD_STRIP,

    PL_NUM_PRIMITIVES
} PLMeshPrimitive;

typedef enum PLMeshDrawMode {
    PL_DRAW_STREAM,
    PL_DRAW_STATIC,
    PL_DRAW_DYNAMIC,

    PL_NUM_DRAWMODES
} PLMeshDrawMode;

typedef struct PLVertex {
    PLVector3 position, normal;
    PLVector2 st[1];//[16]; Limit to one UV channel while setting up graphics

    PLColour colour;
} PLVertex;

typedef struct PLTriangle {
    PLVector3 normal;

    unsigned int indices[3];
} PLTriangle;

typedef struct PLMesh {
    PLVertex *vertices;
    PLTriangle *triangles;

    uint16_t *indices;
    unsigned int num_indices;

    unsigned int num_verts;
    unsigned int num_triangles;

    PLMeshPrimitive primitive;
    PLMeshDrawMode mode;

    PLVector3 position, angles;

    struct {
        unsigned int buffers[32];

        PLMeshPrimitive old_primitive;  /* provided for switching between different primitive modes */
    } internal;
} PLMesh;

typedef struct PLAABB PLAABB;

PL_EXTERN_C

PL_EXTERN PLMesh *plCreateMesh(PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts);
PL_EXTERN PLMesh *plCreateMeshInit(PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts, void* indexData, void* vertexData);
PL_EXTERN void plDeleteMesh(PLMesh *mesh);

PL_EXTERN void plDrawBevelledBorder(int x, int y, unsigned int w, unsigned int h);
PL_EXTERN void plDrawEllipse(unsigned int segments, PLVector2 position, float w, float h, PLColour colour);
PL_EXTERN void plDrawRectangle(int x, int y, unsigned int w, unsigned int h, PLColour colour);
PL_EXTERN void plDrawTexturedRectangle(int x, int y, int w, int h, PLTexture *texture);
PL_EXTERN void plDrawFilledRectangle(PLRectangle2D rect);
PL_EXTERN void plDrawTriangle(int x, int y, unsigned int w, unsigned int h);

PL_EXTERN void plClearMesh(PLMesh *mesh);
PL_EXTERN void plSetMeshTrianglePosition(PLMesh *mesh, unsigned int *index, uint16_t x, uint16_t y, uint16_t z);
PL_EXTERN void plSetMeshVertexPosition(PLMesh *mesh, unsigned int index, PLVector3 vector);
PL_EXTERN void plSetMeshVertexNormal(PLMesh *mesh, unsigned int index, PLVector3 vector);
PL_EXTERN void plSetMeshVertexST(PLMesh *mesh, unsigned int index, float s, float t);
PL_EXTERN void plSetMeshVertexColour(PLMesh *mesh, unsigned int index, PLColour colour);
PL_EXTERN void plSetMeshUniformColour(PLMesh *mesh, PLColour colour);
PL_EXTERN void plUploadMesh(PLMesh *mesh);

PL_EXTERN void plDrawMesh(PLMesh *mesh);

PL_EXTERN PLAABB plCalculateMeshAABB(PLMesh *mesh);

PL_EXTERN PLVector3 plGenerateVertexNormal(PLVector3 a, PLVector3 b, PLVector3 c);

PL_EXTERN void plGenerateMeshNormals(PLMesh *mesh);

PL_EXTERN_C_END
