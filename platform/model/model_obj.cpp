/*
DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
Version 2, December 2004

Copyright (C) 2011-2016 Mark E Sowden <markelswo@gmail.com>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

0. You just DO WHAT THE FUCK YOU WANT TO.
*/

#include "PL/platform.h"

#include "PL/platform_model.h"

using namespace pl::graphics;

/*	OBJ Static Model Format */

enum {
    OBJ_SYNTAX_COMMENT = '#',
    OBJ_SYNTAX_OBJECT = 'o',
    OBJ_SYNTAX_GROUP = 'g',
    OBJ_SYNTAX_SMOOTH = 's',
    OBJ_SYNTAX_VERTEX = 'v',
    OBJ_SYNTAX_VERTEX_ST = 't',    // vt
    OBJ_SYNTAX_VERTEX_NORMAL = 'n',    // vn
    OBJ_SYNTAX_FACE = 'f',
};

std::vector<PLVector3> vertices;
std::vector<PLVector3> normals;

std::ifstream pl_obj_data;

void UnloadOBJModel() {
    if (pl_obj_data.is_open())
        pl_obj_data.close();

    // Shrink our vectors down.
    vertices.empty();
    vertices.shrink_to_fit();
    normals.empty();
    normals.shrink_to_fit();
}

PLModel *LoadOBJModel(const PLchar *path) {
#if 0 // todo
    _plSetCurrentFunction("plLoadOBJModel");

    pl_obj_data.open(path, std::ifstream::binary);
    if (!pl_obj_data) {
        ReportError(PL_RESULT_FILEERR, "Failed to load file! (%s)\n", path);
        return nullptr;
    }

    // Get the length of the file.
    pl_obj_data.seekg(0, pl_obj_data.end);
    std::streamoff length = pl_obj_data.tellg();
    pl_obj_data.seekg(0, pl_obj_data.beg);
    if (length <= 0) {
        UnloadOBJModel();

        ReportError(PL_RESULT_FILETYPE, "Invalid OBJ model! (%s)\n", path);
        return nullptr;
    }

    std::string line;
    while (std::getline(pl_obj_data, line)) {
        switch (line[0]) {
            case OBJ_SYNTAX_COMMENT:
                continue;

            case OBJ_SYNTAX_VERTEX: {
                if (line[1] == OBJ_SYNTAX_VERTEX_NORMAL) {
                    PLVector3 normal = {0, 0, 0};
                    std::sscanf(line.c_str() + 2, "%f %f %f", &normal.x, &normal.y, &normal.z);
                    normals.push_back(PLVector3(normal.x, normal.y, normal.z));
                } else if (line[1] == OBJ_SYNTAX_VERTEX_ST) {}
                else // Vertex coords
                {
                    PLVector3 position = {0, 0, 0};
                    std::sscanf(line.c_str() + 2, "%f %f %f", &position.x, &position.y, &position.z);
                    vertices.push_back(PLVector3(position.x, position.y, position.z));
                }
            } break;
                
            case OBJ_SYNTAX_FACE: {

            } break;

            default:
                // Materials are ignored for now, do we want these?
                if (!strncmp("mtllib", line.c_str(), 6))
                    continue;
                else if (!strncmp("usemtl", line.c_str(), 6))
                    continue;
                break;
        }
    }

    PLModel *model = (PLModel*)malloc(sizeof(PLModel));
    if (model == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));

        UnloadOBJModel();
        return nullptr;
    }

    model->num_triangles = 0;
    model->num_vertices = (unsigned int) vertices.size();
    //model->primitive = PL_MESH_POINTS; // todo, set this within the mesh instead!
#if 0
    // Allocate vertex/triangle arrays.
    model->frame.vertices = new PLVertex[model->num_vertices];
    for (unsigned int i = 0; i < model->num_vertices; i++) {
        PLVertex *vertex = &model->frame.vertices[0];
        //vertex->position = vertices[i];
    }
#endif
    UnloadOBJModel();

    return model;
#else
    return NULL;
#endif
}
