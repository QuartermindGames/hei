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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PLGFrameBufferObjectTarget {
	PLG_FRAMEBUFFER_DEFAULT,
	PLG_FRAMEBUFFER_DRAW,
	PLG_FRAMEBUFFER_READ
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

PL_EXTERN PLGFrameBuffer *PlgCreateFrameBuffer( unsigned int w, unsigned int h, unsigned int flags );
PL_EXTERN void PlgDestroyFrameBuffer( PLGFrameBuffer *buffer );
PL_EXTERN void PlgBindFrameBuffer( PLGFrameBuffer *buffer, PLGFrameBufferObjectTarget target_binding );
PL_EXTERN void PlgBlitFrameBuffers( PLGFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLGFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, bool linear );

PL_EXTERN PLGTexture *PlgGetFrameBufferTextureAttachment( PLGFrameBuffer *buffer, unsigned int component, PLGTextureFilter filter );
PL_EXTERN void PlgGetFrameBufferResolution( const PLGFrameBuffer *buffer, unsigned int *width, unsigned int *height );

PL_EXTERN void PlgSetClearColour( PLColour rgba );
PL_EXTERN void PlgClearBuffers( unsigned int buffers );

#ifdef __cplusplus
};
#endif
