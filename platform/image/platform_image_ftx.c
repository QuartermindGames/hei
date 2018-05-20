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
#include "PL/platform_image.h"
#include "PL/platform_filesystem.h"

/*	Ritual's FTX Format	*/

typedef struct FTXHeader {
    uint32_t width;
    uint32_t height;
    uint32_t alpha;
} FTXHeader;

bool LoadFTXImage(FILE *fin, PLImage *out) {
    FTXHeader header;
    memset(&header, 0, sizeof(FTXHeader));
    header.width = (unsigned int)plGetLittleLong(fin);
    header.height = (unsigned int)plGetLittleLong(fin);
    header.alpha = (unsigned int)plGetLittleLong(fin);

    memset(out, 0, sizeof(PLImage));
    out->size = (unsigned int)(header.width * header.height * 4);
    out->levels = 1;

    out->data = pl_calloc(out->levels, sizeof(uint8_t*));
    if(out->data == NULL) {
        plFreeImage(out);
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "couldn't allocate output image buffer");
        return false;
    }

    out->data[0] = pl_calloc(out->size, sizeof(uint8_t));
    if(out->data[0] == NULL) {
        plFreeImage(out);
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "couldn't allocate output image buffer");
        return false;
    }

    if (fread(out->data[0], sizeof(uint8_t), out->size, fin) != out->size) {
        ReportError(PL_RESULT_FILEREAD, "failed to read image data");
        return false;
    }

    out->format = PL_IMAGEFORMAT_RGBA8;
    out->colour_format = PL_COLOURFORMAT_RGBA;
    out->width = (unsigned int)header.width;
    out->height = (unsigned int)header.height;
    return true;
}