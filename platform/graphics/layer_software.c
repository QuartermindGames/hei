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
#include "graphics_private.h"

#define DEFAULT_WIDTH   320
#define DEFAULT_HEIGHT  240

uint8_t *sw_backbuffer = NULL;
unsigned int sw_backbuffer_size = 0;

void SWClearBuffers(unsigned int buffers) {
    if(buffers & PL_BUFFER_COLOUR) {
        for(unsigned int i = 0; i < sw_backbuffer_size; i += 4) {
            sw_backbuffer[i]        = gfx_state.current_clearcolour.r;
            sw_backbuffer[i + 1]    = gfx_state.current_clearcolour.g;
            sw_backbuffer[i + 2]    = gfx_state.current_clearcolour.b;
            sw_backbuffer[i + 3]    = gfx_state.current_clearcolour.a;
        }
    }
}

/***********************************************/

void _InitSoftware(void) {

}

void ShutdownSoftware(void) {
    free(sw_backbuffer);
    sw_backbuffer_size = 0;
}