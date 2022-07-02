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
#include <plcore/pl_math.h>

PL_EXTERN_C

/* todo: make this structure private */
typedef struct PLConsoleVariable {
	char *name;
	char *description;

	PLVariableType type;

	void ( *CallbackFunction )( const struct PLConsoleVariable *variable );

	/////////////////////////////

#define PL_VAR_VALUE_LENGTH 512

	union {
		float f_value;
		int i_value;
		const char *s_value;
		bool b_value;
	};
	char value[ PL_VAR_VALUE_LENGTH ];
	char default_value[ PL_VAR_VALUE_LENGTH ];
	void *ptrValue;

	/////////////////////////////

	bool archive;
} PLConsoleVariable;

#if !defined( PL_COMPILE_PLUGIN )

void PlGetConsoleVariables( PLConsoleVariable ***vars, size_t *num_vars );

PLConsoleVariable *PlGetConsoleVariable( const char *name );
const char *PlGetConsoleVariableValue( const char *name );
const char *PlGetConsoleVariableDefaultValue( const char *name );
void PlSetConsoleVariable( PLConsoleVariable *var, const char *value );
void PlSetConsoleVariableByName( const char *name, const char *value );

PLConsoleVariable *PlRegisterConsoleVariable( const char *name, const char *description, const char *defaultValue, PLVariableType type, void *ptrValue, void ( *CallbackFunction )( const PLConsoleVariable * ), bool archive );

/* fetch and cache console var for trivial lookup */
#	define PL_GET_CVAR( NAME, STORE )                    \
		static PLConsoleVariable *( STORE ) = NULL;       \
		if ( ( STORE ) == NULL ) {                        \
			( STORE ) = PlGetConsoleVariable( ( NAME ) ); \
			assert( ( STORE ) != NULL );                  \
		}

#endif /* !defined( PL_COMPILE_PLUGIN ) */

/////////////////////////////////////////////////////////////////////////////////////

typedef struct PLConsoleCommand {
	char *name;
	char *description;
	int args;
	void ( *Callback )( unsigned int argc, char *argv[] );
} PLConsoleCommand;

#if !defined( PL_COMPILE_PLUGIN )

void PlGetConsoleCommands( PLConsoleCommand ***cmds, size_t *num_cmds );
void PlRegisterConsoleCommand( const char *name, const char *description, int args, void ( *CallbackFunction )( unsigned int argc, char *argv[] ) );

void PlSetConsoleOutputCallback( void ( *Callback )( int level, const char *msg, PLColour colour ) );

const char **PlAutocompleteConsoleString( const char *string, unsigned int *numElements );
void PlParseConsoleString( const char *string );

void PlExecuteConsoleScript( const char *path );

/////////////////////////////////////////////////////////////////////////////////////

void PlSetupLogOutput( const char *path );
int PlAddLogLevel( const char *prefix, PLColour colour, bool status );
void PlRemoveLogLevel( int id );
void PlSetLogLevelStatus( int id, bool status );
void PlLogMessage( int id, const char *msg, ... );

#	define PlLogWFunction( ID, FORMAT, ... ) PlLogMessage( ( ID ), "(%s) " FORMAT, PL_FUNCTION, ##__VA_ARGS__ )

#endif /* !defined( PL_COMPILE_PLUGIN ) */

#define PL_REGISTER_CMD_SIMPLE( NAME, CALLBACK )               PlRegisterConsoleCommand( ( NAME ), NULL, -1, ( CALLBACK ) )
#define PL_REGISTER_VAR_SIMPLE( NAME, TYPE, DEFAULT, ARCHIVE ) PlRegisterConsoleVariable( ( NAME ), NULL, ( DEFAULT ), ( TYPE ), NULL, NULL, ( ARCHIVE ) )

PL_EXTERN_C_END
