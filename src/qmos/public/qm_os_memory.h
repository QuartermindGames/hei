// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>

#pragma once

#include "qm_os.h"

/////////////////////////////////////////////////////////////////////////////////////
// Memory
/////////////////////////////////////////////////////////////////////////////////////

#if defined( __cplusplus )
extern "C"
{
#endif

	typedef enum QmOsMemoryFailType
	{
		QM_OS_MEMORY_FAIL_TYPE_ALLOC,
		QM_OS_MEMORY_FAIL_TYPE_REALLOC,
	} QmOsMemoryFailType;

	extern QM_OS_EXPORT void *( *qmOsMemoryCAllocCallback )( size_t num, size_t size );
	extern QM_OS_EXPORT void *( *qmOsMemoryReAllocCallback )( void *ptr, size_t newSize );
	extern QM_OS_EXPORT void ( *qmOsMemoryFreeCallback )( void *ptr );
	extern QM_OS_EXPORT void ( *qmOsMemoryFailCallback )( size_t size, QmOsMemoryFailType type );

	void *qm_os_memory_alloc( size_t num, size_t size, void ( *destructor )( void *ptr ) );
	void *qm_os_memory_realloc( void *ptr, size_t newSize );
	void  qm_os_memory_free( void *ptr );

#define QM_OS_MEMORY_MALLOC( SIZE, DESTRUCTOR ) qm_os_memory_alloc( 1, ( SIZE ), DESTRUCTOR )
#define QM_OS_MEMORY_MALLOC_( SIZE )            QM_OS_MEMORY_MALLOC( SIZE, NULL )
#define QM_OS_MEMORY_CALLOC( NUM, SIZE )        qm_os_memory_alloc( NUM, SIZE, NULL )

#define QM_OS_MEMORY_NEW( TYPE )               ( TYPE * ) QM_OS_MEMORY_MALLOC_( sizeof( TYPE ) )
#define QM_OS_MEMORY_NEW_( TYPE, NUM )         ( TYPE * ) qm_os_memory_alloc( NUM, sizeof( TYPE ), NULL )
#define QM_OS_MEMORY_NEW_D( TYPE, DESTRUCTOR ) ( TYPE * ) QM_OS_MEMORY_MALLOC( sizeof( TYPE ), DESTRUCTOR )

	/**
	 * Returns the size of an allocated block of memory,
	 * if allocated through our memory manager.
	 *
	 * @param ptr Pointer to allocated memory.
	 * @return Either the size of the block, or zero on fail.
	 */
	size_t qm_os_memory_get_block_size( void *ptr );

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

	/////////////////////////////////////////////////////////////////////////////////////
	// Heap
	// For quick temporary memory allocation.

	typedef struct QmOsMemoryHeap QmOsMemoryHeap;

	/**
	 * Returns an allocated memory heap pool, with reserved size available.
	 * Call memory_free to destroy.
	 *
	 * @param reserve	Amount of memory to reserve in the pool
	 * @return			Returns memory heap pool on success, otherwise null on fail.
	 */
	QmOsMemoryHeap *qm_os_memory_heap_create( size_t reserve );

	/**
	 * Clears the heap; mind that this does not zero all memory,
	 * just the first byte and relocates current position to the start.
	 *
	 * @param self		Heap to flush.
	 */
	void qm_os_memory_heap_flush( QmOsMemoryHeap *self );

	/**
	 * Returns the remaining space available in the heap.
	 *
	 * @param self		Heap to query.
	 * @return			Remaining size in bytes.
	 */
	size_t qm_os_memory_heap_get_remaining( const QmOsMemoryHeap *self );

	/**
	 * Allocates space in the heap based on the given size.
	 *
	 * @param self		Heap to alloc.
	 * @param size		Size to alloc.
	 * @return			Pointer to allocated storage.
	 */
	void *qm_os_memory_heap_alloc( QmOsMemoryHeap *self, size_t size );

	/////////////////////////////////////////////////////////////////////////////////////

#if defined( __cplusplus )
};
#endif
