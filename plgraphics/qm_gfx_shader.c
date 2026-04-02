// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>
// Purpose: Shaders
// Author:  Mark E. Sowden

#include <plcore/pl_parse.h>
#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"

/////////////////////////////////////////////////////////////////////////////////////
// Shader Stage
/////////////////////////////////////////////////////////////////////////////////////

static void shader_stage_destroy( void *ptr )
{
	CallGfxFunction( DestroyShaderStage, ptr );
}

QmGfxShaderStage *qm_gfx_shader_stage_create( QmGfxShaderStageType type )
{
	QmGfxShaderStage *stage = QM_OS_MEMORY_NEW_D( QmGfxShaderStage, shader_stage_destroy );
	stage->type             = type;

	CallGfxFunction( CreateShaderStage, stage );

	return stage;
}

bool qm_gfx_shader_stage_compile( QmGfxShaderStage *self, const char *buf, size_t length, const char *localDirectory )
{
	CallReturningGfxFunction( CompileShaderStage, false, self, buf, length, localDirectory );
}

void qm_gfx_shader_stage_set_definitions( QmGfxShaderStage *self, const char definitions[][ PLG_MAX_DEFINITION_LENGTH ], unsigned int numDefinitions )
{
	if ( numDefinitions > PLG_MAX_DEFINITIONS )
	{
		numDefinitions = PLG_MAX_DEFINITIONS;
	}

	self->numDefinitions = numDefinitions;
	memcpy( self->definitions, definitions, PLG_MAX_DEFINITION_LENGTH * numDefinitions );
}

/////////////////////////////////////////////////////////////////////////////////////
// Shader Program
/////////////////////////////////////////////////////////////////////////////////////

static void shader_program_destroy( void *ptr )
{
	QmGfxShaderProgram *self = ptr;

	for ( unsigned int i = 0; i < QM_GFX_MAX_SHADER_STAGE_TYPES; ++i )
	{
		if ( self->stages[ i ] != NULL )
		{
			CallGfxFunction( DetachShaderStage, self, self->stages[ i ] );
			qm_os_memory_free( self->stages[ i ] );
		}
	}

	CallGfxFunction( DestroyShaderProgram, self );

	qm_os_memory_free( self->uniforms );
	qm_os_memory_free( self->attributes );
}

QmGfxShaderProgram *qm_gfx_shader_program_create()
{
	QmGfxShaderProgram *program = QM_OS_MEMORY_NEW_D( QmGfxShaderProgram, shader_program_destroy );
	CallGfxFunction( CreateShaderProgram, program );

	return program;
}

void qm_gfx_shader_program_attach_stage( QmGfxShaderProgram *self, QmGfxShaderStage *stage )
{
	self->stages[ self->num_stages++ ] = stage;
	CallGfxFunction( AttachShaderStage, self, stage );
}

/**
 * returns the currently active shader program.
 *
 * @return pointer to the currently active shader program.
 */
QmGfxShaderProgram *PlgGetCurrentShaderProgram( void )
{
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
bool PlgIsShaderProgramEnabled( QmGfxShaderProgram *program )
{
	return gfx_state.current_program == program;
}

bool qm_gfx_shader_program_link( QmGfxShaderProgram *self )
{
	CallGfxFunction( LinkShaderProgram, self );

	return self->is_linked;
}

void PlgSetShaderProgram( QmGfxShaderProgram *program )
{
	if ( program == gfx_state.current_program )
	{
		return;
	}

	CallGfxFunction( SetShaderProgram, program );

	gfx_state.current_program = program;
}

static QmGfxShaderProgram *GetShaderProgram( QmGfxShaderProgram *program )
{
	if ( program != NULL )
	{
		return program;
	}

	if ( gfx_state.current_program != NULL )
	{
		return gfx_state.current_program;
	}

	GfxLog( "NULL shader specified for uniform write, and no active shader program bound!\n" );
	return nullptr;
}

int qm_gfx_shader_program_get_uniform_slot( QmGfxShaderProgram *program, const char *name )
{
	QmGfxShaderProgram *prg = GetShaderProgram( program );
	if ( prg == NULL )
	{
		return -1;
	}

	for ( unsigned int i = 0; i < prg->num_uniforms; ++i )
	{
		if ( *prg->uniforms[ i ].name == '\0' )
		{
			continue;
		}

		if ( pl_strcasecmp( prg->uniforms[ i ].name, name ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

QmGfxShaderUniformType qm_gfx_shader_program_get_uniform_type( const QmGfxShaderProgram *self, int slot )
{
	if ( slot < 0 || ( unsigned int ) slot >= self->num_uniforms )
	{
		return QM_GFX_SHADER_UNIFORM_TYPE_INVALID;
	}

	return self->uniforms[ slot ].type;
}

unsigned int PlgGetNumShaderUniformElements( const QmGfxShaderProgram *program, int slot )
{
	if ( slot < 0 || ( unsigned int ) slot >= program->num_uniforms )
	{
		return 0;
	}

	return program->uniforms[ slot ].numElements;
}

/*****************************************************/
/** shader uniform **/

static int ValidateShaderUniformSlot( QmGfxShaderProgram *program, int slot )
{
	if ( slot == -1 )
	{
		GfxLog( "Invalid shader uniform slot!\n" );
		return -1;
	}
	else if ( ( unsigned int ) ( slot ) >= program->num_uniforms )
	{
		GfxLog( "Potential overflow for uniform slot! (%d / %d)\n", slot, program->num_uniforms );
		return -1;
	}
	else if ( program->uniforms[ slot ].type == QM_GFX_SHADER_UNIFORM_TYPE_INVALID )
	{
		GfxLog( "Unknown uniform type for slot! (%d)\n", slot );
		return -1;
	}

	return slot;
}

void PlgSetShaderUniformDefaultValueByIndex( QmGfxShaderProgram *program, int slot, const void *defaultValue )
{
	switch ( program->uniforms[ slot ].type )
	{
		case QM_GFX_SHADER_UNIFORM_TYPE_FLOAT:
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
			program->uniforms[ slot ].defaultVec2 = *( QmMathVector2f * ) defaultValue;
			break;
		case PLG_UNIFORM_VEC3:
			program->uniforms[ slot ].defaultVec3 = *( QmMathVector3f * ) defaultValue;
			break;
		case PLG_UNIFORM_VEC4:
			program->uniforms[ slot ].defaultVec4 = *( QmMathVector4f * ) defaultValue;
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
void PlgSetShaderUniformDefaultValue( QmGfxShaderProgram *program, const char *name, const void *defaultValue )
{
	int slot = qm_gfx_shader_program_get_uniform_slot( program, name );
	if ( slot == -1 )
	{
		return;
	}

	PlgSetShaderUniformDefaultValueByIndex( program, slot, defaultValue );
}

void PlgSetShaderUniformToDefaultByIndex( QmGfxShaderProgram *program, int slot )
{
	switch ( program->uniforms[ slot ].type )
	{
		case QM_GFX_SHADER_UNIFORM_TYPE_FLOAT:
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

void PlgSetShaderUniformToDefault( QmGfxShaderProgram *program, const char *name )
{
	int slot = qm_gfx_shader_program_get_uniform_slot( program, name );
	if ( slot == -1 )
	{
		return;
	}

	PlgSetShaderUniformToDefaultByIndex( program, slot );
}

void PlgSetShaderUniformsToDefault( QmGfxShaderProgram *program )
{
	for ( unsigned int i = 0; i < program->num_uniforms; ++i )
	{
		PlgSetShaderUniformToDefaultByIndex( program, i );
	}
}

void PlgSetShaderUniformValueByIndex( QmGfxShaderProgram *program, int slot, const void *value, bool transpose )
{
	if ( ValidateShaderUniformSlot( program, slot ) == -1 )
	{
		return;
	}

	/* this should be done by the GL layer!! */
	QmGfxShaderProgram *oldProgram = PlgGetCurrentShaderProgram();
	PlgSetShaderProgram( program );

	CallGfxFunction( SetShaderUniformValue, program, slot, value, transpose );

	/* this should be done by the GL layer!! */
	PlgSetShaderProgram( oldProgram );
}

void PlgSetShaderUniformValue( QmGfxShaderProgram *program, const char *name, const void *value, bool transpose )
{
	PlgSetShaderUniformValueByIndex( program, qm_gfx_shader_program_get_uniform_slot( program, name ), value, transpose );
}
