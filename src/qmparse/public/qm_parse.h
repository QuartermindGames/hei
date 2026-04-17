// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>
// Purpose: A very simple support library for string parsing.
// Author:  Mark E. Sowden

#pragma once

#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum QmParseLineEnd : int8_t
{
	QM_PARSE_LINE_END_INVALID = -1,
	QM_PARSE_LINE_END_EOF,
	QM_PARSE_LINE_END_LF,
	QM_PARSE_LINE_END_CRLF,
} QmParseLineEnd;

#define QM_PARSE_NOT_TERMINATING_CHAR( P ) ( ( P ) != '\0' && ( P ) != '\n' && ( P ) != '\r' )

#if defined( __cplusplus )
extern "C"
{
#endif

	static inline int qm_parse_get_line_end_type( const char *p )
	{
		if ( *p == '\0' )
		{
			return QM_PARSE_LINE_END_EOF;
		}

		if ( *p == '\n' )
		{
			return QM_PARSE_LINE_END_LF;
		}

		if ( *p == '\r' && *( p + 1 ) == '\n' )
		{
			return QM_PARSE_LINE_END_CRLF;
		}

		return QM_PARSE_LINE_END_INVALID;
	}

	static inline bool qm_parse_is_end_of_line( const char *p )
	{
		return qm_parse_get_line_end_type( p ) != QM_PARSE_LINE_END_INVALID;
	}

	static inline bool qm_parse_is_whitespace( const char *p )
	{
		// don't treat line end as whitespace
		if ( qm_parse_is_end_of_line( p ) )
		{
			return false;
		}

		return isspace( *p ) || *p == '\t';
	}

	static inline void qm_parse_skip_whitespace( const char **p )
	{
		if ( !qm_parse_is_whitespace( *p ) )
		{
			return;
		}

		do
		{
			( *p )++;
		} while ( qm_parse_is_whitespace( *p ) );
	}

	static inline void qm_parse_skip_line( const char **p )
	{
		while ( QM_PARSE_NOT_TERMINATING_CHAR( *( *p ) ) ) ( *p )++;
		if ( **p == '\r' ) ( *p )++;
		if ( **p == '\n' ) ( *p )++;
	}

	static inline unsigned int qm_parse_get_enclosed_length( const char *p )
	{
		qm_parse_skip_whitespace( &p );

		const char *s          = p;
		bool        isEnclosed = false;
		if ( *p == '\"' )
		{
			p++;
			isEnclosed = true;
		}

		while ( QM_PARSE_NOT_TERMINATING_CHAR( *p ) )
		{
			if ( !isEnclosed && ( *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' ) )
			{
				break;
			}

			if ( isEnclosed && *p == '\"' )
			{
				p++;
				break;
			}

			p++;
		}

		return p - s;
	}

	static inline const char *qm_parse_enclosed( const char **p, char *dest, size_t size )
	{
		qm_parse_skip_whitespace( p );

		bool isEnclosed = false;
		if ( **p == '\"' )
		{
			( *p )++;
			isEnclosed = true;
		}

		size_t i = 0;
		while ( QM_PARSE_NOT_TERMINATING_CHAR( *( *p ) ) )
		{
			if ( !isEnclosed && ( **p == ' ' || **p == '\t' || **p == '\n' || **p == '\r' ) )
			{
				break;
			}

			if ( isEnclosed && **p == '\"' )
			{
				( *p )++;
				break;
			}

			if ( i + 1 < size )
			{
				dest[ i++ ] = **p;
			}
			( *p )++;
		}

		dest[ i ] = '\0';
		return dest;
	}

	/**
	 * Returns the potential length of a token in a buffer.
	 */
	static inline unsigned int qm_parse_get_token_length( const char *p )
	{
		qm_parse_skip_whitespace( &p );

		const char *s = p;
		while ( *p != '\0' && *p != ' ' )
		{
			if ( qm_parse_is_end_of_line( p ) )
			{
				break;
			}

			p++;
		}

		return p - s;
	}

	static inline const char *qm_parse_token( const char **p, char *dest, size_t size )
	{
		qm_parse_skip_whitespace( p );

		unsigned int length = qm_parse_get_token_length( ( *p ) );
		size_t       i;
		for ( i = 0; i < length; ++i )
		{
			if ( i + 1 >= size )
			{
				break;
			}

			dest[ i ] = ( *p )[ i ];
		}

		*p += length;

		dest[ i ] = '\0';
		return dest;
	}

	static inline int qm_parse_integer( const char **p, bool *status )
	{
		char num[ 64 ];
		if ( qm_parse_token( p, num, sizeof( num ) ) == nullptr )
		{
			if ( status != nullptr )
			{
				*status = false;
			}
			return 0;
		}

		if ( status != nullptr )
		{
			*status = true;
		}

		return ( int ) strtol( num, nullptr, 10 );
	}

	static inline float qm_parse_float( const char **p, bool *status )
	{
		char num[ 64 ];
		if ( qm_parse_token( p, num, sizeof( num ) ) == nullptr )
		{
			if ( status != nullptr )
			{
				*status = false;
			}
			return 0.0f;
		}

		if ( status != nullptr )
		{
			*status = true;
		}

		return strtof( num, nullptr );
	}

	static inline double qm_parse_double( const char **p, bool *status )
	{
		char num[ 64 ];
		if ( qm_parse_token( p, num, sizeof( num ) ) == nullptr )
		{
			if ( status != nullptr )
			{
				*status = false;
			}
			return 0.0;
		}

		if ( status != nullptr )
		{
			*status = true;
		}

		return strtod( num, nullptr );
	}

	static inline float *qm_parse_vectorfv( const char **p, float *dst, size_t numDstElements )
	{
		qm_parse_skip_whitespace( p );

		for ( unsigned int i = 0; i < numDstElements; ++i )
		{
			bool status;
			dst[ i ] = qm_parse_float( p, &status );
			if ( !status )
			{
				return nullptr;
			}
		}

		return dst;
	}

	/**
	 * Returns the potential length of a line in a
	 * buffer. This is either up to the point of a
	 * new line or alternatively, a null-terminator,
	 * whichever is encountered first.
	 */
	static inline unsigned int qm_parse_line_length( const char *p )
	{
		unsigned int length = 0;
		while ( QM_PARSE_NOT_TERMINATING_CHAR( *p ) )
		{
			length++;
			p++;
		}

		return length;
	}

	static inline const char *qm_parse_line( const char **p, char *dest, size_t size )
	{
		unsigned int length = qm_parse_line_length( *p );
		size_t       i;
		for ( i = 0; i < length; ++i )
		{
			if ( i + 1 >= size )
			{
				break;
			}

			dest[ i ] = ( *p )[ i ];
		}

		*p += length + qm_parse_get_line_end_type( *p + length );

		dest[ i ] = '\0';
		return dest;
	}

#if defined( __cplusplus )
}
#endif
