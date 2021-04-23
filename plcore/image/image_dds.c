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

#include <plcore/pl_image.h>

struct DDSPixelFormat {
	uint32_t size;
	uint32_t flags;
	uint32_t fourcc;
	uint32_t rgbbitcount;
} DDSPixelFormat;

typedef struct DDSHeader {
	uint32_t size;// Should always be 124.
	uint32_t flags;
	uint32_t height, width;
	uint32_t pitchlinear;
	uint32_t depth;
	uint32_t levels;
	uint32_t reserved1[ 11 ];

	//

	uint32_t caps, caps2, caps3, caps4;
	uint32_t reserved2;
} DDSHeader;

enum DDSFlag {
	DDS_CAPS,
};

bool PlDDSFormatCheck( PLFile *fin ) {
	PlRewindFile( fin );

	char ident[ 4 ];
	if ( PlReadFile( fin, ident, sizeof( char ), 4 ) != 4 ) {
		return false;
	}

	return ( bool ) ( strncmp( ident, "DDS", 3 ) == 0 );
}

bool PlLoadDDSImage( PLFile *fin, PLImage *out ) {
	DDSHeader header;
	memset( &header, 0, sizeof( DDSHeader ) );
	if ( PlReadFile( fin, &header, sizeof( DDSHeader ), 1 ) != 1 ) {
		return false;
	}

	memset( out, 0, sizeof( PLImage ) );

	out->width = header.width;
	out->height = header.height;

	return true;
}
