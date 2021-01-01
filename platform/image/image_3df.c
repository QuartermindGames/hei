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

/**
 * 3dfx 3DF Loader
 * Added purely because Shadow Man 97 uses this format,
 * and done the bare minimum to get it working with that.
 **/

static PLImageFormat FD3_GetImageFormat( const char *formatStr ) {
	if ( strcmp( "argb1555\n", formatStr ) == 0 ) {
		return PL_IMAGEFORMAT_RGB5A1;
	} else if ( strcmp( "argb4444\n", formatStr ) == 0 ) {
		return PL_IMAGEFORMAT_RGBA4;
	} else if ( strcmp( "rgb565\n", formatStr ) == 0 ) {
		return PL_IMAGEFORMAT_RGB565;
	}

	return PL_IMAGEFORMAT_UNKNOWN;
}

static PLImage *FD3_ReadFile( PLFile *file ) {
	/* read in the header */
	char buf[ 64 ];

	/* identifier */
	if ( plReadString( file, buf, sizeof( buf ) ) == NULL ) {
		return NULL;
	}
	if ( strncmp( buf, "3df ", 4 ) != 0 ) {
		ReportError( PL_RESULT_FILETYPE, "invalid identifier, expected \"3df \"" );
		return NULL;
	}

	/* image format */
	plReadString( file, buf, sizeof( buf ) );
	PLImageFormat dataFormat = FD3_GetImageFormat( buf );
	if ( dataFormat == PL_IMAGEFORMAT_UNKNOWN ) {
		ReportError( PL_RESULT_IMAGEFORMAT, "unsupported image format, \"%s\"", buf );
		return NULL;
	}

	/* lod */
	if ( plReadString( file, buf, sizeof( buf ) ) == NULL ) {
		return NULL;
	}
	int w, h;
	if ( sscanf( buf, "lod range: %d %d\n", &w, &h ) != 2 ) {
		ReportError( PL_RESULT_FILEREAD, "failed to read lod range" );
		return NULL;
	}
	if ( w <= 0 || h <= 0 || w > 256 || h > 256 ) {
		ReportBasicError( PL_RESULT_IMAGERESOLUTION );
		return NULL;
	}

	/* aspect */
	if ( plReadString( file, buf, sizeof( buf ) ) == NULL ) {
		return NULL;
	}
	int x, y;
	if ( sscanf( buf, "aspect ratio: %d %d\n", &x, &y ) != 2 ) {
		ReportError( PL_RESULT_FILEREAD, "failed to read aspect ratio" );
		return NULL;
	}

	switch ( ( x << 4 ) | ( y ) ) {
		case 0x81:
			h = h / 8;
			break;
		case 0x41:
			h = h / 4;
			break;
		case 0x21:
			h = h / 2;
			break;
		case 0x11:
			h = h / 1;
			break;
		case 0x12:
			w = w / 2;
			break;
		case 0x14:
			w = w / 4;
			break;
		case 0x18:
			w = w / 8;
			break;
		default:
			ReportError( PL_RESULT_FAIL, "unexpected aspect-ratio: %dx%d", x, y );
			return NULL;
	}

	/* now we can load the actual data in */
	size_t srcSize = plGetImageSize( dataFormat, w, h );
	uint8_t *srcBuf = pl_malloc( srcSize );
	if ( plReadFile( file, srcBuf, sizeof( char ), srcSize ) != srcSize ) {
		pl_free( srcBuf );
		return NULL;
	}

	/* convert it... */
	size_t dstSize = plGetImageSize( PL_IMAGEFORMAT_RGBA8, w, h );
	uint8_t *dstBuf = pl_malloc( dstSize );
	if ( dataFormat != PL_IMAGEFORMAT_RGBA8 ) {
		switch( dataFormat ) {
			case PL_IMAGEFORMAT_RGB5A1: {
				uint8_t *dstPos = dstBuf;
                for ( size_t i = 0; i < srcSize; i += 2 ) {
					dstPos[ PL_RED ]    = ( ( srcBuf[ i ] & 124 ) << 1 );
					dstPos[ PL_GREEN ]  = ( ( srcBuf[ i ] & 3 ) << 6 ) | ( ( srcBuf[ i + 1 ] & 224 ) >> 2 );
					dstPos[ PL_BLUE ]   = ( ( srcBuf[ i + 1 ] & 31 ) << 3 );
					dstPos[ PL_ALPHA ]  = ( srcBuf[ i ] & 128 ) ? 0 : 255;
					dstPos += 4;
				}

				pl_free( srcBuf );
				break;
			}
			default:
				pl_free( dstBuf );
				dstBuf = srcBuf;
				break;

		}
	}

	PLImage *image = plCreateImage( dstBuf, w, h, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );

	/* no longer need this */
	pl_free( dstBuf );

	return image;
}

PLImage *plLoad3dfImage( const char *path ) {
	PLFile *file = plOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLImage *image = FD3_ReadFile( file );

	plCloseFile( file );

	return image;
}
