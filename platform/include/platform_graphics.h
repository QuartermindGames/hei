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

#define     VL_MODE_OPENGL
//			VL_MODE_OPENGL_CORE
//			VL_MODE_OPENGL_ES
//#define	VL_MODE_GLIDE
//#define	VL_MODE_DIRECT3D
//			VL_MODE_VULKAN

// todo, move these into platform_graphics.cpp
#if defined (VL_MODE_OPENGL)

#	include <GL/glew.h>

#	ifdef _WIN32
#		include <GL/wglew.h>
#	endif
#elif defined (VL_MODE_OPENGL_CORE)
#	include <GL/glcorearb.h>
#elif defined (VL_MODE_GLIDE)
#	ifdef _MSC_VER
#		define __MSC__
#	endif

#	include <glide.h>
#elif defined (VL_MODE_DIRECT3D)
#	include <d3d11.h>

#	pragma comment (lib, "d3d11.lib")
#	pragma comment (lib, "d3dx11.lib")
#	pragma comment (lib, "d3dx10.lib")
#elif defined (VL_MODE_VULKAN)
#endif

typedef PLuint PLVertexArray;
typedef PLuint PLRenderBuffer;
typedef PLuint PLFrameBuffer;

typedef enum PLDataFormat {
#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    PL_UNSIGNED_BYTE = GL_UNSIGNED_BYTE,
    PL_UNSIGNED_INT_8_8_8_8_REV = GL_UNSIGNED_INT_8_8_8_8_REV,
#endif
} PLDataFormat;

typedef enum PLBufferMask {
#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    VL_MASK_COLOUR = GL_COLOR_BUFFER_BIT,
    VL_MASK_DEPTH = GL_DEPTH_BUFFER_BIT,
    VL_MASK_ACCUM = GL_ACCUM_BUFFER_BIT,
    VL_MASK_STENCIL = GL_STENCIL_BUFFER_BIT,
#else
    VL_MASK_COLOUR		= (1 << 0),
    VL_MASK_DEPTH		= (1 << 1),
    VL_MASK_ACCUM		= (1 << 2),
    VL_MASK_STENCIL		= (1 << 3),
#endif
} PLBufferMask;

typedef enum PLColourFormat {
#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    VL_COLOURFORMAT_ARGB,
    VL_COLOURFORMAT_ABGR,
    VL_COLOURFORMAT_RGB = GL_RGB,
    VL_COLOURFORMAT_BGR = GL_BGR,
    VL_COLOURFORMAT_RGBA = GL_RGBA,
    VL_COLOURFORMAT_BGRA = GL_BGRA,
#elif defined (VL_MODE_GLIDE)
    VL_COLOURFORMAT_ARGB	= GR_COLORFORMAT_ARGB,
    VL_COLOURFORMAT_ABGR	= GR_COLORFORMAT_ABGR,
    VL_COLOURFORMAT_RGBA	= GR_COLORFORMAT_RGBA,
    VL_COLOURFORMAT_BGRA	= GR_COLORFORMAT_BGRA,
#else
    VL_COLOURFORMAT_ARGB	= 0,
    VL_COLOURFORMAT_ABGR	= 1,
    VL_COLOURFORMAT_RGBA	= 2,
    VL_COLOURFORMAT_BGRA	= 3,
#endif
} PLColourFormat;

typedef enum VLCullMode {
    VL_CULL_START = -1,

    VL_CULL_POSTIVE,
    VL_CULL_NEGATIVE,

    VL_CULL_END
} VLCullMode;

// Blending Modes
typedef enum PLBlend {
#if defined (VL_MODE_OPENGL)
    VL_BLEND_ZERO = GL_ZERO,
    VL_BLEND_ONE = GL_ONE,
    VL_BLEND_SRC_COLOR = GL_SRC_COLOR,
    VL_BLEND_ONE_MINUS_SRC_COLOR = GL_ONE_MINUS_SRC_COLOR,
    VL_BLEND_SRC_ALPHA = GL_SRC_ALPHA,
    VL_BLEND_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA,
    VL_BLEND_DST_ALPHA = GL_DST_ALPHA,
    VL_BLEND_ONE_MINUS_DST_ALPHA = GL_ONE_MINUS_DST_ALPHA,
    VL_BLEND_DST_COLOR = GL_DST_COLOR,
    VL_BLEND_ONE_MINUS_DST_COLOR = GL_ONE_MINUS_DST_COLOR,
    VL_BLEND_SRC_ALPHA_SATURATE = GL_SRC_ALPHA_SATURATE,
#elif defined (VL_MODE_GLIDE)
    VL_BLEND_ZERO					= GR_BLEND_ZERO,
    VL_BLEND_ONE					= GR_BLEND_ONE,
    VL_BLEND_SRC_COLOR				= GR_BLEND_SRC_COLOR,
    VL_BLEND_ONE_MINUS_SRC_COLOR	= GR_BLEND_ONE_MINUS_SRC_COLOR,
    VL_BLEND_SRC_ALPHA				= GR_BLEND_SRC_ALPHA,
    VL_BLEND_ONE_MINUS_SRC_ALPHA	= GR_BLEND_ONE_MINUS_SRC_ALPHA,
    VL_BLEND_DST_ALPHA				= GR_BLEND_DST_ALPHA,
    VL_BLEND_ONE_MINUS_DST_ALPHA	= GR_BLEND_ONE_MINUS_DST_ALPHA,
    VL_BLEND_DST_COLOR				= GR_BLEND_DST_COLOR,
    VL_BLEND_ONE_MINUS_DST_COLOR	= GR_BLEND_ONE_MINUS_DST_COLOR,
    VL_BLEND_SRC_ALPHA_SATURATE		= GR_BLEND_ALPHA_SATURATE,
#else
    VL_BLEND_ZERO,
    VL_BLEND_ONE,
    VL_BLEND_SRC_COLOR,
    VL_BLEND_ONE_MINUS_SRC_COLOR,
    VL_BLEND_SRC_ALPHA,
    VL_BLEND_ONE_MINUS_SRC_ALPHA,
    VL_BLEND_DST_ALPHA,
    VL_BLEND_ONE_MINUS_DST_ALPHA,
    VL_BLEND_DST_COLOR,
    VL_BLEND_ONE_MINUS_DST_COLOR,
    VL_BLEND_SRC_ALPHA_SATURATE,
#endif
} PLBlend;

// Blending
#define PL_BLEND_ADDITIVE   VL_BLEND_SRC_ALPHA, VL_BLEND_ONE
#define PL_BLEND_DEFAULT    VL_BLEND_SRC_ALPHA, VL_BLEND_ONE_MINUS_SRC_ALPHA

//-----------------
// Capabilities

typedef enum PLGraphicsCapability {
    VL_CAPABILITY_FOG = (1 << 0),           // Fog.
    PL_CAPABILITY_ALPHA_TEST = (1 << 1),    // Alpha-testing.
    PL_CAPABILITY_BLEND = (1 << 2),         // Blending.
    PL_CAPABILITY_TEXTURE_2D = (1 << 3),    // Enables/disables textures.
    VL_CAPABILITY_TEXTURE_GEN_S = (1 << 4), // Generate S coordinate.
    VL_CAPABILITY_TEXTURE_GEN_T = (1 << 5), // Generate T coordinate.
    VL_CAPABILITY_DEPTH_TEST = (1 << 6),    // Depth-testing.
    VL_CAPABILITY_STENCIL_TEST = (1 << 7),  // Stencil-testing.
    VL_CAPABILITY_MULTISAMPLE = (1 << 8),   // Multisampling.
    VL_CAPABILITY_CULL_FACE = (1 << 9),     // Automatically cull faces.
    PL_CAPABILITY_SCISSORTEST = (1 << 10),  // Scissor test for buffer clear.

    // Texture Generation
    PL_CAPABILITY_GENERATEMIPMAP = (1 << 20),
} PLGraphicsCapability;

PL_EXTERN_C

PL_EXTERN PLbool plIsGraphicsStateEnabled(PLuint flags);

PL_EXTERN void plEnableGraphicsStates(PLuint flags);
PL_EXTERN void plDisableGraphicsStates(PLuint flags);

PL_EXTERN_C_END

//-----------------
// Textures

typedef enum PLTextureTarget {
    PL_TEXTURE_1D,
    PL_TEXTURE_2D,
    PL_TEXTURE_3D
} PLTextureTarget;

typedef enum PLTextureClamp {
#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    VL_TEXTURECLAMP_CLAMP = GL_CLAMP_TO_EDGE,
    VL_TEXTURECLAMP_WRAP = GL_REPEAT,
#elif defined (VL_MODE_GLIDE)
    VL_TEXTURECLAMP_CLAMP	= GR_TEXTURECLAMP_CLAMP,
    VL_TEXTURECLAMP_WRAP	= GR_TEXTURECLAMP_WRAP,
#else
    VL_TEXTURECLAMP_CLAMP,
    VL_TEXTURECLAMP_WRAP,
#endif
} PLTextureClamp;

typedef enum PLTextureFilter {
    PL_TEXTUREFILTER_MIPMAP_NEAREST,        // GL_NEAREST_MIPMAP_NEAREST
    PL_TEXTUREFILTER_MIPMAP_LINEAR,         // GL_LINEAR_MIPMAP_LINEAR

    PL_TEXTUREFILTER_MIPMAP_LINEAR_NEAREST,    // GL_LINEAR_MIPMAP_NEAREST
    PL_TEXTUREFILTER_MIPMAP_NEAREST_LINEAR,    // GL_NEAREST_MIPMAP_LINEAR

    PL_TEXTUREFILTER_NEAREST,       // Nearest filtering
    PL_TEXTUREFILTER_LINEAR         // Linear filtering
} PLTextureFilter;

typedef enum PLTextureFormat {
    PL_TEXTUREFORMAT_RGB4,      // 4 4 4 0
    PL_TEXTUREFORMAT_RGBA4,     // 4 4 4 4
    PL_TEXTUREFORMAT_RGB5,      // 5 5 5 0
    PL_TEXTUREFORMAT_RGB5A1,    // 5 5 5 1
    PL_TEXTUREFORMAT_RGB565,    // 5 6 5 0
    VL_TEXTUREFORMAT_RGB8,      // 8 8 8 0
    PL_TEXTUREFORMAT_RGBA8,     // 8 8 8 8
    PL_TEXTUREFORMAT_RGBA12,    // 12 12 12 12
    PL_TEXTUREFORMAT_RGBA16,    // 16 16 16 16
    PL_TEXTUREFORMAT_RGBA16F,   // 16 16 16 16

    VL_TEXTUREFORMAT_RGBA_DXT1,
    VL_TEXTUREFORMAT_RGB_DXT1,
    VL_TEXTUREFORMAT_RGBA_DXT3,
    VL_TEXTUREFORMAT_RGBA_DXT5,

    VL_TEXTUREFORMAT_RGB_FXT1
} PLTextureFormat;

// Texture Environment Modes
typedef enum PLTextureEnvironmentMode {
    PL_TEXTUREMODE_ADD,
    PL_TEXTUREMODE_MODULATE,
    PL_TEXTUREMODE_DECAL,
    PL_TEXTUREMODE_BLEND,
    PL_TEXTUREMODE_REPLACE,
    PL_TEXTUREMODE_COMBINE
} PLTextureEnvironmentMode;

enum PLTextureFlag {
    PL_TEXTUREFLAG_PRESERVE = (1 << 0)
};

typedef struct PLTextureInfo {
    PLbyte *data;

    PLuint x, y;
    PLuint width, height;
    PLuint size;
    PLuint levels;

    PLColourFormat pixel_format;
    PLDataFormat storage_type;
    PLTextureFormat format;

    PLbool initial;

    PLuint flags;
} PLTextureInfo;

typedef struct PLTexture {
    PLuint id;

    PLuint flags;
    PLuint width, height;

    PLuint size;

    PLTextureFormat format;

    PLchar path[PL_MAX_PATH];
} PLTexture;

typedef struct PLImage PLImage;

PL_EXTERN_C

#if 0 // Legacy API
PL_EXTERN void plCreateTexture(PLTexture *texture);
PL_EXTERN void plDeleteTexture(PLTexture *texture);

PL_EXTERN void plUploadTexture(PLTexture texture, const PLTextureInfo *upload);



PL_EXTERN PLTexture plGetCurrentTexture(PLuint tmu);
PL_EXTERN PLuint plGetCurrentTextureUnit(void);

PL_EXTERN void plSetTexture(PLTexture texture);
PL_EXTERN void plSetTextureAnisotropy(PLTexture texture, PLuint amount);
PL_EXTERN void plSetTextureUnit(PLuint target);
PL_EXTERN void plSetTextureFilter(PLTexture texture, PLTextureFilter filter);
PL_EXTERN void plSetTextureEnvironmentMode(PLTextureEnvironmentMode mode);
#else

PL_EXTERN PLTexture *plCreateTexture(void);

PL_EXTERN PLresult plUploadTexture(PLTexture *texture, const PLTextureInfo *upload);
PL_EXTERN PLresult plUploadTextureImage(PLTexture *texture, const PLImage *upload);

PL_EXTERN PLuint plGetMaxTextureSize(void);
PL_EXTERN PLuint plGetMaxTextureUnits(void);
PL_EXTERN PLuint plGetMaxTextureAnistropy(void);

PL_EXTERN PLresult plSetTexture(PLTexture *texture);
PL_EXTERN PLresult plSetTextureFilter(PLTexture *texture, PLTextureFilter filter);
PL_EXTERN PLresult plSetTextureAnisotropy(PLTexture *texture, PLuint amount);

#endif

PL_EXTERN_C_END

//-----------------
// Drawing

typedef enum VLDrawMode {
#if defined (VL_MODE_OPENGL)
    VL_DRAWMODE_STATIC = GL_STATIC_DRAW,
    VL_DRAWMODE_DYNAMIC = GL_DYNAMIC_DRAW,
#else
    VL_DRAWMODE_STATIC,
    VL_DRAWMODE_DYNAMIC,
#endif
} VLDrawMode;

// Primitive Types
typedef enum PLPrimitive {
    VL_PRIMITIVE_IGNORE = -1,

    VL_PRIMITIVE_LINES,
    VL_PRIMITIVE_LINE_STRIP,
    VL_PRIMITIVE_POINTS,
    VL_PRIMITIVE_TRIANGLES,
    VL_PRIMITIVE_TRIANGLE_STRIP,
    VL_PRIMITIVE_TRIANGLE_FAN,
    VL_PRIMITIVE_TRIANGLE_FAN_LINE,
    VL_PRIMITIVE_QUADS,                // Advised to avoid this.

    VL_PRIMITIVE_END
} PLPrimitive;

typedef struct PLVertex {
    PLVector3D position, normal;

    PLVector2D ST[16];

    PLColour colour;
} PLVertex;

typedef struct PLDraw {
    PLVertex *vertices;                            // Array of vertices for the object.

    PLuint numverts;                            // Number of vertices.
    PLuint numtriangles;                        // Number of triangles.

    PLuint8 *indices;                                // List of indices.

    PLPrimitive primitive, primitive_restore;        // Type of primitive, and primitive to restore to.

    unsigned int _gl_vbo[16];                        // Vertex buffer object.
} PLDraw;

PL_EXTERN_C

PL_EXTERN void plSetBlendMode(PLBlend modea, PLBlend modeb);
PL_EXTERN void plSetCullMode(VLCullMode mode);

PL_EXTERN void plDraw(PLDraw *draw);
PL_EXTERN void plDrawVertexNormals(PLDraw *draw);

PL_EXTERN_C_END

//-----------------
// Framebuffers

typedef enum PLFBOTarget {
#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    PL_FRAMEBUFFER_DEFAULT = GL_FRAMEBUFFER,
    PL_FRAMEBUFFER_DRAW = GL_DRAW_FRAMEBUFFER,
    VL_FRAMEBUFFER_READ = GL_READ_FRAMEBUFFER
#else
    PL_FRAMEBUFFER_DEFAULT,
    PL_FRAMEBUFFER_DRAW,
    PL_FRAMEBUFFER_READ
#endif
} PLFBOTarget;

enum {
    PL_BUFFER_COLOUR    = (1 << 0),
    PL_BUFFER_DEPTH     = (1 << 1),
    PL_BUFFER_STENCIL   = (1 << 2),
};

PL_EXTERN_C

PL_EXTERN void plSetClearColour3f(PLfloat r, PLfloat g, PLfloat b);
PL_EXTERN void plSetClearColour4f(PLfloat r, PLfloat g, PLfloat b, PLfloat a);
PL_EXTERN void plSetClearColour4fv(PLColourf rgba);
PL_EXTERN void plSetClearColour(PLColour rgba);

PL_EXTERN void plClearBuffers(PLuint buffers);

PL_EXTERN_C_END

//-----------------
// Shaders

typedef PLuint PLShader;
typedef PLuint PLShaderProgram;

typedef enum PLUniformType {
    VL_UNIFORM_FLOAT,
    VL_UNIFORM_INT,
    VL_UNIFORM_UINT,
    VL_UNIFORM_BOOL,
    PL_UNIFORM_DOUBLE,

    // Textures
    VL_UNIFORM_TEXTURE1D,
    VL_UNIFORM_TEXTURE2D,
    VL_UNIFORM_TEXTURE3D,
    VL_UNIFORM_TEXTURECUBE,
    VL_UNIFORM_TEXTUREBUFFER,

    // Vectors
    VL_UNIFORM_VEC2,
    VL_UNIFORM_VEC3,
    VL_UNIFORM_VEC4,

    // Matrices
    VL_UNIFORM_MAT3,

    VL_UNIFORM_END
} PLUniformType;

#if 0
typedef int PLUniform;
#else
typedef struct PLUniform {
    PLint location;    // Location within the shader.

    PLUniformType type;    // Data type.

    PLchar def[16];    // Default value.
} PLUniform;
#endif

typedef int PLAttribute;

typedef enum PLShaderType {
#if defined (VL_MODE_OPENGL)
    PL_SHADER_FRAGMENT = GL_FRAGMENT_SHADER,
    PL_SHADER_VERTEX = GL_VERTEX_SHADER,
    PL_SHADER_GEOMETRY = GL_GEOMETRY_SHADER
#else
    PL_SHADER_FRAGMENT,
    PL_SHADER_VERTEX,
    PL_SHADER_GEOMETRY
#endif
} PLShaderType;

PL_EXTERN_C

PL_EXTERN void plCreateShader(PLShader *shader, PLShaderType type);
PL_EXTERN void plDeleteShader(PLShader *shader);
PL_EXTERN void plCreateShaderProgram(PLShaderProgram *program);
PL_EXTERN void plDeleteShaderProgram(PLShaderProgram *program);

PL_EXTERN PLShaderProgram plGetCurrentShaderProgram(void);

PL_EXTERN void plSetShaderProgram(PLShaderProgram program);

PL_EXTERN_C_END

//-----------------
// Lighting

typedef struct PLLight {
    PLVector3f position;
    PLColour colour;
    float radius;
} PLLight;

//-----------------

PL_EXTERN_C

PL_EXTERN void plViewport(PLint x, PLint y, PLuint width, PLuint height);
PL_EXTERN void plScissor(PLint x, PLint y, PLuint width, PLuint height);
PL_EXTERN void plFinish(void);

// Initialization
PL_EXTERN PLresult plInitGraphics(void);
PL_EXTERN void plShutdownGraphics(void);

// Hardware Information
PL_EXTERN const PLchar *_plGetHWExtensions(void);
PL_EXTERN const PLchar *_plGetHWRenderer(void);
PL_EXTERN const PLchar *_plGetHWVendor(void);
PL_EXTERN const PLchar *_plGetHWVersion(void);

PL_EXTERN PLbool plHWSupportsMultitexture(void);
PL_EXTERN PLbool plHWSupportsShaders(void);

PL_EXTERN_C_END
