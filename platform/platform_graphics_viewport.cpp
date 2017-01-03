
#include "platform_graphics.h"
#include "platform_filesystem.h"

using namespace pl::graphics;

Viewport::Viewport(int x, int y, unsigned int width, unsigned int height)
        : x_(x), y_(y), width_(width), height_(height), parent_(nullptr), camera_(nullptr) {
    // Nothing to do here.
}

void Viewport::SetSize(unsigned int width, unsigned int height) {
    if(width == 0) {
        width = 1;
    }

    if(height == 0) {
        height = 1;
    }

    width_ = width;
    height_ = height;

    // todo, update camera fov
}

void Viewport::SetPosition(int x, int y) {
    x_ = x; y_ = y;

    // todo, update children
}

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


