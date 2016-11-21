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
    char out[MAX_FUNCTION_LENGTH];
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
