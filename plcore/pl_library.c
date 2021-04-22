/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#if defined( WIN32 )
#	include <Windows.h>
#endif

#include "pl_private.h"
#include "filesystem_private.h"

/**
 * Loads the specified library module.
 * If appendPath is true, the system-specific extension is automatically added.
 */
PLLibrary *PlLoadLibrary( const char *path, bool appendPath ) {
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

void *PlGetLibraryProcedure( PLLibrary *library, const char *procedureName ) {
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

void PlUnloadLibrary( PLLibrary *library ) {
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
