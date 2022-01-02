/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl_filesystem.h>
#include <plcore/pl_math.h>

typedef enum PLImageFormat {
	PL_IMAGEFORMAT_UNKNOWN,

	PL_IMAGEFORMAT_RGB4,   // 4 4 4 0
	PL_IMAGEFORMAT_RGBA4,  // 4 4 4 4
	PL_IMAGEFORMAT_RGB5,   // 5 5 5 0
	PL_IMAGEFORMAT_RGB5A1, // 5 5 5 1
	PL_IMAGEFORMAT_RGB565, // 5 6 5 0
	PL_IMAGEFORMAT_RGB8,   // 8 8 8 0
	PL_IMAGEFORMAT_RGBA8,  // 8 8 8 8
	PL_IMAGEFORMAT_RGBA12, // 12 12 12 12
	PL_IMAGEFORMAT_RGBA16, // 16 16 16 16
	PL_IMAGEFORMAT_RGBA16F,// 16 16 16 16

	PL_IMAGEFORMAT_RGBA_DXT1,
	PL_IMAGEFORMAT_RGB_DXT1,
	PL_IMAGEFORMAT_RGBA_DXT3,
	PL_IMAGEFORMAT_RGBA_DXT5,

	PL_IMAGEFORMAT_RGB_FXT1
} PLImageFormat;

/* todo: deprecate this */
typedef enum PLColourFormat {
	PL_COLOURFORMAT_ARGB,
	PL_COLOURFORMAT_ABGR,
	PL_COLOURFORMAT_RGB,
	PL_COLOURFORMAT_BGR,
	PL_COLOURFORMAT_RGBA,
	PL_COLOURFORMAT_BGRA,
} PLColourFormat;

typedef struct PLImage {
#if 1
	uint8_t **data;
#else
	uint8_t *data;
#endif
	unsigned int x, y;
	unsigned int width, height;
	size_t size;
	unsigned int levels;
	char path[ PL_SYSTEM_MAX_PATH ];
	PLImageFormat format;
	PLColourFormat colour_format;
	unsigned int flags;
} PLImage;

typedef struct PLPalette {
	PLImageFormat format;
	uint8_t *colours;
	unsigned int num_colours;
} PLPalette;

enum {
	PL_IMAGE_FILEFORMAT_ALL = 0,

	PL_BITFLAG( PL_IMAGE_FILEFORMAT_TGA, 0 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_PNG, 1 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_JPG, 2 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_BMP, 3 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_PSD, 4 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_GIF, 5 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_HDR, 6 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_PIC, 7 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_PNM, 8 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_FTX, 9 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_3DF, 10 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_TIM, 11 ),
	PL_BITFLAG( PL_IMAGE_FILEFORMAT_SWL, 12 ),
};

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN void PlRegisterImageLoader( const char *extension, PLImage *( *LoadImage )( const char *path ) );
PL_EXTERN void PlRegisterStandardImageLoaders( unsigned int flags );
PL_EXTERN void PlClearImageLoaders( void );

PL_EXTERN PLImage *PlCreateImage( uint8_t *buf, unsigned int w, unsigned int h, PLColourFormat col, PLImageFormat dat );
PL_EXTERN void PlDestroyImage( PLImage *image );

PL_EXTERN PLImage *PlLoadImage( const char *path );
PL_EXTERN bool PlWriteImage( const PLImage *image, const char *path );

PL_EXTERN bool PlConvertPixelFormat( PLImage *image, PLImageFormat new_format );

PL_EXTERN void PlInvertImageColour( PLImage *image );
PL_EXTERN void PlReplaceImageColour( PLImage *image, PLColour target, PLColour dest );

PL_EXTERN bool PlFlipImageVertical( PLImage *image );

PL_EXTERN unsigned int PlGetNumberOfColourChannels( PLColourFormat format );

PL_EXTERN bool PlImageIsPowerOfTwo( const PLImage *image );

PL_EXTERN void PlFreeImage( PLImage *image );

PL_EXTERN unsigned int PlGetImageSize( PLImageFormat format, unsigned int width, unsigned int height );

unsigned int PlGetImageFormatPixelSize( PLImageFormat format );
unsigned int PlGetNumImageFormatChannels( PLImageFormat format );

bool PlImageHasAlpha( const PLImage *image );

PL_EXTERN const char **PlGetSupportedImageFormats( unsigned int *numElements );

unsigned int PlGetImageWidth( const PLImage *image );
unsigned int PlGetImageHeight( const PLImage *image );
PLImageFormat PlGetImageFormat( const PLImage *image );
const uint8_t *PlGetImageData( const PLImage *image, unsigned int level );

#endif

PL_EXTERN_C_END
