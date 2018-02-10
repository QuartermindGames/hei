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
#if defined(PL_SUPPORT_OPENGL)
#include <PL/platform_console.h>

#include "graphics_private.h"

#include <GL/glew.h>

#include <PL/platform_mesh.h>
#include <PL/platform_graphics.h>

#define DEBUG_GL

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

void ClearBoundTextures(void) {
    for(unsigned int i = 0; i < gfx_state.hw_maxtextureunits; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

void ClearBoundBuffers(void) {
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

void GLGetMaxTextureUnits(unsigned int *num_units) {
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint *) num_units);
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
    static bool is_blending = false;
    if(a == PL_BLEND_NONE && b == PL_BLEND_NONE && is_blending) {
        glDisable(GL_BLEND);
        is_blending = false;
    } else if(!is_blending) {
        glEnable(GL_BLEND);
        is_blending = true;
    }

    glBlendFunc(TranslateBlendFunc(a), TranslateBlendFunc(b));
}

void GLSetCullMode(PLCullMode mode) {
    if(mode == gfx_state.current_cullmode) {
        return;
    }

    if(mode == PL_CULL_NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        switch (mode) {
            default:
            case PL_CULL_NEGATIVE: {
                glFrontFace(GL_CW);
            } break;

            case PL_CULL_POSTIVE: {
                glFrontFace(GL_CCW);
            } break;
        }
    }

    gfx_state.current_cullmode = mode;
}

/////////////////////////////////////////////////////////////
// Texture

unsigned int TranslateImageFormat(PLImageFormat format) {
    switch(format) {
        case PL_IMAGEFORMAT_RGBA8:      return GL_RGBA8;
        case PL_IMAGEFORMAT_RGB4:       return GL_RGB4;
        case PL_IMAGEFORMAT_RGBA4:      return GL_RGBA4;
        case PL_IMAGEFORMAT_RGB5:       return GL_RGB5;
        case PL_IMAGEFORMAT_RGB5A1:     return GL_RGB5_A1;

        case PL_IMAGEFORMAT_RGB_DXT1:   return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case PL_IMAGEFORMAT_RGBA_DXT1:  return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case PL_IMAGEFORMAT_RGB_FXT1:   return GL_COMPRESSED_RGB_FXT1_3DFX;

        default: return 0;
    }
}

unsigned int TranslateImageColourFormat(PLColourFormat format) {
    switch(format) {
        default:
        case PL_COLOURFORMAT_RGBA:  return GL_RGBA;
        case PL_COLOURFORMAT_RGB:   return GL_RGB;
    }
}

void GLCreateTexture(PLTexture *texture) {
    glGenTextures(1, &texture->internal.id);
}

void GLBindTexture(const PLTexture *texture) {
    unsigned int id = 0;
    if(texture != NULL) {
        id = texture->internal.id;
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, id);
}

void GLUploadTexture(PLTexture *texture, const PLImage *upload) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    unsigned int min, mag;
    switch(texture->filter) {
        case PL_TEXTURE_FILTER_LINEAR: {    // linear
            min = mag = GL_LINEAR;
        } break;

        default:
        case PL_TEXTURE_FILTER_NEAREST: {   // nearest
            min = mag = GL_NEAREST;
        } break;

        case PL_TEXTURE_FILTER_MIPMAP_LINEAR: {
            min = GL_LINEAR_MIPMAP_LINEAR;
            mag = GL_LINEAR;
        } break;

        case PL_TEXTURE_FILTER_MIPMAP_LINEAR_NEAREST: {
            min = GL_LINEAR_MIPMAP_NEAREST;
            mag = GL_NEAREST;
        } break;

        case PL_TEXTURE_FILTER_MIPMAP_NEAREST: {
            min = GL_NEAREST_MIPMAP_NEAREST;
            mag = GL_NEAREST;
        } break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min);

    unsigned int levels = upload->levels;
    if(levels == 0) {
        levels = 1;
    }

    for(unsigned int i = 0; i < levels; ++i) {
        if(plIsCompressedImageFormat(upload->format)) {
            glCompressedTexImage2D(
                    GL_TEXTURE_2D,
                    i,
                    TranslateImageFormat(upload->format),
                    texture->w / (unsigned int)pow(2, i),
                    texture->h / (unsigned int)pow(2, i),
                    0,
                    upload->size,
                    upload->data[0]
            );
        } else {
            glTexImage2D(
                    GL_TEXTURE_2D,
                    i,
                    TranslateImageFormat(upload->format),
                    texture->w / (unsigned int)pow(2, i),
                    texture->h / (unsigned int)pow(2, i),
                    0,
                    TranslateImageColourFormat(upload->colour_format),
                    GL_UNSIGNED_BYTE,
                    upload->data[0]
            );
        }
    }

    if(levels == 1 && !(texture->flags & PL_TEXTURE_FLAG_NOMIPS)) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

/////////////////////////////////////////////////////////////
// Mesh

typedef struct MeshTranslatePrimitive {
    PLMeshPrimitive mode;
    unsigned int target;
    const char *name;
} MeshTranslatePrimitive;

MeshTranslatePrimitive primitives[] = {
    {PL_MESH_LINES, GL_LINES, "LINES"},
    {PL_MESH_POINTS, GL_POINTS, "POINTS"},
    {PL_MESH_TRIANGLES, GL_TRIANGLES, "TRIANGLES"},
    {PL_MESH_TRIANGLE_FAN, GL_TRIANGLE_FAN, "TRIANGLE_FAN"},
    {PL_MESH_TRIANGLE_FAN_LINE, GL_LINES, "TRIANGLE_FAN_LINE"},
    {PL_MESH_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP"},
    {PL_MESH_QUADS, GL_TRIANGLES, "QUADS"}   // todo, translate
};

unsigned int TranslatePrimitiveMode(PLMeshPrimitive mode) {
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
    BUFFER_VERTEX_DATA,
    BUFFER_TRIANGLE_DATA,
};

void GLCreateMeshPOST(PLMesh *mesh) {
    if(mesh->mode == PL_DRAW_IMMEDIATE) {
        return;
    }

    glGenBuffers(1, &mesh->internal.buffers[BUFFER_VERTEX_DATA]);
}

void GLUploadMesh(PLMesh *mesh) {
    if(mesh->mode == PL_DRAW_IMMEDIATE) {
        return;
    }

    unsigned int mode = TranslateDrawMode(mesh->mode);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->internal.buffers[BUFFER_VERTEX_DATA]);
    GLsizeiptr size = sizeof(PLVertex) * mesh->num_verts;
    glBufferData(GL_ARRAY_BUFFER, size, mesh->vertices, mode);

    glVertexPointer(3, GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].position);
    glNormalPointer(GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].normal);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PLVertex), &mesh->vertices[0].colour);
    glTexCoordPointer(2, GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].st[0]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLDeleteMesh(PLMesh *mesh) {
    if(mesh->mode == PL_DRAW_IMMEDIATE) {
        return;
    }

    glDeleteBuffers(1, &mesh->internal.buffers[BUFFER_VERTEX_DATA]);
}

void GLDrawMesh(PLMesh *mesh) {
    BindTexture(mesh->texture);

    if(mesh->mode == PL_DRAW_IMMEDIATE) {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].position);
        glNormalPointer(GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].normal);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PLVertex), &mesh->vertices[0].colour);
        glTexCoordPointer(2, GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].st[0]);
    } else {
        if(mesh->internal.buffers[BUFFER_VERTEX_DATA] == 0) {
            GfxLog("invalid buffer provided, skipping draw!\n");
            return;
        }

        glBindBuffer(GL_ARRAY_BUFFER, mesh->internal.buffers[BUFFER_VERTEX_DATA]);

        glVertexAttribLPointer(0, 3, GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].position);
        glVertexAttribLPointer(0, 3, GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].normal);
        glVertexAttribLPointer(0, 4, GL_UNSIGNED_BYTE, sizeof(PLVertex), &mesh->vertices[0].colour);
        glVertexAttribLPointer(0, 2, GL_FLOAT, sizeof(PLVertex), &mesh->vertices[0].st[0]);
    }

    GLuint mode = TranslatePrimitiveMode(mesh->primitive);
    if(mesh->num_indices > 0) {
        glDrawElements(mode, mesh->num_indices, GL_UNSIGNED_SHORT, mesh->indices);
    } else {
        glDrawArrays(mode, 0, mesh->num_verts);
    }

    if(mesh->mode == PL_DRAW_IMMEDIATE) {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

/////////////////////////////////////////////////////////////
// Camera

enum {
    VIEWPORT_FRAMEBUFFER,
    VIEWPORT_RENDERBUFFER_DEPTH,
    VIEWPORT_RENDERBUFFER_COLOUR,
};

void GLCreateCamera(PLCamera *camera) {
    plAssert(camera);
}

void GLDeleteCamera(PLCamera *camera) {
    plAssert(camera);
}

void GLSetupCamera(PLCamera *camera) {
    plAssert(camera);

    if(GLVersion(3, 0)) {
        if (UseBufferScaling(camera)) {
            if (camera->viewport.old_r_h != camera->viewport.r_h &&
                camera->viewport.old_r_w != camera->viewport.r_w) {
                glDeleteFramebuffers(1, &camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
                glDeleteRenderbuffers(1, &camera->viewport.buffers[VIEWPORT_RENDERBUFFER_DEPTH]);
                glDeleteRenderbuffers(1, &camera->viewport.buffers[VIEWPORT_RENDERBUFFER_COLOUR]);

                free(camera->viewport.v_buffer);
                camera->viewport.v_buffer = (uint8_t *) malloc(camera->viewport.r_w * camera->viewport.r_h * 4);
                if(camera->viewport.v_buffer == NULL) {
                    ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));
                }

                glGenFramebuffers(1, &camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
                glGenRenderbuffers(1, &camera->viewport.buffers[VIEWPORT_RENDERBUFFER_COLOUR]);
                glGenRenderbuffers(1, &camera->viewport.buffers[VIEWPORT_RENDERBUFFER_DEPTH]);

                // Colour
                glBindRenderbuffer(GL_RENDERBUFFER, camera->viewport.buffers[VIEWPORT_RENDERBUFFER_COLOUR]);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB4, camera->viewport.r_w, camera->viewport.r_h);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
                glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                          camera->viewport.buffers[VIEWPORT_RENDERBUFFER_COLOUR]);

                // Depth
                glBindRenderbuffer(GL_RENDERBUFFER, camera->viewport.buffers[VIEWPORT_RENDERBUFFER_DEPTH]);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, camera->viewport.r_w, camera->viewport.r_h);
                glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                          camera->viewport.buffers[VIEWPORT_RENDERBUFFER_DEPTH]);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    GfxLog("invalid framebuffer status on frame!\n");
                }

                ClearBoundBuffers();

                camera->viewport.old_r_w = camera->viewport.r_w;
                camera->viewport.old_r_h = camera->viewport.r_h;
            }

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
        }
    }

    glViewport(camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h);
    glScissor(camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h);

#if 1 // todo, modernize start
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    switch(camera->mode) {
        case PL_CAMERA_MODE_PERSPECTIVE: {
            double h = tan(camera->fov / 360 * PL_PI) * 0.1;
            double w = h * camera->viewport.w / camera->viewport.h;
            glFrustum(-w, w, -h, h, 0.1, 100000);

            glRotatef(camera->angles.y, 1, 0, 0);
            glRotatef(camera->angles.x, 0, 1, 0);
            glRotatef(camera->angles.z, 0, 0, 1);
            glTranslatef(camera->position.x, camera->position.y, camera->position.z);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            break;
        }

        case PL_CAMERA_MODE_ORTHOGRAPHIC: {
            glOrtho(0, camera->viewport.w, camera->viewport.h, 0, 0, 1000);
            break;
        }

        case PL_CAMERA_MODE_ISOMETRIC: {
            glOrtho(-camera->fov, camera->fov, -camera->fov, 5, -5, 40);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glRotatef(35.264f, 1, 0, 0);
            glRotatef(camera->angles.x, 0, 1, 0);

            glTranslatef(camera->position.x, camera->position.y, camera->position.z);
            break;
        }

        default: break;
    }
#endif // modernize end

    // keep the gfx_state up-to-date on the situation
    gfx_state.current_viewport = camera->viewport;
}

void GLDrawPerspectivePOST(PLCamera *camera) {
    plAssert(camera);

    if(GLVersion(3, 0)) {
        if (UseBufferScaling(camera)) {
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glReadPixels(
                    0, 0,
                    camera->viewport.r_w,
                    camera->viewport.r_h,
                    GL_BGRA,
                    GL_UNSIGNED_BYTE,
                    &camera->viewport.v_buffer[0]
            );
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            glScissor(0, 0, camera->viewport.w, camera->viewport.h);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, camera->viewport.buffers[VIEWPORT_FRAMEBUFFER]);
            glBlitFramebuffer(
                    0, 0,
                    camera->viewport.r_w, camera->viewport.r_h,
                    0, 0,
                    camera->viewport.w, camera->viewport.h,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST
            );

            ClearBoundBuffers();
        }
    }
}

/////////////////////////////////////////////////////////////
// Shader

#define SHADER_INVALID_TYPE ((uint32_t)0 - 1)

GLenum TranslateShaderStageType(PLShaderStageType type) {
    switch(type) {
        case PL_SHADER_TYPE_VERTEX:     return GL_VERTEX_SHADER;
        case PL_SHADER_TYPE_COMPUTE:    return GL_COMPUTE_SHADER;
        case PL_SHADER_TYPE_FRAGMENT:   return GL_FRAGMENT_SHADER;
        case PL_SHADER_TYPE_GEOMETRY:   return GL_GEOMETRY_SHADER;
        default:                        return SHADER_INVALID_TYPE;
    }
}

const char *GetGLShaderStageDescriptor(GLenum type) {
    switch(type) {
        case GL_VERTEX_SHADER:      return "GL_VERTEX_SHADER";
        case GL_COMPUTE_SHADER:     return "GL_COMPUTE_SHADER";
        case GL_FRAGMENT_SHADER:    return "GL_FRAGMENT_SHADER";
        case GL_GEOMETRY_SHADER:    return "GL_GEOMETRY_SHADER";
        default:                    return "unknown";
    }
}

GLenum TranslateShaderUniformType(PLShaderUniformType type) {
    switch(type) {
        case PL_UNIFORM_BOOL:   return GL_BOOL;
        case PL_UNIFORM_DOUBLE: return GL_DOUBLE;
        case PL_UNIFORM_FLOAT:  return GL_FLOAT;
        case PL_UNIFORM_INT:    return GL_INT;
        case PL_UNIFORM_UINT:   return GL_UNSIGNED_INT;

        case PL_UNIFORM_SAMPLER1D:          return GL_SAMPLER_1D;
        case PL_UNIFORM_SAMPLER1DSHADOW:    return GL_SAMPLER_1D_SHADOW;
        case PL_UNIFORM_SAMPLER2D:          return GL_SAMPLER_2D;
        case PL_UNIFORM_SAMPLER2DSHADOW:    return GL_SAMPLER_2D_SHADOW;
        case PL_UNIFORM_SAMPLER3D:          return GL_SAMPLER_3D;
        case PL_UNIFORM_SAMPLERCUBE:        return GL_SAMPLER_CUBE;

        case PL_UNIFORM_VEC2:   return GL_FLOAT_VEC2;
        case PL_UNIFORM_VEC3:   return GL_FLOAT_VEC3;
        case PL_UNIFORM_VEC4:   return GL_FLOAT_VEC4;

        case PL_UNIFORM_MAT3:   return GL_FLOAT_MAT3;

        default:    return SHADER_INVALID_TYPE;
    }
}

unsigned int TranslateGLShaderUniformType(GLenum type) {
    switch(type) {
        case GL_BOOL:           return PL_UNIFORM_BOOL;
        case GL_DOUBLE:         return PL_UNIFORM_DOUBLE;
        case GL_FLOAT:          return PL_UNIFORM_FLOAT;
        case GL_INT:            return PL_UNIFORM_INT;
        case GL_UNSIGNED_INT:   return PL_UNIFORM_UINT;

        case GL_SAMPLER_1D:         return PL_UNIFORM_SAMPLER1D;
        case GL_SAMPLER_1D_SHADOW:  return PL_UNIFORM_SAMPLER1DSHADOW;
        case GL_SAMPLER_2D:         return PL_UNIFORM_SAMPLER2D;
        case GL_SAMPLER_2D_SHADOW:  return PL_UNIFORM_SAMPLER2DSHADOW;
        case GL_SAMPLER_3D:         return PL_UNIFORM_SAMPLER3D;
        case GL_SAMPLER_CUBE:       return PL_UNIFORM_SAMPLERCUBE;

        case GL_FLOAT_VEC2:   return PL_UNIFORM_VEC2;
        case GL_FLOAT_VEC3:   return PL_UNIFORM_VEC3;
        case GL_FLOAT_VEC4:   return PL_UNIFORM_VEC4;

        case GL_FLOAT_MAT3:   return PL_UNIFORM_MAT3;

        default:    return SHADER_INVALID_TYPE;
    }
}

void GLCreateShaderProgram(PLShaderProgram *program) {

}

void GLDeleteShaderProgram(PLShaderProgram *program) {
    if(program->internal.id == (unsigned int)(-1)) {
        return;
    }

    glDeleteProgram(program->internal.id);
}

void GLCreateShaderStage(PLShaderStage *stage) {
    if(!GLVersion(2,0)) {
        GfxLog("HW shaders unsupported on platform, relying on SW fallback\n");
        return;
    }

    GLenum type = TranslateShaderStageType(stage->type);
    if(type == SHADER_INVALID_TYPE) {
        ReportError(PL_RESULT_INVALID_SHADER_TYPE, "%u", type);
        return;
    }

    if(type == GL_GEOMETRY_SHADER && !GLVersion(3,0)) {
        ReportError(PL_RESULT_UNSUPPORTED_SHADER_TYPE, "%s", GetGLShaderStageDescriptor(type));
        return;
    }

    if(type == GL_COMPUTE_SHADER && !GLVersion(4,3)) {
        ReportError(PL_RESULT_UNSUPPORTED_SHADER_TYPE, "%s", GetGLShaderStageDescriptor(type));
        return;
    }

    stage->internal.id = glCreateShader(type);
    if(stage->internal.id == 0) {
        ReportError(PL_RESULT_INVALID_SHADER_TYPE, "%u", type);
        return;
    }
}

void GLSetShaderProgram(PLShaderProgram *program) {
    unsigned int id = 0;
    if(program != NULL) {
        id = program->internal.id;
    }

    glUseProgram(id);
}

/////////////////////////////////////////////////////////////

char gl_extensions[4096][4096] = { { '\0' } };

#if defined(DEBUG_GL)
void MessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        void *param) {
    UNUSED_ARGS(source, id, length, param)

    const char *s_severity;
    switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH: {
            s_severity = "HIGH";
        } break;

        case GL_DEBUG_SEVERITY_MEDIUM: {
            s_severity = "MEDIUM";
        } break;

        case GL_DEBUG_SEVERITY_LOW: {
            s_severity = "LOW";
        } break;

        default: {
            s_severity = "NOTIFICATION";
        } break;
    }

    const char *s_type;
    switch(type) {
        case GL_DEBUG_TYPE_ERROR: {
            s_type = "ERROR";
        } break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
            s_type = "DEPRECATED";
        } break;

        case GL_DEBUG_TYPE_MARKER: {
            s_type = "MARKER";
        } break;

        case GL_DEBUG_TYPE_PERFORMANCE: {
            s_type = "PERFORMANCE";
        } break;

        case GL_DEBUG_TYPE_PORTABILITY: {
            s_type = "PORTABILITY";
        } break;

        default: {
            s_type = "OTHER";
        } break;
    }

    if(message != NULL && message[0] != '\0') {
        GfxLog("(%s) %s - %s\n", s_type, s_severity, message);
    }
}
#endif

void InitOpenGL(void) {
    GLenum err = glewInit();
    if(err != GLEW_OK) {
        ReportError(PL_RESULT_GRAPHICSINIT, "failed to initialize glew, %s", glewGetErrorString(err));
        return;
    }

    // setup the gfx layer
    gfx_layer.HWSupportsShaders         = GLHWSupportsShaders;
    gfx_layer.HWSupportsMultitexture    = GLHWSupportsMultitexture;
    gfx_layer.GetMaxTextureUnits        = GLGetMaxTextureUnits;

    gfx_layer.SetBlendMode              = GLSetBlendMode;
    gfx_layer.SetCullMode               = GLSetCullMode;

    gfx_layer.SetClearColour            = GLSetClearColour;
    gfx_layer.ClearBuffers              = GLClearBuffers;

    gfx_layer.CreateTexture             = GLCreateTexture;
    gfx_layer.BindTexture               = GLBindTexture;
    gfx_layer.UploadTexture             = GLUploadTexture;

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
    gfx_layer.SetShaderProgram          = GLSetShaderProgram;

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

#if defined(DEBUG_GL)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageCallback((GLDEBUGPROC) MessageCallback, NULL);
#endif

    glPointSize(5.f);
    glLineWidth(2.f);
}

void ShutdownOpenGL(void) {
#if defined(DEBUG_GL)
    glDisable(GL_DEBUG_OUTPUT);
    glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
}

#endif