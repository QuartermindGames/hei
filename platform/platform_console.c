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
#include <PL/platform_graphics_font.h>
#include <PL/platform_filesystem.h>

#include "platform_private.h"
#include "graphics/graphics_private.h"

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

void plRegisterConsoleCommand(const char *name, void(*CallbackFunction)(unsigned int argc, char *argv[]),
                              const char *description) {
    if(name == NULL || name[0] == '\0') {
        ReportError(PL_RESULT_COMMAND_NAME, plGetResultString(PL_RESULT_COMMAND_NAME));
        return;
    }

    if(CallbackFunction == NULL) {
        ReportError(PL_RESULT_COMMAND_FUNCTION, plGetResultString(PL_RESULT_COMMAND_FUNCTION));
        return;
    }

    // Deal with resizing the array dynamically...
    if((1 + _pl_num_commands) > _pl_commands_size) {
        PLConsoleCommand **old_mem = _pl_commands;
        _pl_commands = (PLConsoleCommand**)realloc(_pl_commands, (_pl_commands_size += 128) * sizeof(PLConsoleCommand));
        if(!_pl_commands) {
            ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate %d bytes!\n",
                           _pl_commands_size * sizeof(PLConsoleCommand));
            _pl_commands = old_mem;
            _pl_commands_size -= 128;
            return;
        }
    }

    if(_pl_num_commands < _pl_commands_size) {
        _pl_commands[_pl_num_commands] = (PLConsoleCommand*)malloc(sizeof(PLConsoleCommand));
        if(!_pl_commands[_pl_num_commands]) {
            ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for ConsoleCommand, %d!\n",
                        sizeof(PLConsoleCommand));
            return;
        }

        PLConsoleCommand *cmd = _pl_commands[_pl_num_commands];
        memset(cmd, 0, sizeof(PLConsoleCommand));
        cmd->Callback = CallbackFunction;
        strncpy(cmd->cmd, name, sizeof(cmd->cmd));
        if(description != NULL && description[0] != '\0') {
            strncpy(cmd->description, description, sizeof(cmd->description));
        }

        _pl_num_commands++;
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

PLConsoleVariable *plRegisterConsoleVariable(const char *name, const char *def, PLVariableType type,
                                             void(*CallbackFunction)(const PLConsoleVariable *variable),
                                             const char *desc) {
    plAssert(_pl_variables);

    if(name == NULL || name[0] == '\0') {
        return NULL;
    }

    // Deal with resizing the array dynamically...
    if((1 + _pl_num_variables) > _pl_variables_size) {
        PLConsoleVariable **old_mem = _pl_variables;
        _pl_variables = (PLConsoleVariable**)realloc(_pl_variables, (_pl_variables_size += 128) * sizeof(PLConsoleVariable));
        if(_pl_variables == NULL) {
            ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate %d bytes!\n",
                           _pl_variables_size * sizeof(PLConsoleVariable));
            _pl_variables = old_mem;
            _pl_variables_size -= 128;
            return NULL;
        }
    }

    PLConsoleVariable *out = NULL;
    if(_pl_num_variables < _pl_variables_size) {
        _pl_variables[_pl_num_variables] = (PLConsoleVariable*)malloc(sizeof(PLConsoleVariable));
        if(_pl_variables[_pl_num_variables] == NULL) {
            ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for ConsoleCommand, %d!\n",
                        sizeof(PLConsoleVariable));
            return NULL;
        }

        out = _pl_variables[_pl_num_variables];
        memset(out, 0, sizeof(PLConsoleVariable));
        out->var = name;
        out->description = desc;
        out->default_value = def;
        out->type = type;
        if(CallbackFunction != NULL) {
            out->CallbackFunction = CallbackFunction;
        }
        plSetConsoleVariable(out, out->default_value);

        _pl_num_variables++;
    }

    return out;
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
            Print("Unknown variable type %d, failed to set!\n", var->type);
        } return;

        case pl_int_var: {
            if(pl_strisdigit(value) == -1) {
                Print("Unknown argument type %s, failed to set!\n", value);
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
                Print("Unknown argument type %s, failed to set!\n", value);
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
    if(var->CallbackFunction != NULL) {
        var->CallbackFunction(var);
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
    Print("%s\n", plGetWorkingDirectory());
}

IMPLEMENT_COMMAND(echo, "Prints out string to console.") {
    for(unsigned int i = 0; i < (argc - 1); ++i) {
        Print("%s ", argv[i]);
    }
    Print("\n");
}

IMPLEMENT_COMMAND(clear, "Clears the console buffer.") {
    memset(console_panes[active_console_pane].buffer, 0, 4096);
}

IMPLEMENT_COMMAND(colour, "Changes the colour of the current console.") {
    //console_panes[active_console_pane].
}

IMPLEMENT_COMMAND(time, "Prints out the current time.") {
    Print("%s\n", plGetFormattedTime());
}

IMPLEMENT_COMMAND(mem, "Prints out current memory usage.") {

}

IMPLEMENT_COMMAND(cmds, "Produces list of existing commands.") {
    for(PLConsoleCommand **cmd = _pl_commands; cmd < _pl_commands + _pl_num_commands; ++cmd) {
        Print(" %-20s : %-20s\n", (*cmd)->cmd, (*cmd)->description);
    }
    Print("%zu commands in total\n", _pl_num_commands);
}

IMPLEMENT_COMMAND(vars, "Produces list of existing variables.") {
    for(PLConsoleVariable **var = _pl_variables; var < _pl_variables + _pl_num_variables; ++var) {
        Print(" %-20s : %-5s / %-15s : %-20s\n",
               (*var)->var, (*var)->value, (*var)->default_value, (*var)->description);
    }
    Print("%zu variables in total\n", _pl_num_variables);
}

IMPLEMENT_COMMAND(help, "Returns information regarding specified command or variable.\nUsage: help <cmd/cvar>") {
    if(argc < 1) {
        // provide help on help, gross...
        Print("%s\n", help_var.description);
        return;
    }

    PLConsoleVariable *var = plGetConsoleVariable(argv[2]);
    if(var != NULL) {
        Print(" %-20s : %-5s / %-15s : %-20s\n",
                 var->var, var->value, var->default_value, var->description);
        return;
    }

    PLConsoleCommand *cmd = plGetConsoleCommand(argv[2]);
    if(cmd != NULL) {
        Print(" %-20s : %-20s\n", cmd->cmd, cmd->description);
        return;
    }

    Print("Unknown variable/command, %s!\n", argv[2]);
}

//////////////////////////////////////////////

PLMesh *mesh_line = NULL;
PLBitmapFont *console_font = NULL;

PLresult InitConsole(void) {
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
        return PL_RESULT_MEMORY_ALLOCATION;
    }

    if((_pl_commands = (PLConsoleCommand**)malloc(sizeof(PLConsoleCommand*) * _pl_commands_size)) == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for ConsoleCommand array, %d!\n",
                       sizeof(PLConsoleCommand) * _pl_commands_size);
        return PL_RESULT_MEMORY_ALLOCATION;
    }

    if((_pl_variables = (PLConsoleVariable**)malloc(sizeof(PLConsoleVariable*) * _pl_variables_size)) == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for ConsoleVariable array, %d!\n",
                       sizeof(PLConsoleCommand) * _pl_commands_size);
        return PL_RESULT_MEMORY_ALLOCATION;
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
    for(unsigned int i = 0; i < plArrayElements(base_commands); ++i) {
        plRegisterConsoleCommand(base_commands[i].cmd, base_commands[i].Callback, base_commands[i].description);
    }

    // todo, temporary
    //cmds_func(0, NULL);
    //vars_func(0, NULL);
    //time_func(0, NULL);

    // todo, parse config

    return PL_RESULT_SUCCESS;
}

void ShutdownConsole(void) {
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
        DebugPrint("Invalid string passed to ParseConsoleString!\n");
        return;
    }

    static char **argv = NULL;
    if(argv == NULL) {
        if((argv = (char**)malloc(sizeof(char*) * CONSOLE_MAX_ARGUMENTS)) == NULL) {
            ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));
            return;
        }
        for(char **arg = argv; arg < argv + CONSOLE_MAX_ARGUMENTS; ++arg) {
            (*arg) = (char*)malloc(sizeof(char) * 1024);
            if((*arg) == NULL) {
                ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));
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
            Print("    %s\n", var->var);
            Print("    %s\n", var->description);
            Print("    %-10s : %s\n", var->value, var->default_value);
        }
    } else if((cmd = plGetConsoleCommand(argv[0])) != NULL) {
        if(cmd->Callback != NULL) {
            cmd->Callback(argc, argv);
        } else {
            Print("    Invalid command, no callback provided!\n");
            Print("    %s\n", cmd->cmd);
            Print("    %s\n", cmd->description);
        }
    } else {
        Print("Unknown variable/command, %s!\n", argv[0]);
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

void plConsoleInput(int m_x, int m_y, unsigned int m_buttons, bool is_pressed) {
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

#if 0
#if defined(PL_MODE_OPENGL)
    glDisable(GL_TEXTURE_RECTANGLE);
#endif
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/*	Log System	*/

#define MAX_LOG_LEVELS  128

typedef struct LogLevel {
    int id;
    bool is_enabled;

    char prefix[64];    // e.g. 'warning, 'error'
    PLColour colour;
} LogLevel;

LogLevel levels[MAX_LOG_LEVELS];
LogLevel *GetLogLevel(int level) {
    static bool mem_cleared = false;
    if(!mem_cleared) {
        memset(levels, 0, sizeof(LogLevel) * MAX_LOG_LEVELS);
        mem_cleared = true;

        // todo, eventually some of these should be disabled by default - unless enabled via console command
        plSetupLogLevel(LOG_LEVEL_LOW, "pl", (PLColour){255, 255, 255}, true);
        plSetupLogLevel(LOG_LEVEL_MEDIUM, "pl-warn", (PLColour){255, 255, 0}, true);
        plSetupLogLevel(LOG_LEVEL_HIGH, "pl-error", (PLColour){255, 0, 0}, true);
        plSetupLogLevel(LOG_LEVEL_GRAPHICS, "pl-gfx", (PLColour){0, 255, 255}, true);
        plSetupLogLevel(LOG_LEVEL_FILESYSTEM, "pl-fs", (PLColour){0, 255, 255}, true);
        plSetupLogLevel(LOG_LEVEL_MODEL, "pl-model", (PLColour){0, 255, 255}, true);
    }

    // the following ensures there's no conflict
    // between internal/external log levels
    if(level < 0) {
        level *= -1;
    } else {
        level += LOG_LEVEL_END;
    }

    if(level >= MAX_LOG_LEVELS) {
        ReportError(PL_RESULT_MEMORY_EOA, "failed to find slot for log level %d", level);
        return NULL;
    }

    LogLevel *l = &levels[level];
    if(l->id == 0) {
        // register it as a new level
        l->id = level;
        l->is_enabled = false;
        l->colour = PLColour(255, 255, 255, 255);
    }

    return l;
}

/////////////////////////////////////////////////////////////////////////////////////
// public

char log_output_path[PL_SYSTEM_MAX_PATH] = {'\0'};

void plSetupLogOutput(const char *path) {
    if(path == NULL || path[0] == '\0') {
        return;
    }

    strncpy(log_output_path, path, sizeof(log_output_path));
    if(plFileExists(log_output_path)) {
        unlink(log_output_path);
    }
}

void plSetupLogLevel(int level, const char *prefix, PLColour colour, bool status) {
    LogLevel *l = GetLogLevel(level);
    if(l == NULL) {
        return;
    }

    if(prefix != NULL && prefix[0] != '\0') {
        snprintf(l->prefix, sizeof(l->prefix), "%s", prefix);
    }

    l->colour = colour;
    l->is_enabled = status;
}

void plSetLogLevelStatus(int level, bool status) {
    LogLevel *l = GetLogLevel(level);
    if(l == NULL) {
        return;
    }

    l->is_enabled = status;
}

void plLogMessage(int level, const char *msg, ...) {
    LogLevel *l = GetLogLevel(level);
    if(l == NULL) {
        return;
    }

    if(!l->is_enabled) {
        return;
    }

    char buf[4096] = {'\0'};

    // add the prefix to the start
    int c = 0;
    if(l->prefix[0] != '\0') {
        c = snprintf(buf, sizeof(buf), "[%s] %s: ", plGetFormattedTime(), l->prefix);
    } else {
        c = snprintf(buf, sizeof(buf), "[%s]: ", plGetFormattedTime());
    }

    va_list args;
    va_start(args, msg);
    vsnprintf(buf + c, sizeof(buf) - c, msg, args);
    va_end(args);

    printf("%s", buf);

    // todo, decide how we're going to pass it to the console/log

    static bool avoid_recursion = false;
    if(!avoid_recursion) {
        if (log_output_path[0] != '\0') {
            size_t size = strlen(buf);
            FILE *file = fopen(log_output_path, "a");
            if (file != NULL) {
                if (fwrite(buf, sizeof(char), size, file) != size) {
                    avoid_recursion = true;
                    ReportError(PL_RESULT_FILEERR, "failed to write to log, %s\n%s", log_output_path, strerror(errno));
                }
                fclose(file);
                return;
            }

            // todo, needs to be more appropriate; return details on exact issue
            avoid_recursion = true;
            ReportError(PL_RESULT_FILEREAD, "failed to open %s", log_output_path);
        }
    }
}