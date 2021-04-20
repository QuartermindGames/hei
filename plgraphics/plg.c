/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <PL/platform_console.h>
#include <PL/platform_image.h>

#include "plg_private.h"

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

int LOG_LEVEL_GRAPHICS = 0;

PLFunctionResult PlgInitGraphics( void ) {
	memset( &gfx_state, 0, sizeof( GfxState ) );

	LOG_LEVEL_GRAPHICS = plAddLogLevel( "plgraphics", ( PLColour ){ 0, 255, 255, 255 },
#if !defined( NDEBUG )
	                                    true
#else
	                                    false
#endif
	);

	return PL_RESULT_SUCCESS;
}

void PlgShutdownTextures( void );// platform_graphics_texture

void PlgShutdownGraphics( void ) {
	GRAPHICS_TRACK();

	CallGfxFunction( Shutdown );

	PlgShutdownTextures();
}

/*===========================
	DEBUGGING
===========================*/

void PlgInsertDebugMarker( const char *msg ) {
	CallGfxFunction( InsertDebugMarker, msg );
}

void PlgPushDebugGroupMarker( const char *msg ) {
	CallGfxFunction( PushDebugGroupMarker, msg );
}

void PlgPopDebugGroupMarker( void ) {
	CallGfxFunction( PopDebugGroupMarker );
}

/*===========================
	HARDWARE INFORMATION
===========================*/

bool PlgSupportsHWShaders( void ) {
	GRAPHICS_TRACK();
	CallReturningGfxFunction( SupportsHWShaders, false );
}

/*===========================
	FRAMEBUFFERS
===========================*/

PLGFrameBuffer *PlgCreateFrameBuffer( unsigned int w, unsigned int h, unsigned int flags ) {
	if ( flags == 0 ) {
		return NULL;
	}

	PLGFrameBuffer *buffer = ( PLGFrameBuffer * ) pl_malloc( sizeof( PLGFrameBuffer ) );
	if ( !buffer ) {
		return NULL;
	}

	buffer->width = w;
	buffer->height = h;
	buffer->flags = flags;

	CallGfxFunction( CreateFrameBuffer, buffer );

	return buffer;
}

void PlgDestroyFrameBuffer( PLGFrameBuffer *buffer ) {
	if ( !buffer ) {
		return;
	}

	CallGfxFunction( DeleteFrameBuffer, buffer );

	pl_free( buffer );
}

PLGTexture *PlgGetFrameBufferTextureAttachment( PLGFrameBuffer *buffer, unsigned int component, PLGTextureFilter filter ) {
	CallReturningGfxFunction( GetFrameBufferTextureAttachment, NULL, buffer, component, filter );
}

void PlgGetFrameBufferResolution( const PLGFrameBuffer *buffer, unsigned int *width, unsigned int *height ) {
	*width = buffer->width;
	*height = buffer->height;
}

void PlgBindFrameBuffer( PLGFrameBuffer *buffer, PLGFrameBufferObjectTarget target_binding ) {
	//NOTE: NULL is valid for *buffer, to bind the SDL window default backbuffer
	CallGfxFunction( BindFrameBuffer, buffer, target_binding )
}

void PlgBlitFrameBuffers( PLGFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLGFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, bool linear ) {
	//NOTE: NULL is valid for *srcBuffer/*dstBuffer, to bind the SDL window default backbuffer
	//      SRC and DST can be the same buffer, in order to quickly copy a subregion of the buffer to a new location
	CallGfxFunction( BlitFrameBuffers, src_buffer, src_w, src_h, dst_buffer, dst_w, dst_h, linear );
}

void PlgSetClearColour( PLColour rgba ) {
	if ( plCompareColour( rgba, gfx_state.current_clearcolour ) ) {
		return;
	}

	CallGfxFunction( SetClearColour, rgba );

	gfx_state.current_clearcolour = rgba;
}

void PlgClearBuffers( unsigned int buffers ) {
	CallGfxFunction( ClearBuffers, buffers );
}

/*===========================
	CAPABILITIES
===========================*/

bool PlgIsGraphicsStateEnabled( PLGDrawState state ) {
	return gfx_state.current_capabilities[ state ];
}

void PlgEnableGraphicsState( PLGDrawState state ) {
	if ( PlgIsGraphicsStateEnabled( state ) ) {
		return;
	}

	CallGfxFunction( EnableState, state );

	gfx_state.current_capabilities[ state ] = false;
}

void PlgDisableGraphicsState( PLGDrawState state ) {
	if ( !PlgIsGraphicsStateEnabled( state ) ) {
		return;
	}

	CallGfxFunction( DisableState, state );

	gfx_state.current_capabilities[ state ] = true;
}

/*===========================
	DRAW
===========================*/

void PlgSetBlendMode( PLGBlend a, PLGBlend b ) {
	CallGfxFunction( SetBlendMode, a, b );
}

void PlgSetCullMode( PLGCullMode mode ) {
	if ( mode == gfx_state.current_cullmode ) {
		return;
	}

	CallGfxFunction( SetCullMode, mode );

	gfx_state.current_cullmode = mode;
}

void PlgSetDepthBufferMode( unsigned int mode ) {
	CallGfxFunction( SetDepthBufferMode, mode );
}

void PlgSetDepthMask( bool enable ) {
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

void PlgDrawPixel( int x, int y, PLColour colour ) {
	CallGfxFunction( DrawPixel, x, y, colour );
}

/////////////////////////////////////////////////////////////////////////////////////

#define MAX_GRAPHICS_MODES 32

typedef struct GraphicsMode {
	const char *description;
	const PLGDriverInterface *interface;
} GraphicsMode;
static GraphicsMode graphicsModes[ MAX_GRAPHICS_MODES ];
static unsigned int numGraphicsModes = 0;

void PlgRegisterDriver( const char *description, const PLGDriverInterface *interface ) {
	if ( numGraphicsModes >= MAX_GRAPHICS_MODES ) {
		plReportBasicError( PL_RESULT_MEMORY_EOA );
		return;
	}

	graphicsModes[ numGraphicsModes ].description = description;
	graphicsModes[ numGraphicsModes ].interface = interface;
	numGraphicsModes++;
}

/**
 * Query what available graphics modes there are so that the
 * host can either choose their preference or attempt each.
 */
const char **PlgGetAvailableDriverInterfaces( unsigned int *numModes ) {
	static unsigned int cachedModes = 0;
	static const char *descriptors[ MAX_GRAPHICS_MODES ];
	if ( cachedModes != numGraphicsModes ) {
		for ( unsigned int i = 0; i < numGraphicsModes; ++i ) {
			descriptors[ i ] = graphicsModes[ i ].description;
		}
	}
	*numModes = numGraphicsModes;
	cachedModes = *numModes;

	return descriptors;
}

void PlgSetDriver( const char *mode ) {
	GfxLog( "Initializing graphics abstraction layer...\n" );

	const PLGDriverInterface *interface = NULL;
	for ( unsigned int i = 0; i < numGraphicsModes; ++i ) {
		if ( pl_strcasecmp( mode, graphicsModes[ i ].description ) != 0 ) {
			continue;
		}

		interface = graphicsModes[ i ].interface;
		break;
	}

	if ( interface == NULL ) {
		plReportErrorF( PL_RESULT_GRAPHICSINIT, "invalid graphics interface \"%s\" selected", mode );
		return;
	}

	if ( gfx_state.interface != NULL && interface == gfx_state.interface ) {
		plReportErrorF( PL_RESULT_GRAPHICSINIT, "chosen interface \"%s\" is already active", mode );
		return;
	}

	gfx_state.interface = interface;

	CallGfxFunction( Initialize );

	_InitTextures();
}

/*===========================
	STENCIL OPERATIONS
===========================*/

void PlgStencilFunction( PLGStencilTestFunction function, int reference, unsigned int mask ) {
	CallGfxFunction( StencilFunction, function, reference, mask );
}
