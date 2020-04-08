/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include "platform_private.h"

#if defined( _WIN32 )
#	include <Windows.h>
#endif

typedef struct PLWindow {
	int				x, y;
	unsigned int	w, h;
	char			windowTitle[ 32 ];
} PLWindow;

#if defined( _WIN32 )
static LRESULT WindowCallbackProcedure( HWND windowHandle, unsigned int msg, WPARAM wParam, LPARAM lParam ) {

}
#endif

PLWindow *plCreateWindow( int w, int h, const char *title ) {
#if defined( _WIN32 )
	HINSTANCE instance = GetModuleHandle( 0 );
	if( instance == NULL ) {
		ReportError( PL_RESULT_FAIL, "failed to fetch valid module handle" );
		return NULL;
	}

	/* create the window class */
	WNDCLASS windowClass;
	memset( &windowClass, 0, sizeof( WNDCLASS ) );
	windowClass.lpfnWndProc		= WindowCallbackProcedure;
	windowClass.lpszClassName	= title;
	windowClass.hInstance		= instance;

	RegisterClass( &windowClass );

	/* and now create the window */

	HWND windowInstance = CreateWindowEx(
		0,
		title,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, w, h,

		NULL,
		NULL,
		instance,
		NULL
	);
	if( windowInstance == NULL ) {
		ReportError( PL_RESULT_FAIL, "failed to create window" );
		return NULL;
	}

	ShowWindow( windowInstance, SW_SHOW );
#endif
}
