// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "window_private.h"

/////////////////////////////////////////////////////////////////////////////////////
// Error Management

static PLWError lastError = PLW_ERROR_NONE;
static char lastErrorMessage[ 1024 ] = { '\0' };

void PlwSetError( PLWError error, const char *message, ... ) {
	lastError = error;

	va_list args;
	va_start( args, message );

	vsnprintf( lastErrorMessage, sizeof( lastErrorMessage ), message, args );

	va_end( args );
}

PLWError PlwGetLastError( void ) {
	return lastError;
}

const char *PlwGetLastErrorMessage( void ) {
	return lastErrorMessage;
}

void PlwClearError( void ) {
	lastError = PLW_ERROR_NONE;
	*lastErrorMessage = '\0';
}

/////////////////////////////////////////////////////////////////////////////////////

#if defined( _WIN32 )
typedef struct NativeWin32WindowHandle {
	HWND window;
	HDC deviceContext;
} NativeWin32WindowHandle;

static LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
	switch ( uMsg ) {
		default: {
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	}
}
#endif

PLWWindow *PlwCreateWindow( const char *title, int width, int height ) {
#if defined( _WIN32 )

	WNDCLASSEX wClass;
	PL_ZERO_( wClass );
	wClass.cbSize = sizeof( WNDCLASSEX );
	wClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wClass.lpfnWndProc = WindowProc;
	EXTERN_C IMAGE_DOS_HEADER __ImageBase;
	wClass.hInstance = ( ( HINSTANCE ) &__ImageBase );
	wClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	wClass.hbrBackground = ( HBRUSH ) ( COLOR_BACKGROUND + 1 );
	wClass.lpszClassName = title;

	if ( !RegisterClassEx( &wClass ) ) {
		PlwSetError( PLW_ERROR_WINDOW_CLASS, "failed to register window class (" PL_FMT_hex ")", GetLastError() );
		return NULL;
	}

	HWND hWnd = CreateWindow(
	        title,
	        title,
	        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	        CW_USEDEFAULT, CW_USEDEFAULT,
	        width, height,
	        NULL,
	        NULL,
	        ( ( HINSTANCE ) &__ImageBase ),
	        NULL );
	if ( hWnd == NULL ) {
		PlwSetError( PLW_ERROR_WINDOW_CREATE, "failed to create window (" PL_FMT_hex ")", GetLastError() );
		return NULL;
	}

	PLWWindow *window = PL_NEW( PLWWindow );

	NativeWin32WindowHandle *nativeWindow = PL_NEW( NativeWin32WindowHandle );
	nativeWindow->window = hWnd;
	window->nativeWindow = nativeWindow;

	return window;

#else

#	error "CreateWindow unsupported!"

#endif
}

void PlwDestroyWindow( PLWWindow *window ) {
	if ( window == NULL ) {
		return;
	}

#if defined( _WIN32 )

	if ( window->nativeWindow != NULL ) {
		DestroyWindow( ( ( NativeWin32WindowHandle * ) window->nativeWindow )->window );
	}
	PL_DELETE( window->nativeWindow );

#endif

	PL_DELETE( window );
}

void PlGetWindowPosition( PLWWindow *window, int *x, int *y ) {
	*x = 0;
	*y = 0;

#if defined( _WIN32 )

	RECT position;
	if ( !GetWindowRect( ( ( NativeWin32WindowHandle * ) window->nativeWindow )->window, &position ) ) {
		PlwSetError( PLW_ERROR_WINDOW_GET, "failed to get window position (" PL_FMT_hex ")", GetLastError() );
		return;
	}

	*x = position.left;
	*y = position.top;

#else

#	error "GetWindowPosition unsupported!"

#endif
}
