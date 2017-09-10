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

#include <PL/platform_console.h>
#include <PL/platform_graphics.h>
#include <PL/platform_filesystem.h>
#include <PL/platform_window.h>

/* Example of console API, minus error handling :) */

#define TITLE   "Console"
#define WIDTH   1024
#define HEIGHT  768

void Shutdown(void) {
    plShutdown();

    exit(0);
}

#if 0
void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(action != GLFW_PRESS) {
        return;
    }

    switch(key) {
        default: break;

        case GLFW_KEY_ESCAPE: {
            Shutdown();
        }

        case GLFW_KEY_TAB: {
            plShowConsole(!plIsConsoleVisible());
            break;
        }

        case GLFW_KEY_1: {
            plSetupConsole(1);
            break;
        }
        case GLFW_KEY_2: {
            plSetupConsole(2);
            break;
        }
        case GLFW_KEY_3: {
            plSetupConsole(3);
            break;
        }
        case GLFW_KEY_4: {
            plSetupConsole(4);
            break;
        }
    }
}
#endif

int main(int argc, char **argv) {
    plInitialize(argc, argv, PL_SUBSYSTEM_GRAPHICS | PL_SUBSYSTEM_WINDOW);

    PLWindow *window = plCreateWindow(TITLE, 0, 0, WIDTH, HEIGHT);
    if(!window) {
        printf(plGetError());
        return -1;
    }

    plSetupConsole(1);
    plShowConsole(true);

    plSetConsoleColour(1, plCreateColour4b(128, 128, 0, 128));
    plSetConsoleColour(2, plCreateColour4b(0, 128, 0, 128));
    plSetConsoleColour(3, plCreateColour4b(0, 0, 128, 128));
    plSetConsoleColour(4, plCreateColour4b(0, 0, 0, 128));

    plSetDefaultGraphicsState();
    plSetClearColour(plCreateColour4b(0, 0, 0, 255));

    PLCamera *camera = plCreateCamera();
    if(camera == NULL) {
        exit(EXIT_FAILURE);
    }

    camera->mode            = PLCAMERA_MODE_ORTHOGRAPHIC;
    camera->viewport.w      = window->width;
    camera->viewport.h      = window->height;

#if 1
    while(plIsRunning()) {
        plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH | PL_BUFFER_STENCIL);

        plSetupCamera(camera);

        // todo, input??

        plDrawConsole();

        plSwapBuffers(window);
    }

    Shutdown();
#endif
    return EXIT_SUCCESS;
}