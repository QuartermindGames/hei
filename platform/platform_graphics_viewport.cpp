
#include "platform_graphics.h"
#include "platform_filesystem.h"

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

#if 0
void Viewport::Screenshot(std::string path) {
    unsigned char *buffer = new unsigned char(height_ * width_ * 3);
    glReadPixels(x_, y_, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, buffer);

    // todo, write with PLImage

    delete buffer;
}

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


