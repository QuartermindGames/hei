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

#include "platform_image.h"
#include "platform_filesystem.h"

/*	Ritual's FTX Format	*/

typedef struct FTXHeader {
    PLuint32 width;
    PLuint32 height;
    PLuint32 alpha;
} FTXHeader;

PLresult plLoadFTXImage(FILE *fin, PLImage *out) {
    plSetErrorFunction("plLoadPPMImage");

    FTXHeader header;
    memset(&header, 0, sizeof(FTXHeader));
    header.width = (PLuint)plGetLittleLong(fin);
    header.height = (PLuint)plGetLittleLong(fin);
    header.alpha = (PLuint)plGetLittleLong(fin);

    memset(out, 0, sizeof(PLImage));
    out->size = (PLuint)(header.width * header.height * 4);
    out->data = new PLbyte*[1];
    out->data[0] = new PLbyte[out->size];

    if (fread(out->data[0], sizeof(uint8_t), out->size, fin) != out->size)
        return PL_RESULT_FILEREAD;

    out->format = PL_TEXTUREFORMAT_RGBA8;
    out->colour_format = VL_COLOURFORMAT_RGBA;
    out->width = (PLuint)header.width;
    out->height = (PLuint)header.height;
    return PL_RESULT_SUCCESS;
}