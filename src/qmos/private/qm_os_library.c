// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>
// Purpose: OS library/dll management abstraction.
// Author:  Mark E. Sowden

#include "qmos/public/qm_os_library.h"

#include "qmos/public/qm_os_memory.h"
#include "qmos/public/qm_os_string.h"

#if QM_OS_SYSTEM == QM_OS_SYSTEM_MACOS
#	include <dlfcn.h>
#elif QM_OS_SYSTEM == QM_OS_SYSTEM_WINDOWS
#	include <windows.h>
#elif QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX
#	include <dlfcn.h>
#endif

QmOsLibrary *qm_os_library_load( const char *path, bool appendPath )
{
	char *fullPath;
	if ( ( fullPath = qm_os_string_alloc( "%s%s", path, appendPath ? QM_OS_SYSTEM_LIB_EXT : "" ) ) == nullptr )
	{
		return nullptr;
	}

#if defined( WIN32 )
	HMODULE libraryHandle = LoadLibrary( sysPath );
	//if ( libraryHandle == NULL )
	//{
	//	PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to load library (%d)", GetLastError() );
	//}
#else /* unix */
	void *libraryHandle = dlopen( fullPath, RTLD_LAZY );
	//if ( libraryHandle == NULL )
	//{
	//	PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to load library (%s)", dlerror() );
	//}
#endif

	qm_os_memory_free( fullPath );

	return libraryHandle;
}

void *qm_os_library_get_procedure( QmOsLibrary *self, const char *procedureName )
{
	void *myProcedure;

#if defined( WIN32 )
	myProcedure = GetProcAddress( ( HMODULE ) self, procedureName );
	//if ( myProcedure == NULL )
	//{
	//	PlReportErrorF( PL_RESULT_INVALID_PARM2, "failed to find procedure \"%s\" (%d)", procedureName, GetLastError() );
	//}
#else /* unix */
	myProcedure = dlsym( self, procedureName );
	//if ( myProcedure == NULL )
	//{
	//	PlReportErrorF( PL_RESULT_INVALID_PARM2, "failed to find procedure \"%s\" (%s)", procedureName, dlerror() );
	//}
#endif

	return myProcedure;
}

void qm_os_library_unload( QmOsLibrary *self )
{
#if defined( WIN32 )
	if ( !FreeLibrary( ( HMODULE ) self ) )
	{
		//PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to unload library (%d)", GetLastError() );
	}
#else /* unix */
	if ( dlclose( self ) != 0 )
	{
		//PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to unload library (%s)", dlerror() );
	}
#endif
}
