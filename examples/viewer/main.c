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

//#define COMMAND_ONLY    /* uncomment if you want the viewer window */

#include <PL/platform_math.h>
#include <PL/platform_console.h>
#ifndef COMMAND_ONLY
#include <PL/platform_graphics.h>
#include <PL/platform_graphics_font.h>
#include <PL/platform_graphics_camera.h>
#endif
#include <PL/platform_model.h>
#include <PL/platform_filesystem.h>

#ifndef COMMAND_ONLY
#include <SDL2/SDL.h>
#include <GL/glew.h>
#endif

#include "../shared.h"

#define TITLE "Model Viewer"

#define VERSION_MAJOR   0
#define VERSION_MINOR   3

#ifndef COMMAND_ONLY

#define WIDTH   800
#define HEIGHT  600

#define CENTER_X    (WIDTH >> 1)
#define CENTER_Y    (HEIGHT >> 1)

static bool use_mouse_look = false;

//////////////////////////////////////////

SDL_Window *window = NULL;
static void CreateWindow(void) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

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

static void DestroyWindow(void) {
    if(window == NULL) {
        return;
    }

    SDL_DestroyWindow(window);
}

/* Displays a simple dialogue window. */
static void MessageBox(const char *title, const char *msg, ...) {
    char buf[4096];
    va_list args;
    va_start(args, msg);
    vsprintf(buf, msg, args);
    va_end(args);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, buf, NULL);
}

////////////////////////////////////////////////////////////////////////////////////

static PLCamera *main_camera;

enum {
    VIEW_MODE_LIT,
    VIEW_MODE_WIREFRAME,
    VIEW_MODE_POINTS,
    VIEW_MODE_WEIGHTS,
    VIEW_MODE_SKELETON
};
int view_mode = VIEW_MODE_WIREFRAME;
static void ProcessKeyboard(void) {
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

    if(state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT]) {
        main_camera->angles.x -= 4.f;
    } else if(state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
        main_camera->angles.x += 4.f;
    }

    PLVector3 left, up, forward;
    plAnglesAxes(main_camera->angles, &left, &up, &forward);
    if(state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP]) {
        main_camera->position = plVector3Add(main_camera->position, main_camera->forward);
        //main_camera->position = plVector3Scale(main_camera->position, PLVector3(0.5f, 0.5f, 4.f));
    } else if(state[SDL_SCANCODE_D] || state[SDL_SCANCODE_DOWN]) {
        main_camera->position = plVector3Subtract(main_camera->position, main_camera->forward);
        //main_camera->position = plVector3Scale(main_camera->position, PLVector3(4.f, 4.f, 4.f));
    }

    static unsigned int toggle_delay = 0;
    if(toggle_delay == 0) {
        if(state[SDL_SCANCODE_Q]) {
            use_mouse_look = !use_mouse_look;
            SDL_ShowCursor(!use_mouse_look);
            main_camera->position = PLVector3(0, 2, -50);
            main_camera->angles = PLVector3(0, 0, 0);

            toggle_delay = 10;
        }

        if(state[SDL_SCANCODE_C]) {
            static bool cull = false;
            if(cull) {
                plSetCullMode(PL_CULL_NONE);
            } else {
                plSetCullMode(PL_CULL_NEGATIVE);
            }

            toggle_delay = 10;
        }
    } else {
        toggle_delay--;
    }

    if(state[SDL_SCANCODE_ESCAPE]) {
        exit(EXIT_SUCCESS);
    }
}

#endif

// loads a model in and then frees it
static void TempModelLoad(const char *path) {
    PLModel *model = plLoadModel(path);
    if(model != NULL) {
        plDeleteModel(model);
    }
}

int main(int argc, char **argv) {
    PRINT("\n " TITLE " : Version %d.%d (" __DATE__ ")\n", VERSION_MAJOR, VERSION_MINOR     );
    PRINT(" Developed by...\n"                                                              );
    PRINT("   Mark \"hogsy\" Sowden (http://talonbrave.info/)\n"                            );
    PRINT("\n"                                                                              );
    PRINT(" Usage:\n"                                                                       );
    PRINT("  Left   - rotate model\n"                                                       );
    PRINT("  Right  - move model backward / forward\n"                                      );
    PRINT("  Middle - move model up, down, left and right\n\n"                              );
    PRINT("  WASD   - move camera\n"                                                        );
    PRINT("\n-------------------------------------------------------------------------\n\n" );

    plInitialize(argc, argv);
    plSetupLogOutput("./viewer.log");

    if(argc < 2) {
        PRINT(" viewer -<optional mode> <model path>\n");
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

        plScanDirectory(model_path, model_extension, TempModelLoad, false);
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

#ifndef COMMAND_ONLY
    CreateWindow();

    plInitializeSubSystems(PL_SUBSYSTEM_GRAPHICS);
    plSetGraphicsMode(PL_GFX_MODE_OPENGL);
#endif

    PLModel *model = plLoadModel(model_path);
    if(model == NULL) {
        PRINT_ERROR("Failed to load model \"%s\"!\n%s", model_path, plGetError());
    }

    if(extract_model) {
        if(!plWriteModel("output", model, PL_MODEL_OUTPUT_SMD)) {
            PRINT_ERROR("Failed to write model \"%s\"!\n%s\n", model->name, plGetError());
        }
        return EXIT_SUCCESS;
    }

#ifndef COMMAND_ONLY
    plSetClearColour(PLColour(0, 0, 128, 255));

    plSetupConsole(1);
    plShowConsole(true);
    plSetConsoleColour(1, PLColour(128, 0, 0, 128));

    main_camera = plCreateCamera();
    if (main_camera == NULL) {
        PRINT_ERROR("Failed to create camera!\n");
    }
    main_camera->mode = PL_CAMERA_MODE_PERSPECTIVE;
    main_camera->fov = 75.f;
    main_camera->position = PLVector3(0, 2, -50);
    main_camera->viewport.w = WIDTH;
    main_camera->viewport.h = HEIGHT;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    plSetCullMode(PL_CULL_NONE);

    glEnable(GL_LINE_SMOOTH);

    PLLight light[4];
    memset(&light, 0, sizeof(PLLight) * 4);
    light[0].position   = PLVector3(0, 0, 0);
    light[0].colour     = plCreateColour4f(1.5f, .5f, .5f, 128.f);
    light[0].type       = PL_LIGHT_TYPE_OMNI;

    PLImage uv_chart;
    memset(&uv_chart, 0, sizeof(PLImage));
    uv_chart.levels         = 1;
    uv_chart.width          = 2;
    uv_chart.height         = 2;
    uv_chart.colour_format  = PL_COLOURFORMAT_RGB;
    uv_chart.format         = PL_IMAGEFORMAT_RGB8;
    uv_chart.size           = uv_chart.width * uv_chart.height * 3;
    uv_chart.data           = pl_calloc(uv_chart.levels, sizeof(uint8_t*));
    uv_chart.data[0]        = pl_malloc(uv_chart.size);

    unsigned char uv_map_layout[12]={
            255,0  ,255,
            0  ,0  ,0  ,
            0  ,0  ,0  ,
            0  ,0  ,0
    };
    memcpy(uv_chart.data[0], uv_map_layout, uv_chart.size);

    PLTexture *uv_texture = plCreateTexture();
    plUploadTextureImage(uv_texture, &uv_chart);

    plFreeImage(&uv_chart);

    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        if(model->meshes[i].texture == NULL) {
            model->meshes[i].texture = uv_texture;
        }
    }

    /* compile shaders */

    const char *vertex_stage = {
            "void main() {"
            "   gl_Position = ftransform();"
            "}"
    };

    const char *fragment_stage = {
            "void main() {"
            "   gl_FragColor = vec4(1,1,1,1);"
            "}"
    };

    PLShaderProgram *program = plCreateShaderProgram();
    plRegisterShaderStageFromMemory(program, vertex_stage, strlen(vertex_stage), PL_SHADER_TYPE_VERTEX);
    plRegisterShaderStageFromMemory(program, fragment_stage, strlen(fragment_stage), PL_SHADER_TYPE_FRAGMENT);

    plLinkShaderProgram(program);

    plSetShaderProgram(program);

    /* done, now for main rendering loop! */

    while (plIsRunning()) {
        SDL_PumpEvents();

        // input handlers start..
        int xpos, ypos;
        unsigned int state = SDL_GetMouseState(&xpos, &ypos);

        static PLVector3 object_angles = {0, 0};
        if(use_mouse_look) {
            object_angles = PLVector3(0, 0, 0);

            double n_pos[2] = { xpos - CENTER_X, ypos - CENTER_Y };
            main_camera->angles.x += (n_pos[0] / 10.f);
            main_camera->angles.y += (n_pos[1] / 10.f);
            main_camera->angles.y = plClamp(-90, main_camera->angles.y, 90);

            SDL_WarpMouseInWindow(window, CENTER_X, CENTER_Y);
        } else {
            // Camera rotation
            static double old_left_pos[2] = {0, 0};
            if (state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                double n_x_pos = xpos - old_left_pos[0];
                double n_y_pos = ypos - old_left_pos[1];
                object_angles.x += (n_x_pos / 50.f);
                object_angles.y += (n_y_pos / 50.f);
            } else {
                old_left_pos[0] = xpos;
                old_left_pos[1] = ypos;
            }

            // Zoom in and out thing...
            static double old_right_pos[2] = {0, 0};
            if (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                double n_y_pos = ypos - old_right_pos[1];
                main_camera->position.z += (n_y_pos / 100.f);
            } else {
                old_right_pos[0] = xpos;
                old_right_pos[1] = ypos;
            }

            // panning thing
            static double old_middle_pos[2] = {0, 0};
            if (state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
                double n_x_pos = xpos - old_middle_pos[0];
                double n_y_pos = ypos - old_middle_pos[1];
                main_camera->position.y += (n_y_pos / 50.f);
                main_camera->position.x -= (n_x_pos / 50.f);
            } else {
                old_middle_pos[0] = xpos;
                old_middle_pos[1] = ypos;
            }
        }
        // input handlers end...

        ProcessKeyboard();

        plSetupCamera(main_camera);

        plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH);

        glLoadIdentity();
        glPushMatrix();
        glRotatef(object_angles.y, 1, 0, 0);
        glRotatef(object_angles.x, 0, 1, 0);
        glRotatef(object_angles.z, 0, 0, 1);

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
                plDrawModelSkeleton(model);
            } break;
        }

        glPopMatrix();

        SDL_GL_SwapWindow(window);
    }

    plDeleteModel(model);
    plDeleteCamera(main_camera);

    DestroyWindow();
#endif

    plShutdown();

    return EXIT_SUCCESS;
}