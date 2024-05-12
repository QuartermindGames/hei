/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

PL_EXTERN_C

/* lower-level function pointers
 * these point to the standard C functions by default
 * DO NOT USE DIRECTLY, use *Alloc and *Free functions instead! */
extern PL_DLL void *( *pl_malloc )( size_t size );
extern PL_DLL void *( *pl_calloc )( size_t num, size_t size );
extern PL_DLL void *( *pl_realloc )( void *ptr, size_t newSize );
extern PL_DLL void ( *pl_free )( void *ptr );

/* callback for catching out of memory errors
 * only gets called if calling with abort on error */
typedef void ( *PLMemoryAbortCallbackT )( size_t failSize );
extern PL_DLL PLMemoryAbortCallbackT pl_memory_abort_cb;

void *PlCAlloc( size_t num, size_t size, bool abortOnFail );
void *PlMAlloc( size_t size, bool abortOnFail );
void *PlReAlloc( void *ptr, size_t newSize, bool abortOnFail );
void PlFree( void *ptr );

extern size_t PlGetTotalAllocatedMemory( void );

extern uint64_t PlGetTotalSystemMemory( void );
extern uint64_t PlGetTotalAvailableSystemMemory( void );
extern uint64_t PlGetCurrentMemoryUsage( void );

/* todo: deprecate these... */
#define PlCAllocA( NUM, SIZE )     PlCAlloc( ( NUM ), ( SIZE ), true )
#define PlMAllocA( SIZE )          PlMAlloc( ( SIZE ), true )
#define PlReAllocA( PTR, NEWSIZE ) PlReAlloc( ( PTR ), ( NEWSIZE ), true )

#define PL_CALLOCA( NUM, SIZE )     PlCAlloc( ( NUM ), ( SIZE ), true )
#define PL_MALLOCA( SIZE )          PlMAlloc( ( SIZE ), true )
#define PL_REALLOCA( PTR, NEWSIZE ) PlReAlloc( ( PTR ), ( NEWSIZE ), true )

#define PL_NEW( TYPE )       ( TYPE * ) PL_MALLOCA( sizeof( TYPE ) )
#define PL_NEW_( TYPE, NUM ) ( TYPE * ) PL_CALLOCA( NUM, sizeof( TYPE ) )
#define PL_DELETE( PTR )     PlFree( PTR )
#define PL_DELETEN( PTR )     \
	{                         \
		PL_DELETE( ( PTR ) ); \
		( PTR ) = NULL;       \
	}

typedef struct PLMemoryGroup PLMemoryGroup;
void PlFlushMemoryGroup( PLMemoryGroup *group );
PLMemoryGroup *PlCreateMemoryGroup( void );
void PlDestroyMemoryGroup( PLMemoryGroup *group );
size_t PlGetMemoryGroupSize( PLMemoryGroup *group );
void *PlGroupAlloc( PLMemoryGroup *group, size_t size );

#define PL_GNEW( GROUP, TYPE )       ( TYPE * ) PlGroupAlloc( GROUP, sizeof( TYPE ) )
#define PL_GNEW_( GROUP, TYPE, NUM ) ( TYPE * ) PlGroupAlloc( GROUP, sizeof( TYPE ) * NUM )

typedef struct PLMemoryHeap PLMemoryHeap;
void PlFlushHeap( PLMemoryHeap *heap );
PLMemoryHeap *PlCreateHeap( size_t reserve );
void PlDestroyHeap( PLMemoryHeap *heap );
size_t PlGetAvailableHeapSize( const PLMemoryHeap *heap );
void *PlHeapAlloc( PLMemoryHeap *heap, size_t size );

#define PL_DEFAULT_HEAP_SIZE     0x3D09000                            /* should be 64mb */
#define PL_CREATE_DEFAULT_HEAP() PlCreateHeap( PL_DEFAULT_HEAP_SIZE ) /* init with default */

#define PL_HNEW( HEAP, TYPE )       ( TYPE * ) PlHeapAlloc( HEAP, sizeof( TYPE ) )
#define PL_HNEW_( HEAP, TYPE, NUM ) ( TYPE * ) PlHeapAlloc( HEAP, sizeof( TYPE ) * NUM )

PL_EXTERN_C_END
