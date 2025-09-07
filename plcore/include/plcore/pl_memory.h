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
