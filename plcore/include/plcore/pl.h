// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

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

#define PL_VERSION_MAJOR 1
#define PL_VERSION_MINOR 0
#define PL_VERSION_PATCH 0

// PL_IGNORE_SHARED_HEADERS

#define PL_INCLUDE_STD_BOOL

// Shared headers
#ifndef PL_IGNORE_SHARED_HEADERS
#	include <stdio.h>
#	include <stdlib.h>
#	include <stdarg.h>
#	include <stdlib.h>

#	ifndef __cplusplus
#		ifdef PL_INCLUDE_STD_BOOL
#			include <stdbool.h>
#		endif
#	endif

#	include <stdint.h>
#	include <string.h>
#	include <ctype.h>
#	include <math.h>
#	include <time.h>
#endif

#ifdef __cplusplus
#	define PL_EXTERN_C     extern "C" {
#	define PL_EXTERN_C_END }
#else
#	define PL_EXTERN_C
#	define PL_EXTERN_C_END
#endif

#include <plcore/pl_system.h>

typedef char PLPath[ PL_SYSTEM_MAX_PATH ];

#include <assert.h>

#define PL_UNUSEDVAR( ... ) ( void ) ( __VA_ARGS__ )

#define PL_ARRAY_ELEMENTS( a )  ( sizeof( a ) / sizeof( *( a ) ) )// Returns the number of elements within an array.
#define PL_MAX_ARRAY_INDEX( a ) ( int ) ( PL_ARRAY_ELEMENTS( a ) - 1 )
#define PL_INVALID_STRING( a )  ( a == NULL || *a == '\0' )

#define PL_ZERO( DATA, SIZE ) memset( ( DATA ), 0, ( SIZE ) )
#define PL_ZERO_( DATA )      memset( &( DATA ), 0, sizeof( ( DATA ) ) )

#define PL_MIN( A, B ) ( ( A ) < ( B ) ? ( A ) : ( B ) )
#define PL_MAX( A, B ) ( ( A ) > ( B ) ? ( A ) : ( B ) )

#define PL_STRINGIFY( num ) #num
#define PL_TOSTRING( A )    PL_STRINGIFY( A )

#define PL_MAGIC_TO_NUM( A, B, C, D ) ( ( ( D ) << 24 ) + ( ( C ) << 16 ) + ( ( B ) << 8 ) + ( A ) )

#define PL_BITFLAG( A, B ) A = ( 1U << B )

#ifndef offsetof
#	define PL_OFFSETOF( TYPE, MEMBER ) ( ( size_t ) &( ( ( TYPE * ) 0 )->MEMBER ) )
#else
#	define PL_OFFSETOF( TYPE, MEMBER ) offsetof( TYPE, MEMBER )
#endif

typedef enum PLVariableType {
	PL_VAR_F32,
	PL_VAR_I32,
	PL_VAR_STRING,
	PL_VAR_BOOL,// 0,1 true,false
} PLVariableType;

//////////////////////////////////////////////////////////////////

// Error return values
typedef enum PLFunctionResult {
	PL_RESULT_SUCCESS,
	PL_RESULT_FAIL,
	PL_RESULT_UNSUPPORTED,

	PL_RESULT_INVALID_PARM1,// Invalid function parameter 1
	PL_RESULT_INVALID_PARM2,// Invalid function parameter 2
	PL_RESULT_INVALID_PARM3,// Invalid function parameter 3
	PL_RESULT_INVALID_PARM4,// Invalid function parameter 4

	// FILE I/O
	PL_RESULT_FILEREAD,   // Failed to read file!
	PL_RESULT_FILEWRITE,  // Failed to write file!
	PL_RESULT_FILETYPE,   // Unexpected file type!
	PL_RESULT_FILEVERSION,// Unsupported version!
	PL_RESULT_FILESIZE,   // Invalid file size!
	PL_RESULT_FILEPATH,   // Invalid path!
	PL_RESULT_FILEERR,    // Generic Filesystem error

	// CONSOLE
	PL_RESULT_COMMAND_NAME,    // Invalid name provided for command!
	PL_RESULT_COMMAND_FUNCTION,// Invalid callback provided for command!

	// GRAPHICS
	PL_RESULT_GRAPHICSINIT,            // Graphics failed to initialise!
	PL_RESULT_INVALID_SHADER_TYPE,     /* shader type passed is not a valid type */
	PL_RESULT_UNSUPPORTED_SHADER_TYPE, /* shader type not supported by hardware */
	PL_RESULT_SHADER_COMPILE,          // Failed to compile shader!
	PL_RESULT_SHADER_LINK,             /* shader program failed link */
	PL_RESULT_DRAW_MODE,               // Invalid mesh draw mode!
	PL_RESULT_DRAW_PRIMITIVE,          // Invalid mesh primitive!

	// IMAGE
	PL_RESULT_IMAGERESOLUTION,// Invalid image resolution!
	PL_RESULT_IMAGEFORMAT,    // Unsupported image format!

	// MEMORY
	PL_RESULT_MEMORY_ALLOCATION,// Ran out of memory!
	PL_RESULT_MEMORY_UNDERFLOW,
	PL_RESULT_MEMORY_EOA,// End of array

	PL_RESULT_SYSERR,// Generic system error
} PLFunctionResult;

//////////////////////////////////////////////////////////////////

enum {
	PL_BITFLAG( PL_SUBSYSTEM_IO, 0 ),
};

#if defined( PL_INTERNAL )
#	define PL_DLL PL_EXPORT
#else
#	define PL_DLL PL_IMPORT
#endif

typedef void PLLibrary; /* handle to dll/module */

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN PLFunctionResult PlInitialize( int argc, char **argv );
PL_EXTERN PLFunctionResult PlInitializeSubSystems( unsigned int subsystems );
PL_EXTERN void PlShutdown( void );

/******************************************************************/
/* ERROR HANDLING */

void PlClearError( void );// Resets the error message to "null", so you can ensure you have the correct message from the library.

PLFunctionResult PlGetFunctionResult( void );
const char *PlGetResultString( PLFunctionResult result );
const char *PlGetError( void );// Returns the last recorded error.

void PlReportError( PLFunctionResult result, const char *function, const char *message, ... );
#	define PlReportErrorF( type, ... ) PlReportError( type, PL_FUNCTION, __VA_ARGS__ )
#	define PlReportBasicError( type )  PlReportErrorF( ( type ), PlGetResultString( ( type ) ) )

/******************************************************************/

const char *PlGenerateUniqueIdentifier( char *dest, size_t destLength );

// CL Arguments
const char *PlGetCommandLineArgumentValue( const char *arg );

/**
 * Equivalent of using 'argv' directly, but with bounds checking.
 */
const char *PlGetCommandLineArgumentValueByIndex( unsigned int index );

bool PlHasCommandLineArgument( const char *arg );

//////////////////////////////////////////////////////////////////

unsigned long pl_crc32( const void *data, size_t length, unsigned long crc );

/* standard headers */
#	include <plcore/pl_string.h>
#	include <plcore/pl_memory.h>

/**
 * Formats the current time based on the provided format string.
 *
 * This function retrieves the current time and formats it according to the
 * specified format string. It stores the formatted time in the provided buffer.
 *
 * @param format 	The format string, which follows the same formatting rules
 *               	as the standard strftime function.
 * @param buf 		The buffer where the formatted time string will be stored.
 * @param bufSize 	The size of the buffer.
 * @return 			A pointer to the formatted time string. If the time cannot be
 *         			retrieved, the function returns the string "unknown".
 */
const char *PlGetFormattedTime( const char *format, char *buf, size_t bufSize );

/**
 * Return a basic formatted time string derived from the given epoch timestamp (represented in seconds).
 * Returned string is represented as UTC. The output string will be overwritten with each function call.
 *
 * @param timestamp  64-bit unsigned integer epoch timestamp
 * @return           Pointer to char array representing formatted date and time string.
 */
const char *PlGetFormattedTimeForTimestamp( uint64_t timestamp );

time_t PlStringToTime( const char *ts );

//////////////////////////////////////////////////////////////////

PLLibrary *PlLoadLibrary( const char *path, bool appendPath );
void *PlGetLibraryProcedure( PLLibrary *library, const char *procedureName );
void PlUnloadLibrary( PLLibrary *library );

/**
 * Plugin Interface
 **/

const struct PLPluginExportTable *PlGetExportTable( void );

#endif

PL_EXTERN_C_END
