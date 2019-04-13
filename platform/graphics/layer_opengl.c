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
    bool direct_state_access;
} gl_capabilities;

static int gl_version_major = 0;
static int gl_version_minor = 0;

#define GLVersion(maj, min) (((maj) == gl_version_major && (min) <= gl_version_minor) || (maj) < gl_version_major)

unsigned int gl_num_extensions = 0;

static GLuint VAO[1];

///////////////////////////////////////////
// Debug

static void GLInsertDebugMarker(const char *msg) {
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, msg );
}

static void GLPushDebugGroupMarker(const char *msg) {
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, msg);
}

static void GLPopDebugGroupMarker() {
    glPopDebugGroup();
}


static void ClearBoundTextures(void) {
    for(unsigned int i = 0; i < gfx_state.hw_maxtextureunits; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

static void ClearBoundBuffers(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

/////////////////////////////////////////////////////////////

static bool GLHWSupportsShaders(void) {
    return (GLVersion(2, 1) || (gl_capabilities.fragment_program && gl_capabilities.vertex_program));
}

static bool GLHWSupportsMultitexture(void) {
    return gl_capabilities.multitexture;
}

static void GLGetMaxTextureUnits(unsigned int *num_units) {
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint *) num_units);
}

/////////////////////////////////////////////////////////////

static void GLSetClearColour(PLColour rgba) {
    glClearColor(
            plByteToFloat(rgba.r),
            plByteToFloat(rgba.g),
            plByteToFloat(rgba.b),
            plByteToFloat(rgba.a)
    );
}

static void GLClearBuffers(unsigned int buffers) {
    // Rather ugly, but translate it over to GL.
    unsigned int glclear = 0;
    if(buffers & PL_BUFFER_COLOUR)  glclear |= GL_COLOR_BUFFER_BIT;
    if(buffers & PL_BUFFER_DEPTH)   glclear |= GL_DEPTH_BUFFER_BIT;
    if(buffers & PL_BUFFER_STENCIL) glclear |= GL_STENCIL_BUFFER_BIT;
    glClear(glclear);
}

/////////////////////////////////////////////////////////////

static unsigned int TranslateBlendFunc(PLBlend blend) {
    switch(blend) {
        default:
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
    }
}

static void GLSetBlendMode(PLBlend a, PLBlend b) {
    if(a == PL_BLEND_NONE && b == PL_BLEND_NONE) {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
    }

    glBlendFunc(TranslateBlendFunc(a), TranslateBlendFunc(b));
}

static void GLSetCullMode(PLCullMode mode) {
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
// Framebuffer

static unsigned int TranslateFrameBufferBinding(PLFBOTarget targetBinding) {
    switch(targetBinding) {
        case PL_FRAMEBUFFER_DEFAULT:    return GL_FRAMEBUFFER;
        case PL_FRAMEBUFFER_DRAW:       return GL_DRAW_FRAMEBUFFER;
        case PL_FRAMEBUFFER_READ:       return GL_READ_FRAMEBUFFER;
        default:                        return 0;
    }
}

static void GLCreateFrameBuffer(PLFrameBuffer *buffer) {

    glGenFramebuffers(1, &buffer->fbo );
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer->fbo);
    GfxLog("Created framebuffer %dx%d", buffer->width, buffer->height);

    if(buffer->flags & PL_BUFFER_COLOUR){
            glGenRenderbuffers(1, &buffer->rbo_colour);
            glBindRenderbuffer(GL_RENDERBUFFER, buffer->rbo_colour);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, buffer->width, buffer->height);
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer->rbo_colour);
            GfxLog("Created colour renderbuffer %dx%d", buffer->width, buffer->height);
    }
    if(buffer->flags & PL_BUFFER_DEPTH){
            glGenRenderbuffers(1, &buffer->rbo_depth);
            glBindRenderbuffer(GL_RENDERBUFFER, buffer->rbo_depth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, buffer->width, buffer->height);
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer->rbo_depth);
            GfxLog("Created depth renderbuffer %dx%d", buffer->width, buffer->height);
    }
    if(buffer->flags & PL_BUFFER_STENCIL){
            GfxLog("Stencil renderbuffer not supported yet!");
    }

}

static void GLDeleteFrameBuffer(PLFrameBuffer *buffer) {
    if(buffer){
        glDeleteFramebuffers(1, &buffer->fbo);
        if(buffer->rbo_colour){
            glDeleteRenderbuffers(1, &buffer->rbo_colour);
        }
        if(buffer->rbo_depth){
            glDeleteRenderbuffers(1, &buffer->rbo_depth);
        }
    }
}

static void GLBindFrameBuffer(PLFrameBuffer *buffer, PLFBOTarget target_binding) {
    GLuint binding = TranslateFrameBufferBinding(target_binding);
    if(buffer){
        glBindFramebuffer(binding, buffer->fbo);
    }
    else{
        glBindFramebuffer(binding, 0); //Bind default backbuffer
    }
}

static void GLBlitFrameBuffers(PLFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, bool linear ){

    if(src_buffer){
        glBindFramebuffer(GL_READ_FRAMEBUFFER, src_buffer->fbo);
    }
    else{
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); //Bind default backbuffer
    }
    if(dst_buffer){
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_buffer->fbo);
    }
    else{
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); //Bind default backbuffer
    }

    glBlitFramebuffer(0, 0, src_w, src_h, 0, 0, dst_w, dst_h, GL_COLOR_BUFFER_BIT, linear ? GL_LINEAR : GL_NEAREST);
}


/////////////////////////////////////////////////////////////
// Texture

static unsigned int TranslateImageFormat(PLImageFormat format) {
    switch(format) {
        case PL_IMAGEFORMAT_RGB8:       return GL_RGB8;
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

static unsigned int TranslateStorageFormat(PLDataFormat format) {
    switch(format) {
        case PL_UNSIGNED_BYTE:              return GL_UNSIGNED_BYTE;
        case PL_UNSIGNED_INT_8_8_8_8_REV:   return GL_UNSIGNED_INT_8_8_8_8_REV;
        default:
            plAssert(0);
            return 0; /* todo */
    }
}

static unsigned int TranslateImageColourFormat(PLColourFormat format) {
    switch(format) {
        default:
        case PL_COLOURFORMAT_RGBA:  return GL_RGBA;
        case PL_COLOURFORMAT_RGB:   return GL_RGB;
    }
}

static void GLCreateTexture(PLTexture *texture) {
    glGenTextures(1, &texture->internal.id);
}

static void GLDeleteTexture(PLTexture *texture) {
    glDeleteTextures(1, &texture->internal.id);
}

static void GLBindTexture(const PLTexture *texture) {
    if(texture == NULL) {
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }
    glBindTexture(GL_TEXTURE_2D, texture->internal.id);
}

static void GLUploadTexture(PLTexture *texture, const PLImage *upload) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    unsigned int min, mag;
    switch(texture->filter) {
        case PL_TEXTURE_FILTER_LINEAR: {    // linear
            min = mag = GL_LINEAR;
            texture->flags |= PL_TEXTURE_FLAG_NOMIPS;
        } break;

        default:
        case PL_TEXTURE_FILTER_NEAREST: {   // nearest
            min = mag = GL_NEAREST;
            texture->flags |= PL_TEXTURE_FLAG_NOMIPS;
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

    unsigned int image_format = TranslateImageFormat(upload->format);
    unsigned int colour_format = TranslateImageColourFormat(upload->colour_format);
    unsigned int storage_format = TranslateStorageFormat(texture->storage);

    for(unsigned int i = 0; i < levels; ++i) {
        GLsizei w = texture->w / (unsigned int)pow(2, i);
        GLsizei h = texture->h / (unsigned int)pow(2, i);
        if(plIsCompressedImageFormat(upload->format)) {
            glCompressedTexImage2D(
                    GL_TEXTURE_2D,
                    i,
                    image_format,
                    w, h,
                    0,
                    (GLsizei) upload->size,
                    upload->data[0]
            );
        } else {
            glTexImage2D(
                    GL_TEXTURE_2D,
                    i,
                    image_format,
                    w, h,
                    0,
                    colour_format,
                    storage_format,
                    upload->data[0]
            );
        }
    }

    if(levels == 1 && !(texture->flags & PL_TEXTURE_FLAG_NOMIPS)) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
}

static void GLSetTextureAnisotropy(PLTexture *texture, uint32_t value) {
    plSetTexture(texture, gfx_state.current_textureunit);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (int) value);
}

static void GLActiveTexture(unsigned int target) {
    glActiveTexture(GL_TEXTURE0 + target);
}

/* Swizzle texture channels */

static int TranslateColourChannel(int channel) {
    switch(channel) {
        case PL_RED:    return GL_RED;
        case PL_GREEN:  return GL_GREEN;
        case PL_BLUE:   return GL_BLUE;
        case PL_ALPHA:  return GL_ALPHA;
        default:        return channel;
    }
}

static void SwizzleTexture(const PLTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    GLBindTexture(texture);
    if(GLVersion(3, 3)) {
        int swizzle[] = {
                TranslateColourChannel(r),
                TranslateColourChannel(g),
                TranslateColourChannel(b),
                TranslateColourChannel(a)
        };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    } else {
        ReportError(PL_RESULT_UNSUPPORTED, "missing software implementation");
    }
}

/////////////////////////////////////////////////////////////
// Mesh

typedef struct MeshTranslatePrimitive {
    PLMeshPrimitive     mode;
    unsigned int        target;
    const char          *name;
} MeshTranslatePrimitive;

static MeshTranslatePrimitive primitives[] = {
    {PL_MESH_LINES, GL_LINES, "LINES"},
    {PL_MESH_LINE_LOOP, GL_LINE_LOOP, "LINE_LOOP"},
    {PL_MESH_POINTS, GL_POINTS, "POINTS"},
    {PL_MESH_TRIANGLES, GL_TRIANGLES, "TRIANGLES"},
    {PL_MESH_TRIANGLE_FAN, GL_TRIANGLE_FAN, "TRIANGLE_FAN"},
    {PL_MESH_TRIANGLE_FAN_LINE, GL_LINES, "TRIANGLE_FAN_LINE"},
    {PL_MESH_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP"},
    {PL_MESH_QUADS, GL_TRIANGLES, "QUADS"}   // todo, translate
};

static unsigned int TranslatePrimitiveMode(PLMeshPrimitive mode) {
    for (unsigned int i = 0; i < plArrayElements(primitives); i++) {
        if (mode == primitives[i].mode)
            return primitives[i].target;
    }

    // Hacky, but just return initial otherwise.
    return primitives[0].target;
}

static unsigned int TranslateDrawMode(PLMeshDrawMode mode) {
    switch(mode) {
        case PL_DRAW_DYNAMIC:   return GL_DYNAMIC_DRAW;
        case PL_DRAW_STATIC:    return GL_STATIC_DRAW;
        case PL_DRAW_STREAM:    return GL_STREAM_DRAW;
        default:                return 0;
    }
}

enum {
    BUFFER_VERTEX_DATA,
    BUFFER_TRIANGLE_DATA,
};

static void GLCreateMeshPOST(PLMesh *mesh) {
    GLsizeiptr VBOsize = sizeof(PLVertex) * mesh->num_verts;
    //Create VBO
    glGenBuffers(1, &mesh->internal.buffers[BUFFER_VERTEX_DATA]);
    //Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh->internal.buffers[BUFFER_VERTEX_DATA]);
    //Allocate & populate VBO
    glBufferData(GL_ARRAY_BUFFER, VBOsize, &mesh->vertices[0], GL_DYNAMIC_DRAW); //Todo: Respect draw type
    //We should now have a VBO in VRAM, containing the vertices at time of mesh creation

}

static void GLUploadMesh(PLMesh *mesh) {
    //Write the current CPU vertex data into the VBO
    unsigned int mode = TranslateDrawMode(mesh->mode);
    GLsizeiptr VBOsize = sizeof(PLVertex) * mesh->num_verts;
    glBufferData(GL_ARRAY_BUFFER, VBOsize, &mesh->vertices[0], mode);

    //Point to the different substreams of the interleaved BVO
    //Args: Index, Size, Type, (Normalized), Stride, StartPtr
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PLVertex), (const GLvoid *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,  sizeof(PLVertex), (const GLvoid *)12);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PLVertex), (const GLvoid *)24);
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PLVertex), (const GLvoid *)32);

}

static void GLDeleteMesh(PLMesh *mesh) {
    glDeleteBuffers(1, &mesh->internal.buffers[BUFFER_VERTEX_DATA]);
}

static void GLDrawMesh(PLMesh *mesh) {
    if(mesh->internal.buffers[BUFFER_VERTEX_DATA] == 0) {
        GfxLog("invalid buffer provided, skipping draw!\n");
        return;
    }

    //Write camera matrices to shader shared uniforms
    GLuint view_loc = glGetUniformLocation(gfx_state.current_program->internal.id, "pl_view");
    GLuint proj_loc = glGetUniformLocation(gfx_state.current_program->internal.id, "pl_proj");
    glUniformMatrix4fv( view_loc, 1, GL_FALSE, gfx_state.view_matrix.m);
    glUniformMatrix4fv( proj_loc, 1, GL_FALSE, gfx_state.projection_matrix.m);

    //Ensure VAO/VBO are bound
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->internal.buffers[BUFFER_VERTEX_DATA]);

    //draw
    GLuint mode = TranslatePrimitiveMode(mesh->primitive);
    if(mesh->num_indices > 0) {
        glDrawElements(mode, mesh->num_indices, GL_UNSIGNED_SHORT, mesh->indices);
    } else {
        glDrawArrays(mode, 0, mesh->num_verts);
    }

}

/////////////////////////////////////////////////////////////
// Camera

enum {
    VIEWPORT_FRAMEBUFFER,
    VIEWPORT_RENDERBUFFER_DEPTH,
    VIEWPORT_RENDERBUFFER_COLOUR,
};

static void GLCreateCamera(PLCamera *camera) {
    plAssert(camera);
}

static void GLDeleteCamera(PLCamera *camera) {
    plAssert(camera);
}

static void GLSetupCamera(PLCamera *camera) {
    plAssert(camera);

    int w, h;
    w = camera->viewport.w;
    h = camera->viewport.h;
    if(camera->viewport.auto_scale){
        GLint bound_rbo_w, bound_rbo_h;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &bound_rbo_w);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &bound_rbo_h);
        camera->viewport.x = 0;
        camera->viewport.y = 0;
        camera->viewport.w = bound_rbo_w;
        camera->viewport.h = bound_rbo_h;
    }

    glViewport(camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h);
    glScissor(camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h);

    switch(camera->mode) {
        case PL_CAMERA_MODE_PERSPECTIVE: {
            camera->perspective = plPerspective(
                    camera->fov,
                    (float)w / (float)h,
                    camera->near,
                    camera->far);
            break;
        }

        case PL_CAMERA_MODE_ORTHOGRAPHIC: {
            camera->perspective = plOrtho(0, w, h, 0, camera->near, camera->far);
            break;
        }

        case PL_CAMERA_MODE_ISOMETRIC: {
            camera->perspective = plOrtho(-camera->fov, camera->fov, -camera->fov, 5, -5, 40);
            break;
        }

        default: break;
    }

    // keep the gfx_state up-to-date on the situation
    gfx_state.current_viewport = camera->viewport;

    //Copy camera matrices
    gfx_state.view_matrix = plMatrix4x4Identity();//TODO: Proper camera movement and per-object transforms    
    gfx_state.projection_matrix = camera->perspective;

}

/////////////////////////////////////////////////////////////
// Shader

#define SHADER_INVALID_TYPE ((uint32_t)0 - 1)

static GLenum TranslateShaderStageType(PLShaderStageType type) {
    switch(type) {
        case PL_SHADER_TYPE_VERTEX:     return GL_VERTEX_SHADER;
        case PL_SHADER_TYPE_COMPUTE:    return GL_COMPUTE_SHADER;
        case PL_SHADER_TYPE_FRAGMENT:   return GL_FRAGMENT_SHADER;
        case PL_SHADER_TYPE_GEOMETRY:   return GL_GEOMETRY_SHADER;
        default:                        return SHADER_INVALID_TYPE;
    }
}

static const char *GetGLShaderStageDescriptor(GLenum type) {
    switch(type) {
        case GL_VERTEX_SHADER:      return "GL_VERTEX_SHADER";
        case GL_COMPUTE_SHADER:     return "GL_COMPUTE_SHADER";
        case GL_FRAGMENT_SHADER:    return "GL_FRAGMENT_SHADER";
        case GL_GEOMETRY_SHADER:    return "GL_GEOMETRY_SHADER";
        default:                    return "unknown";
    }
}

static GLenum TranslateShaderUniformType(PLShaderUniformType type) {
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

static unsigned int TranslateGLShaderUniformType(GLenum type) {
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

static void GLCreateShaderProgram(PLShaderProgram *program) {
    if(!GLVersion(2,0)) {
        GfxLog("HW shaders unsupported on platform, relying on SW fallback\n");
        return;
    }

    program->internal.id = glCreateProgram();
    if(program->internal.id == 0) {
        GfxLog("Failed to generate shader program!\n");
        return;
    }
}

static void GLDeleteShaderProgram(PLShaderProgram *program) {
    if(program->internal.id == 0) {
        return;
    }

    if(GLVersion(2,0)) {
        glDeleteProgram(program->internal.id);
    }

    program->internal.id = 0;
}

static void GLCreateShaderStage(PLShaderStage *stage) {
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

static void GLDeleteShaderStage(PLShaderStage *stage) {
    if(!GLVersion(2,0)) {
        return;
    }

    if(stage->program != NULL) {
        glDetachShader(stage->program->internal.id, stage->internal.id);
        stage->program = NULL;
    }
    glDeleteShader(stage->internal.id);
    stage->internal.id = 0;
}

static void GLAttachShaderStage(PLShaderProgram *program, PLShaderStage *stage) {
    if(!GLVersion(2,0)) {
        return;
    }

    glAttachShader(program->internal.id, stage->internal.id);
}

static void GLCompileShaderStage(PLShaderStage *stage, const char *buf, size_t length) {
    if(!GLVersion(2,0)) {
        return;
    }

    glShaderSource(stage->internal.id, 1, &buf, NULL);

    GfxLog("COMPILING SHADER STAGE...\n");
    glCompileShader(stage->internal.id);

    int status;
    glGetShaderiv(stage->internal.id, GL_COMPILE_STATUS, &status);
    if(status == 0) {
        int s_length;
        glGetShaderiv(stage->internal.id, GL_INFO_LOG_LENGTH, &s_length);
        if(s_length > 1) {
            char *log = pl_calloc((size_t) s_length, sizeof(char));
            glGetShaderInfoLog(stage->internal.id, s_length, NULL, log);
            GfxLog(" COMPILE ERROR:\n%s\n",log);
            ReportError(PL_RESULT_SHADER_COMPILE, "%s", log);
            free(log);
        }
    } else {
        GfxLog(" COMPLETED SUCCESSFULLY!\n");
    }
}

static void GLSetShaderUniformMatrix4x4(PLShaderProgram *program, int slot, PLMatrix4x4 value, bool transpose) {
    GLuint loc = (GLuint)slot;
    glUniformMatrix4fv( loc, 1, transpose ? GL_TRUE : GL_FALSE, value.m);
}

static void GLLinkShaderProgram(PLShaderProgram *program) {
    if(!GLVersion(2,0)) {
        return;
    }

    GfxLog("LINKING SHADER PROGRAM...\n");
    glLinkProgram(program->internal.id);

    int status;
    glGetProgramiv(program->internal.id, GL_LINK_STATUS, &status);
    if(status == 0) {
        int s_length;
        glGetProgramiv(program->internal.id, GL_INFO_LOG_LENGTH, &s_length);
        if(s_length > 1) {
            char *log = pl_calloc((size_t)s_length, sizeof(char));
            glGetProgramInfoLog(program->internal.id, s_length, NULL, log);
            GfxLog(" LINK ERROR:\n%s\n", log);
            free(log);

            ReportError(PL_RESULT_SHADER_COMPILE, "%s", log);
        } else {
            GfxLog(" UNKNOWN LINK ERROR!\n");
        }
    } else {
        GfxLog(" LINKED SUCCESSFULLY!\n");
        program->is_linked = true;
    }
}

static void GLSetShaderProgram(PLShaderProgram *program) {
    if(!GLVersion(2,0)) {
        return;
    }

    unsigned int id = 0;
    if(program != NULL) {
        id = program->internal.id;
    }

    glUseProgram(id);
}

/////////////////////////////////////////////////////////////

static char gl_extensions[4096][4096] = { { '\0' } };

#if defined(DEBUG_GL)
static void MessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar *message,
        void *param) {
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

        default:return;
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

void plInitOpenGL(void) {
    glewExperimental = true; 
    GLenum err = glewInit();
    if(err != GLEW_OK) {
        ReportError(PL_RESULT_GRAPHICSINIT, "failed to initialize glew, %s", glewGetErrorString(err));
        return;
    }

    // setup the gfx layer
    gfx_layer.InsertDebugMarker         = GLInsertDebugMarker;
    gfx_layer.PushDebugGroupMarker      = GLPushDebugGroupMarker;
    gfx_layer.PopDebugGroupMarker       = GLPopDebugGroupMarker;

    gfx_layer.HWSupportsShaders         = GLHWSupportsShaders;
    gfx_layer.HWSupportsMultitexture    = GLHWSupportsMultitexture;
    gfx_layer.GetMaxTextureUnits        = GLGetMaxTextureUnits;

    gfx_layer.SetBlendMode              = GLSetBlendMode;
    gfx_layer.SetCullMode               = GLSetCullMode;

    gfx_layer.SetClearColour            = GLSetClearColour;
    gfx_layer.ClearBuffers              = GLClearBuffers;

    gfx_layer.CreateFrameBuffer         = GLCreateFrameBuffer;
    gfx_layer.DeleteFrameBuffer         = GLDeleteFrameBuffer;
    gfx_layer.BindFrameBuffer           = GLBindFrameBuffer;
    gfx_layer.BlitFrameBuffers          = GLBlitFrameBuffers;

    gfx_layer.CreateTexture             = GLCreateTexture;
    gfx_layer.DeleteTexture             = GLDeleteTexture;
    gfx_layer.BindTexture               = GLBindTexture;
    gfx_layer.UploadTexture             = GLUploadTexture;
    gfx_layer.SetTextureAnisotropy      = GLSetTextureAnisotropy;
    gfx_layer.ActiveTexture             = GLActiveTexture;

    gfx_layer.CreateMeshPOST            = GLCreateMeshPOST;
    gfx_layer.DeleteMesh                = GLDeleteMesh;
    gfx_layer.DrawMesh                  = GLDrawMesh;
    gfx_layer.UploadMesh                = GLUploadMesh;

    gfx_layer.CreateCamera              = GLCreateCamera;
    gfx_layer.DeleteCamera              = GLDeleteCamera;
    gfx_layer.SetupCamera               = GLSetupCamera;

    gfx_layer.CreateShaderProgram       = GLCreateShaderProgram;
    gfx_layer.DeleteShaderProgram       = GLDeleteShaderProgram;
    gfx_layer.SetShaderProgram          = GLSetShaderProgram;
    gfx_layer.LinkShaderProgram         = GLLinkShaderProgram;
    gfx_layer.CreateShaderStage         = GLCreateShaderStage;
    gfx_layer.DeleteShaderStage         = GLDeleteShaderStage;
    gfx_layer.AttachShaderStage         = GLAttachShaderStage;
    gfx_layer.CompileShaderStage        = GLCompileShaderStage;

    gfx_layer.SetShaderUniformMatrix4x4   = GLSetShaderUniformMatrix4x4;

    /////////////////////////////////////////////////////////////

    // Get any information that will be presented later.
    gfx_state.hw_extensions     = (const char *) glGetString(GL_EXTENSIONS);
    gfx_state.hw_renderer       = (const char *) glGetString(GL_RENDERER);
    gfx_state.hw_vendor         = (const char *) glGetString(GL_VENDOR);
    gfx_state.hw_version        = (const char *) glGetString(GL_VERSION);

    memset(&gl_capabilities, 0, sizeof(gl_capabilities));

    glGetIntegerv(GL_MINOR_VERSION, &gl_version_minor);
    glGetIntegerv(GL_MAJOR_VERSION, &gl_version_major);
    if(gl_version_major <= 0 && gl_version_minor <= 0) {
        if(gfx_state.hw_version != NULL && (gfx_state.hw_version[0] != '\0' && gfx_state.hw_version[2] != '\0')) {
            gl_version_major = (gfx_state.hw_version[0] - '0');
            gl_version_minor = (gfx_state.hw_version[2] - '0');
        } else {
            GfxLog("failed to get OpenGL version, expect some functionality not to work!\n");
        }
    }

    GfxLog(" OpenGL %d.%d\n", gl_version_major, gl_version_minor);
    GfxLog("  renderer:   %s\n", gfx_state.hw_renderer);
    GfxLog("  vendor:     %s\n", gfx_state.hw_vendor);
    GfxLog("  version:    %s\n", gfx_state.hw_version);
    GfxLog("  extensions:\n");

    if(GLVersion(3,0)) {
        glGetIntegerv(GL_NUM_EXTENSIONS, (GLint *) (&gl_num_extensions));
        for (unsigned int i = 0; i < gl_num_extensions; ++i) {
            const uint8_t *extension = glGetStringi(GL_EXTENSIONS, i);
            sprintf(gl_extensions[i], "%s", extension);
            GfxLog("    %s\n", extension);
        }
    } else {
        // todo
    }

#if defined(DEBUG_GL)
    if(GLVersion(4,3)) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glDebugMessageCallback((GLDEBUGPROC) MessageCallback, NULL);
    }
#endif

    if(GLEW_ARB_direct_state_access || GLVersion(4,5)) {
        gl_capabilities.direct_state_access = true;
    }


    // Init vertex attributes
    glGenVertexArrays(1, VAO);
    glBindVertexArray(VAO[0]);

}

void plShutdownOpenGL(void) {
#if defined(DEBUG_GL)
    if(GLVersion(4,3)) {
        glDisable(GL_DEBUG_OUTPUT);
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
#endif
}

#endif
