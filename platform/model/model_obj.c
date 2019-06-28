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

#include "model_private.h"

/* Obj Static Model Format
 * This is and shall probably remain super duper basic;
 * curves, materials and other features of the Obj format
 * are unsupported.
 * */

typedef struct ObjHandle {
    struct {
        PLVector3       *vn_list;
        unsigned int    vn_size;
        PLVector3       *v_list;
        unsigned int    v_size;
        PLVector2       *vt_list;
        unsigned int    vt_size;
        unsigned int    num_vertices;
    } vertex_data;

    unsigned int num_faces;
} ObjHandle;

static void ReadGeomVertex(ObjHandle *obj, const char *str) {

}

static PLMesh *LoadMesh(ObjHandle *obj, FILE *fp) {
    /* add support for obj they said,
     * it'd be easy they said... */

    obj->vertex_data.num_vertices = 0;
    obj->vertex_data.vt_list = pl_calloc(1, sizeof(PLVector2));
    obj->vertex_data.vt_size = 1;
    obj->vertex_data.vn_list = pl_calloc(1, sizeof(PLVector3));
    obj->vertex_data.vn_size = 1;
    obj->vertex_data.v_list = pl_calloc(1, sizeof(PLVector3));
    obj->vertex_data.v_size = 1;

    char tk[256];
    while(fgets(tk, sizeof(tk), fp) != NULL) {
        if(tk[0] == '#') { /* comment */
            continue;
        } else if(tk[0] == 'v') { /* vertex */
            obj->vertex_data.num_vertices++;
            PLVertex *vertex = &vertices[num_vertices];
            unsigned int n = sscanf(&tk[2], "%f %f %f", &vpositions[.x, &vertex->position.y, &vertex->position.z);
            if(n < 3) {
                ModelLog("Invalid vertex position, less than 3 coords!\n\"%s\"\n", tk);
            } else if(n > 3) {
                ModelLog("Ignoring fourth position parameter, unsupported!\n\"%s\"\n", tk);
            }
            continue;
        } else if(tk[0] == 'v' && tk[1] == 'n') { /* vertex normal */
            PLVertex *vertex = &vertices[num_vertices];
            if(sscanf(&tk[2], "%f %f %f", &vertex->normal.x, &vertex->normal.y, &vertex->normal.z) < 3) {
                ModelLog("Invalid vertex normal, less than 3 coords!\n\"%s\"\n", tk);
            }
            continue;
        } else if(tk[0] == 'v' && tk[1] == 't') { /* uv coord */
            PLVertex *vertex = &vertices[num_vertices];
            unsigned int n = sscanf(&tk[2], "%f %f", &vertex->st[0].x, &vertex->st[0].y);
            if(n < 2) {
                ModelLog("Invalid vertex uv, less than 2 coords!\n\"%s\"\n", tk);
            } else if(n > 2) {
                ModelLog("Ignoring third uv parameter, unsupported!\n\"%s\"\n", tk);
            }
            continue;
        }
    }

    PLMesh *mesh = plCreateMesh(PL_MESH_TRIANGLES, PL_DRAW_STATIC, num_triangles, num_vertices);
    if(mesh == NULL) {
        free(vertices);
        return NULL;
    }

    memcpy(mesh->vertices, vertices, sizeof(PLVertex) * num_vertices);
    free(vertices);
}

static void FreeObjHandle(ObjHandle *obj) {
    free(obj->vertex_data.v_list);
    free(obj->vertex_data.vn_list);
    free(obj->vertex_data.vt_list);
    free(obj);
}

PLModel *plLoadObjModel(const char *path) {
    FILE *fp = fopen(path, "rb");
    if(fp == NULL) {
        ReportError(PL_RESULT_FILEREAD, plGetResultString(PL_RESULT_FILEREAD));
        return NULL;
    }

    ObjHandle *obj = pl_malloc(sizeof(ObjHandle));

    char tk[256];
    while(fgets(tk, sizeof(tk), fp) != NULL) {
        if(tk[0] == '\0' || tk[0] == '#' || tk[0] == 'o') {
            continue;
        }

        ModelLog("Unknown/unsupported parameter '%s', ignoring!\n", tk[0]);
    }

    fclose(fp);

    /* right we're finally done, time to see what we hauled... */

    if(obj->vertex_data.num_vertices == 0) {
        ReportError(PL_RESULT_FAIL, "0 vertices");
        FreeObjHandle(obj);
        return NULL;
    }

    return NULL;
}

bool plWriteObjModel(PLModel *model, const char *path) {
    if(model == NULL) {
        ReportBasicError(PL_RESULT_INVALID_PARM1);
        return false;
    }

    FILE *fp = fopen(path, "w");
    if(fp == NULL) {
        ReportBasicError(PL_RESULT_FILEWRITE);
        return false;
    }

    fprintf(fp, "# generated by 'platform' lib (https://github.com/TalonBraveInfo/platform)\n\n");
    if(model->type == PL_MODELTYPE_SKELETAL) {
        ModelLog("Model is of type skeletal; Obj only supports static models so skeleton will be discarded...\n");
    }

    // todo...
}
