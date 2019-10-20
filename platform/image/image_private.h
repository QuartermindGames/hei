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
#pragma once

#include "platform_private.h"

#include <PL/platform_image.h>

//#define PL_NEW_IMAGE_SUBSYSTEM

bool plDDSFormatCheck(PLFile *fin);
bool plDTXFormatCheck(PLFile *fin);
bool plVTFFormatCheck(PLFile *fin);
bool plTIMFormatCheck(PLFile *fin);
bool plSWLFormatCheck(PLFile *fin);

bool plLoadFtxImage(PLFile *fin, PLImage *out);         // Ritual's FTX image format.
bool plLoadDTXImage(PLFile *fin, PLImage *out);         // Lithtech's DTX image format.
bool plLoadVTFImage(PLFile *fin, PLImage *out);         // Valve's VTF image format.
bool plLoadDDSImage(PLFile *fin, PLImage *out);
bool plLoadTIMImage(PLFile *fin, PLImage *out);         // Sony's TIM image format.
bool plLoadSWLImage(PLFile *fin, PLImage *out);       // Ritual's SWL image format.
