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
            GfxLog(" %s\n", PL_FUNCTION);    \
            num_calls++;                            \
        }                                           \
    }

/*===========================
	INITIALIZATION
===========================*/

void _InitTextures(void);     // platform_graphics_texture
void _InitMaterials(void);    // material

PLresult plInitGraphics(void) {
    memset(&gfx_state, 0, sizeof(GfxState));
    memset(&gfx_layer, 0, sizeof(GfxLayer));

    gfx_layer.mode = PL_GFX_MODE_NONE;

    return PL_RESULT_SUCCESS;
}

void plShutdownTextures(void); // platform_graphics_texture

void plShutdownGraphics(void) {
    GRAPHICS_TRACK();

    plShutdownTextures();

    switch(gfx_layer.mode) {
        default: break;

        case PL_GFX_MODE_OPENGL_CORE:
        case PL_GFX_MODE_OPENGL_ES:
        case PL_GFX_MODE_OPENGL: {
#if defined(PL_SUPPORT_OPENGL)
            plShutdownOpenGL();
#endif
        } break;
    }
}

/*===========================
	DEBUGGING
===========================*/

void plInsertDebugMarker(const char *msg) {
    CallGfxFunction(InsertDebugMarker, msg);
}

void plPushDebugGroupMarker(const char *msg) {
    CallGfxFunction(PushDebugGroupMarker, msg);
}

void plPopDebugGroupMarker(void) {
    CallGfxFunction(PopDebugGroupMarker);
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

PLFrameBuffer *plCreateFrameBuffer(unsigned int w, unsigned int h, unsigned int flags) {
    if(flags == 0){
        return NULL;
    }

    PLFrameBuffer *buffer = (PLFrameBuffer*)pl_malloc(sizeof(PLFrameBuffer));
    if(!buffer) {
        return NULL;
    }

    buffer->width = w;
    buffer->height = h;
    buffer->flags = flags;

    CallGfxFunction(CreateFrameBuffer, buffer);

    return buffer;
}


void plDestroyFrameBuffer(PLFrameBuffer *buffer) {
	if(!buffer) {
		return;
	}

	CallGfxFunction(DeleteFrameBuffer, buffer);

	pl_free(buffer);
}

PLTexture *plGetFrameBufferTextureAttachment( PLFrameBuffer *buffer ) {
	if ( gfx_layer.GetFrameBufferTextureAttachment == NULL ) {
		return NULL;
	}

	return gfx_layer.GetFrameBufferTextureAttachment( buffer );
}

void plGetFrameBufferResolution( const PLFrameBuffer *buffer, unsigned int *width, unsigned int *height ) {
	*width = buffer->width;
	*height = buffer->height;
}

void plBindFrameBuffer(PLFrameBuffer *buffer, PLFBOTarget target_binding) {
    //NOTE: NULL is valid for *buffer, to bind the SDL window default backbuffer
    CallGfxFunction(BindFrameBuffer, buffer, target_binding)
}

void plBlitFrameBuffers(PLFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, bool linear ) {
    //NOTE: NULL is valid for *srcBuffer/*dstBuffer, to bind the SDL window default backbuffer
    //      SRC and DST can be the same buffer, in order to quickly copy a subregion of the buffer to a new location
    CallGfxFunction(BlitFrameBuffers, src_buffer, src_w, src_h, dst_buffer, dst_w, dst_h, linear);
}

void plSetClearColour(PLColour rgba) {
    if (plCompareColour(rgba, gfx_state.current_clearcolour)) {
        return;
    }

    CallGfxFunction(SetClearColour, rgba);

    gfx_state.current_clearcolour = rgba;
}

void plClearBuffers(unsigned int buffers) {
    CallGfxFunction( ClearBuffers, buffers );
}

/*===========================
	CAPABILITIES
===========================*/

bool plIsGraphicsStateEnabled(PLGraphicsState state) {
  return gfx_state.current_capabilities[state];
}

void plEnableGraphicsState(PLGraphicsState state) {
    if (plIsGraphicsStateEnabled(state)) {
        return;
    }

    CallGfxFunction(EnableState, state);
}

void plDisableGraphicsState(PLGraphicsState state) {
    if (!plIsGraphicsStateEnabled(state)) {
        return;
    }

    CallGfxFunction(DisableState, state);
}

/*===========================
	DRAW
===========================*/

void plSetBlendMode(PLBlend a, PLBlend b) {
    CallGfxFunction(SetBlendMode, a, b);
}

void plSetCullMode(PLCullMode mode) {
    CallGfxFunction(SetCullMode, mode);
}

void plSetDepthBufferMode(unsigned int mode) {
    CallGfxFunction(SetDepthBufferMode, mode);
}

void plSetDepthMask(bool enable) {
    CallGfxFunction(SetDepthMask, enable);
}

/*===========================
	TEXTURES
===========================*/

#if 0
PLresult plUploadTextureData(PLTexture *texture, const PLTextureInfo *upload) {
    GRAPHICS_TRACK();

    _plSetActiveTexture(texture);

#if defined (PL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    unsigned int storage = _plTranslateTextureStorageFormat(upload->storage_type);
    unsigned int format = _plTranslateTextureFormat(upload->format);

    unsigned int levels = upload->levels;
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
void plScreenshot(PLViewport *viewport, const char *path) {
    uint8_t *buffer = (uint8_t*)pl_calloc(viewport->height * viewport->width * 3, sizeof(uint8_t));
    glReadPixels(viewport->x, viewport->y, viewport->width, viewport->height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    pl_free(buffer);
}
#endif

/* DRAWING  */

void plDrawPixel(int x, int y, PLColour colour) {
}

/////////////////////////////////////////////////////////////////////////////////////

void plSetGraphicsMode(PLGfxMode mode) {
    GfxLog("Initializing graphics abstraction layer...\n");

    gfx_layer.mode = mode;
    switch(gfx_layer.mode) {
        case PL_GFX_MODE_NONE: break;

#if defined(PL_SUPPORT_DIRECT3D)
        case PL_GFX_MODE_DIRECT3D: break;
#endif

        case PL_GFX_MODE_SOFTWARE: {
            plInitSoftwareGraphicsLayer();
        } break;

#if defined(PL_SUPPORT_OPENGL)
        case PL_GFX_MODE_OPENGL_CORE:
        case PL_GFX_MODE_OPENGL_ES:
        case PL_GFX_MODE_OPENGL: {
            plInitOpenGL();
        } break;
#endif

        default: {
            ReportError(PL_RESULT_GRAPHICSINIT, "invalid graphics layer, %d, selected", gfx_layer.mode);
            DebugPrint("Reverting to software mode...\n");
            plSetGraphicsMode(PL_GFX_MODE_SOFTWARE);
        } break;
    }

    _InitTextures();
    _InitMaterials();
}

void plProcessGraphics(void) {
    plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH | PL_BUFFER_STENCIL);

    plDrawConsole();
    //plDrawPerspective();
}
