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

/*
Platform Library

This library includes standard platform headers,
gives you some standard functions to interact with
the system and includes defines for basic data-types that
you can use in your applications for easier multi-platform
support.

 Anything that uses an underscore before it's name
 is intended only for internal usage!
*/

#define PL_VERSION  200824

// PL_IGNORE_SHARED_HEADERS
// PL_IGNORE_PLATFORM_HEADERS
// PL_IGNORE_STD_HEADERS
// PL_IGNORE_VIDEO_HEADERS

#define PL_INCLUDE_STD_BOOL

// Shared headers
#ifndef PL_IGNORE_SHARED_HEADERS

#	include <stdio.h>
#	include <stdlib.h>
#	include <stdarg.h>
#	include <stdlib.h>

#	ifdef PL_INCLUDE_STD_BOOL

#		include <stdbool.h>

#	endif

#	include <stdint.h>
#	include <string.h>
#	include <ctype.h>
#	include <math.h>
#	include <setjmp.h>
#	include <time.h>

#	include <sys/stat.h>
#	include <sys/types.h>

// C++
#	ifndef PL_IGNORE_STD_HEADERS
#		ifdef __cplusplus
#			include <cstdint>
#			include <memory>
#			include <string>
#			include <vector>
#			include <set>
#			include <unordered_set>
#			include <map>
#			include <unordered_map>
#			include <algorithm>

// istream
#			include <fstream>
#			include <iostream>
#		endif
#	endif
#endif

#ifdef __cplusplus
#	define	PL_EXTERN_C     extern "C" {
#	define	PL_EXTERN_C_END }
#else
#	define  PL_EXTERN_C
#	define  PL_EXTERN_C_END
#endif

#include "platform_system.h"

#if defined(_DEBUG)
#   include <assert.h>

#   define plAssert(a) assert(a)
#else
#   define plAssert(a) (a)
#endif

#define plUnused( ... )	( void )( __VA_ARGS__ )

#define plArrayElements(a)  (sizeof(a) / sizeof(*(a)))          // Returns the number of elements within an array.
#define plIsEmptyString(a)  (((a)[0] == '\0') || ((a)[0] == ' '))

#define PL_STRINGIFY(num)   #num
#define PL_TOSTRING( A )    PL_STRINGIFY( A )

#define PL_BITFLAG( A, B ) A = ( 1 << B )

#ifndef offsetof
#   define pl_offsetof(a, b)    ((size_t)&(((a*)0)->b))
#else
#   define pl_offsetof(a, b)    offsetof(a, b)
#endif

typedef enum PLVariableType {
    pl_float_var,
    pl_int_var,
    pl_string_var,
    pl_bool_var,    // 0,1 true,false
} PLVariableType;

//////////////////////////////////////////////////////////////////

// Error return values
typedef enum {
    PL_RESULT_SUCCESS,
    PL_RESULT_FAIL,
    PL_RESULT_UNSUPPORTED,

    PL_RESULT_INVALID_PARM1,    // Invalid function parameter 1
    PL_RESULT_INVALID_PARM2,    // Invalid function parameter 2
    PL_RESULT_INVALID_PARM3,    // Invalid function parameter 3
    PL_RESULT_INVALID_PARM4,    // Invalid function parameter 4

    // FILE I/O
    PL_RESULT_FILEREAD,     // Failed to read file!
    PL_RESULT_FILEWRITE,    // Failed to write file!
    PL_RESULT_FILETYPE,     // Unexpected file type!
    PL_RESULT_FILEVERSION,  // Unsupported version!
    PL_RESULT_FILESIZE,     // Invalid file size!
    PL_RESULT_FILEPATH,     // Invalid path!
    PL_RESULT_FILEERR,      // Generic Filesystem error

    // CONSOLE
    PL_RESULT_COMMAND_NAME,     // Invalid name provided for command!
    PL_RESULT_COMMAND_FUNCTION, // Invalid callback provided for command!

    // GRAPHICS
    PL_RESULT_GRAPHICSINIT,             // Graphics failed to initialise!
    PL_RESULT_INVALID_SHADER_TYPE,      /* shader type passed is not a valid type */
    PL_RESULT_UNSUPPORTED_SHADER_TYPE,  /* shader type not supported by hardware */
    PL_RESULT_SHADER_COMPILE,           // Failed to compile shader!
    PL_RESULT_SHADER_LINK,              /* shader program failed link */
    PL_RESULT_DRAW_MODE,                // Invalid mesh draw mode!
    PL_RESULT_DRAW_PRIMITIVE,           // Invalid mesh primitive!

    // IMAGE
    PL_RESULT_IMAGERESOLUTION,  // Invalid image resolution!
    PL_RESULT_IMAGEFORMAT,      // Unsupported image format!

    // MEMORY
    PL_RESULT_MEMORY_ALLOCATION,    // Ran out of memory!
    PL_RESULT_MEMORY_UNDERFLOW,
    PL_RESULT_MEMORY_EOA,           // End of array

    PL_RESULT_SYSERR,         // Generic system error
} PLresult;

//////////////////////////////////////////////////////////////////

enum {
	PL_BITFLAG( PL_SUBSYSTEM_GRAPHICS, 0 ),
	PL_BITFLAG( PL_SUBSYSTEM_IO, 1 ),
};

#if defined(PL_INTERNAL)
#   define PL_DLL  PL_EXPORT

// C Exceptions, Inspired by the following
// http://www.di.unipi.it/~nids/docs/longjump_try_trow_catch.html
#if 1
#   define plFunctionStart()
#   define plFunctionEnd()
#else

#include <setjmp.h>

#ifdef __cplusplus

    plFunctionStart() try { \
        plResetError(); plSetErrorFunction(PL_FUNCTION)
    plFunctionEnd() catch(...) { }

#else

#define plFunctionStart() do { jmp_buf ex_buf__; if(!setjmp(ex_buf__)) { \
    plResetError(); plSetErrorFunction(PL_FUNCTION)
#define plFunctionEnd() } else { } } } while(0)

#endif

#endif

#else
#   define PL_DLL  PL_IMPORT
#endif

typedef void PLLibrary; /* handle to dll/module */

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN PLresult plInitialize(int argc, char **argv);
PL_EXTERN PLresult plInitializeSubSystems(unsigned int subsystems);
PL_EXTERN void plShutdown(void);

// todo, kill start
PL_EXTERN void _plResetError(void); // Resets the error message to "null", so you can ensure you have the correct message from the library.
PL_EXTERN void
SetErrorMessage(const char *msg, ...);   // Sets the error message, so we can grab it outside the library.
PL_EXTERN void
plSetCurrentFunction(const char *function, ...);  // Sets the currently active function, for error reporting.
// kill end

PL_EXTERN PLresult plGetFunctionResult(void);
PL_EXTERN const char *plGetResultString(PLresult result);

// todo, kill start
PL_EXTERN const char * plGetError(void);        // Returns the last recorded error.
// kill end

PL_EXTERN const char *plGenerateUniqueIdentifier( char *dest, size_t destLength );

// CL Arguments
PL_EXTERN const char *plGetCommandLineArgumentValue(const char *arg);
PL_EXTERN bool plHasCommandLineArgument(const char *arg);

//////////////////////////////////////////////////////////////////

PL_EXTERN void pl_crc32(const void *data, size_t n_bytes, uint32_t *crc);

//////////////////////////////////////////////////////////////////

bool plIsSubSystemActive(unsigned int subsystem);

//////////////////////////////////////////////////////////////////

/* standard headers */
#include <PL/platform_string.h>
#include <PL/platform_memory.h>

PL_EXTERN const char *plGetFormattedTime(void);
PL_EXTERN time_t plStringToTime( const char *ts );

//////////////////////////////////////////////////////////////////

PL_EXTERN bool plIsRunning(void);

PL_EXTERN double plGetDeltaTime(void);

PL_EXTERN void plProcess(double delta);

PL_EXTERN PLLibrary *plLoadLibrary( const char *path, bool appendPath );
PL_EXTERN void *plGetLibraryProcedure( PLLibrary *library, const char *procedureName );
PL_EXTERN void plUnloadLibrary( PLLibrary *library );

/**
 * Plugin Interface
 **/

PL_EXTERN bool plRegisterPlugin( const char *path );
PL_EXTERN void plRegisterPlugins( const char *pluginDir );
PL_EXTERN void plInitializePlugins( void );

#endif

PL_EXTERN_C_END
