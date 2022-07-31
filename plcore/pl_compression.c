/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_compression.h>

#include "3rdparty/miniz/miniz.h"

/*
 * Expose miniz functionality through the platform library,
 * so other libraries don't need to also include it.
 */

void *PlCompress_Deflate( const void *src, unsigned long srcLength, unsigned long *dstLength ) {
	unsigned long compressedLength = mz_compressBound( srcLength );
	void *compressedData = PlMAllocA( compressedLength );
	int status = mz_compress( compressedData, &compressedLength, src, srcLength );
	if ( status != MZ_OK ) {
		PlReportErrorF( PL_RESULT_FAIL, "failed to compress data: %s", mz_error( status ) );
		PlFree( compressedData );
		return NULL;
	}

	*dstLength = compressedLength;
	return compressedData;
}

/****************************************
 * LZRW1
 * http://ross.net/compression/download/original/old_lzrw1.c
 ****************************************/

#define LZRW1_FLAG_BYTES 4

enum {
	LZRW1_FLAG_COMPRESS = 0,
	LZRW1_FLAG_COPY = 1,
};

void *PlCompress_LZRW1( const void *src, unsigned long srcLength, unsigned long *dstLength ) {
	const void *p = src;
}

void *PlDecompress_LZRW1( const void *src, unsigned long srcLength, unsigned long *dstLength ) {
	const uint8_t *p_src = src + LZRW1_FLAG_BYTES, *p_src_post = src + srcLength;
	if ( *( int * ) src == LZRW1_FLAG_COPY ) {
		*dstLength = srcLength - LZRW1_FLAG_BYTES;
		void *dst = PL_NEW_( uint8_t, *dstLength );
		memcpy( dst, src + LZRW1_FLAG_BYTES, *dstLength );
		return dst;
	}

	uint16_t controlbits = 0, control;
	while ( p_src != p_src_post ) {
		if ( controlbits == 0 ) {
			control = *p_src++;
			control |= ( *p_src++ ) << 8;
			controlbits = 16;
		}
		if ( control & 1 ) {
			uint16_t offset, len;
			uint8_t *p;
			offset = ( *p_src & 0xF0 ) << 4;
			len = 1 + ( *p_src++ & 0xF );
			offset += *p_src++ & 0xFF;
			p = p_dst - offset;
			while ( len-- ) *p_dst++ = *p++;
		} else
			*p_dst++ = *p_src++;
		control >>= 1;
		controlbits--;
	}
	*dstLength = p_dst - p_dst_first;
}
