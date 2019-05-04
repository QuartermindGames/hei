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

#include "platform_private.h"

struct {
    char *buffer, *line_buffer;

    unsigned int buffer_size, line_size;
    unsigned int line_length;

    unsigned int position;
    unsigned int line, line_position;

    unsigned int length;
} parser_block;

#define PARSE_EOF() (parser_block.position >= parser_block.length)

void plResetParser(void) {
    if(parser_block.buffer) {
        pl_free(parser_block.buffer);
    }
    if(parser_block.line_buffer) {
        pl_free(parser_block.line_buffer);
    }
    memset(&parser_block, 0, sizeof(parser_block));
}

void plParseNextLine(void) {
    parser_block.line++;
    parser_block.line_position = 0;
}

void plSkipComment(void) {
    while(!PARSE_EOF() && (parser_block.buffer[parser_block.position] != '\n')) {
        parser_block.position++;
    }
    parser_block.position++;
    plParseNextLine();
}

void plParseLine(void) {
    if(PARSE_EOF()) {
        return;
    }

    while(!PARSE_EOF()) {
        if((parser_block.buffer[parser_block.position] == '-') ||
                (parser_block.buffer[parser_block.position + 1] == '-')) {
            plSkipComment();
            continue;
        }

        if((parser_block.line_position == 0) && (parser_block.buffer[parser_block.position] == '\n')) {
            parser_block.position++;
            continue;
        }

        if(parser_block.buffer[parser_block.position] == '\t') {
            parser_block.position++;
            continue;
        }

        if(parser_block.buffer[parser_block.position] == '\n') {
            parser_block.line_buffer[parser_block.position + 1] = '\0';
            plParseNextLine();
            parser_block.position++;
            break;
        }

        parser_block.line_buffer[parser_block.line_position] = parser_block.buffer[parser_block.position];
        parser_block.position++; parser_block.line_position++;
    }
}

void plSetupParser(const char *buffer, unsigned int length) {
    plAssert(length);

    if(!(parser_block.buffer = (char*)pl_malloc(length))) {
    }

    parser_block.buffer_size = length;
}


