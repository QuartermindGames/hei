/*	
Copyright (C) 2011-2016 OldTimes Software

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "PL/platform_window.h"
#include "PL/platform_log.h"

#if defined(PL_USE_SDL2)
#   include <SDL2/SDL.h>
#else
#   if defined(__linux__)
#       include <X11/X.h>
#       include <X11/Xlib.h>
#   endif
#endif

/*	Simple Window/Display Handling	*/

#define PL_WINDOW_WIDTH    640
#define PL_WINDOW_HEIGHT   480

#include "PL/platform_log.h"
#define PL_WINDOW_LOG  "pl_window"
#ifdef _DEBUG
#	define plWindowLog(...) plWriteLog(PL_WINDOW_LOG, __VA_ARGS__)
#else
#   define plWindowLog(...)
#endif

void _plShutdownWindow(void) {
#if defined(PL_USE_SDL2)
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
}

///////////////////////////////////////////////////////////

#define DEFAULT_SCREEN_WIDTH    640
#define DEFAULT_SCREEN_HEIGHT   480

unsigned int plGetScreenWidth(void) {
    SDL_Rect screen;
    if(SDL_GetDisplayBounds(0, &screen) != 0) {
        plWindowLog("Failed to get display width!\nSDL2: %s\n", SDL_GetError());
        return DEFAULT_SCREEN_WIDTH;
    }

    return (unsigned int) screen.w;
}

unsigned int plGetScreenHeight(void) {
    SDL_Rect screen;
    if(SDL_GetDisplayBounds(0, &screen) != 0) {
        plWindowLog("Failed to get display height!\nSDL2: %s\n", SDL_GetError());
        return DEFAULT_SCREEN_HEIGHT;
    }

    return (unsigned int) screen.h;
}

///////////////////////////////////////////////////////////

unsigned int plGetScreenCount(void) {
    int screens = SDL_GetNumVideoDisplays();
    if(screens < 1) {
        plWindowLog("Failed to get screen count!\nSDL2: %s\n", SDL_GetError());
        return 0;
    }

    return (unsigned int) screens;
}

///////////////////////////////////////////////////////////

/*	Window Creation */

PLWindow *plCreateWindow(const char *title, int x, int y, unsigned int w, unsigned int h) {
    SDL_Window *sdl_window = SDL_CreateWindow(title, x, y, w, h, SDL_WINDOW_OPENGL);
    if(sdl_window == NULL) {
        _plReportError(PL_RESULT_DISPLAY, "SDL2: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_GLContext context = SDL_GL_CreateContext(sdl_window);
    if(context == NULL) {
        _plReportError(PL_RESULT_DISPLAY, "SDL2: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_GL_MakeCurrent(sdl_window, context);
    if(SDL_GL_SetSwapInterval(-1) == -1) {
        SDL_GL_SetSwapInterval(1);
    }

    PLWindow *window = (PLWindow*)malloc(sizeof(PLWindow));
    if (!window) { // Make sure the window has been initialized.
        _plReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate window! (%d)\n", sizeof(PLWindow));
        return NULL;
    }

    memset(window, 0, sizeof(PLWindow));
    window->sys_id = SDL_GetWindowID(sdl_window);
    window->is_active = true;

    SDL_DisableScreenSaver();

    return window;
}

void plDeleteWindow(PLWindow *window) {
    if(window == NULL) {
        return;
    }

    SDL_Window *sdl_window = SDL_GetWindowFromID(window->sys_id);
    if(sdl_window) {
        SDL_DestroyWindow(sdl_window);
    }

    // todo, remove from global pool
    free(window);
}

///////////////////////////////////////////////////////////

/* Displays a simple dialogue window. */
void plMessageBox(const char *title, const char *msg, ...) {
    char buf[4096];
    va_list args;
    va_start(args, msg);
    vsprintf(buf, msg, args);
    va_end(args);

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, buf, NULL);
}

void plSwapBuffers(PLWindow *window) {
#if defined(PL_USE_SDL2)
    SDL_Window *sdl_window = SDL_GetWindowFromID(window->sys_id);
    if(sdl_window == NULL) {
        return;
    }
    SDL_GL_SwapWindow(sdl_window);
#else
#ifdef _WIN32
    SwapBuffers(window->dc);
#else	// Linux
    //glXSwapBuffers() // todo
#endif
#endif
}