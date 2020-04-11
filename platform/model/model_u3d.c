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
    uint16_t size;      // Size of each frame.
} U3DAnimationHeader;

typedef struct U3DDataHeader {
    uint16_t numpolys;    // Number of polygons.
    uint16_t numverts;    // Number of vertices.
    uint16_t rotation;    // Mesh rotation?
    uint16_t frame;       // Initial frame.

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

static int CompareTriangles(const void* a, const void* b) {
    if(((U3DTriangle*)a)->texturenum > ((U3DTriangle*)b)->texturenum) {
        return -1;
    } else if(((U3DTriangle*)a)->texturenum < ((U3DTriangle*)b)->texturenum) {
        return 1;
    }

    return 0;
}

static PLModel* ReadU3DModelData(PLFile* data_ptr, PLFile* anim_ptr) {
    U3DAnimationHeader anim_hdr;
    if(plReadFile(anim_ptr, &anim_hdr, sizeof(U3DAnimationHeader), 1) != 1) {
      return NULL;
    }

    /* validate animation header */

    if(anim_hdr.size == 0) {
        ReportError(PL_RESULT_FILEREAD, "incorrect animation hdr size for \"%s\"", plGetFilePath(anim_ptr));
        return NULL;
    } else if(anim_hdr.frames == 0) {
        ReportError(PL_RESULT_FILEREAD, "invalid number of frames for \"%s\"", plGetFilePath(anim_ptr));
        return NULL;
    }

    U3DDataHeader data_hdr;
    if(plReadFile(data_ptr, &data_hdr, sizeof(U3DDataHeader), 1) != 1) {
      return NULL;
    }

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
    U3DTriangle* triangles = pl_calloc(data_hdr.numpolys, sizeof(U3DTriangle));
    plReadFile(data_ptr, triangles, sizeof(U3DTriangle), data_hdr.numpolys);
    plCloseFile(data_ptr);

    /* sort triangles by texture id */
    qsort(triangles, data_hdr.numpolys, sizeof(U3DTriangle), CompareTriangles);

    /* read in all of the animation data from the anim file */
    U3DVertex* vertices = pl_calloc(data_hdr.numverts * anim_hdr.frames, sizeof(U3DVertex));
    plReadFile(anim_ptr, vertices, sizeof(U3DVertex), data_hdr.numverts * anim_hdr.frames);
    plCloseFile(anim_ptr);

    PLModel* model_ptr = pl_calloc(1, sizeof(PLModel));
    model_ptr->type = PL_MODELTYPE_VERTEX;
    model_ptr->internal.vertex_data.animations = pl_calloc(anim_hdr.frames, sizeof(PLVertexAnimationFrame));

    for(unsigned int i = 0; i < anim_hdr.frames; ++i) {
        PLVertexAnimationFrame* frame = &model_ptr->internal.vertex_data.animations[i];
    }

    pl_free(triangles);
    pl_free(vertices);

    plGenerateModelBounds(model_ptr);

    return NULL;
}

/**
 * Load U3D model from local path.
 * @param path Path to the U3D Data file.
 */
PLModel* plLoadU3DModel(const char *path) {
    char anim_path[PL_SYSTEM_MAX_PATH];
    snprintf(anim_path, sizeof(anim_path), "%s", path);
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

    PLFile* anim_ptr = plOpenFile(anim_path, false);
    PLFile* data_ptr = plOpenFile(path, false);

    PLModel* model_ptr = NULL;
    if(anim_ptr != NULL && data_ptr != NULL) {
        model_ptr = ReadU3DModelData(data_ptr, anim_ptr);
    }

    plCloseFile(data_ptr);
    plCloseFile(anim_ptr);

    return model_ptr;
}

bool plWriteU3DModel(PLModel* ptr, const char* path) {
    return false;
}

/* Example UC file, this is what we _should_ be loading from.

#exec MESH IMPORT MESH=wafr1 ANIVFILE=MODELS\wafr1_a.3D DATAFILE=MODELS\wafr1_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wafr1 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=wafr1 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wafr1 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wafr1 X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=wafr1 NUM=0 TEXTURE=DefaultTexture

#exec MESH IMPORT MESH=wafr2 ANIVFILE=MODELS\wafr2_a.3D DATAFILE=MODELS\wafr2_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wafr2 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=wafr2 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wafr2 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wafr2 X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=wafr2 NUM=0 TEXTURE=DefaultTexture

#exec MESH IMPORT MESH=wafr4 ANIVFILE=MODELS\wafr4_a.3D DATAFILE=MODELS\wafr4_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=wafr4 X=0 Y=0 Z=0 YAW=64 ROLL=64
#exec MESH SEQUENCE MESH=wafr4 SEQ=All  STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=wafr4 SEQ=Still  STARTFRAME=0   NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=wafr4 X=0.1 Y=0.1 Z=0.2
#exec MESHMAP SETTEXTURE MESHMAP=wafr4 NUM=0 TEXTURE=DefaultTexture

*/
