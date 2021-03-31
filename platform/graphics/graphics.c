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

#include <PL/platform_console.h>
#include <PL/platform_image.h>

#include "graphics_private.h"

/*	Graphics	*/

GfxState gfx_state;

/*	TODO:
- Add somewhere we can store tracking
data for each of these functions
- Do this in another thread if possible
- Display that data as an overlay
*/
#define GRAPHICS_TRACK()                    \
	{                                       \
		static unsigned int num_calls = 0;  \
		if ( gfx_state.mode_debug ) {       \
			GfxLog( " %s\n", PL_FUNCTION ); \
			num_calls++;                    \
		}                                   \
	}

/*===========================
	INITIALIZATION
===========================*/

void _InitTextures( void );// platform_graphics_texture

PLresult plInitGraphics( void ) {
	memset( &gfx_state, 0, sizeof( GfxState ) );

	return PL_RESULT_SUCCESS;
}

void plShutdownTextures( void );// platform_graphics_texture

void plShutdownGraphics( void ) {
	GRAPHICS_TRACK();

	CallGfxFunction( Shutdown );

	plShutdownTextures();
}

/*===========================
	DEBUGGING
===========================*/

void plInsertDebugMarker( const char *msg ) {
	CallGfxFunction( InsertDebugMarker, msg );
}

void plPushDebugGroupMarker( const char *msg ) {
	CallGfxFunction( PushDebugGroupMarker, msg );
}

void plPopDebugGroupMarker( void ) {
	CallGfxFunction( PopDebugGroupMarker );
}

/*===========================
	HARDWARE INFORMATION
===========================*/

bool plSupportsHWShaders( void ) {
	GRAPHICS_TRACK();
	CallReturningGfxFunction( SupportsHWShaders, false );
}

/*===========================
	FRAMEBUFFERS
===========================*/

PLFrameBuffer *plCreateFrameBuffer( unsigned int w, unsigned int h, unsigned int flags ) {
	if ( flags == 0 ) {
		return NULL;
	}

	PLFrameBuffer *buffer = ( PLFrameBuffer * ) pl_malloc( sizeof( PLFrameBuffer ) );
	if ( !buffer ) {
		return NULL;
	}

	buffer->width = w;
	buffer->height = h;
	buffer->flags = flags;

	CallGfxFunction( CreateFrameBuffer, buffer );

	return buffer;
}

void plDestroyFrameBuffer( PLFrameBuffer *buffer ) {
	if ( !buffer ) {
		return;
	}

	CallGfxFunction( DeleteFrameBuffer, buffer );

	pl_free( buffer );
}

PLTexture *plGetFrameBufferTextureAttachment( PLFrameBuffer *buffer, unsigned int component, PLTextureFilter filter ) {
	CallReturningGfxFunction( GetFrameBufferTextureAttachment, NULL, buffer, component, filter );
}

void plGetFrameBufferResolution( const PLFrameBuffer *buffer, unsigned int *width, unsigned int *height ) {
	*width = buffer->width;
	*height = buffer->height;
}

void plBindFrameBuffer( PLFrameBuffer *buffer, PLFBOTarget target_binding ) {
	//NOTE: NULL is valid for *buffer, to bind the SDL window default backbuffer
	CallGfxFunction( BindFrameBuffer, buffer, target_binding )
}

void plBlitFrameBuffers( PLFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, bool linear ) {
	//NOTE: NULL is valid for *srcBuffer/*dstBuffer, to bind the SDL window default backbuffer
	//      SRC and DST can be the same buffer, in order to quickly copy a subregion of the buffer to a new location
	CallGfxFunction( BlitFrameBuffers, src_buffer, src_w, src_h, dst_buffer, dst_w, dst_h, linear );
}

void plSetClearColour( PLColour rgba ) {
	if ( plCompareColour( rgba, gfx_state.current_clearcolour ) ) {
		return;
	}

	CallGfxFunction( SetClearColour, rgba );

	gfx_state.current_clearcolour = rgba;
}

void plClearBuffers( unsigned int buffers ) {
	CallGfxFunction( ClearBuffers, buffers );
}

/*===========================
	CAPABILITIES
===========================*/

bool plIsGraphicsStateEnabled( PLGraphicsState state ) {
	return gfx_state.current_capabilities[ state ];
}

void plEnableGraphicsState( PLGraphicsState state ) {
	if ( plIsGraphicsStateEnabled( state ) ) {
		return;
	}

	CallGfxFunction( EnableState, state );

	gfx_state.current_capabilities[ state ] = false;
}

void plDisableGraphicsState( PLGraphicsState state ) {
	if ( !plIsGraphicsStateEnabled( state ) ) {
		return;
	}

	CallGfxFunction( DisableState, state );

	gfx_state.current_capabilities[ state ] = true;
}

/*===========================
	DRAW
===========================*/

void plSetBlendMode( PLBlend a, PLBlend b ) {
	CallGfxFunction( SetBlendMode, a, b );
}

void plSetCullMode( PLCullMode mode ) {
	if ( mode == gfx_state.current_cullmode ) {
		return;
	}

	CallGfxFunction( SetCullMode, mode );

	gfx_state.current_cullmode = mode;
}

void plSetDepthBufferMode( unsigned int mode ) {
	CallGfxFunction( SetDepthBufferMode, mode );
}

void plSetDepthMask( bool enable ) {
	CallGfxFunction( SetDepthMask, enable );
}

/*===========================
	TEXTURES
===========================*/

#if 0
PLresult plUploadTextureData(PLTexture *texture, const PLTextureInfo *upload) {
    GRAPHICS_TRACK();

    _plSetActiveTexture(texture);

#if defined( PL_MODE_OPENGL ) || defined( VL_MODE_OPENGL_CORE )
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
#elif defined( VL_MODE_GLIDE )
#elif defined( VL_MODE_DIRECT3D )
#endif

    return PL_RESULT_SUCCESS;
}
#endif

#if defined( __GNUC__ ) || defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif

#if defined( __GNUC__ ) || defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

/*===========================
	UTILITY FUNCTIONS
===========================*/

void plDrawPixel( int x, int y, PLColour colour ) {
	CallGfxFunction( DrawPixel, x, y, colour );
}

/////////////////////////////////////////////////////////////////////////////////////

typedef struct GraphicsMode {
	const char *description;
	const PLGraphicsInterface *interface;
} GraphicsMode;
static GraphicsMode graphicsModes[ MAX_OBJECT_INTERFACES ];
static unsigned int numGraphicsModes = 0;

void plRegisterGraphicsMode( const char *description, const PLGraphicsInterface *interface ) {
	if ( numGraphicsModes >= MAX_OBJECT_INTERFACES ) {
		ReportBasicError( PL_RESULT_MEMORY_EOA );
		return;
	}

	graphicsModes[ numGraphicsModes ].description = description;
	graphicsModes[ numGraphicsModes ].interface = interface;
	numGraphicsModes++;
}

void plSetGraphicsMode( const char *mode ) {
	FunctionStart();

	GfxLog( "Initializing graphics abstraction layer...\n" );

	const PLGraphicsInterface *interface = NULL;
	for ( unsigned int i = 0; i < numGraphicsModes; ++i ) {
		if ( pl_strcasecmp( mode, graphicsModes[ i ].description ) != 0 ) {
			continue;
		}

		interface = graphicsModes[ i ].interface;
		break;
	}

	if ( interface == NULL ) {
		ReportError( PL_RESULT_GRAPHICSINIT, "invalid graphics interface \"%s\" selected", mode );
		return;
	}

	if ( gfx_state.interface != NULL && interface == gfx_state.interface ) {
		ReportError( PL_RESULT_GRAPHICSINIT, "chosen interface \"%s\" is already active", mode );
		return;
	}

	gfx_state.interface = interface;

	CallGfxFunction( Initialize );

	_InitTextures();
}

/*===========================
	STENCIL OPERATIONS
===========================*/

void plStencilFunction( PLStencilTestFunction function, int reference, unsigned int mask ) {
	CallGfxFunction( StencilFunction, function, reference, mask );
}
