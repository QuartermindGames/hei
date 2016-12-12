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
#include "platform_graphics.h"

enum {
    PL_IMAGEFLAG_FULLBRIGHT     = (1 << 0),

    PL_IMAGEFLAG_NEAREST    = (5 << 0),
    PL_IMAGEFLAG_BILINEAR   = (6 << 0),
    PL_IMAGEFLAG_TRILINEAR  = (7 << 0),

    PL_IMAGEFLAG_NORMALMAP      = (10 << 0),
    PL_IMAGEFLAG_ENVMAP         = (11 << 0),
    PL_IMAGEFLAG_SPHEREMAP      = (12 << 0),
} PLImageFlag;

typedef struct PLImage {
    PLbyte **data;

    PLuint x, y;
    PLuint width, height;
    PLuint size;
    PLuint levels;

    PLchar path[PL_MAX_PATH];

    PLTextureFormat format;
    PLColourFormat  colour_format;

    PLuint flags;
} PLImage;

#define PLIMAGE_EXTENSION_FTX   "ftx"    // Ritual's FTX image format.
#define PLIMAGE_EXTENSION_DTX   "dtx"    // Lithtech's DTX image format.
#define PLIMAGE_EXTENSION_PPM   "ppm"    // Portable Pixel Map format.
#define PLIMAGE_EXTENSION_KTX   "ktx"
#define PLIMAGE_EXTENSION_TGA   "tga"
#define PLIMAGE_EXTENSION_PNG   "png"
#define PLIMAGE_EXTENSION_VTF   "vtf"

PL_EXTERN_C

extern void plFreeImage(PLImage *image);

extern PLbool plIsValidImageSize(PLuint width, PLuint height);

extern PLresult plLoadImage(const PLchar *path, PLImage *out);

extern PLresult plLoadFTXImage(FILE *fin, PLImage *out);    // Ritual's FTX image format.
extern PLresult plLoadPPMImage(FILE *fin, PLImage *out);    // Portable Pixel Map format.
extern PLresult plLoadDTXImage(FILE *fin, PLImage *out);    // Lithtech's DTX image format.
extern PLresult plLoadVTFImage(FILE *fin, PLImage *out);    // Valve's VTF image format.

PL_EXTERN_C_END