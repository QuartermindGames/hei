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
#include <PL/platform_model.h>
#include <PL/platform_console.h>

#include "platform_private.h"
#include "model_private.h"

enum {
    MDL_FLAG_FLAT   = (1 << 0),
    MDL_FLAG_UNLIT  = (1 << 1),
};

#define MAX_MODEL_NAME      128
#define MAX_TEXTURE_NAME    64

#define MAX_INDICES_PER_FACE    8
#define MIN_INDICES_PER_FACE    3

#define MAX_UV_COORDS_PER_FACE  16

/* There seem to be two seperate model formats
 * used within the game, those that are static
 * and then a completely different type of header
 * for those that aren't.
 *
 * The texture names don't used fixed sizes in either,
 * in the static model the length of these is declared
 * but it doesn't appear this is the case in the animated
 * model format; the animated model format also seems
 * to support multiple textures.
 *
 * The first byte in the model format, is a flag which appears to
 * control how the model will be displayed within the game.
 *
 * Within medkit.mdl
 *  OFFSET ?? is X coord of vertex 1
 *  OFFSET 66 is Y coord of vertex 1
 *  OFFSET 6A is Z coord of vertex 1
 *  OFFSET 6E is X coord of vertex 2
 *  OFFSET 72 is Y coord of vertex 2
 *
 *  3 bytes between each vertex coord
 *  12 bytes for each coord set?
 *
 *  First 32bit int after texture name
 *  appears to be number of vertices
 *
 *  ?? ?? X  ?? ?? ?? Y  ?? ?? ?? Z  ??
 *  7E 71 41 3F 4C 1E 0D BC 9F 3C A8 3F
 *
 *  ????X X  ????Y Y  ????Z Z
 *  7E71413F 4C1E0DBC 9F3CA83F
 */

typedef struct __attribute__((packed)) MDLVertex {
#if 0
    uint8_t unknown0[2];
    int16_t x;
    uint8_t unknown1[2];
    int16_t y;
    uint8_t unknown2[2];
    int16_t z;
#else
    float x;
    float y;
    float z;
#endif
} MDLVertex;

// 04:00:00:00:B4:BC:79:00:00:00:00:00:00:00:00:00:
// BC:BC:79:00:1C:00:1F:00:1B:00:19:00:57:D0:76:00:
// 66:42:1B:00:B0:21:78:00:70:E4:19:00:5C:35:6C:00:
// A1:D5:1C:00:9C:48:6A:00:36:DF:1E:00
// Quads are 60 bytes long; immediately follow vertices

// 03:00:00:00:14:BD:79:00:00:00:00:00:01:00:00:00:
// 1A:BD:79:00:05:00:01:00:06:00:E6:25:12:00:A1:DE:
// 22:00:92:CB:08:00:09:C3:17:00:B6:65:22:00:D8:E0:
// 09:00
// Triangles are 50 bytes long

typedef struct MDLFace {
    uint16_t indices[MAX_INDICES_PER_FACE];
    uint8_t num_indices;

    int16_t uv[MAX_UV_COORDS_PER_FACE];
} MDLFace;

#if 0
    uint8_t unknown0;  // typically seeing this as 4?
    // could be number of verts per face?
    uint8_t unknown000;
    uint16_t unknown00;

    uint8_t unknown1[16]; // completely no clue for now!

    uint16_t indices[4];

    uint8_t unknown2[32]; // no clue either!
#endif

/////////////////////////////////////////////////////////////////

PLModel *LoadRequiemModel(const char *path) {
    FILE *file = fopen(path, "rb");
    if(file == NULL) {
        ReportError(PL_RESULT_FILEREAD, plGetResultString(PL_RESULT_FILEREAD));
        return NULL;
    }

    // attempt to figure out if it's valid or not... ho boy...

#define AbortLoad(...) ModelLog(__VA_ARGS__); ReportError(PL_RESULT_FILEREAD, __VA_ARGS__); fclose(file)

    // check which flags have been set for this particular mesh
    int flags = fgetc(file);
    if(flags & MDL_FLAG_FLAT) {} // flat
    if(flags & MDL_FLAG_UNLIT) {} // unlit
    if(!(flags & MDL_FLAG_FLAT) && !(flags & MDL_FLAG_UNLIT)) {} // shaded

    uint32_t texture_name_length = 0;
    if(fread(&texture_name_length, sizeof(uint32_t), 1, file) != 1) {
        AbortLoad("Invalid file length, failed to get texture name length!\n");
        return NULL;
    }

    if(texture_name_length > MAX_TEXTURE_NAME || texture_name_length == 0) {
        AbortLoad("Invalid texture name length, %d!\n", texture_name_length);
        return NULL;
    }

    char texture_name[texture_name_length];
    if(fread(texture_name, sizeof(char), texture_name_length, file) != texture_name_length) {
        AbortLoad("Invalid file length, failed to get texture name!\n");
        return NULL;
    }

    uint16_t num_vertices;
    if(fread(&num_vertices, sizeof(uint16_t), 1, file) != 1) {
        AbortLoad("Invalid file length, failed to get number of vertices!\n");
        return NULL;
    }

    // todo, figure out these two unknown bytes, quads? iirc (we did discuss this)
    fseek(file, 2, SEEK_CUR);

    uint32_t num_faces;
    if(fread(&num_faces, sizeof(uint32_t), 1, file) != 1) {
        AbortLoad("Invalid file length, failed to get number of faces!\n");
        return NULL;
    }

    if(num_faces == 0) {
        AbortLoad("Invalid number of faces, %d!\n", num_faces);
        return NULL;
    }

    MDLVertex vertices[num_vertices];
    if(fread(vertices, sizeof(MDLVertex), num_vertices, file) != num_vertices) {
        AbortLoad("Invalid file length, failed to load vertices!\n");
        return NULL;
    }

#if 1 // Requiem's models seem to be pretty small...
    for(unsigned int i = 0; i < num_vertices; ++i) {
        vertices[i].x *= 100.f;
        vertices[i].y *= 100.f;
        vertices[i].z *= 100.f;
    }
#endif

    unsigned int num_triangles = 0;
    MDLFace faces[num_faces];
    memset(faces, 0, sizeof(MDLFace) * num_faces);
    for(unsigned int i = 0; i < num_faces; ++i) {
        /* 0000:00D0 |                           04 00 00 00  F4 9C 79 00 |         ....ô.y.
         * 0000:00E0 | 00 00 00 00  00 00 00 00  FC 9C 79 00  01 00 03 00 | ........ü.y.....
         * 0000:00F0 | 0B 00 09 00  93 65 72 00  00 80 00 00  00 80 7F 00 | .....er.........
         * 0000:0100 | 66 C0 0A 00  00 80 00 00  66 C0 0A 00  6C 9A 0D 00 | fÀ......fÀ..l...
         * 0000:0110 | 00 80 00 00                                        | ....
         *
         * Number of indices per face
         * 04 00 00 00
         *
         * Unsure...
         * F4 9C 79 00
         */

        long pos = ftell(file);
        if(fread(&faces[i].num_indices, sizeof(uint32_t), 1, file) != 1) {
            AbortLoad("Invalid file length, failed to load number of indices! (offset: %ld)\n", ftell(file));
            return NULL;
        }

        if(faces[i].num_indices < MIN_INDICES_PER_FACE || faces[i].num_indices > MAX_INDICES_PER_FACE) {
            AbortLoad("Invalid number of indices, %d, required for face %d! (offset: %ld)\n",
                      faces[i].num_indices, i, ftell(file));
            return NULL;
        }

        num_triangles += faces[i].num_indices - 2;

        fseek(file, 16, SEEK_CUR); // todo, figure these out
        if(fread(faces[i].indices, sizeof(uint16_t), faces[i].num_indices, file) != faces[i].num_indices) {
            AbortLoad("invalid file length, failed to load indices\n");
            return NULL;
        }

        unsigned int num_uv_coords = faces[i].num_indices * 4;
        //ModelLog(" num bytes for UV coords is %lu\n", num_uv_coords * sizeof(int16_t));
        if(fread(faces[i].uv, sizeof(int16_t), num_uv_coords, file) != num_uv_coords) {
            AbortLoad("invalid file length, failed to load UV coords\n");
            return NULL;
        }

        long npos = ftell(file);
        //ModelLog(" Read %ld bytes for face %d (indices %d)\n", npos - pos, i, faces[i].num_indices);
    }

    fclose(file);

    ModelLog("    texture_name_length: %d\n", texture_name_length);
    ModelLog("    texture_name:        %s\n", texture_name);
    ModelLog("    num_vertices:        %d\n", num_vertices);
    for(unsigned int i = 0; i < num_vertices; ++i) {
        ModelLog("      vertex(%u) x(%f) y(%f) z(%f)\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
    }
    ModelLog("    num_faces:           %d\n", num_faces);
    for(unsigned int i = 0; i < num_faces; ++i) {
        ModelLog("      face(%d) num_indices(%d)\n", i, faces[i].num_indices);
    }
    ModelLog("    num_triangles:       %d\n", num_triangles);

    num_vertices = num_triangles * 3;
    PLMesh *mesh = plCreateMesh(PL_MESH_TRIANGLES, PL_DRAW_IMMEDIATE, num_triangles, num_vertices);
    if(mesh == NULL) {
        return NULL;
    }

    // todo, create a new mesh for faces greater than 4 verts?

    srand(num_faces);
    unsigned int cur_vertex = 0;
    for(unsigned int i = 0; i < num_faces; ++i) {
#if 1
        uint8_t r = (uint8_t)(rand() % 255);
        uint8_t g = (uint8_t)(rand() % 255);
        uint8_t b = (uint8_t)(rand() % 255);
#endif

        if(faces[i].num_indices == 3) { // triangle
            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                      vertices[faces[i].indices[0]].x,
                                      vertices[faces[i].indices[0]].y,
                                      vertices[faces[i].indices[0]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;
            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                      vertices[faces[i].indices[1]].x,
                                      vertices[faces[i].indices[1]].y,
                                      vertices[faces[i].indices[1]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;
            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                      vertices[faces[i].indices[2]].x,
                                      vertices[faces[i].indices[2]].y,
                                      vertices[faces[i].indices[2]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;
        } else if(faces[i].num_indices == 4) { // quad

            // first triangle

            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                            vertices[faces[i].indices[0]].x,
                                            vertices[faces[i].indices[0]].y,
                                            vertices[faces[i].indices[0]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;
            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                      vertices[faces[i].indices[1]].x,
                                      vertices[faces[i].indices[1]].y,
                                      vertices[faces[i].indices[1]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;
            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                      vertices[faces[i].indices[2]].x,
                                      vertices[faces[i].indices[2]].y,
                                      vertices[faces[i].indices[2]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;

            // second triangle

            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                      vertices[faces[i].indices[3]].x,
                                      vertices[faces[i].indices[3]].y,
                                      vertices[faces[i].indices[3]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;
            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                      vertices[faces[i].indices[0]].x,
                                      vertices[faces[i].indices[0]].y,
                                      vertices[faces[i].indices[0]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;
            plSetMeshVertexPosition3f(mesh, cur_vertex,
                                      vertices[faces[i].indices[2]].x,
                                      vertices[faces[i].indices[2]].y,
                                      vertices[faces[i].indices[2]].z);
            plSetMeshVertexColour(mesh, cur_vertex, PLColour(r, g, b, 255));
            cur_vertex++;
        }
    }

    PLModel *model = malloc(sizeof(PLModel));
    if(model == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));
        return NULL;
    }

    memset(model, 0, sizeof(PLModel));
    model->num_lods = 1;
    model->lods[0].meshes = calloc(1, sizeof(PLMesh));
    if(model->lods[0].meshes == NULL) {
        plDeleteModel(model);

        ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));
        return NULL;
    }

    model->lods[0].num_meshes = 1;
    model->lods[0].meshes[0] = *mesh;

    plGenerateModelNormals(model);
    plGenerateModelAABB(model);

    return model;
}