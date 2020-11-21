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

#include <PL/platform_filesystem.h>
#include <PL/pl_graphics.h>
#include <PL/pl_llist.h>

#if defined( _WIN32 )
#	include <Windows.h>
#   ifdef CreateDirectory
#       undef CreateDirectory
#   endif
#endif

#if !defined( _MSC_VER )
#	include <sys/time.h>
#endif
#include <errno.h>

#include "platform_private.h"

/*	Generic functions for platform, such as	error handling.	*/

typedef struct PLSubSystem {
    unsigned int subsystem;

    PLresult(*InitFunction)(void);
    void(*ShutdownFunction)(void);

    bool active;
} PLSubSystem;

PLSubSystem pl_subsystems[]= {
#if defined(PL_USE_GRAPHICS)
        {
                PL_SUBSYSTEM_GRAPHICS,
                &plInitGraphics,
                &plShutdownGraphics
        },
#endif

        {
                PL_SUBSYSTEM_IO,
                &plInitFileSystem,
                &plShutdownFileSystem
        }
};

#if 0 /* incomplete interface? */
typedef struct PLArgument {
    const char *parm;

    void*(*Callback)(const char *arg);
} PLArgument;

PLArgument arguments[]={
        { "arg0" },
        { "arg1" },
};

void plParseArguments(PLArgument args[], unsigned int size) {
    for(unsigned int i = 0; i < size; i++) {
        if(args[i].Callback) {
            args[i].Callback("");
        }
    }
}
#endif

typedef struct PLArguments {
    const char *exe_name;
    const char *arguments[256];

    unsigned int num_arguments;
} PLArguments;

PLArguments pl_arguments;

PLresult plInitialize(int argc, char **argv) {
    static bool is_initialized = false;
    if(!is_initialized) {
        plInitConsole();
    }

    memset(&pl_arguments, 0, sizeof(PLArguments));
    pl_arguments.num_arguments = (unsigned int)argc;
    if(!plIsEmptyString(argv[0])) {
        pl_arguments.exe_name = plGetFileName(argv[0]);
    }

    //pl_arguments.arguments = (const char**)pl_calloc(pl_arguments.num_arguments, sizeof(char*));
    for(unsigned int i = 0; i < pl_arguments.num_arguments; i++) {
        if(plIsEmptyString(argv[i])) {
            continue;
        }

        pl_arguments.arguments[i] = argv[i];
    }

    _plInitPackageSubSystem();
    _plInitModelSubSystem();

    is_initialized = true;

    return PL_RESULT_SUCCESS;
}

PLresult plInitializeSubSystems(unsigned int subsystems) {
    for(unsigned int i = 0; i < plArrayElements(pl_subsystems); i++) {
        if(!pl_subsystems[i].active && (subsystems & pl_subsystems[i].subsystem)) {
            if(pl_subsystems[i].InitFunction) {
                PLresult out = pl_subsystems[i].InitFunction();
                if (out != PL_RESULT_SUCCESS) {
                    return out;
                }
            }

            pl_subsystems[i].active = true;
        }
    }

    return PL_RESULT_SUCCESS;
}

// Returns the name of the current executable.
const char *plGetExecutableName(void) {
    return pl_arguments.exe_name;
}

bool plHasCommandLineArgument(const char *arg) {
    if(pl_arguments.num_arguments < 1) {
        return false;
    }

    for(unsigned int i = 0; i < pl_arguments.num_arguments; i++) {
    	if ( pl_arguments.arguments[ i ] == NULL ) {
    		continue;
    	}

        if(strcmp(pl_arguments.arguments[i], arg) == 0) {
            return true;
        }
    }

    return false;
}

// Returns result for a single command line argument.
const char *plGetCommandLineArgumentValue(const char *arg) {
    if(plIsEmptyString(arg)) {
        return NULL;
    }

    if(pl_arguments.num_arguments < 2) {
        return NULL;
    }

    for(unsigned int i = 0; i < pl_arguments.num_arguments; i++) {
        if(strcmp(pl_arguments.arguments[i], arg) == 0) {
            // check the string is valid before passing it back

            if(i + 1 >= pl_arguments.num_arguments) {
                return NULL;
            }

            const char *ret = pl_arguments.arguments[i + 1];
            if(plIsEmptyString(ret)) {
                return NULL;
            }

            return ret;
        }
    }

    return NULL;
}

bool _plIsSubSystemActive(unsigned int subsystem) {
    for(unsigned int i = 0; i < plArrayElements(pl_subsystems); i++) {
        if(pl_subsystems[i].subsystem == subsystem) {
            return pl_subsystems[i].active;
        }
    }

    return false;
}

void plShutdown(void) {
    for(unsigned int i = 0; i < plArrayElements(pl_subsystems); i++) {
        if(!pl_subsystems[i].active) {
            continue;
        }

        if(pl_subsystems[i].ShutdownFunction) {
            pl_subsystems[i].ShutdownFunction();
        }

        pl_subsystems[i].active = false;
    }

    plShutdownConsole();
}

/*-------------------------------------------------------------------
 * UNIQUE ID GENERATION
 *-----------------------------------------------------------------*/

 /**
  * Generate a simple unique identifier (!!DO NOT USE FOR ANYTHING THAT NEEDS TO BE SECURE!!)
  */
const char *plGenerateUniqueIdentifier( char *dest, size_t destLength ) {
	// Specific char set we will set, so we can preview it after
	static char dataPool[] = {
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
		'k', 'l', 'm', 'n', 'o', 'p', 'w', 'x', 'y', 'z',
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
		'K', 'L', 'M', 'N', 'O', 'P', 'W', 'X', 'Y', 'Z',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	};

	for( unsigned int i = 0; i < destLength; ++i ) {
		dest[ i ] = dataPool[ rand() % plArrayElements( dataPool ) ];
	}

	return dest;
}

/*-------------------------------------------------------------------
 * ERROR HANDLING
 *-----------------------------------------------------------------*/

#if defined( _WIN32 )

const char *GetLastError_strerror(uint32_t errnum) {
    /* TODO: Make this buffer per-thread */
    static char buf[1024] = {'\0'};

    if(!FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errnum,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf,
        sizeof(buf),
        NULL))
    {
        return "INTERNAL ERROR: Could not resolve error message";
    }

    /* Microsoft like to end some of their errors with newlines... */

    char *nl = strrchr(buf, '\n');
    char *cr = strrchr(buf, '\r');

    if(nl != NULL && nl[1] == '\0') { *nl = '\0'; }
    if(cr != NULL && cr[1] == '\0') { *cr = '\0'; }

    return buf;
}

#else

int GetLastError(void) {
    return errno;
}

const char *GetLastError_strerror(int errnum) {
    return strerror(errnum);
}

#endif

#define    MAX_FUNCTION_LENGTH  64
#define    MAX_ERROR_LENGTH     2048

char
        loc_error[MAX_ERROR_LENGTH]         = { '\0' },
        loc_function[MAX_FUNCTION_LENGTH]   = { '\0' };

PLresult global_result = PL_RESULT_SUCCESS;

// Sets the name of the current function.
void plSetCurrentFunction(const char *function, ...) {
#ifdef _DEBUG
    char out[2048]; // todo, shitty work around because linux crap
    va_list args;

    va_start(args, function);
    vsnprintf(out, sizeof(out), function, args);
    va_end(args);

    strncpy(loc_function, out, sizeof(function));
#endif
}

void plSetFunctionResult(PLresult result) {
    global_result = result;
}

// Sets the local error message.
void SetErrorMessage(const char *msg, ...) {
#ifdef _DEBUG
    char out[MAX_ERROR_LENGTH];
    va_list args;

    va_start(args, msg);
    vsnprintf(out, sizeof(out), msg, args);
    va_end(args);

    strncpy(loc_error, out, sizeof(loc_error));
#endif
}

// Returns locally generated error message.
const char *plGetError(void) {
    return loc_error;
}

/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC

PLresult plGetFunctionResult(void) {
    return global_result;
}

const char *plGetResultString(PLresult result) {
    switch (result) {
        case PL_RESULT_SUCCESS:     return "success";
        case PL_RESULT_UNSUPPORTED: return "unsupported";
        case PL_RESULT_SYSERR:      return "system error";

        case PL_RESULT_INVALID_PARM1:   return "invalid function parameter 1";
        case PL_RESULT_INVALID_PARM2:   return "invalid function parameter 2";
        case PL_RESULT_INVALID_PARM3:   return "invalid function parameter 3";
        case PL_RESULT_INVALID_PARM4:   return "invalid function parameter 4";

        // FILE I/O
        case PL_RESULT_FILEREAD:    return "failed to read complete file!";
        case PL_RESULT_FILESIZE:    return "failed to get valid file size!";
        case PL_RESULT_FILETYPE:    return "invalid file type!";
        case PL_RESULT_FILEVERSION: return "unsupported file version!";
        case PL_RESULT_FILEPATH:    return "invalid file path!";
        case PL_RESULT_FILEERR:     return "filesystem error";

        // GRAPHICS
        case PL_RESULT_GRAPHICSINIT:    return "failed to initialize graphics!";
        case PL_RESULT_INVALID_SHADER_TYPE:      return "unsupported shader type!";
        case PL_RESULT_SHADER_COMPILE:   return "failed to compile shader!";

        // IMAGE
        case PL_RESULT_IMAGERESOLUTION: return "invalid image resolution!";
        case PL_RESULT_IMAGEFORMAT:     return "unsupported image format!";

        // MEMORY
        case PL_RESULT_MEMORY_ALLOCATION: 	return "failed to allocate memory!";
        case PL_RESULT_MEMORY_UNDERFLOW: 	return "underflow in array";

        default:    return "an unknown error occurred";
    }
}

void _plResetError(void) {
    loc_function[0] = loc_error[0] = '\0';
    global_result = PL_RESULT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
// Time

#if defined( _MSC_VER )

// https://stackoverflow.com/a/26085827
int gettimeofday( struct timeval* tp, struct timezone* tzp ) {
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970
	static const uint64_t EPOCH = ( (uint64_t)116444736000000000ULL );

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime( &system_time );
	SystemTimeToFileTime( &system_time, &file_time );
	time = ( (uint64_t)file_time.dwLowDateTime );
	time += ( (uint64_t)file_time.dwHighDateTime ) << 32;

	tp->tv_sec = (long)( ( time - EPOCH ) / 10000000L );
	tp->tv_usec = (long)( system_time.wMilliseconds * 1000 );
	return 0;
}

#endif

const char *plGetFormattedTime(void) {
    struct timeval time;
    if(gettimeofday(&time, NULL) != 0) {
        return "unknown";
    }

    static char time_out[32] = { '\0' };
    time_t sec = time.tv_sec;
    strftime(time_out, sizeof(time_out), "%x %X", localtime(&sec));
    return time_out;
}

/**
 * Converts the given string to time.
 * http://stackoverflow.com/questions/1765014/convert-string-from-date-into-a-time-t
 */
time_t plStringToTime( const char *ts ) {
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

/////////////////////////////////////////////////////////////////////////////////////
// Loop

bool plIsRunning(void) {
    return true;
}

double plGetDeltaTime(void) {
    static struct timeval last, now = { 0 };
    last = now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - last.tv_sec) * 1000;
}

double accumulator = 0;

void plProcess(double delta) {
    // todo, game logic @ locked 60fps (rendering is UNLOCKED?)

    while(accumulator >= delta) {
        //plProcessActors();
        //plProcessPhysics();
    }

#if defined(PL_USE_GRAPHICS)
    plProcessGraphics();
#endif
}

/**********************************************************/
/** Plugin Interface **/

#include <PL/pl_plugin_interface.h>

typedef struct PLPlugin {
	char                            pluginPath[ PL_SYSTEM_MAX_PATH ];   /* full path to the plugin */
	PLLibrary                       *libPtr;                            /* library handle */
	PLPluginInitializationFunction  initFunction;                       /* initialization function */
	PLLinkedListNode                *node;
} PLPlugin;

static PLLinkedList *plugins;
static unsigned int numPlugins;

static PLPluginExportTable exportTable = {
        .LocalFileExists = plLocalFileExists,
        .FileExists = plFileExists,
        .LocalPathExists = plLocalPathExists,
        .PathExists = plPathExists,
        .ScanDirectory = plScanDirectory,
        .CreateDirectory = plCreateDirectory,
        .CreatePath = plCreatePath,
        .OpenLocalFile = plOpenLocalFile,
        .OpenFile = plOpenFile,
        .CloseFile = plCloseFile,
        .IsEndOfFile = plIsEndOfFile,
        .GetFilePath = plGetFilePath,
        .GetFileData = plGetFileData,
        .GetFileSize = plGetFileSize,
        .GetFileOffset = plGetFileOffset,
        .ReadFile = plReadFile,
        .ReadInt8 = plReadInt8,
        .ReadInt16 = plReadInt16,
        .ReadInt32 = plReadInt32,
        .ReadInt64 = plReadInt64,
        .ReadString = plReadString,
        .FileSeek = plFileSeek,
        .RewindFile = plRewindFile,

        .RegisterPackageLoader = plRegisterPackageLoader,
        .RegisterModelLoader = plRegisterModelLoader,
        .RegisterImageLoader = plRegisterImageLoader,

        .CreateMesh = plCreateMesh,
        .DestroyMesh = plDestroyMesh,
        .ClearMesh = plClearMesh,
        .ClearMeshVertices = plClearMeshVertices,
        .ClearMeshTriangles = plClearMeshTriangles,
        .ScaleMesh = plScaleMesh,
        .AddMeshVertex = plAddMeshVertex,
        .AddMeshTriangle = plAddMeshTriangle,
        .GenerateMeshNormals = plGenerateMeshNormals,
        .GenerateTextureCoordinates = plGenerateTextureCoordinates,
        .GenerateVertexNormals = plGenerateVertexNormals,

        .CreateStaticModel = plCreateStaticModel,
        .CreateSkeletalModel = plCreateSkeletalModel,
        .DestroyModel = plDestroyModel,
        .GenerateModelNormals = plGenerateModelNormals,
        .GenerateModelBounds = plGenerateModelBounds,

        .CreateImage = plCreateImage,
        .DestroyImage = plDestroyImage,
        .ConvertPixelFormat = plConvertPixelFormat,
        .InvertImageColour = plInvertImageColour,
        .ReplaceImageColour = plReplaceImageColour,
        .FlipImageVertical = plFlipImageVertical,
        .GetNumberOfColourChannels = plGetNumberOfColourChannels,
        .GetImageSize = plGetImageSize,
};

/**
 * Attempts to load the given plugin and validate it.
 */
bool plRegisterPlugin( const char *path ) {
	_plResetError();

	DebugPrint( "Registering plugin: \"%s\"\n", path );

	PLLibrary *library = plLoadLibrary( path, false );
	if ( library == NULL ) {
		return false;
	}

	PLPluginQueryFunction RegisterPlugin;
	PLPluginInitializationFunction InitializePlugin;
	const PLPluginDescription *description;

	RegisterPlugin = ( PLPluginQueryFunction ) plGetLibraryProcedure( library, PL_PLUGIN_QUERY_FUNCTION );
	if ( RegisterPlugin != NULL ) {
		InitializePlugin = ( PLPluginInitializationFunction ) plGetLibraryProcedure( library, PL_PLUGIN_INIT_FUNCTION );
		if ( InitializePlugin != NULL ) {
			/* now fetch the plugin description */
			description = RegisterPlugin( PL_PLUGIN_INTERFACE_VERSION );
			if ( description != NULL ) {
				if ( description->interfaceVersion != PL_PLUGIN_INTERFACE_VERSION ) {
					ReportError( PL_RESULT_UNSUPPORTED, "Unsupported interface version!\n" );
				} else {
					DebugPrint( "Found plugin (%s)\n", description->description );
				}
			}
		}
	}

	if ( plGetFunctionResult() != PL_RESULT_SUCCESS ) {
		plUnloadLibrary( library );
		return false;
	}

	DebugPrint( "Success, adding \"%s\" to plugins list\n", path );

	if ( plugins == NULL ) {
		plugins = plCreateLinkedList();
	}

	PLPlugin *plugin = pl_malloc( sizeof( PLPlugin ) );
	snprintf( plugin->pluginPath, sizeof( plugin->pluginPath ), path );
	plugin->initFunction = InitializePlugin;
	plugin->libPtr = library;
	plugin->node = plInsertLinkedListNode( plugins, plugin );

	numPlugins++;

	return true;
}

/**
 * Private callback when scanning for plugins.
 */
static void _plRegisterScannedPlugin( const char *path, void *unused ) {
	plUnused( unused );

	/* validate it's actually a plugin */
	size_t length = strlen( path );
	const char *c = strrchr( path, '_' );
	if ( c == NULL ) {
		return;
	}

	/* remaining length */
	length -= c - path;
	if ( pl_strncasecmp( c, "_plugin" PL_SYSTEM_LIBRARY_EXTENSION, length ) != 0 ) {
		return;
	}

	plRegisterPlugin( path );
}

/**
 * Scans for libraries in the given directory for supporting other model formats.
 */
void plRegisterPlugins( const char *pluginDir ) {
	_plResetError();

	if ( !plPathExists( pluginDir ) ) {
		ReportBasicError( PL_RESULT_INVALID_PARM1 );
		return;
	}

	Print( "Scanning for plugins in \"%s\"\n", pluginDir );

	plScanDirectory( pluginDir, NULL, _plRegisterScannedPlugin, false, NULL );

	Print( "Done, %d plugins loaded.\n", numPlugins );
}

/**
 * Iterate over all the registered plugins and initialize each.
 */
void plInitializePlugins( void ) {
	PLLinkedListNode *node = plGetRootNode( plugins );
	while ( node != NULL ) {
		PLPlugin *plugin = ( PLPlugin * ) plGetLinkedListNodeUserData( node );

		exportTable.MAlloc = pl_malloc;
		exportTable.CAlloc = pl_calloc;
		exportTable.ReAlloc = pl_realloc;
		exportTable.Free = pl_free;

		plugin->initFunction( &exportTable );

		node = plGetNextLinkedListNode( node );
	}
}
