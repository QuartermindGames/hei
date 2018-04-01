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
#include <PL/platform_graphics.h>
#include <PL/platform_graphics_camera.h>

#include "platform_private.h"

#ifdef _DEBUG
#   define GfxLog(...) plLogMessage(LOG_LEVEL_GRAPHICS, __VA_ARGS__)
#else
#   define GfxLog(...)
#endif

typedef struct PLGraphicsState {
    PLCullMode current_cullmode;

    PLColour current_clearcolour;
    PLColour current_colour;        // Current global colour.

    unsigned int current_capabilities;    // Enabled capabilities.
    unsigned int current_textureunit;

    // Textures

    PLTextureMappingUnit    *tmu;
    PLTexture               **textures;

    unsigned int num_textures;
    unsigned int max_textures;

    // Shader states

    PLShaderProgram* current_program;

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

    PLMatrix4x4 projection_matrix;
    PLMatrix4x4 model_matrix;

    ////////////////////////////////////////

    PLViewport current_viewport;

    bool mode_debug;
} GfxState;

typedef struct GfxLayer {
    PLGfxMode mode;    // Current gfx interface

    /* hw information */

    bool(*HWSupportsMultitexture)(void);
    bool(*HWSupportsShaders)(void);

    void(*GetMaxTextureUnits)(unsigned int *num_units);

    /******************************************/

    void(*SetBlendMode)(PLBlend a, PLBlend b);
    void(*SetCullMode)(PLCullMode mode);

    void(*SetClearColour)(PLColour rgba);
    void(*ClearBuffers)(unsigned int buffers);

    // Mesh
    void(*CreateMeshPOST)(PLMesh *mesh);
    void(*UploadMesh)(PLMesh *mesh);
    void(*DrawMesh)(PLMesh *mesh);
    void(*DeleteMesh)(PLMesh *mesh);

    // Texture
    void(*CreateTexture)(PLTexture *texture);
    void(*DeleteTexture)(PLTexture *texture);
    void(*BindTexture)(const PLTexture *texture);
    void(*UploadTexture)(PLTexture *texture, const PLImage *upload);
    void(*SwizzleTexture)(PLTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    // Camera
    void(*CreateCamera)(PLCamera *camera);
    void(*DeleteCamera)(PLCamera *camera);
    void(*SetupCamera)(PLCamera *camera);
    ///////////////////////////////////////////
    void(*DrawPerspectivePOST)(PLCamera *camera);

    // Shaders
    void(*CreateShaderProgram)(PLShaderProgram *program);
    void(*DeleteShaderProgram)(PLShaderProgram *program);
    void(*AttachShaderStage)(PLShaderProgram *program, PLShaderStage *stage);
    void(*DetachShaderStage)(PLShaderProgram *program, PLShaderStage *stage);
    void(*LinkShaderProgram)(PLShaderProgram *program);
    void(*SetShaderProgram)(PLShaderProgram *program);
    void(*CreateShaderStage)(PLShaderStage *stage);
    void(*DeleteShaderStage)(PLShaderStage *stage);
    void(*CompileShaderStage)(PLShaderStage *stage, const char *buf, size_t length);
} GfxLayer;

#define CallGfxFunction(FUNCTION, ...) \
    if(gfx_layer.FUNCTION != NULL) { \
        gfx_layer.FUNCTION(__VA_ARGS__); \
    } else { \
        GfxLog("unbound layer function %s was called\n", #FUNCTION); \
    }

///////////////////////////////////////////////////////

#define UseBufferScaling(a) \
    ((a)->viewport.r_w != 0 && (a)->viewport.r_h != 0) && \
    ((a)->viewport.r_w != (a)->viewport.w && (a)->viewport.r_h != (a)->viewport.h)

///////////////////////////////////////////////////////

PL_EXTERN_C

PL_EXTERN GfxState gfx_state;
PL_EXTERN GfxLayer gfx_layer;

void BindTexture(const PLTexture *texture);

#if defined(PL_SUPPORT_OPENGL)
void _InitOpenGL(void);
void ShutdownOpenGL(void);
#endif

void _InitSoftware(void);
void ShutdownSoftware(void);

PL_EXTERN_C_END