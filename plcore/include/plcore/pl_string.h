/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <inttypes.h>

PL_EXTERN_C

#define PL_FMT_float   "%f"
#define PL_FMT_double  "%lf"
#define PL_FMT_int16   "%" PRId16
#define PL_FMT_uint16  "%" PRIu16
#define PL_FMT_int32   "%" PRId32
#define PL_FMT_uint32  "%" PRIu32
#define PL_FMT_int64   "%" PRId64
#define PL_FMT_uint64  "%" PRIu64
#define PL_FMT_hex     "%x"
#define PL_FMT_string  "%s"
#define PL_FMT_address "%" PRIdPTR

char *pl_itoa( int val, char *buf, size_t len, int base );

char *pl_strtolower( char *s );
char *pl_strntolower( char *s, size_t n );
char *pl_strtoupper( char *s );
char *pl_strntoupper( char *s, size_t n );

char *pl_strcasestr( const char *s, const char *find );

int pl_strcasecmp( const char *s1, const char *s2 );
int pl_strncasecmp( const char *s1, const char *s2, size_t n );

int pl_strisalpha( const char *s );
int pl_strnisalpha( const char *s, unsigned int n );
int pl_strisalnum( const char *s );
int pl_strnisalnum( const char *s, unsigned int n );
int pl_strisdigit( const char *s );
int pl_strnisdigit( const char *s, unsigned int n );

int pl_vscprintf( const char *format, va_list pArgs );

unsigned int pl_strcnt( const char *s, char c );
unsigned int pl_strncnt( const char *s, char c, unsigned int n );

char *pl_strchunksplit( const char *string, unsigned int length, const char *seperator );
char *pl_strinsert( const char *string, char **buf, size_t *bufSize, size_t *maxBufSize );
char *pl_strnjoin( const char *a, size_t aSize, const char *b, size_t bSize );
char *pl_strjoin( const char *a, const char *b );

char *pl_strnreverse( char *string, size_t size );

char *pl_strrstr( char *haystack, const char *needle );

size_t pl_strlcat( char *dst, const char *src, size_t size );

/**
 * http://www.cse.yorku.ca/~oz/hash.html#sdbm
 */
static inline uint32_t PlGenerateHashSDBM( const char *str ) {
	uint32_t hash = 0;
	int c;
	while ( ( c = *str++ ) ) {
		hash = c + ( hash << 6 ) + ( hash << 16 ) - hash;
	}

	return hash;
}

static inline uint64_t PlGenerateHashFNV1( const void *str, size_t size ) {
	static const uint64_t offset = 14695981039346656037UL;
	static const uint64_t prime = 1099511628211UL;
	uint64_t hash = offset;
	for ( const char *p = ( const char * ) str;; p++ ) {
		hash ^= ( uint64_t ) ( unsigned char ) ( *p );
		hash *= prime;
		if ( --size == 0 ) {
			break;
		}
	}
	return hash;
}

PL_EXTERN_C_END
