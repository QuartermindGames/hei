/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "image_private.h"
#include "qmos/public/qm_os_memory.h"

/*	Ritual's FTX Format	*/

typedef struct FtxHeader {
	uint32_t width;
	uint32_t height;
	uint32_t alpha;
} FtxHeader;

QmImage *qm_image_ftx_parse( QmFsFile *file ) {
	FtxHeader header;
	bool status;
	header.width = qm_fs_file_read_int32( file, false, &status );
	header.height = qm_fs_file_read_int32( file, false, &status );
	header.alpha = qm_fs_file_read_int32( file, false, &status );

	if ( !status ) {
		return NULL;
	}

	unsigned int size = header.width * header.height * 4;
	uint8_t *buffer = QM_OS_MEMORY_MALLOC_( size );
	size_t rSize = qm_file_read( file, buffer, sizeof( uint8_t ), size );

	if ( rSize != size ) {
		qm_os_memory_free( buffer );
		return NULL;
	}

	QmImage *image = PlCreateImage( buffer, header.width, header.height, 0, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );

	/* create image makes a copy of the buffer */
	qm_os_memory_free( buffer );

	return image;
}
