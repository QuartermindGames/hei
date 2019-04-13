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

#include <PL/platform_console.h>
#include <PL/platform_mesh.h>

#include "../graphics/graphics_private.h"

void plGenerateMeshNormals(PLMesh *mesh) {
    plAssert(mesh);

#if 0
#if 0 // per face...
    for (unsigned int j = 0; j < mesh->num_triangles; j++) {
        mesh->triangles[j].normal = plGenerateVertexNormal(
                mesh->vertices[mesh->triangles[j].indices[0]].position,
                mesh->vertices[mesh->triangles[j].indices[1]].position,
                mesh->vertices[mesh->triangles[j].indices[2]].position
        );
    }
#else // per vertex... todo
    for (PLVertex *vertex = &mesh->vertices[0]; vertex; ++vertex) {
            for (PLTriangle *triangle = &mesh->triangles[0]; triangle; ++triangle) {

            }
        }
#endif
#endif
}

PLVector3 plGenerateVertexNormal(PLVector3 a, PLVector3 b, PLVector3 c) {
#if 0
    PLVector3D x = c - b;
    PLVector3D y = a - b;
    return x.CrossProduct(y).Normalize();
#else
    return plVector3Normalize(
            plVector3CrossProduct(
                    PLVector3(
                            c.x - b.x, c.y - b.y, c.z - b.z
                    ),
                    PLVector3(
                            a.x - b.x, a.y - b.y, a.z - b.z
                    )
            )
    );
#endif
}

/* software implementation of gouraud shading */
void plApplyMeshLighting(PLMesh *mesh, const PLLight *light, PLVector3 position) {
    PLVector3 distvec = position;
    plSubtractVector3(&distvec, light->position);
    float distance = (light->colour.a - plVector3Length(distvec)) / 100.f;
    for(unsigned int i = 0; i < mesh->num_verts; i++) {
        PLVector3 normal = mesh->vertices[i].normal;
        float angle = (distance * ((normal.x * distvec.x) + (normal.y * distvec.y) + (normal.z * distvec.z)));
        if(angle < 0) {
            plClearColour(&mesh->vertices[i].colour);
        } else {
            mesh->vertices[i].colour.r = light->colour.r * plFloatToByte(angle);
            mesh->vertices[i].colour.g = light->colour.g * plFloatToByte(angle);
            mesh->vertices[i].colour.b = light->colour.b * plFloatToByte(angle);
        }
        //GfxLog("light angle is %f\n", angle);
    }

#if 0
        /*
        x = Object->Vertices_normalStat[count].x;
        y = Object->Vertices_normalStat[count].y;
        z = Object->Vertices_normalStat[count].z;

        angle = (LightDist*((x * Object->Spotlight.x) + (y * Object->Spotlight.y) + (z * Object->Spotlight.z) ));
        if (angle<0 )
        {
        Object->Vertices_screen[count].r = 0;
        Object->Vertices_screen[count].b = 0;
        Object->Vertices_screen[count].g = 0;
        }
        else
        {
        Object->Vertices_screen[count].r = Object->Vertices_local[count].r * angle;
        Object->Vertices_screen[count].b = Object->Vertices_local[count].b * angle;
        Object->Vertices_screen[count].g = Object->Vertices_local[count].g * angle;
        }
        */
#endif
}

PLMesh *plCreateMesh(PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts) {
    return plCreateMeshInit(primitive, mode, num_tris, num_verts, NULL, NULL);
}

PLMesh *plCreateMeshInit(PLMeshPrimitive primitive, PLMeshDrawMode mode, unsigned int num_tris, unsigned int num_verts, void* indexData, void* vertexData) {
    plAssert(num_verts);

    PLMesh *mesh = (PLMesh*)pl_calloc(1, sizeof(PLMesh));
    if(mesh == NULL) {
        return NULL;
    }

    mesh->primitive     = primitive;
    mesh->num_triangles = num_tris;
    mesh->num_verts     = num_verts;
    mesh->mode          = mode;

    mesh->internal.old_primitive = mesh->primitive;

    if(num_tris > 0) {
        mesh->triangles = (PLTriangle*)pl_calloc(num_tris, sizeof(PLTriangle));
        if(!mesh->triangles) {
            plDeleteMesh(mesh);
            return NULL;
        }
        if(vertexData != NULL){
            memcpy(mesh->triangles, vertexData, num_tris * sizeof(PLTriangle));
        }

        if(mesh->primitive == PL_MESH_TRIANGLES) {
            mesh->num_indices = num_tris * 3;
            if((mesh->indices = pl_calloc(mesh->num_indices, sizeof(uint16_t))) == NULL) {
                plDeleteMesh(mesh);
                return NULL;
            }
            if(indexData != NULL){
                memcpy(mesh->indices, indexData, mesh->num_indices * sizeof(uint16_t));
            }
        }
    }

    mesh->vertices = (PLVertex*)pl_calloc(num_verts, sizeof(PLVertex));
    if(mesh->vertices == NULL) {
        plDeleteMesh(mesh);
        return NULL;
    }

    CallGfxFunction(CreateMeshPOST, mesh);

    return mesh;
}

void plDeleteMesh(PLMesh *mesh) {
    if(mesh == NULL) {
        return;
    }

    CallGfxFunction(DeleteMesh, mesh);

    free(mesh->vertices);
    free(mesh->triangles);
    free(mesh->indices);
    free(mesh);
}

void plClearMesh(PLMesh *mesh) {
    // Reset the data contained by the mesh, if we're going to begin a new draw.
    memset(mesh->vertices, 0, sizeof(PLVertex) * mesh->num_verts);
    memset(mesh->triangles, 0, sizeof(PLTriangle) * mesh->num_triangles);
}

void plSetMeshTrianglePosition(PLMesh *mesh, unsigned int *index, uint16_t x, uint16_t y, uint16_t z) {
    plAssert(*index < mesh->num_indices);
    mesh->indices[(*index)++] = x;
    mesh->indices[(*index)++] = y;
    mesh->indices[(*index)++] = z;
}

void plSetMeshVertexPosition(PLMesh *mesh, unsigned int index, PLVector3 vector) {
    plAssert(index < mesh->num_verts);
    mesh->vertices[index].position = vector;
}

#if 0
void plSetMeshVertexPosition3fv(PLMesh *mesh, unsigned int index, unsigned int size, const float *v) {
    size += index;
    if(size > mesh->num_verts) {
        size -= (size - mesh->num_verts);
    }

    for(unsigned int i = index; i < size; i++) {
        mesh->vertices[i].position.x = v[0];
        mesh->vertices[i].position.y = v[1];
        mesh->vertices[i].position.z = v[2];
    }
}
#endif

void plSetMeshVertexNormal(PLMesh *mesh, unsigned int index, PLVector3 vector) {
    plAssert(index < mesh->num_verts);
    mesh->vertices[index].normal = vector;
}

void plSetMeshVertexST(PLMesh *mesh, unsigned int index, float s, float t) {
    plAssert(index < mesh->num_verts);
    mesh->vertices[index].st[0] = PLVector2(s, t);
}

#if 0
void plSetMeshVertexSTv(PLMesh *mesh, uint8_t unit, unsigned int index, unsigned int size, const float *st) {
    size += index;
    if(size > mesh->num_verts) {
        size -= (size - mesh->num_verts);
    }

    for(unsigned int i = index; i < size; i++) {
        mesh->vertices[i].st[unit].x = st[0];
        mesh->vertices[i].st[unit].y = st[1];
    }
}
#endif

void plSetMeshVertexColour(PLMesh *mesh, unsigned int index, PLColour colour) {
    plAssert(index < mesh->num_verts);
    mesh->vertices[index].colour = colour;
}

void plSetMeshUniformColour(PLMesh *mesh, PLColour colour) {
    for(unsigned int i = 0; i < mesh->num_verts; ++i) {
        mesh->vertices[i].colour = colour;
    }
}

void plUploadMesh(PLMesh *mesh) {
    CallGfxFunction(UploadMesh, mesh);
}

void plDrawMesh(PLMesh *mesh) {
    CallGfxFunction(DrawMesh, mesh);
}

PLAABB plCalculateMeshAABB(PLMesh *mesh) {
    static PLAABB bounds;
    memset(&bounds, 0, sizeof(PLAABB));
    for(unsigned int i = 0; i < mesh->num_verts; ++i) {
        if(bounds.maxs.x < mesh->vertices[i].position.x) {
            bounds.maxs.x = mesh->vertices[i].position.x;
        }

        if(bounds.mins.x > mesh->vertices[i].position.x) {
            bounds.mins.x = mesh->vertices[i].position.x;
        }

        if(bounds.maxs.y < mesh->vertices[i].position.y) {
            bounds.maxs.y = mesh->vertices[i].position.y;
        }

        if(bounds.mins.y > mesh->vertices[i].position.y) {
            bounds.mins.y = mesh->vertices[i].position.y;
        }

        if(bounds.maxs.z < mesh->vertices[i].position.z) {
            bounds.maxs.z = mesh->vertices[i].position.z;
        }

        if(bounds.mins.z > mesh->vertices[i].position.z) {
            bounds.mins.z = mesh->vertices[i].position.z;
        }
    }

    return bounds;
}

/* primitive types */

#if defined(PL_USE_GRAPHICS)    /* todo: move these... */

void plDrawRaisedBox(int x, int y, unsigned int w, unsigned int h) {
    static PLMesh *mesh = NULL;
    if(mesh == NULL) {
        if((mesh = plCreateMesh(
                PL_MESH_LINES,
                PL_DRAW_DYNAMIC,
                0, 4
        )) == NULL) {
            return;
        }
    }
}

void plDrawBevelledBorder(int x, int y, unsigned int w, unsigned int h) {
    static PLMesh *mesh = NULL;
    if(mesh == NULL) {
        if((mesh = plCreateMesh(
                PL_MESH_LINES,
                PL_DRAW_DYNAMIC,
                0, 16
        )) == NULL) {
            return;
        }
    }

    plClearMesh(mesh);

    plSetMeshVertexPosition(mesh, 0, PLVector3(x, y, 0));
    plSetMeshVertexPosition(mesh, 1, PLVector3(x + w, y, 0));
    plSetMeshVertexPosition(mesh, 2, PLVector3(x, y, 0));
    plSetMeshVertexPosition(mesh, 3, PLVector3(x, y + h, 0));

    plSetMeshVertexColour(mesh, 0, PLColourRGB(127, 127, 127));
    plSetMeshVertexColour(mesh, 1, PLColourRGB(127, 127, 127));
    plSetMeshVertexColour(mesh, 2, PLColourRGB(127, 127, 127));
    plSetMeshVertexColour(mesh, 3, PLColourRGB(127, 127, 127));

    /************************/

    plSetMeshVertexPosition(mesh, 4, PLVector3(x, y + h, 0));
    plSetMeshVertexPosition(mesh, 5, PLVector3(x + w, y + h, 0));
    plSetMeshVertexPosition(mesh, 6, PLVector3(x + w, y + h, 0));
    plSetMeshVertexPosition(mesh, 7, PLVector3(x + w, y, 0));

    plSetMeshVertexColour(mesh, 4, PLColourRGB(255, 255, 255));
    plSetMeshVertexColour(mesh, 5, PLColourRGB(255, 255, 255));
    plSetMeshVertexColour(mesh, 6, PLColourRGB(255, 255, 255));
    plSetMeshVertexColour(mesh, 7, PLColourRGB(255, 255, 255));

    /************************/

    plSetMeshVertexPosition(mesh, 8, PLVector3(x + 1, y + 1, 0));
    plSetMeshVertexPosition(mesh, 9, PLVector3(x + w - 1, y + 1, 0));
    plSetMeshVertexPosition(mesh, 10, PLVector3(x + 1, y + 1, 0));
    plSetMeshVertexPosition(mesh, 11, PLVector3(x + 1, y + h - 1, 0));

    plSetMeshVertexColour(mesh, 8, PLColourRGB(63, 63, 63));
    plSetMeshVertexColour(mesh, 9, PLColourRGB(63, 63, 63));
    plSetMeshVertexColour(mesh, 10, PLColourRGB(63, 63, 63));
    plSetMeshVertexColour(mesh, 11, PLColourRGB(63, 63, 63));

    /************************/

    plSetMeshVertexPosition(mesh, 12, PLVector3(x + 1, y + h - 1, 0));
    plSetMeshVertexPosition(mesh, 13, PLVector3(x + w - 1, y + h - 1, 0));
    plSetMeshVertexPosition(mesh, 14, PLVector3(x + w - 1, y + h - 1, 0));
    plSetMeshVertexPosition(mesh, 15, PLVector3(x + w - 1, y + 1, 0));

    plSetMeshVertexColour(mesh, 12, PLColourRGB(63, 63, 63));
    plSetMeshVertexColour(mesh, 13, PLColourRGB(63, 63, 63));
    plSetMeshVertexColour(mesh, 14, PLColourRGB(63, 63, 63));
    plSetMeshVertexColour(mesh, 15, PLColourRGB(63, 63, 63));

    plSetNamedShaderUniformMatrix4x4(NULL, "pl_model", plMatrix4x4Identity(), false);
    plUploadMesh(mesh);
    plDrawMesh(mesh);
}

void plDrawCube(PLVector3 position, float size) {
    /* todo */
}

void plDrawSphere() {}  // todo

void plDrawEllipse(unsigned int segments, PLVector2 position, float w, float h, PLColour colour) {
    static unsigned int last_num_segments = 0;
    static PLMesh *mesh = NULL;
    if(last_num_segments != segments) {
        plDeleteMesh(mesh);
        mesh = NULL;
    }

    if(mesh == NULL) {
        if((mesh = plCreateMesh(
                PL_MESH_TRIANGLE_FAN,
                PL_DRAW_DYNAMIC,
                0, segments
        )) == NULL) {
            return;
        }
    }

    plSetMeshUniformColour(mesh, colour);
    for(unsigned int i = 0, pos = 0; i < 360; i += (360 / segments)) {
        if(pos >= segments) {
            break;
        }

        plSetMeshVertexPosition(mesh, pos++, PLVector3(
                (position.x + w) + cosf(plDegreesToRadians(i)) * w,
                (position.y + h) + sinf(plDegreesToRadians(i)) * h, 0));
    }

    plSetNamedShaderUniformMatrix4x4(NULL, "pl_model", plMatrix4x4Identity(), false);
    plUploadMesh(mesh);
    plDrawMesh(mesh);
}

void plDrawTexturedRectangle(int x, int y, int w, int h, PLTexture *texture) {
    static PLMesh *mesh = NULL;
    if(mesh == NULL) {
        if((mesh = plCreateMesh(
                PL_MESH_TRIANGLE_STRIP,
                PL_DRAW_DYNAMIC,
                2, 4
        )) == NULL) {
            return;
        }
    }

    plClearMesh(mesh);

    plSetMeshVertexPosition(mesh, 0, PLVector3(x, y, 0));
    plSetMeshVertexPosition(mesh, 1, PLVector3(x, y + h, 0));
    plSetMeshVertexPosition(mesh, 2, PLVector3(x + w, y, 0));
    plSetMeshVertexPosition(mesh, 3, PLVector3(x + w, y + h, 0));

    plSetMeshUniformColour(mesh, PLColourRGB(255, 255, 255));

    plSetMeshVertexST(mesh, 0, 0, 0);
    plSetMeshVertexST(mesh, 1, 0, 1);
    plSetMeshVertexST(mesh, 2, 1, 0);
    plSetMeshVertexST(mesh, 3, 1, 1);

    plSetTexture(texture, 0);

    plSetNamedShaderUniformMatrix4x4(NULL, "pl_model", plMatrix4x4Identity(), false);
    plUploadMesh(mesh);
    plDrawMesh(mesh);

    plSetTexture(NULL, 0);
}

void plDrawRectangle(int x, int y, unsigned int w, unsigned int h, PLColour colour) {
    static PLMesh *mesh = NULL;
    if(mesh == NULL) {
        if((mesh = plCreateMesh(
                PL_MESH_LINE_LOOP,
                PL_DRAW_DYNAMIC,
                0, 4
        )) == NULL) {
            return;
        }
    }

    plClearMesh(mesh);

    plSetMeshVertexPosition(mesh, 0, PLVector3(x, y, 0));
    plSetMeshVertexPosition(mesh, 1, PLVector3(x + w, y, 0));
    plSetMeshVertexPosition(mesh, 2, PLVector3(x + w, y + h, 0));
    plSetMeshVertexPosition(mesh, 3, PLVector3(x, y + h, 0));

    plSetMeshUniformColour(mesh, colour);

    plSetNamedShaderUniformMatrix4x4(NULL, "pl_model", plMatrix4x4Identity(), false);
    plUploadMesh(mesh);
    plDrawMesh(mesh);
}

void plDrawFilledRectangle(PLRectangle2D rect) {
    static PLMesh *mesh = NULL;
    if(mesh == NULL) {
        if((mesh = plCreateMesh(
                PL_MESH_TRIANGLE_STRIP,
                PL_DRAW_DYNAMIC,
                2, 4
        )) == NULL) {
            return;
        }
    }

    plClearMesh(mesh);

    plSetMeshVertexPosition(mesh, 0, PLVector3(rect.xy.x, rect.xy.y + rect.wh.y, 0));
    plSetMeshVertexPosition(mesh, 1, PLVector3(rect.xy.x, rect.xy.y, 0));
    plSetMeshVertexPosition(mesh, 2, PLVector3(rect.xy.x + rect.wh.x, rect.xy.y + rect.wh.y, 0));
    plSetMeshVertexPosition(mesh, 3, PLVector3(rect.xy.x + rect.wh.x, rect.xy.y, 0));

    plSetMeshVertexColour(mesh, 0, rect.ll);
    plSetMeshVertexColour(mesh, 1, rect.ul);
    plSetMeshVertexColour(mesh, 2, rect.lr);
    plSetMeshVertexColour(mesh, 3, rect.ur);

    plSetNamedShaderUniformMatrix4x4(NULL, "pl_model", plMatrix4x4Identity(), false);
    plUploadMesh(mesh);
    plDrawMesh(mesh);
}

void plDrawTriangle(int x, int y, unsigned int w, unsigned int h) {
    static PLMesh *mesh = NULL;
    if (mesh == NULL) {
        if((mesh = plCreateMesh(
                PL_MESH_TRIANGLE_FAN,
                PL_DRAW_DYNAMIC,
                1, 3
        )) == NULL) {
            return;
        }
    }

    plClearMesh(mesh);

    plSetMeshVertexPosition(mesh, 0, PLVector3(x, y + h, 0));
    plSetMeshVertexPosition(mesh, 1, PLVector3(x + w / 2, x, 0));
    plSetMeshVertexPosition(mesh, 2, PLVector3(x + w, y + h, 0));

    plSetMeshVertexColour(mesh, 0, PLColour(255, 0, 0, 255));
    plSetMeshVertexColour(mesh, 1, PLColour(0, 255, 0, 255));
    plSetMeshVertexColour(mesh, 2, PLColour(0, 0, 255, 255));

    //plSetMeshUniformColour(mesh, PLColour(255, 0, 0, 255));

    plSetNamedShaderUniformMatrix4x4(NULL, "pl_model", plMatrix4x4Identity(), false);
    plUploadMesh(mesh);
    plDrawMesh(mesh);
}

#endif
