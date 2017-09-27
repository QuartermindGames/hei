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
#include <PL/platform_math.h>

#define TITLE   "ibsp2level\n  Conversion Utility"

enum {
    IBSP_LUMP_ENTITIES,
    IBSP_LUMP_PLANES,
    IBSP_LUMP_VERTICES,
    IBSP_LUMP_VISIBILITY,
    IBSP_LUMP_NODES,
    IBSP_LUMP_TEXINFO,
    IBSP_LUMP_FACES,
    IBSP_LUMP_LIGHTMAPS,
    IBSP_LUMP_LEAVES,
    IBSP_LUMP_FACETABLE,
    IBSP_LUMP_BRUSHTABLE,
    IBSP_LUMP_EDGES,
    IBSP_LUMP_EDGETABLE,
    IBSP_LUMP_MODELS,
    IBSP_LUMP_BRUSHES,
    IBSP_LUMP_SIDES,
    IBSP_LUMP_POP,
    IBSP_LUMP_AREAS,
    IBSP_LUMP_PORTALS,
};

struct {

    struct {
        uint32_t magic;
        uint32_t version;

        struct {
            uint32_t offset;
            uint32_t length;
        } lump[19];
    } header;

    struct {
        uint16_t plane;
        uint16_t plane_side;

        uint32_t first_edge;
        uint16_t num_edges;

        uint16_t texture_info;

        uint8_t lightmap_styles[4];
        uint32_t lightmap_offset;
    } lump_face;

    struct {
        float normal[3];
        float distance;

        uint32_t type;
    } lump_plane;

    struct {
        uint32_t plane;

        int32_t front_child;
        int32_t back_child;

        short bbox_min[3];
        short bbox_max[3];

        uint16_t first_face;
        uint16_t num_faces;
    } lump_node;

    struct {
        uint32_t unknown;

        uint16_t cluster;
        uint16_t area;

        short bbox_min[3];
        short bbox_max[3];

        uint16_t first_leaf_face;
        uint16_t num_leaf_faces;

        uint16_t first_leaf_brush;
        uint16_t num_leaf_brushes;
    } lump_leaf;

    struct {
        float u_axis[3];
        float u_offset;

        float v_axis[3];
        float v_offset;

        uint32_t flags;
        uint32_t value;

        char texture_name[32];

        uint32_t next_texinfo;
    } lump_texinfo;

    struct {
        uint32_t pvs;
        uint32_t phs;
    } lump_vis_offset;

} BSP;

int main(int argc, char **argv) {
    printf("-------------------------------\n");
    printf("  " TITLE "\n");
    printf("      Written by Mark E Sowden\n");
    printf("-------------------------------\n");

    memset(&BSP, 0, sizeof(BSP));

    if(argc < 2) {
        printf("Usage: ibsp2level <ibsp path> ");
    }

    return 0;
}