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
#include "window_private.h"
#include "graphics/graphics_private.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct PLWindow {
	PLSharedWindowState	sharedState;
	Window				winHandle;
} PLWindow;

PLWindow *plCreateWindow( int w, int h, const char *title ) {
	/* before we do anything, check whether or not the graphics
	 * subsystem has been initialised */
	if (	/* for now only support OpenGL modes here */
			gfx_layer.mode != PL_GFX_MODE_OPENGL &&
			gfx_layer.mode != PL_GFX_MODE_OPENGL_CORE &&
			gfx_layer.mode != PL_GFX_MODE_OPENGL_1_0 &&
			gfx_layer.mode != PL_GFX_MODE_OPENGL_ES ) {
		ReportError( PL_RESULT_FAIL, "invalid graphics mode, please set graphics mode" );
		return NULL;
	}

	Display *displayPtr = XOpenDisplay( NULL );
	if ( displayPtr == NULL ) {
		ReportError( PL_RESULT_FAIL, "failed to open display" );
		return NULL;
	}

	/* fetch the "root window", aka the desktop */
	Window rootHandle = XDefaultRootWindow( displayPtr );
}
