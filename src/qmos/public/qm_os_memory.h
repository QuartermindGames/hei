// Copyright © 2020-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#include "qm_os.h"

/////////////////////////////////////////////////////////////////////////////////////
// Memory
/////////////////////////////////////////////////////////////////////////////////////

#if defined( __cplusplus )
extern "C"
{
#endif

	extern QM_OS_EXPORT void *( *qm_os_memory_malloc_callback )( size_t size );
	extern QM_OS_EXPORT void *( *qm_os_memory_calloc_callback )( size_t num, size_t size );
	extern QM_OS_EXPORT void *( *qm_os_memory_realloc_callback )( void *ptr, size_t newSize );
	extern QM_OS_EXPORT void ( *qm_os_memory_free_callback )( void *ptr );

	/**
	 * Get the total memory in bytes.
	 *
	 * @return The total memory in bytes.
	 */
	uint64_t qm_os_memory_get_total();

	/**
	 * Get the total available memory in bytes.
	 *
	 * @return The total *available* memory in bytes.
	 */
	uint64_t qm_os_memory_get_total_available();

	/**
	 * Get the total used memory by the current process in bytes.
	 *
	 * @return The memory usage of the current process in bytes.
	 */
	uint64_t qm_os_memory_get_usage();

#if defined( __cplusplus )
};
#endif
