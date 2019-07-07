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

#include "filesystem_private.h"
#include "model_private.h"

/************************************************************/
/* MTL Format */

/* todo */

/************************************************************/
/* Obj Static Model Format */

typedef struct ObjVectorLst {
    PLVector3 v;
    struct ObjVectorLst *next;
} ObjVectorLst;

typedef struct ObjFaceLst {
    PLVertex *vertices;
    unsigned int num_vertices;
    char mtl_name[64];
    struct ObjFaceLst *next;
} ObjFaceLst;

typedef struct ObjHandle {
    ObjVectorLst *vertex_normals;
    ObjVectorLst *vertex_positions;
    ObjVectorLst *vertex_tex_coords;
    ObjFaceLst *faces;
    char mtllib_path[PL_SYSTEM_MAX_PATH];
} ObjHandle;

static void FreeObjHandle(ObjHandle *obj) {
    free(obj);
}

static ObjVectorLst *GetVectorIndex(ObjVectorLst *start, unsigned int idx) {
    ObjVectorLst *cur = start;
    for(unsigned int i = 0; i < idx - 1; ++i) {
        if(cur->next == NULL) {
            ModelLog("Invalid vector index (%d)!\n", i);
            return NULL;
        }

        cur = cur->next;
    }

    return cur;
}

PLModel *plLoadObjModel(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        ReportError(PL_RESULT_FILEREAD, plGetResultString(PL_RESULT_FILEREAD));
        return NULL;
    }

    ObjHandle *obj = pl_malloc(sizeof(ObjHandle));

    ObjVectorLst **cur_v = &(obj->vertex_positions);
    ObjVectorLst **cur_vn = &(obj->vertex_normals);
    ObjVectorLst **cur_vt = &(obj->vertex_tex_coords);
    *cur_v = *cur_vn = *cur_vt = NULL;

    ObjFaceLst **cur_face = &(obj->faces);
    *cur_face = NULL;

    char tk[256];
    while (fgets(tk, sizeof(tk), fp) != NULL) {
        if (tk[0] == '\0' || tk[0] == '#' || tk[0] == 'o' || tk[0] == 'g' || tk[0] == 's') {
            continue;
        } else if(tk[0] == 'v' && tk[1] == ' ') { /* vertex position */
            ObjVectorLst *this_v = pl_malloc(sizeof(ObjVectorLst));
            unsigned int n = sscanf(&tk[2], "%f %f %f", &this_v->v.x, &this_v->v.y, &this_v->v.z);
            if (n < 3) {
                ModelLog("Invalid vertex position, less than 3 coords!\n\"%s\"\n", tk);
            } else if (n > 3) {
                ModelLog("Ignoring fourth vertex position parameter, unsupported!\n\"%s\"\n", tk);
            }

            *cur_v = this_v;
            this_v->next = NULL;
            cur_v = &(this_v->next);
            continue;
        } else if(tk[0] == 'v' && tk[1] == 't') { /* vertex texture */
            ObjVectorLst *this_vt = pl_malloc(sizeof(ObjVectorLst));
            unsigned int n = sscanf(&tk[2], "%f %f", &this_vt->v.x, &this_vt->v.y);
            if (n < 2) {
                ModelLog("Invalid vertex uv, less than 2 coords!\n\"%s\"\n", tk);
            } else if (n > 2) {
                ModelLog("Ignoring third uv parameter, unsupported!\n\"%s\"\n", tk);
            }

            *cur_vt = this_vt;
            this_vt->next = NULL;
            cur_vt = &(this_vt->next);
            continue;
        } else if(tk[0] == 'v' && tk[1] == 'n') { /* vertex normal */
            ObjVectorLst *this_vn = pl_malloc(sizeof(ObjVectorLst));
            if (sscanf(&tk[2], "%f %f %f", &this_vn->v.x, &this_vn->v.y, &this_vn->v.z) < 3) {
                ModelLog("Invalid vertex normal, less than 3 coords!\n\"%s\"\n", tk);
            }

            *cur_vn = this_vn;
            this_vn->next = NULL;
            cur_vn = &(this_vn->next);
            continue;
        } else if(tk[0] == 'f' && tk[1] == ' ') { /* face */
            int idx[64];
            unsigned int num_elements = 1;

            /* faces are kind of a special case, so we'll need
             * to parse them manually here */
            char *pos = tk;
            while(*pos != '\0' && *pos != '\n' && *pos != '\r') {
                if(*pos == ' ') {
                    continue;
                }

                pos++;
            }

            ObjFaceLst *this_face = pl_malloc(sizeof(ObjFaceLst));

            *cur_face = this_face;
            this_face->next = NULL;
            cur_face = &(this_face->next);
            continue;
        }

        ModelLog("Unknown/unsupported parameter '%s', ignoring!\n", tk[0]);
    }

    fclose(fp);

    /* right we're finally done, time to see what we hauled... */

    return NULL;
}

bool plWriteObjModel(PLModel *model, const char *path) {
    if (model == NULL) {
        ReportBasicError(PL_RESULT_INVALID_PARM1);
        return false;
    }

    PLModelLod *lod = plGetModelLodLevel(model, 0);
    if (lod == NULL) {
        ModelLog("No LOD for model, aborting!\n");
        return false;
    }

    FILE *fp = fopen(path, "w");
    if (fp == NULL) {
        ReportBasicError(PL_RESULT_FILEWRITE);
        return false;
    }

    fprintf(fp, "# generated by 'platform' lib (https://github.com/TalonBraveInfo/platform)\n");
    if (model->type == PL_MODELTYPE_SKELETAL) {
        ModelLog("Model is of type skeletal; Obj only supports static models so skeleton will be discarded...\n");
    }

    /* for now, use the same name as the model for the material */
    const char *filename = plGetFileName(path);
    size_t len = strlen(filename);
    char mtl_name[len];
    snprintf(mtl_name, len - 4, "%s", plGetFileName(path));
    fprintf(fp, "mtllib ./%s.mtl\n", mtl_name);

    /* todo: kill duplicated data */
    for (unsigned int i = 0; i < lod->num_meshes; ++i) {
        PLMesh *mesh = lod->meshes[i];
        if (mesh->primitive == PL_MESH_TRIANGLES) {
            fprintf(fp, "o mesh.%0d\n", i);
            /* print out vertices */
            for (unsigned int vi = 0; vi < mesh->num_verts; ++i) {
                fprintf(fp, "v %s\n", plPrintVector3(mesh->vertices[vi].position, pl_float_var));
            }
            /* print out texture coords */
            for (unsigned int vi = 0; vi < mesh->num_verts; ++i) {
                fprintf(fp, "vt %s\n", plPrintVector2(mesh->vertices[vi].st[0], pl_float_var));
            }
            /* print out vertex normals */
            for (unsigned int vi = 0; vi < mesh->num_verts; ++i) {
                fprintf(fp, "vn %s\n", plPrintVector3(mesh->vertices[vi].normal, pl_float_var));
            }
            fprintf(fp, "# %d vertices\n", mesh->num_verts);

            if (mesh->texture != NULL && !plIsEmptyString(mesh->texture->name)) {
                fprintf(fp, "usemtl %s\n", mesh->texture->name);
            }

            for (unsigned int fi = 0; fi < mesh->num_triangles; ++i) {
                fprintf(fp, "f %d/%d/%d\n",
                        mesh->indices[fi],
                        mesh->indices[fi],
                        mesh->indices[fi]);
            }
        }
    }

    fclose(fp);

    // todo...
    return true;
}
