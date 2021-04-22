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

#include "pl_private.h"

#include <plcore/pl_math.h>

/* Matrix Stack, sorta mirrors OpenGL behaviour */

#define MAX_STACK_SIZE 64
static PLMatrix4 stacks[ PL_NUM_MATRIX_MODES ][ MAX_STACK_SIZE ];
static PLMatrixMode curMatrixMode = PL_MODELVIEW_MATRIX;
static unsigned int curStackSlot[ PL_NUM_MATRIX_MODES ];

/**
 * Returns a pointer to the current matrix stack.
 */
PLMatrix4 *plGetMatrix( PLMatrixMode mode ) {
	return &stacks[ mode ][ curStackSlot[ mode ] ];
}

/**
 * Private function that initializes all of the matrix stacks.
 */
void _plInitializeMatrixStacks( void ) {
	/* make sure all of the slots default to 0 */
	memset( curStackSlot, 0, sizeof( unsigned int ) * PL_NUM_MATRIX_MODES );

	/* every stack starts off with identity matrix for first slot */
	for ( unsigned int i = 0; i < PL_NUM_MATRIX_MODES; ++i ) {
		stacks[ i ][ 0 ] = plMatrix4Identity();
	}
}

void plMatrixMode( PLMatrixMode mode ) {
	curMatrixMode = mode;
}

PLMatrixMode plGetMatrixMode( void ) {
	return curMatrixMode;
}

void plLoadMatrix( const PLMatrix4 *matrix ) {
	PLMatrix4 *curStack = plGetMatrix( curMatrixMode );
	*curStack = *matrix;
}

void plLoadIdentityMatrix( void ) {
	PLMatrix4 identity = plMatrix4Identity();
	plLoadMatrix( &identity );
}

void plMultiMatrix( const PLMatrix4 *matrix ) {
	PLMatrix4 *curStack = plGetMatrix( curMatrixMode );
	*curStack = plMultiplyMatrix4( *curStack, *matrix );
}

void plRotateMatrix( float angle, float x, float y, float z ) {
	PLMatrix4 rotation = plRotateMatrix4( angle, PLVector3( x, y, z ) );
	plMultiMatrix( &rotation );
}

void plTranslateMatrix( PLVector3 vector ) {
	PLMatrix4 translate = plTranslateMatrix4( vector );
	plMultiMatrix( &translate );
}

void plScaleMatrix( PLVector3 scale ) {
	PLMatrix4 *curStack = plGetMatrix( curMatrixMode );
	*curStack = plScaleMatrix4( *curStack, scale );
}

void plPushMatrix( void ) {
	if ( curStackSlot[ curMatrixMode ] >= MAX_STACK_SIZE ) {
		plReportBasicError( PL_RESULT_MEMORY_EOA );
		return;
	}

	PLMatrix4 *curStack = plGetMatrix( curMatrixMode );
	curStackSlot[ curMatrixMode ]++;
	PLMatrix4 *newStack = plGetMatrix( curMatrixMode );
	*newStack = *curStack;
}

void plPopMatrix( void ) {
	if ( curStackSlot[ curMatrixMode ] == 0 ) {
		plReportBasicError( PL_RESULT_MEMORY_UNDERFLOW );
		return;
	}

	curStackSlot[ curMatrixMode ]--;
}
