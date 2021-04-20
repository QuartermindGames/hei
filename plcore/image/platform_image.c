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

#include <PL/platform_filesystem.h>
#include <PL/platform_image.h>
#include <PL/platform_math.h>

#include <errno.h>
#include <filesystem_private.h>

#include "image_private.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined( STB_IMAGE_WRITE_IMPLEMENTATION )
#include "stb_image_write.h"
#endif

#define STB_IMAGE_IMPLEMENTATION
#if defined( STB_IMAGE_IMPLEMENTATION )
#include "stb_image.h"

static PLImage *LoadStbImage( const char *path ) {
	PLFile *file = plOpenFile( path, true );
	if ( file == NULL ) {
		return NULL;
	}

	int x, y, component;
	unsigned char *data = stbi_load_from_memory( file->data, ( int ) file->size, &x, &y, &component, 4 );

	plCloseFile( file );

	if ( data == NULL ) {
		plReportErrorF( PL_RESULT_FILEREAD, "failed to read in image (%s)", stbi_failure_reason() );
		return NULL;
	}

	PLImage *image = pl_calloc( 1, sizeof( PLImage ) );
	image->colour_format = PL_COLOURFORMAT_RGBA;
	image->format = PL_IMAGEFORMAT_RGBA8;
	image->width = ( unsigned int ) x;
	image->height = ( unsigned int ) y;
	image->size = plGetImageSize( image->format, image->width, image->height );
	image->levels = 1;
	image->data = pl_calloc( image->levels, sizeof( uint8_t * ) );
	image->data[ 0 ] = data;

	return image;
}

#endif

#define MAX_IMAGE_LOADERS 4096

typedef struct PLImageLoader {
	const char *extension;
	PLImage *( *LoadImage )( const char *path );
} PLImageLoader;

static PLImageLoader imageLoaders[ MAX_IMAGE_LOADERS ];
static unsigned int numImageLoaders = 0;

void plRegisterImageLoader( const char *extension, PLImage *( *LoadImage )( const char *path ) ) {
	if ( numImageLoaders >= MAX_IMAGE_LOADERS ) {
		plReportBasicError( PL_RESULT_MEMORY_EOA );
		return;
	}

	imageLoaders[ numImageLoaders ].extension = extension;
	imageLoaders[ numImageLoaders ].LoadImage = LoadImage;

	numImageLoaders++;
}

void plRegisterStandardImageLoaders( unsigned int flags ) {
	typedef struct SImageLoader {
		unsigned int flag;
		const char *extension;
		PLImage *( *LoadFunction )( const char *path );
	} SImageLoader;

	static const SImageLoader loaderList[] = {
	        { PL_IMAGE_FILEFORMAT_TGA, "tga", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_PNG, "png", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_JPG, "jpg", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_BMP, "bmp", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_PSD, "psd", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_GIF, "gif", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_HDR, "hdr", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_PIC, "pic", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_PNM, "pnm", LoadStbImage },
	        { PL_IMAGE_FILEFORMAT_FTX, "ftx", plLoadFtxImage },
	        { PL_IMAGE_FILEFORMAT_3DF, "3df", plLoad3dfImage },
	        { PL_IMAGE_FILEFORMAT_TIM, "tim", plLoadTimImage },
	};

	for ( unsigned int i = 0; i < plArrayElements( loaderList ); ++i ) {
		if ( flags != PL_IMAGE_FILEFORMAT_ALL && !( flags & loaderList[ i ].flag ) ) {
			continue;
		}

		plRegisterImageLoader( loaderList[ i ].extension, loaderList[ i ].LoadFunction );
	}
}

void plClearImageLoaders( void ) {
	numImageLoaders = 0;
}

PLImage *plCreateImage( uint8_t *buf, unsigned int w, unsigned int h, PLColourFormat col, PLImageFormat dat ) {
	PLImage *image = pl_malloc( sizeof( PLImage ) );
	if ( image == NULL ) {
		return NULL;
	}

	image->width = w;
	image->height = h;
	image->colour_format = col;
	image->format = dat;
	image->size = plGetImageSize( image->format, image->width, image->height );
	image->levels = 1;

	image->data = pl_calloc( image->levels, sizeof( uint8_t * ) );
	if ( image->data == NULL ) {
		plDestroyImage( image );
		return NULL;
	}

	image->data[ 0 ] = pl_calloc( image->size, sizeof( uint8_t ) );
	if ( image->data[ 0 ] == NULL ) {
		plDestroyImage( image );
		return NULL;
	}

	if ( buf != NULL ) {
		memcpy( image->data[ 0 ], buf, image->size );
	}

	return image;
}

void plDestroyImage( PLImage *image ) {
	if ( image == NULL ) {
		return;
	}

	plFreeImage( image );
	free( image );
}

PLImage *plLoadImage( const char *path ) {
	if ( !plFileExists( path ) ) {
		plReportBasicError( PL_RESULT_FILEPATH );
		return NULL;
	}

	const char *extension = plGetFileExtension( path );
	for ( unsigned int i = 0; i < numImageLoaders; ++i ) {
		if ( pl_strcasecmp( extension, imageLoaders[ i ].extension ) == 0 ) {
			PLImage *image = imageLoaders[ i ].LoadImage( path );
			if ( image != NULL ) {
				strncpy( image->path, path, sizeof( image->path ) );
				return image;
			}
		}
	}

	plReportBasicError( PL_RESULT_UNSUPPORTED );

	return NULL;
}

bool plWriteImage( const PLImage *image, const char *path ) {
	if ( plIsEmptyString( path ) ) {
		plReportErrorF( PL_RESULT_FILEPATH, plGetResultString( PL_RESULT_FILEPATH ) );
		return false;
	}

	int comp = ( int ) plGetNumberOfColourChannels( image->colour_format );
	if ( comp == 0 ) {
		plReportErrorF( PL_RESULT_IMAGEFORMAT, "invalid colour format" );
		return false;
	}

	const char *extension = plGetFileExtension( path );
	if ( !plIsEmptyString( extension ) ) {
		if ( !pl_strncasecmp( extension, "bmp", 3 ) ) {
			if ( stbi_write_bmp( path, ( int ) image->width, ( int ) image->height, comp, image->data[ 0 ] ) == 1 ) {
				return true;
			}
		} else if ( !pl_strncasecmp( extension, "png", 3 ) ) {
			if ( stbi_write_png( path, ( int ) image->width, ( int ) image->height, comp, image->data[ 0 ], 0 ) == 1 ) {
				return true;
			}
		} else if ( !pl_strncasecmp( extension, "tga", 3 ) ) {
			if ( stbi_write_tga( path, ( int ) image->width, ( int ) image->height, comp, image->data[ 0 ] ) == 1 ) {
				return true;
			}
		} else if ( !pl_strncasecmp( extension, "jpg", 3 ) || !pl_strncasecmp( extension, "jpeg", 3 ) ) {
			if ( stbi_write_jpg( path, ( int ) image->width, ( int ) image->height, comp, image->data[ 0 ], 90 ) == 1 ) {
				return true;
			}
		}
	}

	plReportErrorF( PL_RESULT_FILETYPE, plGetResultString( PL_RESULT_FILETYPE ) );
	return false;
}

// Returns the number of samples per-pixel depending on the colour format.
unsigned int plGetNumberOfColourChannels( PLColourFormat format ) {
	switch ( format ) {
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

static bool RGB8toRGBA8( PLImage *image ) {
	size_t size = plGetImageSize( PL_IMAGEFORMAT_RGBA8, image->width, image->height );
	if ( size == 0 ) {
		return false;
	}

	typedef struct RGB8 {
		uint8_t r, g, b;
	} RGB8;
	typedef struct RGBA8 {
		uint8_t r, g, b, a;
	} RGBA8;
	size_t num_pixels = image->size / 3;

	RGB8 *src = ( RGB8 * ) image->data[ 0 ];
	RGBA8 *dst = pl_malloc( size );
	for ( size_t i = 0; i < num_pixels; ++i ) {
		dst->r = src->r;
		dst->g = src->g;
		dst->b = src->b;
		dst->a = 255;
		dst++;
		src++;
	}

	pl_free( image->data[ 0 ] );

	image->data[ 0 ] = ( uint8_t * ) ( &dst[ 0 ] );
	image->size = size;
	image->format = PL_IMAGEFORMAT_RGBA8;
	image->colour_format = PL_COLOURFORMAT_RGBA;

	return true;
}

#define scale_5to8( i ) ( ( ( ( double ) ( i ) ) / 31 ) * 255 )

static uint8_t *ImageDataRGB5A1toRGBA8( const uint8_t *src, size_t n_pixels ) {
	uint8_t *dst = pl_malloc( n_pixels * 4 );
	if ( dst == NULL ) {
		return NULL;
	}

	uint8_t *dp = dst;

	for ( size_t i = 0; i < n_pixels; ++i ) {
		/* Red */
		*( dp++ ) = scale_5to8( ( src[ 0 ] & 0xF8 ) >> 3 );

		/* Green */
		*( dp++ ) = scale_5to8( ( ( src[ 0 ] & 0x07 ) << 2 ) | ( ( src[ 1 ] & 0xC0 ) >> 6 ) );

		/* Blue */
		*( dp++ ) = scale_5to8( ( src[ 1 ] & 0x3E ) >> 1 );

		/* Alpha */
		*( dp++ ) = ( src[ 1 ] & 0x01 ) ? 255 : 0;

		src += 2;
	}

	return dst;
}

bool plConvertPixelFormat( PLImage *image, PLImageFormat new_format ) {
	if ( image->format == new_format ) {
		return true;
	}

	switch ( image->format ) {
		case PL_IMAGEFORMAT_RGB8: {
			if ( new_format == PL_IMAGEFORMAT_RGBA8 ) { return RGB8toRGBA8( image ); }
		}

		case PL_IMAGEFORMAT_RGB5A1: {
			if ( new_format == PL_IMAGEFORMAT_RGBA8 ) {
				uint8_t **levels = pl_calloc( image->levels, sizeof( uint8_t * ) );

				/* Make a new copy of each detail level in the new format. */

				unsigned int lw = image->width;
				unsigned int lh = image->height;

				for ( unsigned int l = 0; l < image->levels; ++l ) {
					levels[ l ] = ImageDataRGB5A1toRGBA8( image->data[ l ], lw * lh );
					if ( levels[ l ] == NULL ) {
						/* Memory allocation failed, ditch any buffers we've created so far. */
						for ( unsigned int m = 0; m < l; ++m ) {
							pl_free( levels[ m ] );
						}

						pl_free( levels );

						plReportErrorF( PL_RESULT_MEMORY_ALLOCATION, "couldn't allocate memory for image data" );
						return false;
					}

					lw /= 2;
					lh /= 2;
				}

				/* Now that all levels have been converted, free and replace the old buffers. */

				for ( unsigned int l = 0; l < image->levels; ++l ) {
					pl_free( image->data[ l ] );
				}

				pl_free( image->data );
				image->data = levels;

				image->format = new_format;
				/* TODO: Update colour_format */

				return true;
			}
		} break;

		default:
			break;
	}

	plReportErrorF( PL_RESULT_IMAGEFORMAT, "unsupported image format conversion" );
	return false;
}

unsigned int plGetImageSize( PLImageFormat format, unsigned int width, unsigned int height ) {
	switch ( format ) {
		case PL_IMAGEFORMAT_RGB_DXT1:
			return ( width * height ) >> 1;
		default: {
			unsigned int bytes = plImageBytesPerPixel( format );
			return width * height * bytes;
		}
	}
}

/* Returns the number of BYTES per pixel for the given PLImageFormat.
 *
 * If the format doesn't have a predictable size or the size isn't a multiple
 * of one byte, returns ZERO. */
unsigned int plImageBytesPerPixel( PLImageFormat format ) {
	switch ( format ) {
		case PL_IMAGEFORMAT_RGBA4:
		case PL_IMAGEFORMAT_RGB5A1:
		case PL_IMAGEFORMAT_RGB565:
			return 2;
		case PL_IMAGEFORMAT_RGB8:
			return 3;
		case PL_IMAGEFORMAT_RGBA_DXT1:
		case PL_IMAGEFORMAT_RGBA8:
			return 4;
		case PL_IMAGEFORMAT_RGBA12:
			return 6;
		case PL_IMAGEFORMAT_RGBA16:
		case PL_IMAGEFORMAT_RGBA16F:
			return 8;
		default:
			return 0;
	}
}

void plInvertImageColour( PLImage *image ) {
	unsigned int num_colours = plGetNumberOfColourChannels( image->colour_format );
	switch ( image->format ) {
		case PL_IMAGEFORMAT_RGB8:
		case PL_IMAGEFORMAT_RGBA8: {
			for ( size_t i = 0; i < image->size; i += num_colours ) {
				uint8_t *pixel = &image->data[ 0 ][ i ];
				pixel[ 0 ] = ~pixel[ 0 ];
				pixel[ 1 ] = ~pixel[ 1 ];
				pixel[ 2 ] = ~pixel[ 2 ];
			}
		} break;

		default:
			break;
	}

	plReportErrorF( PL_RESULT_IMAGEFORMAT, "unsupported image format" );
}

/* utility function */
void plGenerateStipplePattern( PLImage *image, unsigned int depth ) {
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

void plReplaceImageColour( PLImage *image, PLColour target, PLColour dest ) {
	unsigned int num_colours = plGetNumberOfColourChannels( image->colour_format );
	switch ( image->format ) {
		case PL_IMAGEFORMAT_RGB8:
		case PL_IMAGEFORMAT_RGBA8: {
			for ( unsigned int i = 0; i < image->size; i += num_colours ) {
				uint8_t *pixel = &image->data[ 0 ][ i ];
				if ( num_colours == 4 ) {
					if ( plCompareColour( PLColour( pixel[ 0 ], pixel[ 1 ], pixel[ 2 ], pixel[ 3 ] ), target ) ) {
						pixel[ 0 ] = dest.r;
						pixel[ 1 ] = dest.g;
						pixel[ 2 ] = dest.b;
						pixel[ 3 ] = dest.a;
					}
				} else {
					if ( plCompareColour( PLColourRGB( pixel[ 0 ], pixel[ 1 ], pixel[ 2 ] ), target ) ) {
						pixel[ 0 ] = dest.r;
						pixel[ 1 ] = dest.g;
						pixel[ 2 ] = dest.b;
					}
				}
			}
		} break;

		default:
			break;
	}

	plReportErrorF( PL_RESULT_IMAGEFORMAT, "unsupported image format" );
}

void plFreeImage( PLImage *image ) {
	FunctionStart();

	if ( image == NULL || image->data == NULL ) {
		return;
	}

	for ( unsigned int levels = 0; levels < image->levels; ++levels ) {
		pl_free( image->data[ levels ] );
	}

	pl_free( image->data );
}

bool plImageIsPowerOfTwo( const PLImage *image ) {
	if ( ( ( image->width == 0 ) || ( image->height == 0 ) ) || ( !plIsPowerOfTwo( image->width ) || !plIsPowerOfTwo( image->height ) ) ) {
		return false;
	}

	return true;
}

bool plFlipImageVertical( PLImage *image ) {
	unsigned int width = image->width;
	unsigned int height = image->height;

	unsigned int bytes_per_pixel = plImageBytesPerPixel( image->format );
	if ( bytes_per_pixel == 0 ) {
		plReportErrorF( PL_RESULT_IMAGEFORMAT, "cannot flip images in this format" );
		return false;
	}

	unsigned int bytes_per_row = width * bytes_per_pixel;

	unsigned char *swap = pl_malloc( bytes_per_row );
	if ( swap == NULL ) {
		return false;
	}

	for ( unsigned int l = 0; l < image->levels; ++l ) {
		for ( unsigned int r = 0; r < height / 2; ++r ) {
			unsigned char *tr = image->data[ l ] + ( r * bytes_per_row );
			unsigned char *br = image->data[ l ] + ( ( ( height - 1 ) - r ) * bytes_per_row );

			memcpy( swap, tr, bytes_per_row );
			memcpy( tr, br, bytes_per_row );
			memcpy( br, swap, bytes_per_row );
		}

		bytes_per_row /= 2;
		height /= 2;
	}

	free( swap );

	return true;
}

/**
 * Returns a list of file extensions representing all
 * the formats supported by the image loader.
 */
const char **plGetSupportedImageFormats( unsigned int *numElements ) {
	static const char *imageFormats[ MAX_IMAGE_LOADERS ];
	for ( unsigned int i = 0; i < numImageLoaders; ++i ) {
		imageFormats[ i ] = imageLoaders[ i ].extension;
	}

	*numElements = numImageLoaders;

	return imageFormats;
}
