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

#include <GLFW/glfw3.h>

/* Example of console API, minus error handling :) */

#define TITLE "Console"

int main(int argc, char **argv) {
    glfwInit();

    GLFWwindow *window = glfwCreateWindow(640, 480, TITLE, NULL, NULL);
    glfwMakeContextCurrent(window);

    plInitialize(argc, argv, PL_SUBSYSTEM_GRAPHICS | PL_SUBSYSTEM_CONSOLE);

    plSetupConsole(4);

    plSetConsoleColour(1, plCreateColour4b(128, 0, 0, 128));
    plSetConsoleColour(2, plCreateColour4b(0, 128, 0, 128));
    plSetConsoleColour(3, plCreateColour4b(0, 0, 128, 128));
    plSetConsoleColour(4, plCreateColour4b(0, 0, 0, 128));

    plSetDefaultGraphicsState();

    PLCamera *camera = plCreateCamera();
    camera->mode = PL_CAMERAMODE_ORTHOGRAPHIC;
    glfwGetFramebufferSize(window, (int*)&camera->viewport.width, (int*)&camera->viewport.height);

    while(!glfwWindowShouldClose(window)) {
        plClearBuffers(PL_BUFFER_COLOUR | PL_BUFFER_DEPTH);

        plSetupCamera(camera);
        plDrawConsole();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    plShutdown();

    return 0;
}