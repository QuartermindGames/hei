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

#include "graphics/graphics_private.h"

#include <PL/platform_console.h>
#include <PL/platform_graphics_font.h>
#include <PL/platform_filesystem.h>

#define CONSOLE_MAX_ARGUMENTS 8

/* Multi Console Manager */
// todo, should the console be case-sensitive?
// todo, mouse input callback
// todo, keyboard input callback

PLConsoleCommand **_pl_commands = NULL;
size_t _pl_num_commands = 0;
size_t _pl_commands_size = 512;

#define IMPLEMENT_COMMAND(NAME, DESC) \
    void NAME ## _func(unsigned int argc, char *argv[]); \
    PLConsoleCommand NAME ## _var = {#NAME, NAME ## _func, DESC}; \
    void NAME ## _func(unsigned int argc, char *argv[])

void plRegisterConsoleCommands(PLConsoleCommand cmds[], unsigned int num_cmds) {

    // Deal with resizing the array dynamically...
    if((num_cmds + _pl_num_commands) > _pl_commands_size) {
        PLConsoleCommand **old_mem = _pl_commands;
        _pl_commands = (PLConsoleCommand**)realloc(_pl_commands, (_pl_commands_size += 128) * sizeof(PLConsoleCommand));
        if(!_pl_commands) {
            ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate %d bytes!\n",
                           _pl_commands_size * sizeof(PLConsoleCommand));
            _pl_commands = old_mem;
            _pl_commands_size -= 128;
            return;
        }
    }

    for(unsigned int i = 0; i < num_cmds; i++) {
        if((cmds[i].cmd[0] == '\0') || !(cmds[i].Callback)) {
            continue;
        }

        if(_pl_num_commands < _pl_commands_size) {
            _pl_commands[_pl_num_commands] = (PLConsoleCommand*)malloc(sizeof(PLConsoleCommand));
            if(!_pl_commands[_pl_num_commands]) {
                ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for ConsoleCommand, %d!\n",
                               sizeof(PLConsoleCommand));
                break;
            }
            memcpy(_pl_commands[_pl_num_commands], &cmds[i], sizeof(PLConsoleCommand));
            _pl_num_commands++;
        }
    }
}

void plGetConsoleCommands(PLConsoleCommand *** const cmds, size_t * const num_cmds) {
    *cmds = _pl_commands;
    *num_cmds = _pl_num_commands;
}

PLConsoleCommand *plGetConsoleCommand(const char *name) {
    for(PLConsoleCommand **cmd = _pl_commands; cmd < _pl_commands + _pl_num_commands; ++cmd) {
        if(pl_strcasecmp(name, (*cmd)->cmd) == 0) {
            return (*cmd);
        }
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////

// todo, console variable implementation goes here!

PLConsoleVariable **_pl_variables = NULL;
size_t _pl_num_variables = 0;
size_t _pl_variables_size = 512;

void plRegisterConsoleVariables(PLConsoleVariable vars[], unsigned int num_vars) {
    plAssert(_pl_variables);

    // Deal with resizing the array dynamically...
    if((num_vars + _pl_num_variables) > _pl_variables_size) {
        PLConsoleVariable **old_mem = _pl_variables;
        _pl_variables = (PLConsoleVariable**)realloc(_pl_variables, (_pl_variables_size += 128) * sizeof(PLConsoleVariable));
        if(!_pl_variables) {
            ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate %d bytes!\n",
                           _pl_variables_size * sizeof(PLConsoleVariable));
            _pl_variables = old_mem;
            _pl_variables_size -= 128;
            return;
        }
    }

    for(unsigned int i = 0; i < num_vars; i++) {
        if(vars[i].var[0] == '\0') {
            continue;
        }

        if(_pl_num_variables < _pl_variables_size) {
            _pl_variables[_pl_num_variables] = (PLConsoleVariable*)malloc(sizeof(PLConsoleVariable));
            if(!_pl_variables[_pl_num_variables]) {
                ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for ConsoleCommand, %d!\n",
                               sizeof(PLConsoleVariable));
                break;
            }
            memcpy(_pl_variables[_pl_num_variables], &vars[i], sizeof(PLConsoleVariable));
            plSetConsoleVariable(_pl_variables[_pl_num_variables], _pl_variables[_pl_num_variables]->default_value);
            _pl_num_variables++;
        }
    }
}

void plGetConsoleVariables(PLConsoleVariable *** const vars, size_t * const num_vars) {
    *vars = _pl_variables;
    *num_vars = _pl_num_variables;
}

PLConsoleVariable *plGetConsoleVariable(const char *name) {
    for(PLConsoleVariable **var = _pl_variables; var < _pl_variables + _pl_num_variables; ++var) {
        if(pl_strcasecmp(name, (*var)->var) == 0) {
            return (*var);
        }
    }
    return NULL;
}

// Set console variable, with sanity checks...
void plSetConsoleVariable(PLConsoleVariable *var, const char *value) {
    plAssert(var);
    switch(var->type) {
        default: {
            _plPrint("Unknown variable type %d, failed to set!\n", var->type);
        } return;

        case pl_int_var: {
            if(pl_strisdigit(value) == -1) {
                _plPrint("Unknown argument type %s, failed to set!\n", value);
                return;
            }

            var->i_value = (int)strtol(value, NULL, 8);
        } break;

        case pl_string_var: {
            var->s_value = &var->value[0];
        } break;

        case pl_float_var: {
            var->f_value = strtof(value, NULL);
        } break;

        case pl_bool_var: {
            if(pl_strisalnum(value) == -1) {
                _plPrint("Unknown argument type %s, failed to set!\n", value);
                return;
            }

            if(strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
                var->b_value = true;
            } else {
                var->b_value = false;
            }
        } break;
    }

    strncpy(var->value, value, sizeof(var->value));

    if(var->Callback != NULL) {
        var->Callback();
    }
}

/////////////////////////////////////////////////////////////////////////////////////

#define CONSOLE_MAX_INSTANCES 4

#define CONSOLE_DEFAULT_COLOUR 128, 0, 0, 128

typedef struct PLConsolePane {
    PLRectangle2D display;
    PLColour colour;

    char buffer[4096];
    unsigned int buffer_pos;
} PLConsolePane;

PLConsolePane console_panes[CONSOLE_MAX_INSTANCES];
unsigned int num_console_panes;
unsigned int active_console_pane = 0;

bool console_visible = false;

/////////////////////////////////////////////////////////////////////////////////////
// PRIVATE

IMPLEMENT_COMMAND(pwd, "Print current working directory.") {
    _plPrint("%s\n", plGetWorkingDirectory());
}

IMPLEMENT_COMMAND(echo, "Prints out string to console.") {
    for(unsigned int i = 0; i < (argc - 1); ++i) {
        _plPrint("%s ", argv[i]);
    }
    _plPrint("\n");
}

IMPLEMENT_COMMAND(clear, "Clears the console buffer.") {
    memset(console_panes[active_console_pane].buffer, 0, 4096);
}

IMPLEMENT_COMMAND(colour, "Changes the colour of the current console.") {
    //console_panes[active_console_pane].
}

IMPLEMENT_COMMAND(time, "Prints out the current time.") {
    _plPrint("%s\n", plGetFormattedTime());
}

IMPLEMENT_COMMAND(mem, "Prints out current memory usage.") {

}

IMPLEMENT_COMMAND(cmds, "Produces list of existing commands.") {
    for(PLConsoleCommand **cmd = _pl_commands; cmd < _pl_commands + _pl_num_commands; ++cmd) {
        _plPrint(" %-20s : %-20s\n", (*cmd)->cmd, (*cmd)->description);
    }
    _plPrint("%zu commands in total\n", _pl_num_commands);
}

IMPLEMENT_COMMAND(vars, "Produces list of existing variables.") {
    for(PLConsoleVariable **var = _pl_variables; var < _pl_variables + _pl_num_variables; ++var) {
        _plPrint(" %-20s : %-5s / %-15s : %-20s\n",
               (*var)->var, (*var)->value, (*var)->default_value, (*var)->description);
    }
    _plPrint("%zu variables in total\n", _pl_num_variables);
}

IMPLEMENT_COMMAND(help, "Returns information regarding specified command or variable.\nUsage: help <cmd/cvar>") {
    if(argc < 1) {
        // provide help on help, gross...
        _plPrint("%s\n", help_var.description);
        return;
    }

    PLConsoleVariable *var = plGetConsoleVariable(argv[2]);
    if(var != NULL) {
        _plPrint(" %-20s : %-5s / %-15s : %-20s\n",
                 var->var, var->value, var->default_value, var->description);
        return;
    }

    PLConsoleCommand *cmd = plGetConsoleCommand(argv[2]);
    if(cmd != NULL) {
        _plPrint(" %-20s : %-20s\n", cmd->cmd, cmd->description);
        return;
    }

    _plPrint("Unknown variable/command, %s!\n", argv[2]);
}

//////////////////////////////////////////////

PLMesh *mesh_line = NULL;
PLBitmapFont *console_font = NULL;

PLresult _plInitConsole(void) {
    console_visible = false;

#if 0
    if((console_font = plCreateBitmapFont("fonts/console.font")) == NULL) {
        // todo, print warning...
    }
#endif

    memset(&console_panes, 0, sizeof(PLConsolePane) * CONSOLE_MAX_INSTANCES);
    active_console_pane = num_console_panes = 0;
    for(unsigned int i = 0; i < CONSOLE_MAX_INSTANCES; ++i) {
        console_panes[i].colour = plCreateColour4b(CONSOLE_DEFAULT_COLOUR);
    }

    if((mesh_line = plCreateMesh(PLMESH_LINES, PL_DRAW_IMMEDIATE, 0, 4)) == NULL) {
        return PL_RESULT_MEMORYALLOC;
    }

    if((_pl_commands = (PLConsoleCommand**)malloc(sizeof(PLConsoleCommand*) * _pl_commands_size)) == NULL) {
        ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for ConsoleCommand array, %d!\n",
                       sizeof(PLConsoleCommand) * _pl_commands_size);
        return PL_RESULT_MEMORYALLOC;
    }

    if((_pl_variables = (PLConsoleVariable**)malloc(sizeof(PLConsoleVariable*) * _pl_variables_size)) == NULL) {
        ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for ConsoleVariable array, %d!\n",
                       sizeof(PLConsoleCommand) * _pl_commands_size);
        return PL_RESULT_MEMORYALLOC;
    }

    PLConsoleCommand base_commands[]={
            clear_var,
            help_var,
            time_var,
            mem_var,
            colour_var,
            cmds_var,
            vars_var,
            pwd_var,
    };
    plRegisterConsoleCommands(base_commands, plArrayElements(base_commands));

    plAddConsoleVariable(MyVar, "true", pl_bool_var, NULL, "Example console variable, that does nothing!");
    plAddConsoleVariable(YourVar, "false", pl_bool_var, NULL, "Example console variable, that does nothing!");
    plAddConsoleVariable(HisVar, "apple", pl_string_var, NULL, "Example console variable, that does nothing!");

    // todo, temporary
    //cmds_func(0, NULL);
    //vars_func(0, NULL);
    //time_func(0, NULL);

    // todo, parse config

    return PL_RESULT_SUCCESS;
}

void _plShutdownConsole(void) {
    console_visible = false;

    plDeleteBitmapFont(console_font);

    memset(&console_panes, 0, sizeof(PLConsolePane) * CONSOLE_MAX_INSTANCES);
    active_console_pane = num_console_panes = 0;

    if(_pl_commands) {
        for(PLConsoleCommand **cmd = _pl_commands; cmd < _pl_commands + _pl_num_commands; ++cmd) {
            // todo, should we return here; assume it's the end?
            if ((*cmd) == NULL) {
                continue;
            }

            free((*cmd));
        }
        free(_pl_commands);
    }

    if(_pl_variables) {
        for(PLConsoleVariable **var = _pl_variables; var < _pl_variables + _pl_num_variables; ++var) {
            // todo, should we return here; assume it's the end?
            if ((*var) == NULL) {
                continue;
            }

            free((*var));
        }
        free(_pl_variables);
    }
}

void plSetConsoleOutCallback(void(Callback)(unsigned int )) {

}

/////////////////////////////////////////////////////

void plParseConsoleString(const char *string) {
    if(string == NULL || string[0] == '\0') {
        _plDebugPrint("Invalid string passed to ParseConsoleString!\n");
        return;
    }

    static char **argv = NULL;
    if(argv == NULL) {
        if((argv = (char**)malloc(sizeof(char*) * CONSOLE_MAX_ARGUMENTS)) == NULL) {
            ReportError(PL_RESULT_MEMORYALLOC, plGetResultString(PL_RESULT_MEMORYALLOC));
            return;
        }
        for(char **arg = argv; arg < argv + CONSOLE_MAX_ARGUMENTS; ++arg) {
            (*arg) = (char*)malloc(sizeof(char) * 1024);
            if((*arg) == NULL) {
                ReportError(PL_RESULT_MEMORYALLOC, plGetResultString(PL_RESULT_MEMORYALLOC));
                break; // continue to our doom... ?
            }
        }
    }

    unsigned int argc = 0;
    for(const char *pos = string; *pos;) {
        size_t arglen = strcspn(pos, " ");
        if(arglen > 0) {
            strncpy(argv[argc], pos, arglen);
            argv[argc][arglen] = '\0';
            ++argc;
        }
        pos += arglen;
        pos += strspn(pos, " ");
    }

    PLConsoleVariable *var;
    PLConsoleCommand *cmd;

    if((var = plGetConsoleVariable(argv[0])) != NULL) {
        // todo, should the var not be set by defacto here?

        if(argc > 1) {
            plSetConsoleVariable(var, argv[1]);
        } else {
            _plPrint("    %s\n", var->var);
            _plPrint("    %s\n", var->description);
            _plPrint("    %-10s : %s\n", var->value, var->default_value);
        }
    } else if((cmd = plGetConsoleCommand(argv[0])) != NULL) {
        if(cmd->Callback != NULL) {
            cmd->Callback(argc, argv);
        } else {
            _plPrint("    Invalid command, no callback provided!\n");
            _plPrint("    %s\n", cmd->cmd);
            _plPrint("    %s\n", cmd->description);
        }
    } else {
        _plPrint("Unknown variable/command, %s!\n", argv[0]);
    }
}

#define _MAX_ROWS       2
#define _MAX_COLUMNS    2

// todo, correctly handle rows and columns.
void ResizeConsoles(void) {
    unsigned int screen_w = gfx_state.current_viewport.w;
    unsigned int screen_h = gfx_state.current_viewport.h;
    if(screen_w == 0 || screen_h == 0) {
        screen_w = 640;
        screen_h = 480;
    }
    unsigned int width = screen_w; // / num_console_panes;
    if(num_console_panes > 1) {
        width = screen_w / 2;
    }
    unsigned int height = screen_h;
    if(num_console_panes > 2) {
        height = screen_h / 2;
    }
    unsigned int position_x = 0, position_y = 0;
    for(unsigned int i = 0; i < num_console_panes; i++) {
        if(i > 0) {
            position_x += width;
            if(position_x >= screen_w) {
                // Move onto the next row
                position_x = 0;
                position_y += height;
                if(position_y > screen_h) {
                    // Return back to the first row (this shouldn't occur...)
                    position_y = 0;
                }
            }
        }

        console_panes[i].display.wh.x = width;
        console_panes[i].display.wh.y = height;
        console_panes[i].display.xy.x = position_x;
        console_panes[i].display.xy.x = position_y;
    }
}

bool IsConsolePaneVisible(unsigned int id) {
    if(!console_visible) {
        return false;
    }

    if(
        console_panes[id].display.ll.a == 0 &&
        console_panes[id].display.lr.a == 0 &&
        console_panes[id].display.ul.a == 0 &&
        console_panes[id].display.ur.a == 0 ) {
        return false;
    }

    return true;
}

// INPUT

void _plConsoleInput(int m_x, int m_y, unsigned int m_buttons, bool is_pressed) {
    if(!is_pressed) {
        return;
    }

    for(unsigned int i = 0; i < num_console_panes; i++) {
        if(!IsConsolePaneVisible(i)) {
            continue;
        }

        PLConsolePane *pane = &console_panes[i];

        int pane_min_x = (int)pane->display.xy.x;
        int pane_max_x = pane_min_x + (int)(pane->display.wh.x);
        if(m_x < pane_min_x || m_x > pane_max_x) {
            continue;
        }

        int pane_min_y = (int)pane->display.xy.y;
        int pane_max_y = pane_min_y + (int)(pane->display.wh.y);
        if(m_y < pane_min_y || m_y > pane_max_y) {
            continue;
        }

#if 0
        if(m_buttons & PLINPUT_MOUSE_LEFT) {
            active_console_pane = i;

            pane->display.xy.x += m_x; pane->display.xy.y += m_y;
            if(pane->display.xy.x <= gfx_state.viewport_x) {
                pane->display.xy.x = gfx_state.viewport_x + 1;
            } else if(pane->display.xy.x >= gfx_state.viewport_width) {
                pane->display.xy.x = gfx_state.viewport_width - 1;
            } else if(pane->display.xy.y <= gfx_state.viewport_y) {
                pane->display.xy.y = gfx_state.viewport_y + 1;
            } else if(pane->display.xy.y >= gfx_state.viewport_height) {
                pane->display.xy.y = gfx_state.viewport_height - 1;
            }

            static int old_x = 0, old_y = 0;

            return;
        }

        if(m_buttons & PLINPUT_MOUSE_RIGHT) {
        // todo, display context menu
            return;
        }
#endif

        return;
    }

    // If we reached here, then we failed to hit anything...

#if 0
    if(m_buttons & PLINPUT_MOUSE_RIGHT) {
    // todo, display context menu
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
// PUBLIC

// GENERAL

void plSetupConsole(unsigned int num_instances) {
    if(num_instances == 0) {
        return;
    }

    if(num_instances > CONSOLE_MAX_INSTANCES) {
        num_instances = CONSOLE_MAX_INSTANCES;
    }

    memset(&console_panes, 0, sizeof(PLConsolePane) * num_instances);

    for(unsigned int i = 0; i < num_instances; i++) {
        console_panes[i].colour = plCreateColour4b(CONSOLE_DEFAULT_COLOUR);
        plSetRectangleUniformColour(&console_panes[i].display, console_panes[i].colour);
    }

    num_console_panes = num_instances;
    ResizeConsoles();
}

void plShowConsole(bool show) {
    console_visible = show;
}

void plSetConsoleColour(unsigned int id, PLColour colour) {
    plSetRectangleUniformColour(&console_panes[id-1].display, colour);
}

// todo, hook with log output?
// todo, multiple warning/error levels?
// todo, correctly adjust buffer.
void plPrintConsoleMessage(unsigned int id, const char *msg, ...) {
    if(id > CONSOLE_MAX_INSTANCES) {
        return;
    }

    char buf[2048] = { 0 };

    va_list args;
    va_start(args, msg);
    vsprintf(buf, msg, args);
    va_end(args);

    strncat(console_panes[id].buffer, buf, strlen(buf));
}

#define _COLOUR_INACTIVE_ALPHA_TOP      128
#define _COLOUR_INACTIVE_ALPHA_BOTTOM   80

#define _COLOUR_HEADER_INACTIVE         20, 0, 0, _COLOUR_INACTIVE_ALPHA_TOP

#define _COLOUR_ACTIVE_ALPHA_TOP        255
#define _COLOUR_ACTIVE_ALPHA_BOTTOM     128

#define _COLOUR_HEADER_ACTIVE_TOP       128, 0, 0, _COLOUR_ACTIVE_ALPHA_TOP
#define _COLOUR_HEADER_ACTIVE_BOTTOM    82, 0, 0, _COLOUR_ACTIVE_ALPHA_BOTTOM

void plDrawConsole(void) {
    ResizeConsoles();

    for(unsigned int i = 0; i < num_console_panes; i++) {
        if(!IsConsolePaneVisible(i)) {
            continue;
        }

        if(i == active_console_pane) {
            plDrawRectangle(console_panes[i].display);

            plDrawRectangle(plCreateRectangle(
                    PLVector2(
                            console_panes[i].display.xy.x + 4,
                            console_panes[i].display.xy.y + 4
                    ),

                    PLVector2(
                            console_panes[i].display.wh.x - 8,
                            20
                    ),

                    plCreateColour4b(_COLOUR_HEADER_ACTIVE_TOP),
                    plCreateColour4b(_COLOUR_HEADER_ACTIVE_TOP),
                    plCreateColour4b(_COLOUR_HEADER_ACTIVE_BOTTOM),
                    plCreateColour4b(_COLOUR_HEADER_ACTIVE_BOTTOM)
            ));

            // todo, console title
            // todo, display scroll bar
        } else {
            plDrawRectangle(plCreateRectangle(
                    PLVector2(
                            console_panes[i].display.xy.x,
                            console_panes[i].display.xy.y
                    ),

                    PLVector2(
                            console_panes[i].display.wh.x,
                            console_panes[i].display.wh.y
                    ),

                    PLColour(
                            (uint8_t) (console_panes[i].display.ul.r / 2),
                            (uint8_t) (console_panes[i].display.ul.g / 2),
                            (uint8_t) (console_panes[i].display.ul.b / 2),
                            _COLOUR_INACTIVE_ALPHA_TOP
                    ),

                    PLColour(
                            (uint8_t) (console_panes[i].display.ur.r / 2),
                            (uint8_t) (console_panes[i].display.ur.g / 2),
                            (uint8_t) (console_panes[i].display.ur.b / 2),
                            _COLOUR_INACTIVE_ALPHA_TOP
                    ),

                    PLColour(
                            (uint8_t) (console_panes[i].display.ll.r / 2),
                            (uint8_t) (console_panes[i].display.ll.g / 2),
                            (uint8_t) (console_panes[i].display.ll.b / 2),
                            _COLOUR_INACTIVE_ALPHA_BOTTOM
                    ),

                    PLColour(
                            (uint8_t) (console_panes[i].display.lr.r / 2),
                            (uint8_t) (console_panes[i].display.lr.g / 2),
                            (uint8_t) (console_panes[i].display.lr.b / 2),
                            _COLOUR_INACTIVE_ALPHA_BOTTOM
                    )
            ));

            plDrawRectangle(plCreateRectangle(
                    PLVector2(
                            console_panes[i].display.xy.x + 4,
                            console_panes[i].display.xy.y + 4
                    ),

                    PLVector2(
                            console_panes[i].display.wh.x - 8,
                            20
                    ),

                    plCreateColour4b(_COLOUR_HEADER_INACTIVE),
                    plCreateColour4b(_COLOUR_HEADER_INACTIVE),
                    plCreateColour4b(_COLOUR_HEADER_INACTIVE),
                    plCreateColour4b(_COLOUR_HEADER_INACTIVE)
            ));

            // todo, dim console title
        }

        // todo, display buffer size display
        // todo, display buffer text
    }

    if(console_font == NULL) {
        return;
    }

#if 0
#if defined(PL_MODE_OPENGL)
    glEnable(GL_TEXTURE_RECTANGLE);

    glBindTexture(GL_TEXTURE_RECTANGLE, console_font->texture->id);
#endif
#endif

    plDrawCharacter(console_font, 20, 20, 2, 'A');
    plDrawCharacter(console_font, 20, 30, 2, 'B');
    plDrawCharacter(console_font, 20, 40, 2, 'C');
    plDrawCharacter(console_font, 20, 50, 4, 'D');
#if 0
#if defined(PL_MODE_OPENGL)
    glDisable(GL_TEXTURE_RECTANGLE);
#endif
#endif
}
