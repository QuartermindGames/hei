// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>
// Purpose: Win32 helper functions

#include "pl_private.h"

bool PlOSWin32_IsWine( void ) {
#if defined( _WIN32 )

	static bool cached = false;
	static bool isWine = false;
	if ( cached ) {
		return isWine;
	}

	PLLibrary *library = PlLoadLibrary( "ntdll.dll", false );
	if ( library == NULL ) {
		return false;
	}

	if ( PlGetLibraryProcedure( library, "wine_get_version" ) != NULL ) {
		isWine = true;
	}

	PlUnloadLibrary( library );
	cached = true;

	return isWine;

#else

	return false;

#endif
}
