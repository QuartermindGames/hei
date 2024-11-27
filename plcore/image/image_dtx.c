/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
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

#define DTX_VERSION_2   -2// Lithtech 1.0 (Shogo)
// Lithtech 1.5
// Lithtech 2.0
// Lithtech 2.2
// Lithtech 2.4
#define DTX_VERSION_5   -5// Lithtech Talon (Purge)
// Lithtech Jupiter
// Lithtech Jupiter EX
#define DTX_VERSION_MIN DTX_VERSION_2
#define DTX_VERSION_MAX DTX_VERSION_5

#define DTX_GROUP( a )  a.extra[ 0 ]
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

static uint8_t GetDTXFormat( DTXHeader *dtx ) {
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

PLImage *PlParseDtxImage_( PLFile *file ) {
	DTXHeader header;
	if ( PlReadFile( file, &header, sizeof( DTXHeader ), 1 ) != 1 ) {
		return NULL;
	}

	if ( header.version < DTX_VERSION_MAX || header.version > DTX_VERSION_MIN ) {
		PlReportErrorF( PL_RESULT_FILEVERSION, "invalid version: %d", header.version );
		return NULL;
	}
	if ( header.width < 8 || header.height < 8 ) {
		PlReportErrorF( PL_RESULT_IMAGERESOLUTION, "invalid resolution: w(%d) h(%d)", header.width, header.height );
		return NULL;
	}

	size_t size = 0;
	PLImageFormat imageFormat;
	PLColourFormat colourFormat;
	switch ( GetDTXFormat( &header ) ) {
		case DTX_FORMAT_8PALLETTE: {
			size = header.width * header.height;
			imageFormat = PL_IMAGEFORMAT_RGB8;
			colourFormat = PL_COLOURFORMAT_RGB;
			break;
		}
		case DTX_FORMAT_S3TC_DXT1: {
			size = ( header.width * header.height ) >> 1;
			imageFormat = PL_IMAGEFORMAT_RGB_DXT1;
			colourFormat = PL_COLOURFORMAT_RGB;
			break;
		}
		case DTX_FORMAT_S3TC_DXT3: {
			size = header.width * header.height;
			imageFormat = PL_IMAGEFORMAT_RGBA_DXT3;
			colourFormat = PL_COLOURFORMAT_RGBA;
			break;
		}
		case DTX_FORMAT_S3TC_DXT5: {
			size = header.width * header.height;
			imageFormat = PL_IMAGEFORMAT_RGBA_DXT5;
			colourFormat = PL_COLOURFORMAT_RGBA;
			break;
		}
		case DTX_FORMAT_8: {
			size = header.width * header.height;
			imageFormat = PL_IMAGEFORMAT_RGB8;
			colourFormat = PL_COLOURFORMAT_RGB;
			break;
		}
		case DTX_FORMAT_32: {
			size = ( header.width * header.height ) * 4;
			imageFormat = PL_IMAGEFORMAT_RGBA8;
			colourFormat = PL_COLOURFORMAT_RGBA;
			break;
		}
		default:
			PlReportErrorF( PL_RESULT_IMAGEFORMAT, "unsupported image format" );
			return NULL;
	}

	if ( size == 0 ) {
		PlReportErrorF( PL_RESULT_IMAGEFORMAT, "invalid image size" );
		return NULL;
	}

	PLImage *image = PlCreateImage( NULL, header.width, header.height, 0, colourFormat, imageFormat );
	if ( image == NULL ) {
		return NULL;
	}

	if ( PlReadFile( file, image->data[ 0 ], sizeof( uint8_t ), size ) != size ) {
		PlDestroyImage( image );
		image = NULL;
	}

	return image;
}
