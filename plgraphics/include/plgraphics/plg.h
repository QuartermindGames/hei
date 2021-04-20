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

#include <PL/platform.h>
#include <PL/platform_math.h>

#include <plgraphics/plg_mesh.h>
#include <plgraphics/plg_texture.h>

typedef struct PLGDriverInterface PLGDriverInterface;

typedef enum PLGCullMode {
	PLG_CULL_NONE, /* disables backface culling */
	PLG_CULL_POSTIVE,
	PLG_CULL_NEGATIVE
} PLGCullMode;

// Blending Modes
typedef enum PLGBlend {
	PLG_BLEND_NONE,// disables blending

	PLG_BLEND_ZERO,
	PLG_BLEND_ONE,
	PLG_BLEND_SRC_COLOR,
	PLG_BLEND_ONE_MINUS_SRC_COLOR,
	PLG_BLEND_SRC_ALPHA,
	PLG_BLEND_ONE_MINUS_SRC_ALPHA,
	PLG_BLEND_DST_ALPHA,
	PLG_BLEND_ONE_MINUS_DST_ALPHA,
	PLG_BLEND_DST_COLOR,
	PLG_BLEND_ONE_MINUS_DST_COLOR,
	PLG_BLEND_SRC_ALPHA_SATURATE,

	PLG_MAX_BLEND_MODES
} PLGBlend;
#define PLG_BLEND_DISABLE PLG_BLEND_NONE, PLG_BLEND_NONE
#define PLG_BLEND_ADDITIVE PLG_BLEND_SRC_ALPHA, PLG_BLEND_ONE
#define PLG_BLEND_DEFAULT PLG_BLEND_SRC_ALPHA, PLG_BLEND_ONE_MINUS_SRC_ALPHA

//-----------------
// Capabilities

/* Replace these with generic functions, rather than relying on just flags?
 * plEnableFog(bool active)
 * plEnableAlphaTest(bool active)
 * plEnableBlend(bool active)
 */

typedef enum PLGDrawState {
	PLG_GFX_STATE_FOG,            // Fog.
	PLG_GFX_STATE_ALPHATEST,      // Alpha-testing.
	PLG_GFX_STATE_BLEND,          // Blending.
	PLG_GFX_STATE_DEPTHTEST,      // Depth-testing.
	PLG_GFX_STATE_STENCILTEST,    // Stencil-testing.
	PLG_GFX_STATE_MULTISAMPLE,    // Multisampling.
	PLG_GFX_STATE_SCISSORTEST,    // Scissor test for buffer clear.
	PLG_GFX_STATE_ALPHATOCOVERAGE,// Alpha to Coverage
	PLG_GFX_STATE_DEPTH_CLAMP,

	PLG_GFX_MAX_STATES
} PLGDrawState;

//-----------------
// Framebuffers
// todo: move all of this into pl_graphics_framebuffer.h

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

typedef enum PLGStencilTestFunction {
	PLG_STENCIL_TEST_ALWAYS,
	PLG_STENCIL_TEST_NEVER,
	PLG_STENCIL_TEST_LESS,
	PLG_STENCIL_TEST_LEQUAL,

	PLG_MAX_STENCIL_TEST_OPERATIONS
} PLGStencilTestFunction;

typedef enum PLGStencilFace {
	PLG_STENCIL_FACE_FRONT,
	PLG_STENCIL_FACE_BACK,
} PLGStencilFace;

//-----------------
// Lighting

typedef enum PLGLightType {
	PLG_LIGHT_TYPE_SPOT,// Spotlight
	PLG_LIGHT_TYPE_OMNI // Omni-directional
} PLGLightType;

typedef struct PLGLight {
	PLVector3 position;
	PLVector3 angles;

	PLColour colour;

	PLGLightType type;
} PLGLight;

//-----------------
// Shaders
// todo: move all of this into pl_graphics_shader.h

typedef int PLGShaderAttribute;

typedef enum PLGShaderStageType {
	PLG_SHADER_TYPE_VERTEX,
	PLG_SHADER_TYPE_FRAGMENT,
	PLG_SHADER_TYPE_GEOMETRY,
	PLG_SHADER_TYPE_COMPUTE,

	PLG_MAX_SHADER_TYPES
} PLGShaderStageType;

typedef enum PLGShaderUniformType {
	PLG_INVALID_UNIFORM,

	PLG_UNIFORM_FLOAT,
	PLG_UNIFORM_INT,
	PLG_UNIFORM_UINT,
	PLG_UNIFORM_BOOL,
	PLG_UNIFORM_DOUBLE,

	/* textures */
	PLG_UNIFORM_SAMPLER1D,
	PLG_UNIFORM_SAMPLER2D,
	PLG_UNIFORM_SAMPLER3D,
	PLG_UNIFORM_SAMPLERCUBE,
	PLG_UNIFORM_SAMPLER1DSHADOW,
	PLG_UNIFORM_SAMPLER2DSHADOW,

	/* vectors */
	PLG_UNIFORM_VEC2,
	PLG_UNIFORM_VEC3,
	PLG_UNIFORM_VEC4,

	/* matrices */
	PLG_UNIFORM_MAT3,
	PLG_UNIFORM_MAT4,

	PLG_MAX_UNIFORM_TYPES
} PLGShaderUniformType;

typedef struct PLGShaderProgram PLGShaderProgram;

typedef struct PLGShaderStage {
	PLGShaderStageType type;

	/* software implementation of the shader stage */
	void ( *SWFallback )( PLGShaderProgram *program, PLGShaderStageType type );

	PLGShaderProgram *program;

	struct {
		unsigned int id;
	} internal;
} PLGShaderStage;

typedef struct PLGShaderProgram {
	struct {
		char *name;
		unsigned int slot;
		PLGShaderUniformType type;

		union {
			int defaultInt;
			unsigned int defaultUInt;
			bool defaultBool;
			double defaultDouble;
			float defaultFloat;

			PLVector2 defaultVec2;
			PLVector3 defaultVec3;
			PLVector4 defaultVec4;

			PLMatrix3 defaultMat3;
			PLMatrix4 defaultMat4;
		};
	} * uniforms;
	unsigned int num_uniforms;

	struct {
		char name[ 32 ];

		unsigned int slot;
	} * attributes;
	unsigned int num_attributes;

	PLGShaderStage *stages[ PLG_MAX_SHADER_TYPES ];
	unsigned int num_stages;

	bool is_linked;

	struct {
		unsigned int id;

		PLGShaderAttribute v_position;
		PLGShaderAttribute v_normal;
		PLGShaderAttribute v_uv;
		PLGShaderAttribute v_colour;
		PLGShaderAttribute v_tangent, v_bitangent;
	} internal;
} PLGShaderProgram;

typedef struct PLGPolygon PLGPolygon;
typedef struct PLGVertex PLGVertex;
typedef struct PLGMesh PLGMesh;
typedef struct PLGCamera PLGCamera;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

//PL_EXTERN PLShaderStage *plCreateShaderStage(PLShaderStageType type);
PL_EXTERN PLGShaderStage *PlgParseShaderStage( PLGShaderStageType type, const char *buf, size_t length );
PL_EXTERN PLGShaderStage *PlgLoadShaderStage( const char *path, PLGShaderStageType type );
PL_EXTERN void PlgCompileShaderStage( PLGShaderStage *stage, const char *buf, size_t length );

PL_EXTERN void PlgAttachShaderStage( PLGShaderProgram *program, PLGShaderStage *stage );
PL_EXTERN bool PlgRegisterShaderStageFromMemory( PLGShaderProgram *program, const char *buffer, size_t length,
                                                 PLGShaderStageType type );
PL_EXTERN bool PlgRegisterShaderStageFromDisk( PLGShaderProgram *program, const char *path, PLGShaderStageType type );
PL_EXTERN bool PlgLinkShaderProgram( PLGShaderProgram *program );

PL_EXTERN int PlgGetShaderUniformSlot( PLGShaderProgram *program, const char *name );
PL_EXTERN PLGShaderUniformType PlgGetShaderUniformType( const PLGShaderProgram *program, int slot );

PL_EXTERN void PlgSetShaderUniformDefaultValueByIndex( PLGShaderProgram *program, int slot, const void *defaultValue );
PL_EXTERN void PlgSetShaderUniformDefaultValue( PLGShaderProgram *program, const char *name, const void *defaultValue );
PL_EXTERN void PlgSetShaderUniformToDefaultByIndex( PLGShaderProgram *program, int slot );
PL_EXTERN void PlgSetShaderUniformToDefault( PLGShaderProgram *program, const char *name );
PL_EXTERN void PlgSetShaderUniformsToDefault( PLGShaderProgram *program );

PL_EXTERN void PlgSetShaderUniformValueByIndex( PLGShaderProgram *program, int slot, const void *value, bool transpose );
PL_EXTERN void PlgSetShaderUniformValue( PLGShaderProgram *program, const char *name, const void *value, bool transpose );

#if !defined( PL_EXCLUDE_DEPRECATED_API )
PL_EXTERN PL_DEPRECATED( void PlgSetShaderUniformFloat( PLGShaderProgram *program, int slot, float value ) );
PL_EXTERN PL_DEPRECATED( void PlgSetShaderUniformInt( PLGShaderProgram *program, int slot, int value ) );
PL_EXTERN PL_DEPRECATED( void PlgSetShaderUniformVector4( PLGShaderProgram *program, int slot, PLVector4 value ) );
PL_EXTERN PL_DEPRECATED( void PlgSetShaderUniformVector3( PLGShaderProgram *program, int slot, PLVector3 value ) );
PL_EXTERN PL_DEPRECATED( void PlgSetShaderUniformMatrix4( PLGShaderProgram *program, int slot, PLMatrix4 value, bool transpose ) );
#endif

PL_EXTERN PLGShaderProgram *PlgCreateShaderProgram( void );
PL_EXTERN void PlgDestroyShaderProgram( PLGShaderProgram *program, bool free_stages );

PL_EXTERN PLGShaderProgram *PlgGetCurrentShaderProgram( void );

PL_EXTERN void PlgSetShaderProgram( PLGShaderProgram *program );
PL_EXTERN bool PlgIsShaderProgramEnabled( PLGShaderProgram *program );

// Debugging
PL_EXTERN void PlgInsertDebugMarker( const char *msg );
PL_EXTERN void PlgPushDebugGroupMarker( const char *msg );
PL_EXTERN void PlgPopDebugGroupMarker( void );

// Hardware Information
PL_EXTERN const char *PlgGetHWExtensions( void );
PL_EXTERN const char *PlgGetHWRenderer( void );
PL_EXTERN const char *PlgGetHWVendor( void );
PL_EXTERN const char *PlgGetHWVersion( void );

PL_EXTERN bool PlgSupportsHWShaders( void );

PL_EXTERN void PlgSetCullMode( PLGCullMode mode );
PL_EXTERN void PlgSetBlendMode( PLGBlend a, PLGBlend b );

PL_EXTERN void PlgSetDepthBufferMode( unsigned int mode );
PL_EXTERN void PlgSetDepthMask( bool enable );

PL_EXTERN void PlgRegisterDriver( const char *description, const PLGDriverInterface *interface );
PL_EXTERN const char **PlgGetAvailableDriverInterfaces( unsigned int *numModes );
PL_EXTERN void PlgSetDriver( const char *mode );

/* stencil operations */

PL_EXTERN void PlgStencilFunction( PLGStencilTestFunction function, int reference, unsigned int mask );

PL_EXTERN PLGFrameBuffer *PlgCreateFrameBuffer( unsigned int w, unsigned int h, unsigned int flags );
PL_EXTERN void PlgDestroyFrameBuffer( PLGFrameBuffer *buffer );
PL_EXTERN void PlgBindFrameBuffer( PLGFrameBuffer *buffer, PLGFrameBufferObjectTarget target_binding );
PL_EXTERN void PlgBlitFrameBuffers( PLGFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLGFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, bool linear );

PL_EXTERN PLGTexture *PlgGetFrameBufferTextureAttachment( PLGFrameBuffer *buffer, unsigned int component, PLGTextureFilter filter );
PL_EXTERN void PlgGetFrameBufferResolution( const PLGFrameBuffer *buffer, unsigned int *width, unsigned int *height );

PL_EXTERN void PlgSetClearColour( PLColour rgba );
PL_EXTERN void PlgClearBuffers( unsigned int buffers );

PL_EXTERN bool PlgIsGraphicsStateEnabled( PLGDrawState state );

PL_EXTERN void PlgEnableGraphicsState( PLGDrawState state );
PL_EXTERN void PlgDisableGraphicsState( PLGDrawState state );

#endif

#define PLG_INTERFACE_VERSION_MAJOR 1
#define PLG_INTERFACE_VERSION_MINOR 0

PL_EXTERN_C_END

typedef struct PLGDriverDescription {
	const char *renderer;
	const char *vendor;
	const char *version;
} PLGDriverDescription;

typedef struct PLGDriverInterface {
	uint16_t version[ 2 ];

	const PLGDriverDescription *( *Initialize )( void );
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

	void ( *SetClearColour )( PLColour rgba );
	void ( *ClearBuffers )( unsigned int buffers );

	void ( *DrawPixel )( int x, int y, PLColour colour );

	void ( *SetDepthBufferMode )( unsigned int mode );
	void ( *SetDepthMask )( bool enable );

	// Mesh
	void ( *CreateMesh )( PLGMesh *mesh );
	void ( *UploadMesh )( PLGMesh *mesh, PLGShaderProgram *program );
	void ( *DrawMesh )( PLGMesh *mesh, PLGShaderProgram *program );
	void ( *DrawInstancedMesh )( PLGMesh *mesh, PLGShaderProgram *program, const PLMatrix4 *transforms, unsigned int instanceCount );
	void ( *DeleteMesh )( PLGMesh *mesh );

	// Framebuffer
	void ( *CreateFrameBuffer )( PLGFrameBuffer *buffer );
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

	// Texture
	void ( *CreateTexture )( PLGTexture *texture );
	void ( *DeleteTexture )( PLGTexture *texture );
	void ( *BindTexture )( const PLGTexture *texture );
	void ( *UploadTexture )( PLGTexture *texture, const PLImage *upload );
	void ( *SwizzleTexture )( PLGTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	void ( *SetTextureAnisotropy )( PLGTexture *texture, uint32_t value );
	void ( *ActiveTexture )( unsigned int target );

	// Camera
	/* todo: simplify/remove this interface */
	void ( *CreateCamera )( PLGCamera *camera );
	void ( *DestroyCamera )( PLGCamera *camera );
	void ( *SetupCamera )( PLGCamera *camera );
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
	void ( *CompileShaderStage )( PLGShaderStage *stage, const char *buf, size_t length );
	void ( *SetShaderUniformValue )( PLGShaderProgram *program, int slot, const void *value, bool transpose );

	// Stencil operations
	void ( *StencilFunction )( PLGStencilTestFunction function, int reference, unsigned int mask );
} PLGDriverInterface;
