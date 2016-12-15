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

#   error "Unsupported system type."

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

#   error "Unsupported CPU type."

#endif
