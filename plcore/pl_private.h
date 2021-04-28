/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <plcore/pl.h>

#if defined( _MSC_VER )
#pragma warning( disable : 4204 )
#pragma warning( disable : 4820 )
#pragma warning( disable : 4668 )
#endif

// OS Specific Headers
#if defined( _WIN32 )
//#define WIN32_LEAN_AND_MEAN 1
//#include <windows.h>
//#include <afxres.h>
//#define STRSAFE_NO_DEPRECATE 1
//#include <strsafe.h>
#undef far
#undef near
#endif

/* Hard Limits */

#define MAX_OBJECT_INTERFACES 512

/*********************************/

extern int LOG_LEVEL_LOW, LOG_LEVEL_MEDIUM, LOG_LEVEL_HIGH, LOG_LEVEL_DEBUG;
extern int LOG_LEVEL_FILESYSTEM;

#define Print( FORMAT, ... ) PlLogWFunction( LOG_LEVEL_LOW, FORMAT, ## __VA_ARGS__ )
#define PrintWarning( FORMAT, ... ) PlLogWFunction( LOG_LEVEL_MEDIUM, FORMAT, ## __VA_ARGS__ )
#define PrintError( FORMAT, ... ) PlLogWFunction( LOG_LEVEL_HIGH, FORMAT, ## __VA_ARGS__ )
#if !defined( NDEBUG )
#define debug_printf( ... ) printf( __VA_ARGS__ )
#define DebugPrint( FORMAT, ... ) PlLogWFunction( LOG_LEVEL_DEBUG, FORMAT, ## __VA_ARGS__ )
#else
#define debug_printf( ... )
#define DebugPrint( ... ) Print( __VA_ARGS__ )
#endif

#define FunctionStart() PlClearError()

/* * * * * * * * * * * * * * * * * * * */
/* Sub Systems                         */

PLFunctionResult PlgInitGraphics( void );
void PlgShutdownGraphics( void );

PLFunctionResult PlInitFileSystem( void );
void PlShutdownFileSystem( void );

PLFunctionResult PlInitConsole( void );
void PlShutdownConsole( void );

void PlInitPackageSubSystem( void );

/* * * * * * * * * * * * * * * * * * * */

#ifdef _WIN32

const char *GetLastError_strerror( uint32_t errnum );

#else

int GetLastError( void );
const char *GetLastError_strerror( int errnum );

#define WSAGetLastError() GetLastError()
#define WSAGetLastError_strerror( errnum ) GetLastError_strerror( errnum )

#endif

/* * * * * * * * * * * * * * * * * * * */
/* Console Utilities                   */

#define IMPLEMENT_COMMAND( NAME, DESC )                                \
	static void NAME##_func( unsigned int argc, char *argv[] );        \
	static PLConsoleCommand NAME##_var = { #NAME, NAME##_func, DESC }; \
	static void NAME##_func( unsigned int argc, char *argv[] )
