
#include "platform_graphics.h"
#include "platform_filesystem.h"

void plSetupViewport(PLViewport *viewport, PLint x, PLint y, PLuint width, PLuint height) {
    memset(viewport, 0, sizeof(PLViewport));

    plSetViewportPosition(viewport, x, y);
    plSetViewportSize(viewport, width, height);
}

void plSetViewportPosition(PLViewport *viewport, PLint x, PLint y) {
    if((viewport->x == x) && (viewport->y == y)) {
        return;
    }

    viewport->x = x;
    viewport->y = y;

    // todo, update children
}

void plSetViewportSize(PLViewport *viewport, PLuint width, PLuint height) {
    if((viewport->width == width) && (viewport->height == height)) {
        return;
    }

    if(width == 0) {
        width = 1;
    }

    if(height == 0) {
        height = 1;
    }

    viewport->width = width;
    viewport->height = height;

    // todo, update camera fov
}

void plSetCurrentViewport(PLViewport *viewport) {
    if(!viewport || (pl_graphics_state.current_viewport == viewport)) {
        return;
    }

    plViewport(viewport->x, viewport->y, viewport->width, viewport->height);
    plScissor(viewport->x, viewport->y, viewport->width, viewport->height);

    pl_graphics_state.current_viewport = viewport;
}

#if 0
#define PL_PATH_SCREENSHOTS "./screenshots" // todo, this SHOULDN'T be hardcoded like this :(

void Viewport::Screenshot() {
    if(!plCreateDirectory(PL_PATH_SCREENSHOTS)) {
        plGraphicsLog("Failed to create screenshot directory!\n");
        return;
    }

    unsigned int i = 0; std::string scrname, localname;
    do {
        scrname.clear();
        localname = PL_PATH_SCREENSHOTS;
        localname += "screen" + std::to_string(i) + ".tga";
        scrname.append(PL_PATH_SCREENSHOTS + localname);

        i++;
        // This is incredibly unlikely to happen, but better safe than sorry?
        if(i == ((unsigned int) - 1)) {
            plGraphicsLog("Failed to find an unused slot to write screenshot! (%s)\n", scrname.c_str());
            return;
        }
    } while(plFileExists(scrname.c_str()));

    Screenshot(scrname);
}
#endif

void plScreenshot(PLViewport *viewport, const PLchar *path) {
    PLbyte *buffer = (PLbyte*)calloc(viewport->height * viewport->width * 3, sizeof(PLbyte));
    glReadPixels(viewport->x, viewport->y, viewport->width, viewport->height, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    free(buffer);
}


