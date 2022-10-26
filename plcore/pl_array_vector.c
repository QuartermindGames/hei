/**
 * Hei Platform Library
 * Copyright (C) 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "pl_private.h"

#include <plcore/pl_array_vector.h>

typedef struct PLVectorArray {
	unsigned int numElements;
	unsigned int maxElements;
	void **data;
} PLVectorArray;

void PlResizeVectorArray( PLVectorArray *vectorArray, unsigned int newMaxElements ) {
	if ( newMaxElements == vectorArray->maxElements ) {
		return;
	}

	assert( newMaxElements < vectorArray->maxElements );
	if ( newMaxElements < vectorArray->maxElements ) {
		return;
	}

	vectorArray->data = PlReAllocA( vectorArray->data, sizeof( void * ) * newMaxElements );
	vectorArray->maxElements = newMaxElements;
}

void **PlGetVectorArrayData( PLVectorArray *vectorArray ) {
	return vectorArray->data;
}

void PlShrinkVectorArray( PLVectorArray *vectorArray ) {
	PlResizeVectorArray( vectorArray, vectorArray->numElements );
}

void PlPopVectorArrayBack( PLVectorArray *vectorArray ) {
	vectorArray->data[ vectorArray->numElements ] = NULL;
	vectorArray->numElements--;
}

void PlEraseVectorArrayElement( PLVectorArray *vectorArray, unsigned int at ) {
	for ( unsigned int i = ( at + 1 ); i < vectorArray->numElements; ++i ) {
		vectorArray->data[ i - 1 ] = vectorArray->data[ i ];
	}
	vectorArray->numElements--;
}

/**
 * Clears all elements from the array.
 * Keep in mind this won't free the individual elements.
 */
void PlClearVectorArray( PLVectorArray *vectorArray ) {
	PL_ZERO( vectorArray->data, sizeof( void * ) * vectorArray->numElements );
	vectorArray->numElements = 0;
}

void PlPushBackVectorArrayElement( PLVectorArray *vectorArray, void *value ) {
	if ( vectorArray->numElements + 1 >= vectorArray->maxElements ) {
		PlResizeVectorArray( vectorArray, vectorArray->maxElements + 1 );
	}

	vectorArray->data[ vectorArray->numElements ] = value;
	vectorArray->numElements++;
}

unsigned int PlGetNumVectorArrayElements( const PLVectorArray *vectorArray ) {
	return vectorArray->numElements;
}

unsigned int PlGetMaxVectorArrayElements( const PLVectorArray *vectorArray ) {
	return vectorArray->maxElements;
}

bool PlIsVectorArrayEmpty( const PLVectorArray *vectorArray ) {
	return ( vectorArray->numElements == 0 );
}

void *PlGetVectorArrayElementAt( PLVectorArray *vectorArray, unsigned int at ) {
	if ( at >= vectorArray->numElements ) {
		PlReportErrorF( PL_RESULT_MEMORY_EOA, "at is beyond scope of array" );
		return NULL;
	}

	return vectorArray->data[ at ];
}

void *PlGetVectorArrayFront( PLVectorArray *vectorArray ) {
	return vectorArray->data[ 0 ];
}

void *PlGetVectorArrayBack( PLVectorArray *vectorArray ) {
	return vectorArray->data[ vectorArray->numElements - 1 ];
}

/**
 * Create a vector array.
 */
PLVectorArray *PlCreateVectorArray( unsigned int size ) {
	PLVectorArray *vectorArray = PL_NEW( PLVectorArray );
	vectorArray->maxElements = size;
	vectorArray->data = PL_NEW_( void *, vectorArray->maxElements );
	return vectorArray;
}

void PlDestroyVectorArray( PLVectorArray *vectorArray ) {
	PlClearVectorArray( vectorArray );
	PL_DELETE( vectorArray );
}
