/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

PL_EXTERN_C

extern PL_DLL void *(*pl_malloc)(size_t size);
extern PL_DLL void *(*pl_calloc)(size_t num, size_t size);
extern PL_DLL void *(*pl_realloc)(void* ptr, size_t newSize);
extern PL_DLL void (*pl_free)(void* ptr);

extern uint64_t PlGetTotalSystemMemory( void );
extern uint64_t PlGetTotalAvailableSystemMemory( void );
extern uint64_t PlGetCurrentMemoryUsage( void );

PL_EXTERN_C_END
