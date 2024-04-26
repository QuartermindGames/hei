// SPDX-License-Identifier: MIT
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "plcore/pl_image.h"

// Quickly whipped up loader for 3D Realms' TEX format

PLImage *PlParse3drTexImage( PLFile *file ) {
	// possibly some sort of image mode? non-zero for alpha textures
	uint16_t mode = PL_READUINT16( file, false, NULL );
	if ( mode != 0 && mode != 0x500 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected mode" );
		return NULL;
	}

	uint16_t numMipmaps = PL_READUINT16( file, false, NULL );
	if ( numMipmaps == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected number of mipmaps" );
		return NULL;
	}

	uint16_t width = PL_READUINT16( file, false, NULL );
	uint16_t height = PL_READUINT16( file, false, NULL );
	if ( width == 0 || height == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid image dimensions" );
		return NULL;
	}

	// no idea...
	PlFileSeek( file, 28, PL_SEEK_CUR );

	// not sure why we've got this again, but make sure it matches for some validation
	uint16_t w2 = PL_READUINT16( file, false, NULL );
	uint16_t h2 = PL_READUINT16( file, false, NULL );
	if ( w2 != width || h2 != height ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "secondary image dimensions do not match" );
		return NULL;
	}

	// again, no idea...
	PlFileSeek( file, 12, PL_SEEK_CUR );

	unsigned int size = width * height;
	uint16_t *src = PL_NEW_( uint16_t, size );
	if ( PlReadFile( file, src, sizeof( uint16_t ), size ) != size ) {
		PL_DELETE( src );
		return NULL;
	}

	PLColour *dst = PL_NEW_( PLColour, size );
	if ( mode == 0x500 ) {
		const unsigned int shiftA = 12;
		const unsigned int shiftR = 8;
		const unsigned int shiftG = 4;
		const unsigned int shiftB = 0;

		const unsigned int mask = 15;

		for ( unsigned int i = 0; i < size; i++ ) {
			dst[ i ].a = ( ( src[ i ] & ( mask << shiftA ) ) >> shiftA ) * 255 / mask;
			dst[ i ].r = ( ( src[ i ] & ( mask << shiftR ) ) >> shiftR ) * 255 / mask;
			dst[ i ].g = ( ( src[ i ] & ( mask << shiftG ) ) >> shiftG ) * 255 / mask;
			dst[ i ].b = ( ( src[ i ] & ( mask << shiftB ) ) >> shiftB ) * 255 / mask;
		}
	} else {
		const unsigned int shiftR = 11;
		const unsigned int shiftG = 5;
		const unsigned int shiftB = 0;

		const unsigned int maskR = 31;
		const unsigned int maskG = 63;
		const unsigned int maskB = 31;

		for ( unsigned int i = 0; i < size; i++ ) {
			dst[ i ].a = 255;
			dst[ i ].r = ( ( src[ i ] & ( maskR << shiftR ) ) >> shiftR ) * 255 / maskR;
			dst[ i ].g = ( ( src[ i ] & ( maskG << shiftG ) ) >> shiftG ) * 255 / maskG;
			dst[ i ].b = ( ( src[ i ] & ( maskB << shiftB ) ) >> shiftB ) * 255 / maskB;
		}
	}

	PLImage *image = PlCreateImage( dst, width, height, 0, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );

	PL_DELETE( src );
	PL_DELETE( dst );

	return image;
}

void PlRegister3drTexImageLoader( void ) {
	PlRegisterImageLoader( "tex", PlParse3drTexImage );
}
