/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "plg_private.h"

PLGLight * plCreateLight( PLGLightType type, PLColour colour) {
	PLGLight * light = ( PLGLight *)pl_malloc(sizeof( PLGLight ));
    if(light == NULL) {
        return NULL;
    }

    memset(light, 0, sizeof( PLGLight ));

    gfx_state.num_lights++;
    light->colour   = colour;
    light->type     = type;
    return light;
}

void plDestroyLight( PLGLight *light) {
    if(light == NULL) {
        return;
    }

    gfx_state.num_lights--;
    pl_free(light);
}

void plDrawLight( PLGLight * light) {
    if(light->colour.a <= 0) {
        return;
    }
}
