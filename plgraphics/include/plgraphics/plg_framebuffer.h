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
	PL_BITFLAG( PLG_BUFFER_COLOUR, 0 ),
	PL_BITFLAG( PLG_BUFFER_DEPTH, 1 ),
	PL_BITFLAG( PLG_BUFFER_STENCIL, 2 ),
} PLGFrameBufferRenderFlags;

enum {
	PLG_RENDERBUFFER_COLOUR,
	PLG_RENDERBUFFER_DEPTH,
	PLG_RENDERBUFFER_STENCIL,

	PLG_MAX_RENDERBUFFER_TYPES
};

/* !!!
 * be careful changing this as it could break
 * the plugin interface
 * !!! */
typedef struct PLGFrameBuffer {
	unsigned int fbo;
	unsigned int renderBuffers[ PLG_MAX_RENDERBUFFER_TYPES ];
	unsigned int width;
	unsigned int height;
	PLGFrameBufferRenderFlags flags;
} PLGFrameBuffer;

enum {
	PLG_DEPTHBUFFER_DISABLE,
	PLG_DEPTHBUFFER_ENABLE,
};

#if !defined( PL_COMPILE_PLUGIN )

PLGFrameBuffer *PlgCreateFrameBuffer( unsigned int w, unsigned int h, unsigned int flags );
void PlgDestroyFrameBuffer( PLGFrameBuffer *buffer );
void PlgBindFrameBuffer( PLGFrameBuffer *buffer, PLGFrameBufferObjectTarget target_binding );
void PlgBlitFrameBuffers( PLGFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLGFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, bool linear );

PLGTexture *PlgGetFrameBufferTextureAttachment( PLGFrameBuffer *buffer, unsigned int component, PLGTextureFilter filter );
void PlgGetFrameBufferResolution( const PLGFrameBuffer *buffer, unsigned int *width, unsigned int *height );

void PlgSetClearColour( PLColour rgba );
void PlgClearBuffers( unsigned int buffers );
bool PlgSetFrameBufferSize( PLGFrameBuffer *frameBuffer, unsigned int width, unsigned int height );
void *PlgReadFrameBufferRegion( PLGFrameBuffer *frameBuffer, uint32_t x, uint32_t y, uint32_t w, uint32_t h, size_t dstSize, void *dstBuf );
void PlgColourMask( bool r, bool g, bool b, bool a );

#endif

PL_EXTERN_C_END
