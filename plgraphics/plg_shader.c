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

#include "plg_private.h"

#include <PL/pl_parse.h>

/* shader implementation */

/**********************************************************/
/** shader stages **/

/**
 * Create the shader stage and generate it on the GPU,
 * if applicable.
 *
 * @param type the type of shader stage.
 * @return the new shader stage.
 */
static PLGShaderStage *CreateShaderStage( PLGShaderStageType type ) {
	PLGShaderStage *stage = pl_calloc( 1, sizeof( PLGShaderStage ) );
	if ( stage == NULL ) {
		return NULL;
	}

	stage->type = type;

	CallGfxFunction( CreateShaderStage, stage );

	return stage;
}

/**
 * Delete the given shader stage and wipe it off the
 * GPU, if applicable.
 *
 * @param stage stage we're deleting.
 */
void plDestroyShaderStage( PLGShaderStage *stage ) {
	if ( stage == NULL ) {
		return;
	}

	CallGfxFunction( DestroyShaderStage, stage );
}

/**
 * Compiles the given shader stage on the GPU, otherwise it
 * will be ignored when performing software-rendering or if
 * your GPU doesn't support the shader compilation.
 *
 * If the compilation fails an error will be reported and this will
 * automatically fallback when rendering anything if it's active.
 *
 * @param stage the stage we're going to be compiling.
 * @param buf pointer to buffer containing the shader we're compiling.
 * @param length the length of the buffer.
 */
void PlgCompileShaderStage( PLGShaderStage *stage, const char *buf, size_t length ) {
	plClearError();

	CallGfxFunction( CompileShaderStage, stage, buf, length );
}

/**
 * This actually parses _and_ compiles the given stage.
 * Returns NULL on fail.
 */
PLGShaderStage *PlgParseShaderStage( PLGShaderStageType type, const char *buf, size_t length ) {
	PLGShaderStage *stage = CreateShaderStage( type );
	if ( stage == NULL ) {
		return NULL;
	}

	PlgCompileShaderStage( stage, buf, length );
	if ( plGetFunctionResult() == PL_RESULT_SHADER_COMPILE ) {
		plDestroyShaderStage( stage );
		return NULL;
	}

	return stage;
}

/**
 * Shortcut function that can be used to quickly produce a new
 * shader stage. this will automatically handle loading the given
 * shader into memory, compiling it and then returning the new
 * shader stage object. If the compilation fails an error will be reported and the
 * function will return a null pointer.
 *
 * @param path Path to the shader stage document.
 * @param type The type of shader stage.
 * @return The new shader stage on success, otherwise a null pointer.
 */
PLGShaderStage *PlgLoadShaderStage( const char *path, PLGShaderStageType type ) {
	PLFile *fp = plOpenFile( path, false );
	if ( fp == NULL ) {
		plReportErrorF( PL_RESULT_FILEREAD, "failed to open %s", path );
		return NULL;
	}

	size_t length = plGetFileSize( fp );
	char *buf = pl_malloc( length + 1 );
	if ( buf == NULL ) {
		return NULL;
	}

	size_t rlen = plReadFile( fp, buf, length, 1 );
	if ( rlen != 1 ) {
		GfxLog( "Failed to read in entirety of %s (%d)!\n"
		        "Continuing anyway but expect issues...",
		        path, rlen );
	}
	buf[ length ] = '\0';
	plCloseFile( fp );

	PLGShaderStage *stage = PlgParseShaderStage( type, buf, length );
	pl_free( buf );
	return stage;
}

/**********************************************************/
/** shader program **/

/**
 * allocates a new shader program in memory and generates it
 * on the GPU, if applicable.
 *
 * @return the new shader program.
 */
PLGShaderProgram *PlgCreateShaderProgram( void ) {
	PLGShaderProgram *program = pl_calloc( 1, sizeof( PLGShaderProgram ) );
	if ( program == NULL ) {
		return NULL;
	}

	memset( program, 0, sizeof( PLGShaderProgram ) );

	CallGfxFunction( CreateShaderProgram, program );

	return program;
}

/**
 * deletes the given shader program and also clears it on the GPU,
 * if applicable.
 *
 * @param program the program being deleted.
 * @param free_stages if true, automatically frees any linked shader stages.
 */
void PlgDestroyShaderProgram( PLGShaderProgram *program, bool free_stages ) {
	if ( program == NULL ) {
		return;
	}

	for ( unsigned int i = 0; i < PLG_MAX_SHADER_TYPES; ++i ) {
		if ( program->stages[ i ] != NULL ) {
			CallGfxFunction( DetachShaderStage, program, program->stages[ i ] );
			if ( free_stages ) {
				pl_free( program->stages[ i ] );
			}
		}
	}

	CallGfxFunction( DestroyShaderProgram, program );

	/* free uniforms */
	for ( unsigned int i = 0; i < program->num_uniforms; ++i ) {
		free( program->uniforms[ i ].name );
	}
	pl_free( program->uniforms );

	pl_free( program->attributes );
	pl_free( program );
}

void PlgAttachShaderStage( PLGShaderProgram *program, PLGShaderStage *stage ) {
	plAssert( program != NULL );
	plAssert( stage != NULL );
	program->stages[ program->num_stages++ ] = stage;
	CallGfxFunction( AttachShaderStage, program, stage );
}

bool PlgRegisterShaderStageFromMemory( PLGShaderProgram *program, const char *buffer, size_t length,
                                      PLGShaderStageType type ) {
	if ( program->num_stages >= PLG_MAX_SHADER_TYPES ) {
		plReportErrorF( PL_RESULT_MEMORY_EOA, "reached maximum number of available shader stage slots (%u)",
		             program->num_stages );
		return false;
	}

	PLGShaderStage *stage = PlgParseShaderStage( type, buffer, length );
	if ( stage == NULL ) {
		return false;
	}

	PlgAttachShaderStage( program, stage );

	return true;
}

bool PlgRegisterShaderStageFromDisk( PLGShaderProgram *program, const char *path, PLGShaderStageType type ) {
	if ( program->num_stages >= PLG_MAX_SHADER_TYPES ) {
		plReportErrorF( PL_RESULT_MEMORY_EOA, "reached maximum number of available shader stage slots (%u)",
		             program->num_stages );
		return false;
	}

	PLGShaderStage *stage = PlgLoadShaderStage( path, type );
	if ( stage == NULL ) {
		return false;
	}

	PlgAttachShaderStage( program, stage );

	return true;
}

/**
 * returns the currently active shader program.
 *
 * @return pointer to the currently active shader program.
 */
PLGShaderProgram *PlgGetCurrentShaderProgram( void ) {
	return gfx_state.current_program;
}

/**
 * checks whether the given shader program is currently
 * enabled - meaning it will be used in any subsequent draw
 * calls.
 *
 * @param program
 * @return true if the program is equal to that currently enabled.
 */
bool PlgIsShaderProgramEnabled( PLGShaderProgram *program ) {
	if ( gfx_state.current_program == program ) {
		return true;
	}

	return false;
}

bool PlgLinkShaderProgram( PLGShaderProgram *program ) {
	CallGfxFunction( LinkShaderProgram, program );

	return program->is_linked;
}

/**
 * sets the currently active shader program. means that any
 * subsequent draw calls will use this shader program.
 *
 * @param program
 */
void PlgSetShaderProgram( PLGShaderProgram *program ) {
	if ( program == gfx_state.current_program ) {
		return;
	}

	CallGfxFunction( SetShaderProgram, program );

	gfx_state.current_program = program;
}

static PLGShaderProgram *GetShaderProgram( PLGShaderProgram *program ) {
	if ( program != NULL ) {
		return program;
	} else if ( gfx_state.current_program != NULL ) {
		return gfx_state.current_program;
	}

	GfxLog( "NULL shader specified for uniform write, and no active shader program bound!\n" );
	return NULL;
}

/**
 * Searches through programs registered uniforms
 * for the specified uniform entry.
 *
 * If it fails to find the uniform it'll return '-1'.
 */
int PlgGetShaderUniformSlot( PLGShaderProgram *program, const char *name ) {
	PLGShaderProgram *prg = GetShaderProgram( program );
	if ( prg == NULL ) {
		return -1;
	}

	for ( int i = 0; i < prg->num_uniforms; ++i ) {
		if ( prg->uniforms[ i ].name == NULL ) {
			continue;
		}

		if ( pl_strcasecmp( prg->uniforms[ i ].name, name ) == 0 ) {
			return i;
		}
	}

	return -1;
}

PLGShaderUniformType PlgGetShaderUniformType( const PLGShaderProgram *program, int slot ) {
	if ( slot == -1 || slot >= program->num_uniforms ) {
		return PLG_INVALID_UNIFORM;
	}

	return program->uniforms[ slot ].type;
}

/*****************************************************/
/** shader uniform **/

static int ValidateShaderUniformSlot( PLGShaderProgram *program, int slot ) {
	if ( slot == -1 ) {
		GfxLog( "Invalid shader uniform slot, \"%d\"!\n", slot );
		return -1;
	} else if ( ( unsigned int ) ( slot ) >= program->num_uniforms ) {
		GfxLog( "Potential overflow for uniform slot! (%d / %d)\n", slot, program->num_uniforms );
		return -1;
	} else if ( program->uniforms[ slot ].type == PLG_INVALID_UNIFORM ) {
		GfxLog( "Unknown uniform type for slot! (%d)\n", slot );
		return -1;
	}

	return slot;
}

void PlgSetShaderUniformDefaultValueByIndex( PLGShaderProgram *program, int slot, const void *defaultValue ) {
	switch ( program->uniforms[ slot ].type ) {
		case PLG_UNIFORM_FLOAT:
			program->uniforms[ slot ].defaultFloat = *( float * ) defaultValue;
			break;
		case PLG_UNIFORM_SAMPLER2D:
		case PLG_UNIFORM_INT:
			program->uniforms[ slot ].defaultInt = *( int * ) defaultValue;
			break;
		case PLG_UNIFORM_UINT:
			program->uniforms[ slot ].defaultUInt = *( unsigned int * ) defaultValue;
			break;
		case PLG_UNIFORM_BOOL:
			program->uniforms[ slot ].defaultBool = *( bool * ) defaultValue;
			break;
		case PLG_UNIFORM_DOUBLE:
			program->uniforms[ slot ].defaultDouble = *( double * ) defaultValue;
			break;
		case PLG_UNIFORM_VEC2:
			program->uniforms[ slot ].defaultVec2 = *( PLVector2 * ) defaultValue;
			break;
		case PLG_UNIFORM_VEC3:
			program->uniforms[ slot ].defaultVec3 = *( PLVector3 * ) defaultValue;
			break;
		case PLG_UNIFORM_VEC4:
			program->uniforms[ slot ].defaultVec4 = *( PLVector4 * ) defaultValue;
			break;
		case PLG_UNIFORM_MAT3:
			program->uniforms[ slot ].defaultMat3 = *( PLMatrix3 * ) defaultValue;
			break;
		case PLG_UNIFORM_MAT4:
			program->uniforms[ slot ].defaultMat4 = *( PLMatrix4 * ) defaultValue;
			break;
		default:
			break;
	}
}

/**
 * Set a default value to use for the particular uniform that
 * it can be reset to later.
 */
void PlgSetShaderUniformDefaultValue( PLGShaderProgram *program, const char *name, const void *defaultValue ) {
	int slot = PlgGetShaderUniformSlot( program, name );
	if ( slot == -1 ) {
		return;
	}

	PlgSetShaderUniformDefaultValueByIndex( program, slot, defaultValue );
}

void PlgSetShaderUniformToDefaultByIndex( PLGShaderProgram *program, int slot ) {
	switch ( program->uniforms[ slot ].type ) {
		case PLG_UNIFORM_FLOAT:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultFloat, false );
			break;
		case PLG_UNIFORM_SAMPLER2D:
		case PLG_UNIFORM_INT:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultInt, false );
			break;
		case PLG_UNIFORM_UINT:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultUInt, false );
			break;
		case PLG_UNIFORM_BOOL:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultBool, false );
			break;
		case PLG_UNIFORM_DOUBLE:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultDouble, false );
			break;
		case PLG_UNIFORM_VEC2:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultVec2, false );
			break;
		case PLG_UNIFORM_VEC3:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultVec3, false );
			break;
		case PLG_UNIFORM_VEC4:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultVec4, false );
			break;
		case PLG_UNIFORM_MAT3:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultMat3, false );
			break;
		case PLG_UNIFORM_MAT4:
			PlgSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultMat4, false );
			break;
		default:
			break;
	}
}

void PlgSetShaderUniformToDefault( PLGShaderProgram *program, const char *name ) {
	int slot = PlgGetShaderUniformSlot( program, name );
	if ( slot == -1 ) {
		return;
	}

	PlgSetShaderUniformToDefaultByIndex( program, slot );
}

void PlgSetShaderUniformsToDefault( PLGShaderProgram *program ) {
	for ( int i = 0; i < program->num_uniforms; ++i ) {
		PlgSetShaderUniformToDefaultByIndex( program, i );
	}
}

void PlgSetShaderUniformValueByIndex( PLGShaderProgram *program, int slot, const void *value, bool transpose ) {
	if ( ValidateShaderUniformSlot( program, slot ) == -1 ) {
		return;
	}

	/* this should be done by the GL layer!! */
	PLGShaderProgram *oldProgram = PlgGetCurrentShaderProgram();
	PlgSetShaderProgram( program );

	CallGfxFunction( SetShaderUniformValue, program, slot, value, transpose );

	/* this should be done by the GL layer!! */
	PlgSetShaderProgram( oldProgram );
}

void PlgSetShaderUniformValue( PLGShaderProgram *program, const char *name, const void *value, bool transpose ) {
	PlgSetShaderUniformValueByIndex( program, PlgGetShaderUniformSlot( program, name ), value, transpose );
}

void plSetShaderUniformFloat( PLGShaderProgram *program, int slot, float value ) {
	PlgSetShaderUniformValueByIndex( program, slot, &value, false );
}

void plSetShaderUniformVector3( PLGShaderProgram *program, int slot, PLVector3 value ) {
	PlgSetShaderUniformValueByIndex( program, slot, &value, false );
}

void plSetShaderUniformVector4( PLGShaderProgram *program, int slot, PLVector4 value ) {
	PlgSetShaderUniformValueByIndex( program, slot, &value, false );
}

void plSetShaderUniformInt( PLGShaderProgram *program, int slot, int value ) {
	PlgSetShaderUniformValueByIndex( program, slot, &value, false );
}

void plSetShaderUniformMatrix4( PLGShaderProgram *program, int slot, PLMatrix4 value, bool transpose ) {
	PlgSetShaderUniformValueByIndex( program, slot, &value, transpose );
}
