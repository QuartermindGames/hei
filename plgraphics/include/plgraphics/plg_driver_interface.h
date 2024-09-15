// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_plugin_interface.h>
#include <plgraphics/plg.h>

PL_EXTERN_C

/**
 * For a little further clarity, technically
 * speaking 'drivers' in this context are handled
 * exactly the same as plugins for the most part
 * but are setup through the plgraphics module
 * instead - and are exclusively assumed to support
 * either audio/video.
 */

#define PLG_INTERFACE_VERSION_MAJOR 7
#define PLG_INTERFACE_VERSION_MINOR 0
#define PLG_INTERFACE_VERSION \
	( uint16_t[ 2 ] ) { PLG_INTERFACE_VERSION_MAJOR, PLG_INTERFACE_VERSION_MINOR }

typedef struct PLGDriverDescription {
	uint16_t coreInterfaceVersion[ 2 ];
	uint16_t graphicsInterfaceVersion[ 2 ];

	const char *identifier;          /* 'opengl3' */
	unsigned int driverVersion[ 3 ]; /* major, minor and patch */
	const char *description;         /* 'OpenGL 3.3 Graphics Driver' */
} PLGDriverDescription;

typedef struct PLGDriverExportTable {
	const PLPluginExportTable *core;

	/* plg_texture */
	PLGTexture *( *CreateTexture )( void );
	void ( *DestroyTexture )( PLGTexture *texture );
	unsigned int ( *GetMaxTextureSize )( void );
	unsigned int ( *GetMaxTextureUnits )( void );
	unsigned int ( *GetMaxTextureAnistropy )( void );
	void ( *SetTextureAnisotropy )( PLGTexture *texture, unsigned int amount );
	void ( *SetTexture )( PLGTexture *texture, unsigned int tmu );
	void ( *SetTextureUnit )( unsigned int tmu );
	void ( *SetTextureEnvironmentMode )( PLGTextureEnvironmentMode mode );
	void ( *SetTextureFlags )( PLGTexture *texture, unsigned int flags );

	const char *( *GetShaderCacheLocation )( void );
} PLGDriverExportTable;

typedef struct PLGDriverImportTable {
	PLFunctionResult ( *Initialize )( void );
	void ( *Shutdown )( void );

	// Debug
	void ( *InsertDebugMarker )( const char *msg );
	void ( *PushDebugGroupMarker )( const char *msg );
	void ( *PopDebugGroupMarker )( void );

	/* hw information */

	bool ( *SupportsHWShaders )( void );

	void ( *GetMaxTextureUnits )( unsigned int *num_units );
	void ( *GetMaxTextureSize )( unsigned int *s );

	/******************************************/

	/* generic state management */
	void ( *EnableState )( PLGDrawState state );
	void ( *DisableState )( PLGDrawState state );

	void ( *SetBlendMode )( PLGBlend a, PLGBlend b );
	void ( *SetCullMode )( PLGCullMode mode );

	void ( *SetClearColour )( const PLColourF32 *rgba );
	void ( *ClearBuffers )( unsigned int buffers );

	void ( *SetDepthBufferMode )( unsigned int mode );

	void ( *DepthMask )( bool enable );
	void ( *ColourMask )( bool r, bool g, bool b, bool a );

	// Mesh
#pragma message "TODO: this should all be removed!!"
	void ( *CreateMesh )( PLGMesh *mesh );
	void ( *UploadMesh )( PLGMesh *mesh, PLGShaderProgram *program ); /* todo: deprecate? */
	void ( *DrawMesh )( PLGMesh *mesh, PLGShaderProgram *program );
	void ( *DrawInstancedMesh )( PLGMesh *mesh, PLGShaderProgram *program, const PLMatrix4 *transforms, unsigned int instanceCount );
	void ( *DeleteMesh )( PLGMesh *mesh );

	//TODO: new API...replaces the above
	void ( *Draw )( const PLGVertexLayout *layout, PLGShaderProgram *program, unsigned int baseVertexIndex, unsigned int numVertices );
	void ( *DrawInstanced )( const PLGVertexLayout *layout, PLGShaderProgram *program, unsigned int baseVertexIndex, unsigned int numVertices, const PLMatrix4 *transforms, unsigned int numInstances );
	void ( *DrawPixel )( int x, int y, const PLColourF32 *colour );

	// Framebuffer
	bool ( *CreateFrameBuffer )( PLGFrameBuffer *buffer );
	void ( *DeleteFrameBuffer )( PLGFrameBuffer *buffer );
	void ( *BindFrameBuffer )( PLGFrameBuffer *buffer, PLGFrameBufferObjectTarget targetBinding );
	PLGTexture *( *GetFrameBufferTextureAttachment )( PLGFrameBuffer *buffer, unsigned int component, PLGTextureFilter filter );
	void ( *BlitFrameBuffers )( PLGFrameBuffer *srcBuffer,
	                            unsigned int srcW,
	                            unsigned int srcH,
	                            PLGFrameBuffer *dstBuffer,
	                            unsigned int dstW,
	                            unsigned int dstH,
	                            bool linear );
	void ( *SetFrameBufferSize )( PLGFrameBuffer *frameBuffer, unsigned int width, unsigned int height );

	// Texture
	void ( *CreateTexture )( PLGTexture *texture );
	void ( *DeleteTexture )( PLGTexture *texture );
	void ( *BindTexture )( const PLGTexture *texture );
	void ( *UploadTexture )( PLGTexture *texture, const PLImage *upload );
	void ( *SwizzleTexture )( PLGTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	void ( *SetTextureAnisotropy )( PLGTexture *texture, uint32_t value );
	void ( *SetTextureFilter )( PLGTexture *texture, PLGTextureFilter filter );
	void ( *ActiveTexture )( unsigned int target );

	/* viewport */
	void ( *ClipViewport )( int x, int y, int width, int height );
	void ( *SetViewport )( int x, int y, int width, int height );

	///////////////////////////////////////////

	// Shaders
	/* todo: simplify this interface */
	void ( *CreateShaderProgram )( PLGShaderProgram *program );
	void ( *DestroyShaderProgram )( PLGShaderProgram *program );
	void ( *AttachShaderStage )( PLGShaderProgram *program, PLGShaderStage *stage );
	void ( *DetachShaderStage )( PLGShaderProgram *program, PLGShaderStage *stage );
	void ( *LinkShaderProgram )( PLGShaderProgram *program );
	void ( *SetShaderProgram )( PLGShaderProgram *program );
	void ( *CreateShaderStage )( PLGShaderStage *stage );
	void ( *DestroyShaderStage )( PLGShaderStage *stage );
	void ( *CompileShaderStage )( PLGShaderStage *stage, const char *buf, size_t length, const char *directory );
	void ( *SetShaderUniformValue )( PLGShaderProgram *program, int slot, const void *value, bool transpose );

	// Stencil operations
	void ( *StencilBufferFunction )( PLGCompareFunction compareFunction, int reference, unsigned int mask );
	void ( *DepthBufferFunction )( PLGCompareFunction compareFunction );

	void ( *StencilOp )( PLGStencilFace face, PLGStencilOp stencilFailOp, PLGStencilOp depthFailOp, PLGStencilOp depthPassOp );

	// v3.1
	void *( *ReadFrameBufferRegion )( PLGFrameBuffer *frameBuffer, uint32_t x, uint32_t y, uint32_t w, uint32_t h, size_t dstSize, void *dstBuf );

	// v3.2
	void ( *SetTextureWrapMode )( PLGTexture *texture, PLGTextureWrapMode wrapMode );
} PLGDriverImportTable;

#if !defined( PL_COMPILE_PLUGIN )
#	define PLG_DRIVER_QUERY_FUNCTION "QueryGraphicsDriver"
typedef const PLGDriverDescription *( *PLGDriverQueryFunction )( void );
#	define PLG_DRIVER_INIT_FUNCTION "InitializeGraphicsDriver"
typedef const PLGDriverImportTable *( *PLGDriverInitializationFunction )( const PLGDriverExportTable *exportTable );

PL_EXTERN bool PlgRegisterDriver( const char *path );
PL_EXTERN unsigned int PlgScanForDrivers( const char *path );

PL_EXTERN const PLGDriverDescription **PlgGetAvailableDriverInterfaces( unsigned int *numModes );
PL_EXTERN PLFunctionResult PlgSetDriver( const char *mode );
#endif

PL_EXTERN_C_END
