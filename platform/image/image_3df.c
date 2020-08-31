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
	sscanf( buf, "lod range: %d %d\n", &w, &h );

	/* aspect */
	if ( plReadString( file, buf, sizeof( buf ) ) == NULL ) {
		return NULL;
	}
	int x, y;
	sscanf( buf, "aspect ratio: %d %d\n", &x, &y );
	if ( x != 1 && y != 1 ) {
		ReportError( PL_RESULT_IMAGERESOLUTION, "unsupported aspect ratio, \"%s\"", buf );
		return NULL;
	}

	/* now we can load the actual data in */
	size_t imgBufSize = plGetImageSize( dataFormat, w, h );
	uint8_t *imgBuf = pl_malloc( imgBufSize );

	if ( plReadFile( file, imgBuf, sizeof( char ), imgBufSize ) != imgBufSize ) {
		pl_free( imgBuf );
		return NULL;
	}

	PLImage *image = plCreateImage( imgBuf, w, h, PL_COLOURFORMAT_ARGB, dataFormat );

	/* no longer need this */
	pl_free( imgBuf );

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
