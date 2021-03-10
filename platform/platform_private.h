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

#include <PL/platform.h>

#if defined(_MSC_VER)
#pragma warning ( disable : 4204 )
#pragma warning ( disable : 4820 )
#pragma warning ( disable : 4668 )
#endif

// OS Specific Headers
#if defined(_WIN32)
//#define WIN32_LEAN_AND_MEAN 1
//#include <windows.h>
//#include <afxres.h>
//#define STRSAFE_NO_DEPRECATE 1
//#include <strsafe.h>
#undef far
#undef near
#endif

/* Hard Limits */

#define MAX_OBJECT_INTERFACES   512

/*********************************/

extern int LOG_LEVEL_LOW, LOG_LEVEL_MEDIUM, LOG_LEVEL_HIGH, LOG_LEVEL_DEBUG;
extern int LOG_LEVEL_GRAPHICS;
extern int LOG_LEVEL_FILESYSTEM;
extern int LOG_LEVEL_MODEL;

void plLogMessage( int level, const char* msg, ... );

#define Print(...)				plLogMessage(LOG_LEVEL_LOW, __VA_ARGS__)
#define PrintWarning( ... )		plLogMessage(LOG_LEVEL_MEDIUM, __VA_ARGS__)
#define PrintError( ... )		plLogMessage(LOG_LEVEL_HIGH, __VA_ARGS__)
#ifdef _DEBUG
#   define debug_printf(...)    printf(__VA_ARGS__)
#   define DebugPrint(...)      plLogMessage(LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#   define debug_printf(...)
#   define DebugPrint(...)      Print(__VA_ARGS__)
#endif

void plSetFunctionResult(PLresult result);

#define FunctionStart() \
    _plResetError()

#define ReportError( type, ... ) \
    plSetCurrentFunction(PL_FUNCTION); \
    plSetFunctionResult(type); \
    SetErrorMessage(__VA_ARGS__)
#define ReportBasicError( type ) \
    ReportError((type), plGetResultString((type)))

#define SetResult(r)    ReportError(r, plGetResultString((r)))

/* * * * * * * * * * * * * * * * * * * */
/* Sub Systems                         */

PLresult plInitGraphics(void);
void plShutdownGraphics(void);

PLresult plInitFileSystem(void);
void plShutdownFileSystem(void);

PLresult plInitConsole(void);
void plShutdownConsole(void);

void _plInitPackageSubSystem(void);
void _plInitModelSubSystem(void);

/* * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32

const char *GetLastError_strerror(uint32_t errnum);

#else

int GetLastError( void );
const char* GetLastError_strerror( int errnum );

#define WSAGetLastError() GetLastError()
#define WSAGetLastError_strerror( errnum ) GetLastError_strerror(errnum)

#endif

/* * * * * * * * * * * * * * * * * * * */
/* Console Utilities                   */

#define IMPLEMENT_COMMAND( NAME, DESC ) \
    static void NAME ## _func(unsigned int argc, char *argv[]); \
    static PLConsoleCommand NAME ## _var = {#NAME, NAME ## _func, DESC}; \
    static void NAME ## _func(unsigned int argc, char *argv[])
