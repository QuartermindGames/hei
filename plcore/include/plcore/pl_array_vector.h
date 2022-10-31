/**
 * Hei Platform Library
 * Copyright (C) 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>

PL_EXTERN_C

typedef struct PLVectorArray PLVectorArray;

void PlResizeVectorArray( PLVectorArray *vectorArray, unsigned int newMaxElements );
void **PlGetVectorArrayData( PLVectorArray *vectorArray );
void PlShrinkVectorArray( PLVectorArray *vectorArray );
void PlPopVectorArrayBack( PLVectorArray *vectorArray );
void PlEraseVectorArrayElement( PLVectorArray *vectorArray, unsigned int at );
void PlClearVectorArray( PLVectorArray *vectorArray );
void PlPushBackVectorArrayElement( PLVectorArray *vectorArray, void *value );

unsigned int PlGetNumVectorArrayElements( const PLVectorArray *vectorArray );
unsigned int PlGetMaxVectorArrayElements( const PLVectorArray *vectorArray );

bool PlIsVectorArrayEmpty( const PLVectorArray *vectorArray );

void *PlGetVectorArrayElementAt( PLVectorArray *vectorArray, unsigned int at );
void *PlGetVectorArrayFront( PLVectorArray *vectorArray );
void *PlGetVectorArrayBack( PLVectorArray *vectorArray );

PLVectorArray *PlCreateVectorArray( unsigned int maxElements );
void PlDestroyVectorArray( PLVectorArray *vectorArray );

PL_EXTERN_C_END
