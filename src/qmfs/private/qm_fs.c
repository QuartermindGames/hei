// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Foundation of the QM filesystem.
// Author:  Mark E. Sowden

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "qmos/public/qm_os.h"

char *qm_fs_get_temp_path( char *dst, const size_t dstSize )
{
#if ( QM_OS_SYSTEM == QM_OS_SYSTEM_LINUX )

	char dir[] = "/tmp/qmfsXXXXXX";
	if ( mkdtemp( dir ) == nullptr )
	{
		return nullptr;
	}

	snprintf( dst, dstSize, "%s", dir );

#elif ( QM_OS_SYSTEM == QM_OS_SYSTEM_WINDOWS )
	//TODO: this is untested...

	char dir[ 128 ];
	GetTempPath( sizeof( dir ), dir );
	if ( !PlCreateDirectory( dir ) )
	{
		return NULL;
	}

	snprintf( dst, dstSize, "%s", dir );

#else
#	error "Unimplemented!"
#endif

	return dst;
}
