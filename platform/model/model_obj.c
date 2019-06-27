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

static PLMesh *LoadMesh(FILE *fp) {
    unsigned int num_vertices = 0;
    unsigned int max_vertices = 4096;
    PLVertex *vertices = pl_malloc(sizeof(PLVertex) * max_vertices);

    unsigned int num_triangles = 0;

    char tk[256];
    while(fgets(tk, sizeof(tk), fp) != NULL) {
        if(tk[0] == '#') { /* comment */
            continue;
        } else if(tk[0] == 'v') { /* vertex */
            num_vertices++;
            if(num_vertices > max_vertices) {
                /* todo: realloc */
            }

            PLVertex *vertex = &vertices[num_vertices];
            unsigned int n = sscanf(&tk[2], "%f %f %f", &vertex->position.x, &vertex->position.y, &vertex->position.z);
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

PLModel *plLoadObjModel(const char *path) {
    FILE *fp = fopen(path, "rb");
    if(fp == NULL) {
        ReportError(PL_RESULT_FILEREAD, plGetResultString(PL_RESULT_FILEREAD));
        return NULL;
    }

    char tk[256];
    while(fgets(tk, sizeof(tk), fp) != NULL) {
        if(tk[0] == '#') { /* comment */
            continue;
        }

        if(tk[0] == 'o') { /* object name */

        }

        if(tk[0] == 'g') { /* group */

        }
    }

    return NULL;
}
