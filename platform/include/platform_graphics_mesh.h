
#pragma once

#if 0
typedef struct PLDraw {
    PLuint id;

    PLVertex *vertices;                            // Array of vertices for the object.

    PLuint numverts;                            // Number of vertices.
    PLuint numtriangles;                        // Number of triangles.

    PLuint8 *indices;                                // List of indices.

    PLPrimitive primitive, primitive_restore;        // Type of primitive, and primitive to restore to.
} PLDraw;
#endif

#ifdef __cplusplus

namespace pl {
    namespace graphics {

        typedef enum Primitive {
            PRIMITIVE_LINES,
            PRIMITIVE_LINE_STRIP,
            PRIMITIVE_POINTS,
            PRIMITIVE_TRIANGLES,
            PRIMITIVE_TRIANGLE_STRIP,
            PRIMITIVE_TRIANGLE_FAN,
            PRIMITIVE_TRIANGLE_FAN_LINE,
            PRIMITIVE_QUADS
        } Primitive;

        typedef struct Vertex {
            math::Vector3D position, normal;
            math::Vector2D ST[16];

            math::Colour colour;
        } Vertex;

        typedef struct Triangle {
            math::Vector3D normal;

            unsigned int indices[3];
        } Triangle;

        typedef enum MeshMode {
#if 0
#if defined (PL_MODE_OPENGL)
            VL_DRAWMODE_STATIC = GL_STATIC_DRAW,
            VL_DRAWMODE_DYNAMIC = GL_DYNAMIC_DRAW,
#else
            VL_DRAWMODE_STATIC,
            VL_DRAWMODE_DYNAMIC,
#endif
#else
            MESH_DYNAMIC,
            MESH_STATIC
#endif
        } MeshMode;

        class Mesh {

        };

        class StaticMesh : Mesh {};
        class DynamicMesh : Mesh {};

    }
}

#endif
