/*
DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
Version 2, December 2004

Copyright (C) 2011-2016 Mark E Sowden <markelswo@gmail.com>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

0. You just DO WHAT THE FUCK YOU WANT TO.
*/

#pragma once

#include "platform.h"
#include "platform_graphics.h"

enum PLImageFlag {
    PL_IMAGEFLAG_FULLBRIGHT     = (1 << 0),

    PL_IMAGEFLAG_NEAREST    = (5 << 0),
    PL_IMAGEFLAG_BILINEAR   = (6 << 0),
    PL_IMAGEFLAG_TRILINEAR  = (7 << 0),

    PL_IMAGEFLAG_NORMALMAP      = (10 << 0),
    PL_IMAGEFLAG_ENVMAP         = (11 << 0),
    PL_IMAGEFLAG_SPHEREMAP      = (12 << 0),
} PLImageFlag;

typedef struct PLImage {
    PLbyte *data;

    PLuint x, y;
    PLuint width, height;
    PLuint size;
    PLuint levels;

    PLchar path[PL_MAX_PATH];

    PLTextureFormat format;
    PLColourFormat  colour_format;

    PLuint flags;
} PLImage;

#define PLIMAGE_EXTENSION_FTX    ".ftx"    // Ritual's FTX image format.
#define PLIMAGE_EXTENSION_DTX    ".dtx"    // Lithtech's DTX image format.
#define PLIMAGE_EXTENSION_PPM    ".ppm"    // Portable Pixel Map format.
#define PLIMAGE_EXTENSION_KTX    ".ktx"
#define PLIMAGE_EXTENSION_TGA    ".tga"
#define PLIMAGE_EXTENSION_PNG    ".png"

PL_EXTERN_C

extern void plFreeImage(PLImage *image);

extern PLbool plIsValidImageSize(PLuint width, PLuint height);

extern PLresult plLoadFTXImage(FILE *fin, PLImage *out);    // Ritual's FTX image format.
extern PLresult plLoadPPMImage(FILE *fin, PLImage *out);    // Portable Pixel Map format.
extern PLresult plLoadDTXImage(FILE *fin, PLImage *out);    // Lithtech's DTX image format.

PL_EXTERN_C_END