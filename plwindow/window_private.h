// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plwindow/plw.h>

#if defined( _WIN32 )
#	define WIN32_LEAN_AND_MEAN 1
#	include <Windows.h>
#elif defined( __linux__ )
#	include <X11/Xlib.h>
#endif

typedef struct PLWWindow {
	void *nativeWindow;
} PLWWindow;

void PlwSetError( PLWError error, const char *message, ... );
