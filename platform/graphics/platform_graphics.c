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
#include <PL/platform_log.h>
#include <PL/platform_image.h>
#include <PL/platform_console.h>

#include "graphics_private.h"

/*	Graphics	*/

GfxState gfx_state;
GfxLayer gfx_layer;

/*	TODO:
- Add somewhere we can store tracking
data for each of these functions
- Do this in another thread if possible
- Display that data as an overlay
*/
#define GRAPHICS_TRACK()                            \
    {                                               \
        static unsigned int num_calls = 0;          \
        if(gfx_state.mode_debug) {                  \
            plGraphicsLog(" %s\n", PL_FUNCTION);    \
            num_calls++;                            \
        }                                           \
    }

/*===========================
	INITIALIZATION
===========================*/

void InitTextures(void);     // platform_graphics_texture
void InitCameras(void);      // platform_graphics_camera
void InitMaterials(void);    // material

PLresult InitGraphics(void) {
    GRAPHICS_TRACK();

    plClearLog(PL_GRAPHICS_LOG);
    plGraphicsLog("Initializing graphics abstraction layer...\n");

    memset(&gfx_state, 0, sizeof(GfxState));

    gfx_layer.mode = PL_GFX_MODE_OPENGL; // todo, temp
    switch(gfx_layer.mode) {
        default: {
            ReportError(PL_RESULT_GRAPHICSINIT, "Invalid graphics layer, %d, selected!\n", gfx_layer.mode);
            _plDebugPrint("Reverting to software mode...\n");
            gfx_layer.mode = PL_GFX_MODE_SOFTWARE;
            return InitGraphics();
        }

        case PL_GFX_MODE_DIRECT3D: break;

        case PL_GFX_MODE_OPENGL_CORE:
        case PL_GFX_MODE_OPENGL_ES:
        case PL_GFX_MODE_OPENGL: {
            InitOpenGL();
        } break;
    }

    InitCameras();
    InitTextures();
    InitMaterials();

    return PL_RESULT_SUCCESS;
}

void ShutdownTextures(void); // platform_graphics_texture
void ShutdownCameras(void);  // platform_graphics_camera

void ShutdownGraphics(void) {
    GRAPHICS_TRACK();

    ShutdownCameras();
    ShutdownTextures();

    switch(gfx_layer.mode) {
        default: break;

        case PL_GFX_MODE_OPENGL_CORE:
        case PL_GFX_MODE_OPENGL_ES:
        case PL_GFX_MODE_OPENGL: {
            ShutdownOpenGL();
        } break;
    }
}

/*===========================
	HARDWARE INFORMATION
===========================*/

bool plHWSupportsMultitexture(void) {
    GRAPHICS_TRACK();

    if(gfx_layer.HWSupportsMultitexture) {
        return gfx_layer.HWSupportsMultitexture();
    }

    return false;
}

bool plHWSupportsShaders(void) {
    GRAPHICS_TRACK();

    if(gfx_layer.HWSupportsShaders) {
        return gfx_layer.HWSupportsShaders();
    }

    return false;
}

/*===========================
	FRAMEBUFFERS
===========================*/

PLFrameBuffer *plCreateFrameBuffer(unsigned int w, unsigned int h) {
    PLFrameBuffer *buffer = (PLFrameBuffer*)malloc(sizeof(PLFrameBuffer));
    if(!buffer) {
        ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate %d bytes for FrameBuffer!", sizeof(PLFrameBuffer));
        return NULL;
    }

#if 0
    glGenFramebuffers(1, &buffer->fbo);
    glGenRenderbuffers(1, &buffer->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer->fbo);
    glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer->rbo);
#endif

    return buffer;
}

void plDeleteFrameBuffer(PLFrameBuffer *buffer) {
    if(!buffer) {
        return;
    }

#if 0
    glDeleteFramebuffers(1, &buffer->fbo);
    glDeleteRenderbuffers(1, &buffer->rbo);
#endif
}

void plBindFrameBuffer(PLFrameBuffer *buffer) {
    if(!buffer) {
        // todo, warning regarding invalid buffer blah blah
        return;
    }

#if defined(PL_MODE_OPENGL)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer->fbo);
#endif
}

void plSetClearColour(PLColour rgba) {
    if (plCompareColour(rgba, gfx_state.current_clearcolour)) {
        return;
    }

#if 0
    glClearColor(
            plByteToFloat(rgba.r),
            plByteToFloat(rgba.g),
            plByteToFloat(rgba.b),
            plByteToFloat(rgba.a)
    );
#endif

    plCopyColour(&gfx_state.current_clearcolour, rgba);
}

void plClearBuffers(unsigned int buffers) {
    GRAPHICS_TRACK();

#if defined (PL_MODE_OPENGL) || defined (PL_MODE_OPENGL_CORE)
    // Rather ugly, but translate it over to GL.
    unsigned int glclear = 0;
    if(buffers & PL_BUFFER_COLOUR)  glclear |= GL_COLOR_BUFFER_BIT;
    if(buffers & PL_BUFFER_DEPTH)   glclear |= GL_DEPTH_BUFFER_BIT;
    if(buffers & PL_BUFFER_STENCIL) glclear |= GL_STENCIL_BUFFER_BIT;
    glClear(glclear);
#elif defined (PL_MODE_GLIDE)
    // Glide only supports clearing a single buffer.
    grBufferClear(
        // Convert buffer_clearcolour to something that works with Glide.
        _plConvertColour4fv(PL_COLOURFORMAT_RGBA, graphics_state.buffer_clearcolour),
        1, 1);
#elif defined (PL_MODE_DIRECT3D)
    pl_d3d_context->lpVtbl->ClearRenderTargetView(pl_d3d_context,
        pl_d3d_backbuffer,
        graphics_state.current_clearcolour
    );
#elif defined (PL_MODE_SOFTWARE)
    if(buffers & PL_BUFFER_COLOUR) {
        for (unsigned int i = 0; i < pl_sw_backbuffer_size; i += 4) {
            pl_sw_backbuffer[i] = gfx_state.current_clearcolour.r;
            pl_sw_backbuffer[i + 1] = gfx_state.current_clearcolour.g;
            pl_sw_backbuffer[i + 2] = gfx_state.current_clearcolour.b;
            pl_sw_backbuffer[i + 3] = gfx_state.current_clearcolour.a;
        }
    }
#endif
}

/*===========================
	CAPABILITIES
===========================*/

typedef struct _PLGraphicsCapabilities {
    PLuint pl_parm, to_parm;

    const PLchar *ident;
} _PLGraphicsCapabilities;

_PLGraphicsCapabilities graphics_capabilities[] =
        {
#if defined (PL_MODE_OPENGL)
#if defined (PL_MODE_OPENGL_CORE)
                {PL_CAPABILITY_ALPHA_TEST, 0, "ALPHA_TEST"},
#else
                {PL_CAPABILITY_ALPHA_TEST, GL_ALPHA_TEST, "ALPHA_TEST"},
#endif
                {PL_CAPABILITY_BLEND, GL_BLEND, "BLEND"},
                {PL_CAPABILITY_DEPTHTEST, GL_DEPTH_TEST, "DEPTH_TEST"},
                {PL_CAPABILITY_TEXTURE_2D, GL_TEXTURE_2D, "TEXTURE_2D"},
#if defined (PL_MODE_OPENGL_CORE)
                {PL_CAPABILITY_TEXTURE_GEN_S, 0, "TEXTURE_GEN_S"},
                {PL_CAPABILITY_TEXTURE_GEN_T, 0, "TEXTURE_GEN_T"},
#else
                {PL_CAPABILITY_TEXTURE_GEN_S, GL_TEXTURE_GEN_S, "TEXTURE_GEN_S"},
                {PL_CAPABILITY_TEXTURE_GEN_T, GL_TEXTURE_GEN_T, "TEXTURE_GEN_T"},
#endif
                {PL_CAPABILITY_CULL_FACE, GL_CULL_FACE, "CULL_FACE"},
                {PL_CAPABILITY_STENCILTEST, GL_STENCIL_TEST, "STENCIL_TEST"},
                {PL_CAPABILITY_MULTISAMPLE, GL_MULTISAMPLE, "MULTISAMPLE"},
                {PL_CAPABILITY_SCISSORTEST, GL_SCISSOR_TEST, "SCISSOR_TEST"},

                {PL_CAPABILITY_GENERATEMIPMAP, 0, "GENERATE_MIPMAP"},
#else
        { PL_CAPABILITY_ALPHA_TEST, 0, "ALPHA_TEST" },
        { PL_CAPABILITY_BLEND, 0, "BLEND" },
        { PL_CAPABILITY_DEPTHTEST, 0, "DEPTH_TEST" },
        { PL_CAPABILITY_TEXTURE_2D, 0, "TEXTURE_2D" },
        { PL_CAPABILITY_TEXTURE_GEN_S, 0, "TEXTURE_GEN_S" },
        { PL_CAPABILITY_TEXTURE_GEN_T, 0, "TEXTURE_GEN_T" },
        { PL_CAPABILITY_CULL_FACE, 0, "CULL_FACE" },
        { PL_CAPABILITY_STENCILTEST, 0, "STENCIL_TEST" },
        { PL_CAPABILITY_MULTISAMPLE, 0, "MULTISAMPLE" },
#endif

                {0}
        };

bool plIsGraphicsStateEnabled(PLuint flags) {
    GRAPHICS_TRACK();

    return (bool)(flags & gfx_state.current_capabilities);
}

void plEnableGraphicsStates(PLuint flags) {
    GRAPHICS_TRACK();

    if (plIsGraphicsStateEnabled(flags)) {
        return;
    }

    for (unsigned int i = 0; i < sizeof(graphics_capabilities); i++) {
        if (graphics_capabilities[i].pl_parm == 0) {
            break;
        }

        if (gfx_state.mode_debug) {
            plGraphicsLog("Enabling %s\n", graphics_capabilities[i].ident);
        }

        if (flags & PL_CAPABILITY_TEXTURE_2D) {
            gfx_state.tmu[gfx_state.current_textureunit].active = true;
        }
#if defined (VL_MODE_GLIDE)
        if (flags & PL_CAPABILITY_FOG)
            // TODO: need to check this is supported...
            grFogMode(GR_FOG_WITH_TABLE_ON_FOGCOORD_EXT);
        if (flags & PL_CAPABILITY_DEPTHTEST)
            grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
        if (flags & PL_CAPABILITY_CULL_FACE)
            grCullMode(graphics_state.current_cullmode);
#endif

        if ((flags & graphics_capabilities[i].pl_parm) && (graphics_capabilities[i].to_parm != 0)) {
#if defined (PL_MODE_OPENGL)
            glEnable(graphics_capabilities[i].to_parm);
#elif defined (VL_MODE_GLIDE)
            grEnable(graphics_capabilities[i].to_parm);
#endif
        }

        gfx_state.current_capabilities |= graphics_capabilities[i].pl_parm;
    }
}

void plDisableGraphicsStates(PLuint flags) {
    GRAPHICS_TRACK();

    if (!plIsGraphicsStateEnabled(flags)) {
        return;
    }

    for (unsigned int i = 0; i < sizeof(graphics_capabilities); i++) {
        if (graphics_capabilities[i].pl_parm == 0) break;

        if (gfx_state.mode_debug) {
            plGraphicsLog("Disabling %s\n", graphics_capabilities[i].ident);
        }

        if (flags & PL_CAPABILITY_TEXTURE_2D) {
            gfx_state.tmu[gfx_state.current_textureunit].active = false;
        }
#if defined (VL_MODE_GLIDE)
        if (flags & PL_CAPABILITY_FOG)
            grFogMode(GR_FOG_DISABLE);
        if (flags & PL_CAPABILITY_DEPTHTEST)
            grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
        if (flags & PL_CAPABILITY_CULL_FACE)
            grCullMode(graphics_state.current_cullmode);
#endif

        if ((flags & graphics_capabilities[i].pl_parm) && (graphics_capabilities[i].to_parm != 0)) {
#if defined (PL_MODE_OPENGL)
            glDisable(graphics_capabilities[i].to_parm);
#elif defined (VL_MODE_GLIDE)
            grDisable(graphics_capabilities[i].to_parm);
#endif
        }

        gfx_state.current_capabilities &= ~graphics_capabilities[i].pl_parm;
    }
}

/*===========================
	DRAW
===========================*/

void plSetBlendMode(PLBlend a, PLBlend b) {
    if(gfx_layer.SetBlendMode) {
        gfx_layer.SetBlendMode(a, b);
    }
}

void plSetCullMode(PLCullMode mode) {
    if(gfx_layer.SetCullMode) {
        gfx_layer.SetCullMode(mode);
    }
}

/*===========================
	SHADERS
===========================*/

unsigned int plGetCurrentShaderProgram(void) {
    return gfx_state.current_program;
}

void plEnableShaderProgram(unsigned int program) {
    GRAPHICS_TRACK();

    if (program == gfx_state.current_program) {
        return;
    }

   // glUseProgram(program);

    gfx_state.current_program = program;
}

void plDisableShaderProgram(unsigned int program) {
    GRAPHICS_TRACK();

    if(program != gfx_state.current_program) {
        return;
    }

   // glUseProgram(0);

    gfx_state.current_program = 0;
}

/*===========================
	TEXTURES
===========================*/

#if 0
PLresult plUploadTextureData(PLTexture *texture, const PLTextureInfo *upload) {
    GRAPHICS_TRACK();

    _plSetActiveTexture(texture);

#if defined (PL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    PLuint storage = _plTranslateTextureStorageFormat(upload->storage_type);
    PLuint format = _plTranslateTextureFormat(upload->format);

    PLuint levels = upload->levels;
    if(!levels) {
        levels = 1;
    }

    if (upload->initial) {
        glTexStorage2D(GL_TEXTURE_2D, levels, format, upload->width, upload->height);
    }

    // Check the format, to see if we're getting a compressed
    // format type.
    if (_plIsCompressedTextureFormat(upload->format)) {
        glCompressedTexSubImage2D
                (
                        GL_TEXTURE_2D,
                        0,
                        upload->x, upload->y,
                        upload->width, upload->height,
                        format,
                        upload->size,
                        upload->data
                );
    } else {
        glTexSubImage2D
                (
                        GL_TEXTURE_2D,
                        0,
                        upload->x, upload->y,
                        upload->width, upload->height,
                        _plTranslateColourFormat(upload->pixel_format),
                        storage,
                        upload->data
                );
    }

    if (plIsGraphicsStateEnabled(PL_CAPABILITY_GENERATEMIPMAP) && (levels > 1)) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
#elif defined (VL_MODE_GLIDE)
#elif defined (VL_MODE_DIRECT3D)
#endif

    return PL_RESULT_SUCCESS;
}
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic pop
#endif

/*===========================
	UTILITY FUNCTIONS
===========================*/

#if 0
void plScreenshot(PLViewport *viewport, const PLchar *path) {
    uint8_t *buffer = (uint8_t*)calloc(viewport->height * viewport->width * 3, sizeof(uint8_t));
    glReadPixels(viewport->x, viewport->y, viewport->width, viewport->height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    free(buffer);
}
#endif

void plSetDefaultGraphicsState(void) {
    plSetClearColour(plCreateColour4b(PL_COLOUR_BLACK));
    plSetCullMode(PL_CULL_NEGATIVE);
    plSetTextureUnit(0);

#if defined(PL_MODE_OPENGL)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#if !defined(GL_VERSION_3_0)
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#endif

    glDepthRange(0, 1);
    glDepthFunc(GL_LEQUAL);
#endif

    plEnableGraphicsStates(PL_CAPABILITY_SCISSORTEST);
}

/* Shared Matrix Functions */

void plLoadMatrixIdentity(void) {

}

void plTranslateMatrix(PLVector3 translation) {
    if(plCompareVector3(translation, PLVector3(0, 0, 0))) {
        return;
    }
}

void plScaleMatrix(PLVector3 scale) {
    if(plCompareVector3(scale, PLVector3(0, 0, 0))) {
        return;
    }
}

void plSetMatrixMode(unsigned int mode) {

}

/////////////////////////////////////////////////////////////////////////////////////
// VIEWPORT/CAMERA

// http://nehe.gamedev.net/article/replacement_for_gluperspective/21002/
void plPerspective(double fov_y, double aspect, double near, double far) {
    GRAPHICS_TRACK();

#if defined(PL_MODE_OPENGL)
    double h = tan(fov_y / 360 * PL_PI) * near;
    double w = h * aspect;
    glFrustum(-w, w, -h, h, near, far);
#endif
}

/* DRAWING  */

void plDrawPixel(int x, int y, PLColour colour) {
}

/////////////////////////////////////////////////////////////////////////////////////

void plProcessGraphics(void) {
    plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH | PL_BUFFER_STENCIL);

    plDrawConsole();
    plDrawPerspective();
}