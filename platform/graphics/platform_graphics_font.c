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

#include <PL/platform_graphics_font.h>
#include <PL/platform_filesystem.h>

/////////////////////////////////////////////////////////////////////////////////////
// FONT PARSER

#define _PLFONT_MAX_LENGTH  2048
#define _PLFONT_MIN_LENGTH  10

#define _PLFONT_MAX_LINE    256

typedef struct _PLFontScript {
    char buffer[_PLFONT_MAX_LENGTH];
    char line_buffer[_PLFONT_MAX_LINE];

    unsigned int position;
    unsigned int line, line_position;
    unsigned int length;
} _PLFontScript;

_PLFontScript _pl_font_script;

#define _BUFFER         _pl_font_script.buffer
#define _BUFFER_LINE    _pl_font_script.line_buffer
#define _LENGTH         _pl_font_script.length
#define _POSITION       _pl_font_script.position
#define _LINE_POSITION  _pl_font_script.line_position
#define _LINE           _pl_font_script.line

void _plResetFontParser(void) {
    memset(&_pl_font_script, 0, sizeof(_PLFontScript));
}

void _plNextFontLine(void) {
    _LINE++;
    _LINE_POSITION = 0;
}

bool _plFontEOF(void) {
    if(_POSITION >= _LENGTH) {
        return true;
    }

    return false;
}

void _plSkipFontComment(void) {
    while(!_plFontEOF() && (_BUFFER[_POSITION] != '\n')) {
        _POSITION++;
    }

    _POSITION++;
    _plNextFontLine();
}

void _plParseFontLine(void) {
    if(_POSITION >= _LENGTH) {
        return;
    }

    while(!_plFontEOF()) {
        if((_BUFFER[_POSITION] == '-') && ((_BUFFER[_POSITION + 1] == '-'))) {
            _plSkipFontComment();
            continue;
        } else if((_LINE_POSITION == 0) && (_BUFFER[_POSITION] == '\n')) {
            _POSITION++;
            continue;
        } else if(_BUFFER[_POSITION] == '\t') {
            _POSITION++;
            continue;
        } else if(_BUFFER[_POSITION] == '\n') {
            _BUFFER_LINE[_LINE_POSITION + 1] = '\0';

            _plNextFontLine();
            _POSITION++;
            break;
        }

        _BUFFER_LINE[_LINE_POSITION] = _BUFFER[_POSITION];
        _POSITION++; _LINE_POSITION++;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// BITMAP FONT RENDERING

#define _PLFONT_FORMAT_VERSION  1

PLBitmapFont *plCreateBitmapFont(const char *path) {
    _plResetFontParser();

    FILE *file = fopen(path, "r");
    if(!file) {
        _plReportError(PL_RESULT_FILEPATH, "Failed to open %s!\n", path);
        return NULL;
    }

    _pl_font_script.length = (unsigned int)fread(_pl_font_script.buffer, 1, _PLFONT_MAX_LENGTH, file);
    fclose(file);

    if(_pl_font_script.length < _PLFONT_MIN_LENGTH) {
        _plReportError(PL_RESULT_FILESIZE, "Invalid length, %d, for %s!\n", _LENGTH, path);
        return NULL;
    }

    _plParseFontLine();
    if(!strncmp(_BUFFER_LINE, "VERSION ", 8)) {
        int version = atoi(_BUFFER_LINE + 8);
        if (version <= 0 || version > _PLFONT_FORMAT_VERSION) {
            _plReportError(PL_RESULT_FILEVERSION, "Expected version %d, received %d, for %s!\n",
                           _PLFONT_FORMAT_VERSION, version, path);
            return NULL;
        }
    } else {
        _plReportError(PL_RESULT_FILEVERSION, "Failed to fetch version for %s!\n", path);
        return NULL;
    }

    _plParseFontLine();
    if(!plFileExists(_BUFFER_LINE)) {
        _plReportError(PL_RESULT_FILEPATH, "Failed to find texture at %s, for %s!\n", _BUFFER_LINE, path);
        return NULL;
    }

    PLTexture *texture = plCreateTexture();
    if(!texture) {
        return NULL;
    }

    PLImage image;
    if(plLoadImage(_BUFFER_LINE, &image) != PL_RESULT_SUCCESS) {
        return NULL;
    }

    plUploadTextureImage(texture, &image);

    PLBitmapFont *font = (PLBitmapFont*)malloc(sizeof(PLBitmapFont));
    if(!font) {
        plDeleteTexture(texture, false);
        plFreeImage(&image);

        _plReportError(PL_RESULT_MEMORYALLOC, "Failed to allocate memory for BitmapFont, %d!\n", sizeof(PLBitmapFont));
        return NULL;
    }

    font->texture = texture;

    while(!_plFontEOF()) {
        _plParseFontLine();

        //font->chars[_BUFFER_LINE[0]].character = _BUFFER_LINE[0];
        sscanf(_BUFFER_LINE + 2, "%d %d %d %d",
                         &font->chars[_BUFFER_LINE[0]].x, &font->chars[_BUFFER_LINE[0]].y,
                         &font->chars[_BUFFER_LINE[0]].w, &font->chars[_BUFFER_LINE[0]].h
        );

#if 0
        printf("CHAR(%c) X(%d) Y(%d) W(%d) H(%d)\n",
               font->chars[_BUFFER_LINE[0]].character,
               font->chars[_BUFFER_LINE[0]].x,
               font->chars[_BUFFER_LINE[0]].y,
               font->chars[_BUFFER_LINE[0]].w,
               font->chars[_BUFFER_LINE[0]].h
        );
#endif
    }

    return NULL;
}

void plDeleteBitmapFont(PLBitmapFont *font) {
    if(!font) {
        return;
    }

    plDeleteTexture(font->texture, false);
}