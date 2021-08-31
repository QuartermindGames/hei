/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <stdlib.h>
#if defined( _WIN32 )
#include <windows.h>
#include <psapi.h>
#endif

#include "pl_private.h"

static void *MemoryCountAlloc( size_t num, size_t size ) {
	void *buf = calloc( num, size );
	if ( buf == NULL ) {
		PlReportErrorF( PL_RESULT_MEMORY_ALLOCATION, "failed to allocate %lu bytes", ( unsigned long ) size * num );
	}

	return buf;
}

static void *MemoryAlloc( size_t size ) {
	return MemoryCountAlloc( 1, size );
}

static void *MemoryReAlloc( void *ptr, size_t newSize ) {
	void *buf = realloc( ptr, newSize );
	if ( buf == NULL ) {
		PlReportErrorF( PL_RESULT_MEMORY_ALLOCATION, "failed to allocate %lu bytes", newSize );
	}

	return buf;
}

PL_DLL void *( *pl_malloc )( size_t size ) = MemoryAlloc;
PL_DLL void *( *pl_calloc )( size_t num, size_t size ) = MemoryCountAlloc;
PL_DLL void *( *pl_realloc )( void *ptr, size_t newSize ) = MemoryReAlloc;
PL_DLL void ( *pl_free )( void *ptr ) = free;

/**
 * Returns the total amount of system memory in bytes.
 */
uint64_t PlGetTotalSystemMemory( void ) {
#if defined( __linux__ )
	long pages = sysconf( _SC_PHYS_PAGES );
	long pageSize = sysconf( _SC_PAGE_SIZE );
	return pages * pageSize;
#elif defined( _WIN32 )
	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof( stat );
	GlobalMemoryStatusEx( &stat );
	return stat.ullTotalPageFile;
#else
#error "Missing implementation!"
#endif
}

/**
 * Returns the total amount of available system memory in bytes.
 */
uint64_t PlGetTotalAvailableSystemMemory( void ) {
#if defined( __linux__ )
	long pages = sysconf( _SC_AVPHYS_PAGES );
	long pageSize = sysconf( _SC_PAGE_SIZE );
	return pages * pageSize;
#elif defined( _WIN32 )
	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof( stat );
	GlobalMemoryStatusEx( &stat );
	return stat.ullAvailPhys;
#else
#error "Missing implementation!"
#endif
}

/**
 * Returns the memory usage of the current process in bytes.
 */
uint64_t PlGetCurrentMemoryUsage( void ) {
#if defined( __linux__ )
	return 0; /* todo */
#elif defined( _WIN32 )
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo( GetCurrentProcess(), &pmc, sizeof( pmc ) );
	return pmc.WorkingSetSize;
#else
#error "Missing implementation!"
#endif
}
