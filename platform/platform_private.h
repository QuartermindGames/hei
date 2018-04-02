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
#pragma once

enum {
    LOG_LEVEL_HIGH = -1,
    LOG_LEVEL_MEDIUM = -2,
    LOG_LEVEL_LOW = -3,

    LOG_LEVEL_DEBUG = -4,

    LOG_LEVEL_GRAPHICS = -5,
    LOG_LEVEL_FILESYSTEM = -6,
    LOG_LEVEL_MODEL = -7,

    LOG_LEVEL_END = 10
};

#define Print(...)          plLogMessage(LOG_LEVEL_LOW, __VA_ARGS__)
#define PrintWarning(...)   plLogMessage(LOG_LEVEL_MEDIUM, __VA_ARGS__)
#ifdef _DEBUG
#   define DebugPrint(...)      plLogMessage(LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#   define DebugPrint(...)      Print(__VA_ARGS__)
#endif

/* * * * * * * * * * * * * * * * * * * */
/* Sub Systems                         */

PLresult InitGraphics(void);
void ShutdownGraphics(void);

PLresult _InitIO(void);
void _ShutdownIO(void);

PLresult _plInitConsole(void);
void _plShutdownConsole(void);

/* * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32

const char *GetLastError_strerror(uint32_t errnum);

#else

int GetLastError(void);
const char *GetLastError_strerror(int errnum);

#define WSAGetLastError() GetLastError()
#define WSAGetLastError_strerror(errnum) GetLastError_strerror(errnum)

#endif
