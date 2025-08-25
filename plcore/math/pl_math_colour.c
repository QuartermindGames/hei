/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_math.h>

/****************************************
 * Type Conversion
 ****************************************/

QmMathColour4ub PlColourF32ToU8( const QmMathColour4f *in ) {
	return ( QmMathColour4ub ){
	        PlFloatToByte( in->r ),
	        PlFloatToByte( in->g ),
	        PlFloatToByte( in->b ),
	        PlFloatToByte( in->a ) };
}

QmMathColour4f PlColourU8ToF32( const QmMathColour4ub *in ) {
	return ( QmMathColour4f ){
	        PlByteToFloat( in->r ),
	        PlByteToFloat( in->g ),
	        PlByteToFloat( in->b ),
	        PlByteToFloat( in->a ) };
}

/****************************************
 * Addition
 ****************************************/

QmMathColour4ub PlAddColour( const QmMathColour4ub *c, const QmMathColour4ub *c2 ) {
	return ( QmMathColour4ub ){
	        c->r + c2->r,
	        c->g + c2->g,
	        c->b + c2->b,
	        c->a + c2->a };
}

QmMathColour4f PlAddColourF32( const QmMathColour4f *c, const QmMathColour4f *c2 ) {
	return ( QmMathColour4f ){
		c->r + c2->r,
		c->g + c2->g,
		c->b + c2->b,
		c->a + c2->a };
}
