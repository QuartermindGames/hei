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
#include <PL/platform_filesystem.h>
#include <PL/platform_model.h>
#include <PL/platform_console.h>

#include <platform_private.h>

#include "model_private.h"

typedef struct __attribute__((packed)) HDVHeader {
    char identity[32]; /* includes start indicator before text string */

    uint32_t face_offset;
    uint32_t vert_offset;

    uint32_t file_size[2];  /* provided twice, for some reason */

    int32_t unknown[2];

    uint16_t num_vertices;
    uint16_t version;
    uint16_t num_faces;     /* -2, due to some left-over data */

    /* the rest of this is unknown - skip to the face offsets once done here! */
} HDVHeader;

typedef struct __attribute__((packed)) HDVFace {
    uint8_t u0[2];
    uint8_t c_flag;

    uint8_t u1[8];

    uint8_t u2;
    uint16_t vertex_offsets[4];

    int8_t unknown1[16];
} HDVFace;

typedef struct __attribute__((packed)) HDVVertex {
    int32_t x;
    int32_t y;
    int32_t z;
} HDVVertex;

PLModel *plLoadHDVModel(const char *path) {
    FILE *file = fopen(path, "rb");
    if(file == NULL) {
        ReportError(PL_RESULT_FILEREAD, plGetResultString(PL_RESULT_FILEREAD));
        return NULL;
    }

    HDVHeader header;
    if(fread(&header, sizeof(HDVHeader), 1, file) != 1) {
        ReportError(PL_RESULT_FILEREAD, "failed to read in header");
        goto ABORT;
    }

    if(header.identity[0] != '\017' || strncmp("TRITON Vec.Obj", header.identity + 1, 14) != 0) {
        ReportError(PL_RESULT_FILETYPE, "invalid HDV header");
        goto ABORT;
    }

    size_t size = plGetFileSize(path);
    if(header.file_size[0] != size) {
        ReportError(PL_RESULT_FILETYPE, "invalid file size in HDV header");
        goto ABORT;
    }

    if(header.num_vertices > 2048) {
        ReportError(PL_RESULT_FILETYPE, "model exceeds max vertex limit (2048)");
        goto ABORT;
    }

    if(header.face_offset < sizeof(HDVHeader)) {
        ReportError(PL_RESULT_FILETYPE, "invalid face offset in HDV header");
        goto ABORT;
    }

    if(header.vert_offset < sizeof(HDVHeader)) {
        ReportError(PL_RESULT_FILETYPE, "invalid vertex offset in HDV header");
        goto ABORT;
    }

    fseek(file, header.face_offset, SEEK_SET);

    HDVFace faces[2048];
    if(fread(faces, sizeof(HDVFace), header.num_faces, file) != header.num_faces) {
        ReportError(PL_RESULT_FILEREAD, "failed to read in all faces");
        goto ABORT;
    }

    fseek(file, header.vert_offset, SEEK_SET);

    HDVVertex vertices[2048];
    if(fread(vertices, sizeof(HDVVertex), header.num_vertices, file) != header.num_vertices) {
        ReportError(PL_RESULT_FILEREAD, "failed to read in all vertices");
        goto ABORT;
    }

    fclose(file);
    file = NULL;

    PLMesh *mesh = plCreateMesh(PL_MESH_TRIANGLES, PL_DRAW_DYNAMIC,
                                (header.num_faces - 2U) * 2, header.num_vertices);
    if(mesh == NULL) {
        goto ABORT;
    }

#if 1 /* debug */
    srand(mesh->num_verts);
    for(unsigned int i = 0; i < mesh->num_verts; ++i) {
        uint8_t r = (uint8_t)(rand() % 255);
        uint8_t g = (uint8_t)(rand() % 255);
        uint8_t b = (uint8_t)(rand() % 255);
        plSetMeshVertexPosition(mesh, i, PLVector3(-vertices[i].x / 100, -vertices[i].y / 100, vertices[i].z / 100));
        plSetMeshVertexColour(mesh, i, PLColour(r, g, b, 255));
    }
#endif

    unsigned int cur_index = 0;
    for(unsigned int i = 0; i < (header.num_faces - 2U); ++i) {
        /* cast above to shut the compiler up, very odd... */

        //ModelLog(" num_verts %u\n", faces[i].u0[0]);

        // first triangle
        plSetMeshTrianglePosition(mesh, &cur_index,
                                  (uint16_t) (faces[i].vertex_offsets[0] / 12),
                                  (uint16_t) (faces[i].vertex_offsets[1] / 12),
                                  (uint16_t) (faces[i].vertex_offsets[2] / 12)
        );

        if(faces[i].u0[0] == 4) {
            // second triangle
            plSetMeshTrianglePosition(mesh, &cur_index,
                                      (uint16_t) (faces[i].vertex_offsets[3] / 12),
                                      (uint16_t) (faces[i].vertex_offsets[0] / 12),
                                      (uint16_t) (faces[i].vertex_offsets[2] / 12)
            );
        }
    }

    plUploadMesh(mesh);

    PLModel *model = plCreateBasicStaticModel(mesh);
    if(model == NULL) {
        plDestroyMesh(mesh);
        return NULL;
    }

    plGenerateModelNormals(model);
    plGenerateModelBounds(model);

    return model;

    ABORT:
    if(file != NULL) {
        fclose(file);
    }
    return NULL;
}