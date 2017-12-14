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
#ifndef _WIN32
#   include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <PL/platform_filesystem.h>

/*	File System	*/

PLresult _plInitIO(void) {
    return PL_RESULT_SUCCESS;
}

void _plShutdownIO(void) {

}

// Checks whether a file has been modified or not.
bool plIsFileModified(time_t oldtime, const char *path) {
    if (!oldtime) {
        ReportError(PL_RESULT_FILEERR, "invalid time, skipping check");
        return false;
    }

    struct stat attributes;
    if (stat(path, &attributes) == -1) {
        ReportError(PL_RESULT_FILEERR, "failed to stat %s: %s", path, strerror(errno));
        return false;
    }

    if (attributes.st_mtime > oldtime) {
        return true;
    }

    return false;
}

time_t plGetFileModifiedTime(const PLchar *path) {
    struct stat attributes;
    if (stat(path, &attributes) == -1) {
        ReportError(PL_RESULT_FILEERR, "Failed to stat %s: %s", path, strerror(errno));
        return 0;
    }
    return attributes.st_mtime;
}

// Creates a folder at the given path.
bool plCreateDirectory(const char *path) {
    if(plPathExists(path)) {
        return true;
    }

    if (mkdir(path, 0777) == 0) {
        return true;
    }

    ReportError(PL_RESULT_FILEERR, "%s", strerror(errno));

    return false;
}

bool plCreatePath(const char *path) {
    size_t length = strlen(path);
    char dir_path[length + 1];
    memset(dir_path, 0, sizeof(dir_path));
    for(size_t i = 0; i < length; ++i) {
        dir_path[i] = path[i];
        if(i != 0 &&
            (path[i] == '\\' || path[i] == '/') &&
            (path[i - 1] != '\\' && path[i - 1] != '/')) {
            if(!plCreateDirectory(dir_path)) {
                return false;
            }
        }
    }

    return plCreateDirectory(dir_path);
}

// Returns the extension for the file.
const char *plGetFileExtension(const char *in) {
    if (!plIsValidString(in)) {
        return "";
    }

    const char *s = strrchr(in, '.');
    if(!s || s == in) {
        return "";
    }

    return s + 1;
}

// Strips the extension from the filename.
void plStripExtension(char *dest, const char *in) {
    if (!plIsValidString(in)) {
        *dest = 0;
        return;
    }

    const char *s = strrchr(in, '.');
    while (in < s) *dest++ = *in++;
    *dest = 0;
}

// Returns a pointer to the last component in the given filename. 
const char *plGetFileName(const char *path) {
    const char *lslash = strrchr(path, '/');
    if (lslash != NULL) {
        path = lslash + 1;
    }
    return path;
}

// Returns the name of the systems current user.
void plGetUserName(char *out) {
#ifdef _WIN32
    PLchar userstring[PL_MAX_USERNAME];

    // Set these AFTER we update active function.
    DWORD name = sizeof(userstring);
    if (GetUserName(userstring, &name) == 0)
        // If it fails, just set it to user.
        sprintf(userstring, "user");
#else   // Linux
    char *userstring = getenv("LOGNAME");
    if (userstring == NULL) {
        // If it fails, just set it to user.
        userstring = "user";
    }
#endif
    int i = 0, userlength = (int) strlen(userstring);
    while (i < userlength) {
        if (userstring[i] == ' ') {
            out[i] = '_';
        } else {
            out[i] = (char) tolower(userstring[i]);
        } i++;
    }

    //strncpy(out, cUser, sizeof(out));
}

/*	Scans the given directory.
	On each found file it calls the given function to handle the file.
*/
void plScanDirectory(const char *path, const char *extension, void (*Function)(const char *), bool recursive) {
    char filestring[PL_SYSTEM_MAX_PATH + 1];
#ifdef _WIN32
        WIN32_FIND_DATA	finddata;
        HANDLE			find;

        snprintf(filestring, sizeof(filestring), "%s/*", path);

        find = FindFirstFile(filestring, &finddata);
        if (find == INVALID_HANDLE_VALUE) {
            plSetError("Failed to find an initial file!\n");
            return;
        }

        do {
            if(strcmp(finddata.cFileName, ".") == 0 || strcmp(finddata.cFileName, "..") == 0) {
                continue;
            }

            snprintf(filestring, sizeof(filestring), "%s/%s", path, finddata.cFileName);

            if(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if(recursive) {
                    plScanDirectory(filestring, extension, Function, recursive);
                }
            }
            else{
                if(pl_strcasecmp(plGetFileExtension(finddata.cFileName), extension) == 0) {
                    Function(filestring);
                }
            }
        } while(FindNextFile(find, &finddata));

        FindClose(find);
#else
    DIR *directory = opendir(path);
    if (directory) {
        struct dirent *entry;
        while ((entry = readdir(directory))) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            snprintf(filestring, sizeof(filestring), "%s/%s", path, entry->d_name);

            struct stat st;
            if(stat(filestring, &st) == 0) {
                if(S_ISREG(st.st_mode)) {
                    if(pl_strcasecmp(plGetFileExtension(entry->d_name), extension) == 0) {
                        Function(filestring);
                    }
                } else if(S_ISDIR(st.st_mode) && recursive) {
                    plScanDirectory(filestring, extension, Function, recursive);
                }
            }
        }

        closedir(directory);
    } else {
        ReportError(PL_RESULT_FILEPATH, "opendir failed!");
    }
#endif
}

const char *plGetWorkingDirectory(void) {
    static char out[PL_SYSTEM_MAX_PATH] = { '\0' };
    if (getcwd(out, PL_SYSTEM_MAX_PATH) == NULL) {
        /* The MSDN documentation for getcwd() is gone, but it proooobably uses
         * errno and friends.
         */
        ReportError(PL_RESULT_SYSERR, "%s", strerror(errno));
        return NULL;
    }
    return out;
}

void plSetWorkingDirectory(const char *path) {
    if(chdir(path) != 0) {
        ReportError(PL_RESULT_SYSERR, "%s", strerror(errno));
        /* TODO: Return error condition */
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// FILE I/O

// Checks if a file exists or not.
bool plFileExists(const char *path) {
    struct stat buffer;
    return (bool) (stat(path, &buffer) == 0);
}

bool plPathExists(const char *path) {
    DIR *dir = opendir(path);
    if(dir) {
        closedir(dir);
        return true;
    }
    return false;
}

bool plDeleteFile(const char *path) {
    if(!plFileExists(path)) {
        return true;
    }

    int result = remove(path);
    if(result == 0) {
        return true;
    }

    ReportError(PL_RESULT_FILEREAD, strerror(errno));
    return false;
}

bool plCopyFile(const char *path, const char *dest) {
    size_t file_size = plGetFileSize(path);
    if(file_size == 0) {
        return false;
    }

    uint8_t *data = calloc(file_size, 1);
    if(data == NULL) {
        ReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate buffer for %s, with size %d!", path, file_size);
        return false;
    }

    FILE *original = NULL;
    FILE *copy = NULL;

    // read in the original
    if((original = fopen(path, "rb")) == NULL) {
        ReportError(PL_RESULT_FILEREAD, "Failed to open %s!", path);
        goto BAIL;
    }
    if(fread(data, 1, file_size, original) != file_size) {
        ReportError(PL_RESULT_FILEREAD, "failed to read in %d bytes for %s", file_size, path);
        goto BAIL;
    }
    fclose(original); original = NULL;

    // write out the copy
    if((copy = fopen(dest, "wb")) == NULL) {
        ReportError(PL_RESULT_FILEWRITE, "failed to open %s for write", path);
        goto BAIL;
    }
    if(fwrite(data, 1, file_size, copy) != file_size) {
        ReportError(PL_RESULT_FILEWRITE, "failed to write out %d bytes for %s", file_size, path);
        goto BAIL;
    }
    fclose(copy);

    return true;

    BAIL:

    if(data != NULL) {
        free(data);
    }

    if(original != NULL) {
        fclose(original);
    }

    if(copy != NULL) {
        fclose(copy);
    }

    return false;
}

size_t plGetFileSize(const char *path) {
    struct stat buf;
    if(stat(path, &buf) != 0) {
        ReportError(PL_RESULT_FILEERR, "failed to stat %s: %s", path, strerror(errno));
        return 0;
    }

    return (size_t)buf.st_size;
}

///////////////////////////////////////////

int16_t plGetLittleShort(FILE *fin) {
    int b1 = fgetc(fin);
    int b2 = fgetc(fin);
    return (int16_t) (b1 + b2 * 256);
}

int32_t plGetLittleLong(FILE *fin) {
    int b1 = fgetc(fin);
    int b2 = fgetc(fin);
    int b3 = fgetc(fin);
    int b4 = fgetc(fin);
    return b1 + (b2 << 8) + (b3 << 16) + (b4 << 24);
}
