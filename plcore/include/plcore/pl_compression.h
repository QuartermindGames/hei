/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>

typedef enum PLCompressionType {
	PL_COMPRESSION_NONE,
	PL_COMPRESSION_DEFLATE,
	PL_COMPRESSION_GZIP,
	PL_COMPRESSION_IMPLODE,

	PL_COMPRESSION_LZRW1,

	PL_MAX_COMPRESSION_FORMATS
} PLCompressionType;

PL_EXTERN_C

void *PlCompress_Deflate( const void *src, unsigned long srcLength, unsigned long *dstLength );
void *PlCompress_LZRW1( const void *src, unsigned long length, unsigned long *outLength );

void *PlDecompress_LZRW1( const void *src, unsigned long srcLength, unsigned long *dstLength );

PL_EXTERN_C_END
