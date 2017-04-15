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

#include <platform_log.h>
#include <platform_window.h>
#include <platform_graphics.h>
#include <platform_graphics_mesh.h>
#include <platform_filesystem.h>

#include <GLFW/glfw3.h>

#define TITLE "H0G Loader"
#define LOG "hog_loader"
#define PRINT(...) printf(__VA_ARGS__); plWriteLog(LOG, __VA_ARGS__);
#define WIDTH 1024
#define HEIGHT 768

typedef struct VTXCoord {
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t padding;
} VTXCoord;

typedef struct FACHeader {
    uint32_t padding[4];    // This is always blank
    uint32_t num_blocks;    // Number of FACBlocks
    //uint32_t unknown2;
    uint32_t unknown3;
    uint32_t unknown4;
} FACHeader;


// BLANK    BLANK    BLANK    BLANK    BLOCKS   UNKNOWN      I0   I1  I2
// 00000000 00000000 00000000 00000000 04000000 19120E1C 01120F00 0E001000
// NM0 NM1  NM2 UV1  UV2 PAD  UNKNOWN  UNKNOWN  UN  NM0      I0   I1  I2
// 0F000E00 100025FF 59000000 1A83F81A 00008A4A 19120112 0E1C1300 14000E00
// 13001400 0E005152 59000000 00000085 000009B8 01120E1C 01121000 0E001400
typedef struct __attribute__((packed)) FACBlock {
    uint32_t unknown0;

    //uint16_t unknown1;

    uint16_t indices[3];    // Vertex indices
    uint16_t normal[3];     // Normals
    uint16_t texture[2];    // UV coords?

    uint16_t padding;

    int32_t unknown2;
    int32_t unknown3;

    uint16_t unknown4;
} FACBlock;

typedef struct FACTriangle {
    // ???????? ???????? ???????? ???????? A0  A1   A2  B0   B1  B2   ???????? ????????
    // 000000EB 0000023B 00090000 1B001B09 09000700 0B000D00 09000700 0B000D00 5A000000
    int32_t block0;
    int32_t block1;
    int32_t block2;
    int32_t block3;

    uint16_t indices[3];
    uint16_t indices1[3]; // again?

    int32_t block5;
    int32_t block6;
} FACTriangle;

typedef struct PIGModel {
    VTXCoord coords[2048];

    unsigned int num_vertices;
    unsigned int num_triangles;

    PLMesh *tri_mesh;
} PIGModel;

PIGModel model;

void load_fac_file(const char *path) {
    FACHeader header;
    memset(&header, 0, sizeof(FACHeader));

    PRINT("\nOpening %s\n", path);

    FILE *file = fopen(path, "r");
    if(!file) {
        PRINT("Failed to load file %s!\n", path);
        return;
    }

    if(fread(&header, sizeof(FACHeader), 1, file) != 1) {
        PRINT("Invalid file header...\n");
        fclose(file);
        return;
    }

    for(int i = 0; i < plArrayElements(header.padding); i++) {
        if(header.padding[0] != 0) {
            PRINT("Invalid FAC file!\n");
            fclose(file);
            return;
        }
    }

    PRINT("num_blocks: %d\n", header.num_blocks);

    PRINT("FacBlock Size = %d\n", (int)sizeof(FACBlock));

    FACBlock block[header.num_blocks];
    if(header.num_blocks != 0) {
        if(fread(block, sizeof(FACBlock), header.num_blocks, file) != header.num_blocks) {
            PRINT("Unexpected block size!\n");
            goto CLEANUP;
        }

        for(unsigned int i = 0; i < header.num_blocks; i++) {
            PRINT("BLOCK %d\n", i);
            PRINT("    indices(%d %d %d)\n", block[i].indices[0], block[i].indices[1], block[i].indices[2]);
            PRINT("    normal(%d %d %d)\n", block[i].normal[0], block[i].normal[1], block[i].normal[2]);
            PRINT("    uv(%d %d)\n", block[i].texture[0], block[i].texture[1]);
        }
    }

    // Something unknown dangling after the blocks...
    fseek(file, 4, SEEK_CUR);

    FACTriangle triangles[2048];
    memset(triangles, 0, sizeof(FACTriangle));
    unsigned int num_triangles = (unsigned int) fread(triangles, sizeof(FACTriangle), 2048, file);
    for(unsigned int i = 0; i < num_triangles; i++) {
        PRINT("%d : I0(%d) I1(%d) I2(%d)\n", i,
              triangles[i].indices[0],
              triangles[i].indices[1],
              triangles[i].indices[2]);
        PRINT("%d : I0(%d) I1(%d) I2(%d)\n", i,
              triangles[i].indices1[0],
              triangles[i].indices1[1],
              triangles[i].indices1[2]);
    }

    model.num_triangles = header.num_blocks + num_triangles;

    PRINT("Triangles: %d\n", model.num_triangles);

    model.tri_mesh = plCreateMesh(
            PL_PRIMITIVE_TRIANGLES,
            PL_DRAW_IMMEDIATE,
            model.num_triangles,
#if 1
            model.num_triangles * 3
#else
            model.num_vertices
#endif
    );
    unsigned int cur_vert = 0;
    for(unsigned int i = 0; i < header.num_blocks; i++, cur_vert++) {
        plSetMeshVertexPosition3f(model.tri_mesh, cur_vert,
                                  model.coords[block[i].indices[0]].x,
                                  model.coords[block[i].indices[0]].y,
                                  model.coords[block[i].indices[0]].z
        );
        plSetMeshVertexColour(model.tri_mesh, cur_vert, plCreateColour4b(PL_COLOUR_RED));
        cur_vert++;
        plSetMeshVertexPosition3f(model.tri_mesh, cur_vert,
                                  model.coords[block[i].indices[1]].x,
                                  model.coords[block[i].indices[1]].y,
                                  model.coords[block[i].indices[1]].z
        );
        plSetMeshVertexColour(model.tri_mesh, cur_vert, plCreateColour4b(PL_COLOUR_GREEN));
        cur_vert++;
        plSetMeshVertexPosition3f(model.tri_mesh, cur_vert,
                                  model.coords[block[i].indices[2]].x,
                                  model.coords[block[i].indices[2]].y,
                                  model.coords[block[i].indices[2]].z
        );
        plSetMeshVertexColour(model.tri_mesh, cur_vert, plCreateColour4b(PL_COLOUR_BLUE));

#if 1
        PRINT(" %d 0(%d %d %d) 1(%d %d %d) 2(%d %d %d)\n",
              i,

              model.coords[block[i].indices[0]].x,
              model.coords[block[i].indices[0]].y,
              model.coords[block[i].indices[0]].z,

              model.coords[block[i].indices[1]].x,
              model.coords[block[i].indices[1]].y,
              model.coords[block[i].indices[1]].z,

              model.coords[block[i].indices[2]].x,
              model.coords[block[i].indices[2]].y,
              model.coords[block[i].indices[2]].z
        );
#endif
    }

#if 1
    for(unsigned int i = 0; i < num_triangles; i++, cur_vert++) {

        plSetMeshVertexPosition3f(model.tri_mesh, cur_vert,
                                  model.coords[triangles[i].indices[0]].x,
                                  model.coords[triangles[i].indices[0]].y,
                                  model.coords[triangles[i].indices[0]].z
        );
        plSetMeshVertexColour(model.tri_mesh, cur_vert, plCreateColour4b(PL_COLOUR_RED));
        cur_vert++;
        plSetMeshVertexPosition3f(model.tri_mesh, cur_vert,
                                  model.coords[triangles[i].indices[1]].x,
                                  model.coords[triangles[i].indices[1]].y,
                                  model.coords[triangles[i].indices[1]].z
        );
        plSetMeshVertexColour(model.tri_mesh, cur_vert, plCreateColour4b(PL_COLOUR_GREEN));
        cur_vert++;
        plSetMeshVertexPosition3f(model.tri_mesh, cur_vert,
                                  model.coords[triangles[i].indices[2]].x,
                                  model.coords[triangles[i].indices[2]].y,
                                  model.coords[triangles[i].indices[2]].z
        );
        plSetMeshVertexColour(model.tri_mesh, cur_vert, plCreateColour4b(PL_COLOUR_BLUE));

#if 1
        PRINT(" %d 0(%d %d %d) 1(%d %d %d) 2(%d %d %d)\n",
              i,

              model.coords[triangles[i].indices[0]].x,
              model.coords[triangles[i].indices[0]].y,
              model.coords[triangles[i].indices[0]].z,

              model.coords[triangles[i].indices[1]].x,
              model.coords[triangles[i].indices[1]].y,
              model.coords[triangles[i].indices[1]].z,

              model.coords[triangles[i].indices[2]].x,
              model.coords[triangles[i].indices[2]].y,
              model.coords[triangles[i].indices[2]].z
        );
        PRINT(" %d 0(%d %d %d) 1(%d %d %d) 2(%d %d %d)\n",
              i,

              model.coords[triangles[i].indices1[0]].x,
              model.coords[triangles[i].indices1[0]].y,
              model.coords[triangles[i].indices1[0]].z,

              model.coords[triangles[i].indices1[1]].x,
              model.coords[triangles[i].indices1[1]].y,
              model.coords[triangles[i].indices[1]].z,

              model.coords[triangles[i].indices[2]].x,
              model.coords[triangles[i].indices[2]].y,
              model.coords[triangles[i].indices[2]].z
        );
#endif
    }
#endif
    plUploadMesh(model.tri_mesh);

    CLEANUP:
    fclose(file);
}

PLMesh *load_vtx_file(const char *path) {
    PRINT("\nOpening %s\n", path);

    FILE *file = fopen(path, "r");
    if(!file) {
        PRINT("Failed to load file %s!\n", path);
        return NULL;
    }

    model.num_vertices = (unsigned int) fread(model.coords, sizeof(VTXCoord), 2048, file);
    if(!model.num_vertices) {
        PRINT("Empty model!\n");
        fclose(file);
        return NULL;
    }

    PRINT("Vertices: %d\n", model.num_vertices);

    PLMesh *pigmesh = plCreateMesh(
            PL_PRIMITIVE_POINTS,
            PL_DRAW_IMMEDIATE,
            0,
            model.num_vertices
    );

    for(unsigned int i = 0; i < model.num_vertices; i++) {
        PRINT("%d : X(%d) Y(%d) Z(%d)\n", i,
              model.coords[i].x,
              model.coords[i].y,
              model.coords[i].z
        );
        plSetMeshVertexPosition3f(pigmesh, i, model.coords[i].x, model.coords[i].y, model.coords[i].z);
        plSetMeshVertexColour(pigmesh, i, plCreateColour4b(PL_COLOUR_RED));
    }

    plUploadMesh(pigmesh);

    return pigmesh;
}

int main(int argc, char **argv) {
    plInitialize(argc, argv, PL_SUBSYSTEM_LOG);
    plClearLog(TITLE);

    PRINT(" = = = = = = = = = = = = = = = = = = = = = = =\n");
    PRINT("   H0G Loader, created by Mark \"hogsy\" Sowden\n");
    PRINT(" = = = = = = = = = = = = = = = = = = = = = = =\n")

    if(argc < 2) {
        PRINT("Arguments:\n");
        PRINT("    -path <file path>        specifies path to model.\n");
        PRINT("    -folder <folder path>    scans a directory and prints out information.\n");
        PRINT("    -review <file path>      will print out information regarding model without displaying it.\n");
    }

    memset(&model, 0, sizeof(PIGModel));

    const char *folder_arg = plGetCommandLineArgument("-folder");
    if(folder_arg && folder_arg[0] != '\0') {
        plScanDirectory(folder_arg, ".fac", load_fac_file);
    }

    const char *scan_arg = plGetCommandLineArgument("-review");
    if(scan_arg && scan_arg[0] != '\0') {

        char vtx_path[PL_SYSTEM_MAX_PATH] = { '\0' };
        snprintf(vtx_path, sizeof(vtx_path), "%s.vtx", scan_arg);
        char fac_path[PL_SYSTEM_MAX_PATH] = { '\0' };
        snprintf(fac_path, sizeof(fac_path), "%s.fac", scan_arg);
        char no2_path[PL_SYSTEM_MAX_PATH] = { '\0' };
        snprintf(no2_path, sizeof(no2_path), "%s.no2", scan_arg);

        load_vtx_file(vtx_path);
        load_fac_file(fac_path);
    }

    const char *path_arg = plGetCommandLineArgument("-path");
    if(path_arg && path_arg[0] != '\0') {
        if(!glfwInit()) {
            plMessageBox(TITLE, "Failed to initialize GLFW!\n");
            return -1;
        }

        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
        if(!window) {
            glfwTerminate();

            plMessageBox(TITLE, "Failed to create window!\n");
            return -1;
        }

        glfwMakeContextCurrent(window);

        plInitialize(argc, argv, PL_SUBSYSTEM_GRAPHICS);

        glfwSetWindowTitle(window, path_arg);

        char vtx_path[PL_SYSTEM_MAX_PATH] = { '\0' };
        snprintf(vtx_path, sizeof(vtx_path), "%s.vtx", path_arg);
        char fac_path[PL_SYSTEM_MAX_PATH] = { '\0' };
        snprintf(fac_path, sizeof(fac_path), "%s.fac", path_arg);
        char no2_path[PL_SYSTEM_MAX_PATH] = { '\0' };
        snprintf(no2_path, sizeof(no2_path), "%s.no2", path_arg);

        PLMesh *meshypiggy = load_vtx_file(vtx_path);
        if(!meshypiggy) {
            PRINT("Invalid mesh!\n");
            return -1;
        }

        load_fac_file(fac_path);

        plSetDefaultGraphicsState();
        plSetClearColour(plCreateColour4b(0, 0, 128, 255));

        PLCamera *camera = plCreateCamera();
        if(!camera) {
            PRINT("Failed to create camera!");
            return -1;
        }
        camera->mode = PL_CAMERAMODE_PERSPECTIVE;
        glfwGetFramebufferSize(window, (int *) &camera->viewport.width, (int *) &camera->viewport.height);
        camera->fov = 90.f;

        plSetCameraPosition(camera, plCreateVector3D(0, 12, -500));

        glPointSize(5.f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        float angles = 0;
        while(!glfwWindowShouldClose(window)) {
            plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH | PL_BUFFER_STENCIL);

            angles += 0.5f;

            // draw stuff start
            plSetupCamera(camera);

            plEnableGraphicsStates(PL_CAPABILITY_DEPTHTEST);
            glDepthFunc(GL_LEQUAL);

            glLoadIdentity();
            glRotatef(angles, 0, 1, 0);
            glRotatef(180.f, 0, 0, 1);

            plDrawMesh(meshypiggy);
            if(model.tri_mesh) {
                plDrawMesh(model.tri_mesh);
            } else {

            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        plDeleteCamera(camera);

        glfwTerminate();
    }

    plShutdown();

    return 0;
}