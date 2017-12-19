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
#if defined(PL_USE_GL)
#include <PL/platform_console.h>

#include "graphics_private.h"

#include <GL/glew.h>

struct {
    bool generate_mipmap;
    bool depth_texture;
    bool shadow;
    bool vertex_buffer_object;
    bool texture_compression;
    bool texture_compression_s3tc;
    bool multitexture;
    bool texture_env_combine;
    bool texture_env_add;
    bool vertex_program;
    bool fragment_program;
} gl_capabilities;

int gl_version_major = 0;
int gl_version_minor = 0;

#define GLVersion(maj, min) (((maj) == gl_version_major && (min) <= gl_version_minor) || (maj) < gl_version_major)

unsigned int gl_num_extensions = 0;

void GLClearBoundTextures(void) {
    for(unsigned int i = 0; i < gfx_state.hw_maxtextureunits; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

void GLClearBoundFramebuffers(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_RENDERBUFFER, 0);
}

/////////////////////////////////////////////////////////////

bool GLHWSupportsShaders(void) {
    return (GLVersion(2, 1) || (gl_capabilities.fragment_program && gl_capabilities.vertex_program));
}

bool GLHWSupportsMultitexture(void) {
    return gl_capabilities.multitexture;
}

/////////////////////////////////////////////////////////////

void GLSetClearColour(PLColour rgba) {
    glClearColor(
            plByteToFloat(rgba.r),
            plByteToFloat(rgba.g),
            plByteToFloat(rgba.b),
            plByteToFloat(rgba.a)
    );
}

void GLClearBuffers(unsigned int buffers) {
    // Rather ugly, but translate it over to GL.
    unsigned int glclear = 0;
    if(buffers & PL_BUFFER_COLOUR)  glclear |= GL_COLOR_BUFFER_BIT;
    if(buffers & PL_BUFFER_DEPTH)   glclear |= GL_DEPTH_BUFFER_BIT;
    if(buffers & PL_BUFFER_STENCIL) glclear |= GL_STENCIL_BUFFER_BIT;
    glClear(glclear);
}

/////////////////////////////////////////////////////////////

unsigned int TranslateBlendFunc(PLBlend blend) {
    switch(blend) {
        case PL_BLEND_ONE: return GL_ONE;
        case PL_BLEND_ZERO: return GL_ZERO;
        case PL_BLEND_SRC_COLOR: return GL_SRC_COLOR;
        case PL_BLEND_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
        case PL_BLEND_SRC_ALPHA: return GL_SRC_ALPHA;
        case PL_BLEND_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
        case PL_BLEND_DST_ALPHA: return GL_DST_ALPHA;
        case PL_BLEND_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
        case PL_BLEND_DST_COLOR: return GL_DST_COLOR;
        case PL_BLEND_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
        case PL_BLEND_SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
        default:return 0;
    }
}

void GLSetBlendMode(PLBlend a, PLBlend b) {
    glBlendFunc(TranslateBlendFunc(a), TranslateBlendFunc(b));
}

void GLSetCullMode(PLCullMode mode) {
    if(mode == gfx_state.current_cullmode) {
        return;
    }

    glCullFace(GL_BACK);
    switch(mode) {
        default:
        case PL_CULL_NEGATIVE: {
            glFrontFace(GL_CW);
        } break;

        case PL_CULL_POSTIVE: {
            glFrontFace(GL_CCW);
        } break;
    }
    gfx_state.current_cullmode = mode;
}

/////////////////////////////////////////////////////////////
// Mesh

typedef struct MeshTranslatePrimitive {
    PLMeshPrimitive mode;
    unsigned int target;
    const char *name;
} MeshTranslatePrimitive;

MeshTranslatePrimitive primitives[] = {
    {PLMESH_LINES, GL_LINES, "LINES"},
    {PLMESH_POINTS, GL_POINTS, "POINTS"},
    {PLMESH_TRIANGLES, GL_TRIANGLES, "TRIANGLES"},
    {PLMESH_TRIANGLE_FAN, GL_TRIANGLE_FAN, "TRIANGLE_FAN"},
    {PLMESH_TRIANGLE_FAN_LINE, GL_LINES, "TRIANGLE_FAN_LINE"},
    {PLMESH_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP"},
    {PLMESH_QUADS, GL_TRIANGLES, "QUADS"}   // todo, translate
};

unsigned int TranslatePrimitiveMode(PLMeshPrimitive mode) {
    if(mode == PL_PRIMITIVE_IGNORE) {
        return 0;
    }

    for (unsigned int i = 0; i < plArrayElements(primitives); i++) {
        if (mode == primitives[i].mode)
            return primitives[i].target;
    }

    // Hacky, but just return initial otherwise.
    return primitives[0].target;
}

unsigned int TranslateDrawMode(PLMeshDrawMode mode) {
    switch(mode) {
        case PL_DRAW_DYNAMIC:   return GL_DYNAMIC_DRAW;
        case PL_DRAW_STATIC:    return GL_STATIC_DRAW;
        default:                return 0;
    }
}

enum {
    MESH_VBO
};

void GLCreateMeshPOST(PLMesh *mesh) {
    glGenBuffers(1, &mesh->_buffers[MESH_VBO]);
}

void GLUploadMesh(PLMesh *mesh) {
    if(mesh->mode == PL_DRAW_IMMEDIATE) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->_buffers[MESH_VBO]);
    //glBufferSubData(GL_ARRAY_BUFFER, )
}

void GLDeleteMesh(PLMesh *mesh) {
    glDeleteBuffers(1, &mesh->_buffers[MESH_VBO]);
}

void GLDrawMesh(PLMesh *mesh) {
    if(mesh->mode == PL_DRAW_IMMEDIATE) {
        glBegin(TranslatePrimitiveMode(mesh->primitive));
        for(unsigned int i = 0; i < mesh->num_verts; ++i) {
            glVertex3f(mesh->vertices[i].position.x, mesh->vertices[i].position.y, mesh->vertices[i].position.z);
            glTexCoord2f(mesh->vertices[i].st[0].x, mesh->vertices[i].st[0].y);
            glColor3ub(mesh->vertices[i].colour.r, mesh->vertices[i].colour.g, mesh->vertices[i].colour.b);
            glNormal3f(mesh->vertices[i].normal.x, mesh->vertices[i].normal.y, mesh->vertices[i].normal.z);
        }
        glEnd();
        return;
    }

    if(mesh->mode == PL_DRAW_STATIC) {

    } else {

    }

    GLuint mode = TranslatePrimitiveMode(mesh->primitive);
    if(mode == GL_TRIANGLES) {
        glDrawElements(
                mode,
                mesh->num_triangles * 3,
                GL_UNSIGNED_BYTE,
                mesh->indices
        );
    } else {
        glDrawArrays(mode, 0, mesh->num_verts);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/////////////////////////////////////////////////////////////
// Camera

#define VIEWPORT_FRAMEBUFFER            0
#define VIEWPORT_RENDERBUFFER_DEPTH     1
#define VIEWPORT_RENDERBUFFER_COLOUR    2

void GLCreateCamera(PLCamera *camera) {
    plAssert(camera);

    if(GLVersion(3, 0)) {
        glGenFramebuffers(1, &camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
        glGenRenderbuffers(1, &camera->viewport.buffers[VIEWPORT_RENDERBUFFER_DEPTH]);
        glGenRenderbuffers(1, &camera->viewport.buffers[VIEWPORT_RENDERBUFFER_COLOUR]);
    }
}

void GLDeleteCamera(PLCamera *camera) {
    plAssert(camera);

    if(GLVersion(3, 0)) {
        glDeleteFramebuffers(1, &camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
        glDeleteRenderbuffers(1, &camera->viewport.buffers[VIEWPORT_RENDERBUFFER_DEPTH]);
        glDeleteRenderbuffers(1, &camera->viewport.buffers[VIEWPORT_RENDERBUFFER_COLOUR]);
    }
}

void GLSetupCamera(PLCamera *camera) {
    plAssert(camera);

    if(GLVersion(3, 0)) {
        if (UseBufferScaling(camera)) {
            if (camera->viewport.old_r_height != camera->viewport.r_height &&
                camera->viewport.old_r_width != camera->viewport.r_width) {
                GLDeleteCamera(camera);

                if (camera->viewport.v_buffer) {
                    free(camera->viewport.v_buffer);
                }

                camera->viewport.v_buffer = (uint8_t *) malloc(
                        camera->viewport.r_width * camera->viewport.r_height * 4);
                camera->viewport.old_r_width = camera->viewport.r_width;
                camera->viewport.old_r_height = camera->viewport.r_height;
            }

            // Colour
            glBindRenderbuffer(GL_RENDERBUFFER, camera->viewport.buffers[VIEWPORT_RENDERBUFFER_COLOUR]);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB4, camera->viewport.r_width, camera->viewport.r_height);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      camera->viewport.buffers[VIEWPORT_RENDERBUFFER_COLOUR]);

            // Depth
            glBindRenderbuffer(GL_RENDERBUFFER, camera->viewport.buffers[VIEWPORT_RENDERBUFFER_DEPTH]);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, camera->viewport.r_width,
                                  camera->viewport.r_height);
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                      camera->viewport.buffers[VIEWPORT_RENDERBUFFER_DEPTH]);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                GfxLog("Invalid framebuffer status on frame!\n");
            }

            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    }

    glViewport(camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h);
    glScissor(camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h);

    // keep the gfx_state up-to-date on the situation
    gfx_state.current_viewport = camera->viewport;
}

void GLDrawPerspectivePOST(PLCamera *camera) {
    plAssert(camera);

    if(GLVersion(3, 0)) {
        if (UseBufferScaling(camera)) {
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glReadPixels(0, 0, camera->viewport.r_width, camera->viewport.r_height, GL_BGRA, GL_UNSIGNED_BYTE,
                         &camera->viewport.v_buffer[0]);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glScissor(0, 0, camera->viewport.w, camera->viewport.h);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
            glBlitFramebuffer(
                    0, 0,
                    camera->viewport.r_width, camera->viewport.r_height,
                    0, 0,
                    camera->viewport.w, camera->viewport.h,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST
            );
        }
    }
}

/////////////////////////////////////////////////////////////
// Shader

void GLCreateShaderProgram(PLShaderProgram *program) {}

void GLDeleteShaderProgram(PLShaderProgram *program) {
    if(program->id == (unsigned int)(-1)) {
        return;
    }

    glDeleteProgram(program->id);
}

/////////////////////////////////////////////////////////////

char gl_extensions[4096][4096] = { { '\0' } };

void InitOpenGL(void) {
    GLenum err = glewInit();
    if(err != GLEW_OK) {
        ReportError(PL_RESULT_GRAPHICSINIT, "failed to initialize glew, %s", glewGetErrorString(err));
        return;
    }

    // setup the gfx layer
    gfx_layer.HWSupportsShaders         = GLHWSupportsShaders;
    gfx_layer.HWSupportsMultitexture    = GLHWSupportsMultitexture;
    gfx_layer.SetBlendMode              = GLSetBlendMode;
    gfx_layer.SetCullMode               = GLSetCullMode;

    gfx_layer.SetClearColour            = GLSetClearColour;
    gfx_layer.ClearBuffers              = GLClearBuffers;

    gfx_layer.CreateMeshPOST            = GLCreateMeshPOST;
    gfx_layer.DeleteMesh                = GLDeleteMesh;
    gfx_layer.DrawMesh                  = GLDrawMesh;
    gfx_layer.UploadMesh                = GLUploadMesh;

    gfx_layer.CreateCamera              = GLCreateCamera;
    gfx_layer.DeleteCamera              = GLDeleteCamera;
    gfx_layer.SetupCamera               = GLSetupCamera;
    gfx_layer.DrawPerspectivePOST       = GLDrawPerspectivePOST;

    gfx_layer.CreateShaderProgram       = GLCreateShaderProgram;
    gfx_layer.DeleteShaderProgram       = GLDeleteShaderProgram;

    /////////////////////////////////////////////////////////////

    // Get any information that will be presented later.
    gfx_state.hw_extensions     = (const char *) glGetString(GL_EXTENSIONS);
    gfx_state.hw_renderer       = (const char *) glGetString(GL_RENDERER);
    gfx_state.hw_vendor         = (const char *) glGetString(GL_VENDOR);
    gfx_state.hw_version        = (const char *) glGetString(GL_VERSION);

    memset(&gl_capabilities, 0, sizeof(gl_capabilities));

    glGetIntegerv(GL_NUM_EXTENSIONS, (GLint*)(&gl_num_extensions));

    glGetIntegerv(GL_MINOR_VERSION, &gl_version_minor);
    glGetIntegerv(GL_MAJOR_VERSION, &gl_version_major);

    GfxLog(" OpenGL %d.%d\n", gl_version_major, gl_version_minor);
    GfxLog("  renderer:   %s\n", gfx_state.hw_renderer);
    GfxLog("  vendor:     %s\n", gfx_state.hw_vendor);
    GfxLog("  version:    %s\n", gfx_state.hw_version);
    GfxLog("  extensions:\n");
    for(unsigned int i = 0; i < gl_num_extensions; ++i) {
        const uint8_t *extension = glGetStringi(GL_EXTENSIONS, i);
        sprintf(gl_extensions[i], "%s", extension);
        GfxLog("    %s\n", extension);
    }
}

void ShutdownOpenGL() {

}

#endif