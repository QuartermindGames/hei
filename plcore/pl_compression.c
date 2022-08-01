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
#define LZRW1_ITEM_MAX   16

enum {
	LZRW1_FLAG_COMPRESS = 0,
	LZRW1_FLAG_COPY = 1,
};

void *PlCompress_LZRW1( const void *src, size_t srcLength, size_t *dstLength ) {
	uint8_t *dst = PL_NEW_( uint8_t, srcLength );

	const uint8_t *p_src_post = src + srcLength;
	const uint8_t *p_src_max1 = p_src_post - LZRW1_ITEM_MAX, *p_src_max16 = p_src_post - 16 * LZRW1_ITEM_MAX;
	const uint8_t *hash[ 4096 ];
	uint16_t control = 0, control_bits = 0;

	uint8_t *p_dst = dst;
	*p_dst = LZRW1_FLAG_COMPRESS;
	p_dst += LZRW1_FLAG_BYTES;
	uint8_t *p_control = p_dst;
	p_dst += 2;

	const uint8_t *p_src = src;
	uint8_t *p_dst_post = dst + srcLength;
	while ( true ) {
		const uint8_t *p, *s;
		uint16_t unroll = 16, len, index;
		uint32_t offset;
		if ( p_dst > p_dst_post ) {
			memcpy( dst + LZRW1_FLAG_BYTES, src, srcLength );
			*dst = LZRW1_FLAG_COPY;
			*dstLength = srcLength + LZRW1_FLAG_BYTES;
			return dst;
		}

		if ( p_src > p_src_max16 ) {
			unroll = 1;
			if ( p_src > p_src_max1 ) {
				if ( p_src == p_src_post ) break;
				goto literal;
			}
		}

	begin_unrolled_loop:
		index = ( ( 40543 * ( ( ( ( p_src[ 0 ] << 4 ) ^ p_src[ 1 ] ) << 4 ) ^ p_src[ 2 ] ) ) >> 4 ) & 0xFFF;
		p = hash[ index ];
		hash[ index ] = s = p_src;
		offset = s - p;
#define PS *p++ != *s++
		if ( offset > 4095 || p < ( uint8_t * ) src || offset == 0 || PS || PS || PS ) {
		literal:
			*p_dst++ = *p_src++;
			control >>= 1;
			control_bits++;
		} else {
			PS || PS || PS || PS || PS || PS || PS || PS || PS || PS || PS || PS || PS || s++;
			len = s - p_src - 1;
			*p_dst++ = ( ( offset & 0xF00 ) >> 4 ) + ( len - 1 );
			*p_dst++ = offset & 0xFF;
			p_src += len;
			control = ( control >> 1 ) | 0x8000;
			control_bits++;
		}

		if ( --unroll ) {
			goto begin_unrolled_loop;
		}

		if ( control_bits == 16 ) {
			*p_control = control & 0xFF;
			*( p_control + 1 ) = control >> 8;
			p_control = p_dst;
			p_dst += 2;
			control = control_bits = 0;
		}
	}

	control >>= 16 - control_bits;
	*p_control++ = control & 0xFF;
	*p_control++ = control >> 8;
	if ( p_control == p_dst ) {
		p_dst -= 2;
	}

	*dstLength = p_dst - dst;
	dst = PlReAllocA( dst, *dstLength );

	return dst;
}

void *PlDecompress_LZRW1( const void *src, size_t srcLength, size_t *dstLength ) {
	if ( *( int * ) src == LZRW1_FLAG_COPY ) {
		*dstLength = srcLength - LZRW1_FLAG_BYTES;
		void *dst = PL_NEW_( uint8_t, *dstLength );
		memcpy( dst, src + LZRW1_FLAG_BYTES, *dstLength );
		return dst;
	}

	unsigned int allocSize = ( *dstLength < srcLength ) ? srcLength : *dstLength;
	uint8_t *dst = PL_NEW_( uint8_t, allocSize ), *p_dst = dst;

	const uint8_t *p_src = src + LZRW1_FLAG_BYTES, *p_src_post = src + srcLength;
	uint16_t controlbits = 0, control;
	while ( p_src != p_src_post ) {
		if ( controlbits == 0 ) {
			control = *p_src++;
			control |= ( *p_src++ ) << 8;
			controlbits = 16;
		}

		if ( control & 1 ) {
			uint16_t offset = ( *p_src & 0xF0 ) << 4;
			uint16_t len = 1 + ( *p_src++ & 0xF );
			offset += *p_src++ & 0xFF;
			uint8_t *p = p_dst - offset;
			while ( len-- ) {
				*p_dst++ = *p++;
				*dstLength = p_dst - dst;
				if ( *dstLength >= allocSize ) {
					dst = PlReAllocA( dst, allocSize += 512 );
				}
			}
		} else {
			*p_dst++ = *p_src++;
			*dstLength = p_dst - dst;
			if ( *dstLength >= allocSize ) {
				dst = PlReAllocA( dst, allocSize += 512 );
			}
		}

		control >>= 1;
		controlbits--;
	}

	/* now downscale it */
	dst = PlReAllocA( dst, *dstLength );

	return dst;
}
