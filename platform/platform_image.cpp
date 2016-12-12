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
#include "platform_filesystem.h"

PLresult plLoadImage(const PLchar *path, PLImage *out)
{
    plFunctionStart();

    if (!plIsValidString(path))
        return PL_RESULT_FILEPATH;

    // Xenon uses a lot of long extensions, as do some other modern
    // applications, so that's why we're using a size 16.
    const PLchar *extension = plGetFileExtension(path);
    if (!plIsValidString(extension))
    {
        // This is the slowest loader type, now we need to take a stab
        // at which format this file potentially is using some trickery
        // but it's useful for cases in which we don't care so much about
        // the type of file we're loading.

        /* Scan directory
         * Find first file with extension
         * Load file, if fails, load next with extension
         * Failed? Okay, we give up!
         */

        if (plFileExists(path))
        {
            // Apparently it exists without an extension... Ho boy...


        }

        std::string full_name = path;
    }

    FILE *fin = fopen(path, "rb");
    if(!fin)
        return PL_RESULT_FILEREAD;

    PLresult result = PL_RESULT_FILETYPE;
    if (!strncmp(extension, PLIMAGE_EXTENSION_DTX, 3))       result = plLoadDTXImage(fin, out);
    else if (!strncmp(extension, PLIMAGE_EXTENSION_FTX, 3))  result = plLoadFTXImage(fin, out);
    else if (!strncmp(extension, PLIMAGE_EXTENSION_VTF, 3))  result = plLoadVTFImage(fin, out);
    else if (!strncmp(extension, PLIMAGE_EXTENSION_PPM, 3))  result = plLoadPPMImage(fin, out);

    fclose(fin);

    return result;
}

void plFreeImage(PLImage *image) {
    plFunctionStart();

    if (!image || !image->data)
        return;

    delete[] image->data;

    plFunctionEnd();
}

PLbool plIsValidImageSize(PLuint width, PLuint height) {
    plFunctionStart();

    if((width < 8) || (height < 8))
        return false;
    else if(!plIsPowerOfTwo(width) || !plIsPowerOfTwo(height))
        return false;

    return true;

    plFunctionEnd();
}
