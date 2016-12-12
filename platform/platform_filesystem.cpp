/*
Copyright (C) 2011-2016 OldTimes Software

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "platform_filesystem.h"

/*	File System	*/

// Checks whether a file has been modified or not.
PLbool plIsFileModified(time_t oldtime, const PLchar *path) {
    plFunctionStart();
    if (!oldtime) {
        plSetError("Invalid time, skipping check!\n");
        return PL_FALSE;
    }

    struct stat sAttributes;
    if (stat(path, &sAttributes) == -1) {
        plSetError("Failed to get file stats!\n");
        return PL_FALSE;
    }

    if (sAttributes.st_mtime > oldtime)
        return PL_TRUE;

    return PL_FALSE;
    plFunctionEnd();
}

time_t plGetFileModifiedTime(const PLchar *path) {
    plFunctionStart();
    struct stat sAttributes;
    if (stat(path, &sAttributes) == -1) {
        plSetError("Failed to get modification time!\n");
        return 0;
    }
    return sAttributes.st_mtime;
    plFunctionEnd();
}

void plLowerCasePath(PLchar *out) {
    plFunctionStart();
    for (int i = 0; out[i]; i++)
        out[i] = (PLchar) tolower(out[i]);
    plFunctionEnd();
}

// Creates a folder at the given path.
PLbool plCreateDirectory(const PLchar *ccPath) {
    plFunctionStart();
#ifdef _WIN32
    if(CreateDirectory(ccPath, NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
        return PL_TRUE;
    else if(GetLastError() == ERROR_PATH_NOT_FOUND)
        plSetError("Failed to find an intermediate directory! (%s)\n", ccPath);
    else    // Assume it already exists.
        plSetError("Unknown error! (%s)\n", ccPath);
#else
    {
        struct stat ssBuffer;

        if (stat(ccPath, &ssBuffer) == -1) {
            if (mkdir(ccPath, 0777) == 0)
                return PL_TRUE;
            else {
                switch (errno) {
                    case EACCES:
                        plSetError("Failed to get permission! (%s)\n", ccPath);
                    case EROFS:
                        plSetError("File system is read only! (%s)\n", ccPath);
                    case ENAMETOOLONG:
                        plSetError("Path is too long! (%s)\n", ccPath);
                    default:
                        plSetError("Failed to create directory! (%s)\n", ccPath);
                }
            }
        } else
            // Path already exists, so this is fine.
            return PL_TRUE;
    }
#endif

    return PL_FALSE;
    plFunctionEnd();
}

// Returns the extension for the file.
const PLchar *plGetFileExtension(const PLchar *in) {
    plFunctionStart();
    if (!plIsValidString(in)) {
        return "";
    }

    const PLchar *s = strrchr(in, '.');
    if(!s || s == in) {
        return "";
    }

    return s + 1;
    plFunctionEnd();
}

// Strips the extension from the filename.
void plStripExtension(PLchar *dest, const PLchar *in) {
    if (!plIsValidString(in)) {
        *dest = 0;
        return;
    }

    const PLchar *s = strrchr(in, '.');
    while (in < s) *dest++ = *in++;
    *dest = 0;
}

// Returns a pointer to the last component in the given filename. 
const PLchar *plGetFileName(const PLchar *path) {
    const PLchar *lslash = strrchr(path, '/');
    if (lslash != NULL) path = lslash + 1;
    return path;
}

/*	Returns the name of the systems	current user.
	TODO:
		Move this into platform_system
*/
void plGetUserName(PLchar *out) {
    plFunctionStart();
#ifdef _WIN32
    PLchar userstring[PL_MAX_USERNAME];

    // Set these AFTER we update active function.
    DWORD name = sizeof(userstring);
    if (GetUserName(userstring, &name) == 0)
        // If it fails, just set it to user.
        sprintf(userstring, "user");
#else   // Linux
    PLchar *userstring = getenv("LOGNAME");
    if (userstring == NULL)
        // If it fails, just set it to user.
        userstring = "user";
#endif
    {
        int i = 0,
                userlength = (int) strlen(userstring);
        while (i < userlength) {
            if (userstring[i] == ' ')
                out[i] = '_';
            else
                out[i] = (char) tolower(userstring[i]);
            i++;
        }
    }

    //strncpy(out, cUser, sizeof(out));
    plFunctionEnd();
}

/*	Scans the given directory.
	On each found file it calls the given function to handle the file.
*/
void plScanDirectory(const PLchar *path, const PLchar *extension, void(*Function)(PLchar *filepath)) {
    plFunctionStart();
    if (path[0] == ' ') {
        plSetError("Invalid path!\n");
        return;
    }

    char filestring[PL_MAX_PATH];
#ifdef _WIN32
    {
        WIN32_FIND_DATA	finddata;
        HANDLE			find;

        sprintf(filestring, "%s/*%s", path, extension);

        find = FindFirstFile(filestring, &finddata);
        if (find == INVALID_HANDLE_VALUE)
        {
            plSetError("Failed to find an initial file!\n");
            return;
        }

        do
        {
            // Pass the entire dir + filename.
            sprintf(filestring, "%s/%s", path, finddata.cFileName);
            Function(filestring);
        } while(FindNextFile(find, &finddata));
    }
#else
    {
        DIR *dDirectory;
        struct dirent *dEntry;

        dDirectory = opendir(path);
        if (dDirectory) {
            while ((dEntry = readdir(dDirectory))) {
                if (strstr(dEntry->d_name, extension)) {
                    sprintf(filestring, "%s/%s", path, dEntry->d_name);
                    Function(filestring);
                }
            }

            closedir(dDirectory);
        }
    }
#endif
    plFunctionEnd();
}

void plGetWorkingDirectory(PLchar *out) {
    plFunctionStart();
    if (!getcwd(out, PL_MAX_PATH)) {
        switch (errno) {
            default:
                break;

            case EACCES:
                plSetError("Permission to read or search a component of the filename was denied!\n");
                break;
            case EFAULT:
                plSetError("buf points to a bad address!\n");
                break;
            case EINVAL:
                plSetError("The size argument is zero and buf is not a null pointer!\n");
                break;
            case ENOMEM:
                plSetError("Out of memory!\n");
                break;
            case ENOENT:
                plSetError("The current working directory has been unlinked!\n");
                break;
            case ERANGE:
                plSetError("The size argument is less than the length of the absolute pathname of the working directory, including the terminating null byte. \
						You need to allocate a bigger array and try again!\n");
                break;
        }
        return;
    }
    strcat(out, "\\");
    plFunctionEnd();
}

/*	File I/O	*/

// Checks if a file exists or not.
PLbool plFileExists(const PLchar *path) {
    struct stat buffer;
    return (PLbool) (stat(path, &buffer) == 0);
}

PLint plGetLittleShort(FILE *fin) {
    PLint b1 = fgetc(fin);
    PLint b2 = fgetc(fin);
    return (PLshort) (b1 + b2 * 256);
}

PLint plGetLittleLong(FILE *fin) {
    PLint b1 = fgetc(fin);
    PLint b2 = fgetc(fin);
    PLint b3 = fgetc(fin);
    PLint b4 = fgetc(fin);
    return b1 + (b2 << 8) + (b3 << 16) + (b4 << 24);
}
