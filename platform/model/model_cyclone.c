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

#define MAX_INDICES_PER_POLYGON    9
#define MIN_INDICES_PER_POLYGON    3

#define MAX_UV_COORDS_PER_FACE  36

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
    float x;
    float y;
    float z;
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

typedef struct MDLPolygon {
    uint16_t indices[MAX_INDICES_PER_POLYGON];
    uint32_t num_indices;
    int16_t uv[MAX_UV_COORDS_PER_FACE];
} MDLPolygon;

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

    uint32_t num_vertices;
    if(fread(&num_vertices, sizeof(uint32_t), 1, file) != 1) {
        AbortLoad("Invalid file length, failed to get number of vertices!\n");
        return NULL;
    }

    if(num_vertices == 0) {
        AbortLoad("invalid number of vertices, %d", num_vertices);
        return NULL;
    }

    uint32_t num_polygons;
    if(fread(&num_polygons, sizeof(uint32_t), 1, file) != 1) {
        AbortLoad("Invalid file length, failed to get number of polygons!\n");
        return NULL;
    }

    if(num_polygons == 0) {
        AbortLoad("Invalid number of faces, %d!\n", num_polygons);
        return NULL;
    }

    MDLVertex vertices[num_vertices];
    if(fread(vertices, sizeof(MDLVertex), num_vertices, file) != num_vertices) {
        AbortLoad("Invalid file length, failed to load vertices!\n");
        return NULL;
    }

    unsigned int num_triangles = 0;
    unsigned int num_indices = 0;
    MDLPolygon polygons[num_polygons];
    memset(polygons, 0, sizeof(MDLPolygon) * num_polygons);
    for(unsigned int i = 0; i < num_polygons; ++i) {
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
        if(fread(&polygons[i].num_indices, sizeof(uint32_t), 1, file) != 1) {
            AbortLoad("Invalid file length, failed to load number of indices! (offset: %ld)\n", ftell(file));
            return NULL;
        }

        if(polygons[i].num_indices < MIN_INDICES_PER_POLYGON || polygons[i].num_indices > MAX_INDICES_PER_POLYGON) {
            AbortLoad("Invalid number of indices, %d, required for polygon %d! (offset: %ld)\n",
                      polygons[i].num_indices, i, ftell(file));
            return NULL;
        }

        num_triangles += polygons[i].num_indices - 2;
        if(polygons[i].num_indices == 4) {
            // conversion of quad to triangles
            num_indices += 6;
        } else {
            num_indices += polygons[i].num_indices * 100;
        }

        fseek(file, 16, SEEK_CUR); // todo, figure these out
        if(fread(polygons[i].indices, sizeof(uint16_t), polygons[i].num_indices, file) != polygons[i].num_indices) {
            AbortLoad("invalid file length, failed to load indices\n");
            return NULL;
        }

        unsigned int num_uv_coords = (unsigned int) (polygons[i].num_indices * 4);
        //ModelLog(" num bytes for UV coords is %lu\n", num_uv_coords * sizeof(int16_t));
        if(fread(polygons[i].uv, sizeof(int16_t), num_uv_coords, file) != num_uv_coords) {
            AbortLoad("invalid file length, failed to load UV coords\n");
            return NULL;
        }

        long npos = ftell(file);
        //ModelLog(" Read %ld bytes for polygon %d (indices %d)\n", npos - pos, i, polygons[i].num_indices);
    }

    fclose(file);

#if 0 // check the indices are in range
    for(unsigned int i = 0; i < num_polygons; ++i) {
        for(unsigned int j = 0; j < polygons[i].num_indices; ++j) {
            assert(polygons[i].indices[j] < num_vertices);
        }
    }
#endif

#if 0 // debug info
    ModelLog("texture_name_length: %d\n", texture_name_length);
    ModelLog("texture_name:        %s\n", texture_name);
    ModelLog("num_vertices:        %d\n", num_vertices);
    for(unsigned int i = 0; i < num_vertices; ++i) {
        ModelLog("  vertex(%u) x(%f) y(%f) z(%f)\n", i, vertices[i].x, vertices[i].y, vertices[i].z);
    }
    ModelLog("num_polygons:        %d\n", num_polygons);
    for(unsigned int i = 0; i < num_polygons; ++i) {
        ModelLog("  face(%d) num_indices(%d)\n", i, polygons[i].num_indices);
        for(unsigned int j = 0; j < polygons[i].num_indices; ++j) {
            ModelLog("   index %d\n", polygons[i].indices[j]);
        }
    }
    ModelLog("num_triangles:       %d\n", num_triangles);
    ModelLog("num_indices:         %d\n", num_indices);
#endif

    PLMesh *mesh = plCreateMesh(PL_MESH_TRIANGLES, PL_DRAW_IMMEDIATE, num_triangles, num_vertices);
    if(mesh == NULL) {
        return NULL;
    }

    mesh->num_indices = num_indices;
    mesh->indices = calloc(mesh->num_indices, sizeof(uint16_t));
    if(mesh->indices == NULL) {
        plDeleteMesh(mesh);
        return NULL;
    }

    srand(num_vertices);
    for(unsigned int i = 0; i < num_vertices; ++i) {
        uint8_t r = (uint8_t)(rand() % 255);
        uint8_t g = (uint8_t)(rand() % 255);
        uint8_t b = (uint8_t)(rand() % 255);

        plSetMeshVertexPosition(mesh, i, PLVector3(vertices[i].x, vertices[i].y, vertices[i].z));
        plSetMeshVertexColour(mesh, i, PLColour(r, g, b, 255));
    }

    unsigned int cur_index = 0;
    for(unsigned int i = 0; i < num_polygons; ++i) {
        if(polygons[i].num_indices == 4) { // quad
            assert((cur_index + 6) <= mesh->num_indices);
            // first triangle
            mesh->indices[cur_index++] = polygons[i].indices[0];
            mesh->indices[cur_index++] = polygons[i].indices[1];
            mesh->indices[cur_index++] = polygons[i].indices[2];
            // second triangle
            mesh->indices[cur_index++] = polygons[i].indices[3];
            mesh->indices[cur_index++] = polygons[i].indices[0];
            mesh->indices[cur_index++] = polygons[i].indices[2];
        } else if((polygons[i].num_indices % 3) == 0) { // triangle
            assert((cur_index + polygons[i].num_indices) <= mesh->num_indices);
            for(unsigned int j = 0; j < polygons[i].num_indices; ++j) {
                mesh->indices[cur_index++] = polygons[i].indices[j];
            }
        } else { // triangle strip, converted into triangles
#if 0
            for(unsigned int j = 0; j + 2 < polygons[i].num_indices; ++j) {
                mesh->indices[cur_index++] = polygons[i].indices[j];
                mesh->indices[cur_index++] = polygons[i].indices[j + 1];
                mesh->indices[cur_index++] = polygons[i].indices[j + 2];
            }
#else
            /* Triangle fan */
            for(unsigned int j = 1; j + 1 < polygons[i].num_indices; ++j) {
                mesh->indices[cur_index++] = polygons[i].indices[0];
                mesh->indices[cur_index++] = polygons[i].indices[j];
                mesh->indices[cur_index++] = polygons[i].indices[j + 1];
            }
#endif
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
        plDeleteMesh(mesh);
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