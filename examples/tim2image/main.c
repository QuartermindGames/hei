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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <IL/il.h>
#include <PL/platform_image.h>

int main(int argc, char **argv) {
    plInitialize(argc, argv);

    if(argc != 3) {
        fprintf(stderr, "Usage: %s <input.tim> <output.XXX>\n", argv[0]);
        return 1;
    }

    /* Load the TIM into a PLImage structure. */

    PLImage image;
    bool result = plLoadImage(argv[1], &image);
    if(result != PL_RESULT_SUCCESS) {
        printf("Failed to load TIM image!\n%s", plGetError());
        return 1;
    }

    /* Convert to a pixel format supported by DevIL (RGBA8). */

    assert(plConvertPixelFormat(&image, PL_IMAGEFORMAT_RGBA8));

    /* Write the output image.
     * TODO: Error handling here.
    */

    ilInit();

    ILuint ImageName;
    ilGenImages(1, &ImageName);
    ilBindImage(ImageName);

    ilTexImage(image.width, image.height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, image.data[0]);

    ilEnable(IL_FILE_OVERWRITE);
    ilSaveImage(argv[2]);

    ilShutDown();
    
    plFreeImage(&image);

    return 0;
}