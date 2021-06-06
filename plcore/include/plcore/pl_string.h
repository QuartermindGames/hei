/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <stdarg.h>

#if 0
inline static void plGetStringExtension(char *out, const char *in, unsigned int length) {
    const char *s = strrchr(in, '.');type
    if(!s || s[0] == '\0') {
        return;
    }

    strncpy(out, s + 1, length);
}

inline static void plStripStringExtension(char *out, const char *in, unsigned int length) {
    const char *s = strrchr(in, '.');
    while(in < s) {
        *out++ = *in++;
    }
    *out = 0;
}
#endif

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

char *PlStrInsert( const char *string, char **buf, size_t *bufSize, size_t *maxBufSize );

/**
 * http://www.cse.yorku.ca/~oz/hash.html#sdbm
 */
static inline unsigned long PlStrHash_sdbm( const unsigned char *str ) {
	unsigned long hash = 0;
	int c;
	while( ( c = *str++ ) ) {
		hash = c + ( hash << 6 ) + ( hash << 16 ) - hash;
	}

	return hash;
}
