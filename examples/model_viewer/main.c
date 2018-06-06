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
#include <PL/platform_filesystem.h>

#include <SDL2/SDL.h>

#include <GL/glew.h>

#include "../shared.h"

#define TITLE "Model Viewer"

#define VERSION_MAJOR   0
#define VERSION_MINOR   3

#define WIDTH   800
#define HEIGHT  600

//////////////////////////////////////////

SDL_Window *window = NULL;
void CreateWindow(void) {
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

void DestroyWindow(void) {
    if(window == NULL) {
        return;
    }

    SDL_DestroyWindow(window);
}

/* Displays a simple dialogue window. */
void MessageBox(const char *title, const char *msg, ...) {
    char buf[4096];
    va_list args;
    va_start(args, msg);
    vsprintf(buf, msg, args);
    va_end(args);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, buf, NULL);
}

////////////////////////////////////////////////////////////////////////////////////

void WriteSMDVertex(FILE *file, const PLVertex *vertex) {
    /*             P X  Y  Z  NX NY NZ U  V */
    fprintf(file, "0 %f %f %f %f %f %f %f %f\n",

            vertex->position.x,
            vertex->position.y,
            vertex->position.z,

            vertex->normal.x,
            vertex->normal.y,
            vertex->normal.z,

            vertex->st[0].x,
            vertex->st[0].y);
}

/* writes given model out to Valve's SMD model format */
void WriteSMD(PLModel *model) {
    char body_path[PL_SYSTEM_MAX_PATH];
    snprintf(body_path, sizeof(body_path), "./%s_body.smd", model->name);
    FILE *fout = fopen(body_path, "w");
    if(fout == NULL) {
        printf("%s\n", plGetError());
        exit(EXIT_FAILURE);
    }

    /* header */
    fprintf(fout, "version 1\n\n");

    /* write out the nodes block */
    fprintf(fout, "nodes\n");
    if(model->skeleton.num_bones == 0) {
        /* write out a dummy bone! */
        fprintf(fout, "0 \"root\" -1\n");
    } else {
        /* todo, revisit this so we're correctly connecting child/parent */
        for (unsigned int i = 0; i < model->skeleton.num_bones; ++i) {
            fprintf(fout, "%u %s %d\n", i, model->skeleton.bones[i].name, (int) i - 1);
        }
    }
    fprintf(fout, "end\n\n");

    /* skeleton block */
    fprintf(fout, "skeleton\ntime 0\n");
    if(model->skeleton.num_bones == 0) {
        /* write out dummy bone coords! */
        fprintf(fout, "0 0 0 0 0 0 0\n");
    } else {
        /* todo, print out default coords for each bone */
    }
    fprintf(fout, "end\n\n");

    /* triangles block */
    fprintf(fout, "triangles\n");
    for(unsigned int i = 0; i < model->num_meshes; ++i) {
        PLModelMesh *cur_mesh = &model->meshes[i];
        for(unsigned int j = 0; j < cur_mesh->mesh->num_indices; ) {
            if(cur_mesh->texture == NULL) {
                fprintf(fout, "null\n");
            } else {
                fprintf(fout, "%s\n", cur_mesh->texture->name);
            }
            WriteSMDVertex(fout, &cur_mesh->mesh->vertices[cur_mesh->mesh->indices[j++]]);
            WriteSMDVertex(fout, &cur_mesh->mesh->vertices[cur_mesh->mesh->indices[j++]]);
            WriteSMDVertex(fout, &cur_mesh->mesh->vertices[cur_mesh->mesh->indices[j++]]);
        }
    }
    fprintf(fout, "end\n\n");

    /* and leave a blank line at the end, to keep studiomdl happy */
    fprintf(fout, "\n");

    fclose(fout);
}

////////////////////////////////////////////////////////////////////////////////////

PLCamera *main_camera;

// loads a model in and then frees it
void TempModelLoad(const char *path) {
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
void ProcessKeyboard(void) {
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

    if(state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP]) {

    } else if(state[SDL_SCANCODE_D] || state[SDL_SCANCODE_DOWN]) {

    }

    if(state[SDL_SCANCODE_C]) {
        static bool cull = false;
        if(cull) {
            plSetCullMode(PL_CULL_NONE);
        } else {
            plSetCullMode(PL_CULL_NEGATIVE);
        }
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
    PRINT("  Left   - rotate model\n"                                                       );
    PRINT("  Right  - move model backward / forward\n"                                      );
    PRINT("  Middle - move model up, down, left and right\n\n"                              );
    PRINT("  WASD   - move camera\n"                                                        );
    PRINT("\n-------------------------------------------------------------------------\n\n" );

    plInitialize(argc, argv);
    plSetupLogOutput("./viewer.log");

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

    PLModel *model = plLoadModel(model_path);
    if(model == NULL) {
        PRINT_ERROR("Failed to load model \"%s\"!\n%s", model_path, plGetError());
    }

    if(extract_model) {
        WriteSMD(model);
        return EXIT_SUCCESS;
    }

    CreateWindow();

    plInitializeSubSystems(PL_SUBSYSTEM_GRAPHICS);
    plSetGraphicsMode(PL_GFX_MODE_OPENGL);

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
    //main_camera->viewport.r_w = 320;
    //main_camera->viewport.r_h = 224;

    PLCamera *ui_camera = plCreateCamera();
    if(ui_camera == NULL) {
        PRINT_ERROR("failed to create ui camera!\n");
    }
    ui_camera->mode         = PL_CAMERA_MODE_ORTHOGRAPHIC;
    ui_camera->viewport.w   = WIDTH;
    ui_camera->viewport.h   = HEIGHT;
    ui_camera->near         = 0;
    ui_camera->far          = 1000;

    PLBitmapFont *font = plCreateDefaultBitmapFont();
    if(font == NULL) {
        PRINT_ERROR("%s", plGetError());
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    plSetCullMode(PL_CULL_NONE);

    glEnable(GL_LINE_SMOOTH);

    PLLight light[4];
    memset(&light, 0, sizeof(PLLight) * 4);
    light[0].position   = PLVector3(0, 0, 0);
    light[0].colour     = plCreateColour4f(1.5f, .5f, .5f, 128.f);
    light[0].type       = PL_LIGHT_TYPE_OMNI;

#if 0
    PLTexture *base_texture = plLoadTextureImage("./textures/base_uv.png", PL_TEXTURE_FILTER_NEAREST);
    if(base_texture == NULL) {
        PRINT("failed to load base texture\n");
    }

    plSetModelTexture(model, 0, base_texture);
#endif

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
            angles.x += (nxpos / 50.f);
            angles.y += (nypos / 50.f);
        } else {
            oldlmpos[0] = xpos;
            oldlmpos[1] = ypos;
        }

        // Zoom in and out thing...
        static double oldrmpos[2] = {0, 0};
        if (state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            double nypos = ypos - oldrmpos[1];
            main_camera->position.z += (nypos / 100.f);
        } else {
            oldrmpos[0] = xpos;
            oldrmpos[1] = ypos;
        }

        // panning thing
        static double oldmmpos[2] = {0, 0};
        if(state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
            double nxpos = xpos - oldmmpos[0];
            double nypos = ypos - oldmmpos[1];
            main_camera->position.y += (nypos / 50.f);
            main_camera->position.x -= (nxpos / 50.f);
        } else {
            oldmmpos[0] = xpos;
            oldmmpos[1] = ypos;
        }
        // input handlers end...

        ProcessKeyboard();

        plSetupCamera(main_camera);

        plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH);

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

        const char *vertex_stage = {
                "#version 330\n"
                "layout (location = 0) in vec3 inPosition;\n"
                "layout (location = 1) in vec3 inColor;\n"
                "smooth out vec3 theColor;\n"
                "void main() {\n"
                "gl_Position = vec4(inPosition, 1.0);\n"
                "theColor = inColor;\n"
                "}"
        };

        const char *fragment_stage = {
                "#version 330\n"
        };

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

        plDrawPerspectivePOST(main_camera);

        plSetupCamera(ui_camera);

        plDrawBitmapString(font, 10, 10, 4.f, PLColour(255, 0, 0, 255), "Hello World!\n");

        //plDrawConsole();

        SDL_GL_SwapWindow(window);
    }

    plDeleteModel(model);
    plDeleteCamera(main_camera);

    DestroyWindow();

    plShutdown();

    return EXIT_SUCCESS;
}