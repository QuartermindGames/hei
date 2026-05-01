// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2026 Mark E Sowden <hogsy@oldtimes-software.com>
// Purpose: Loader for Resident Evil 4's ARGB format.

#include "image_private.h"

PLImage *PlParseArgbImage( PLFile *file ) {
	PLImageFormat imageFormat = PL_IMAGEFORMAT_UNKNOWN;

	const int32_t format = PlReadInt32( file, false, NULL );
	if ( format == PL_MAGIC_TO_NUM( 'B', 'G', 'R', 'A' ) ) {
		const uint8_t bf = PL_READUINT8( file, NULL );
		const uint8_t gf = PL_READUINT8( file, NULL );
		const uint8_t rf = PL_READUINT8( file, NULL );
		const uint8_t af = PL_READUINT8( file, NULL );
		if ( bf == 8 && gf == 8 && rf == 8 && af == 8 ) {
			imageFormat = PL_IMAGEFORMAT_BGRA8;
		}
	}

	if ( imageFormat == PL_IMAGEFORMAT_UNKNOWN ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unknown argb format (%d)", format );
		return NULL;
	}

	const uint32_t w = PL_READUINT32( file, false, NULL );
	const uint32_t h = PL_READUINT32( file, false, NULL );
	// sanity checking...
	if ( w == 0 || w >= INT16_MAX || h == 0 || h >= INT16_MAX ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid argb resolution (%ux%u)", w, h );
		return NULL;
	}

	const unsigned int bpp = PlGetImageFormatPixelSize( imageFormat );
	const unsigned int size = w * h * bpp;
	uint8_t *buf = PL_NEW_( uint8_t, size );
	if ( buf == NULL ) {
		return NULL;
	}

	if ( PlReadFile( file, buf, sizeof( uint8_t ), size ) != size ) {
		PL_DELETE( buf );
		return NULL;
	}

	PLImage *image = PlCreateImage( buf, w, h, 0, PL_COLOURFORMAT_BGRA, PL_IMAGEFORMAT_BGRA8 );

	PL_DELETE( buf );

	PlConvertPixelFormat( image, PL_IMAGEFORMAT_RGBA8 );

	return image;
}
