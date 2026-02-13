/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl_image.h>

// Texture Environment Modes
typedef enum PLGTextureEnvironmentMode {
	PLG_TEXTUREMODE_ADD,
	PLG_TEXTUREMODE_MODULATE,
	PLG_TEXTUREMODE_DECAL,
	PLG_TEXTUREMODE_BLEND,
	PLG_TEXTUREMODE_REPLACE,
	PLG_TEXTUREMODE_COMBINE
} PLGTextureEnvironmentMode;

typedef enum PLGTextureWrapMode {
	PLG_TEXTURE_WRAP_MODE_REPEAT,
	PLG_TEXTURE_WRAP_MODE_MIRRORED_REPEAT,
	PLG_TEXTURE_WRAP_MODE_CLAMP_EDGE,
	PLG_TEXTURE_WRAP_MODE_CLAMP_BORDER,

	PLG_MAX_TEXTURE_WRAP_MODES
} PLGTextureWrapMode;

typedef enum PLGTextureFilter {
	PLG_TEXTURE_FILTER_MIPMAP_NEAREST,
	PLG_TEXTURE_FILTER_MIPMAP_LINEAR,
	PLG_TEXTURE_FILTER_MIPMAP_LINEAR_NEAREST,
	PLG_TEXTURE_FILTER_MIPMAP_NEAREST_LINEAR,
	PLG_TEXTURE_FILTER_NEAREST,// Nearest
	PLG_TEXTURE_FILTER_LINEAR, // Linear

	PLG_MAX_TEXTURE_FILTER_MODES
} PLGTextureFilter;

typedef enum PLGTextureTarget {
	PLG_TEXTURE_1D,
	PLG_TEXTURE_2D,
	PLG_TEXTURE_3D
} PLGTextureTarget;

enum PLGTextureFlag {
	PLG_TEXTURE_FLAG_PRESERVE = ( 1 << 1 ),
	PLG_TEXTURE_FLAG_NOMIPS = ( 1 << 2 ),
};

typedef struct PLGTextureMappingUnit {
	bool active;

	unsigned int current_texture;
	unsigned int current_capabilities;

	PLGTextureEnvironmentMode current_envmode;
} PLGTextureMappingUnit;

typedef struct PLGTexture {
	unsigned int flags;

	unsigned int x, y;
	unsigned int w, h;

	size_t size;
	unsigned int levels;
	unsigned int crc;

	char name[ 64 ];
	char path[ PL_SYSTEM_MAX_PATH ];

	PLGTextureFilter filter;
	PLGTextureWrapMode wrapMode;

	PLImageFormat format;
	PLColourFormat pixel;

	void *driver;// driver specific data
} PLGTexture;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN PLGTexture *PlgCreateTexture( void );
PL_EXTERN void PlgDestroyTexture( PLGTexture *texture );

PL_EXTERN bool PlgUploadTextureImage( PLGTexture *texture, const PLImage *upload );

PL_EXTERN unsigned int PlgGetMaxTextureSize( void );
PL_EXTERN unsigned int PlgGetMaxTextureUnits( void );
PL_EXTERN unsigned int PlgGetMaxTextureAnistropy( void );

PL_EXTERN void PlgSetTextureAnisotropy( PLGTexture *texture, unsigned int amount );
PL_EXTERN void PlgSetTextureFilter( PLGTexture *texture, PLGTextureFilter filter );
void PlgSetTextureWrapMode( PLGTexture *texture, PLGTextureWrapMode wrapMode );

PL_EXTERN void PlgSetTexture( PLGTexture *texture, unsigned int tmu );
PL_EXTERN void PlgSetTextureEnvironmentMode( PLGTextureEnvironmentMode mode );
PL_EXTERN void PlgSetTextureFlags( PLGTexture *texture, unsigned int flags );

#endif /* !defined( PL_COMPILE_PLUGIN ) */

PL_EXTERN_C_END
