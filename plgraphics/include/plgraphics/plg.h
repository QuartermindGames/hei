/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_math.h>

#include <plgraphics/plg_mesh.h>
#include <plgraphics/plg_texture.h>
#include <plgraphics/plg_framebuffer.h>

typedef struct PLGDriverImportTable PLGDriverImportTable;

typedef enum PLGCullMode {
	PLG_CULL_NONE, /* disables backface culling */
	PLG_CULL_POSITIVE,
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
#define PLG_BLEND_DISABLE  PLG_BLEND_NONE, PLG_BLEND_NONE
#define PLG_BLEND_ADDITIVE PLG_BLEND_SRC_ALPHA, PLG_BLEND_ONE
#define PLG_BLEND_DEFAULT  PLG_BLEND_SRC_ALPHA, PLG_BLEND_ONE_MINUS_SRC_ALPHA

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
	PLG_GFX_STATE_WIREFRAME,

	PLG_GFX_MAX_STATES
} PLGDrawState;

//-----------------
// Framebuffers
// todo: move all of this into pl_graphics_framebuffer.h

typedef enum PLGStencilOp {
	PLG_STENCIL_OP_KEEP,
	PLG_STENCIL_OP_ZERO,
	PLG_STENCIL_OP_REPLACE,
	PLG_STENCIL_OP_INCR,
	PLG_STENCIL_OP_INCRWRAP,
	PLG_STENCIL_OP_DECR,
	PLG_STENCIL_OP_DECRWRAP,
	PLG_STENCIL_OP_INVERT,

	PLG_MAX_STENCIL_OPS
} PLGStencilOp;

typedef enum PLGStencilFace {
	PLG_STENCIL_FACE_FRONT,
	PLG_STENCIL_FACE_BACK,
	PLG_STENCIL_FACE_FRONTANDBACK,
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
typedef struct PLGShaderStage PLGShaderStage;

#define PLG_MAX_DEFINITION_LENGTH 16
#define PLG_MAX_DEFINITIONS       16

#define PLG_MAX_SHADER_PROGRAM_ID 128

typedef struct PLGShaderStage {
	PLGShaderStageType type;

	/* software implementation of the shader stage */
	void ( *SWFallback )( PLGShaderProgram *program, PLGShaderStageType type );

	PLGShaderProgram *program;

	struct {
		unsigned int id;
	} internal;

	unsigned int numDefinitions;
	char definitions[ PLG_MAX_DEFINITIONS ][ PLG_MAX_DEFINITION_LENGTH ];

	PLPath path; /* original location it was loaded from */
} PLGShaderStage;

typedef struct PLGShaderProgram {
	struct {
		char *name;
		int slot;
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
	} *uniforms;
	unsigned int num_uniforms;

	struct {
		char name[ 32 ];

		unsigned int slot;
	} *attributes;
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

	char id[ PLG_MAX_SHADER_PROGRAM_ID ];
} PLGShaderProgram;

typedef struct PLGPolygon PLGPolygon;
typedef struct PLGVertex PLGVertex;
typedef struct PLGMesh PLGMesh;
typedef struct PLGCamera PLGCamera;

typedef enum PLGCompareFunction {
	PLG_COMPARE_NEVER,
	PLG_COMPARE_LESS,
	PLG_COMPARE_EQUAL,
	PLG_COMPARE_LEQUAL,
	PLG_COMPARE_GREATER,
	PLG_COMPARE_NOTEQUAL,
	PLG_COMPARE_GEQUAL,
	PLG_COMPARE_ALWAYS,
} PLGCompareFunction;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PLFunctionResult PlgInitializeGraphics( void );
void PlgShutdownGraphics( void );

PLGShaderStage *PlgCreateShaderStage( PLGShaderStageType type );
PLGShaderStage *PlgParseShaderStage( PLGShaderStageType type, const char *buf, size_t length );
PLGShaderStage *PlgLoadShaderStage( const char *path, PLGShaderStageType type );
void PlgCompileShaderStage( PLGShaderStage *stage, const char *buf, size_t length, const char *localDirectory );

void PlgSetShaderStageDefinitions( PLGShaderStage *stage, const char definitions[][ PLG_MAX_DEFINITION_LENGTH ], unsigned int numDefinitions );

void PlgAttachShaderStage( PLGShaderProgram *program, PLGShaderStage *stage );
bool PlgRegisterShaderStageFromMemory( PLGShaderProgram *program, const char *buffer, size_t length,
                                       PLGShaderStageType type );
bool PlgRegisterShaderStageFromDisk( PLGShaderProgram *program, const char *path, PLGShaderStageType type );
bool PlgLinkShaderProgram( PLGShaderProgram *program );

int PlgGetShaderUniformSlot( PLGShaderProgram *program, const char *name );
PLGShaderUniformType PlgGetShaderUniformType( const PLGShaderProgram *program, int slot );

void PlgSetShaderUniformDefaultValueByIndex( PLGShaderProgram *program, int slot, const void *defaultValue );
void PlgSetShaderUniformDefaultValue( PLGShaderProgram *program, const char *name, const void *defaultValue );
void PlgSetShaderUniformToDefaultByIndex( PLGShaderProgram *program, int slot );
void PlgSetShaderUniformToDefault( PLGShaderProgram *program, const char *name );
void PlgSetShaderUniformsToDefault( PLGShaderProgram *program );

void PlgSetShaderUniformValueByIndex( PLGShaderProgram *program, int slot, const void *value, bool transpose );
void PlgSetShaderUniformValue( PLGShaderProgram *program, const char *name, const void *value, bool transpose );

#	if !defined( PL_EXCLUDE_DEPRECATED_API )
PL_DEPRECATED( void PlgSetShaderUniformFloat( PLGShaderProgram *program, int slot, float value ) );
PL_DEPRECATED( void PlgSetShaderUniformInt( PLGShaderProgram *program, int slot, int value ) );
PL_DEPRECATED( void PlgSetShaderUniformVector4( PLGShaderProgram *program, int slot, PLVector4 value ) );
PL_DEPRECATED( void PlgSetShaderUniformVector3( PLGShaderProgram *program, int slot, PLVector3 value ) );
PL_DEPRECATED( void PlgSetShaderUniformMatrix4( PLGShaderProgram *program, int slot, PLMatrix4 value, bool transpose ) );
#	endif

PLGShaderProgram *PlgCreateShaderProgram( void );
PLGShaderProgram *PlgLoadCachedShaderProgram( const char *path );
void PlgDestroyShaderProgram( PLGShaderProgram *program, bool free_stages );

void PlgSetShaderProgramId( PLGShaderProgram *program, const char *id );
void PlgClearShaderProgramId( PLGShaderProgram *program );
const char *PlgGetShaderProgramId( PLGShaderProgram *program );

void PlgSetShaderCacheLocation( const char *path );
void PlgClearShaderCacheLocation( const char *path );
const char *PlgGetShaderCacheLocation( void );

PLGShaderProgram *PlgGetCurrentShaderProgram( void );

void PlgSetShaderProgram( PLGShaderProgram *program );
bool PlgIsShaderProgramEnabled( PLGShaderProgram *program );

// Debugging
void PlgInsertDebugMarker( const char *msg );
void PlgPushDebugGroupMarker( const char *msg );
void PlgPopDebugGroupMarker( void );

// Hardware Information
const char *PlgGetHWExtensions( void );
const char *PlgGetHWRenderer( void );
const char *PlgGetHWVendor( void );
const char *PlgGetHWVersion( void );

bool PlgSupportsHWShaders( void );

void PlgSetCullMode( PLGCullMode mode );
void PlgSetBlendMode( PLGBlend a, PLGBlend b );

void PlgSetDepthBufferMode( unsigned int mode );
void PlgDepthMask( bool enable );

/* stencil operations */

void PlgDepthBufferFunction( PLGCompareFunction compareFunction );
void PlgStencilBufferFunction( PLGCompareFunction function, int reference, unsigned int mask );

void PlgStencilOp( PLGStencilFace face, PLGStencilOp stencilFailOp, PLGStencilOp depthFailOp, PLGStencilOp depthPassOp );

bool PlgIsGraphicsStateEnabled( PLGDrawState state );

void PlgEnableGraphicsState( PLGDrawState state );
void PlgDisableGraphicsState( PLGDrawState state );

#endif

PL_EXTERN_C_END
