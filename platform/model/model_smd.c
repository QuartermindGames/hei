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

static FILE *fp_out = NULL;

static void WriteSMDVertex(const PLVertex *vertex) {
    /*               P X  Y  Z  NX NY NZ U  V */
    fprintf(fp_out, "0 %f %f %f %f %f %f %f %f\n",

            vertex->position.x,
            vertex->position.y,
            vertex->position.z,

            vertex->normal.x,
            vertex->normal.y,
            vertex->normal.z,

            vertex->st[0].x,
            vertex->st[0].y);
}

/* writes given model out to Valve's SMD model format */
bool _plWriteSMDModel(const char *path, const PLModel *model) {
    char full_path[PL_SYSTEM_MAX_PATH];
    snprintf(full_path, sizeof(full_path), "./models/%s.smd", path);
    fp_out = fopen(full_path, "w");
    if(fp_out == NULL) {
        ReportError(PL_RESULT_FILEWRITE, plGetResultString(PL_RESULT_FILEWRITE));
        return false;
    }

    /* header */
    fprintf(fp_out, "version 1\n\n");

    /* write out the nodes block */
    fprintf(fp_out, "nodes\n");
    if(model->num_bones == 0) {
        /* write out a dummy bone! */
        fprintf(fp_out, "0 \"root\" -1\n");
    } else {
        /* todo, revisit this so we're correctly connecting child/parent */
        for (unsigned int i = 0; i < model->num_bones; ++i) {
            fprintf(fp_out, "%u %s %d\n", i, model->bones[i].name, (int) i - 1);
        }
    }
    fprintf(fp_out, "end\n\n");

    /* skeleton block */
    fprintf(fp_out, "skeleton\ntime 0\n");
    if(model->num_bones == 0) {
        /* write out dummy bone coords! */
        fprintf(fp_out, "0 0 0 0 0 0 0\n");
    } else {
        /* todo, print out default coords for each bone */
    }
    fprintf(fp_out, "end\n\n");

    /* triangles block */
    fprintf(fp_out, "triangles\n");
    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        for(unsigned int j = 0; j < model->meshes[i].mesh->num_indices; ) {
            if(model->meshes[i].mesh->texture == NULL) {
                fprintf(fp_out, "null\n");
            } else {
                fprintf(fp_out, "%s\n", model->meshes[i].mesh->texture->name);
            }
            WriteSMDVertex(&model->meshes[i].mesh->vertices[model->meshes[i].mesh->indices[j++]]);
            WriteSMDVertex(&model->meshes[i].mesh->vertices[model->meshes[i].mesh->indices[j++]]);
            WriteSMDVertex(&model->meshes[i].mesh->vertices[model->meshes[i].mesh->indices[j++]]);
        }
    }
    fprintf(fp_out, "end\n\n");

    /* and leave a blank line at the end, to keep studiomdl happy */
    fprintf(fp_out, "\n");

    fclose(fp_out);
    fp_out = NULL;
    return true;
}
