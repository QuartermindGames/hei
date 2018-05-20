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
#include "graphics_private.h"

#include <PL/platform_graphics_font.h>
#include <PL/platform_filesystem.h>
#include <PL/platform_console.h>

/* ZX Font */
#include "graphics_font_default.h"

/////////////////////////////////////////////////////////////////////////////////////
// FONT PARSER

#define SCRIPT_MAX_LENGTH  2048
#define SCRIPT_MIN_LENGTH  10

#define SCRIPT_MAX_LINE    256

struct {
    char buffer[SCRIPT_MAX_LENGTH];
    char line_buffer[SCRIPT_MAX_LINE];

    unsigned int position;
    unsigned int line, line_position;
    size_t length;
} font_script;

void _plResetParser(void) {
    memset(&font_script, 0, sizeof(font_script));
}

void NextLine(void) {
    font_script.line++;
    font_script.line_position = 0;
}

bool IsEOF(void) {
    if(font_script.position >= font_script.length) {
        return true;
    }

    return false;
}

void SkipComment(void) {
    while(!IsEOF() && (font_script.buffer[font_script.position] != '\n')) {
        font_script.position++;
    }

    font_script.position++;
    NextLine();
}

void ParseLine(void) {
    if(font_script.position >= font_script.length) {
        return;
    }

    while(!IsEOF()) {
        if((font_script.buffer[font_script.position] == '-') && ((font_script.buffer[font_script.position + 1] == '-'))) {
            SkipComment();
            continue;
        }

        if((font_script.line_position == 0) && (font_script.buffer[font_script.position] == '\n')) {
            font_script.position++;
            continue;
        }

        if(font_script.buffer[font_script.position] == '\t') {
            font_script.position++;
            continue;
        }

        if(font_script.buffer[font_script.position] == '\n') {
            font_script.line_buffer[font_script.line_position + 1] = '\0';

            NextLine();
            font_script.position++;
            break;
        }

        font_script.line_buffer[font_script.line_position] = font_script.buffer[font_script.position];
        font_script.position++; font_script.line_position++;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// BITMAP FONT RENDERING

#define _PLFONT_FORMAT_VERSION  1

PLBitmapFont *plParseBitmapFont(const char *name, char *buf, size_t length) {
    _plResetParser();

    font_script.length = length;
    memcpy(font_script.buffer, buf, font_script.length);
    if(font_script.length < SCRIPT_MIN_LENGTH) {
        ReportError(PL_RESULT_FILESIZE, "invalid length, %d, for %s", font_script.length, name);
        return NULL;
    }

    ParseLine();
    if(!strncmp(font_script.line_buffer, "VERSION ", 8)) {
        long version = strtol(font_script.line_buffer + 8, NULL, 0);
        if (version <= 0 || version > _PLFONT_FORMAT_VERSION) {
            ReportError(PL_RESULT_FILEVERSION, "expected version %d, received %d, for %s!",
                        _PLFONT_FORMAT_VERSION, version, name);
            return NULL;
        }
    } else {
        ReportError(PL_RESULT_FILEVERSION, "failed to fetch version for %s", name);
        return NULL;
    }

    ParseLine();
    char texture_path[PL_SYSTEM_MAX_PATH];
    strncpy(texture_path, font_script.line_buffer, sizeof(texture_path));
    if(plIsEmptyString(texture_path)) {
        ReportError(PL_RESULT_FILEPATH, "invalid texture path in font, %s", name);
        return NULL;
    }

    PLBitmapFont *font = (PLBitmapFont*)pl_calloc(1, sizeof(PLBitmapFont));
    if(font == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for BitmapFont, %d!\n", sizeof(PLBitmapFont));
        return NULL;
    }

    bool enable_filter = false;
    while(!IsEOF()) {
        ParseLine();

        if(strncmp(font_script.line_buffer, "FILTER ", 7) == 0) {
            if(font_script.line_buffer[8] == '1') {
                enable_filter = true;
            }
            continue;
        }

        int8_t character = font_script.line_buffer[0];
        if(character == ' ') {
            continue;
        }

        char *pos;
        font->chars[character].x = (int)strtol(font_script.line_buffer + 2, &pos, 10);
        font->chars[character].y = (int)strtol(pos, &pos, 10);
        font->chars[character].w = (int)strtoul(pos, &pos, 10);
        font->chars[character].h = (int)strtoul(pos, &pos, 10);

#if 0
        printf("CHAR(%c) X(%d) Y(%d) W(%d) H(%d)\n",
               character,
               font->chars[character].x,
               font->chars[character].y,
               font->chars[character].w,
               font->chars[character].h
        );
#endif
    }

    PLImage image;
    if(pl_strncasecmp("$zx", texture_path, 3) == 0) {
        memset(&image, 0, sizeof(PLImage));
        image.width         = 128;
        image.height        = 56;
        image.colour_format = PL_COLOURFORMAT_RGB;
        image.format        = PL_IMAGEFORMAT_RGB8;
        image.size          = plGetImageSize(image.format, image.width, image.height);
        image.levels        = 1;

        if((image.data = pl_calloc(image.levels, sizeof(uint8_t*))) == NULL ||
                (image.data[0] = pl_calloc(image.size, sizeof(uint8_t))) == NULL) {
            plDeleteBitmapFont(font);

            ReportError(PL_RESULT_MEMORY_ALLOCATION, "failed to allocate memory for texture data");
            return NULL;
        }
        memcpy(image.data[0], f_zx_image, sizeof(f_zx_image));

        plReplaceImageColour(&image, PL_COLOUR_BLACK, PLColour(0, 0, 0, 0));
    } else {
        if(!plLoadImage(texture_path, &image)) {
            plDeleteBitmapFont(font);

            ReportError(PL_RESULT_FILEPATH, "failed to find texture at %s, for %s", texture_path, name);
            return NULL;
        }
    }

    if((font->texture = plCreateTexture()) == NULL) {
        plFreeImage(&image);
        plDeleteBitmapFont(font);
        return false;
    }

    if(enable_filter) {
        font->texture->filter = PL_TEXTURE_FILTER_LINEAR;
    } else {
        font->texture->filter = PL_TEXTURE_FILTER_NEAREST;
    }

    plSetTextureFlags(font->texture, PL_TEXTURE_FLAG_NOMIPS | PL_TEXTURE_FLAG_PRESERVE);

    plUploadTextureImage(font->texture, &image);
    plFreeImage(&image);

    return font;
}

PLBitmapFont *plCreateDefaultBitmapFont(void) {
    _plResetError();

    static PLBitmapFont *font = NULL;
    if(font == NULL) {
        size_t length = sizeof(f_zx_script);
        char buf[length];
        memcpy(buf, f_zx_script, length);
        if((font = plParseBitmapFont("default", buf, length)) == NULL) {
            /* ParseBitmapFont takes care of error reporting */
            return NULL;
        }
    }
    return font;
}

PLBitmapFont *plCreateBitmapFont(const char *path) {
    if(plIsEmptyString(path)) {
        ReportError(PL_RESULT_FILEPATH, "invalid path for bitmap font");
        return NULL;
    }

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        ReportError(PL_RESULT_FILEPATH, "failed to open %s", path);
        return NULL;
    }

    char buf[SCRIPT_MAX_LENGTH];
    size_t length = fread(buf, 1, SCRIPT_MAX_LENGTH, file);
    fclose(file);

    PLBitmapFont *font = plParseBitmapFont(path, buf, length);
    if(font == NULL) {
        /* ParseBitmapFont takes care of error reporting */
        return NULL;
    }

    return font;
}

void plDeleteBitmapFont(PLBitmapFont *font) {
    if(font == NULL) {
        return;
    }

    if(font->texture != NULL) {
        plDeleteTexture(font->texture, false);
    }

    free(font);
}

void plDrawBitmapCharacter(PLBitmapFont *font, int x, int y, float scale, PLColour colour, int8_t character) {
    if(font->chars[character].w == 0 || font->chars[character].h == 0) {
        return;
    }

    if(!isalnum(character) || isspace(character)) {
        return;
    }

    static PLMesh *mesh = NULL;
    if(mesh == NULL) {
        if((mesh = plCreateMesh(
                PL_MESH_TRIANGLE_STRIP,
                PL_DRAW_IMMEDIATE,
                2, 4
        )) == NULL) {
            return;
        }
    }

    float w = font->chars[character].w;/* * scale; */
    float h = font->chars[character].h;/* * scale; */

    plClearMesh(mesh);

    plSetTexture(font->texture, 0);

    plSetMeshUniformColour(mesh, colour);

    plSetMeshVertexPosition(mesh, 0, PLVector3(x, y, 0));
    plSetMeshVertexPosition(mesh, 1, PLVector3(x, y + h, 0));
    plSetMeshVertexPosition(mesh, 2, PLVector3(x + w, y, 0));
    plSetMeshVertexPosition(mesh, 3, PLVector3(x + w, y + h, 0));

    float tw = font->chars[character].w / font->texture->w;
    float th = font->chars[character].h / font->texture->h;
    float tx = font->chars[character].x / font->texture->w;
    float ty = font->chars[character].y / font->texture->h;
    plSetMeshVertexST(mesh, 0, tx, ty);
    plSetMeshVertexST(mesh, 1, tx, ty + th);
    plSetMeshVertexST(mesh, 2, tx + tw, ty);
    plSetMeshVertexST(mesh, 3, tx + tw, ty + th);

    plUploadMesh(mesh);
    plDrawMesh(mesh);
}

void plDrawBitmapString(PLBitmapFont *font, int x, int y, float scale, PLColour colour, const char *msg) {
    plDrawTexturedRectangle(0, 0, font->texture->w, font->texture->h, font->texture);

    if(colour.a == 0) {
        return;
    }

    if(scale <= 0 || x < 0 || y > (int)gfx_state.current_viewport.w || y < 0 || y > (int)gfx_state.current_viewport.h) {
        return;
    }

    unsigned int length = (unsigned int)strlen(msg);
    if(length == 0) {
        return;
    }

    int n_x = x;
    int n_y = y;
    for(unsigned int i = 0; i < length; i++) {
        if(msg[i] < 0 || msg[i] > 127) {
            // out of range
            continue;
        }

        uint8_t c = (uint8_t) msg[i];
        plDrawBitmapCharacter(font, n_x, n_y, scale, colour, msg[i]);
        if(msg[i] == '\n') {
            n_y += font->chars[c].h;
            n_x = x;
        } else {
            n_x += font->chars[c].w;
        }
    }
}