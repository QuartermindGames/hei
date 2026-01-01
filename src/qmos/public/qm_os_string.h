// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#include "qm_os.h"

/////////////////////////////////////////////////////////////////////////////////////
// String
/////////////////////////////////////////////////////////////////////////////////////

#if defined( __cplusplus )
extern "C"
{
#endif

	char  *qm_os_string_alloc( size_t *size, const char *format, ... );
	char  *qm_os_string_convert_int( int value, char *dst, size_t dstSize, int base );
	size_t qm_os_string_count( const char *s, char c, size_t size );
	char  *qm_os_string_reverse( char *s, size_t size );
	int    qm_os_string_alnum( const char *s, size_t size );
	int    qm_os_string_alpha( const char *s, size_t size );
	int    qm_os_string_digit( const char *s, size_t size );
	char  *qm_os_string_to_upper( char *s, size_t size );
	char  *qm_os_string_to_lower( char *s, size_t size );

	/**
	 * Splits a string up by the given seperator, every length characters.
	 * Returns a newly allocated buffer on success.
	 */
	char *qm_os_string_split( const char *s, size_t size, const char *sep );

#if defined( __cplusplus )
};
#endif
