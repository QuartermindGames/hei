/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PLVector3 PLVector3;

#if !defined( PL_COMPILE_PLUGIN )

extern bool PlIsEndOfLine( const char **p );
extern void PlSkipWhitespace( const char **p );
extern void PlSkipLine( const char **p );
extern const char *PlParseEnclosedString( const char **p, char *dest, size_t size );
extern const char *PlParseToken( const char **p, char *dest, size_t size );
extern int PlParseInteger( const char **p, bool *status );
extern float PlParseFloat( const char **p, bool *status );
extern PLVector3 PlParseVector( const char **p, bool *status );

#endif /* !defined( PL_COMPILE_PLUGIN ) */

#ifdef __cplusplus
};
#endif
