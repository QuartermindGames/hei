/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <inttypes.h>

PL_EXTERN_C

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
