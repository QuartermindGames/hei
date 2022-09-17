/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>

enum {
	PL_PARSE_NL_INVALID = -1,
	PL_PARSE_NL_EOF = 0,
	PL_PARSE_NL_LF = 1,
	PL_PARSE_NL_CRLF = 2,
};

PL_EXTERN_C

typedef struct PLVector3 PLVector3;

#if !defined( PL_COMPILE_PLUGIN )

bool PlIsEndOfLine( const char *p );
int PlGetLineEndType( const char *p );

void PlSkipWhitespace( const char **p );
void PlSkipLine( const char **p );
const char *PlParseEnclosedString( const char **p, char *dest, size_t size );
unsigned int PlDetermineTokenLength( const char *p );
const char *PlParseToken( const char **p, char *dest, size_t size );
int PlParseInteger( const char **p, bool *status );
float PlParseFloat( const char **p, bool *status );
PLVector3 PlParseVector( const char **p, bool *status );

unsigned int PlDetermineLineLength( const char *p );
const char *PlParseLine( const char **p, char *dest, size_t size );

#endif /* !defined( PL_COMPILE_PLUGIN ) */

PL_EXTERN_C_END
