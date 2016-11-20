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

/* This document provides platform-specific
 * headers and any additional system information.
 */

#if defined(_WIN32)

#   define PL_SYSTEM_NAME   "WINDOWS"

#elif defined(__APPLE__)

#   define PL_SYSTEM_NAME   "MACOS"

#elif defined(__linux__) // Linux

#   define PL_SYSTEM_NAME   "LINUX"

#else

#   define PL_SYSTEM_NAME   "UNKNOWN"

#endif

#if defined(__amd64) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)

#   define PL_SYSTEM_CPU    "AMD64"

#elif defined(__arm__) || defined(__thumb__) || defined(_M_ARM) || defined(_M_ARMT)

#   define PL_SYSTEM_CPU    "ARM"

#elif defined(__aarch64__)

#   define PL_SYSTEM_CPU    "ARM64"

#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(_M_I86) || defined(_M_IX86) || defined(_X86_)

#   define PL_SYSTEM_CPU    "INTEL86"

#else

#   define PL_SYSTEM_CPU    "UNKNOWN"

#endif
