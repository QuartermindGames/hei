
#include <PL/platform.h>

#include <SDL2/SDL.h>

/* Input Wrapper */

#define INPUT_MAX_CONTROLLERS   3       // todo, dumb limit...
#define INPUT_MAX_VIBRATION     65535   // todo, make configurable?
#define INPUT_MAX_ZONE          32767   // todo, make configurable
#define INPUT_MIN_ZONE          3000    // todo, make configurable

struct {
    bool controller_support;

    struct {
#if defined(PL_USE_SDL2)
        SDL_Joystick *joystick;
        SDL_GameController *pad;
#endif

        bool is_active;
        float left_magnitude, right_magnitude;
    } controller[INPUT_MAX_CONTROLLERS];
} _pl_input;

void _plInitInput(void) {
    memset(&_pl_input, 0, sizeof(_pl_input));

#if defined(PL_USE_SDL2)
    if(SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0) {
        _pl_input.controller_support = true;

        SDL_JoystickEventState(true);
        SDL_GameControllerEventState(true);

        for(unsigned int i = 0; i < SDL_NumJoysticks(); ++i) {
            if(SDL_IsGameController(i)) {
                if((_pl_input.controller[i].pad = SDL_GameControllerOpen(i))) {
                    _pl_input.controller[i].is_active = true;
                }
            } else {
                if((_pl_input.controller[i].joystick = SDL_JoystickOpen(i))) {
                    _pl_input.controller[i].is_active = true;
                }
            }
        }
    } else {
        DPRINT("Failed to initialise controller support!\n%s", SDL_GetError());
    }
#endif
}

void _plShutdownInput(void) {
#if defined(PL_USE_SDL2)
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
#endif
}

void _plInputFrame(void) {
#if defined(PL_USE_SDL2)
#endif
}

// Keyboard

// Mouse



