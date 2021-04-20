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

#include <PL/platform_image.h>

// Texture Environment Modes
typedef enum PLGTextureEnvironmentMode {
    PL_TEXTUREMODE_ADD,
    PL_TEXTUREMODE_MODULATE,
    PL_TEXTUREMODE_DECAL,
    PL_TEXTUREMODE_BLEND,
    PL_TEXTUREMODE_REPLACE,
    PL_TEXTUREMODE_COMBINE
} PLGTextureEnvironmentMode;

typedef enum PLGTextureClamp {
    PL_TEXTURE_CLAMP_CLAMP,
    PL_TEXTURE_CLAMP_WRAP,
} PLGTextureClamp;

typedef enum PLGTextureFilter {
    PL_TEXTURE_FILTER_MIPMAP_NEAREST,
    PL_TEXTURE_FILTER_MIPMAP_LINEAR,
    PL_TEXTURE_FILTER_MIPMAP_LINEAR_NEAREST,
    PL_TEXTURE_FILTER_MIPMAP_NEAREST_LINEAR,
    PL_TEXTURE_FILTER_NEAREST,                  // Nearest
    PL_TEXTURE_FILTER_LINEAR                    // Linear
} PLGTextureFilter;

typedef enum PLGTextureTarget {
    PL_TEXTURE_1D,
    PL_TEXTURE_2D,
    PL_TEXTURE_3D
} PLGTextureTarget;

enum PLTextureFlag {
    PL_TEXTURE_FLAG_PRESERVE    = (1 << 1),
    PL_TEXTURE_FLAG_NOMIPS      = (1 << 2),
};

typedef struct PLGTextureMappingUnit {
    bool active;

    unsigned int current_texture;
    unsigned int current_capabilities;

	PLGTextureEnvironmentMode current_envmode;
} PLGTextureMappingUnit;

typedef struct PLGTexture {
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

	PLGTextureFilter filter;

    PLImageFormat format;
    PLDataFormat storage;
    PLColourFormat pixel;
} PLGTexture;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN PLGTexture *plCreateTexture(void);
PL_EXTERN PLGTexture *plLoadTextureFromImage(const char *path, PLGTextureFilter filter_mode);
PL_EXTERN void plDestroyTexture( PLGTexture *texture);

//PL_EXTERN PLresult plUploadTextureData(PLTexture *texture, const PLTextureInfo *upload);
PL_EXTERN bool plUploadTextureImage( PLGTexture *texture, const PLImage *upload);

PL_EXTERN unsigned int plGetMaxTextureSize(void);
PL_EXTERN unsigned int plGetMaxTextureUnits(void);
PL_EXTERN unsigned int plGetMaxTextureAnistropy(void);

PL_EXTERN void plSetTextureAnisotropy( PLGTexture *texture, unsigned int amount);

PL_EXTERN void plSetTexture( PLGTexture *texture, unsigned int tmu);
PL_EXTERN void plSetTextureUnit(unsigned int target);
PL_EXTERN void plSetTextureEnvironmentMode( PLGTextureEnvironmentMode mode);
PL_EXTERN void plSetTextureFlags( PLGTexture *texture, unsigned int flags);

PL_EXTERN const char *plPrintTextureMemoryUsage(void);

#endif /* !defined( PL_COMPILE_PLUGIN ) */

PL_EXTERN_C_END
