/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>

typedef enum PLCompressionType {
	PL_COMPRESSION_NONE,
	PL_COMPRESSION_UNKNOWN,
	PL_COMPRESSION_DEFLATE,
	PL_COMPRESSION_GZIP,
	PL_COMPRESSION_IMPLODE,

	PL_COMPRESSION_LZRW1,

	PL_MAX_COMPRESSION_FORMATS
} PLCompressionType;

PL_EXTERN_C

void *PlCompress_Deflate( const void *src, size_t srcLength, size_t *dstLength );
void *PlCompress_LZRW1( const void *src, size_t length, size_t *outLength );

void *PlDecompress_LZRW1( const void *src, size_t srcLength, size_t *dstLength );

PL_EXTERN_C_END
