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

#include <PL/platform_image.h>
#include <PL/platform_filesystem.h>
#include <PL/platform_math.h>

#include <errno.h>

#include "image_private.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(STB_IMAGE_WRITE_IMPLEMENTATION)
#   include "stb_image_write.h"
#endif

#define STB_IMAGE_IMPLEMENTATION
#if defined(STB_IMAGE_IMPLEMENTATION)
#   include "stb_image.h"

static bool LoadSTBImage(uint8_t *data, int x, int y, int component, PLImage *out) {
    memset(out, 0, sizeof(PLImage));
    out->colour_format  = PL_COLOURFORMAT_RGBA;
    out->format         = PL_IMAGEFORMAT_RGBA8;
    out->width          = (unsigned int)x;
    out->height         = (unsigned int)y;
    out->size           = plGetImageSize(out->format, out->width, out->height);
    out->levels         = 1;

    out->data = pl_calloc(out->levels, sizeof(uint8_t*));
    if(out->data != NULL) {
        out->data[0] = pl_calloc(out->size, sizeof(uint8_t));
        if(out->data[0] != NULL) {
            out->data[0] = data;
            return true;
        }
        free(out->data);
    }
    free(data);

    return false;
}

static bool LoadSTBImageFromFile(FILE *fp, PLImage *out) {
    rewind(fp);

    int x, y, component;
    uint8_t *data = stbi_load_from_file(fp, &x, &y, &component, 4);
    if(data == NULL) {
        ReportError(PL_RESULT_FILEREAD, "failed to read in image (%s)", stbi_failure_reason());
        return false;
    }

    return LoadSTBImage(data, x, y, component, out);
}

static bool LoadSTBImageFromMemory(const uint8_t *buffer, size_t length, PLImage *out) {
    int x, y, component;
    unsigned char *data = stbi_load_from_memory(buffer, (int) length, &x, &y, &component, 4);
    if(data == NULL) {
        ReportError(PL_RESULT_FILEREAD, "failed to read in image (%s)", stbi_failure_reason());
        return false;
    }

    return LoadSTBImage(data, x, y, component, out);
}

#endif

#ifdef PL_NEW_IMAGE_SUBSYSTEM

typedef struct PLImageLoader {
    const char  *ext[32];   /* file extensions */

    bool(*CheckFormat)(uint8_t *data, size_t length);
    bool(*LoadImage)(uint8_t *data, size_t length, PLImage *out);
    bool(*WriteImage)(PLImage *in, const char *dest);
} PLImageLoader;

PLImageLoader image_formats[]={
        { { "swl" }, plSWLFormatCheck, plLoadSWLImage },
        { { "ftx" }, NULL, plLoadFTXImage },
};

bool plNewLoadImage(const char *ext, uint8_t *data, size_t length, PLImage *out) {
    for(unsigned int i = 0; i < plArrayElements(image_formats); ++i) {
        PLImageLoader *loader = NULL;
        for(unsigned int j = 0; j < 32; j++) {
            if(pl_strncasecmp(ext, image_formats[i].ext[j], sizeof(ext)) == 0) {
                loader = &image_formats[i];
                break;
            }
        }

        if(loader == NULL) {
            SetResult(PL_RESULT_IMAGEFORMAT);
            continue;
        }

        if(loader->CheckFormat && !loader->CheckFormat(data, length)) {
            /* CheckFormat sets result for us */
            continue;
        }

        if(!loader->LoadImage(data, length, out)) {
            /* LoadImage sets result for us */
            continue;
        }
    }

    /* check if it's been successfully loaded... for now
     * we'll go by levels, but maybe we should do this
     * differently? */
    if(out->levels == 0) {
        return false;
    }

    return true;
}

#endif /* PL_NEW_IMAGE_SUBSYSTEM */

PLImage *plCreateImage(uint8_t *buf, unsigned int w, unsigned int h, PLColourFormat col, PLImageFormat dat) {
    PLImage *image = pl_malloc(sizeof(PLImage));
    if(image == NULL) {
        return NULL;
    }

    image->width             = w;
    image->height            = h;
    image->colour_format     = col;
    image->format            = dat;
    image->size              = plGetImageSize(image->format, image->width, image->height);
    image->levels            = 1;

    image->data = pl_calloc(image->levels, sizeof(uint8_t*));
    if(image->data == NULL) {
        plDestroyImage(image);
        return NULL;
    }

    image->data[0] = pl_calloc(image->size, sizeof(uint8_t));
    if(image->data[0] == NULL) {
        plDestroyImage(image);
        return NULL;
    }

    if(buf != NULL) {
        memcpy(image->data[0], buf, image->size);
    }

    return image;
}

void plDestroyImage(PLImage *image) {
    if(image == NULL) {
        return;
    }

    plFreeImage(image);
    free(image);
}

static bool LoadImageFromFile(FILE *fin, const char *path, PLImage *out) {
#if defined(STB_IMAGE_IMPLEMENTATION)
  if(LoadSTBImageFromFile(fin, out)) {
    return true;
  }
#endif

  if(plDDSFormatCheck(fin) && plLoadDDSImage(fin, out)) {
    return true;
  } else if(plSWLFormatCheck(fin) && plLoadSWLImage(fin, out)) {
    return true;
  } else if(plTIMFormatCheck(fin) && plLoadTIMImage(fin, out)) {
    return true;
  } else if(plVTFFormatCheck(fin) && plLoadVTFImage(fin, out)) {
    return true;
  } else if(plDTXFormatCheck(fin) && plLoadDTXImage(fin, out)) {
    return true;
  } else {
    const char *extension = plGetFileExtension(path);
    if(extension && extension[0] != '\0') {
      if (!strncmp(extension, "ftx", 3) && LoadFTXImage(fin, out)) {
        return true;
      } else if (!strncmp(extension, "ppm", 3) && LoadPPMImage(fin, out)) {
        return true;
      }
    }
  }

  return false;
}

bool plLoadImageFromFile(FILE *fin, const char *path, PLImage *out) {
    if(fin == NULL) {
        ReportError(PL_RESULT_FILEREAD, "invalid file handle");
        return false;
    }

    if(LoadImageFromFile(fin, path, out)) {
      strncpy(out->path, path, sizeof(out->path));
      return true;
    }

    /* if we reached here, and our status is still successful, that's clearly
     * not correct... because we're here! we haven't loaded anything! */
    if(plGetFunctionResult() == PL_RESULT_SUCCESS) {
        ReportError(PL_RESULT_FILETYPE, "unrecognised image type (%s)", path);
    }

    return false;
}

bool plLoadImageFromMemory(const uint8_t *data, size_t length, const char *type, PLImage *out) {
    if(data == NULL || length == 0) {
        ReportError(PL_RESULT_MEMORY_EOA, "invalid data buffer for image (%s) (%d)", type, length);
        return false;
    }

    if(LoadSTBImageFromMemory(data, length, out)) {
        return true;
    }

    /* todo: need to overhaul loaders for this to work everywhere else... */

    /* if we reached here, and our status is still successful, that's clearly
     * not correct... because we're here! we haven't loaded anything! */
    if(plGetFunctionResult() == PL_RESULT_SUCCESS) {
        ReportError(PL_RESULT_IMAGEFORMAT, "unrecognised image type");
    }

    return false;
}

bool plLoadImage(const char *path, PLImage *out) {
    memset(out, 0, sizeof(PLImage));

    if (plIsEmptyString(path)) {
        ReportError(PL_RESULT_FILEPATH, "invalid path (%s) passed for image", path);
        return false;
    }

#ifdef PL_NEW_IMAGE_SUBSYSTEM /* wip future implementation */
    PLIOBuffer buffer;
    plLoadFile(path, &buffer);

    const char *ext = plGetFileExtension(path);

    plLoadFromMemory(buffer.data, buffer.size, ext, out);
#else
    FILE *fin = fopen(path, "rb");
    if(fin == NULL) {
        ReportError(PL_RESULT_FILEREAD, "failed to load image (%s) (%s)", path, strerror(errno));
        return false;
    }

    if(strrchr(path, ':')) {
        // Very likely a packaged image.
        // example/package.wad:myimage
    }

    bool result = plLoadImageFromFile(fin, path, out);

    fclose(fin);
#endif /* PL_NEW_IMAGE_SUBSYSTEM */

    return result;
}

bool plWriteImage(const PLImage *image, const char *path) {
    if (plIsEmptyString(path)) {
        ReportError(PL_RESULT_FILEPATH, plGetResultString(PL_RESULT_FILEPATH));
        return false;
    }

    unsigned int comp = plGetSamplesPerPixel(image->colour_format);
    if(comp == 0) {
        ReportError(PL_RESULT_IMAGEFORMAT, "invalid colour format");
        return false;
    }

    const char *extension = plGetFileExtension(path);
    if(!plIsEmptyString(extension)) {
        if (!pl_strncasecmp(extension, "bmp", 3)) {
            if(stbi_write_bmp(path, image->width, image->height, comp, image->data[0]) == 1) {
                return true;
            }
        } else if(!pl_strncasecmp(extension, "png", 3)) {
            if(stbi_write_png(path, image->width, image->height, comp, image->data[0], 0) == 1) {
                return true;
            }
        } else if(!pl_strncasecmp(extension, "tga", 3)) {
            if(stbi_write_tga(path, image->width, image->height, comp, image->data[0]) == 1) {
                return true;
            }
        } else if(!pl_strncasecmp(extension, "jpg", 3) || !pl_strncasecmp(extension, "jpeg", 3)) {
            if(stbi_write_jpg(path, image->width, image->height, comp, image->data[0], 90) == 1) {
                return true;
            }
        }
    }

    ReportError(PL_RESULT_FILETYPE, plGetResultString(PL_RESULT_FILETYPE));
    return false;
}

// Returns the number of samples per-pixel depending on the colour format.
unsigned int plGetSamplesPerPixel(PLColourFormat format) {
    switch(format) {
        case PL_COLOURFORMAT_ABGR:
        case PL_COLOURFORMAT_ARGB:
        case PL_COLOURFORMAT_BGRA:
        case PL_COLOURFORMAT_RGBA: {
            return 4;
        }

        case PL_COLOURFORMAT_BGR:
        case PL_COLOURFORMAT_RGB: {
            return 3;
        }
    }

    return 0;
}

bool plConvertPixelFormat(PLImage *image, PLImageFormat new_format) {
    if(image->format == new_format) {
        return true;
    }

    switch(image->format) {
        case PL_IMAGEFORMAT_RGB5A1: {
            if(new_format == PL_IMAGEFORMAT_RGBA8) {
                uint8_t *levels[image->levels];
                memset(levels, 0, image->levels * sizeof(uint8_t*));

                /* Make a new copy of each detail level in the new format. */

                unsigned int lw = image->width;
                unsigned int lh = image->height;

                for(unsigned int l = 0; l < image->levels; ++l) {
                    levels[l] = plImageDataRGB5A1toRGBA8(image->data[l], lw * lh);
                    if(levels[l] == NULL) {
                        /* Memory allocation failed, ditch any buffers we've created so far. */
                        for(unsigned int m = 0; m < image->levels; ++m) {
                            free(levels[m]);
                        }

                        ReportError(PL_RESULT_MEMORY_ALLOCATION, "couldn't allocate memory for image data");
                        return false;
                    }

                    lw /= 2;
                    lh /= 2;
                }

                /* Now that all levels have been converted, free and replace the old buffers. */

                for(unsigned int l = 0; l < image->levels; ++l) {
                    free(image->data[l]);
                    image->data[l] = levels[l];
                }

                image->format = new_format;
                /* TODO: Update colour_format */

                return true;
            }
        } break;

        default:break;
    }

    ReportError(PL_RESULT_IMAGEFORMAT, "unsupported image format conversion");
    return false;
}

unsigned int plGetImageSize(PLImageFormat format, unsigned int width, unsigned int height) {
    switch(format) {
        case PL_IMAGEFORMAT_RGB_DXT1:   return (width * height) >> 1;
        case PL_IMAGEFORMAT_RGBA_DXT1:  return width * height * 4;
        case PL_IMAGEFORMAT_RGBA_DXT3:
        case PL_IMAGEFORMAT_RGBA_DXT5:  return width * height;

        case PL_IMAGEFORMAT_RGB5A1:     return width * height * 2;
        case PL_IMAGEFORMAT_RGB8:
        case PL_IMAGEFORMAT_RGB565:     return width * height * 3;
        case PL_IMAGEFORMAT_RGBA4:
        case PL_IMAGEFORMAT_RGBA8:      return width * height * 4;
        case PL_IMAGEFORMAT_RGBA16F:
        case PL_IMAGEFORMAT_RGBA16:     return width * height * 8;

        default:    return 0;
    }
}

/* Returns the number of BYTES per pixel for the given PLImageFormat.
 *
 * If the format doesn't have a predictable size or the size isn't a multiple
 * of one byte, returns ZERO. */
unsigned int plImageBytesPerPixel(PLImageFormat format) {
    switch(format) {
        case PL_IMAGEFORMAT_RGBA4:   return 2;
        case PL_IMAGEFORMAT_RGB5A1:  return 2;
        case PL_IMAGEFORMAT_RGB565:  return 2;
        case PL_IMAGEFORMAT_RGB8:    return 3;
        case PL_IMAGEFORMAT_RGBA8:   return 4;
        case PL_IMAGEFORMAT_RGBA12:  return 6;
        case PL_IMAGEFORMAT_RGBA16:  return 8;
        case PL_IMAGEFORMAT_RGBA16F: return 8;
        default:                     return 0;
    }
}

void plInvertImageColour(PLImage *image) {
    unsigned int num_colours = plGetSamplesPerPixel(image->colour_format);
    switch(image->format) {
        case PL_IMAGEFORMAT_RGB8:
        case PL_IMAGEFORMAT_RGBA8: {
            for(unsigned int i = 0; i < image->size; i += num_colours) {
                uint8_t *pixel = &image->data[0][i];
                pixel[0] = ~pixel[0];
                pixel[1] = ~pixel[1];
                pixel[2] = ~pixel[2];
            }
        } break;

        default:break;
    }

    ReportError(PL_RESULT_IMAGEFORMAT, "unsupported image format");
}

/* utility function */
void plGenerateStipplePattern(PLImage *image, unsigned int depth) {
#if 0
    unsigned int p = 0;
    unsigned int num_colours = plGetSamplesPerPixel(image->colour_format);
    switch(image->format) {
        case PL_IMAGEFORMAT_RGB8: break;
        case PL_IMAGEFORMAT_RGBA8: {
            for(unsigned int i = 0; i < image->size; i += num_colours) {
                uint8_t *pixel = &image->data[0][i];
                if(num_colours == 4) {
                    //if()
                }
            }
        } break;

        default:break;
    }

    ReportError(PL_RESULT_IMAGEFORMAT, "unsupported image format");
#else
    /* todo */
#endif
}

void plReplaceImageColour(PLImage *image, PLColour target, PLColour dest) {
    unsigned int num_colours = plGetSamplesPerPixel(image->colour_format);
    switch(image->format) {
        case PL_IMAGEFORMAT_RGB8:
        case PL_IMAGEFORMAT_RGBA8: {
            for(unsigned int i = 0; i < image->size; i += num_colours) {
                uint8_t *pixel = &image->data[0][i];
                if(num_colours == 4) {
                    if(plCompareColour(PLColour(pixel[0], pixel[1], pixel[2], pixel[3]), target)) {
                        pixel[0] = dest.r;
                        pixel[1] = dest.g;
                        pixel[2] = dest.b;
                        pixel[3] = dest.a;
                    }
                } else {
                    if(plCompareColour(PLColourRGB(pixel[0], pixel[1], pixel[2]), target)) {
                        pixel[0] = dest.r;
                        pixel[1] = dest.g;
                        pixel[2] = dest.b;
                    }
                }
            }
        } break;

        default:break;
    }

    ReportError(PL_RESULT_IMAGEFORMAT, "unsupported image format");
}

#if 0 /* unused */
void _plAllocateImage(PLImage *image, unsigned int size, unsigned int levels) {
    image->data = (uint8_t**)pl_calloc(levels, sizeof(uint8_t));
}
#endif

void plFreeImage(PLImage *image) {
    FunctionStart();

    if (image == NULL || image->data == NULL) {
        return;
    }

    for(unsigned int levels = 0; levels < image->levels; ++levels) {
        if(image->data[levels] == NULL) {
            continue;
        }

        free(image->data[levels]);
        image->data[levels] = NULL;
    }

    free(image->data);
    image->data = NULL;
}

bool plImageIsPowerOfTwo(unsigned int width, unsigned int height) {
    if(((width == 0) || (height == 0)) || (!plIsPowerOfTwo(width) || !plIsPowerOfTwo(height))) {
        return false;
    }

    return true;
}

bool plIsCompressedImageFormat(PLImageFormat format) {
    switch (format) {
        default:    return false;
        case PL_IMAGEFORMAT_RGBA_DXT1:
        case PL_IMAGEFORMAT_RGBA_DXT3:
        case PL_IMAGEFORMAT_RGBA_DXT5:
        case PL_IMAGEFORMAT_RGB_DXT1:
        case PL_IMAGEFORMAT_RGB_FXT1:
            return true;
    }
}

#define scale_5to8(i) ((((double)(i)) / 31) * 255)

uint8_t *plImageDataRGB5A1toRGBA8(const uint8_t *src, size_t n_pixels) {
    uint8_t *dst = pl_malloc(n_pixels * 4);
    if(dst == NULL) {
        return NULL;
    }

    uint8_t *dp = dst;

    for(size_t i = 0; i < n_pixels; ++i) {
        /* Red */
        *(dp++) = scale_5to8((src[0] & 0xF8) >> 3);

        /* Green */
        *(dp++) = scale_5to8(((src[0] & 0x07) << 2) | ((src[1] & 0xC0) >> 6));

        /* Blue */
        *(dp++) = scale_5to8((src[1] & 0x3E) >> 1);

        /* Alpha */
        *(dp++) = (src[1] & 0x01) ? 255 : 0;

        src += 2;
    }

    return dst;
}

bool plFlipImageVertical(PLImage *image) {
	unsigned int width  = image->width;
	unsigned int height = image->height;
	
	unsigned int bytes_per_pixel = plImageBytesPerPixel(image->format);
	if(bytes_per_pixel == 0) {
		ReportError(PL_RESULT_IMAGEFORMAT, "cannot flip images in this format");
		return false;
	}
	
	unsigned int bytes_per_row = width * bytes_per_pixel;
	
	unsigned char *swap = pl_malloc(bytes_per_row);
	if(swap == NULL) {
		return false;
	}
	
	for(unsigned int l = 0; l < image->levels; ++l) {
		for(unsigned int r = 0; r < height / 2; ++r) {
			unsigned char *tr = image->data[l] + (r * bytes_per_row);
			unsigned char *br = image->data[l] + (((height - 1) - r) * bytes_per_row);
			
			memcpy(swap, tr, bytes_per_row);
			memcpy(tr, br,   bytes_per_row);
			memcpy(br, swap, bytes_per_row);
		}
		
		bytes_per_row /= 2;
		height        /= 2;
	}
	
	free(swap);
	
	return true;
}
