// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2025 Mark E Sowden <hogsy@oldtimes-software.com>

#include "image_private.h"
#include "qmos/public/qm_os_memory.h"

/**
 * Nexus, an early prototype using Plasma, seems to use HSM versions 0 and 1.
 * Format is incredibly simple as far as I can tell. No compression?
 *
 * Uru Online seems to use version 2 which has compression. I've not bothered
 * looking too much at that yet.
 */

#define HSM_MAX_VERSION 1

PLImage *PlParseHsmImage( PLFile *file ) {
	bool status;
	uint32_t version = PL_READUINT32( file, true, &status );
	uint32_t width = PL_READUINT32( file, true, &status );
	uint32_t height = PL_READUINT32( file, true, &status );
	if ( !status ) {
		return NULL;
	}

	if ( version > HSM_MAX_VERSION ) {
		PlReportErrorF( PL_RESULT_FILEVERSION, "unexpected version (%u != %u)", version, HSM_MAX_VERSION );
		return NULL;
	}

	if ( width == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid width (%u)", width );
		return NULL;
	}

	if ( height == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid height (%u)", height );
		return NULL;
	}

	// not currently sure of these...
	PL_READUINT32( file, true, &status );
	PL_READUINT32( file, true, &status );
	PL_READUINT16( file, true, &status );
	if ( !status ) {
		return NULL;
	}

	unsigned int size = width * height * 4;
	uint8_t *buf = QM_OS_MEMORY_NEW_( uint8_t, size );
	if ( PlReadFile( file, buf, sizeof( uint8_t ), size ) != size ) {
		qm_os_memory_free( buf );
		return NULL;
	}

	PLImage *image = PlCreateImage( buf, width, height, 0, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );

	qm_os_memory_free( buf );
	return image;
}
