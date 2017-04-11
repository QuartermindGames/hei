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

#include <platform_log.h>
#include <platform_image.h>

#define LOG "tim2tiff"

int main(int argc, char **argv) {
    plInitialize(argc, argv, PL_SUBSYSTEM_IMAGE | PL_SUBSYSTEM_LOG);

    PLImage image;
#if 1
    PLresult result = plLoadImage("./images/tim/CRATE4.TIM", &image);
#else
    // do by arg
#endif
    if(result != PL_RESULT_SUCCESS) {
        printf("Failed to load image!\n%s", plGetResultString(result));
        return -1;
    }

#if 1
    char opath[PL_SYSTEM_MAX_PATH] = { '\0' };
    snprintf(opath, sizeof(opath), "./images/out/%d.tif", (int)(time(NULL) % 1000));
    printf("Writing TIFF to %s\n", opath);
    plWriteImage(&image, opath);
#else
    // do by arg
#endif

    _plFreeImage(&image);

    return 0;
}