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

#include "platform.h"
#include "platform_math.h"
#include "platform_image.h"

typedef enum PLGfxMode {
    PL_GFX_MODE_NONE,

    PL_GFX_MODE_SOFTWARE,

    PL_GFX_MODE_OPENGL,
    PL_GFX_MODE_OPENGL_CORE,
    PL_GFX_MODE_OPENGL_ES,

    /* enforced compatibility modes */
	PL_GFX_MODE_OPENGL_1_0,

    PL_GFX_MODE_VULKAN,

    PL_GFX_MODE_GLIDE,

    PL_GFX_MODE_DIRECT3D,
} PLGfxMode;

typedef unsigned int PLVertexArray;
typedef unsigned int PLRenderBuffer;

typedef void PLGraphicsContext;

typedef enum PLDataFormat {
    PL_UNSIGNED_BYTE,
    PL_UNSIGNED_INT_8_8_8_8_REV,
} PLDataFormat;

typedef enum PLCullMode {
    PL_CULL_NONE,       /* disables backface culling */
    PL_CULL_POSTIVE,
    PL_CULL_NEGATIVE
} PLCullMode;

// Blending Modes
typedef enum PLBlend {
    PL_BLEND_NONE,  // disables blending

    PL_BLEND_ZERO,
    PL_BLEND_ONE,
    PL_BLEND_SRC_COLOR,
    PL_BLEND_ONE_MINUS_SRC_COLOR,
    PL_BLEND_SRC_ALPHA,
    PL_BLEND_ONE_MINUS_SRC_ALPHA,
    PL_BLEND_DST_ALPHA,
    PL_BLEND_ONE_MINUS_DST_ALPHA,
    PL_BLEND_DST_COLOR,
    PL_BLEND_ONE_MINUS_DST_COLOR,
    PL_BLEND_SRC_ALPHA_SATURATE,
} PLBlend;

// Blending
#define PL_BLEND_DISABLE    PL_BLEND_NONE, PL_BLEND_NONE
#define PL_BLEND_ADDITIVE   PL_BLEND_SRC_ALPHA, PL_BLEND_ONE
#define PL_BLEND_DEFAULT    PL_BLEND_SRC_ALPHA, PL_BLEND_ONE_MINUS_SRC_ALPHA

//-----------------
// Capabilities

/* Replace these with generic functions, rather than relying on just flags?
 * plEnableFog(bool active)
 * plEnableAlphaTest(bool active)
 * plEnableBlend(bool active)
 */

typedef enum PLGraphicsState {
    PL_GFX_STATE_FOG,             // Fog.
    PL_GFX_STATE_ALPHATEST,       // Alpha-testing.
    PL_GFX_STATE_BLEND,           // Blending.
    PL_GFX_STATE_DEPTHTEST,       // Depth-testing.
    PL_GFX_STATE_STENCILTEST,     // Stencil-testing.
    PL_GFX_STATE_MULTISAMPLE,     // Multisampling.
    PL_GFX_STATE_SCISSORTEST,     // Scissor test for buffer clear.
    PL_GFX_STATE_ALPHATOCOVERAGE, // Alpha to Coverage

    PL_GFX_MAX_STATES
} PLGraphicsState;

PL_EXTERN_C

PL_EXTERN bool plIsGraphicsStateEnabled(PLGraphicsState state);

PL_EXTERN void plEnableGraphicsState(PLGraphicsState state);
PL_EXTERN void plDisableGraphicsState(PLGraphicsState state);

PL_EXTERN_C_END

//-----------------
// Textures

#include "pl_graphics_texture.h"

//-----------------
// Meshes

#include "platform_mesh.h"

//-----------------
// Framebuffers
// todo: move all of this into pl_graphics_framebuffer.h

typedef enum PLFBOTarget {
    PL_FRAMEBUFFER_DEFAULT,
    PL_FRAMEBUFFER_DRAW,
    PL_FRAMEBUFFER_READ
} PLFBOTarget;

typedef enum PLFBORenderFlags {
    PL_BUFFER_COLOUR    = (1 << 0),
    PL_BUFFER_DEPTH     = (1 << 1),
    PL_BUFFER_STENCIL   = (1 << 2),
} PLFrameBufferRenderFlags;

typedef struct PLFrameBuffer PLFrameBuffer;
typedef struct PLTexture PLTexture;

PL_EXTERN_C

PL_EXTERN PLFrameBuffer *plCreateFrameBuffer(unsigned int w, unsigned int h, unsigned int flags);
PL_EXTERN void plDestroyFrameBuffer(PLFrameBuffer *buffer);
PL_EXTERN void plBindFrameBuffer(PLFrameBuffer *buffer, PLFBOTarget target_binding);
PL_EXTERN void plBlitFrameBuffers(PLFrameBuffer *src_buffer, unsigned int src_w, unsigned int src_h, PLFrameBuffer *dst_buffer, unsigned int dst_w, unsigned int dst_h, bool linear );

PL_EXTERN PLTexture *plGetFrameBufferTextureAttachment( PLFrameBuffer *buffer );
PL_EXTERN void plGetFrameBufferResolution( const PLFrameBuffer *buffer, unsigned int *width, unsigned int *height );

PL_EXTERN void plSetClearColour(PLColour rgba);
PL_EXTERN void plClearBuffers(unsigned int buffers);

PL_EXTERN_C_END

//-----------------
// Lighting

typedef enum PLLightType {
    PL_LIGHT_TYPE_SPOT,  // Spotlight
    PL_LIGHT_TYPE_OMNI   // Omni-directional
} PLLightType;

typedef struct PLLight {
    PLVector3 position;
    PLVector3 angles;

    PLColour colour;

    PLLightType type;
} PLLight;

//-----------------
// Shaders
// todo: move all of this into pl_graphics_shader.h

typedef int PLSampler1D, PLSampler2D, PLSampler3D;
typedef int PLSamplerCube;
typedef int PLSampler1DShadow, PLSampler2DShadow;
typedef int PLShaderAttribute;

typedef enum PLShaderType {
    PL_SHADER_TYPE_VERTEX,
    PL_SHADER_TYPE_FRAGMENT,
    PL_SHADER_TYPE_GEOMETRY,
    PL_SHADER_TYPE_COMPUTE,

    PL_NUM_SHADER_TYPES
} PLShaderStageType;

typedef enum PLShaderUniformType {
    PL_INVALID_UNIFORM,

    PL_UNIFORM_FLOAT,
    PL_UNIFORM_INT,
    PL_UNIFORM_UINT,
    PL_UNIFORM_BOOL,
    PL_UNIFORM_DOUBLE,

    /* textures */
    PL_UNIFORM_SAMPLER1D,
    PL_UNIFORM_SAMPLER2D,
    PL_UNIFORM_SAMPLER3D,
    PL_UNIFORM_SAMPLERCUBE,
    PL_UNIFORM_SAMPLER1DSHADOW,
    PL_UNIFORM_SAMPLER2DSHADOW,

    /* vectors */
    PL_UNIFORM_VEC2,
    PL_UNIFORM_VEC3,
    PL_UNIFORM_VEC4,

    /* matrices */
    PL_UNIFORM_MAT3,
    PL_UNIFORM_MAT4,
} PLShaderUniformType;

typedef struct PLShaderProgram PLShaderProgram;

typedef struct PLShaderStage {
    PLShaderStageType type;

    /* software implementation of the shader stage */
	void(*SWFallback)(PLShaderProgram *program, PLShaderStageType type);

    PLShaderProgram *program;

    struct {
        unsigned int id;
    } internal;
} PLShaderStage;

typedef struct PLShaderProgram {
    struct {
        char name[32];

        unsigned int slot;

        PLShaderUniformType type;
    } *uniforms;
    unsigned int num_uniforms;

    struct {
        char name[32];

        unsigned int slot;
    } *attributes;
    unsigned int num_attributes;

    PLShaderStage *stages[PL_NUM_SHADER_TYPES];
    unsigned int num_stages;

    bool is_linked;

    struct {
        unsigned int id;

        PLShaderAttribute   v_position;
        PLShaderAttribute   v_normal;
        PLShaderAttribute   v_uv;
        PLShaderAttribute   v_colour;
    } internal;
} PLShaderProgram;

PL_EXTERN_C

//PL_EXTERN PLShaderStage *plCreateShaderStage(PLShaderStageType type);
PL_EXTERN PLShaderStage *plParseShaderStage(PLShaderStageType type, const char *buf, size_t length);
PL_EXTERN PLShaderStage *plLoadShaderStage(const char *path, PLShaderStageType type);
PL_EXTERN void plCompileShaderStage(PLShaderStage *stage, const char *buf, size_t length);

PL_EXTERN void plAttachShaderStage(PLShaderProgram *program, PLShaderStage *stage);
PL_EXTERN bool plRegisterShaderStageFromMemory(PLShaderProgram *program, const char *buffer, size_t length,
                                               PLShaderStageType type);
PL_EXTERN bool plRegisterShaderStageFromDisk(PLShaderProgram *program, const char *path, PLShaderStageType type);
PL_EXTERN bool plLinkShaderProgram(PLShaderProgram *program);

PL_EXTERN int plGetShaderUniformSlot(PLShaderProgram *program, const char *name);

PL_EXTERN void plSetShaderUniformFloat(PLShaderProgram *program, int slot, float value); /* todo */
PL_EXTERN void plSetShaderUniformDouble(PLShaderProgram *program, int slot, double value); /* todo */
PL_EXTERN void plSetShaderUniformBool(PLShaderProgram *program, int slot, bool value); /* todo */
PL_EXTERN void plSetShaderUniformInt(PLShaderProgram *program, int slot, int value);
PL_EXTERN void plSetShaderUniformUInt(PLShaderProgram *program, int slot, unsigned int value); /* todo */
PL_EXTERN void plSetShaderUniformVector4(PLShaderProgram* program, int slot, PLVector4 value);
PL_EXTERN void plSetShaderUniformVector3(PLShaderProgram *program, int slot, PLVector3 value); /* todo */
PL_EXTERN void plSetShaderUniformVector2(PLShaderProgram *program, int slot, PLVector2 value); /* todo */
PL_EXTERN void plSetShaderUniformMatrix4(PLShaderProgram *program, int slot, PLMatrix4 value, bool transpose);

#define plSetNamedShaderUniformFloat(program, name, value)  \
    plSetShaderUniformFloat((program), plGetShaderUniformSlot((program), (name)), (value))
#define plSetNamedShaderUniformDouble(program, name, value) \
    plSetShaderUniformDouble((program), plGetShaderUniformSlot((program), (name)), (value))
#define plSetNamedShaderUniformBool(program, name, value) \
    plSetShaderUniformBool((program), plGetShaderUniformSlot((program), (name)), (value))
#define plSetNamedShaderUniformInt(program, name, value) \
    plSetShaderUniformInt((program), plGetShaderUniformSlot((program), (name)), (value))
#define plSetNamedShaderUniformUInt(program, name, value) \
    plSetShaderUniformUInt((program), plGetShaderUniformSlot((program), (name)), (value))
#define plSetNamedShaderUniformVector4(program, name, value) \
    plSetShaderUniformVector4((program), plGetShaderUniformSlot((program), (name)), (value))
#define plSetNamedShaderUniformVector3(program, name, value) \
    plSetShaderUniformVector3((program), plGetShaderUniformSlot((program), (name)), (value))
#define plSetNamedShaderUniformVector2(program, name, value) \
    plSetShaderUniformVector2((program), plGetShaderUniformSlot((program), (name)), (value))
#define plSetNamedShaderUniformMatrix4(program, name, value, transpose) \
    plSetShaderUniformMatrix4((program), plGetShaderUniformSlot((program), (name)), (value), (transpose))

PL_EXTERN PLShaderProgram *plCreateShaderProgram(void);
PL_EXTERN void plDestroyShaderProgram(PLShaderProgram *program, bool free_stages);

PL_EXTERN PLShaderProgram *plGetCurrentShaderProgram(void);

PL_EXTERN void plSetShaderProgram(PLShaderProgram *program);
PL_EXTERN bool plIsShaderProgramEnabled(PLShaderProgram *program);

PL_EXTERN_C_END

//-----------------

PL_EXTERN_C

// Debugging
PL_EXTERN void plInsertDebugMarker(const char *msg);
PL_EXTERN void plPushDebugGroupMarker(const char *msg);
PL_EXTERN void plPopDebugGroupMarker(void);

// Hardware Information
PL_EXTERN const char *plGetHWExtensions(void);
PL_EXTERN const char *plGetHWRenderer(void);
PL_EXTERN const char *plGetHWVendor(void);
PL_EXTERN const char *plGetHWVersion(void);

PL_EXTERN bool plHWSupportsMultitexture(void);
PL_EXTERN bool plHWSupportsShaders(void);

PL_EXTERN void plSetCullMode(PLCullMode mode);
PL_EXTERN void plSetBlendMode(PLBlend a, PLBlend b);

enum {
    PL_DEPTHBUFFER_DISABLE,
    PL_DEPTHBUFFER_ENABLE,
};
PL_EXTERN void plSetDepthBufferMode(unsigned int mode);
PL_EXTERN void plSetDepthMask(bool enable);

PL_EXTERN void plProcessGraphics(void);

PL_EXTERN void plSetGraphicsMode(PLGfxMode mode);

/* polygons */

typedef struct PLPolygon PLPolygon;
typedef struct PLVertex PLVertex;
typedef struct PLMesh PLMesh;

PL_EXTERN PLPolygon *plCreatePolygon( void );
PL_EXTERN void plDestroyPolygon( PLPolygon *polygon );
PL_EXTERN void plAddPolygonVertex( PLPolygon *polygon, const PLVertex *vertex );
PL_EXTERN void plRemovePolygonVertex( PLPolygon *polygon, unsigned int vertIndex );
PL_EXTERN unsigned int plGetNumOfPolygonVertices( const PLPolygon *polygon );
PL_EXTERN PLVertex *plGetPolygonVertex( PLPolygon *polygon, unsigned int vertIndex );
PL_EXTERN PLVertex *plGetPolygonVertices( PLPolygon *polygon, unsigned int *numVertices );
PL_EXTERN unsigned int plGetNumOfPolygonTriangles( const PLPolygon *polygon );
PL_EXTERN unsigned int *plConvertPolygonToTriangles( const PLPolygon *polygon, unsigned int *numTriangles );
PL_EXTERN PLMesh *plConvertPolygonToMesh( const PLPolygon *polygon );

PL_EXTERN_C_END
