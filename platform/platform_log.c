/*
DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
Version 2, December 2004

Copyright (C) 2011-2016 Mark E Sowden <markelswo@gmail.com>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

0. You just DO WHAT THE FUCK YOU WANT TO.
*/

#include "platform_log.h"

/*	Log System	*/

#define    LOG_FILE_EXTENSION    ".log"

void plWriteLog(const char *path, const char *msg, ...) {
    pFUNCTION_START

        char newpath[PLATFORM_MAX_PATH];
        sprintf(newpath, "%s"LOG_FILE_EXTENSION, path);

        static char buffer[1024];
        va_list args;
        va_start(args, msg);
        vsnprintf(buffer, sizeof(buffer), msg, args);
        va_end(args);

        size_t size = strlen(buffer);
        FILE *file = fopen(newpath, "a");
        if (fwrite(buffer, sizeof(char), size, file) != size)
            plSetError("Failed to write to log! (%s)", newpath);
        fclose(file);

    pFUNCTION_END
}

void plClearLog(const char *path) {
    pFUNCTION_START

        char newpath[PLATFORM_MAX_PATH];
        sprintf(newpath, "%s"LOG_FILE_EXTENSION, path);
        unlink(newpath);

    pFUNCTION_END
}
