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
#include <PL/platform_filesystem.h>

/*	Ritual's FTX Format	*/

typedef struct FtxHeader {
    uint32_t width;
    uint32_t height;
    uint32_t alpha;
} FtxHeader;

PLImage *plLoadFtxImage( const char *path ) {
	PLFile *file = plOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	FtxHeader header;
	bool status;
	header.width = plReadInt32( file, false, &status );
	header.height = plReadInt32( file, false, &status );
	header.alpha = plReadInt32( file, false, &status );

	if ( !status ) {
		plCloseFile( file );
		return NULL;
	}

	unsigned int size = header.width * header.height * 4;
	uint8_t *buffer = pl_malloc( size );
	size_t rSize = plReadFile( file, buffer, sizeof( uint8_t ), size );

	plCloseFile( file );

	if ( rSize != size ) {
		pl_free( buffer );
		return NULL;
	}

	PLImage *image = plCreateImage( buffer, header.width, header.height, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );

	/* create image makes a copy of the buffer */
	pl_free( buffer );

	return image;
}
