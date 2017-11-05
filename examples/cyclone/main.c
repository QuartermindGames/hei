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

#include <PL/platform_math.h>
#include <PL/platform_console.h>
#include <PL/platform_graphics.h>
#include <PL/platform_model.h>

#include <GLFW/glfw3.h>

#include "../shared.h"

#define TITLE "Model Viewer"

#define VERSION_MAJOR   0
#define VERSION_MINOR   2

#define WIDTH   800
#define HEIGHT  600

// loads a model in and then frees it
void load_mdl_temp(const char *path) {
    PLModel *model = plLoadModel(path);
    if(model != NULL) {
        plDeleteModel(model);
    }
}

enum {
    VIEW_MODE_LIT,
    VIEW_MODE_WIREFRAME,
    VIEW_MODE_POINTS,
    VIEW_MODE_WEIGHTS,
    VIEW_MODE_SKELETON
};
int view_mode = VIEW_MODE_WIREFRAME;
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {

    switch(key) {
        default: break;

        case GLFW_KEY_1: {
            if(action == GLFW_PRESS) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                view_mode = VIEW_MODE_LIT;
            }
            break;
        }
        case GLFW_KEY_2: {
            if((action == GLFW_PRESS) && (mode != VIEW_MODE_WIREFRAME)) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDisable(GL_LIGHTING);
                view_mode = VIEW_MODE_WIREFRAME;
            }
            break;
        }
        case GLFW_KEY_3: {
            if((action == GLFW_PRESS) && (mode != VIEW_MODE_POINTS)) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                glDisable(GL_LIGHTING);
                view_mode = VIEW_MODE_POINTS;
            }
            break;
        }
        case GLFW_KEY_4: {
            if((action == GLFW_PRESS) && (mode != VIEW_MODE_WEIGHTS)) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                view_mode = VIEW_MODE_WEIGHTS;
            }
            break;
        }
        case GLFW_KEY_5: {
            if((action == GLFW_PRESS) && (mode != VIEW_MODE_SKELETON)) {
                view_mode = VIEW_MODE_SKELETON;
            }
            break;
        }

        case GLFW_KEY_ESCAPE: {
            if(action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
            break;
        }
    }
}

int main(int argc, char **argv) {
    PRINT("\n " TITLE " : Version %d.%d (" __DATE__ ")\n", VERSION_MAJOR, VERSION_MINOR     );
    PRINT(" Developed by...\n"                                                              );
    PRINT("   Mark \"hogsy\" Sowden (http://talonbrave.info/)\n"                            );
    PRINT("\n"                                                                              );
    PRINT(" Usage:\n"                                                                       );
    PRINT("  Left   - orbit camera\n"                                                       );
    PRINT("  Right  - move camera backward / forward\n"                                     );
    PRINT("  Middle - move camera up, down, left and right\n\n"                             );
    PRINT("\n-------------------------------------------------------------------------\n\n" );

    // Initialize GLFW...

    if (!glfwInit()) {
        PRINT_ERROR("Failed to initialize GLFW!\n");
    }

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Cyclone Viewer", NULL, NULL);
    if (!window) {
        glfwTerminate();

        PRINT_ERROR("Failed to create window!\n");
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    // And now for ours...

    plInitialize(argc, argv, PL_SUBSYSTEM_GRAPHICS);

    plSetDefaultGraphicsState();
    plSetClearColour(plCreateColour4b(0, 0, 128, 255));

    plSetupConsole(1);
    plShowConsole(true);
    plSetConsoleColour(1, plCreateColour4b(128, 0, 0, 128));

    plParseConsoleString("cmds");
    plParseConsoleString("vars");

    PLCamera *main_camera = plCreateCamera();
    if (main_camera == NULL) {
        PRINT_ERROR("Failed to create camera!\n");
    }
    main_camera->mode = PLCAMERA_MODE_PERSPECTIVE;
    main_camera->fov = 90.f;
    main_camera->position = plCreateVector3D(0, 12, -500);
    glfwGetFramebufferSize(window,
                           (int*)&main_camera->viewport.w,
                           (int*)&main_camera->viewport.h);

#if 0
    plScanDirectory("./Models/", "mdl", load_mdl_temp, false);

    return EXIT_SUCCESS;
#else
    PLModel *model = plLoadModel("./Models/medkit.mdl");
    if(model == NULL) {
        PRINT_ERROR("Failed to load model!\n");
    }

    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LINE_SMOOTH);

    PLLight light[4];
    memset(&light, 0, sizeof(PLLight) * 4);
    light[0].position   = plCreateVector3D(0, 12.f, -800.f);
    light[0].colour     = plCreateColour4f(1.5f, .5f, .5f, 128.f);
    light[0].type       = PLLIGHT_TYPE_OMNI;

    glPointSize(5.f);
    glLineWidth(2.f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH | PL_BUFFER_STENCIL);

        // input handlers start..
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Camera rotation
        static double oldlmpos[2] = {0, 0};
        static PLVector3D angles = { 0, 0 };
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (state == GLFW_PRESS) {
            double nxpos = xpos - oldlmpos[0];
            double nypos = ypos - oldlmpos[1];
            angles.x += (nxpos / 100.f);
            angles.y += (nypos / 100.f);
        } else {
            oldlmpos[0] = xpos;
            oldlmpos[1] = ypos;
        }

        // Zoom in and out thing...
        static double oldrmpos[2] = {0, 0};
        state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        if (state == GLFW_PRESS) {
            double nypos = ypos - oldrmpos[1];
            main_camera->position.z += (nypos / 10.f);
        } else {
            oldrmpos[0] = xpos;
            oldrmpos[1] = ypos;
        }

        static double oldmmpos[2] = {0, 0};
        state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
        if(state == GLFW_PRESS) {
            double nxpos = xpos - oldmmpos[0];
            double nypos = ypos - oldmmpos[1];
            main_camera->position.y += (nypos / 5.f);
            main_camera->position.x -= (nxpos / 5.f);
        } else {
            oldmmpos[0] = xpos;
            oldmmpos[1] = ypos;
        }
        // input handlers end...

        plSetupCamera(main_camera);

        glLoadIdentity();

        glPushMatrix();
        glRotatef(angles.y, 1, 0, 0);
        glRotatef(angles.x, 0, 1, 0);
        glRotatef(angles.z + 180.f, 0, 0, 1);

        switch (view_mode) {
            default:
                break;

            case VIEW_MODE_LIT: {
                glEnable(GL_LIGHTING);
                glShadeModel(GL_FLAT);

                plDrawModel(model);

                glShadeModel(GL_SMOOTH);
                glDisable(GL_LIGHTING);
                break;
            }

            case VIEW_MODE_WEIGHTS:
            case VIEW_MODE_WIREFRAME: {
                plDrawModel(model);
                break;
            }

            case VIEW_MODE_SKELETON: {
               // plDrawMesh(cur_model);
               // glDisable(GL_DEPTH_TEST);
               // plDrawMesh(model.skeleton_mesh);
               // glEnable(GL_DEPTH_TEST);
            }
        }

        glPopMatrix();

        //plDrawConsole();

        glfwSwapBuffers(window);
    }

    plDeleteModel(model);
    plDeleteCamera(main_camera);

    plShutdown();

    glfwTerminate();

    return 0;
#endif
}