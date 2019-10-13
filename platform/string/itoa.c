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

#include <stdio.h>

int pl_dectobin(int dec) {
    if(dec == 0) {
        return 0;
    }

    return (dec % 2 + 10 * pl_dectobin(dec / 2));
}

// lazy ass implementation ...
char *pl_itoa(int val, char *buf, size_t len, int base) {
    switch(base) {
        default:return buf;
        case 10:
			snprintf(buf, len, "%d", val);
            break;
        case 16:
			snprintf(buf, len, "%x", val);
            break;
        case 8:
			snprintf(buf, len, "%o", val);
            break;
        case 2:
            val = pl_dectobin(val);
			snprintf(buf, len, "%d", val);
            break;
    }

    return buf;
}
