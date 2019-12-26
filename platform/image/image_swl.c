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

/* Ritual Entertainment's SWL Format, used by SiN */

typedef struct SWLHeader {
  char path[64];   /* file path, kinda useless for us */
  uint32_t width;
  uint32_t height;
} SWLHeader;

bool plSWLFormatCheck(PLFile* fin) {
  plRewindFile(fin);

  SWLHeader header;
  if (plReadFile(fin, &header, sizeof(header), 1) != 1) {
    return false;
  }

  plRewindFile(fin);

  if (header.width > 512 || header.width == 0 ||
      header.height > 512 || header.height == 0) {
    SetResult(PL_RESULT_IMAGERESOLUTION);
    return false;
  }

#if 0 /* sadly, isn't always true... */
  if(!plImageIsPowerOfTwo(header.width, header.height)) {
      SetResult(PL_RESULT_IMAGERESOLUTION);
      return false;
  }
#endif

  return true;
}

bool plLoadSWLImage(PLFile* fin, PLImage* out) {
  SWLHeader header;
  if (plReadFile(fin, &header, sizeof(header), 1) != 1) {
    return false;
  }

  memset(out, 0, sizeof(PLImage));

  struct {
    uint8_t r, g, b, a;
  } palette[256];
  if (plReadFile(fin, palette, 4, 256) != 256) {
    SetResult(PL_RESULT_FILEREAD);
    return false;
  }

  /* according to sources, this is a collection of misc data that's
   * specific to SiN itself, so we'll skip it. */
  if (!plFileSeek(fin, 0x4D4, PL_SEEK_SET)) {
    SetResult(PL_RESULT_FILEREAD);
    return false;
  }

  out->width = header.width;
  out->height = header.height;
  out->levels = 4;
  out->colour_format = PL_COLOURFORMAT_RGBA;
  out->format = PL_IMAGEFORMAT_RGBA8;
  out->size = plGetImageSize(out->format, out->width, out->height);

  unsigned int mip_w = out->width;
  unsigned int mip_h = out->height;
  for (unsigned int i = 0; i < out->levels; ++i) {
    if (i > 0) {
      mip_w = out->width >> (i + 1);
      mip_h = out->height >> (i + 1);
    }

    size_t buf_size = mip_w * mip_h;
    uint8_t *buf = pl_malloc(buf_size);
    if (plReadFile(fin, buf, 1, buf_size) != buf_size) {
		pl_free(buf);
      return false;
    }

	out->data = pl_calloc(out->levels, sizeof(uint8_t*));

    size_t level_size = plGetImageSize(out->format, mip_w, mip_h);
    out->data[i] = pl_calloc(level_size, sizeof(uint8_t));

    /* now we fill in the buf we just allocated,
     * by using the palette */
    for (size_t j = 0, k = 0; j < buf_size; ++j, k += 4) {
      out->data[i][k] = palette[buf[j]].r;
      out->data[i][k + 1] = palette[buf[j]].g;
      out->data[i][k + 2] = palette[buf[j]].b;

      /* the alpha channel appears to be used more like
       * a flag to say "yes this texture will be transparent",
       * rather than actual levels of alpha for this pixel.
       *
       * because of that we'll just ignore it */
      out->data[i][k + 3] = 255; /*(uint8_t) (255 - palette[buf[j]].a);*/
    }

	pl_free(buf);
  }

  return true;
}

bool plWriteSWLImage(const PLImage* image, const char* path) {
  /* todo */
  plUnused( path );
  plUnused( image );
  return false;
}
