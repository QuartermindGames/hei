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
bool LoadSTBImage(FILE *fin, PLImage *out) {
    rewind(fin);

    int x, y, component;
    unsigned char *data = stbi_load_from_file(fin, &x, &y, &component, 4);
    if(data == NULL) {
        ReportError(PL_RESULT_FILEREAD, "failed to read in image, %s", stbi_failure_reason());
        return false;
    }

    memset(out, 0, sizeof(PLImage));
    out->colour_format  = PL_COLOURFORMAT_RGBA;
    out->format         = PL_IMAGEFORMAT_RGBA8;
    out->width          = (unsigned int)x;
    out->height         = (unsigned int)y;
    out->size           = plGetImageSize(out->format, out->width, out->height);
    out->levels         = 1;

    out->data = calloc(out->levels, sizeof(uint8_t*));
    if(out->data != NULL) {
        out->data[0] = calloc(out->size, sizeof(uint8_t));
        if(out->data[0] != NULL) {
            out->data[0] = data;
            return true;
        }
        free(out->data);
    }
    free(data);

    ReportError(PL_RESULT_MEMORY_ALLOCATION, "failed to allocate memory for image buffer");
    return false;
}
#endif

bool plLoadImagef(FILE *fin, const char *path, PLImage *out) {
    if(fin == NULL) {
        ReportError(PL_RESULT_FILEREAD, "invalid file handle");
        return false;
    }

    strncpy(out->path, path, sizeof(out->path));

    if(DDSFormatCheck(fin) && LoadDDSImage(fin, out)) {
        return true;
    } else if(TIMFormatCheck(fin) && LoadTIMImage(fin, out)) {
        return true;
    } else if(VTFFormatCheck(fin) && LoadVTFImage(fin, out)) {
        return true;
    } else if(DTXFormatCheck(fin) && LoadDTXImage(fin, out)) {
        return true;
    } else {
        const char *extension = plGetFileExtension(path);
        if(extension && extension[0] != '\0') {
            if (!strncmp(extension, PL_EXTENSION_FTX, 3) && LoadFTXImage(fin, out)) {
                return true;
            } else if (!strncmp(extension, PL_EXTENSION_PPM, 3) && LoadPPMImage(fin, out)) {
                return true;
            }
        }
    }

#if defined(STB_IMAGE_IMPLEMENTATION)
    if(LoadSTBImage(fin, out)) {
        return true;
    }
#endif

    ReportError(PL_RESULT_FILETYPE, "unrecognised image type");
    return false;
}

bool plLoadImage(const char *path, PLImage *out) {
    if (plIsEmptyString(path)) {
        ReportError(PL_RESULT_FILEPATH, "invalid path (%s) passed for image", path);
        return false;
    }

    FILE *fin = fopen(path, "rb");
    if(fin == NULL) {
        ReportError(PL_RESULT_FILEREAD, "failed to load image (%s) (%s)", path, strerror(errno));
        return false;
    }

    if(strrchr(path, ':')) {
        // Very likely a packaged image.
        // example/package.wad:myimage
    }

    bool result = plLoadImagef(fin, path, out);

    fclose(fin);

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
        if (!strncmp(extension, PL_EXTENSION_BMP, 3)) {
            if(stbi_write_bmp(path, image->width, image->height, comp, image->data[0]) == 1) {
                return true;
            }
        } else if(!strncmp(extension, PL_EXTENSION_PNG, 3)) {
            if(stbi_write_png(path, image->width, image->height, comp, image->data[0], 0) == 1) {
                return true;
            }
        } else if(!strncmp(extension, PL_EXTENSION_TGA, 3)) {
            if(stbi_write_tga(path, image->width, image->height, comp, image->data[0]) == 1) {
                return true;
            }
        } else if(!strncmp(extension, PL_EXTENSION_JPG, 3)) {
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

    /* TODO: Make this thing more extensible... */
    if(image->format == PL_IMAGEFORMAT_RGB5A1 && new_format == PL_IMAGEFORMAT_RGBA8) {
        uint8_t *levels[image->levels];
        memset(levels, 0, image->levels * sizeof(uint8_t*));

        /* Make a new copy of each detail level in the new format. */

        unsigned int lw = image->width;
        unsigned int lh = image->height;

        for(unsigned int l = 0; l < image->levels; ++l) {
            levels[l] = _plImageDataRGB5A1toRGBA8(image->data[l], lw * lh);
            if(levels[l] == NULL) {
                /* Memory allocation failed, ditch any buffers we've created so far. */
                for(unsigned int m = 0; m < image->levels; ++m) {
                    free(levels[m]);
                }

                ReportError(PL_RESULT_MEMORY_ALLOCATION, "Couldn't allocate memory for image data");
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

    ReportError(PL_RESULT_IMAGEFORMAT, "Unsupported image format conversion");
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
 * of one byte, returns ZERO.
*/
unsigned int _plImageBytesPerPixel(PLImageFormat format) {
    switch(format) {
        case PL_IMAGEFORMAT_RGBA4:   return 2;
        case PL_IMAGEFORMAT_RGB5A1:  return 2;
        case PL_IMAGEFORMAT_RGB565:  return 2;
        case PL_IMAGEFORMAT_RGB8:    return 3;
        case PL_IMAGEFORMAT_RGBA8:   return 4;
        case PL_IMAGEFORMAT_RGBA12:  return 6;
        case PL_IMAGEFORMAT_RGBA16:  return 8;
        case PL_IMAGEFORMAT_RGBA16F: return 8;

        default:
            return 0;
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

        default: {
            /* todo, log warning message */
        } break;
    }
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

        default: {
            /* todo, log warning message */
        } break;
    }
}

void _plAllocateImage(PLImage *image, PLuint size, PLuint levels) {
    image->data = (uint8_t**)calloc(levels, sizeof(uint8_t));
}

void plFreeImage(PLImage *image) {
    plFunctionStart();

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

bool plIsValidImageSize(unsigned int width, unsigned int height) {
    if(((width < 2) || (height < 2)) || (!plIsPowerOfTwo(width) || !plIsPowerOfTwo(height))) {
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

uint8_t *_plImageDataRGB5A1toRGBA8(const uint8_t *src, size_t n_pixels) {
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
	
	unsigned int bytes_per_pixel = _plImageBytesPerPixel(image->format);
	if(bytes_per_pixel == 0) {
		ReportError(PL_RESULT_IMAGEFORMAT, "Cannot flip images in this format");
		return false;
	}
	
	unsigned int bytes_per_row = width * bytes_per_pixel;
	
	unsigned char *swap = pl_malloc(bytes_per_row);
	if(swap == NULL) {
		ReportError(PL_RESULT_MEMORY_ALLOCATION, "Cannot allocate memory");
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
