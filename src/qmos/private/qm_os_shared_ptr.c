// Copyright © 2020-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Memory management system (pulled from Core)
// Author:  Mark E. Sowden

#include <assert.h>

#include "qmos/public/qm_os_shared_ptr.h"
#include "qmos/public/qm_os_memory.h"

/////////////////////////////////////////////////////////////////////////////////////
// Shared Pointer
/////////////////////////////////////////////////////////////////////////////////////

typedef struct QmOsSharedPtr
{
	int   numRefs;
	void *ptr;
} QmOsSharedPtr;

QmOsSharedPtr *qm_os_shared_ptr_create( void *ptr )
{
	QmOsSharedPtr *ref = QM_OS_MEMORY_NEW( QmOsSharedPtr );
	ref->ptr           = ptr;
	ref->numRefs       = 1;
	return ref;
}

void qm_os_shared_ptr_add( QmOsSharedPtr *self )
{
	self->numRefs++;
}

bool qm_os_shared_ptr_release( QmOsSharedPtr *self )
{
	assert( self->numRefs != 0 );

	self->numRefs--;
	if ( self->numRefs == 0 )
	{
		assert( self->ptr == nullptr );

		qm_os_memory_free( self );
		return true;
	}

	return false;
}

void *qm_os_shared_ptr_get( const QmOsSharedPtr *self )
{
	return self->ptr;
}

void qm_os_shared_ptr_set( QmOsSharedPtr *self, void *ptr )
{
	self->ptr = ptr;
}
