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

/*	Ritual's FTX Format	*/

typedef struct FtxHeader {
	uint32_t width;
	uint32_t height;
	uint32_t alpha;
} FtxHeader;

PLImage *PlLoadFtxImage( const char *path ) {
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	FtxHeader header;
	bool status;
	header.width = PlReadInt32( file, false, &status );
	header.height = PlReadInt32( file, false, &status );
	header.alpha = PlReadInt32( file, false, &status );

	if ( !status ) {
		PlCloseFile( file );
		return NULL;
	}

	unsigned int size = header.width * header.height * 4;
	uint8_t *buffer = pl_malloc( size );
	size_t rSize = PlReadFile( file, buffer, sizeof( uint8_t ), size );

	PlCloseFile( file );

	if ( rSize != size ) {
		pl_free( buffer );
		return NULL;
	}

	PLImage *image = PlCreateImage( buffer, header.width, header.height, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );

	/* create image makes a copy of the buffer */
	pl_free( buffer );

	return image;
}
