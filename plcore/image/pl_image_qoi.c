/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "qmos/public/qm_os_memory.h"
#include "image_private.h"

#define QOI_MALLOC( sz ) QM_OS_MEMORY_MALLOC_( sz )
#define QOI_FREE( p )    qm_os_memory_free( p )

#define QOI_IMPLEMENTATION
#include "3rdparty/qoi.h"

PLImage *PlParseQoiImage( PLFile *file ) {
	PLImage *image = NULL;

	int size = ( int ) PlGetFileSize( file );
	uint8_t *buf = QM_OS_MEMORY_NEW_( uint8_t, size + 1 );
	if ( PlReadFile( file, buf, sizeof( uint8_t ), size ) == size ) {
		qoi_desc desc;
		uint8_t *dstBuf = qoi_decode( buf, size, &desc, 0 );
		if ( dstBuf == NULL ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "failed to decode qoi image" );
			return NULL;
		}

		image = PlCreateImage( dstBuf, desc.width, desc.height, 0, PL_COLOURFORMAT_RGBA,
		                       ( desc.channels == 4 ) ? PL_IMAGEFORMAT_RGBA8 : PL_IMAGEFORMAT_RGB8 );

		qm_os_memory_free( dstBuf );
	}

	qm_os_memory_free( buf );

	return image;
}

bool PlWriteQoiImage( const PLImage *image, const char *path ) {
	if ( image->format != PL_IMAGEFORMAT_RGBA8 && image->format != PL_IMAGEFORMAT_RGB8 ) {
		PlReportBasicError( PL_RESULT_IMAGEFORMAT );
		return false;
	}

	qoi_desc desc;
	PL_ZERO_( desc );
	desc.width = image->width;
	desc.height = image->height;
	desc.channels = ( image->format == PL_IMAGEFORMAT_RGBA8 ) ? 4 : 3;

	if ( qoi_write( path, image->data[ 0 ], &desc ) == 0 ) {
		PlReportBasicError( PL_RESULT_FILEWRITE );
		return false;
	}

	return true;
}
