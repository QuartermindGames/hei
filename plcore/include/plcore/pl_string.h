/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <stdarg.h>

#if 0
PL_INLINE static void plGetStringExtension(char *out, const char *in, unsigned int length) {
    const char *s = strrchr(in, '.');type
    if(!s || s[0] == '\0') {
        return;
    }

    strncpy(out, s + 1, length);
}

PL_INLINE static void plStripStringExtension(char *out, const char *in, unsigned int length) {
    const char *s = strrchr(in, '.');
    while(in < s) {
        *out++ = *in++;
    }
    *out = 0;
}
#endif

char *pl_itoa(int val, char *buf, size_t len, int base);

char *pl_strtolower(char *s);
char *pl_strntolower(char *s, size_t n);
char *pl_strtoupper(char *s);
char *pl_strntoupper(char *s, size_t n);

char *pl_strcasestr(const char *s, const char *find);

int pl_strcasecmp(const char *s1, const char *s2);
int pl_strncasecmp(const char *s1, const char *s2, size_t n);

int pl_strisalpha(const char *s);
int pl_strnisalpha(const char *s, unsigned int n);
int pl_strisalnum(const char *s);
int pl_strnisalnum(const char *s, unsigned int n);
int pl_strisdigit(const char *s);
int pl_strnisdigit(const char *s, unsigned int n);

int pl_vscprintf( const char *format, va_list pArgs );

unsigned int pl_strcnt( const char *s, char c );
unsigned int pl_strncnt( const char *s, char c, unsigned int n );

char *PlStrInsert( const char *string, char **buf, size_t *bufSize, size_t *maxBufSize );
