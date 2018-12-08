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

#include <PL/platform_image.h>
#include "platform_private.h"

/* Ritual Entertainment's SWL Format, used by SiN */

typedef struct SWLHeader {
    char        path[64];   /* file path, kinda useless for us */
    uint32_t    width;
    uint32_t    height;
} SWLHeader;

bool plSWLFormatCheck(FILE *fin) {
    rewind(fin);

    SWLHeader header;
    if(fread(&header, sizeof(header), 1, fin) != 1) {
        SetResult(PL_RESULT_FILEREAD);
        return false;
    }

    debug_printf(
            "path: %s\n"
            "w   : %d\n"
            "h   : %d\n"
            ,

            header.path,
            header.width,
            header.height
            );

#if 0
    if(pl_strnisalnum(header.path, sizeof(header.path)) == 0) {
        SetResult(PL_RESULT_FILETYPE);
        return false;
    }
#endif

    if(header.width > 1024 || header.width == 0 ||
       header.height > 1024 || header.height == 0) {
        SetResult(PL_RESULT_IMAGERESOLUTION);
        return false;
    }

    if(!plIsValidImageSize(header.width, header.height)) {
        SetResult(PL_RESULT_IMAGERESOLUTION);
        return false;
    }

    return true;
}

bool plLoadSWLImage(FILE *fin, PLImage *out) {
    SWLHeader header;
    if(fread(&header, sizeof(header), 1, fin) != 1) {
        SetResult(PL_RESULT_FILEREAD);
        return false;
    }

    memset(out, 0, sizeof(PLImage));
    out->width  = header.width;
    out->height = header.height;

    static const unsigned int palette_size = 1024;
    unsigned char palette[palette_size];
    if(fread(palette, sizeof(unsigned char), 1024, fin) != 1024) {
        SetResult(PL_RESULT_FILEREAD);
        return false;
    }

    for(unsigned int i = 0; i < 1024; ++i) {
        debug_printf("%d ", palette[i]);
    } debug_printf("\n");

    /* according to sources, this is a collection of misc data that's
     * specific to SiN itself. */
    if(fseek(fin, 0x4D4, SEEK_SET) != 0) {
        SetResult(PL_RESULT_FILEREAD);
        return false;
    }

    out->levels = 4;
    out->data = pl_calloc(out->levels, sizeof(uint8_t*));
    if(out->data == NULL) {
        plFreeImage(out);
        SetResult(PL_RESULT_MEMORY_ALLOCATION);
        return false;
    }

    out->colour_format  = PL_COLOURFORMAT_RGB;
    out->format         = PL_IMAGEFORMAT_RGB8;
    out->size           = plGetImageSize(out->format, out->width, out->height);

    for(unsigned int i = 0; i < out->levels; ++i) {
        unsigned int mip_w = out->width >> (i + 1);
        unsigned int mip_h = out->height >> (i + 1);
        out->data[0] = pl_calloc(out->size, sizeof(uint8_t));
        if(out->data[0] == NULL) {
            plFreeImage(out);
            SetResult(PL_RESULT_MEMORY_ALLOCATION);
            return false;
        }
    }

//    out->data

    return true;
}
