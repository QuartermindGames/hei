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

typedef struct PLModuleFunction {
    const char *name;

    void **Function;
} PLModuleFunction;

PL_EXTERN_C

#ifdef _WIN32
#	define PL_MODULE_EXTENSION	".dll"
#	define PL_MODULE_EXPORT		__declspec(dllexport)
#	define PL_MODULE_IMPORT		__declspec(dllimport)
#else   // Linux
#	define PL_MODULE_EXTENSION    ".so"
#	define PL_MODULE_EXPORT        __attribute__((visibility("default")))
#	define PL_MODULE_IMPORT        __attribute__((visibility("hidden")))
#endif

PL_EXTERN PL_FARPROC plFindLibraryFunction(PL_INSTANCE instance, const PLchar *function);

PL_EXTERN PLvoid *plLoadLibraryInterface(PL_INSTANCE instance, const PLchar *path, const PLchar *entry, PLvoid *handle);

PL_INSTANCE plLoadLibrary(const PLchar *path);    // Loads new library instance.
PL_EXTERN PLvoid plUnloadLibrary(PL_INSTANCE instance);    // Unloads library instance.

PL_EXTERN_C_END
