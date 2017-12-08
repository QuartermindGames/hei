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
#include "graphics_private.h"

#ifdef __APPLE__
#   if 0
#       include <OpenGL/gl3.h>
#       include <OpenGL/gl3ext.h>

#   else
#       include <OpenGL/gl.h>
#       include <OpenGL/glext.h>
#       include <OpenGL/glu.h>

#       include <OpenGL/gl3.h>
#       include <OpenGL/gl3ext.h>
#   endif
#else
#   define GL_GLEXT_PROTOTYPES
//#   include <GL/gl.h>
#   include <GL/glcorearb.h>

#	ifdef _WIN32
#		include <GL/wglew.h>
#	endif
#endif

typedef struct GLCapabilities {
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
} GLCapabilities;

GLCapabilities gl_capabilities;

int gl_version_major = 0;
int gl_version_minor = 0;

#define GLVersion(maj, min) (((maj) == gl_version_major && (min) <= gl_version_minor) || (maj) < gl_version_major)

int gl_num_extensions = 0;

/////////////////////////////////////////////////////////////

bool GLHWSupportsShaders(void) {
    return (GLVersion(2, 1) || (gl_capabilities.fragment_program && gl_capabilities.vertex_program));
}

bool GLHWSupportsMultitexture(void) {
    return gl_capabilities.multitexture;
}

/////////////////////////////////////////////////////////////

unsigned int TranslateBlendFunc(PLBlend blend) {
    switch(blend) {
        case PL_BLEND_ONE: return GL_ONE;
        case PL_BLEND_ZERO: return GL_ZERO;
        case VL_BLEND_SRC_COLOR: return GL_SRC_COLOR;
        case VL_BLEND_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
        case VL_BLEND_SRC_ALPHA: return GL_SRC_ALPHA;
        case VL_BLEND_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
        case VL_BLEND_DST_ALPHA: return GL_DST_ALPHA;
        case VL_BLEND_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
        case VL_BLEND_DST_COLOR: return GL_DST_COLOR;
        case VL_BLEND_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
        case VL_BLEND_SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
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
                plGraphicsLog("Invalid framebuffer status on frame!\n");
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
    if(program->id == -1) {
        return;
    }

    glDeleteProgram(program->id);
}

/////////////////////////////////////////////////////////////

char gl_extensions[4096][4096] = { { '\0' } };

void InitOpenGL(void) {
    // setup the gfx layer
    gfx_layer.HWSupportsShaders         = GLHWSupportsShaders;
    gfx_layer.HWSupportsMultitexture    = GLHWSupportsMultitexture;
    gfx_layer.SetBlendMode              = GLSetBlendMode;
    gfx_layer.SetCullMode               = GLSetCullMode;

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

    memset(&gl_capabilities, 0, sizeof(GLCapabilities));

    glGetIntegerv(GL_NUM_EXTENSIONS, &gl_num_extensions);

    glGetIntegerv(GL_MINOR_VERSION, &gl_version_minor);
    glGetIntegerv(GL_MAJOR_VERSION, &gl_version_major);
    unsigned int err = glGetError();
    if(err == GL_INVALID_ENUM) { // try again the old fashioned way...
        if(gfx_state.hw_version[0] != '\0') {
            gl_version_major = gfx_state.hw_version[0];
        }
        if(gfx_state.hw_version[2] != '\0') {
            gl_version_minor = gfx_state.hw_version[1];
        }
    }

    plGraphicsLog(" OpenGL %d.%d\n", gl_version_major, gl_version_minor);
    plGraphicsLog("  renderer:   %s\n", gfx_state.hw_renderer);
    plGraphicsLog("  vendor:     %s\n", gfx_state.hw_vendor);
    plGraphicsLog("  version:    %s\n", gfx_state.hw_version);
    plGraphicsLog("  extensions:\n");
    for(unsigned int i = 0; i < gl_num_extensions; ++i) {
        const uint8_t *extension = glGetStringi(GL_EXTENSIONS, i);
        sprintf(gl_extensions[i], "%s", extension);
        plGraphicsLog("    %s\n", extension);
    }
}

void ShutdownOpenGL() {

}