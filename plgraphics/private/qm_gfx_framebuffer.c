// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>
// Purpose: Framebuffer implementation.
// Author:  Mark E. Sowden

#include "qmos/public/qm_os_memory.h"

#include "plg_private.h"

static bool create_framebuffer( QmGfxFramebuffer *frameBuffer )
{
	CallReturningGfxFunction( CreateFrameBuffer, false, frameBuffer );
}

QmGfxFramebuffer *qm_gfx_framebuffer_create( unsigned int w, unsigned int h, unsigned int flags, unsigned int numSamples )
{
	if ( flags == 0 )
	{
		return nullptr;
	}

	QmGfxFramebuffer *buffer = QM_OS_MEMORY_NEW( QmGfxFramebuffer );
	if ( buffer == nullptr )
	{
		return nullptr;
	}

	buffer->width      = w;
	buffer->height     = h;
	buffer->flags      = flags;
	buffer->numSamples = numSamples;

	if ( !create_framebuffer( buffer ) )
	{
		qm_gfx_framebuffer_destroy( buffer );
		return nullptr;
	}

	return buffer;
}

void qm_gfx_framebuffer_destroy( QmGfxFramebuffer *buffer )
{
	if ( !buffer )
	{
		return;
	}

	CallGfxFunction( DeleteFrameBuffer, buffer );

	qm_os_memory_free( buffer );
}

QmGfxTexture *PlgGetFrameBufferTextureAttachment( QmGfxFramebuffer *buffer, unsigned int component, QmGfxTextureFilter filter, QmGfxTextureWrapMode wrap )
{
	CallReturningGfxFunction( GetFrameBufferTextureAttachment, NULL, buffer, component, filter, wrap );
}

// should really be 'GetFrameBufferSize', urgh...
void PlgGetFrameBufferResolution( const QmGfxFramebuffer *buffer, unsigned int *width, unsigned int *height )
{
	*width  = buffer->width;
	*height = buffer->height;
}

void qm_gfx_framebuffer_bind( QmGfxFramebuffer *buffer, PLGFrameBufferObjectTarget target_binding )
{
	//NOTE: NULL is valid for *buffer, to bind the SDL window default backbuffer
	CallGfxFunction( BindFrameBuffer, buffer, target_binding );
}

void qm_gfx_framebuffer_blit( QmGfxFramebuffer *src_buffer, unsigned int src_w, unsigned int src_h, QmGfxFramebuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, unsigned int mask, bool linear )
{
	//NOTE: NULL is valid for *srcBuffer/*dstBuffer, to bind the SDL window default backbuffer
	//      SRC and DST can be the same buffer, in order to quickly copy a subregion of the buffer to a new location
	CallGfxFunction( BlitFrameBuffers, src_buffer, src_w, src_h, dst_buffer, dst_w, dst_h, mask, linear );
}

void PlgSetClearColour( QmMathColour4ub rgba )
{
	if ( qm_math_colour4ub_compare( rgba, gfx_state.current_clearcolour ) )
	{
		return;
	}

	CallGfxFunction( SetClearColour, rgba );

	gfx_state.current_clearcolour = rgba;
}

void PlgClearBuffers( unsigned int buffers )
{
	CallGfxFunction( ClearBuffers, buffers );
}

/**
 * Resizes the given framebuffer to the specified size.
 */
bool qm_gfx_framebuffer_set_resolution( QmGfxFramebuffer *frameBuffer, unsigned int width, unsigned int height )
{
	if ( width == 0 )
	{
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "invalid width" );
		return false;
	}

	if ( height == 0 )
	{
		PlReportErrorF( PL_RESULT_INVALID_PARM3, "invalid height" );
		return false;
	}

	if ( frameBuffer->width == width && frameBuffer->height == height )
	{
		return true;
	}

	CallGfxFunction( SetFrameBufferSize, frameBuffer, width, height );
	return true;
}

void *qm_gfx_framebuffer_read_region( QmGfxFramebuffer *frameBuffer, uint32_t x, uint32_t y, uint32_t w, uint32_t h, size_t dstSize, void *dstBuf )
{
	CallReturningGfxFunction( ReadFrameBufferRegion, NULL, frameBuffer, x, y, w, h, dstSize, dstBuf );
}
