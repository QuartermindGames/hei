// SPDX-License-Identifier: MIT
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "plcore/pl_image.h"

// Red Storm Entertainment RSB Format

static const uint32_t RSB_VERSION_MIN = 0;
static const uint32_t RSB_VERSION_MAX = 1;

PLImage *PlParseRsbImage( PLFile *file ) {
	// always 1
	uint32_t version = PL_READUINT32( file, false, NULL );
	if ( version < RSB_VERSION_MIN || version > RSB_VERSION_MAX ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected version (%u)", version );
		return NULL;
	}

	// size
	uint32_t width = PL_READUINT32( file, false, NULL );
	uint32_t height = PL_READUINT32( file, false, NULL );
	if ( width == 0 || height == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid image dimensions (%ux%u)", width, height );
		return NULL;
	}

	if ( version == 0 ) {
		PL_READUINT32( file, false, NULL );// unknown
	}

	// format
	uint32_t r = PL_READUINT32( file, false, NULL );
	uint32_t g = PL_READUINT32( file, false, NULL );
	uint32_t b = PL_READUINT32( file, false, NULL );
	uint32_t a = PL_READUINT32( file, false, NULL );

	PLImage *image = NULL;

	unsigned int size = width * height;
	uint16_t *src = PL_NEW_( uint16_t, size );
	if ( PlReadFile( file, src, sizeof( uint16_t ), size ) == size ) {
		PLColour *dst = PL_NEW_( PLColour, size );
		uint32_t maskR = ( 1 << r ) - 1;
		uint32_t maskG = ( 1 << g ) - 1;
		uint32_t maskB = ( 1 << b ) - 1;
		uint32_t maskA = ( 1 << a ) - 1;

		uint32_t shiftA = r + g + b;
		uint32_t shiftR = g + b;
		uint32_t shiftG = b;
		uint32_t shiftB = 0;

		for ( unsigned int i = 0; i < size; i++ ) {
			dst[ i ].r = ( ( src[ i ] & ( maskR << shiftR ) ) >> shiftR ) * 255 / maskR;
			dst[ i ].g = ( ( src[ i ] & ( maskG << shiftG ) ) >> shiftG ) * 255 / maskG;
			dst[ i ].b = ( ( src[ i ] & ( maskB << shiftB ) ) >> shiftB ) * 255 / maskB;
			dst[ i ].a = a ? ( ( src[ i ] & ( maskA << shiftA ) ) >> shiftA ) * 255 / maskA : 255;
		}

		image = PlCreateImage( dst, width, height, 0, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );

		PL_DELETE( dst );
	}

	PL_DELETE( src );

	return image;
}

void PlRegisterRsbImageLoader( void ) {
	PlRegisterImageLoader( "rsb", PlParseRsbImage );
}
