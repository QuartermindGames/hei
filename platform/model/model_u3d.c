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

#include "platform_private.h"

#include <PL/platform_filesystem.h>
#include <PL/platform_model.h>

/*
	UNREAL 3D Animated Model Format

	The following is based on information from the following page...
	http://paulbourke.net/dataformats/unreal/
*/

typedef struct U3DAnimationHeader {
    uint16_t frames;    // Number of frames.
    uint16_t size;    // Size of each frame.
} U3DAnimationHeader;

typedef struct U3DDataHeader {
    uint16_t numpolys;    // Number of polygons.
    uint16_t numverts;    // Number of vertices.
    uint16_t rotation;    // Mesh rotation?
    uint16_t frame;        // Initial frame.

    uint32_t norm_x;
    uint32_t norm_y;
    uint32_t norm_z;

    uint32_t fixscale;
    uint32_t unused[3];
} U3DDataHeader;

#define    U3D_FLAG_UNLIT       16
#define    U3D_FLAG_FLAT        32
#define    U3D_FLAG_ENVIRONMENT 64
#define    U3D_FLAG_NEAREST     128

enum U3DType {
    U3D_TYPE_NORMAL,
    U3D_TYPE_NORMALTWOSIDED,
    U3D_TYPE_TRANSLUCENT,
    U3D_TYPE_MASKED,
    U3D_TYPE_MODULATE,
    U3D_TYPE_ATTACHMENT
};

typedef struct U3DVertex {
    // This is a bit funky...
    int32_t x : 11;
    int32_t y : 11;
    int32_t z : 10;
} U3DVertex;

typedef struct U3DTriangle {
    uint16_t vertex[3]; // Vertex indices

    uint8_t type;       // Triangle type
    uint8_t colour;     // Triangle colour
    uint8_t ST[3][2];   // Texture coords
    uint8_t texturenum; // Texture offset
    uint8_t flags;      // Triangle flags
} U3DTriangle;

static PLModel* ReadU3DModelData(PLFile* data_ptr, PLFile* anim_ptr) {
    U3DAnimationHeader anim_hdr;
    plReadFile(anim_ptr, &anim_hdr, sizeof(U3DAnimationHeader), 1);

    /* validate animation header */

    if(anim_hdr.size == 0) {
        ReportError(PL_RESULT_FILEREAD, "incorrect animation hdr size for \"%s\"", plGetFilePath(anim_ptr));
        return NULL;
    } else if(anim_hdr.frames == 0) {
        ReportError(PL_RESULT_FILEREAD, "invalid number of frames for \"%s\"", plGetFilePath(anim_ptr));
        return NULL;
    }

    U3DDataHeader data_hdr;
    plReadFile(data_ptr, &data_hdr, sizeof(U3DDataHeader), 1);

    /* validate data header */

    if(data_hdr.numverts == 0) {
        ReportError(PL_RESULT_FILEREAD, "no vertices in model, \"%s\"", plGetFilePath(data_ptr));
        return NULL;
    } else if(data_hdr.numpolys == 0) {
        ReportError(PL_RESULT_FILEREAD, "no polygons in model, \"%s\"", plGetFilePath(data_ptr));
        return NULL;
    } else if(data_hdr.frame > anim_hdr.frames) {
        ReportError(PL_RESULT_FILEREAD, "invalid frame specified in model, \"%s\"", plGetFilePath(data_ptr));
        return NULL;
    }

    /* skip unused header data */
    plFileSeek(data_ptr, 12, PL_SEEK_CUR);

    /* read all the triangle data from the data file */
    U3DTriangle* triangles = pl_malloc(sizeof(U3DTriangle) * data_hdr.numpolys);
    plReadFile(data_ptr, triangles, sizeof(U3DTriangle), data_hdr.numpolys);

    /* generate meshes per frame */
    PLMesh* meshes[anim_hdr.frames];
    for(unsigned int i = 0; i < anim_hdr.frames; ++i) {

        plGenerateMeshNormals(meshes[i]);
    }

    /* and now create the model container, a lot of this will change in the longer term */
    PLModel* model_ptr = pl_malloc(sizeof(PLModel));
    model_ptr->type = PL_MODELTYPE_VERTEX;
    model_ptr->current_level = data_hdr.frame;

    plGenerateModelBounds(model_ptr);

    return NULL;
}

/**
 * Load U3D model from local path.
 * @param path Path to the U3D Data file.
 */
PLModel* plLoadU3DModel(const char *path) {
    char anim_path[PL_SYSTEM_MAX_PATH];
    strncpy(anim_path, path, sizeof(anim_path));
    char* p_ext = strstr(anim_path, "Data");
    if(p_ext != NULL) {
        strcpy(p_ext, "Anim");
    } else {
        p_ext = strstr(anim_path, "D.");
        if(p_ext != NULL) {
            p_ext[0] = 'A';
        }
    }

    if(!plFileExists(anim_path)) {
        ReportError(PL_RESULT_FILEPATH, "failed to find anim companion for \"%s\" at \"%s\"", path, anim_path);
        return NULL;
    }

    PLFile* anim_ptr = plOpenFile(anim_path, true);
    PLFile* data_ptr = plOpenFile(path, true);

    PLModel* model_ptr = NULL;
    if(anim_ptr != NULL && data_ptr != NULL) {
        model_ptr = ReadU3DModelData(data_ptr, anim_ptr);
    }

    plCloseFile(data_ptr);
    plCloseFile(anim_ptr);

    return model_ptr;

#if 0
    // Allocate the triangle/vertex arrays.
    model->frames = new PLModelFrame[model->num_frames];
    for (unsigned int i = 0; i < model->num_frames; i++) {
        model->frames[i].vertices = new PLVertex[model->num_vertices];
        model->frames[i].triangles = new PLTriangle[model->num_triangles];
    }


    // Go through each vertex.
    std::vector<U3DVertex> uvertices;
    for (unsigned int i = 0; i < model->num_frames; i++) {
        if (std::fread(&uvertices[i], sizeof(U3DVertex), 1, pl_u3d_animf) != 1) {
            _plSetErrorMessage("Failed to process vertex! (%i)\n", i);

            plDestroyAnimatedModel(model);

            _plUnloadU3DModel();
            return nullptr;
        }

        for (unsigned int j = 0; j < model->num_triangles; j++) {

        }
    }

    // Calculate normals.
    _plGenerateAnimatedModelNormals(model);

    _plUnloadU3DModel();

    return model;
#else
    return NULL;
#endif
}
