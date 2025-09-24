/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl_console.h>
#include <plcore/pl_filesystem.h>
#include <plcore/pl_linkedlist.h>
#include <plcore/pl_parse.h>
#include <plcore/pl_hashtable.h>

#include "pl_private.h"
#include "qmos/public/qm_os_memory.h"

#include <errno.h>
#if defined( _WIN32 )
#	include <windows.h>
#	include <io.h>
#endif

#define CONSOLE_MAX_ARGUMENTS 8

/* Multi Console Manager */

static PLConsoleCommand **_pl_commands = NULL;
static size_t _pl_num_commands = 0;
static size_t _pl_commands_size = 512;
static PLHashTable *commandHashes = NULL;

static PLConsoleCommand *PlGetConsoleCommand( const char *name ) {
	if ( commandHashes == NULL ) {
		PlReportErrorF( PL_RESULT_MEMORY_EOA, "no console commands registered" );
		return NULL;
	}

	// convert it so it's case insensitive here...
	size_t s = strlen( name ) + 1;
	char *tmp = QM_OS_MEMORY_NEW_( char, s );
	for ( unsigned int i = 0; i < s; ++i ) {
		tmp[ i ] = ( char ) tolower( name[ i ] );
	}

	PLConsoleCommand *command = PlLookupHashTableUserData( commandHashes, tmp, s );

	qm_os_memory_free( tmp );

	return command;
}

void PlRegisterConsoleCommand( const char *name, const char *description, int args, void ( *CallbackFunction )( unsigned int argc, char *argv[] ) ) {
	FunctionStart();

	if ( PlGetConsoleCommand( name ) != NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "command with name has already been registered" );
		return;
	}

	if ( CallbackFunction == NULL ) {
		PlReportErrorF( PL_RESULT_COMMAND_FUNCTION, PlGetResultString( PL_RESULT_COMMAND_FUNCTION ) );
		return;
	}

	// Deal with resizing the array dynamically...
	if ( ( 1 + _pl_num_commands ) > _pl_commands_size ) {
		_pl_commands = ( PLConsoleCommand ** ) qm_os_memory_realloc( _pl_commands, ( _pl_commands_size += 128 ) * sizeof( PLConsoleCommand ) );
	}

	if ( commandHashes == NULL ) {
		commandHashes = PlCreateHashTable();
	}

	if ( _pl_num_commands < _pl_commands_size ) {
		_pl_commands[ _pl_num_commands ] = QM_OS_MEMORY_NEW( PLConsoleCommand );
		if ( !_pl_commands[ _pl_num_commands ] ) {
			return;
		}

		PLConsoleCommand *cmd = _pl_commands[ _pl_num_commands ];
		PL_ZERO( cmd, sizeof( PLConsoleCommand ) );
		cmd->Callback = CallbackFunction;

		size_t s = strlen( name ) + 1;
		cmd->name = QM_OS_MEMORY_NEW_( char, s );
		strncpy( cmd->name, name, s );
		pl_strtolower( cmd->name );

		PlInsertHashTableNode( commandHashes, cmd->name, s, cmd );

		// restore name back to case variant
		strncpy( cmd->name, name, s );

		if ( description != NULL ) {
			s = strlen( description ) + 1;
			cmd->description = QM_OS_MEMORY_NEW_( char, s );
			strncpy( cmd->description, description, s );
		}

		cmd->args = args;

		_pl_num_commands++;
	}
}

void PlGetConsoleCommands( PLConsoleCommand ***cmds, size_t *num_cmds ) {
	*cmds = _pl_commands;
	*num_cmds = _pl_num_commands;
}

/////////////////////////////////////////////////////////////////////////////////////

static PLConsoleVariable **_pl_variables = NULL;
static size_t _pl_num_variables = 0;
static size_t _pl_variables_size = 512;
static PLHashTable *variableHashes = NULL;

PLConsoleVariable *PlRegisterConsoleVariable( const char *name, const char *description, const char *defaultValue, PLVariableType type, void *ptrValue, void ( *CallbackFunction )( PLConsoleVariable * ), bool archive ) {
	FunctionStart();

	assert( _pl_variables );

	if ( PlGetConsoleVariable( name ) != NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "variable with name has already been registered" );
		return NULL;
	}

	// Deal with resizing the array dynamically...
	if ( ( 1 + _pl_num_variables ) > _pl_variables_size ) {
		_pl_variables = ( PLConsoleVariable ** ) qm_os_memory_realloc( _pl_variables, ( _pl_variables_size += 128 ) * sizeof( PLConsoleVariable ) );
	}

	if ( variableHashes == NULL ) {
		variableHashes = PlCreateHashTable();
	}

	PLConsoleVariable *out = NULL;
	if ( _pl_num_variables < _pl_variables_size ) {
		_pl_variables[ _pl_num_variables ] = ( PLConsoleVariable * ) QM_OS_MEMORY_MALLOC_( sizeof( PLConsoleVariable ) );
		out = _pl_variables[ _pl_num_variables ];

		out->type = type;
		if ( ptrValue != NULL ) {
			out->ptrValue = ptrValue;
		}

		out->archive = archive;

		size_t s = strlen( name ) + 1;
		out->name = QM_OS_MEMORY_NEW_( char, s );
		strncpy( out->name, name, s );
		pl_strtolower( out->name );

		PlInsertHashTableNode( variableHashes, out->name, s, out );

		// restore name back to case variant
		strncpy( out->name, name, s );

		if ( description != NULL ) {
			s = strlen( description ) + 1;
			out->description = QM_OS_MEMORY_NEW_( char, s );
			strncpy( out->description, description, s );
		}

		snprintf( out->default_value, sizeof( out->default_value ), "%s", defaultValue );

		PlSetConsoleVariable( out, out->default_value );

		// Ensure the callback is only called afterwards
		if ( CallbackFunction != NULL ) {
			out->CallbackFunction = CallbackFunction;
		}

		_pl_num_variables++;
	}

	return out;
}

void PlGetConsoleVariables( PLConsoleVariable ***vars, size_t *num_vars ) {
	*vars = _pl_variables;
	*num_vars = _pl_num_variables;
}

PLConsoleVariable *PlGetConsoleVariable( const char *name ) {
	if ( variableHashes == NULL ) {
		PlReportErrorF( PL_RESULT_MEMORY_EOA, "no console variables registered" );
		return NULL;
	}

	// convert it so it's case insensitive here...
	size_t s = strlen( name ) + 1;
	char *tmp = QM_OS_MEMORY_NEW_( char, s );
	for ( unsigned int i = 0; i < s; ++i ) {
		tmp[ i ] = ( char ) tolower( name[ i ] );
	}

	PLConsoleVariable *var = PlLookupHashTableUserData( variableHashes, tmp, s );

	qm_os_memory_free( tmp );

	return var;
}

const char *PlGetConsoleVariableValue( const char *name ) {
	PLConsoleVariable *var = PlGetConsoleVariable( name );
	if ( var == NULL ) {
		return NULL;
	}

	return var->value;
}

const char *PlGetConsoleVariableDefaultValue( const char *name ) {
	PLConsoleVariable *var = PlGetConsoleVariable( name );
	if ( var == NULL ) {
		return NULL;
	}

	return var->default_value;
}

// Set console variable, with sanity checks...
void PlSetConsoleVariable( PLConsoleVariable *var, const char *value ) {
	assert( var );
	switch ( var->type ) {
		default:
			PrintWarning( "Unknown variable type %d, failed to set!\n", var->type );
			return;

		case PL_VAR_I32:
			if ( pl_strisdigit( value ) != -1 ) {
				PrintWarning( "Unknown argument type %s, failed to set!\n", value );
				return;
			}

			var->i_value = ( int ) strtol( value, NULL, 10 );
			if ( var->ptrValue != NULL ) {
				*( int * ) var->ptrValue = var->i_value;
			}
			break;

		case PL_VAR_STRING:
			var->s_value = &var->value[ 0 ];
			if ( var->ptrValue != NULL ) {
				snprintf( var->ptrValue, PL_VAR_VALUE_LENGTH, "%s", value );
			}
			break;

		case PL_VAR_F32:
			var->f_value = strtof( value, NULL );
			if ( var->ptrValue != NULL ) {
				*( float * ) var->ptrValue = var->f_value;
			}
			break;

		case PL_VAR_BOOL:
			if ( pl_strisalnum( value ) == -1 ) {
				PrintWarning( "Unknown argument type %s, failed to set!\n", value );
				return;
			}

			if ( strcmp( value, "true" ) == 0 || strcmp( value, "1" ) == 0 ) {
				var->b_value = true;
			} else {
				var->b_value = false;
			}

			if ( var->ptrValue != NULL ) {
				*( bool * ) var->ptrValue = var->b_value;
			}
			break;
	}

	snprintf( var->value, sizeof( var->value ), "%s", value );

	if ( var->CallbackFunction != NULL ) {
		var->CallbackFunction( var );
	}
}

void PlSetConsoleVariableByName( const char *name, const char *value ) {
	PLConsoleVariable *var = PlGetConsoleVariable( name );
	if ( var == NULL ) {
		Print( "Failed to find console variable \"%s\"!\n", name );
		return;
	}

	PlSetConsoleVariable( var, value );
}

/////////////////////////////////////////////////////////////////////////////////////
// PRIVATE

static void PrintVarDetails( const PLConsoleVariable *var ) {
	Print( " %-25s : %-5s / %-15s : %-20s\n",
	       var->name,
	       var->value,
	       var->default_value,
	       var->description != NULL ? var->description : "None" );
}

static void PrintCmdDetails( const PLConsoleCommand *cmd ) {
	Print( " %-25s : %-20s\n",
	       cmd->name,
	       cmd->description != NULL ? cmd->description : "None" );
}

IMPLEMENT_COMMAND( pwd ) {
	PL_UNUSEDVAR( argv );
	PL_UNUSEDVAR( argc );
	Print( "%s\n", PlGetWorkingDirectory() );
}

IMPLEMENT_COMMAND( echo ) {
	for ( unsigned int i = 0; i < ( argc - 1 ); ++i ) {
		Print( "%s ", argv[ i ] );
	}
	Print( "\n" );
}

IMPLEMENT_COMMAND( time ) {
	PL_UNUSEDVAR( argv );
	PL_UNUSEDVAR( argc );

	char buf[ 64 ];
	Print( "%s\n", PlGetFormattedTime( "%x %X", buf, sizeof( buf ) ) );
}

IMPLEMENT_COMMAND( mem ) {
	PL_UNUSEDVAR( argv );
	PL_UNUSEDVAR( argc );
	/* this intentionally sucks for now... */
	Print( "System usage:        %llu\n"
	       "Local/tracked usage: %llu\n",
	       qm_os_memory_get_usage(),
	       qm_os_memory_get_total() );
}

IMPLEMENT_COMMAND( cmds ) {
	PL_UNUSEDVAR( argv );
	PL_UNUSEDVAR( argc );
	for ( PLConsoleCommand **cmd = _pl_commands; cmd < _pl_commands + _pl_num_commands; ++cmd ) {
		PrintCmdDetails( ( *cmd ) );
	}
	Print( "%zu commands in total\n", _pl_num_commands );
}

IMPLEMENT_COMMAND( vars ) {
	PL_UNUSEDVAR( argv );
	PL_UNUSEDVAR( argc );
	for ( PLConsoleVariable **var = _pl_variables; var < _pl_variables + _pl_num_variables; ++var ) {
		PrintVarDetails( ( *var ) );
	}
	Print( "%zu variables in total\n", _pl_num_variables );
}

IMPLEMENT_COMMAND( help ) {
	PLConsoleVariable *var = PlGetConsoleVariable( argv[ 1 ] );
	if ( var != NULL ) {
		PrintVarDetails( var );
		return;
	}

	PLConsoleCommand *cmd = PlGetConsoleCommand( argv[ 1 ] );
	if ( cmd != NULL ) {
		PrintCmdDetails( cmd );
		return;
	}

	Print( "Unknown variable/command, %s!\n", argv[ 1 ] );
}

/**
 * Find the specific var/cmd, by name or description.
 */
static void find_cmd( PL_UNUSED unsigned int argc, char **argv ) {
	if ( argc <= 1 ) {
		Print( "No arguments provided!\n" );
		return;
	}

	const char *term = argv[ 1 ];

	bool findVars = true;
	bool findCommands = true;
	if ( argc > 2 ) {
		if ( strcmp( term, "var" ) == 0 ) {
			findVars = true;
			findCommands = false;
			term = argv[ 2 ];
		} else if ( strcmp( term, "cmd" ) == 0 ) {
			findVars = false;
			findCommands = true;
			term = argv[ 2 ];
		}
	} else {
		findVars = findCommands = true;
	}

	if ( term == NULL ) {
		Print( "Invalid arguments provided!\n" );
		return;
	}

	if ( findVars ) {
		Print( "Variables that match the term \"%s\"\n", term );
		for ( PLConsoleVariable **var = _pl_variables; var < _pl_variables + _pl_num_variables; ++var ) {
			if ( ( pl_strcasestr( ( *var )->name, term ) == NULL ) && ( ( *var )->description != NULL && pl_strcasestr( ( *var )->description, term ) == NULL ) ) {
				continue;
			}

			PrintVarDetails( ( *var ) );
		}
	}

	if ( findCommands ) {
		Print( "Commands that match the term \"%s\"\n", term );
		for ( PLConsoleCommand **cmd = _pl_commands; cmd < _pl_commands + _pl_num_commands; ++cmd ) {
			if ( ( pl_strcasestr( ( *cmd )->name, term ) == NULL ) && ( ( *cmd )->description != NULL && pl_strcasestr( ( *cmd )->description, term ) == NULL ) ) {
				continue;
			}

			PrintCmdDetails( ( *cmd ) );
		}
	}
}

//////////////////////////////////////////////

void ( *ConsoleOutputCallback )( int level, const char *msg, QmMathColour4ub colour );

static void InitializeDefaultLogLevels( void );
PLFunctionResult PlInitConsole( void ) {
	ConsoleOutputCallback = NULL;

	if ( ( _pl_commands = ( PLConsoleCommand ** ) QM_OS_MEMORY_MALLOC_( sizeof( PLConsoleCommand * ) * _pl_commands_size ) ) == NULL ) {
		return PL_RESULT_MEMORY_ALLOCATION;
	}

	if ( ( _pl_variables = ( PLConsoleVariable ** ) QM_OS_MEMORY_MALLOC_( sizeof( PLConsoleVariable * ) * _pl_variables_size ) ) == NULL ) {
		return PL_RESULT_MEMORY_ALLOCATION;
	}

	PlRegisterConsoleCommand( "help", "Returns information regarding specified command or variable.", 1, help_cmd );
	PlRegisterConsoleCommand( "time", "Returns the current time.", 0, time_cmd );
	PlRegisterConsoleCommand( "mem", "Returns the current memory usage stats.", 0, mem_cmd );
	PlRegisterConsoleCommand( "cmds", "Prints a list of available commands.", 0, cmds_cmd );
	PlRegisterConsoleCommand( "vars", "Prints a list of available variables.", 0, vars_cmd );
	PlRegisterConsoleCommand( "pwd", "Prints out the current working directory.", 0, pwd_cmd );
	PlRegisterConsoleCommand( "echo", "Echos the given input to the console output.", 1, echo_cmd );
	PlRegisterConsoleCommand( "find", "Find the specific var/cmd, by name or description."
	                                  "You can specify 'cmd' or 'vars' to filter.",
	                          -1, find_cmd );

	/* initialize our internal log levels here
	 * as we depend on the console variables being setup first */
	InitializeDefaultLogLevels();

	return PL_RESULT_SUCCESS;
}

void PlShutdownConsole( void ) {
	PlDestroyHashTable( commandHashes );
	commandHashes = NULL;
	if ( _pl_commands ) {
		for ( PLConsoleCommand **cmd = _pl_commands; cmd < _pl_commands + _pl_num_commands; ++cmd ) {
			// todo, should we return here; assume it's the end?
			if ( ( *cmd ) == NULL ) {
				continue;
			}

			qm_os_memory_free( ( *cmd )->name );
			qm_os_memory_free( ( *cmd )->description );

			qm_os_memory_free( ( *cmd ) );
		}
		qm_os_memory_free( _pl_commands );
		_pl_commands = NULL;
	}

	PlDestroyHashTable( variableHashes );
	variableHashes = NULL;
	if ( _pl_variables ) {
		for ( PLConsoleVariable **var = _pl_variables; var < _pl_variables + _pl_num_variables; ++var ) {
			// todo, should we return here; assume it's the end?
			if ( ( *var ) == NULL ) {
				continue;
			}

			qm_os_memory_free( ( *var )->name );
			qm_os_memory_free( ( *var )->description );

			qm_os_memory_free( ( *var ) );
		}
		qm_os_memory_free( _pl_variables );
		_pl_variables = NULL;
	}

	ConsoleOutputCallback = NULL;
}

void PlSetConsoleOutputCallback( void ( *Callback )( int level, const char *msg, QmMathColour4ub colour ) ) {
	ConsoleOutputCallback = Callback;
}

/////////////////////////////////////////////////////

/**
 * Takes a string and returns a list of possible options.
 */
const char **PlAutocompleteConsoleString( const char *string, unsigned int *numElements ) {
#define MAX_AUTO_OPTIONS 16
	static const char *options[ MAX_AUTO_OPTIONS ];
	unsigned int c = 0;
	size_t sl = strlen( string );
	/* gather all the console variables */
	for ( unsigned int i = 0; i < _pl_num_variables; ++i ) {
		if ( c >= MAX_AUTO_OPTIONS ) {
			break;
		} else if ( pl_strncasecmp( string, _pl_variables[ i ]->name, sl ) != 0 ) {
			continue;
		}
		options[ c++ ] = _pl_variables[ i ]->name;
	}
	/* gather up all the console commands */
	for ( unsigned int i = 0; i < _pl_num_commands; ++i ) {
		if ( c >= MAX_AUTO_OPTIONS ) {
			break;
		} else if ( pl_strncasecmp( string, _pl_commands[ i ]->name, sl ) != 0 ) {
			continue;
		}
		options[ c++ ] = _pl_commands[ i ]->name;
	}

	*numElements = c;
	return options;
}

void PlParseConsoleString( const char *string ) {
	if ( string == NULL || *string == '\0' ) {
		DebugPrint( "Invalid string passed to ParseConsoleString!\n" );
		return;
	}

	static char **argv = NULL;
	if ( argv == NULL ) {
		if ( ( argv = ( char ** ) QM_OS_MEMORY_MALLOC_( sizeof( char * ) * CONSOLE_MAX_ARGUMENTS ) ) == NULL ) {
			return;
		}
		for ( char **arg = argv; arg < argv + CONSOLE_MAX_ARGUMENTS; ++arg ) {
			( *arg ) = ( char * ) QM_OS_MEMORY_MALLOC_( sizeof( char ) * 1024 );
			if ( ( *arg ) == NULL ) {
				break;// continue to our doom... ?
			}
		}
	}

	unsigned int argc = 0;
	for ( const char *pos = string; *pos; ) {
		size_t arglen = strcspn( pos, " " );
		if ( arglen > 0 ) {
			strncpy( argv[ argc ], pos, arglen );
			argv[ argc ][ arglen ] = '\0';
			++argc;
		}
		pos += arglen;
		pos += strspn( pos, " " );
	}

	PLConsoleVariable *var;
	PLConsoleCommand *cmd;

	if ( ( var = PlGetConsoleVariable( argv[ 0 ] ) ) != NULL ) {
		// todo, should the var not be set by defacto here?

		if ( argc > 1 ) {
			PlSetConsoleVariable( var, argv[ 1 ] );
		} else {
			Print( "    %s\n", var->name );
			Print( "    %s\n", var->description != NULL ? var->description : "None" );
			Print( "    %-10s : %s\n", var->value, var->default_value );
		}
	} else if ( ( cmd = PlGetConsoleCommand( argv[ 0 ] ) ) != NULL ) {
		if ( cmd->Callback == NULL ) {
			Print( "    Invalid command, no callback provided!\n" );
			Print( "    %s\n", cmd->name );
			Print( "    %s\n", cmd->description != NULL ? cmd->description : "None" );
			return;
		}

		if ( cmd->args >= 0 && ( argc - 1 ) != cmd->args ) {
			Print( "    Invalid number of arguments!\n" );
			Print( "    %s\n", cmd->description != NULL ? cmd->description : "None" );
			return;
		}

		cmd->Callback( argc, argv );
	} else {
		Print( "Unknown variable/command, %s!\n", argv[ 0 ] );
	}
}

/*	Log System	*/

#define MAX_LOG_LEVELS 512

typedef struct LogLevel {
	bool isReserved;
	char prefix[ 64 ];// e.g. 'warning, 'error'
	QmMathColour4ub colour;
	PLConsoleVariable *var;
} LogLevel;
static LogLevel levels[ MAX_LOG_LEVELS ];
unsigned int numLogLevels = 0;

int LOG_LEVEL_LOW = 0;
int LOG_LEVEL_MEDIUM = 0;
int LOG_LEVEL_HIGH = 0;
int LOG_LEVEL_DEBUG = 0;
int LOG_LEVEL_FILESYSTEM = 0;

static void InitializeDefaultLogLevels( void )
{
	memset( levels, 0, sizeof( LogLevel ) * MAX_LOG_LEVELS );

	LOG_LEVEL_LOW    = PlAddLogLevel( "plcore", QM_MATH_COLOUR4UB( 255, 255, 255, 255 ), true );
	LOG_LEVEL_MEDIUM = PlAddLogLevel( "plcore/warning", QM_MATH_COLOUR4UB( 255, 255, 0, 255 ), true );
	LOG_LEVEL_HIGH   = PlAddLogLevel( "plcore/error", QM_MATH_COLOUR4UB( 255, 0, 0, 255 ), true );
	LOG_LEVEL_DEBUG  = PlAddLogLevel( "plcore/debug", QM_MATH_COLOUR4UB( 255, 255, 255, 255 ),
#if !defined( NDEBUG )
	                                 true
#else
	                                 false
#endif
	);
	LOG_LEVEL_FILESYSTEM = PlAddLogLevel( "plcore/filesystem", QM_MATH_COLOUR4UB( 0, 255, 255, 255 ), true );
}

/**
 * Converts an external log level id into an
 * internal one.
 */
static LogLevel *GetLogLevelForId( int id ) {
	if ( id < 0 || id >= MAX_LOG_LEVELS ) {
		PlReportErrorF( PL_RESULT_MEMORY_EOA, "failed to find slot for log level %d", id );
		return NULL;
	}

	return &levels[ id ];
}

/**
 * Fetches the next unreserved slot.
 */
static int GetNextFreeLogLevel( void ) {
	for ( int i = 0; i < MAX_LOG_LEVELS; ++i ) {
		if ( !levels[ i ].isReserved ) {
			return i;
		}
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////
// public

static char logOutputPath[ PL_SYSTEM_MAX_PATH ] = { '\0' };

void PlSetupLogOutput( const char *path ) {
	if ( path == NULL || path[ 0 ] == '\0' ) {
		return;
	}

	snprintf( logOutputPath, sizeof( logOutputPath ), "%s", path );
	if ( PlFileExists( logOutputPath ) ) {
		unlink( logOutputPath );
	}
}

int PlAddLogLevel( const char *prefix, QmMathColour4ub colour, bool status ) {
	int i = GetNextFreeLogLevel();
	if ( i == -1 ) {
		return -1;
	}

	LogLevel *l = &levels[ i ];
	l->colour = colour;
	l->isReserved = true;

	if ( prefix != NULL && prefix[ 0 ] != '\0' ) {
		snprintf( l->prefix, sizeof( l->prefix ), "%s", prefix );
	}

	char var[ 32 ];
	snprintf( var, sizeof( var ), "log/%s", prefix );
	l->var = PlRegisterConsoleVariable( var, "Console output level.", status ? "1" : "0", PL_VAR_BOOL, NULL, NULL, false );
	numLogLevels++;

	return i;
}

/**
 * Removes the specified log levels.
 * Keep in mind that this will not remove any
 * console variables generated from adding those
 * levels.
 */
void PlRemoveLogLevel( int id ) {
	LogLevel *l = GetLogLevelForId( id );
	if ( l == NULL ) {
		return;
	}

	PL_ZERO( l, sizeof( LogLevel ) );
}

void PlSetLogLevelStatus( int id, bool status ) {
	LogLevel *l = GetLogLevelForId( id );
	if ( l == NULL ) {
		return;
	}

	PlSetConsoleVariable( l->var, status ? "1" : "0" );
}

void PlLogMessage( int id, const char *msg, ... ) {
	LogLevel *l = GetLogLevelForId( id );
	if ( l == NULL || !l->var->b_value ) {
		return;
	}

	va_list args;
	va_start( args, msg );

	int length = pl_vscprintf( msg, args ) + 1;
	if ( length <= 0 )
		return;

	char *buf = QM_OS_MEMORY_NEW_( char, length );
	vsnprintf( buf, length, msg, args );

	va_end( args );

	/* apply prefix if provided */
#if 0
	if ( *l->prefix != '\0' ) {
		length += ( int ) strlen( l->prefix ) + 3;
		char *tmp = QM_OS_MEMORY_NEW_( char, length );
		snprintf( tmp, length, "[%s] %s", l->prefix, buf );
		qm_os_memory_free( buf );
		buf = tmp;
	}
#endif

#if defined( _WIN32 )
	OutputDebugString( buf );
#endif

	printf( "%s", buf );

	if ( ConsoleOutputCallback != NULL ) {
		// todo: pass back level
		ConsoleOutputCallback( id, buf, l->colour );
	}

	static bool avoid_recursion = false;
	if ( !avoid_recursion ) {
		if ( logOutputPath[ 0 ] != '\0' ) {
			FILE *file = fopen( logOutputPath, "a" );
			if ( file != NULL ) {
				char time[ 64 ];
				PlGetFormattedTime( "%x %X", time, sizeof( time ) );

				// add the prefix to the start
				char prefix[ 128 ];
				snprintf( prefix, sizeof( prefix ), "[%s]: ", time );

				size_t nl = strlen( prefix ) + length;
				char *logBuf = QM_OS_MEMORY_CALLOC( nl, sizeof( char ) );
				snprintf( logBuf, nl, "%s%s", prefix, buf );

				if ( fwrite( logBuf, sizeof( char ), nl, file ) != nl ) {
					avoid_recursion = true;
					PlReportErrorF( PL_RESULT_FILEERR, "failed to write to log, %s\n%s", logOutputPath, strerror( errno ) );
				}
				fclose( file );

				qm_os_memory_free( logBuf );
			} else {
				// todo, needs to be more appropriate; return details on exact issue
				avoid_recursion = true;
				PlReportErrorF( PL_RESULT_FILEREAD, "failed to open %s", logOutputPath );
			}
		}
	}

	qm_os_memory_free( buf );
}

unsigned int PlGetNumLogLevels( void ) {
	return numLogLevels;
}

/**
 * Loads in a file that provides a command per line
 * and executes each command in order.
 */
void PlExecuteConsoleScript( const char *path ) {
	PLFile *file = PlOpenFile( path, true );
	if ( file == NULL ) {
		return;
	}

	size_t length = PlGetFileSize( file );
	char *buf = QM_OS_MEMORY_NEW_( char, length + 1 );
	memcpy( buf, PlGetFileData( file ), length );
	PlCloseFile( file );

	PLLinkedList *queuedCommands = PlCreateLinkedList();

	const char *p = buf;
	while ( *p != '\0' ) {
		unsigned int l = PlDetermineLineLength( buf );
		if ( l == 0 ) {
			PlSkipLine( &p );
			continue;
		}

		char *command = QM_OS_MEMORY_NEW_( char, ++l );
		PlParseLine( &p, command, l );

		PlInsertLinkedListNode( queuedCommands, command );
	}

	qm_os_memory_free( buf );

	char *command;
	PL_ITERATE_LINKED_LIST( command, char, queuedCommands, i ) {
		PlParseConsoleString( command );
		qm_os_memory_free( command );
	}
	PlDestroyLinkedList( queuedCommands );
}
