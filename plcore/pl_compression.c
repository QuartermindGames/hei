// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2020-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#include <plcore/pl_compression.h>

#define MINIZ_NO_ARCHIVE_APIS
#include "3rdparty/miniz/miniz.h"

/*
 * Expose miniz functionality through the platform library,
 * so other libraries don't need to also include it.
 */

void *PlCompress_Deflate( const void *src, size_t srcLength, size_t *dstLength ) {
	unsigned long compressedLength = mz_compressBound( ( mz_ulong ) srcLength );
	void *compressedData = PL_NEW_( char, compressedLength );
	int status = mz_compress( compressedData, &compressedLength, src, ( mz_ulong ) srcLength );
	if ( status != MZ_OK ) {
		PlReportErrorF( PL_RESULT_FAIL, "failed to compress data: %s", mz_error( status ) );
		PL_DELETE( compressedData );
		return NULL;
	}

	*dstLength = compressedLength;
	return compressedData;
}

void *PlDecompress_Deflate( const void *src, size_t srcLength, size_t *dstLength, bool raw ) {
	uint8_t *dst = PL_NEW_( uint8_t, srcLength );

	mz_stream stream;
	PL_ZERO_( stream );
	stream.next_in = src;
	stream.avail_in = srcLength;
	stream.next_out = dst;
	stream.avail_out = *dstLength;

	int status = mz_inflateInit2( &stream, raw ? -MZ_DEFAULT_WINDOW_BITS : MZ_DEFAULT_WINDOW_BITS );
	if ( status == MZ_OK ) {
		status = mz_inflate( &stream, MZ_FINISH );
		if ( status == MZ_STREAM_END ) {
			*dstLength = stream.total_out;
			if ( ( status = mz_inflateEnd( &stream ) ) != MZ_OK ) {
				PL_DELETEN( dst );
			}
		} else {
			mz_inflateEnd( &stream );
			PL_DELETEN( dst );
		}
	} else {
		PL_DELETEN( dst );
	}

	if ( dst == NULL ) {
		PlReportErrorF( PL_RESULT_FILEERR, "decompression failed (%s)", zError( status ) );
	}

	return dst;
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

#if defined( __GNUC__ )
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wunused-value"
#endif
void *PlCompress_LZRW1( const void *src, size_t srcLength, size_t *dstLength ) {
	const uint8_t *p_src_post = ( uint8_t * ) src + srcLength;
	const uint8_t *p_src_max1 = p_src_post - LZRW1_ITEM_MAX;
	const uint8_t *p_src_max16 = p_src_post - 16 * LZRW1_ITEM_MAX;
	const uint8_t *hash[ 4096 ];
	uint16_t control = 0, control_bits = 0;

	uint8_t *dst = PL_NEW_( uint8_t, srcLength + LZRW1_FLAG_BYTES );
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
#if defined( __GNUC__ )
#	pragma GCC diagnostic pop
#endif

void *PlDecompress_LZRW1( const void *src, size_t srcLength, size_t *dstLength ) {
	if ( *( int * ) src == LZRW1_FLAG_COPY ) {
		*dstLength = srcLength - LZRW1_FLAG_BYTES;
		void *dst = PL_NEW_( uint8_t, *dstLength );
		memcpy( dst, ( char * ) src + LZRW1_FLAG_BYTES, *dstLength );
		return dst;
	}

	static const unsigned int padSize = 512;
	unsigned int allocSize = ( unsigned int ) srcLength + padSize;
	uint8_t *dst = PL_NEW_( uint8_t, allocSize );
	uint8_t *p_dst = dst;

	const uint8_t *p_src = ( uint8_t * ) src + LZRW1_FLAG_BYTES;
	const uint8_t *p_src_post = ( uint8_t * ) src + srcLength;
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
					dst = PlReAllocA( dst, allocSize += padSize );
					p_dst = dst + *dstLength;
				}
			}
		} else {
			*p_dst++ = *p_src++;
			*dstLength = p_dst - dst;
			if ( *dstLength >= allocSize ) {
				dst = PlReAllocA( dst, allocSize += padSize );
				p_dst = dst + *dstLength;
			}
		}

		control >>= 1;
		controlbits--;
	}

	/* now downscale it */
	dst = PlReAllocA( dst, *dstLength );

	return dst;
}
