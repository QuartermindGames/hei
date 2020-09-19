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

#include "filesystem_private.h"
#include "graphics_private.h"

#include <PL/pl_parse.h>

/* todo: move this shit out of here! */
#if defined( PL_USE_GLEW )
#   include <GL/glew.h>
#else
#   ifndef GL_GLEXT_PROTOTYPES
#       define GL_GLEXT_PROTOTYPES
#   endif

#   include <GL/gl.h>
#   include <GL/glext.h>
#endif

/* shader implementation */

/**********************************************************/
/** preprocessor **/

#if defined(PL_SUPPORT_OPENGL)

/**
 * Inserts the given string into an existing string buffer.
 * Automatically reallocs buffer if it doesn't fit.
 */
static char *_plInsertString( const char *string, char **buf, size_t *bufSize, size_t *maxBufSize ) {
	/* check if it's going to fit first */
	size_t strLength = strlen( string );
	size_t originalSize = *bufSize;
	*bufSize += strLength;
	if ( *bufSize >= *maxBufSize ) {
		*maxBufSize = *bufSize + strLength;
		*buf = pl_realloc( *buf, *maxBufSize );
	}

	/* now copy it into our buffer */
	strncpy( *buf + originalSize, string, strLength );

	return *buf + originalSize + strLength;
}

static char *_plSkipLine( char *buf ) {
	while ( *buf != '\0' &&
	        *buf != '\n' &&
	        *buf != '\r' ) {
		buf++;
	}

	return buf;
}

/**
 * A basic pre-processor for GLSL - will condense the shader as much as possible
 * and handle any pre-processor commands.
 * todo: this is dumb... rewrite and move it
 */
static char *GLPreProcessGLSLShader( char *buf, size_t *length, PLShaderStageType type ) {
	/* setup the destination buffer */
	size_t actualLength = 0;
	size_t maxLength = *length;
    char *dstBuffer = pl_calloc( maxLength, sizeof( char ) );
	char *dstPos = dstBuffer;

    /* built-ins */
#define insert( str ) dstPos = _plInsertString( ( str ), &dstBuffer, &actualLength, &maxLength )
    insert( "#version 150 core\n" ); //OpenGL 3.2 == GLSL 150
	insert( "uniform mat4 pl_model;" );
	insert( "uniform mat4 pl_view;" );
	insert( "uniform mat4 pl_proj;" );
    if(type == PL_SHADER_TYPE_VERTEX) {
		insert( "in vec3 pl_vposition;" );
		insert( "in vec3 pl_vnormal;" );
		insert( "in vec2 pl_vuv;" );
		insert( "in vec4 pl_vcolour;" );
    } else if(type == PL_SHADER_TYPE_FRAGMENT) {
		insert( "out vec4 pl_frag;" );
    }

	char *srcPos = buf;
    char *srcEnd = buf + *length;
    while( srcPos < srcEnd ) {
    	if ( *srcPos == '\0' ) {
    		break;
    	}

	    if(*srcPos == '\n' || *srcPos == '\r' || *srcPos == '\t') {
		    srcPos++;
		    continue;
	    }

	    if(srcPos[0] == ' ' && srcPos[1] == ' ') {
		    srcPos += 2;
		    srcPos = plSkipSpaces( srcPos );
		    continue;
	    }

	    /* skip comments */
	    if(srcPos[0] == '/' && srcPos[1] == '*') {
		    srcPos += 2;
		    while(!(srcPos[0] == '*' && srcPos[1] == '/')) srcPos++;
		    srcPos += 2;
		    continue;
	    }

	    if(srcPos[0] == '/' && srcPos[1] == '/') {
		    srcPos += 2;
		    srcPos = _plSkipLine( srcPos );
		    continue;
	    }

	    if ( ++actualLength > maxLength ) {
		    ++maxLength;

		    char *oldDstBuffer = dstBuffer;
		    dstBuffer = pl_realloc(dstBuffer, maxLength);

		    dstPos = dstBuffer + (dstPos - oldDstBuffer);
	    }

	    *dstPos++ = *srcPos++;
    }

    /* free the original buffer that was passed in */
	pl_free( buf );

    /* resize and update buf to match */
	*length = actualLength;

	return dstBuffer;
}

#endif

/**********************************************************/

/**********************************************************/
/** shader stages **/

/**
 * Create the shader stage and generate it on the GPU,
 * if applicable.
 *
 * @param type the type of shader stage.
 * @return the new shader stage.
 */
static PLShaderStage *CreateShaderStage(PLShaderStageType type) {
    PLShaderStage *stage = pl_calloc(1, sizeof(PLShaderStage));
    if(stage == NULL) {
        return NULL;
    }

    stage->type = type;

    CallGfxFunction(CreateShaderStage, stage);

    return stage;
}

/**
 * Delete the given shader stage and wipe it off the
 * GPU, if applicable.
 *
 * @param stage stage we're deleting.
 */
void plDestroyShaderStage(PLShaderStage *stage) {
    if(stage == NULL) {
        return;
    }

    CallGfxFunction(DestroyShaderStage, stage);
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
void plCompileShaderStage( PLShaderStage *stage, const char *buf, size_t length ) {
	_plResetError();

#if defined( PL_SUPPORT_OPENGL )
	char *bufDst = pl_malloc( length );
	memcpy( bufDst, buf, length );

	bufDst = GLPreProcessGLSLShader( bufDst, &length, stage->type );

	CallGfxFunction( CompileShaderStage, stage, bufDst, length );

	pl_free( bufDst );
#else
	CallGfxFunction( CompileShaderStage, stage, buf, length );
#endif
}

PLShaderStage *plParseShaderStage(PLShaderStageType type, const char *buf, size_t length) {
    PLShaderStage *stage = CreateShaderStage(type);
    if(stage == NULL) {
        return NULL;
    }

    plCompileShaderStage(stage, buf, length);
    if(plGetFunctionResult() == PL_RESULT_SHADER_COMPILE) {
        plDestroyShaderStage(stage);
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
PLShaderStage *plLoadShaderStage(const char *path, PLShaderStageType type) {
    PLFile *fp = plOpenFile(path, false);
    if(fp == NULL) {
        ReportError(PL_RESULT_FILEREAD, "failed to open %s", path);
        return NULL;
    }

    size_t length = plGetFileSize(fp);
    char *buf = pl_malloc(length + 1);
    if(buf == NULL) {
      return NULL;
    }

    size_t rlen = plReadFile(fp, buf, length, 1);
    if(rlen != 1) {
        GfxLog("Failed to read in entirety of %s (%d)!\n"
               "Continuing anyway but expect issues...", path, rlen);
    }
    buf[length] = '\0';
    plCloseFile(fp);

    PLShaderStage *stage = plParseShaderStage( type, buf, length );
    pl_free(buf);
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
PLShaderProgram *plCreateShaderProgram(void) {
    PLShaderProgram *program = pl_calloc(1, sizeof(PLShaderProgram));
    if(program == NULL) {
        return NULL;
    }

    memset(program, 0, sizeof(PLShaderProgram));

    CallGfxFunction(CreateShaderProgram, program);

    return program;
}

/**
 * deletes the given shader program and also clears it on the GPU,
 * if applicable.
 *
 * @param program the program being deleted.
 * @param free_stages if true, automatically frees any linked shader stages.
 */
void plDestroyShaderProgram(PLShaderProgram *program, bool free_stages) {
    if(program == NULL) {
        return;
    }

    for(unsigned int i = 0; i < PL_MAX_SHADER_TYPES; ++i) {
        if(program->stages[i] != NULL) {
            CallGfxFunction(DetachShaderStage, program, program->stages[i]);
            if(free_stages) {
                pl_free(program->stages[i]);
            }
        }
    }

    CallGfxFunction(DestroyShaderProgram, program);

	/* free uniforms */
	for ( unsigned int i = 0; i < program->num_uniforms; ++i ) {
		free( program->uniforms[ i ].name );
	}
    pl_free(program->uniforms);

    pl_free(program->attributes);
    pl_free(program);
}

void plAttachShaderStage(PLShaderProgram *program, PLShaderStage *stage) {
    plAssert(program != NULL);
    plAssert(stage != NULL);
    program->stages[program->num_stages++] = stage;
    CallGfxFunction(AttachShaderStage, program, stage);
}

bool plRegisterShaderStageFromMemory(PLShaderProgram *program, const char *buffer, size_t length,
                                     PLShaderStageType type) {
    if(program->num_stages >= PL_MAX_SHADER_TYPES) {
        ReportError(PL_RESULT_MEMORY_EOA, "reached maximum number of available shader stage slots (%u)",
                    program->num_stages);
        return false;
    }

    PLShaderStage *stage = plParseShaderStage(type, buffer, length);
    if(stage == NULL) {
        return false;
    }

    plAttachShaderStage(program, stage);

    return true;
}

bool plRegisterShaderStageFromDisk(PLShaderProgram *program, const char *path, PLShaderStageType type) {
    if(program->num_stages >= PL_MAX_SHADER_TYPES) {
        ReportError(PL_RESULT_MEMORY_EOA, "reached maximum number of available shader stage slots (%u)",
                    program->num_stages);
        return false;
    }

    PLShaderStage *stage = plLoadShaderStage(path, type);
    if(stage == NULL) {
        return false;
    }

    plAttachShaderStage(program, stage);

    return true;
}

/**
 * returns the currently active shader program.
 *
 * @return pointer to the currently active shader program.
 */
PLShaderProgram *plGetCurrentShaderProgram(void) {
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
bool plIsShaderProgramEnabled(PLShaderProgram *program) {
    if(gfx_state.current_program == program) {
        return true;
    }

    return false;
}

static void RegisterShaderProgramData(PLShaderProgram *program);
bool plLinkShaderProgram(PLShaderProgram *program) {
    FunctionStart();

    CallGfxFunction(LinkShaderProgram, program);

    RegisterShaderProgramData(program);

    return program->is_linked;
}

/**
 * sets the currently active shader program. means that any
 * subsequent draw calls will use this shader program.
 *
 * @param program
 */
void plSetShaderProgram(PLShaderProgram *program) {
    if (program == gfx_state.current_program) {
        return;
    }

    CallGfxFunction(SetShaderProgram, program);

    gfx_state.current_program = program;
}

static PLShaderProgram *GetShaderProgram(PLShaderProgram *program) {
    if(program != NULL) {
        return program;
    } else if(gfx_state.current_program != NULL) {
        return gfx_state.current_program;
    }

    GfxLog("NULL shader specified for uniform write, and no active shader program bound!\n");
    return NULL;
}

/**
 * Searches through programs registered uniforms
 * for the specified uniform entry.
 *
 * If it fails to find the uniform it'll return '-1'.
 */
int plGetShaderUniformSlot( PLShaderProgram *program, const char *name ) {
	PLShaderProgram *prg = GetShaderProgram( program );
	if ( prg == NULL ) {
		return -1;
	}

	for ( unsigned int i = 0; i < prg->num_uniforms; ++i ) {
		if ( prg->uniforms[ i ].name == NULL ) {
			continue;
		}

		if ( pl_strcasecmp( prg->uniforms[ i ].name, name ) == 0 ) {
			return i;
		}
	}

	return -1;
}

PLShaderUniformType plGetShaderUniformType( const PLShaderProgram *program, int slot ) {
	if ( slot == -1 || slot >= program->num_uniforms ) {
		return PL_INVALID_UNIFORM;
	}

	return program->uniforms[ slot ].type;
}

/*****************************************************/
/** shader uniform **/

#if defined(PL_SUPPORT_OPENGL)

static const char *uniformDescriptors[ PL_MAX_UNIFORM_TYPES ] = {
		[ PL_INVALID_UNIFORM ] = "invalid",
        [ PL_UNIFORM_FLOAT ] = "float",
		[ PL_UNIFORM_INT ] = "int",
		[ PL_UNIFORM_UINT ] = "uint",
		[ PL_UNIFORM_BOOL ] = "bool",
		[ PL_UNIFORM_DOUBLE ] = "double",
		[ PL_UNIFORM_SAMPLER1D ] = "sampler1D",
		[ PL_UNIFORM_SAMPLER2D ] = "sampler2D",
		[ PL_UNIFORM_SAMPLER3D ] = "sampler3D",
		[ PL_UNIFORM_SAMPLERCUBE ] = "samplerCube",
		[ PL_UNIFORM_SAMPLER1DSHADOW ] = "sampler1DShadow",
		[ PL_UNIFORM_VEC2 ] = "vec2",
		[ PL_UNIFORM_VEC3 ] = "vec3",
		[ PL_UNIFORM_VEC4 ] = "vec4",
		[ PL_UNIFORM_MAT3 ] = "mat3",
		[ PL_UNIFORM_MAT4 ] = "mat4",
};

/* todo, move into layer_opengl */
static PLShaderUniformType GLConvertGLUniformType(unsigned int type) {
    switch(type) {
        case GL_FLOAT:      return PL_UNIFORM_FLOAT;
        case GL_FLOAT_VEC2: return PL_UNIFORM_VEC2;
        case GL_FLOAT_VEC3: return PL_UNIFORM_VEC3;
        case GL_FLOAT_VEC4: return PL_UNIFORM_VEC4;
        case GL_FLOAT_MAT3: return PL_UNIFORM_MAT3;
        case GL_FLOAT_MAT4: return PL_UNIFORM_MAT4;

        case GL_DOUBLE: return PL_UNIFORM_DOUBLE;

        case GL_INT:            return PL_UNIFORM_INT;
        case GL_UNSIGNED_INT:   return PL_UNIFORM_UINT;

        case GL_BOOL:   return PL_UNIFORM_BOOL;

        case GL_SAMPLER_1D:         return PL_UNIFORM_SAMPLER1D;
        case GL_SAMPLER_1D_SHADOW:  return PL_UNIFORM_SAMPLER1DSHADOW;
        case GL_SAMPLER_2D:         return PL_UNIFORM_SAMPLER2D;
        case GL_SAMPLER_2D_SHADOW:  return PL_UNIFORM_SAMPLER2DSHADOW;

        default: {
            GfxLog("Unhandled GLSL data type, \"%u\"!\n", type);
            return PL_INVALID_UNIFORM;
        }
    }
}

#endif

static void RegisterShaderProgramData(PLShaderProgram *program) {
    /* todo, move into layer_opengl */

    if(program->uniforms != NULL) {
        GfxLog("Uniforms have already been initialised!\n");
        return;
    }

#if defined(PL_SUPPORT_OPENGL)
    program->internal.v_position = glGetAttribLocation(program->internal.id, "pl_vposition");
    program->internal.v_normal = glGetAttribLocation(program->internal.id, "pl_vnormal");
    program->internal.v_uv = glGetAttribLocation(program->internal.id, "pl_vuv");
    program->internal.v_colour = glGetAttribLocation(program->internal.id, "pl_vcolour");

    int num_uniforms = 0;
    glGetProgramiv(program->internal.id, GL_ACTIVE_UNIFORMS, &num_uniforms);
    if(num_uniforms <= 0) {
        /* true, because technically this isn't a fault - there just aren't any */
        GfxLog("No uniforms found in shader program...\n");
        return;
    }
    program->num_uniforms = (unsigned int) num_uniforms;

    GfxLog("Found %u uniforms in shader\n", program->num_uniforms);

	program->uniforms = pl_calloc( ( size_t ) program->num_uniforms, sizeof( *program->uniforms ) );
	unsigned int registered = 0;
	for ( unsigned int i = 0; i < program->num_uniforms; ++i ) {
		int maxUniformNameLength;
		glGetActiveUniformsiv( program->internal.id, 1, &i, GL_UNIFORM_NAME_LENGTH, &maxUniformNameLength );

		GLchar *uniformName = malloc( maxUniformNameLength );
		GLsizei nameLength;

		GLenum glType;
		GLint uniformSize;

		glGetActiveUniform( program->internal.id, i, maxUniformNameLength, &nameLength, &uniformSize, &glType, uniformName );
		if ( nameLength == 0 ) {
			free( uniformName );

			GfxLog( "No information available for uniform %d!\n", i );
			continue;
		}

		program->uniforms[ i ].type = GLConvertGLUniformType( glType );
		program->uniforms[ i ].slot = i;
		program->uniforms[ i ].name = uniformName;

		/* fetch it's current value, assume it's the default */
		switch( program->uniforms[ i ].type ) {
			case PL_UNIFORM_FLOAT:
				glGetUniformfv( program->internal.id, i, &program->uniforms[ i ].defaultFloat );
				break;
			case PL_UNIFORM_SAMPLER2D:
			case PL_UNIFORM_INT:
				glGetUniformiv( program->internal.id, i, &program->uniforms[ i ].defaultInt );
				break;
			case PL_UNIFORM_UINT:
				glGetUniformuiv( program->internal.id, i, &program->uniforms[ i ].defaultUInt );
				break;
			case PL_UNIFORM_BOOL:
				glGetUniformiv( program->internal.id, i, ( GLint * ) &program->uniforms[ i ].defaultBool );
				break;
			case PL_UNIFORM_DOUBLE:
				glGetUniformdv( program->internal.id, i, &program->uniforms[ i ].defaultDouble );
				break;
			case PL_UNIFORM_VEC2:
				glGetUniformfv( program->internal.id, i, ( GLfloat * ) &program->uniforms[ i ].defaultVec2 );
				break;
			case PL_UNIFORM_VEC3:
				glGetUniformfv( program->internal.id, i, ( GLfloat * ) &program->uniforms[ i ].defaultVec3 );
				break;
			case PL_UNIFORM_VEC4:
				glGetUniformfv( program->internal.id, i, ( GLfloat * ) &program->uniforms[ i ].defaultVec4 );
				break;
			case PL_UNIFORM_MAT3:
				glGetUniformfv( program->internal.id, i, ( GLfloat * ) &program->uniforms[ i ].defaultMat3 );
				break;
			case PL_UNIFORM_MAT4:
				glGetUniformfv( program->internal.id, i, ( GLfloat * ) &program->uniforms[ i ].defaultMat4 );
				break;
			default:
				break;
		}

		GfxLog( " %4d (%20s) %s\n", i, program->uniforms[ i ].name, uniformDescriptors[ program->uniforms[ i ].type ] );

		registered++;
	}

	if(registered == 0) {
        GfxLog("Failed to validate any shader program uniforms!\n");
    }
#endif
}

static int ValidateShaderUniformSlot(PLShaderProgram* program, int slot) {
    if(slot == -1) {
        GfxLog("Invalid shader uniform slot, \"%d\"!\n", slot);
        return -1;
    } else if((unsigned int)(slot) >= program->num_uniforms) {
        GfxLog("Potential overflow for uniform slot! (%d / %d)\n", slot, program->num_uniforms);
        return -1;
    } else if(program->uniforms[slot].type == PL_INVALID_UNIFORM) {
        GfxLog("Unknown uniform type for slot! (%d)\n", slot);
        return -1;
    }

    return slot;
}

void plSetShaderUniformDefaultValueByIndex( PLShaderProgram *program, int slot, const void *defaultValue ) {
	switch ( program->uniforms[ slot ].type ) {
		case PL_UNIFORM_FLOAT:
			program->uniforms[ slot ].defaultFloat = *( float * ) defaultValue;
			break;
		case PL_UNIFORM_SAMPLER2D:
		case PL_UNIFORM_INT:
			program->uniforms[ slot ].defaultInt = *( int * ) defaultValue;
			break;
		case PL_UNIFORM_UINT:
			program->uniforms[ slot ].defaultUInt = *( unsigned int * ) defaultValue;
			break;
		case PL_UNIFORM_BOOL:
			program->uniforms[ slot ].defaultBool = *( bool * ) defaultValue;
			break;
		case PL_UNIFORM_DOUBLE:
			program->uniforms[ slot ].defaultDouble = *( double * ) defaultValue;
			break;
		case PL_UNIFORM_VEC2:
			program->uniforms[ slot ].defaultVec2 = *( PLVector2 * ) defaultValue;
			break;
		case PL_UNIFORM_VEC3:
			program->uniforms[ slot ].defaultVec3 = *( PLVector3 * ) defaultValue;
			break;
		case PL_UNIFORM_VEC4:
			program->uniforms[ slot ].defaultVec4 = *( PLVector4 * ) defaultValue;
			break;
		case PL_UNIFORM_MAT3:
			program->uniforms[ slot ].defaultMat3 = *( PLMatrix3 * ) defaultValue;
			break;
		case PL_UNIFORM_MAT4:
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
void plSetShaderUniformDefaultValue( PLShaderProgram *program, const char *name, const void *defaultValue ) {
	int slot = plGetShaderUniformSlot( program, name );
	if ( slot == -1 ) {
		return;
	}

	plSetShaderUniformDefaultValueByIndex( program, slot, defaultValue );
}

void plSetShaderUniformToDefaultByIndex( PLShaderProgram *program, int slot ) {
	switch ( program->uniforms[ slot ].type ) {
		case PL_UNIFORM_FLOAT:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultFloat, false );
			break;
		case PL_UNIFORM_SAMPLER2D:
		case PL_UNIFORM_INT:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultInt, false );
			break;
		case PL_UNIFORM_UINT:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultUInt, false );
			break;
		case PL_UNIFORM_BOOL:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultBool, false );
			break;
		case PL_UNIFORM_DOUBLE:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultDouble, false );
			break;
		case PL_UNIFORM_VEC2:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultVec2, false );
			break;
		case PL_UNIFORM_VEC3:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultVec3, false );
			break;
		case PL_UNIFORM_VEC4:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultVec4, false );
			break;
		case PL_UNIFORM_MAT3:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultMat3, false );
			break;
		case PL_UNIFORM_MAT4:
			plSetShaderUniformValueByIndex( program, slot, &program->uniforms[ slot ].defaultMat4, false );
			break;
		default:
			break;
	}
}

void plSetShaderUniformToDefault( PLShaderProgram *program, const char *name ) {
	int slot = plGetShaderUniformSlot( program, name );
	if ( slot == -1 ) {
		return;
	}

	plSetShaderUniformToDefaultByIndex( program, slot );
}

void plSetShaderUniformsToDefault( PLShaderProgram *program ) {
	for ( unsigned int i = 0; i < program->num_uniforms; ++i ) {
		plSetShaderUniformToDefaultByIndex( program, i );
	}
}

void plSetShaderUniformValueByIndex( PLShaderProgram *program, int slot, const void *value, bool transpose ) {
#if defined(PL_SUPPORT_OPENGL) /* todo, move into layer_opengl */
	/* this should be done by the GL layer!! */
	PLShaderProgram* oldProgram = plGetCurrentShaderProgram();
	plSetShaderProgram( program );

	switch ( program->uniforms[ slot ].type ) {
		case PL_UNIFORM_FLOAT:
			glUniform1f( program->uniforms[ slot ].slot, *( float * ) value );
			break;
		case PL_UNIFORM_SAMPLER2D:
		case PL_UNIFORM_INT:
			glUniform1i(  program->uniforms[ slot ].slot, *( int * ) value );
			break;
		case PL_UNIFORM_UINT:
			glUniform1ui(  program->uniforms[ slot ].slot, *( unsigned int * ) value );
			break;
		case PL_UNIFORM_BOOL:
			glUniform1i(  program->uniforms[ slot ].slot, *( bool * ) value );
			break;
		case PL_UNIFORM_DOUBLE:
			glUniform1d(  program->uniforms[ slot ].slot, *( double * ) value );
			break;
		case PL_UNIFORM_VEC2: {
			PLVector2 vec2 = *( PLVector2 * ) value;
			glUniform2f( program->uniforms[ slot ].slot, vec2.x, vec2.y );
			break;
		}
		case PL_UNIFORM_VEC3: {
			PLVector3 vec3 = *( PLVector3 * ) value;
			glUniform3f( program->uniforms[ slot ].slot, vec3.x, vec3.y, vec3.z );
			break;
		}
		case PL_UNIFORM_VEC4: {
			PLVector4 vec4 = *( PLVector4 * ) value;
			glUniform4f( program->uniforms[ slot ].slot, vec4.x, vec4.y, vec4.z, vec4.w );
			break;
		}
		case PL_UNIFORM_MAT3: {
			PLMatrix3 mat3 = *( PLMatrix3 * ) value;
			glUniformMatrix3fv( program->uniforms[ slot ].slot, 1, transpose ? GL_TRUE : GL_FALSE, mat3.m );
			break;
		}
		case PL_UNIFORM_MAT4: {
			PLMatrix4 mat4 = *( PLMatrix4 * ) value;
			glUniformMatrix4fv( program->uniforms[ slot ].slot, 1, transpose ? GL_TRUE : GL_FALSE, mat4.m );
			break;
		}
		default:
			break;
	}

	/* this should be done by the GL layer!! */
	plSetShaderProgram( oldProgram );
#endif
}

void plSetShaderUniformValue( PLShaderProgram *program, const char *name, const void *value, bool transpose ) {
	int slot = plGetShaderUniformSlot( program, name );
	if ( slot == -1 ) {
		return;
	}

	plSetShaderUniformValueByIndex( program, slot, value, transpose );
}

void plSetShaderUniformFloat(PLShaderProgram *program, int slot, float value) {
	PLShaderProgram *prg = GetShaderProgram( program );
	if ( prg == NULL || prg->uniforms == NULL ) {
		return;
	}

	if(ValidateShaderUniformSlot(prg, slot) == -1) {
        return;
    }

    PLShaderProgram* old_program = plGetCurrentShaderProgram();
    plSetShaderProgram(prg);

#if defined(PL_SUPPORT_OPENGL) /* todo, move into layer_opengl */
    glUniform1f(prg->uniforms[slot].slot, value);
#endif

    plSetShaderProgram(old_program);
}

void plSetShaderUniformVector3(PLShaderProgram* program, int slot, PLVector3 value) {
  PLShaderProgram* prg = GetShaderProgram(program);
  if(prg == NULL || prg->uniforms == NULL) {
    return;
  }

  if(ValidateShaderUniformSlot(prg, slot) == -1) {
    return;
  }

  PLShaderProgram* old_program = plGetCurrentShaderProgram();
  plSetShaderProgram(prg);

#if defined(PL_SUPPORT_OPENGL) /* todo, move into layer_opengl */
  glUniform3f(prg->uniforms[slot].slot, value.x, value.y, value.z);
#endif

  plSetShaderProgram(old_program);
}

void plSetShaderUniformVector4(PLShaderProgram* program, int slot, PLVector4 value) {
    PLShaderProgram* prg = GetShaderProgram(program);
    if(prg == NULL || prg->uniforms == NULL) {
        return;
    }

    if(ValidateShaderUniformSlot(prg, slot) == -1) {
        return;
    }

    PLShaderProgram* old_program = plGetCurrentShaderProgram();
    plSetShaderProgram(prg);

#if defined(PL_SUPPORT_OPENGL) /* todo, move into layer_opengl */
    glUniform4f(prg->uniforms[slot].slot, value.x, value.y, value.z, value.w);
#endif

    plSetShaderProgram(old_program);
}

void plSetShaderUniformInt(PLShaderProgram *program, int slot, int value) {
    PLShaderProgram *prg = GetShaderProgram(program);
    if(prg == NULL || prg->uniforms == NULL) {
        return;
    }

    if(ValidateShaderUniformSlot(prg, slot) == -1) {
        return;
    }

    PLShaderProgram *old_program = plGetCurrentShaderProgram();
    plSetShaderProgram(prg);

#if defined(PL_SUPPORT_OPENGL)
    /* todo, move into layer_opengl */
    glUniform1i(prg->uniforms[slot].slot, value);
#endif

    plSetShaderProgram(old_program);
}

void plSetShaderUniformMatrix4(PLShaderProgram *program, int slot, PLMatrix4 value, bool transpose){
    PLShaderProgram *prg = GetShaderProgram(program);
    if(prg == NULL || prg->uniforms == NULL) {
        return;
    }

    if(ValidateShaderUniformSlot(prg, slot) == -1) {
        return;
    }

    CallGfxFunction(SetShaderUniformMatrix4, prg, slot, value, transpose);
}
