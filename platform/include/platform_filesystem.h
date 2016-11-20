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

#pragma once

#include "platform.h"

PL_EXTERN_C

extern void plGetUserName(PLchar *out);

extern void plGetWorkingDirectory(PLchar *out);

extern void plStripExtension(PLchar *dest, const PLchar *in);

extern PLchar *plGetFileExtension(PLchar *dest, const PLchar *in);

extern const PLchar *plGetFileName(const PLchar *path);

extern void plScanDirectory(const PLchar *path, const PLchar *extension, void(*Function)(PLchar *filepath));

extern void plLowerCasePath(PLchar *out);

extern PLbool plCreateDirectory(const PLchar *path);

// File I/O ...

extern PLbool plFileExists(const PLchar *path);

extern PLbool plIsFileModified(time_t oldtime, const PLchar *path);

extern time_t plGetFileModifiedTime(const PLchar *path);

extern PLint plGetLittleShort(FILE *fin);

extern PLint plGetLittleLong(FILE *fin);

PL_EXTERN_C_END
