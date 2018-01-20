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
    unsigned int length;
} font_script;

void ResetParser(void) {
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

PLBitmapFont *plCreateBitmapFont(const char *path) {
    ResetParser();

    FILE *file = fopen(path, "r");
    if(file == NULL) {
        ReportError(PL_RESULT_FILEPATH, "Failed to open %s!\n", path);
        return NULL;
    }

    font_script.length = (unsigned int)fread(font_script.buffer, 1, SCRIPT_MAX_LENGTH, file);
    fclose(file);

    if(font_script.length < SCRIPT_MIN_LENGTH) {
        ReportError(PL_RESULT_FILESIZE, "Invalid length, %d, for %s!\n", font_script.length, path);
        return NULL;
    }

    ParseLine();
    if(!strncmp(font_script.line_buffer, "VERSION ", 8)) {
        long version = strtol(font_script.line_buffer + 8, NULL, 0);
        if (version <= 0 || version > _PLFONT_FORMAT_VERSION) {
            ReportError(PL_RESULT_FILEVERSION, "Expected version %d, received %d, for %s!\n",
                           _PLFONT_FORMAT_VERSION, version, path);
            return NULL;
        }
    } else {
        ReportError(PL_RESULT_FILEVERSION, "Failed to fetch version for %s!\n", path);
        return NULL;
    }

    ParseLine();
    if(!plFileExists(font_script.line_buffer)) {
        ReportError(PL_RESULT_FILEPATH, "Failed to find texture at %s, for %s!\n", font_script.line_buffer, path);
        return NULL;
    }

    char image_path[PL_SYSTEM_MAX_PATH] = { 0 };
    strncpy(image_path, font_script.line_buffer, sizeof(image_path));

    PLBitmapFont *font = (PLBitmapFont*)malloc(sizeof(PLBitmapFont));
    if(font == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for BitmapFont, %d!\n", sizeof(PLBitmapFont));
        return NULL;
    }
    memset(font, 0, sizeof(PLBitmapFont));

    //bool enable_filter = false;
    while(!IsEOF()) {
        ParseLine();

        if(!strncmp(font_script.line_buffer, "FILTER ", 7)) {
            if(font_script.line_buffer[8] == '1') {
                //enable_filter = true;
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
    if(plLoadImage(image_path, &image) != PL_RESULT_SUCCESS) {
        plDeleteBitmapFont(font);
        return NULL;
    }

    if((font->texture = plCreateTexture()) == NULL) {
        plDeleteBitmapFont(font);
        plFreeImage(&image);
        return NULL;
    }

    plSetTextureFlags(font->texture, PL_TEXTURE_FLAG_NOMIPS | PL_TEXTURE_FLAG_PRESERVE);
    plUploadTextureImage(font->texture, &image);

    plFreeImage(&image);

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

    float w = font->chars[character].w * scale;
    float h = font->chars[character].h * scale;

    plClearMesh(mesh);

    plSetMeshUniformColour(mesh, colour);

    plSetMeshVertexPosition2f(mesh, 0, x, y);
    plSetMeshVertexPosition2f(mesh, 1, x, y + h);
    plSetMeshVertexPosition2f(mesh, 2, x + w, y);
    plSetMeshVertexPosition2f(mesh, 3, x + w, y + h);

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
        if(msg[i] < 0 || msg[i] > 255) {
            // out of range
            continue;
        }

        unsigned char ch = (unsigned char)msg[i];
        plDrawBitmapCharacter(font, n_x, n_y, scale, colour, ch);
        if(ch == '\n') {
            n_y += font->chars[ch].h;
            n_x = x;
        } else {
            n_x += font->chars[ch].w;
        }
    }
}