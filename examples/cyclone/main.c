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

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

#include <GLFW/glfw3.h>
#include <PL/platform_filesystem.h>
#include <PL/platform_model.h>

#include "../shared.h"

#define TITLE "Cyclone Example"

#define VERSION_MAJOR   0
#define VERSION_MINOR   1

#define WIDTH   800
#define HEIGHT  600

//#define DEBUG_VERSIONS

/////////////////////////////////////////////////////////////////////////

/* There seem to be two seperate model formats
 * used within the game, those that are static
 * and then a completely different type of header
 * for those that aren't.
 *
 * The texture names don't used fixed sizes in either,
 * in the static model the length of these is declared
 * but it doesn't appear this is the case in the animated
 * model format; the animated model format also seems
 * to support multiple textures.
 *
 * The first byte in the model format, is a flag which appears to
 * control how the model will be displayed within the game.
 *
 * Within medkit.mdl
 *  OFFSET ?? is X coord of vertex 1
 *  OFFSET 66 is Y coord of vertex 1
 *  OFFSET 6A is Z coord of vertex 1
 *  OFFSET 6E is X coord of vertex 2
 *  OFFSET 72 is Y coord of vertex 2
 *
 *  3 bytes between each vertex coord
 *  12 bytes for each coord set?
 *
 *  First 32bit int after texture name
 *  appears to be number of vertices
 *
 *  ?? ?? X  ?? ?? ?? Y  ?? ?? ?? Z  ??
 *  7E 71 41 3F 4C 1E 0D BC 9F 3C A8 3F
 *
 *  ????X X  ????Y Y  ????Z Z
 *  7E71413F 4C1E0DBC 9F3CA83F
 */

#define MAX_MODEL_NAME      128
#define MAX_TEXTURE_NAME    64

typedef struct __attribute__((packed)) MDLVertex {
    uint8_t unknown0[2];
    int8_t x_;
    int8_t x;
    uint8_t unknown1[2];
    int8_t y_;
    int8_t y;
    uint8_t unknown2[2];
    int8_t z_;
    int8_t z;
} MDLVertex;

// 04:00:00:00:B4:BC:79:00:00:00:00:00:00:00:00:00:
// BC:BC:79:00:1C:00:1F:00:1B:00:19:00:57:D0:76:00:
// 66:42:1B:00:B0:21:78:00:70:E4:19:00:5C:35:6C:00:
// A1:D5:1C:00:9C:48:6A:00:36:DF:1E:00
// Quads are 60 bytes long; immediately follow vertices

// 03:00:00:00:14:BD:79:00:00:00:00:00:01:00:00:00:
// 1A:BD:79:00:05:00:01:00:06:00:E6:25:12:00:A1:DE:
// 22:00:92:CB:08:00:09:C3:17:00:B6:65:22:00:D8:E0:
// 09:00
// Triangles are 50 bytes long

typedef struct MDLFace {
    uint8_t num_indices;
    uint16_t indices[5];
} MDLFace;

#if 0
    uint8_t unknown0;  // typically seeing this as 4?
    // could be number of verts per face?
    uint8_t unknown000;
    uint16_t unknown00;

    uint8_t unknown1[16]; // completely no clue for now!

    uint16_t indices[4];

    uint8_t unknown2[32]; // no clue either!
#endif

#if defined(DEBUG_VERSIONS)
static int version_position = 0;
int versions[32];
#endif

enum {
    MDL_FLAG_FLAT       = (1 << 0),
    MDL_FLAG_UNLIT      = (1 << 1),
    MDL_FLAG_UNKNOWN0   = (1 << 2),
};

PLMesh *load_mdl(const char *path) {
    PRINT("\nOpening %s...\n", path);

    FILE *file = fopen(path, "rb");
    if(file == NULL) {
        PRINT_ERROR("Failed to load %s!\n", path);
    }

#define ABORT_LOAD(...) printf(__VA_ARGS__); fclose(file); return NULL

    // Check which flags have been set for this particular mesh
    int flags = fgetc(file);
    PRINT("   flags: %d - ", flags);
    if(flags & MDL_FLAG_FLAT) {
        PRINT("flat ");
    }
    if(flags & MDL_FLAG_UNLIT) {
        PRINT("unlit ");
    }
    if(!(flags & MDL_FLAG_FLAT) && !(flags & MDL_FLAG_UNLIT)) {
        PRINT("gouraud ");
    }
    PRINT("\n");

    uint32_t texture_name_length = 0;
    fread(&texture_name_length, sizeof(uint32_t), 1, file);
    if(texture_name_length > MAX_TEXTURE_NAME || texture_name_length == 0) {
        ABORT_LOAD("Odd texture name length returned, %d!\n", texture_name_length);
    }
    char *texture_name = malloc(texture_name_length);
    if(texture_name == NULL) {
        ABORT_LOAD("Failed to allocate memory for storing texture name!\n");
    }
    fread(texture_name, 1, texture_name_length, file);
    PRINT("   texture_name_length: %d\n", texture_name_length);
    PRINT("   texture:             %s\n", texture_name);
    //strncpy(out->texture_name, texture_name, texture_name_length);
    free(texture_name);

    uint16_t num_vertices;
    fread(&num_vertices, sizeof(uint16_t), 1, file);
    if(num_vertices == 0) {
        ABORT_LOAD("Invalid number of vertices!\n");
    }
    PRINT("   num_vertices: %d\n", num_vertices);

    // skip over two unknown bytes...
    fseek(file, 2, SEEK_CUR);

    uint32_t num_faces;
    fread(&num_faces, sizeof(uint32_t), 1, file);
    if(num_faces == 0) {
        ABORT_LOAD("Invalid number of quads!\n");
    }

    MDLVertex vertices[num_vertices];
    if(fread(vertices, sizeof(MDLVertex), num_vertices, file) != num_vertices) {
        ABORT_LOAD("Failed to load all the vertices for this model, fuck!\n");
    }

    PRINT("Loading faces (%d)...\n", num_faces);
#if 0
    MDLQuad quads[num_faces];
    if(fread(quads, sizeof(MDLQuad), num_faces, file) != num_faces) {
        // this assumes all faces within the model are
        // quads, which has since been proven wrong...
        ABORT_LOAD("Failed to load all the quads!\n");
    }
#else   // Models in Requiem support both quads and triangles? And pentagons too, apparently (god have mercy on us all)
        // holy tits batman, we've got hexagons!
    unsigned int num_triangles = 0;

    MDLFace faces[num_faces];
    memset(faces, 0, sizeof(MDLFace) * num_faces);
    for(unsigned int i = 0; i < num_faces; ++i) {
        long pos = ftell(file);
        fread(&faces[i].num_indices, sizeof(uint32_t), 1, file);
        if(faces[i].num_indices < 3 || faces[i].num_indices > 5) {
            ABORT_LOAD("Invalid number of vertices, %d, required for face at index %d! (offset: %ld)\n",
                       faces[i].num_indices, i, ftell(file));
        }
        num_triangles += faces[i].num_indices - 2;

        fseek(file, 16, SEEK_CUR); // skip over unknown bytes for now
        fread(faces[i].indices, sizeof(uint16_t), faces[i].num_indices, file);

        if(faces[i].num_indices == 5) {
            fseek(file, 42, SEEK_CUR); // skip over unknown bytes for now
        } else if(faces[i].num_indices == 4) {
            fseek(file, 32, SEEK_CUR); // skip over unknown bytes for now
        } else if(faces[i].num_indices == 3) {
            fseek(file, 24, SEEK_CUR); // skip over unknown bytes for now
        }

        long npos = ftell(file);
        PRINT(" Read %ld bytes for face %d (indices %d)\n", npos - pos, i, faces[i].num_indices);
    }
    PRINT("%d triangles in total\n", num_triangles);
#endif

    fclose(file);

#if 1
    PLMesh *out = plCreateMesh(PLMESH_POINTS, PL_DRAW_IMMEDIATE, num_triangles, num_vertices);
    if(out == NULL) {
        PRINT_ERROR(plGetError());
    }

    PRINT("\nVertices...\n");
    srand(num_vertices);
    for(unsigned int i = 0; i < num_vertices; ++i) {
        PRINT("%d - X: %d Y: %d Z: %d\n", i, vertices[i].x_, vertices[i].y_, vertices[i].z_);
#if 0
        plSetMeshVertexPosition3f(out, i,
                                  (vertices[i].x_ + vertices[i].x) / 10,
                                  (vertices[i].y_ + vertices[i].y) / 10,
                                  (vertices[i].z_ + vertices[i].z) / 10);
#else
        plSetMeshVertexPosition3f(out, i,
                                  vertices[i].x_ * vertices[i].x,
                                  vertices[i].y_ * vertices[i].y,
                                  vertices[i].z_ * vertices[i].z);
#endif
        plSetMeshVertexColour(out, i, plCreateColour4b(
                (uint8_t) (rand() % 255), (uint8_t) (rand() % 255), (uint8_t) (rand() % 255), 255)
        );
    }
#else
    PLMesh *out = plCreateMesh(PLMESH_POINTS, PL_DRAW_IMMEDIATE, num_triangles, num_vertices);
    if(out == NULL) {
        PRINT_ERROR(plGetError());
    }

    PRINT("\nVertices...\n");
    srand(num_vertices);

    for(unsigned int i = 0, cur_vert = 0; i < num_triangles; ++i, ++cur_vert) {
        PRINT("X: %d Y: %d Z: %d\n", vertices[i].x, vertices[i].y, vertices[i].z);

        plSetMeshVertexPosition3f(out, cur_vert, vertices[cur_vert].x, vertices[i].y, vertices[i].z);
        plSetMeshVertexColour(out, cur_vert, plCreateColour4b(255, 0, 0, 255));
        cur_vert++;
        plSetMeshVertexPosition3f(out, cur_vert, vertices[cur_vert].x, vertices[i].y, vertices[i].z);
        plSetMeshVertexColour(out, cur_vert, plCreateColour4b(0, 255, 0, 255));
        cur_vert++;
        plSetMeshVertexPosition3f(out, cur_vert, vertices[cur_vert].x, vertices[i].y, vertices[i].z);
        plSetMeshVertexColour(out, cur_vert, plCreateColour4b(0, 0, 255, 255));

    }
#endif

    PRINT("\nFaces...\n");
    for(unsigned int i = 0; i < num_faces; ++i) {
        PRINT("%d - 0: %d 1: %d 2: %d 3: %d 4: %d (num indices: %d)\n", i,
              faces[i].indices[0], faces[i].indices[1], faces[i].indices[2], faces[i].indices[3], faces[i].indices[5],
              faces[i].num_indices
        );
    }

    return out;
}

// loads a model in and then frees it
void load_mdl_temp(const char *path) {
    PLMesh *mesh = load_mdl(path);
    if(mesh != NULL) {
        plDeleteMesh(mesh);
    }
}

void write_smd(PLMesh *mesh, const char *name) {
    char out_path[10 + MAX_MODEL_NAME] = { '\0' };
    snprintf(out_path, sizeof(out_path), "./Models/%s.smd", name);
    FILE *out = fopen(out_path, "w");
    if(out == NULL) {
        PRINT_ERROR("Failed to open %s for writing!\n", out_path);
    }
    fprintf(out, "version 1\n\n");
    fprintf(out, "nodes\n");
    fprintf(out, "0 \"root\" -1");
    fprintf(out, "end\n\n");
    fclose(out);
}

/////////////////////////////////////////////////////////////////////////

typedef struct ANI {

} ANI;

/////////////////////////////////////////////////////////////////////////

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

#if defined(DEBUG_VERSIONS)
int compare_integers(const void *a, const void *b) {
    return (*(int*)a - *(int*)b);
}
#endif

int main(int argc, char **argv) {
    plInitialize(argc, argv, PL_SUBSYSTEM_WINDOW | PL_SUBSYSTEM_GRAPHICS);

    PRINT("\n " TITLE " : Version %d.%d (" __DATE__ ")\n", VERSION_MAJOR, VERSION_MINOR     );
    PRINT(" Developed by...\n"                                                              );
    PRINT("   Mark \"hogsy\" Sowden (http://talonbrave.info/)\n"                            );
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

    // Initialize DevIL...

    ilInit();
    iluInit();
    ilutRenderer(ILUT_OPENGL);

    ilEnable(IL_CONV_PAL);

    // And now for ours...

    plInitialize(argc, argv, PL_SUBSYSTEM_GRAPHICS);

    plSetDefaultGraphicsState();
    plSetClearColour(plCreateColour4b(0, 0, 128, 255));

    plSetupConsole(1);
    plShowConsole(true);
    plSetConsoleColour(1, plCreateColour4b(128, 0, 0, 128));

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

#if 1

    PLStaticModel *model = plLoadStaticModel("./Models/medkit.mdl");
    if(model == NULL) {
        PRINT_ERROR("Failed to load model!\n");
    }

#else
#if 0
#if defined(DEBUG_VERSIONS)
    memset(&versions, 0, sizeof(versions));
#endif

    plScanDirectory("./Models/", "mdl", load_mdl_temp, false);

    return EXIT_SUCCESS;

#if defined(DEBUG_VERSIONS)
    PRINT("\nFound the following \"version\" numbers...\n");
    qsort(versions, (size_t)version_position, sizeof(int), compare_integers);
    for(unsigned int i = 0; i < version_position; ++i) {
        PRINT("%d ", versions[i]);
    }
#endif
#else
    PLMesh *cur_model = load_mdl("./Models/medkit.mdl");
    if(cur_model == NULL) {
        PRINT_ERROR("Failed to load model!\n");
    }
#endif
#endif

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
        static double oldmpos[2] = {0, 0};
        static PLVector3D angles = { 0, 0 };
        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (state == GLFW_PRESS) {
            double nxpos = xpos - oldmpos[0];
            double nypos = ypos - oldmpos[1];
            angles.x += (nxpos / 100.f);
            angles.y += (nypos / 100.f);
        } else {
            oldmpos[0] = xpos;
            oldmpos[1] = ypos;
        }

        // Zoom in and out thing...
        static double oldrmpos[2] = {0, 0};
        state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        if (state == GLFW_PRESS) {
            double nypos = ypos - oldrmpos[1];
            main_camera->position.z += (nypos / 20.f);
        } else {
            oldrmpos[0] = xpos;
            oldrmpos[1] = ypos;
        }
        // input handlers end...

        plSetupCamera(main_camera);

#if 1
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

#if 1
                plDrawStaticModel(model);
#else
                plDrawMesh(cur_model);
#endif

                glShadeModel(GL_SMOOTH);
                glDisable(GL_LIGHTING);
                break;
            }

            case VIEW_MODE_WEIGHTS:
            case VIEW_MODE_WIREFRAME: {
#if 1
                plDrawStaticModel(model);
#else
                plDrawMesh(cur_model);
#endif
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
#endif

        //plDrawConsole();

        glfwSwapBuffers(window);
    }

#if 1
    plDeleteStaticModel(model);
#else
    plDeleteMesh(cur_model);
#endif
    plDeleteCamera(main_camera);

    plShutdown();

    glfwTerminate();

    return 0;
}