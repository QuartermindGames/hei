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

#include <plcore/pl_parse.h>
#include <plcore/pl_math.h>

bool PlIsEndOfLine( const char **p ) {
    if ( *( *p ) == '\n' || *( *p ) == '\r' ) {
        return true;
    }

    return false;
}

void PlSkipWhitespace( const char **p ) {
	while ( *( *p ) == ' ' ) ( *p )++;
	if ( *( *p ) == ' ' ) ++( *p );
}

void PlSkipLine( const char **p ) {
	while ( *( *p ) != '\0' && *( *p ) != '\n' && *( *p ) != '\r' ) ( *p )++;
	if ( *( *p ) == '\r' ) ( *p )++;
	if ( *( *p ) == '\n' ) ( *p )++;
}

const char *PlParseEnclosedString( const char **p, char *dest, size_t size ) {
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

const char *PlParseToken( const char **p, char *dest, size_t size ) {
	PlSkipWhitespace( p );

	size_t i = 0;
	while ( *( *p ) != '\0' &&
	        *( *p ) != '\n' &&
	        *( *p ) != '\r' &&
	        *( *p ) != ' ' ) {
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

int PlParseInteger( const char **p, bool *status ) {
	*status = false;

	char num[ 64 ];
	if ( !PlParseToken( p, num, sizeof( num ) ) ) {
		return 0;
	}

	int v = strtol( num, NULL, 10 );
	/* todo: validate strtol succeeded! */

	*status = true;

	return v;
}

float PlParseFloat( const char **p, bool *status ) {
	*status = false;

	char num[ 64 ];
	if ( !PlParseToken( p, num, sizeof( num ) ) ) {
		return 0.0f;
	}

	float v = strtof( num, NULL );
	/* todo: validate strtof succeeded! */

	*status = true;

	return v;
}

PLVector3 PlParseVector( const char **p, bool *status ) {
	PlSkipWhitespace( p );
	if ( *( *p ) == '(' ) { ( *p )++; }
	float x = PlParseFloat( p, status );
	float y = PlParseFloat( p, status );
	float z = PlParseFloat( p, status );
	if ( *( *p ) == ')' ) { ( *p )++; }
	return PLVector3( x, y, z );
}
