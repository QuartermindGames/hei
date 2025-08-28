// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: String manipulation.
// Author:  Mark E. Sowden

#include "qmos/public/qm_os.h"
#include "qmos/public/qm_os_memory.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

char *qm_os_string_alloc( size_t *size, const char *format, ... )
{
	va_list v;
	va_start( v, format );
	int n = vsnprintf( nullptr, 0, format, v );
	va_end( v );

	if ( n < 0 )
	{
		return nullptr;
	}

	size_t sn = n + 1;

	char *string = qm_os_memory_alloc( sn, sizeof( char ), nullptr );
	if ( string == nullptr )
	{
		return nullptr;
	}

	va_start( v, format );
	n = vsnprintf( string, sn, format, v );
	va_end( v );

	if ( n < 0 )
	{
		qm_os_memory_free( string );
		string = nullptr;
	}

	if ( size != nullptr )
	{
		*size = sn;
	}

	return string;
}

static int decimal_to_binary( const int value )
{
	if ( value == 0 )
	{
		return 0;
	}

	return value % 2 + 10 * decimal_to_binary( value / 2 );
}

char *qm_os_string_convert_int( int value, char *dst, const size_t dstSize, const int base )
{
	const char *format;
	switch ( base )
	{
		default:
			return nullptr;
		case 10:
			format = "%d";
			break;
		case 16:
			format = "%x";
			break;
		case 8:
			format = "%o";
			break;
		case 2:
			value  = decimal_to_binary( value );
			format = "%d";
			break;
	}

	snprintf( dst, dstSize, format, value );

	return dst;
}

size_t qm_os_string_count( const char *s, const char c, const size_t size )
{
	size_t num = 0;
	for ( size_t i = 0; i < size; ++i )
	{
		if ( s[ i ] == '\0' )
		{
			break;
		}

		if ( s[ i ] != c )
		{
			continue;
		}

		num++;
	}

	return num;
}

char *qm_os_string_reverse( char *s, const size_t size )
{
	for ( size_t i = 0, j = strnlen( s, size ) - 1; i < j; ++i, j-- )
	{
		const char c = s[ i ];
		if ( c == '\0' )
		{
			break;
		}

		s[ i ] = s[ j ];
		s[ j ] = c;
	}

	return s;
}

int qm_os_string_alnum( const char *s, const size_t size )
{
	for ( size_t i = 0; i < size; ++i )
	{
		if ( s[ i ] == '\0' )
		{
			break;
		}

		if ( isalnum( s[ i ] ) )
		{
			return i;
		}
	}

	return -1;
}

int qm_os_string_alpha( const char *s, const size_t size )
{
	for ( size_t i = 0; i < size; ++i )
	{
		if ( s[ i ] == '\0' )
		{
			break;
		}

		if ( isalpha( s[ i ] ) )
		{
			return i;
		}
	}

	return -1;
}

int qm_os_string_digit( const char *s, const size_t size )
{
	for ( size_t i = 0; i < size; ++i )
	{
		if ( s[ i ] == '\0' )
		{
			break;
		}

		if ( !( ( i == 0 && s[ i ] == '-' && isdigit( s[ i + 1 ] ) ) || isdigit( s[ i ] ) ) )
		{
			return i;
		}
	}

	return -1;
}

char *qm_os_string_to_upper( char *s, const size_t size )
{
	for ( size_t i = 0; i < size; ++i )
	{
		if ( s[ i ] == '\0' )
		{
			break;
		}

		s[ i ] = ( char ) toupper( s[ i ] );
	}

	return s;
}

char *qm_os_string_to_lower( char *s, const size_t size )
{
	for ( size_t i = 0; i < size; ++i )
	{
		if ( s[ i ] == '\0' )
		{
			break;
		}

		s[ i ] = ( char ) tolower( s[ i ] );
	}

	return s;
}
