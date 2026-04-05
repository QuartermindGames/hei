/**
 * Hei Platform Library
 * Copyright (C) 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plgraphics/plg_texture.h>

PL_EXTERN_C

typedef enum PLGFrameBufferObjectTarget {
	PLG_FRAMEBUFFER_DEFAULT,
	PLG_FRAMEBUFFER_DRAW,
	PLG_FRAMEBUFFER_READ,

	PLG_MAX_FRAMEBUFFER_TARGETS
} PLGFrameBufferObjectTarget;

typedef enum PLGFrameBufferRenderFlags {
	QM_OS_BIT_FLAG( PLG_BUFFER_COLOUR, 0 ),
	QM_OS_BIT_FLAG( PLG_BUFFER_DEPTH, 1 ),
	QM_OS_BIT_FLAG( PLG_BUFFER_STENCIL, 2 ),
} PLGFrameBufferRenderFlags;

enum {
	PLG_RENDERBUFFER_COLOUR,
	PLG_RENDERBUFFER_DEPTH,
	PLG_RENDERBUFFER_STENCIL,

	PLG_MAX_RENDERBUFFER_TYPES
};

typedef struct QmGfxFramebuffer
{
	unsigned int              fbo;
	unsigned int              renderBuffers[ PLG_MAX_RENDERBUFFER_TYPES ];
	unsigned int              width;
	unsigned int              height;
	unsigned int              numSamples;
	PLGFrameBufferRenderFlags flags;
} QmGfxFramebuffer;

enum {
	PLG_DEPTHBUFFER_DISABLE,
	PLG_DEPTHBUFFER_ENABLE,
};

#if !defined( PL_COMPILE_PLUGIN )

QmGfxFramebuffer *qm_gfx_framebuffer_create( unsigned int w, unsigned int h, unsigned int flags, unsigned int numSamples );
void            qm_gfx_framebuffer_destroy( QmGfxFramebuffer *buffer );
void            qm_gfx_framebuffer_bind( QmGfxFramebuffer *buffer, PLGFrameBufferObjectTarget target_binding );
void            qm_gfx_framebuffer_blit( QmGfxFramebuffer *src_buffer, unsigned int src_w, unsigned int src_h, QmGfxFramebuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, unsigned int mask, bool linear );

QmGfxTexture *PlgGetFrameBufferTextureAttachment( QmGfxFramebuffer *buffer, unsigned int component, QmGfxTextureFilter filter, QmGfxTextureWrapMode wrap );
void        PlgGetFrameBufferResolution( const QmGfxFramebuffer *buffer, unsigned int *width, unsigned int *height );

void  PlgSetClearColour( QmMathColour4ub rgba );
void  PlgClearBuffers( unsigned int buffers );
bool  qm_gfx_framebuffer_set_resolution( QmGfxFramebuffer *frameBuffer, unsigned int width, unsigned int height );
void *qm_gfx_framebuffer_read_region( QmGfxFramebuffer *frameBuffer, uint32_t x, uint32_t y, uint32_t w, uint32_t h, size_t dstSize, void *dstBuf );

void PlgColourMask( bool r, bool g, bool b, bool a );
void PlgStencilMask( unsigned int mask );

#endif

PL_EXTERN_C_END
