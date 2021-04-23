/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "image_private.h"

/* Ritual Entertainment's SWL Format, used by SiN */

typedef struct SWLHeader {
	char path[ 64 ]; /* file path, kinda useless for us */
	uint32_t width;
	uint32_t height;
} SWLHeader;

static PLImage *ReadSwlImage( PLFile *fin ) {
	SWLHeader header;
	if ( PlReadFile( fin, &header, sizeof( header ), 1 ) != 1 ) {
		return NULL;
	}

	if ( header.width > 512 || header.width == 0 ||
	     header.height > 512 || header.height == 0 ) {
		PlReportBasicError( PL_RESULT_IMAGERESOLUTION );
		return NULL;
	}

	struct {
		uint8_t r, g, b, a;
	} palette[ 256 ];
	if ( PlReadFile( fin, palette, 4, 256 ) != 256 ) {
		PlReportBasicError( PL_RESULT_FILEREAD );
		return false;
	}

	/* according to sources, this is a collection of misc data that's
   * specific to SiN itself, so we'll skip it. */
	if ( !PlFileSeek( fin, 0x4D4, PL_SEEK_SET ) ) {
		PlReportBasicError( PL_RESULT_FILEREAD );
		return false;
	}

	PLImage *out = malloc( sizeof( PLImage ) );
	out->width = header.width;
	out->height = header.height;
	out->levels = 4;
	out->colour_format = PL_COLOURFORMAT_RGBA;
	out->format = PL_IMAGEFORMAT_RGBA8;
	out->size = PlGetImageSize( out->format, out->width, out->height );

	unsigned int mip_w = out->width;
	unsigned int mip_h = out->height;
	for ( unsigned int i = 0; i < out->levels; ++i ) {
		if ( i > 0 ) {
			mip_w = out->width >> ( i + 1 );
			mip_h = out->height >> ( i + 1 );
		}

		size_t buf_size = mip_w * mip_h;
		uint8_t *buf = pl_malloc( buf_size );
		if ( PlReadFile( fin, buf, 1, buf_size ) != buf_size ) {
			PlDestroyImage( out );
			pl_free( buf );
			return false;
		}

		out->data = pl_calloc( out->levels, sizeof( uint8_t * ) );

		size_t level_size = PlGetImageSize( out->format, mip_w, mip_h );
		out->data[ i ] = pl_calloc( level_size, sizeof( uint8_t ) );

		/* now we fill in the buf we just allocated,
     * by using the palette */
		for ( size_t j = 0, k = 0; j < buf_size; ++j, k += 4 ) {
			out->data[ i ][ k ] = palette[ buf[ j ] ].r;
			out->data[ i ][ k + 1 ] = palette[ buf[ j ] ].g;
			out->data[ i ][ k + 2 ] = palette[ buf[ j ] ].b;

			/* the alpha channel appears to be used more like
       * a flag to say "yes this texture will be transparent",
       * rather than actual levels of alpha for this pixel.
       *
       * because of that we'll just ignore it */
			out->data[ i ][ k + 3 ] = 255; /*(uint8_t) (255 - palette[buf[j]].a);*/
		}

		pl_free( buf );
	}

	return out;
}

PLImage *PlLoadSwlImage( const char *path ) {
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLImage *image = ReadSwlImage( file );

	PlCloseFile( file );

	return image;
}
