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

#include "platform.h"

#include "platform_system.h"

/*	Generic functions for platform, such as	error handling.	*/

/*	ERROR HANDLING	*/

#define    MAX_FUNCTION_LENGTH    64
#define    MAX_ERROR_LENGTH    2048

char
        sys_error[MAX_ERROR_LENGTH],
        loc_error[MAX_ERROR_LENGTH],
        loc_function[MAX_FUNCTION_LENGTH];

/*	Sets the name of the currently entered function.
*/
void plSetErrorFunction(const char *function, ...) {
    char out[2048]; // todo, shitty work around because linux crap    //[MAX_FUNCTION_LENGTH];
    va_list args;

    va_start(args, function);
    vsprintf(out, function, args);
    va_end(args);

    strncpy(loc_function, out, sizeof(function));
}

void plResetError(void) {
    // Set everything to "null".
    sprintf(loc_error, "null");
    sprintf(sys_error, "null");
    sprintf(loc_function, "null");
}

/*	Sets the local error message.
*/
void plSetError(const char *msg, ...) {
    char out[MAX_ERROR_LENGTH];
    va_list args;

    va_start(args, msg);
    vsprintf(out, msg, args);
    va_end(args);

    strncpy(loc_error, out, sizeof(loc_error));
}

/*	Returns the locally generated error message.
*/
char *plGetError(void) {
    return loc_error;
}

/*	Returns a system error message.
*/
char *plGetSystemError(void) {
#ifdef _WIN32
    char	*buffer = NULL;
    int		error;

    error = GetLastError();
    if (error == 0)
        return "Unknown system error!";

    if (!FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buffer,
        0, NULL))
        return "Failed to get system error details!";

    strcpy(sys_error, _strdup(buffer));

    LocalFree(buffer);

    return sys_error;
#else
    strcpy(sys_error, dlerror());
    return sys_error;
#endif
}

const PLchar *plGetResultString(PLresult result) {
    switch (result) {
        case PL_RESULT_SUCCESS: return "Success";

            // FILE I/O
        case PL_RESULT_FILEREAD:    return "Failed to read complete file!";
        case PL_RESULT_FILESIZE:    return "Failed to get valid file size!";
        case PL_RESULT_FILETYPE:    return "Invalid file type!";
        case PL_RESULT_FILEVERSION: return "Unsupported file version!";
        case PL_RESULT_FILEPATH:    return "Invalid file path!";

            // IMAGE
        case PL_RESULT_IMAGERESOLUTION: return "Invalid image resolution!";
        case PL_RESULT_IMAGEFORMAT:     return "Invalid image format!";

            // MEMORY
        case PL_RESULT_MEMORYALLOC: return "Failed to allocate memory!";

        default:    return "An unknown error occurred!";
    }
}
