
#pragma once

#include <PL/platform.h>

#define PLINPUT_MOUSE_LEFT      (1 << 0)
#define PLINPUT_MOUSE_RIGHT     (1 << 1)
#define PLINPUT_MOUSE_MIDDLE    (1 << 2)

typedef struct PLMouseState {
    int position[2];
    bool button_state[3]; // left, middle, right
} PLMouseState;

enum {
    PL_KEY_BS       = 8,
    PL_KEY_TAB      = 9,
    PL_KEY_CR       = 13,
    PL_KEY_ESCAPE   = 27,
    PL_KEY_SPACE    = 32,
    PL_KEY_DELETE   = 127,

    PL_KEY_F1,
    PL_KEY_F2,
    PL_KEY_F3,
    PL_KEY_F4,
    PL_KEY_F5,
    PL_KEY_F6,
    PL_KEY_F7,
    PL_KEY_F8,
    PL_KEY_F9,
    PL_KEY_F10,
    PL_KEY_F11,
    PL_KEY_F12,

    PL_KEY_PAUSE,

    PL_KEY_UP,
    PL_KEY_DOWN,
    PL_KEY_LEFT,
    PL_KEY_RIGHT,

    PL_KEY_CTRL,
    PL_KEY_ALT,
    PL_KEY_SHIFT,

    PL_KEY_INSERT,
    PL_KEY_HOME,
    PL_KEY_END,
    PL_KEY_PAGEUP,
    PL_KEY_PAGEDOWN,

    PL_KEY_PRINTSCREEN,

    PL_MOUSE_LEFT = 0,
    PL_MOUSE_MIDDLE,
    PL_MOUSE_RIGHT,
};

typedef struct PLWindow PLWindow;

PL_EXTERN_C

PL_EXTERN void plClearKeyboardState(void);
PL_EXTERN void plSetKeyboardCallback(void(*Callback)(PLWindow *window, bool state, char key));

PL_EXTERN bool plGetKeyState(char key);
PL_EXTERN bool plGetMouseState(char button);

PL_EXTERN void plSetMouseCallback(void(*Callback)(PLWindow *window, bool state, char button, int x, int y));
PL_EXTERN void plGetCursorPosition(PLWindow *window, int *x, int *y);

PL_EXTERN void plProcessInput(void);

PL_EXTERN_C_END