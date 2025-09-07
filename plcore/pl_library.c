/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#if defined( WIN32 )
#	include <windows.h>
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
		snprintf( sysPath, sizeof( sysPath ), "%s", path );
	}

	/* check that it actually exists first, since Windows doesn't give a very verbose message for these cases */
	if ( !PlLocalFileExists( sysPath ) ) {
		PlReportErrorF( PL_RESULT_FILEPATH, "failed to find library, \"%s\"", sysPath );
		return NULL;
	}

#if defined( WIN32 )
	HMODULE libraryHandle = LoadLibrary( sysPath );
	if ( libraryHandle == NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to load library (%d)", GetLastError() );
	}
#else /* unix */
	void *libraryHandle = dlopen( sysPath, RTLD_LAZY );
	if ( libraryHandle == NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to load library (%s)", dlerror() );
	}
#endif

	return libraryHandle;
}

void *PlGetLibraryProcedure( PLLibrary *library, const char *procedureName ) {
	void *myProcedure;

#if defined( WIN32 )
	myProcedure = GetProcAddress( ( HMODULE ) library, procedureName );
	if ( myProcedure == NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "failed to find procedure \"%s\" (%d)", procedureName, GetLastError() );
	}
#else /* unix */
	myProcedure = dlsym( library, procedureName );
	if ( myProcedure == NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "failed to find procedure \"%s\" (%s)", procedureName, dlerror() );
	}
#endif

	return myProcedure;
}

void PlUnloadLibrary( PLLibrary *library ) {
#if defined( WIN32 )
	if ( !FreeLibrary( ( HMODULE ) library ) ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to unload library (%d)", GetLastError() );
	}
#else /* unix */
	if ( dlclose( library ) != 0 ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to unload library (%s)", dlerror() );
	}
#endif
}
