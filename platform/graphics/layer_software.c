#if 0

// clear buffer
if(buffers & PL_BUFFER_COLOUR) {
        for (unsigned int i = 0; i < pl_sw_backbuffer_size; i += 4) {
            pl_sw_backbuffer[i] = gfx_state.current_clearcolour.r;
            pl_sw_backbuffer[i + 1] = gfx_state.current_clearcolour.g;
            pl_sw_backbuffer[i + 2] = gfx_state.current_clearcolour.b;
            pl_sw_backbuffer[i + 3] = gfx_state.current_clearcolour.a;
        }
    }

#endif