/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl.h>

/**
 * Splits a string up by the given seperator, every length characters.
 * Returns a newly allocated buffer on success.
 */
char *pl_strchunksplit( const char *string, unsigned int insLength, const char *seperator ) {
	size_t sl = strlen( string );
	size_t pl = strlen( seperator );
	char *dest = ( char * ) PlMAllocA( ( sl + ( pl * ( sl / insLength ) ) ) + 1 );
	char *p = dest;
	for ( size_t i = 0, j = 1; i < sl; ++i, ++j ) {
		*p++ = string[ i ];
		if ( j == insLength ) {
			j = 0;
			strcpy( p, seperator );
			p += pl;
		}
	}
	return dest;
}

/**
 * Inserts the given string into an existing string buffer.
 * Automatically reallocs buffer if it doesn't fit.
 */
char *pl_strinsert( const char *string, char **buf, size_t *bufSize, size_t *maxBufSize ) {
	/* check if it's going to fit first */
	size_t strLength = strlen( string );
	size_t originalSize = *bufSize;
	*bufSize += strLength;
	if ( *bufSize >= *maxBufSize ) {
		*maxBufSize = *bufSize + strLength;
		*buf = PlReAllocA( *buf, *maxBufSize );
	}

	/* now copy it into our buffer */
	strncpy( *buf + originalSize, string, strLength );

	return *buf + originalSize + strLength;
}

/**
 * Joins two strings together; returns a newly allocated null-terminated buffer.
 */
char *pl_strnjoin( const char *a, size_t aSize, const char *b, size_t bSize ) {
	char *buf = PL_NEW_( char, aSize + bSize + 1 );
	strncpy( buf, a, aSize );
	strncpy( buf + aSize, b, bSize );
	return buf;
}

/**
 * Joins two strings together; returns a newly allocated null-terminated buffer.
 */
char *pl_strjoin( const char *a, const char *b ) {
	return pl_strnjoin( a, strlen( a ), b, strlen( b ) );
}

char *pl_strnreverse( char *string, size_t size ) {
	for ( size_t i = 0, j = strnlen( string, size ) - 1; i < j; ++i, j-- ) {
		char c = string[ i ];
		if ( c == '\0' )
			break;

		string[ i ] = string[ j ];
		string[ j ] = c;
	}

	return string;
}

char *pl_strrstr( char *haystack, const char *needle ) {
	size_t ns = strlen( needle );
	if ( ns == 0 ) {
		return haystack;
	}

	size_t hs = strlen( haystack );
	if ( ns > hs ) {
		return NULL;
	}

	for ( ssize_t i = hs - ns; i >= 0; --i ) {
		char *p = &haystack[ i ];
		if ( strncmp( p, needle, ns ) != 0 ) {
			continue;
		}

		return p;
	}

	return NULL;
}

size_t pl_strlcat( char *dst, const char *src, size_t size ) {
	size_t dsize = strlen( dst );
	size_t ssize = strlen( src );
	size_t avail = size > dsize ? size - dsize - 1 : 0;
	if ( avail > 0 ) {
		size_t cpy = ssize < avail ? ssize : avail;
		memcpy( dst + dsize, src, cpy );
		dst[ dsize + cpy ] = '\0';
	}

	return dsize + ssize;
}
