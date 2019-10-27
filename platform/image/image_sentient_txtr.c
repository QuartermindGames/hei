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

#include "image_private.h"

/* Sentient's TXTR format */

bool plSentientTxtrFormatCheck(PLFile* ptr) {
  plRewindFile(ptr);

  struct {
    char          ident[4]; // RTXT
    unsigned int  length;   // Length of chunk
  } header;
  if (plReadFile(ptr, &header, sizeof(header), 1) != 1) {
    ReportError(PL_RESULT_INVALID_PARM1, "failed to read image header");
    return false;
  }

  if(strncmp("RTXT", header.ident, 4) != 0) {
    ReportError(PL_RESULT_INVALID_PARM1, "invalid identifer, %s vs RTXT", header.ident);
    return false;
  }

  if(header.length > plGetFileSize(ptr)) {
    ReportError(PL_RESULT_INVALID_PARM1, "invalid size indicated by header");
    return false;
  }

  return true;
}

bool plLoadSentientTxtrImage(PLFile* ptr, PLImage* out) {
  plRewindFile(ptr);
  plFileSeek(ptr, 4, PL_SEEK_CUR);

  bool status;
  unsigned int length = plReadInt32(ptr, false, &status);
  if(!status) {
    ReportError(PL_RESULT_INVALID_PARM1, "failed to read length from header");
    return false;
  }

  // HACK HACK HACK...
  unsigned int wh = sqrt(length / 3);  //plRoundUp(sqrt(length / 3), 2);
  if(!plIsPowerOfTwo(wh)) {
    if(wh < 100) {
      wh = plRoundUp(sqrt(length / 3), 2);
    }
  }

  out->width = out->height = wh;
  out->colour_format = PL_COLOURFORMAT_RGB;
  out->format = PL_IMAGEFORMAT_RGB8;
  out->size = plGetImageSize(out->format, out->width, out->height);
  out->levels = 1;
  out->data = pl_calloc(out->levels, sizeof(uint8_t*));
  out->data[0] = pl_calloc(out->size, sizeof(uint8_t));
  plReadFile(ptr, out->data[0], 1, out->size);
  return true;
}
