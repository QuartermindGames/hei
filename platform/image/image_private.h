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

#ifndef PL_NEW_IMAGE_SUBSYSTEM
bool plDDSFormatCheck(FILE *fin);
bool plDTXFormatCheck(FILE *fin);
bool plVTFFormatCheck(FILE *fin);
bool plTIMFormatCheck(FILE *fin);
bool BMPFormatCheck(FILE *fin);
bool plSWLFormatCheck(FILE *fin);

bool LoadFTXImage(FILE *fin, PLImage *out);         // Ritual's FTX image format.
bool LoadPPMImage(FILE *fin, PLImage *out);         // Portable Pixel Map format.
bool plLoadDTXImage(FILE *fin, PLImage *out);         // Lithtech's DTX image format.
bool plLoadVTFImage(FILE *fin, PLImage *out);         // Valve's VTF image format.
bool plLoadDDSImage(FILE *fin, PLImage *out);
bool plLoadTIMImage(FILE *fin, PLImage *out);         // Sony's TIM image format.
bool LoadBMPImage(FILE *fin, PLImage *out);
bool plLoadSWLImage(FILE *fin, PLImage *out);       // Ritual's SWL image format.
#else
bool plLoadSWLImage(uint8_t *data, size_t length, PLImage *out);
bool plSWLFormatCheck(uint8_t *data, size_t length);

bool plLoadFTXImage(uint8_t *data, size_t length, PLImage *out);
#endif
