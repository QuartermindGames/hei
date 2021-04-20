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

#if defined( WIN32 )
#	include <Windows.h>
#endif

#include "platform_private.h"
#include "filesystem_private.h"

/**
 * Loads the specified library module.
 * If appendPath is true, the system-specific extension is automatically added.
 */
PLLibrary *plLoadLibrary( const char *path, bool appendPath ) {
	char sysPath[ PL_SYSTEM_MAX_PATH ];
	if ( appendPath ) {
		snprintf( sysPath, sizeof( sysPath ), "%s%s", path, PL_SYSTEM_LIBRARY_EXTENSION );
	} else {
		strncpy( sysPath, path, sizeof( sysPath ) );
	}

	/* check that it actually exists first, since Windows doesn't give a very verbose message for these cases */
	if ( !plLocalFileExists( sysPath ) ) {
		plReportErrorF( PL_RESULT_FILEPATH, "failed to find library, \"%s\"", sysPath );
		return NULL;
	}

#if defined( WIN32 )
	HMODULE libraryHandle = LoadLibrary( sysPath );
	if ( libraryHandle == NULL ) {
		plReportErrorF( PL_RESULT_INVALID_PARM1, "failed to load library (%d)", GetLastError() );
	}
#else /* unix */
	void *libraryHandle = dlopen( sysPath, RTLD_LAZY );
	if ( libraryHandle == NULL ) {
		ReportError( PL_RESULT_INVALID_PARM1, "failed to load library (%s)", dlerror() );
	}
#endif

	return ( PLLibrary* ) libraryHandle;
}

void *plGetLibraryProcedure( PLLibrary *library, const char *procedureName ) {
	void *myProcedure;

#if defined( WIN32 )
	myProcedure = GetProcAddress( ( HMODULE ) library, procedureName );
	if ( myProcedure == NULL ) {
		plReportErrorF( PL_RESULT_INVALID_PARM2, "failed to find procedure (%d)", GetLastError() );
	}
#else /* unix */
	myProcedure = dlsym( library, procedureName );
	if ( myProcedure == NULL ) {
		ReportError( PL_RESULT_INVALID_PARM2, "failed to find procedure (%s)", dlerror() );
	}
#endif

	return myProcedure;
}

void plUnloadLibrary( PLLibrary *library ) {
#if defined( WIN32 )
	if ( !FreeLibrary( ( HMODULE ) library ) ) {
		plReportErrorF( PL_RESULT_INVALID_PARM1, "failed to unload library (%d)", GetLastError() );
	}
#else /* unix */
	if ( dlclose( library ) != 0 ) {
		ReportError( PL_RESULT_INVALID_PARM1, "failed to unload library (%s)", dlerror() );
	}
#endif
}
