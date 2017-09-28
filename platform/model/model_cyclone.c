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

#include "model_private.h"

enum {
    MDL_FLAG_FLAT   = (1 << 0),
    MDL_FLAG_UNLIT  = (1 << 1),
};

#define MAX_MODEL_NAME      128
#define MAX_TEXTURE_NAME    64

typedef struct __attribute__((packed)) MDLVertex {
    uint8_t unknown0[2];
    int8_t x_;
    int8_t x;
    uint8_t unknown1[2];
    int8_t y_;
    int8_t y;
    uint8_t unknown2[2];
    int8_t z_;
    int8_t z;
} MDLVertex;

typedef struct MDLFace {
    uint8_t num_indices;
    uint16_t indices[5];
} MDLFace;

FILE *_plLoadRequiemModel(const char *path) {
    FILE *file = fopen(path, "rb");
    if(file == NULL) {
        _plReportError(PL_RESULT_FILEREAD, plGetResultString(PL_RESULT_FILEREAD));
        return NULL;
    }
}

PLStaticModel *_plLoadStaticRequiemModel(const char *path) {
    FILE *file = _plLoadRequiemModel(path);
    if(file == NULL) {
        return NULL;
    }


}