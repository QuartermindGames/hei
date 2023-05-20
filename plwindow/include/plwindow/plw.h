// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>
#include <plgraphics/plg.h>

PL_EXTERN_C

typedef struct PLWWindow PLWWindow;

typedef enum PLWError {
	PLW_ERROR_NONE,
	PLW_ERROR_WINDOW_CREATE,
	PLW_ERROR_WINDOW_CLASS,
	PLW_ERROR_WINDOW_GET,

	PLW_MAX_ERRORS
} PLWError;
PLWError PlwGetLastError( void );
const char *PlwGetLastErrorMessage( void );
void PlwClearError( void );

typedef enum PLWMessage {
	PLW_MESSAGE_CREATE,
	PLW_MESSAGE_DESTROY,
	PLW_MESSAGE_ENABLE,
	PLW_MESSAGE_QUIT,

	PLW_MESSAGE_SIZE,
	PLW_MESSAGE_SIZING,

	PLW_MESSAGE_MOVE,
	PLW_MESSAGE_MOVING,

	PLW_MAX_MESSAGES
} PLWMessage;

PLWWindow *PlwCreateWindow( const char *title, int width, int height );
void PlwDestroyWindow( PLWWindow *window );

void PlwGetWindowPosition( PLWWindow *window, int *x, int *y );

void PlwSwapBuffers( PLWWindow *window );

PL_EXTERN_C_END
