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

#include "PL/platform_image.h"

/*	PPM Format	*/

#define PPM_HEADER_SIZE 70

PLresult LoadPPMImage(FILE *fin, PLImage *out) {
    _plSetCurrentFunction("_plLoadPPMImage");

    char header[PPM_HEADER_SIZE];
    memset(&header, 0, sizeof(header));

    fgets(header, PPM_HEADER_SIZE, fin);
    if (strncmp(header, "P6", 2) != 0) {
        ReportError(PL_RESULT_FILEVERSION, "Unsupported PPM type!\n");
        return PL_RESULT_FILEVERSION;
    }

    int i = 0, d;
    unsigned int w = 0, h = 0;
    while (i < 3) {
        fgets(header, PPM_HEADER_SIZE, fin);
        if (header[0] == '#')
            continue;

        if (i == 0) {
            i += sscanf(header, "%d %d %d", &w, &h, &d);
        } else if (i == 1) {
            i += sscanf(header, "%d %d", &h, &d);
        } else if (i == 2) {
            i += sscanf(header, "%d", &d);
        }
    }

    memset(out, 0, sizeof(PLImage));

    out->levels = 1;
    out->size = w * h * 3;
    out->data = calloc(out->levels, sizeof(uint8_t*));
    if(out->data == NULL) {
        plFreeImage(out);
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "couldn't allocate output image buffer");
        return PL_RESULT_MEMORY_ALLOCATION;
    }

    out->data[0] = calloc(out->size, sizeof(uint8_t));
    if(out->data[0] == NULL) {
        plFreeImage(out);
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "couldn't allocate output image buffer");
        return PL_RESULT_MEMORY_ALLOCATION;
    }

    fread(out->data[0], sizeof(uint8_t), out->size, fin);

    out->width = w;
    out->height = h;
    out->format = PL_IMAGEFORMAT_RGB8;

    return PL_RESULT_SUCCESS;
}