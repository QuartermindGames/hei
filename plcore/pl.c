/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "qmos/public/qm_os_memory.h"

#include <plcore/pl_filesystem.h>
#include <plcore/pl_linkedlist.h>
#include <plcore/pl_parse.h>
#include <plcore/pl_image.h>

#if defined( _WIN32 )
#	include <windows.h>
#	ifdef CreateDirectory
#		undef CreateDirectory
#	endif
#else
#	include <fcntl.h>
#endif

#if !defined( _MSC_VER )
#	include <sys/time.h>
#endif

#include <errno.h>

#include "pl_private.h"

/*	Generic functions for platform, such as	error handling.	*/

typedef struct PLSubSystem {
	unsigned int subsystem;

	PLFunctionResult ( *InitFunction )( void );
	void ( *ShutdownFunction )( void );

	bool active;
} PLSubSystem;

PLSubSystem pl_subsystems[] = {
        { PL_SUBSYSTEM_IO,
         &PlInitFileSystem,
         &PlShutdownFileSystem }
};

typedef struct PLArguments {
	const char *exe_name;
	const char *arguments[ 256 ];

	unsigned int num_arguments;
} PLArguments;

PLArguments pl_arguments;

void PlInitializeMatrixStacks_( void );

PLFunctionResult PlInitialize( int argc, char **argv ) {
	static bool is_initialized = false;
	if ( !is_initialized ) {
		PlInitConsole();
	}

	memset( &pl_arguments, 0, sizeof( PLArguments ) );
	pl_arguments.num_arguments = ( unsigned int ) argc;
	if ( pl_arguments.num_arguments > 0 ) {
		pl_arguments.exe_name = PlGetFileName( argv[ 0 ] );

		for ( unsigned int i = 0; i < pl_arguments.num_arguments; i++ ) {
			pl_arguments.arguments[ i ] = argv[ i ];
		}
	}

	PlInitPackageSubSystem();
	PlInitializeMatrixStacks_();

	is_initialized = true;

	return PL_RESULT_SUCCESS;
}

PLFunctionResult PlInitializeSubSystems( unsigned int subsystems ) {
	for ( unsigned int i = 0; i < PL_ARRAY_ELEMENTS( pl_subsystems ); i++ ) {
		if ( !pl_subsystems[ i ].active && ( subsystems & pl_subsystems[ i ].subsystem ) ) {
			if ( pl_subsystems[ i ].InitFunction ) {
				PLFunctionResult out = pl_subsystems[ i ].InitFunction();
				if ( out != PL_RESULT_SUCCESS ) {
					return out;
				}
			}

			pl_subsystems[ i ].active = true;
		}
	}

	return PL_RESULT_SUCCESS;
}

// Returns the name of the current executable.
const char *plGetExecutableName( void ) {
	return pl_arguments.exe_name;
}

bool PlHasCommandLineArgument( const char *arg ) {
	if ( pl_arguments.num_arguments < 1 ) {
		return false;
	}

	for ( unsigned int i = 0; i < pl_arguments.num_arguments; i++ ) {
		if ( pl_arguments.arguments[ i ] == NULL ) {
			continue;
		}

		if ( strcmp( pl_arguments.arguments[ i ], arg ) == 0 ) {
			return true;
		}
	}

	return false;
}

// Returns result for a single command line argument.
const char *PlGetCommandLineArgumentValue( const char *arg ) {
	if ( arg == NULL || *arg == '\0' ) {
		return NULL;
	}

	if ( pl_arguments.num_arguments < 2 ) {
		return NULL;
	}

	for ( unsigned int i = 0; i < pl_arguments.num_arguments; i++ ) {
		if ( strcmp( pl_arguments.arguments[ i ], arg ) == 0 ) {
			if ( i + 1 >= pl_arguments.num_arguments ) {
				return NULL;
			}

			return pl_arguments.arguments[ i + 1 ];
		}
	}

	return NULL;
}

const char *PlGetCommandLineArgumentValueByIndex( unsigned int index ) {
	if ( index >= pl_arguments.num_arguments ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "invalid argument index" );
		return NULL;
	}

	return pl_arguments.arguments[ index ];
}

void PlShutdown( void ) {
	for ( unsigned int i = 0; i < PL_ARRAY_ELEMENTS( pl_subsystems ); i++ ) {
		if ( !pl_subsystems[ i ].active ) {
			continue;
		}

		if ( pl_subsystems[ i ].ShutdownFunction ) {
			pl_subsystems[ i ].ShutdownFunction();
		}

		pl_subsystems[ i ].active = false;
	}

	PlShutdownConsole();
}

/*-------------------------------------------------------------------
 * UNIQUE ID GENERATION
 *-----------------------------------------------------------------*/

/**
  * Generate a simple unique identifier (!!DO NOT USE FOR ANYTHING THAT NEEDS TO BE SECURE!!)
  */
const char *PlGenerateUniqueIdentifier( char *dest, size_t destLength ) {
	// Specific char set we will set, so we can preview it after
	static char dataPool[] = {
	        'a',
	        'b',
	        'c',
	        'd',
	        'e',
	        'f',
	        'g',
	        'h',
	        'i',
	        'j',
	        'k',
	        'l',
	        'm',
	        'n',
	        'o',
	        'p',
	        'w',
	        'x',
	        'y',
	        'z',
	        'A',
	        'B',
	        'C',
	        'D',
	        'E',
	        'F',
	        'G',
	        'H',
	        'I',
	        'J',
	        'K',
	        'L',
	        'M',
	        'N',
	        'O',
	        'P',
	        'W',
	        'X',
	        'Y',
	        'Z',
	        '0',
	        '1',
	        '2',
	        '3',
	        '4',
	        '5',
	        '6',
	        '7',
	        '8',
	        '9',
	};

	for ( unsigned int i = 0; i < destLength; ++i ) {
		dest[ i ] = dataPool[ rand() % PL_ARRAY_ELEMENTS( dataPool ) ];
	}

	return dest;
}

/*-------------------------------------------------------------------
 * ERROR HANDLING
 *-----------------------------------------------------------------*/

#if defined( _WIN32 )

const char *GetLastError_strerror( uint32_t errnum ) {
	/* TODO: Make this buffer per-thread */
	static char buf[ 1024 ] = { '\0' };

	if ( !FormatMessage(
	             FORMAT_MESSAGE_FROM_SYSTEM,
	             NULL,
	             errnum,
	             MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
	             buf,
	             sizeof( buf ),
	             NULL ) ) {
		return "INTERNAL ERROR: Could not resolve error message";
	}

	/* Microsoft like to end some of their errors with newlines... */

	char *nl = strrchr( buf, '\n' );
	char *cr = strrchr( buf, '\r' );

	if ( nl != NULL && nl[ 1 ] == '\0' ) { *nl = '\0'; }
	if ( cr != NULL && cr[ 1 ] == '\0' ) { *cr = '\0'; }

	return buf;
}

#else

int GetLastError( void ) {
	return errno;
}

const char *GetLastError_strerror( int errnum ) {
	return strerror( errnum );
}

#endif

#define MAX_FUNCTION_LENGTH 64
#define MAX_ERROR_LENGTH    2048

char
        loc_error[ MAX_ERROR_LENGTH ] = { '\0' },
                                    loc_function[ MAX_FUNCTION_LENGTH ] = { '\0' };

PLFunctionResult global_result = PL_RESULT_SUCCESS;

// Returns locally generated error message.
const char *PlGetError( void ) {
	return loc_error;
}

void PlReportError( PLFunctionResult result, const char *function, const char *message, ... ) {
	va_list args;
	va_start( args, message );

	char buf[ MAX_ERROR_LENGTH ];
	vsnprintf( buf, sizeof( buf ), message, args );
	strncpy( loc_error, buf, sizeof( loc_error ) );

	va_end( args );

	snprintf( loc_function, sizeof( loc_function ), "%s", function );

	global_result = result;
}

/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC

PLFunctionResult PlGetFunctionResult( void ) {
	return global_result;
}

const char *PlGetResultString( PLFunctionResult result ) {
	switch ( result ) {
		case PL_RESULT_SUCCESS:
			return "success";
		case PL_RESULT_UNSUPPORTED:
			return "unsupported";
		case PL_RESULT_SYSERR:
			return "system error";

		case PL_RESULT_INVALID_PARM1:
			return "invalid function parameter 1";
		case PL_RESULT_INVALID_PARM2:
			return "invalid function parameter 2";
		case PL_RESULT_INVALID_PARM3:
			return "invalid function parameter 3";
		case PL_RESULT_INVALID_PARM4:
			return "invalid function parameter 4";

		// FILE I/O
		case PL_RESULT_FILEREAD:
			return "failed to read complete file!";
		case PL_RESULT_FILESIZE:
			return "failed to get valid file size!";
		case PL_RESULT_FILETYPE:
			return "invalid file type!";
		case PL_RESULT_FILEVERSION:
			return "unsupported file version!";
		case PL_RESULT_FILEPATH:
			return "invalid file path!";
		case PL_RESULT_FILEERR:
			return "filesystem error";

		// GRAPHICS
		case PL_RESULT_GRAPHICSINIT:
			return "failed to initialize graphics!";
		case PL_RESULT_INVALID_SHADER_TYPE:
			return "unsupported shader type!";
		case PL_RESULT_SHADER_COMPILE:
			return "failed to compile shader!";

		// IMAGE
		case PL_RESULT_IMAGERESOLUTION:
			return "invalid image resolution!";
		case PL_RESULT_IMAGEFORMAT:
			return "unsupported image format!";

		// MEMORY
		case PL_RESULT_MEMORY_ALLOCATION:
			return "failed to allocate memory!";
		case PL_RESULT_MEMORY_UNDERFLOW:
			return "underflow in array";

		default:
			return "an unknown error occurred";
	}
}

void PlClearError( void ) {
	loc_function[ 0 ] = loc_error[ 0 ] = '\0';
	global_result = PL_RESULT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
// Time

#if defined( _MSC_VER )

// https://stackoverflow.com/a/26085827
int gettimeofday( struct timeval *tp, struct timezone *tzp ) {
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970
	static const uint64_t EPOCH = ( ( uint64_t ) 116444736000000000ULL );

	SYSTEMTIME system_time;
	FILETIME file_time;
	uint64_t time;

	GetSystemTime( &system_time );
	SystemTimeToFileTime( &system_time, &file_time );
	time = ( ( uint64_t ) file_time.dwLowDateTime );
	time += ( ( uint64_t ) file_time.dwHighDateTime ) << 32;

	tp->tv_sec = ( long ) ( ( time - EPOCH ) / 10000000L );
	tp->tv_usec = ( long ) ( system_time.wMilliseconds * 1000 );
	return 0;
}

#endif

const char *PlGetFormattedTimeForTimestamp( uint64_t timestamp ) {
	time_t time = ( time_t ) timestamp;

	struct tm *tmPtr;
	tmPtr = gmtime( &time );

	static char timeOut[ 32 ];
	strftime( timeOut, sizeof( timeOut ), "%x %X", tmPtr );

	return timeOut;
}

const char *PlGetFormattedTime( const char *format, char *buf, size_t bufSize ) {
	struct timeval time;
	if ( gettimeofday( &time, NULL ) != 0 ) {
		return "unknown";
	}

	time_t sec = time.tv_sec;
	strftime( buf, bufSize, format, localtime( &sec ) );
	return buf;
}

/**
 * Converts the given string to time.
 * http://stackoverflow.com/questions/1765014/convert-string-from-date-into-a-time-t
 */
time_t PlStringToTime( const char *ts ) {
	char s_month[ 5 ];
	int day, year;
	sscanf( ts, "%s %d %d", s_month, &day, &year );

	static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	int month = ( int ) ( ( strstr( months, s_month ) - months ) / 3 );
	struct tm time = { 0 };
	time.tm_mon = month;
	time.tm_mday = day;
	time.tm_year = year - 1900;
	time.tm_isdst = -1;

	return mktime( &time );
}

/**********************************************************/
/** Plugin Interface **/

#include <plcore/pl_plugin_interface.h>

typedef struct PLPlugin {
	PLLibrary *libPtr;                           /* library handle */
	PLPluginInitializationFunction initFunction; /* initialization function */
	PLLinkedListNode *node;
} PLPlugin;

static PLLinkedList *plugins = NULL;
static unsigned int numPlugins = 0;

static PLImage *CreateImageOldWrapper( uint8_t *buf, unsigned int width, unsigned int height, PLColourFormat colourFormat, PLImageFormat dataFormat ) {
	return PlCreateImage( buf, width, height, 0, colourFormat, dataFormat );
}

static bool WriteImageOldWrapper( const PLImage *image, const char *path ) {
	return PlWriteImage( image, path, 100 );
}

static void RegisterPackageLoaderWrapper( const char *extension, PLPackage *( *Callback )( const char * ) ) {
	PlRegisterPackageLoader( extension, Callback, NULL );
}

static PLPluginExportTable exportTable = {
        .ReportError = PlReportError,
        .GetError = PlGetError,

        /* pl_filesystem.h */
        .LocalFileExists = PlLocalFileExists,
        .FileExists = PlFileExists,
        .LocalPathExists = PlLocalPathExists,
        .PathExists = PlPathExists,
        .ScanDirectory = PlScanDirectory,
        .CreateDirectory = PlCreateDirectory,
        .CreatePath = PlCreatePath,
        .CreateFileFromMemory = PlCreateFileFromMemory,
        .OpenLocalFile = PlOpenLocalFile,
        .OpenFile = PlOpenFile,
        .CloseFile = PlCloseFile,
        .IsEndOfFile = PlIsEndOfFile,
        .GetFilePath = PlGetFilePath,
        .GetFileData = PlGetFileData,
        .GetFileSize = PlGetFileSize,
        .GetFileOffset = PlGetFileOffset,
        .ReadFile = PlReadFile,
        .ReadInt8 = PlReadInt8,
        .ReadInt16 = PlReadInt16,
        .ReadInt32 = PlReadInt32,
        .ReadInt64 = PlReadInt64,
        .ReadString = PlReadString,
        .FileSeek = PlFileSeek,
        .RewindFile = PlRewindFile,
        .CacheFile = PlCacheFile,
        .NormalizePath = PlNormalizePath,

        .RegisterPackageLoader = RegisterPackageLoaderWrapper,
        .RegisterImageLoader = PlRegisterImageLoader,

        /* pl_image.h */
        .CreateImage = CreateImageOldWrapper,
        .CreateImage2 = PlCreateImage,
        .DestroyImage = PlDestroyImage,
        .ConvertPixelFormat = PlConvertPixelFormat,
        .InvertImageColour = PlInvertImageColour,
        .ReplaceImageColour = PlReplaceImageColour,
        .FlipImageVertical = PlFlipImageVertical,
        .GetImageSize = PlGetImageSize,
        .WriteImage = WriteImageOldWrapper,
        .ParseImage = PlParseImage,

        .CreatePackageHandle = PlCreatePackageHandle,
        .GetPackagePath = PlGetPackagePath,
        .GetPackageTableSize = PlGetPackageTableSize,
        .GetPackageTableIndex = PlGetPackageTableIndex,
        .GetPackageFileName = PlGetPackageFileName,

        .AddLogLevel = PlAddLogLevel,
        .LogMessage = PlLogMessage,
        .GetConsoleVariableValue = PlGetConsoleVariableValue,
        .GetConsoleVariableDefaultValue = PlGetConsoleVariableDefaultValue,
        .SetConsoleVariable = PlSetConsoleVariableByName,
        .RegisterConsoleVariable = PlRegisterConsoleVariable,
        .RegisterConsoleCommand = PlRegisterConsoleCommand,
        .ParseConsoleString = PlParseConsoleString,

        .IsEndOfLine = PlIsEndOfLine,
        .SkipWhitespace = PlSkipWhitespace,
        .SkipLine = PlSkipLine,
        .ParseEnclosedString = PlParseEnclosedString,
        .ParseToken = PlParseToken,
        .ParseInteger = PlParseInteger,
        .ParseFloat = PlParseFloat,

        .GenerateChecksumCRC32 = pl_crc32,

        /* pl_string.h */
        .itoa = pl_itoa,
        .strtolower = pl_strtolower,
        .strntolower = pl_strntolower,
        .strtoupper = pl_strtoupper,
        .strntoupper = pl_strntoupper,
        .strcasestr = pl_strcasestr,
        .strcasecmp = pl_strcasecmp,
        .strncasecmp = pl_strncasecmp,
        .strisalpha = pl_strisalpha,
        .strnisalpha = pl_strnisalpha,
        .strisalnum = pl_strisalnum,
        .strnisalnum = pl_strnisalnum,
        .strisdigit = pl_strisdigit,
        .strnisdigit = pl_strnisdigit,
        .vscprintf = pl_vscprintf,
        .strcnt = pl_strcnt,
        .strncnt = pl_strncnt,
        .strchunksplit = pl_strchunksplit,
        .strinsert = pl_strinsert,

        .SetupPath = PlSetupPath,
        .AppendPath = PlAppendPath,
        .PrefixPath = PlPrefixPath,
};

static void *p_malloc( size_t size, bool die )
{
	void *p = qm_os_memory_alloc( size, sizeof( char ), NULL );
	if ( p == NULL && die )
	{
		abort();
	}

	return p;
}

static void *p_calloc( size_t num, size_t size, bool die )
{
	void *p = qm_os_memory_alloc( num, size, NULL );
	if ( p == NULL && die )
	{
		abort();
	}

	return p;
}

static void *p_realloc( void *ptr, size_t size, bool die )
{
	void *p = qm_os_memory_realloc( ptr, size );
	if ( p == NULL && die )
	{
		abort();
	}

	return p;
}

const PLPluginExportTable *PlGetExportTable( void )
{
	exportTable.MAlloc  = p_malloc;
	exportTable.CAlloc  = p_calloc;
	exportTable.ReAlloc = p_realloc;
	exportTable.Free    = qm_os_memory_free;
	return &exportTable;
}
