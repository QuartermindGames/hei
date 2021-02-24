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

#include <PL/pl_graphics.h>
#include <PL/platform_image.h>

// Texture Environment Modes
typedef enum PLTextureEnvironmentMode {
    PL_TEXTUREMODE_ADD,
    PL_TEXTUREMODE_MODULATE,
    PL_TEXTUREMODE_DECAL,
    PL_TEXTUREMODE_BLEND,
    PL_TEXTUREMODE_REPLACE,
    PL_TEXTUREMODE_COMBINE
} PLTextureEnvironmentMode;

typedef enum PLTextureClamp {
    PL_TEXTURE_CLAMP_CLAMP,
    PL_TEXTURE_CLAMP_WRAP,
} PLTextureClamp;

typedef enum PLTextureFilter {
    PL_TEXTURE_FILTER_MIPMAP_NEAREST,
    PL_TEXTURE_FILTER_MIPMAP_LINEAR,
    PL_TEXTURE_FILTER_MIPMAP_LINEAR_NEAREST,
    PL_TEXTURE_FILTER_MIPMAP_NEAREST_LINEAR,
    PL_TEXTURE_FILTER_NEAREST,                  // Nearest
    PL_TEXTURE_FILTER_LINEAR                    // Linear
} PLTextureFilter;

typedef enum PLTextureTarget {
    PL_TEXTURE_1D,
    PL_TEXTURE_2D,
    PL_TEXTURE_3D
} PLTextureTarget;

enum PLTextureFlag {
    PL_TEXTURE_FLAG_PRESERVE    = (1 << 1),
    PL_TEXTURE_FLAG_NOMIPS      = (1 << 2),
};

typedef struct PLTextureMappingUnit {
    bool active;

    unsigned int current_texture;
    unsigned int current_capabilities;

    PLTextureEnvironmentMode current_envmode;
} PLTextureMappingUnit;

typedef struct PLTexture {
    struct {
        unsigned int id;
    } internal;

    unsigned int flags;

    unsigned int x, y;
    unsigned int w, h;

    size_t size;
    unsigned int levels;
    unsigned int crc;

    char name[64];
	char path[ PL_SYSTEM_MAX_PATH ];

    PLTextureFilter filter;

    PLImageFormat format;
    PLDataFormat storage;
    PLColourFormat pixel;
} PLTexture;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN PLTexture *plCreateTexture(void);
PL_EXTERN PLTexture *plLoadTextureFromImage(const char *path, PLTextureFilter filter_mode);
PL_EXTERN void plDestroyTexture(PLTexture *texture);

//PL_EXTERN PLresult plUploadTextureData(PLTexture *texture, const PLTextureInfo *upload);
PL_EXTERN bool plUploadTextureImage(PLTexture *texture, const PLImage *upload);

PL_EXTERN unsigned int plGetMaxTextureSize(void);
PL_EXTERN unsigned int plGetMaxTextureUnits(void);
PL_EXTERN unsigned int plGetMaxTextureAnistropy(void);

PL_EXTERN void plSetTextureAnisotropy(PLTexture *texture, unsigned int amount);

PL_EXTERN void plSetTexture(PLTexture *texture, unsigned int tmu);
PL_EXTERN void plSetTextureUnit(unsigned int target);
PL_EXTERN void plSetTextureEnvironmentMode(PLTextureEnvironmentMode mode);
PL_EXTERN void plSetTextureFlags(PLTexture *texture, unsigned int flags);

PL_EXTERN const char *plPrintTextureMemoryUsage(void);

#endif

PL_EXTERN_C_END