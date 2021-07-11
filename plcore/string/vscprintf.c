/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <stdio.h>
#include <stdarg.h>

/* https://stackoverflow.com/a/19692380 */
int pl_vscprintf( const char *format, va_list pArgs ) {
	va_list argCopy;
	va_copy( argCopy, pArgs );
	int retVal = vsnprintf( NULL, 0, format, argCopy );
	va_end( argCopy );
	return retVal;
}
