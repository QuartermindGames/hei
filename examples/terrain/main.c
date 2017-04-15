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

#include <GLFW/glfw3.h>

#define TITLE "terrain"
#define PRINT(...) printf(__VA_ARGS__); plWriteLog(TITLE, __VA_ARGS__);
#define WIDTH 1024
#define HEIGHT 768

typedef struct PIGCoord {
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t padding;
} PIGCoord;

PLMesh *load_vtx_file(const char *path) {
    FILE *file = fopen(path, "r");
    if(!file) {
        PRINT("Failed to load file %s!\n", path);
        return NULL;
    }

    PIGCoord coords[2048];
    uint num_verts = (uint) fread(coords, sizeof(PIGCoord), 2048, file);
    if(!num_verts) {
        PRINT("Empty model!\n");
        fclose(file);
        return NULL;
    }

    PLMesh *pigmesh = plCreateMesh(
            PL_PRIMITIVE_POINTS,
            PL_DRAW_IMMEDIATE,
            0,
            num_verts
    );

    for(unsigned int i = 0; i < num_verts; i++) {
        plSetMeshVertexPosition3f(pigmesh, i, coords[i].x, coords[i].y, coords[i].z);
        plSetMeshVertexColour(pigmesh, i, plCreateColour4b(PL_COLOUR_RED));
    }

    plUploadMesh(pigmesh);

    return pigmesh;
}

#define VTX_PATH "./models/vtx/sp_hi.VTX"

int main(int argc, char **argv) {
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

    plInitialize(argc, argv, PL_SUBSYSTEM_IMAGE | PL_SUBSYSTEM_GRAPHICS | PL_SUBSYSTEM_LOG);
    plClearLog(TITLE);

    plSetDefaultGraphicsState();
    plSetClearColour(plCreateColour4b(0, 0, 128, 255));

    plEnableGraphicsStates(PL_CAPABILITY_DEPTHTEST);

    PLCamera *camera = plCreateCamera();
    if(!camera) {
        PRINT("Failed to create camera!");
        return -1;
    }
    camera->mode = PL_CAMERAMODE_PERSPECTIVE;
    glfwGetFramebufferSize(window, (int *) &camera->viewport.width, (int *) &camera->viewport.height);
    camera->fov = 90.f;

    plSetCameraPosition(camera, plCreateVector3D(0, 12, -500));

    PLMesh *meshypiggy = load_vtx_file(VTX_PATH);
    if(!meshypiggy) {
        PRINT("Invalid mesh!\n");
        return -1;
    }

    glPointSize(2.f);

    float angles = 0;

    while(!glfwWindowShouldClose(window)) {
        plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH | PL_BUFFER_STENCIL);

        angles += 0.5f;

        // draw stuff start
        plSetupCamera(camera);

        glLoadIdentity();
        glRotatef(angles, 0, 1, 0);

        plDrawMesh(meshypiggy);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    plDeleteCamera(camera);

    glfwTerminate();

    plShutdown();

    return 0;
}