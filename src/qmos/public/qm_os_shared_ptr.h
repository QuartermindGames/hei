// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#include "qm_os.h"

/////////////////////////////////////////////////////////////////////////////////////
// Shared Ptr
/////////////////////////////////////////////////////////////////////////////////////

#if defined( __cplusplus )
extern "C"
{
#endif

	typedef struct QmOsSharedPtr QmOsSharedPtr;

	QmOsSharedPtr *qm_os_shared_ptr_create( void *ptr );

	void  qm_os_shared_ptr_add( QmOsSharedPtr *self );
	bool  qm_os_shared_ptr_release( QmOsSharedPtr *self );
	void *qm_os_shared_ptr_get( const QmOsSharedPtr *self );
	void  qm_os_shared_ptr_set( QmOsSharedPtr *self, void *ptr );

#if defined( __cplusplus )
};
#endif
