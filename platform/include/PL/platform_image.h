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

#include <PL/platform_filesystem.h>
#include <PL/platform_math.h>

typedef enum PLDataFormat {
	PL_UNSIGNED_BYTE,
	PL_UNSIGNED_INT_8_8_8_8_REV,
} PLDataFormat;

typedef enum PLImageFormat {
    PL_IMAGEFORMAT_UNKNOWN,

    PL_IMAGEFORMAT_RGB4,      // 4 4 4 0
    PL_IMAGEFORMAT_RGBA4,     // 4 4 4 4
    PL_IMAGEFORMAT_RGB5,      // 5 5 5 0
    PL_IMAGEFORMAT_RGB5A1,    // 5 5 5 1
    PL_IMAGEFORMAT_RGB565,    // 5 6 5 0
    PL_IMAGEFORMAT_RGB8,      // 8 8 8 0
    PL_IMAGEFORMAT_RGBA8,     // 8 8 8 8
    PL_IMAGEFORMAT_RGBA12,    // 12 12 12 12
    PL_IMAGEFORMAT_RGBA16,    // 16 16 16 16
    PL_IMAGEFORMAT_RGBA16F,   // 16 16 16 16

    PL_IMAGEFORMAT_RGBA_DXT1,
    PL_IMAGEFORMAT_RGB_DXT1,
    PL_IMAGEFORMAT_RGBA_DXT3,
    PL_IMAGEFORMAT_RGBA_DXT5,

    PL_IMAGEFORMAT_RGB_FXT1
} PLImageFormat;

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
    uint8_t         **data;
#else
    uint8_t         *data;
#endif
    unsigned int    x, y;
    unsigned int    width, height;
    size_t          size;
    unsigned int    levels;
    char            path[PL_SYSTEM_MAX_PATH];
    PLImageFormat   format;
    PLColourFormat  colour_format;
    unsigned int    flags;
} PLImage;

typedef struct PLPalette {
    PLImageFormat   format;
    uint8_t         *colours;
    unsigned int    num_colours;
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
};

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN void plRegisterImageLoader( const char *extension, PLImage *(*LoadImage)( const char *path ) );
PL_EXTERN void plRegisterStandardImageLoaders( unsigned int flags );
PL_EXTERN void plClearImageLoaders( void );

PL_EXTERN PLImage *plCreateImage(uint8_t *buf, unsigned int w, unsigned int h, PLColourFormat col, PLImageFormat dat);
PL_EXTERN void plDestroyImage(PLImage *image);

PL_EXTERN PLImage *plLoadImage( const char *path );
PL_EXTERN bool plWriteImage(const PLImage *image, const char *path);

PL_EXTERN bool plConvertPixelFormat(PLImage *image, PLImageFormat new_format);
//PL_EXTERN bool plConvertColourFormat( PLImage *image, PLColourFormat newFormat );

PL_EXTERN void plInvertImageColour(PLImage *image);
PL_EXTERN void plReplaceImageColour(PLImage *image, PLColour target, PLColour dest);

PL_EXTERN bool plFlipImageVertical(PLImage *image);

PL_EXTERN unsigned int plGetNumberOfColourChannels(PLColourFormat format);

PL_EXTERN bool plImageIsPowerOfTwo( const PLImage *image );

PL_EXTERN void plFreeImage(PLImage *image);

PL_EXTERN unsigned int plGetImageSize(PLImageFormat format, unsigned int width, unsigned int height);

unsigned int plImageBytesPerPixel(PLImageFormat format);

PL_EXTERN const char **plGetSupportedImageFormats( unsigned int *numElements );

#endif

PL_EXTERN_C_END
