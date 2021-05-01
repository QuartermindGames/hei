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

/*	Monolith's DTX Format (http://www.cnblogs.com/crsky/p/4702916.html)	*/

typedef struct {
	int32_t version;// Version of the format, Lithtech used negative numbers.

	uint16_t width, height;// Width and height of the texture.
	uint16_t mipmaps;      // Number of mipmaps included.
	uint16_t sections;

	int32_t flags, userflags;
	uint8_t extra[ 12 ];
	char commandstring[ 128 ];
} DTXHeader;

#define DTX_VERSION_2 -2// Lithtech 1.0 (Shogo)
// Lithtech 1.5
// Lithtech 2.0
// Lithtech 2.2
// Lithtech 2.4
#define DTX_VERSION_5 -5// Lithtech Talon (Purge)
// Lithtech Jupiter
// Lithtech Jupiter EX
#define DTX_VERSION_MIN DTX_VERSION_2
#define DTX_VERSION_MAX DTX_VERSION_5

#define DTX_GROUP( a ) a.extra[ 0 ]
#define DTX_MIPMAP( a ) a.extra[ 1 ]
#define DTX_FORMAT( a ) a.extra[ 2 ]
#define DTX_OFFSET( a ) a.extra[ 3 ]

enum DTXFlag {
	DTX_FLAG_FULLBRIGHT = ( 1 << 0 ),
	DTX_FLAG_16BIT = ( 1 << 1 ),
	DTX_FLAG_4444 = ( 1 << 7 ),
	DTX_FLAG_5551 = ( 1 << 8 ),
	DTX_FLAG_CUBEMAP = ( 1 << 10 ),
	DTX_FLAG_NORMALMAP = ( 1 << 11 ),
} DTXFlag;

enum DTXFormat {
	DTX_FORMAT_8PALLETTE,
	DTX_FORMAT_8,
	DTX_FORMAT_16,
	DTX_FORMAT_32,

	// Compressed Formats
	DTX_FORMAT_S3TC_DXT1,
	DTX_FORMAT_S3TC_DXT3,
	DTX_FORMAT_S3TC_DXT5,
} DTXFormat;

uint8_t GetDTXFormat( DTXHeader *dtx ) {
	// This is a little hacky, DTX version 2 images don't seem to use
	// the extra[2] slot the same way as later versions. So we need to
	// basically switch it to 0 since 99.9% of textures from that period
	// use a pallette anyway.
	//
	// tl;dr this is a hack because I'm lazy!
	if ( dtx->version >= DTX_VERSION_2 ) {
		return DTX_FORMAT_8PALLETTE;
	}

	return dtx->extra[ 2 ];
}

bool PlDtxFormatCheck( PLFile *fin ) {
	PlRewindFile( fin );

	// Try reading in the type first, as Lithtech has "resource types" rather than idents.
	int type;
	if ( PlReadFile( fin, &type, sizeof( int ), 1 ) != 1 ) {
		return false;
	}

	return ( type == 0 );
}

bool PlLoadDtxImage( PLFile *fin, PLImage *out ) {
	DTXHeader header;
	if ( PlReadFile( fin, &header, sizeof( DTXHeader ), 1 ) != 1 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, PlGetResultString( PL_RESULT_FILEREAD ) );
		return false;
	} else if ( ( header.version < DTX_VERSION_MAX ) || ( header.version > DTX_VERSION_MIN ) ) {
		PlReportErrorF( PL_RESULT_FILEVERSION, "invalid version: %d", header.version );
		return false;
	} else if ( ( header.width < 8 ) || ( header.height < 8 ) ) {
		PlReportErrorF( PL_RESULT_IMAGERESOLUTION, "invalid resolution: w(%d) h(%d)", header.width, header.height );
		return false;
	}

	memset( out, 0, sizeof( PLImage ) );

	out->width = header.width;
	out->height = header.height;

	switch ( GetDTXFormat( &header ) ) {
		case DTX_FORMAT_8PALLETTE:
			out->size = header.width * header.height;
			out->format = PL_IMAGEFORMAT_RGB8;
			out->colour_format = PL_COLOURFORMAT_RGB;
			break;

		case DTX_FORMAT_S3TC_DXT1:
			out->size = ( header.width * header.height ) >> 1;
			out->format = PL_IMAGEFORMAT_RGB_DXT1;
			out->colour_format = PL_COLOURFORMAT_RGB;
			break;
		case DTX_FORMAT_S3TC_DXT3:
			out->size = header.width * header.height;
			out->format = PL_IMAGEFORMAT_RGBA_DXT3;
			out->colour_format = PL_COLOURFORMAT_RGBA;
			break;
		case DTX_FORMAT_S3TC_DXT5:
			out->size = header.width * header.height;
			out->format = PL_IMAGEFORMAT_RGBA_DXT5;
			out->colour_format = PL_COLOURFORMAT_RGBA;
			break;

		case DTX_FORMAT_8:
			out->size = header.width * header.height;
			out->format = PL_IMAGEFORMAT_RGB8;
			out->colour_format = PL_COLOURFORMAT_RGB;
			break;
		default:
		case DTX_FORMAT_16:
		case DTX_FORMAT_32:
			out->size = ( unsigned int ) ( header.width * header.height * 4 );
			out->format = PL_IMAGEFORMAT_RGBA8;
			out->colour_format = PL_COLOURFORMAT_RGBA;
			break;
	}

	if ( !out->size ) {
		PlReportErrorF( PL_RESULT_FILESIZE, "invalid image size" );
		return false;
	}

#if 0
    for (int i = 0; i < header.mipmaps; i++) {
    }
#endif

	out->levels = 1;
	out->data = pl_calloc( out->levels, sizeof( uint8_t * ) );
	if ( out->data == NULL ) {
		PlFreeImage( out );
		return false;
	}

	out->data[ 0 ] = pl_calloc( out->size, sizeof( uint8_t ) );
	if ( out->data[ 0 ] == NULL ) {
		PlFreeImage( out );
		return false;
	}

	PlReadFile( fin, out->data[ 0 ], sizeof( uint8_t ), out->size );

	/*	for (unsigned int i = 0; i < (unsigned int)size; i += 4)
    {
    image[i + 0] ^= image[i + 2];
    image[i + 2] ^= image[i + 0];
    image[i + 0] ^= image[i + 2];
    }*/

	return true;
}