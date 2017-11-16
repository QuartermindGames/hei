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
#include <PL/platform_window.h>
#include <PL/platform_input.h>

#include "../shared.h"

#define TITLE "Model Viewer"

#define VERSION_MAJOR   0
#define VERSION_MINOR   2

#define WIDTH   800
#define HEIGHT  600

PLWindow *main_window = NULL;

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
void key_callback(PLWindow* window, bool state, char key) {
    if(window != main_window) {
        return;
    }

    switch(key) {
        default: break;

        case '1': {
            if(state) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                view_mode = VIEW_MODE_LIT;
            }
            break;
        }
        case '2': {
            if(state && (view_mode != VIEW_MODE_WIREFRAME)) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDisable(GL_LIGHTING);
                view_mode = VIEW_MODE_WIREFRAME;
            }
            break;
        }
        case '3': {
            if(state && (view_mode != VIEW_MODE_POINTS)) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                glDisable(GL_LIGHTING);
                view_mode = VIEW_MODE_POINTS;
            }
            break;
        }
        case '4': {
            if(state && (view_mode != VIEW_MODE_WEIGHTS)) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                view_mode = VIEW_MODE_WEIGHTS;
            }
            break;
        }
        case '5': {
            if(state && (view_mode != VIEW_MODE_SKELETON)) {
                view_mode = VIEW_MODE_SKELETON;
            }
            break;
        }
#if 0
        case GLFW_KEY_ESCAPE: {
            if(state) {
                glfwSetWindowShouldClose(window, true);
            }
            break;
        }
#endif
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

    plInitialize(argc, argv, PL_SUBSYSTEM_WINDOW);

    main_window = plCreateWindow(
            "Cyclone Viewer",
            plGetScreenWidth() / 2, plGetScreenHeight() / 2,
            WIDTH, HEIGHT
    );

    plInitialize(argc, argv, PL_SUBSYSTEM_GRAPHICS);

    plSetKeyboardCallback(key_callback);

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
    main_camera->viewport.w = WIDTH;
    main_camera->viewport.h = HEIGHT;

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

    while (plIsRunning()) {
        plProcessInput();

        plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH | PL_BUFFER_STENCIL);

        // input handlers start..
        int xpos, ypos;
        plGetCursorPosition(main_window, &xpos, &ypos);

        // Camera rotation
        static double oldlmpos[2] = {0, 0};
        static PLVector3D angles = { 0, 0 };
        bool state = plGetMouseState(PLINPUT_MOUSE_LEFT);
        if (state) {
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
        state = plGetMouseState(PLINPUT_MOUSE_RIGHT);
        if (state) {
            double nypos = ypos - oldrmpos[1];
            main_camera->position.z += (nypos / 10.f);
        } else {
            oldrmpos[0] = xpos;
            oldrmpos[1] = ypos;
        }

        static double oldmmpos[2] = {0, 0};
        state = plGetMouseState(PLINPUT_MOUSE_MIDDLE);
        if(state) {
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
            default: {

            } break;

            case VIEW_MODE_LIT: {
                glEnable(GL_LIGHTING);
                glShadeModel(GL_FLAT);

                plDrawModel(model);

                glShadeModel(GL_SMOOTH);
                glDisable(GL_LIGHTING);
            } break;

            case VIEW_MODE_WEIGHTS:
            case VIEW_MODE_WIREFRAME: {
                plDrawModel(model);
            } break;

            case VIEW_MODE_SKELETON: {
               // plDrawMesh(cur_model);
               // glDisable(GL_DEPTH_TEST);
               // plDrawMesh(model.skeleton_mesh);
               // glEnable(GL_DEPTH_TEST);
            } break;
        }

        glPopMatrix();

        //plDrawConsole();

        plSwapBuffers(main_window);
    }

    plDeleteModel(model);
    plDeleteCamera(main_camera);
    plDeleteWindow(main_window);

    plShutdown();

    return 0;
#endif
}