// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Memory management helpers.
// Author:  Mark E. Sowden

#include <assert.h>
#include <stdlib.h>

#include "qmos/public/qm_os_memory.h"

#if ( QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX ) || ( QM_OS_SYSTEM == QM_OS_SYSTEM_MACOS )
#	include <sys/resource.h>
#	include <unistd.h>
#elif ( QM_OS_SYSTEM == QM_OS_SYSTEM_WINDOWS )
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <psapi.h>
#endif

QM_OS_EXPORT void *( *qmOsMemoryCAllocCallback )( size_t num, size_t size )           = calloc;
QM_OS_EXPORT void *( *qmOsMemoryReAllocCallback )( void *ptr, size_t newSize )        = realloc;
QM_OS_EXPORT void ( *qmOsMemoryFreeCallback )( void *ptr )                            = free;
QM_OS_EXPORT void ( *qmOsMemoryFailCallback )( size_t size, QmOsMemoryFailType type ) = nullptr;

static constexpr uint32_t QM_OS_MEMORY_MAGIC = QM_OS_MAGIC_TO_NUM( 'Q', 'M', 'O', 'S' );
typedef struct QmOsMemoryBlockHeader
{
	uint32_t magic;
	size_t   size;
	void ( *destructorCallback )( void *ptr );
} QmOsMemoryBlockHeader;

void *qm_os_memory_alloc( const size_t num, size_t size, void ( *destructor )( void *ptr ) )
{
	// make room for our header
	const size_t totalSize = num * size + sizeof( QmOsMemoryBlockHeader );

	uint8_t *buf = qmOsMemoryCAllocCallback( 1, totalSize );
	if ( buf == nullptr )
	{
		// unlike previous design, caller decides via callback if they want to abort or not
		if ( qmOsMemoryFailCallback != nullptr )
		{
			qmOsMemoryFailCallback( num * size, QM_OS_MEMORY_FAIL_TYPE_ALLOC );
		}

		return nullptr;
	}

	QmOsMemoryBlockHeader *header = ( QmOsMemoryBlockHeader * ) buf;
	header->magic                 = QM_OS_MEMORY_MAGIC;
	header->size                  = num * size;
	header->destructorCallback    = destructor;

	return buf + sizeof( QmOsMemoryBlockHeader );
}

void *qm_os_memory_realloc( void *ptr, size_t newSize )
{
	uint8_t *buf = ptr;
	if ( buf == nullptr )
	{
		return qm_os_memory_alloc( 1, newSize, nullptr );
	}

	QmOsMemoryBlockHeader *header = ( QmOsMemoryBlockHeader * ) ( buf - sizeof( QmOsMemoryBlockHeader ) );
	assert( header->magic == QM_OS_MEMORY_MAGIC );

	// ensure we'll still have room for the header
	newSize += sizeof( QmOsMemoryBlockHeader );
	// and we'll need to resize from the header pos
	buf = ( uint8_t * ) header;

	buf = qmOsMemoryReAllocCallback( buf, newSize );
	if ( buf == nullptr )
	{
		// unlike previous design, caller decides via callback if they want to abort or not
		if ( qmOsMemoryFailCallback != nullptr )
		{
			qmOsMemoryFailCallback( newSize, QM_OS_MEMORY_FAIL_TYPE_REALLOC );
		}

		return nullptr;
	}

	header = ( QmOsMemoryBlockHeader * ) buf;
	assert( header->magic == QM_OS_MEMORY_MAGIC );

	// amend the header size to be the new size
	header->size = newSize;

	// return where the user data is
	return buf + sizeof( QmOsMemoryBlockHeader );
}

void qm_os_memory_free( void *ptr )
{
	uint8_t *buf = ptr;
	if ( buf == nullptr )
	{
		return;
	}

	QmOsMemoryBlockHeader *header = ( QmOsMemoryBlockHeader * ) ( buf - sizeof( QmOsMemoryBlockHeader ) );
	assert( header->magic == QM_OS_MEMORY_MAGIC );

	if ( header->destructorCallback != nullptr )
	{
		header->destructorCallback( ptr );
	}

	buf = ( uint8_t * ) header;

	qmOsMemoryFreeCallback( buf );
}

size_t qm_os_memory_get_block_size( void *ptr )
{
	uint8_t *buf = ptr;

	const QmOsMemoryBlockHeader *header = ( QmOsMemoryBlockHeader * ) ( buf - sizeof( QmOsMemoryBlockHeader ) );
	assert( header != nullptr && header->magic == QM_OS_MEMORY_MAGIC );

	return header->size;
}

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
