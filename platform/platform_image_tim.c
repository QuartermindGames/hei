
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

#include "platform_image.h"

/*  http://rewiki.regengedanken.de/wiki/.TIM
 *  https://mrclick.zophar.net/TilEd/download/timgfx.txt
 */

typedef struct TIMHeader {
    uint32_t type;
    uint32_t offset;
} TIMHeader;

typedef struct TIMHeader4 {
    uint16_t palette_org_x;
    uint16_t palette_org_y;
    uint16_t palette_colours;
    uint16_t num_palettes;
} TIMHeader4;

typedef struct TIMImageInfo {
    uint16_t org_x;
    uint16_t org_y;
    uint16_t width;
    uint16_t height;
} TIMImageInfo;

enum TIMType {
    TIM_TYPE_4BPP   = 0x08,
    TIM_TYPE_8BPP   = 0x09,
    TIM_TYPE_16BPP  = 0x02,
    TIM_TYPE_24BPP  = 0x03,
};

#define TIM_IDENT   16

PLbool _plTIMFormatCheck(FILE *fin) {
    rewind(fin);

    uint32_t ident;
    if(fread(&ident, sizeof(uint32_t), 1, fin) != 1) {
        return false;
    }

    return (PLbool)(ident == TIM_IDENT);
}

PLresult _plLoadTIMImage(FILE *fin, PLImage *out) {
    plFunctionStart();

    TIMHeader header;
    if (fread(&header, sizeof(TIMHeader), 1, fin) != 1) {
        return PL_RESULT_FILEREAD;
    }

    memset(out, 0, sizeof(PLImage));

    switch(header.type) {

        case TIM_TYPE_4BPP:
        case TIM_TYPE_8BPP: {
            TIMHeader4 header4;
            if(fread(&header4, sizeof(TIMHeader4), 1, fin) != 1) {
                return PL_RESULT_FILEREAD;
            }

            uint32_t palettes[header4.num_palettes][header4.palette_colours];
            for(PLuint i = 0; i < header4.num_palettes; i++) {
                if(fread(palettes[i], sizeof(uint32_t), header4.palette_colours, fin) != header4.palette_colours) {
                    return PL_RESULT_FILEREAD;
                }
            }

            fseek(fin, 4, SEEK_CUR);

            TIMImageInfo image_info;
            if(fread(&image_info, sizeof(TIMImageInfo), 1, fin) != 1) {
                return PL_RESULT_FILEREAD;
            } else if(!plIsValidImageSize(image_info.width, image_info.height)) {
                return PL_RESULT_IMAGERESOLUTION;
            }

            out->levels = 1;
            // The width of the image depends on its type.
            out->width = (PLuint) (header.type == TIM_TYPE_4BPP ?
                                   (PLuint) (image_info.width * 4) :
                                   (PLuint) (image_info.width * 2));
            out->height = image_info.height;

            PLuint size = (PLuint)(image_info.width * image_info.height / 2);
            out->data = (PLbyte**)calloc(1, sizeof(PLbyte*));
            out->data[0] = (PLbyte*)calloc(size, sizeof(PLbyte));

            PLbyte img[size];
            if(fread(img, sizeof(PLbyte), size, fin) != 1) {
                _plFreeImage(out);
                return PL_RESULT_FILEREAD;
            }

            for(PLuint i = 0; i < size; i++) {
                out->data[0][i] = palettes[0][img[i]];
            }

            out->format = PL_IMAGEFORMAT_RGB5A1;

            break;
        }

        case TIM_TYPE_16BPP:
        case TIM_TYPE_24BPP: {
            TIMImageInfo image_info;

            printf("fuck...\n");

            break;
        }

        default: {
            return PL_RESULT_FILETYPE;
        }
    }

    out->colour_format = PL_COLOURFORMAT_ABGR;

    return PL_RESULT_SUCCESS;
}
