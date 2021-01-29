/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include <PL/pl_parse.h>
#include <PL/platform_math.h>

void plSkipWhitespace( const char **p ) {
	while ( *( *p ) == ' ' ) ( *p )++;
	if ( *( *p ) == ' ' ) ++( *p );
}

void plSkipLine( const char **p ) {
	while ( *( *p ) != '\0' && *( *p ) != '\n' && *( *p ) != '\r' ) ( *p )++;
	if ( *( *p ) == '\r' ) ( *p )++;
	if ( *( *p ) == '\n' ) ( *p )++;
}

const char *plParseEnclosedString( const char **p, char *dest, size_t size ) {
	if ( *( *p ) == '\"' ) ( *p )++;
	size_t i = 0;
	/* todo!!!
	 * you'll probably notice an issue here. yes, indeed.
	 * if the string is not enclosed, this will iterate until
	 * it hits the end of the buffer. not good. i want to rewrite
	 * this anyway, so i'll review it soooonish. */
	while ( *( *p ) != '\0' && *( *p ) != '\"' ) {
		if ( ( i + 1 ) < size ) dest[ i++ ] = *( *p );
		( *p )++;
	}
	if ( *( *p ) == '\"' ) ( *p )++;
	dest[ i ] = '\0';
	return dest;
}

const char *plParseToken( const char **p, char *dest, size_t size ) {
	plSkipWhitespace( p );

	size_t i = 0;
	while ( *( *p ) != '\0' && *( *p ) != ' ' ) {
		if ( ( i + 1 ) < size ) {
			dest[ i++ ] = *( *p );
		}
		( *p )++;
	}

	if ( *( *p ) == ' ' ) {
		( *p )++;
	}

	dest[ i ] = '\0';
	return dest;
}

int plParseInteger( const char **p ) {
	char num[ 64 ];
	if ( !plParseToken( p, num, sizeof( num ) ) ) {
		printf( "Failed to parse integer!\n" );
		return 0;
	}

	return strtol( num, NULL, 10 );
}

float plParseFloat( const char **p ) {
	char num[ 64 ];
	if ( !plParseToken( p, num, sizeof( num ) ) ) {
		printf( "Failed to parse float!\n" );
		return 0;
	}

	return strtof( num, NULL );
}

PLVector3 plParseVector( const char **p ) {
	plSkipWhitespace( p );
	if ( *( *p ) == '(' ) { ( *p )++; }
	float x = plParseFloat( p );
	float y = plParseFloat( p );
	float z = plParseFloat( p );
	if ( *( *p ) == ')' ) { ( *p )++; }
	return PLVector3( x, y, z );
}
