// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Memory management helpers.
// Author:  Mark E. Sowden

#include "qmos/public/qm_os.h"

#if ( QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX ) || ( QM_OS_SYSTEM == QM_OS_SYSTEM_MACOS )
#	include <sys/resource.h>
#	include <unistd.h>
#endif

#include <stdlib.h>

#define MEMORY_MAGIC

static void *default_malloc( const size_t size )
{
	return calloc( 1, size );
}

QM_OS_EXPORT void *( *qm_os_memory_malloc_callback )( size_t size )                = default_malloc;
QM_OS_EXPORT void *( *qm_os_memory_calloc_callback )( size_t num, size_t size )    = calloc;
QM_OS_EXPORT void *( *qm_os_memory_realloc_callback )( void *ptr, size_t newSize ) = realloc;
QM_OS_EXPORT void ( *qm_os_memory_free_callback )( void *ptr )                     = free;

uint64_t qm_os_memory_get_total()
{
#if ( QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX ) || ( QM_OS_SYSTEM == QM_OS_SYSTEM_MACOS )

	const long pages    = sysconf( _SC_PHYS_PAGES );
	const long pageSize = sysconf( _SC_PAGE_SIZE );
	return pages * pageSize;

#elif defined( _WIN32 )

	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof( stat );
	GlobalMemoryStatusEx( &stat );
	return stat.ullTotalPageFile;

#else
#	error "Unimplemented!"
#endif
}

uint64_t qm_os_memory_get_total_available()
{
#if ( QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX ) || ( QM_OS_SYSTEM == QM_OS_SYSTEM_MACOS )

	// unfortunately, on OSX, we don't have AVPHYS_PAGES...
#	if ( QM_OS_SYSTEM == QM_OS_SYSTEM_MACOS )
	const long pages = sysconf( _SC_PHYS_PAGES );
#	else
	const long pages = sysconf( _SC_AVPHYS_PAGES );
#	endif

	const long pageSize = sysconf( _SC_PAGE_SIZE );

	return pages * pageSize;

#elif defined( _WIN32 )

	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof( stat );
	GlobalMemoryStatusEx( &stat );
	return stat.ullAvailPhys;

#else
#	error "Unimplemented!"
#endif
}

uint64_t qm_os_memory_get_usage()
{
#if ( QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX ) || ( QM_OS_SYSTEM == QM_OS_SYSTEM_MACOS )

	/* this is probably the only thing close to an api
	 * that provides this info on linux, without having
	 * to parse input...but it returns kb and isn't
	 * supported everywhere... */
	struct rusage usage;
	getrusage( RUSAGE_SELF, &usage );
	return usage.ru_maxrss * 1000;

#elif ( QM_OS_SYSTEM == QM_OS_SYSTEM_WINDOWS )

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof( pmc ) );
	return pmc.WorkingSetSize;

#else
#	error "Unimplemented!"
#endif
}
