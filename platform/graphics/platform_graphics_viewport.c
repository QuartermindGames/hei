
#include "platform_graphics.h"
#include "platform_filesystem.h"

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


