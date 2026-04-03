// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>
// Purpose: API for dealing with threads.
// Author:  Mark E. Sowden

#include "qmos/public/qm_os.h"

#if ( QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX )
#	include <unistd.h>
#endif

unsigned int qm_os_thread_get_available()
{
#if ( QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX )
	return sysconf( _SC_NPROCESSORS_ONLN );
#else
#	error "Unimplemented!"
#endif
}
