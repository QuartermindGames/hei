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
#include <PL/platform_console.h>

#include "filesystem_private.h"
#include "graphics_private.h"

/* shader implementation */

/**********************************************************/
/** shader stages **/

/**
 * create the shader stage and generate it on the GPU,
 * if applicable.
 *
 * @param type the type of shader stage.
 * @return the new shader stage.
 */
PLShaderStage *plCreateShaderStage(PLShaderStageType type) {
    PLShaderStage *stage = calloc(1, sizeof(PLShaderStage));
    if(stage == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "failed to allocate shader stage");
        return NULL;
    }

    CallGfxFunction(CreateShaderStage, stage);

    return stage;
}

/**
 * delete the given shader stage and wipe it off the
 * GPU, if applicable.
 *
 * @param stage stage we're deleting.
 */
void plDeleteShaderStage(PLShaderStage *stage) {
    if(stage == NULL) {
        return;
    }

    CallGfxFunction(DeleteShaderStage, stage);
}

/**
 * compiles the given shader stage on the GPU, otherwise it
 * will be ignored when performing software-rendering or if
 * your GPU doesn't support the shader compilation.
 *
 * if the compilation fails an error will be reported and this will
 * automatically fallback when rendering anything if it's active.
 *
 * @param stage the stage we're going to be compiling.
 * @param buf pointer to buffer containing the shader we're compiling.
 * @param length the length of the buffer.
 */
void plCompileShaderStage(PLShaderStage *stage, const char *buf, size_t length) {
    _plResetError();
    CallGfxFunction(CompileShaderStage, stage, buf, length);
}

/**
 * shortcut function that can be used to quickly produce a new
 * shader stage. this will automatically handle loading the given
 * shader into memory, compiling it and then returning the new
 * shader stage object.
 *
 * if the compilation fails an error will be reported and the
 * function will return a null pointer.
 *
 * @param path path to the shader stage document.
 * @param type the type of shader stage.
 * @return the new shader stage.
 */
PLShaderStage *plLoadShaderStage(const char *path, PLShaderStageType type) {
    FILE *fp = fopen(path, "r");
    if(fp == NULL) {
        ReportError(PL_RESULT_FILEREAD, "failed to open %s", path);
        return NULL;
    }

    size_t length = plGetFileSize(path);
    char buf[length];
    if(fread(buf, sizeof(char), length, fp) != length) {
        GfxLog("failed to read in entirety of %s, continuing anyway but expect issues");
    }

    fclose(fp);

    PLShaderStage *stage = plCreateShaderStage(type);
    if(stage == NULL) {
        return NULL;
    }

    plCompileShaderStage(stage, buf, length);
    if(plGetFunctionResult() == PL_RESULT_SHADER_COMPILE) {
        plDeleteShaderStage(stage);
        return NULL;
    }

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
    PLShaderProgram *program = calloc(1, sizeof(PLShaderProgram));
    if(program == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "failed to create shader program");
        return NULL;
    }

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
void plDeleteShaderProgram(PLShaderProgram *program, bool free_stages) {
    if(program == NULL) {
        return;
    }

    for(unsigned int i = 0; i < PL_NUM_SHADER_TYPES; ++i) {
        if(program->stages[i] != NULL) {
            CallGfxFunction(DetachShaderStage, program, program->stages[i]);
            if(free_stages) {
                free(program->stages[i]);
            }
        }
    }

    CallGfxFunction(DeleteShaderProgram, program);

    free(program);
}

bool plRegisterShaderStage(PLShaderProgram *program, const char *path, PLShaderStageType type) {
    if((program->num_stages + 1) > 4) {
        ReportError(PL_RESULT_MEMORY_EOA, "reached maximum number of available shader stage slots (%u)",
                    program->num_stages);
        return false;
    }

    PLShaderStage *stage = plLoadShaderStage(path, type);
    if(stage == NULL) {
        return false;
    }

    program->stages[program->num_stages++] = stage;

    CallGfxFunction(AttachShaderStage, program, stage);

    return false;
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

bool plLinkShaderProgram(PLShaderProgram *program) {
    _plResetError();
    CallGfxFunction(LinkShaderProgram, program);
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

#define IMPLEMENT_UNIFORM_FUNCTION()    bool
#define IMPLEMENT_ATTRIBUTE_FUNCTION()  bool