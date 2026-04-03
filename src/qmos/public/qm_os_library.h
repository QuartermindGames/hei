// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>

#pragma once

#include "qm_os.h"

#if defined( __cplusplus )
extern "C"
{
#endif

	typedef void QmOsLibrary;

	/**
	 * Loads the specified library module.
	 * If appendPath is true, the system-specific extension is automatically added.
	 */
	QmOsLibrary *qm_os_library_load( const char *path, bool appendPath );

	void *qm_os_library_get_procedure( QmOsLibrary *self, const char *procedureName );

	void qm_os_library_unload( QmOsLibrary *self );

#if defined( __cplusplus )
};
#endif
