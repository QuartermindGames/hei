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

#include "package_private.h"

/*  MAD/MTD Format Specification    */
/* The MAD/MTD format is the package format used by
 * Hogs of War to store and index content used by
 * the game.
 *
 * Files within these packages are expected to be in
 * a specific order, as both the game and other assets
 * within the game rely on this order so that they, for
 * example, will know which textures to load in / use.
 *
 * Because of this, any package that's recreated will need
 * to be done so in a way that preserves the original file
 * order.
 *
 * Thanks to solemnwarning for his help on this one!
 */

typedef struct __attribute__((packed)) MADIndex {
    char file[16];

    uint32_t offset;
    uint32_t length;
} MADIndex;

bool _plMADFormatCheck(const char *path, FILE *fin) {
    rewind(fin);

    // fetch the size...
    size_t length = plGetFileSize(path);
    if(plGetFunctionResult() != PL_RESULT_SUCCESS) {
        return false;
    }

    // attempt to read in the first index of the MAD package...
    MADIndex index;
    if(fread(&index, sizeof(MADIndex), 1, fin) != 1) {
        return false;
    }

    // ensure the file name is valid...
    for(unsigned int i = 0; i < 16; ++i) {
        if(isprint(index.file[i]) == 0 && index.file[i] != '\0') {
            return false;
        }
    }

    // check we have a sane offset and length...
    if(index.offset > sizeof(MADIndex) || index.offset >= length || index.length == 0 || index.length > length) {
        return false;
    }

    return true;
}

PLPackage *_plLoadMADPackage(FILE *fin) {

}