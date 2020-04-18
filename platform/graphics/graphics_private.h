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

#pragma once

#include "platform_private.h"

#include <PL/graphics.h>
#include <PL/platform_graphics_camera.h>

#ifdef _DEBUG
#   define GfxLog( ... ) plLogMessage(LOG_LEVEL_GRAPHICS, __VA_ARGS__)
#else
#   define GfxLog(...)
#endif

typedef struct GfxState {
	PLCullMode current_cullmode;

	PLColour current_clearcolour;
	PLColour current_colour;        // Current global colour.

	bool current_capabilities[PL_GFX_MAX_STATES];    // Enabled capabilities.
	unsigned int current_textureunit;

	// Textures

	PLTextureMappingUnit *tmu;
	PLTexture **textures;

	unsigned int num_textures;
	unsigned int max_textures;

	// Shader states

	PLShaderProgram *current_program;

	// Hardware / Driver information

	const char *hw_vendor;
	const char *hw_renderer;
	const char *hw_version;
	const char *hw_extensions;

	unsigned int hw_maxtexturesize;
	unsigned int hw_maxtextureunits;
	unsigned int hw_maxtextureanistropy;

	// Lighting

	unsigned int num_lights;

	// Cameras

	PLCamera **cameras;

	unsigned int num_cameras;
	unsigned int max_cameras;

	PLMatrix4 projection_matrix;
	PLMatrix4 view_matrix;

	////////////////////////////////////////

	PLViewport *current_viewport;

	bool mode_debug;
} GfxState;

typedef struct GfxLayer {
	PLGfxMode mode;    // Current gfx interface

	// Debug
	void (*InsertDebugMarker)( const char *msg );
	void (*PushDebugGroupMarker)( const char *msg );
	void (*PopDebugGroupMarker)( void );

	/* hw information */

	bool (*HWSupportsMultitexture)( void );
	bool (*HWSupportsShaders)( void );

	void (*GetMaxTextureUnits)( unsigned int *num_units );
	void (*GetMaxTextureSize)( unsigned int *s );

	/******************************************/

	/* generic state management */
	void (*EnableState)( PLGraphicsState state );
	void (*DisableState)( PLGraphicsState state );

	void (*SetBlendMode)( PLBlend a, PLBlend b );
	void (*SetCullMode)( PLCullMode mode );

	void (*SetClearColour)( PLColour rgba );
	void (*ClearBuffers)( unsigned int buffers );

	void (*DrawPixel)( int x, int y, PLColour colour );

	void (*SetDepthBufferMode)( unsigned int mode );
	void (*SetDepthMask)( bool enable );

	// Mesh
	void (*CreateMesh)( PLMesh *mesh );
	void (*UploadMesh)( PLMesh *mesh );
	void (*DrawMesh)( PLMesh *mesh );
	void (*DeleteMesh)( PLMesh *mesh );

	// Framebuffer
	void (*CreateFrameBuffer)( PLFrameBuffer *buffer );
	void (*DeleteFrameBuffer)( PLFrameBuffer *buffer );
	void (*BindFrameBuffer)( PLFrameBuffer *buffer, PLFBOTarget targetBinding );
	PLTexture *(*GetFrameBufferTextureAttachment)( PLFrameBuffer *buffer );
	void (*BlitFrameBuffers)( PLFrameBuffer *srcBuffer,
							  unsigned int srcW,
							  unsigned int srcH,
							  PLFrameBuffer *dstBuffer,
							  unsigned int dstW,
							  unsigned int dstH,
							  bool linear );

	// Texture
	void (*CreateTexture)( PLTexture *texture );
	void (*DeleteTexture)( PLTexture *texture );
	void (*BindTexture)( const PLTexture *texture );
	void (*UploadTexture)( PLTexture *texture, const PLImage *upload );
	void (*SwizzleTexture)( PLTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	void (*SetTextureAnisotropy)( PLTexture *texture, uint32_t value );
	void (*ActiveTexture)( unsigned int target );

	// Camera
	void (*CreateCamera)( PLCamera *camera );
	void (*DestroyCamera)( PLCamera *camera );
	void (*SetupCamera)( PLCamera *camera );
	///////////////////////////////////////////

	// Shaders
	void (*CreateShaderProgram)( PLShaderProgram *program );
	void (*DestroyShaderProgram)( PLShaderProgram *program );
	void (*AttachShaderStage)( PLShaderProgram *program, PLShaderStage *stage );
	void (*DetachShaderStage)( PLShaderProgram *program, PLShaderStage *stage );
	void (*LinkShaderProgram)( PLShaderProgram *program );
	void (*SetShaderProgram)( PLShaderProgram *program );
	void (*CreateShaderStage)( PLShaderStage *stage );
	void (*DestroyShaderStage)( PLShaderStage *stage );
	void (*CompileShaderStage)( PLShaderStage *stage, const char *buf, size_t length );

	//Shader uniforms
	void (*SetShaderUniformMatrix4)( PLShaderProgram *program, int slot, PLMatrix4 value, bool transpose );

} GfxLayer;

#if defined(PL_USE_GRAPHICS)
#   define CallGfxFunction( FUNCTION, ... ) \
    if(gfx_layer.FUNCTION != NULL) { \
        gfx_layer.FUNCTION(__VA_ARGS__); \
    } else { \
        GfxLog("Unbound layer function %s was called\n", #FUNCTION); \
    }
#else
#   define CallGfxFunction(FUNCTION, ...)
#endif

///////////////////////////////////////////////////////

#if 1
#   define UseBufferScaling( a ) \
    ((a)->viewport.r_w != 0 && (a)->viewport.r_h != 0) && \
    ((a)->viewport.r_w != (a)->viewport.w && (a)->viewport.r_h != (a)->viewport.h)
#else /* for debugging purposes */
#   define UseBufferScaling(a) 0
#endif

///////////////////////////////////////////////////////

typedef struct PLFrameBuffer {
	unsigned int fbo;
	unsigned int rbo_colour;
	unsigned int rbo_depth;
	unsigned int width;
	unsigned int height;
	PLFrameBufferRenderFlags flags;
} PLFrameBuffer;

#define PL_POLYGON_MAX_SIDES 32

typedef struct PLPolygon {
	PLVertex vertices[ PL_POLYGON_MAX_SIDES ];
	unsigned int numVertices;
} PLPolygon;

PL_EXTERN_C

PL_EXTERN GfxState gfx_state;
PL_EXTERN GfxLayer gfx_layer;

void _plBindTexture( const PLTexture *texture );

#if defined(PL_SUPPORT_OPENGL)
void plInitOpenGL( void );
void plShutdownOpenGL( void );
#endif

void plInitSoftwareGraphicsLayer( void );
void plShutdownSoftwareGraphicsLayer( void );

PL_EXTERN_C_END
