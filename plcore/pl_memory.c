/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */
/****************************************
 * MEMORY MANAGEMENT
 ****************************************/

#include "qmos/public/qm_os_memory.h"


#include <stdlib.h>
#if defined( _WIN32 )
#	include <windows.h>
#	include <psapi.h>
#elif defined( __linux__ )
#	include <sys/resource.h>
#elif defined( __FreeBSD__ )
#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/resource.h>
#endif

#include "pl_private.h"

#include <plcore/pl_linkedlist.h>

/****************************************
 * Heap
 * For quick temporary memory allocation
 ****************************************/

typedef struct PLMemoryHeap {
	void *store;
	void *pos;
	size_t size;
} PLMemoryHeap;

void PlFlushHeap( PLMemoryHeap *heap ) {
	heap->pos = heap->store;
	*( char * ) heap->pos = '\0';
}

PLMemoryHeap *PlCreateHeap( size_t reserve ) {
	PLMemoryHeap *heap = QM_OS_MEMORY_NEW( PLMemoryHeap );
	heap->store = QM_OS_MEMORY_NEW_( char, reserve );
	heap->pos = heap->store;
	heap->size = reserve;
	return heap;
}

void PlDestroyHeap( PLMemoryHeap *heap ) {
	qm_os_memory_free( heap->store );
	qm_os_memory_free( heap );
}

size_t PlGetAvailableHeapSize( const PLMemoryHeap *heap ) {
	return heap->size - ( ( char * ) heap->pos - ( char * ) heap->store );
}

void *PlHeapAlloc( PLMemoryHeap *heap, size_t size ) {
	size_t end = ( ( ( char * ) heap->pos - ( char * ) heap->store ) + size );
	if ( end >= heap->size ) {
		PlReportErrorF( PL_RESULT_MEMORY_ALLOCATION, "requested size will not fit in heap" );
		return NULL;
	}

	void *p = heap->pos;
	heap->pos = ( char * ) heap->pos + size;
	return p;
}

/****************************************
 * Memory Allocation Groups
 ****************************************/

typedef struct PLMemoryGroup {
	PLLinkedList *allocations;
	size_t totalSize;
} PLMemoryGroup;

void PlFlushMemoryGroup( PLMemoryGroup *group ) {
	PLLinkedListNode *node = PlGetFirstNode( group->allocations );
	while ( node != NULL ) {
		qm_os_memory_free( PlGetLinkedListNodeUserData( node ) );
		node = PlGetNextLinkedListNode( node );
	}
	PlDestroyLinkedListNodes( group->allocations );
	group->totalSize = 0;
}

PLMemoryGroup *PlCreateMemoryGroup( void ) {
	PLMemoryGroup *group = QM_OS_MEMORY_NEW( PLMemoryGroup );
	group->allocations = PlCreateLinkedList();
	return group;
}

void PlDestroyMemoryGroup( PLMemoryGroup *group ) {
	PlFlushMemoryGroup( group );
	PlDestroyLinkedList( group->allocations );
	qm_os_memory_free( group );
}

size_t PlGetMemoryGroupSize( PLMemoryGroup *group ) {
	return group->totalSize;
}

void *PlGroupAlloc( PLMemoryGroup *group, size_t size ) {
	void *p = QM_OS_MEMORY_MALLOC_( size );
	PlInsertLinkedListNode( group->allocations, p );
	group->totalSize += size;
	return p;
}
