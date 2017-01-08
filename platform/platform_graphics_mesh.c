
#include "platform_graphics.h"

typedef struct PLTranslatePrimitive {
    PLPrimitive mode;

    PLuint target;

    const PLchar *name;
} PLTranslatePrimitive;

PLTranslatePrimitive _pl_primitives[] = {
#if defined (PL_MODE_OPENGL) || (VL_MODE_OPENGL_CORE)
        {PL_PRIMITIVE_LINES, GL_LINES, "LINES"},
        {PL_PRIMITIVE_POINTS, GL_POINTS, "POINTS"},
        {PL_PRIMITIVE_TRIANGLES, GL_TRIANGLES, "TRIANGLES"},
        {PL_PRIMITIVE_TRIANGLE_FAN, GL_TRIANGLE_FAN, "TRIANGLE_FAN"},
        {PL_PRIMITIVE_TRIANGLE_FAN_LINE, GL_LINES, "TRIANGLE_FAN_LINE"},
        {PL_PRIMITIVE_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP"},
        {PL_PRIMITIVE_QUADS, GL_QUADS, "QUADS"}
#elif defined (VL_MODE_GLIDE)
{ PL_PRIMITIVE_LINES,					GR_LINES,			"LINES" },
    { PL_PRIMITIVE_LINE_STRIP,				GR_LINE_STRIP,		"LINE_STRIP" },
    { PL_PRIMITIVE_POINTS,					GR_POINTS,			"POINTS" },
    { PL_PRIMITIVE_TRIANGLES,				GR_TRIANGLES,		"TRIANGLES" },
    { PL_PRIMITIVE_TRIANGLE_FAN,			GR_TRIANGLE_FAN,	"TRIANGLE_FAN" },
    { PL_PRIMITIVE_TRIANGLE_FAN_LINE,		GR_LINES,			"TRIANGLE_FAN_LINE" },
    { PL_PRIMITIVE_TRIANGLE_STRIP,			GR_TRIANGLE_STRIP,	"TRIANGLE_STRIP" },
    { PL_PRIMITIVE_QUADS,					0,					"QUADS" }
#elif defined (VL_MODE_DIRECT3D)
#elif defined (VL_MODE_VULKAN)
    { PL_PRIMITIVE_LINES,					VK_PRIMITIVE_TOPOLOGY_LINE_LIST,		"LINES" },
    { PL_PRIMITIVE_POINTS,					VK_PRIMITIVE_TOPOLOGY_POINT_LIST,		"POINTS" },
    { PL_PRIMITIVE_TRIANGLES,				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,	"TRIANGLES" },
    { PL_PRIMITIVE_TRIANGLE_FAN,			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,		"TRIANGLE_FAN" },
    { PL_PRIMITIVE_TRIANGLE_FAN_LINE,		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,		"TRIANGLE_FAN_LINE" },
    { PL_PRIMITIVE_TRIANGLE_STRIP,			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,	"TRIANGLE_STRIP" },
    { PL_PRIMITIVE_QUADS,					0,										"QUADS" }
#else
    { 0 }
#endif
};

PLuint _plTranslatePrimitiveMode(PLPrimitive mode) {
    if(mode == PL_PRIMITIVE_IGNORE) {
        return 0;
    }

    for (PLint i = 0; i < plArrayElements(_pl_primitives); i++)
        if (mode == _pl_primitives[i].mode)
            return _pl_primitives[i].target;

    // Hacky, but just return initial otherwise.
    return _pl_primitives[0].target;
}

#if 0 // legacy







PLDraw *plCreateMesh(void) {
    PLDraw *draw = new PLDraw();

#if defined(PL_MODE_OPENGL)
    glGenVertexArrays(1, &draw->id);
#endif

    return draw;
}

void plDeleteDraw(PLDraw *draw) {
#if defined(PL_MODE_OPENGL)
    glDeleteVertexArrays(1, &draw->id);
#endif

    delete draw;
}

void _plDrawArrays(PLPrimitive mode, PLuint first, PLuint count) {
    if ((count == 0) || (first > count)) {
        return;
    }
#if defined(PL_MODE_OPENGL)
    glDrawArrays(_plTranslatePrimitiveMode(mode), first, count);
#endif
}

void _plDrawElements(PLPrimitive mode, PLuint count, PLuint type, const PLvoid *indices) {
    if ((count == 0) || !indices) {
        return;
    }
#if defined(PL_MODE_OPENGL)
    glDrawElements(_plTranslatePrimitiveMode(mode), count, type, indices);
#endif
}

void plBeginDraw(PLDraw *draw) {

}

void plEndDraw(PLDraw *draw) {

}

#endif

unsigned int _plTranslateDrawMode(PLDrawMode mode) {
#if defined(PL_MODE_OPENGL)
    if(mode == PL_DRAW_DYNAMIC) {
        return GL_DYNAMIC_DRAW;
    } else if(mode == PL_DRAW_STATIC) {
        return GL_STATIC_DRAW;
    }

    return 0;
#else
    return mode;
#endif
}

PLMesh *plCreateMesh(PLPrimitive primitive, PLDrawMode mode, PLuint num_tris, PLuint num_verts) {
    plFunctionStart();

    if(primitive == PL_PRIMITIVE_IGNORE) {
        plSetError("Primitive cannot be set to 'ignore' upon creation!\n");
        return NULL;
    } else if((primitive < PL_PRIMITIVE_IGNORE) || (primitive > PL_NUM_PRIMITIVES)) {
        plSetError("Invalid primitive! (%d)\n", primitive);
        return NULL;
    }

    PLuint umode = _plTranslateDrawMode(mode);
    if(!umode) {
        plSetError("Invalid/unsupported draw mode! (%d)\n", mode);
        return NULL;
    }

    PLMesh *mesh = (PLMesh*)calloc(1, sizeof(PLMesh));
    if(!mesh) {
        plSetError("Failed to create mesh!\n");
        return NULL;
    }

    memset(mesh, 0, sizeof(PLMesh));
    mesh->primitive = primitive;
    mesh->numtriangles = num_tris;
    mesh->numverts = num_verts;
    mesh->mode = mode;

#if defined(PL_MODE_OPENGL)
   // glGenBuffers()
#endif
}

void plDeleteMesh(PLMesh *mesh) {
    if(!mesh) {
        return;
    }

    if(mesh->indices) {
        free(mesh->indices);
    }
    if(mesh->vertices) {
        free(mesh->vertices);
    }
    if(mesh->triangles) {
        free(mesh->triangles);
    }
    free(mesh);
}

void plDrawVertexNormals(PLMesh *mesh) {
    if (mesh->primitive == PL_PRIMITIVE_LINES) {
        return;
    }

#if 0
    PLVector3D endpos;
    for (PLuint i = 0; i < mesh->numverts; i++) {
        endpos = (mesh->vertices[i].normal * 2.0f) + mesh->vertices[i].position;
        //plDrawLine blah
    }
#endif
}
