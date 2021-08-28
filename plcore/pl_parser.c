/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_parse.h>
#include <plcore/pl_math.h>

bool PlIsEndOfLine( const char **p ) {
	if ( *( *p ) == '\n' || *( *p ) == '\r' ) {
		return true;
	}

	return false;
}

bool PlIsWhitespace( const char **p ) {
	return ( *( *p ) == ' ' || *( *p ) == '\t' );
}

void PlSkipWhitespace( const char **p ) {
	if ( !PlIsWhitespace( p ) ) {
		return;
	}

	do {
        ( *p )++;
	} while( PlIsWhitespace( p ) );
}

#define NOT_TERMINATING_CHAR( P ) ( ( P ) != '\0' && ( P ) != '\n' && ( P ) != '\r' )

void PlSkipLine( const char **p ) {
	while ( NOT_TERMINATING_CHAR( *( *p ) ) ) ( *p )++;
	if ( *( *p ) == '\r' ) ( *p )++;
	if ( *( *p ) == '\n' ) ( *p )++;
}

const char *PlParseEnclosedString( const char **p, char *dest, size_t size ) {
	bool isEnclosed = false;
	if ( *( *p ) == '\"' ) {
		( *p )++;
		isEnclosed = true;
	}
	size_t i = 0;
	while ( NOT_TERMINATING_CHAR( *( *p ) ) ) {
		if ( !isEnclosed && ( *( *p ) == ' ' || *( *p ) == '\t' || *( *p ) == '\n' || *( *p ) == '\r' ) ) {
			break;
		} else if ( isEnclosed && *( *p ) == '\"' ) {
			( *p )++;
			break;
		}

		if ( ( i + 1 ) < size ) {
			dest[ i++ ] = *( *p );
		}
		( *p )++;
	}

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
	if ( status != NULL ) *status = false;

	char num[ 64 ] = { '\0' };
	if ( !PlParseToken( p, num, sizeof( num ) ) ) {
		return 0;
	}

	int v = strtol( num, NULL, 10 );
	/* todo: validate strtol succeeded! */

	if ( status != NULL ) *status = true;

	return v;
}

float PlParseFloat( const char **p, bool *status ) {
	if ( status != NULL ) *status = false;

	char num[ 64 ];
	if ( !PlParseToken( p, num, sizeof( num ) ) ) {
		return 0.0f;
	}

	float v = strtof( num, NULL );
	/* todo: validate strtof succeeded! */

	if ( status != NULL ) *status = true;

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
