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
#include <PL/platform_graphics_font.h>
#include <PL/platform_graphics_camera.h>
#include <PL/platform_model.h>

#include <SDL2/SDL.h>

#include <GL/glew.h>
#include <PL/platform_filesystem.h>

#include "../shared.h"

#define TITLE "Model Viewer"

#define VERSION_MAJOR   0
#define VERSION_MINOR   2

#define WIDTH   800
#define HEIGHT  600

//////////////////////////////////////////

SDL_Window *window = NULL;
void create_window(void) {
    window = SDL_CreateWindow(
            TITLE,
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            WIDTH, HEIGHT,
            SDL_WINDOW_OPENGL
    );
    if(window == NULL) {
        PRINT_ERROR("SDL2: %s\n", SDL_GetError());
    }

    PLGraphicsContext *context = SDL_GL_CreateContext(window);
    if(context == NULL) {
        PRINT_ERROR("SDL2: %s\n", SDL_GetError());
    }

    SDL_GL_MakeCurrent(window, context);
    if(SDL_GL_SetSwapInterval(-1) == -1) {
        SDL_GL_SetSwapInterval(1);
    }

    SDL_DisableScreenSaver();
}

void destroy_window(void) {
    if(window == NULL) {
        return;
    }

    SDL_DestroyWindow(window);
}

/* Displays a simple dialogue window. */
void message_box(const char *title, const char *msg, ...) {
    char buf[4096];
    va_list args;
    va_start(args, msg);
    vsprintf(buf, msg, args);
    va_end(args);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, buf, NULL);
}

//////////////////////////////////////////

/* writes given model out to Valve's SMD model format */
void write_smd(PLModel *model) {
    //FILE *fout = fopen("./")
    //
}

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
void process_keyboard(void) {
    const uint8_t *state = SDL_GetKeyboardState(NULL);
    if(state[SDL_SCANCODE_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        view_mode = VIEW_MODE_LIT;
    } else if(state[SDL_SCANCODE_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_LIGHTING);
        view_mode = VIEW_MODE_WIREFRAME;
    } else if(state[SDL_SCANCODE_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glDisable(GL_LIGHTING);
        view_mode = VIEW_MODE_POINTS;
    } else if(state[SDL_SCANCODE_4]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        view_mode = VIEW_MODE_WEIGHTS;
    } else if(state[SDL_SCANCODE_5]) {
        view_mode = VIEW_MODE_SKELETON;
    }

    if(state[SDL_SCANCODE_ESCAPE]) {
        exit(EXIT_SUCCESS);
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

    plInitialize(argc, argv);
    plSetupLogOutput("./viewer.log");

    create_window();

    if(argc < 2) {
        PRINT(" model_viewer -<optional mode> <model path>\n");
        PRINT("  -smd    : write model out to an SMD\n");
        PRINT("  -scan   : scans through a directory, must provide extension as follow-up argument to model path\n");
        return EXIT_SUCCESS;
    }

    char model_path[PL_SYSTEM_MAX_PATH] = {'\0'};
    char model_extension[12] = {'\0'};

    bool extract_model = false;
    bool scan_directory = false;

    for(int i = 1; i < argc; ++i) {
        if(argv[i] == NULL || argv[i][0] == '\0') {
            continue;
        }

        if(pl_strcasecmp("-smd", argv[i]) == 0) {
            extract_model = true;
        } else if(pl_strcasecmp("-scan", argv[i]) == 0) {
            scan_directory = true;
        } else if(argv[i][0] != '-') { // probably model path, probably
            if(model_path[0] == '\0') {
                strncpy(model_path, argv[i], sizeof(model_path));
            } else { // probably extension, probably
                strncpy(model_extension, argv[i], sizeof(model_extension));
            }
        }
    }

    if(model_path[0] == '\0') {
        printf("invalid path for model, aborting!\n");
        return EXIT_FAILURE;
    }

    if(scan_directory) {
        if(model_extension[0] == '\0') {
            printf("invalid extension for scan, aborting!\n");
            return EXIT_FAILURE;
        }

        plScanDirectory(model_path, model_extension, load_mdl_temp, false);
        return EXIT_SUCCESS;
    }

    /* debris3
     * debris2
     * armor1
     * shellcasing2sm
     * medkit
     * lamp7
     * lamp
     * medlab
     * lion
     * ctable
     * throne
     * lamp4
     */

    PLModel *model = plLoadModel(model_path);
    if(model == NULL) {
        PRINT_ERROR("Failed to load model \"%s\"!\n", model_path);
    }

    if(extract_model) {
        write_smd(model);
        return EXIT_SUCCESS;
    }

    plInitializeSubSystems(PL_SUBSYSTEM_GRAPHICS);
    plSetGraphicsMode(PL_GFX_MODE_OPENGL);

    plSetDefaultGraphicsState();
    plSetClearColour(PLColour(0, 0, 128, 255));

    plSetupConsole(1);
    plShowConsole(true);
    plSetConsoleColour(1, PLColour(128, 0, 0, 128));

    plParseConsoleString("cmds");
    plParseConsoleString("vars");

    PLCamera *main_camera = plCreateCamera();
    if (main_camera == NULL) {
        PRINT_ERROR("Failed to create camera!\n");
    }
    main_camera->mode = PL_CAMERA_MODE_PERSPECTIVE;
    main_camera->fov = 90.f;
    main_camera->position = PLVector3(0, 12, -500);
    main_camera->viewport.w = WIDTH;
    main_camera->viewport.h = HEIGHT;

    PLCamera *ui_camera = plCreateCamera();
    if(ui_camera == NULL) {
        PRINT_ERROR("failed to create ui camera!\n");
    }
    ui_camera->mode = PL_CAMERA_MODE_ORTHOGRAPHIC;
    ui_camera->viewport.w = WIDTH;
    ui_camera->viewport.h = HEIGHT;

#if 0
    PLBitmapFont *font = plCreateBitmapFont("./fonts/console.font");
    if(font == NULL) {
        PRINT_ERROR("%s", plGetError());
    }
#endif

    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LINE_SMOOTH);

    PLLight light[4];
    memset(&light, 0, sizeof(PLLight) * 4);
    light[0].position   = PLVector3(0, 0, 0);
    light[0].colour     = plCreateColour4f(1.5f, .5f, .5f, 128.f);
    light[0].type       = PL_LIGHT_TYPE_OMNI;

    glPointSize(5.f);
    glLineWidth(2.f);

    while (plIsRunning()) {
        SDL_PumpEvents();

        // input handlers start..
        int xpos, ypos;
        unsigned int state = SDL_GetMouseState(&xpos, &ypos);

        // Camera rotation
        static double oldlmpos[2] = {0, 0};
        static PLVector3 angles = { 0, 0 };
        if (state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
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
        if (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            double nypos = ypos - oldrmpos[1];
            main_camera->position.z += (nypos / 10.f);
        } else {
            oldrmpos[0] = xpos;
            oldrmpos[1] = ypos;
        }

        // panning thing
        static double oldmmpos[2] = {0, 0};
        if(state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
            double nxpos = xpos - oldmmpos[0];
            double nypos = ypos - oldmmpos[1];
            main_camera->position.y += (nypos / 5.f);
            main_camera->position.x -= (nxpos / 5.f);
        } else {
            oldmmpos[0] = xpos;
            oldmmpos[1] = ypos;
        }
        // input handlers end...

        process_keyboard();

        plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH | PL_BUFFER_STENCIL);

        plSetupCamera(main_camera);

        glLoadIdentity();
        glPushMatrix();
        glRotatef(angles.y, 1, 0, 0);
        glRotatef(angles.x, 0, 1, 0);
        glRotatef(angles.z, 0, 0, 1);

        //light[0].position = PLVector3(0, 10.f, 0);
        //plApplyModelLighting(model, &light[0], PLVector3(20.f, 50.f, 80.f));
        //plApplyModelLighting(model, &light[1], PLVector3(0, 0, 0));
        //plApplyModelLighting(model, &light[2], PLVector3(0, 0, 0));
        //plApplyModelLighting(model, &light[3], PLVector3(0, 0, 0));

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

        plSetupCamera(ui_camera);

        //plSetBlendMode(PL_BLEND_ADDITIVE);
        //plDrawBitmapString(font, 10, 10, 4.f, PLColour(255, 0, 0, 255), "Hello World!\n");
        //plSetBlendMode(PL_BLEND_DISABLE);

        //plDrawConsole();

        SDL_GL_SwapWindow(window);
    }

    plDeleteModel(model);
    plDeleteCamera(main_camera);

    destroy_window();

    plShutdown();

    return 0;
}