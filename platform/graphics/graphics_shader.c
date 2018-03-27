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

PLShaderStage *plCreateShaderStage(PLShaderStageType type) {
    PLShaderStage *stage = calloc(1, sizeof(PLShaderStage));
    if(stage == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "failed to allocate shader stage");
        return NULL;
    }

    CallGfxFunction(CreateShaderStage, stage);

    return stage;
}

void plDeleteShaderStage(PLShaderStage *stage) {
    if(stage == NULL) {
        return;
    }

    CallGfxFunction(DeleteShaderStage, stage);
}

void plCompileShaderStage(PLShaderStage *stage, const char *buf, size_t length) {
    _plResetError();
    CallGfxFunction(CompileShaderStage, stage, buf, length);
}

/* does everything above all at once */
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

PLShaderProgram *plCreateShaderProgram(void) {
    PLShaderProgram *program = calloc(1, sizeof(PLShaderProgram));
    if(program == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "failed to create shader program");
        return NULL;
    }

    CallGfxFunction(CreateShaderProgram, program);

    program->max_stages = 2;
    program->stages = calloc(program->max_stages, sizeof(PLShaderStage));

    return program;
}

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

bool plAttachShaderStage(PLShaderProgram *program, PLShaderStage *stage) {
    program->num_stages++;


    for(unsigned int i = 0; i < program->num_stages; i++) {

    }
    return false;
}

PLShaderProgram *plGetCurrentShaderProgram(void) {
    return gfx_state.current_program;
}

bool plIsShaderProgramEnabled(PLShaderProgram *program) {
    if(gfx_state.current_program == program) {
        return true;
    }

    return false;
}

void plSetShaderProgram(PLShaderProgram *program) {
    if (program == gfx_state.current_program) {
        return;
    }

    CallGfxFunction(SetShaderProgram, program);

    gfx_state.current_program = program;
}
