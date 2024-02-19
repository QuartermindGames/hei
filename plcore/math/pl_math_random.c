/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_math.h>

int *PlSeedRandom( int seed ) {
	int *s = ( int * ) PlMAllocA( sizeof( int ) );
	*s = seed;
	return s;
}

/****************************************
 * Perlin noise generation, based on 'Improved Noise'.
 * https://mrl.nyu.edu/~perlin/noise/
 ****************************************/

#define PERLIN_NOISE_SAMPLE 512

static const int perlinHash[] = {
        151, 160, 137, 91, 90, 15,
        131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
        190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
        88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
        77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
        102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
        135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
        5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
        223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
        129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
        251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
        49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
        138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };

/**
 * The first parameter is optional but allows
 * you to provide different patterns to the
 * noise algorithm if desired, otherwise if it's
 * null it just uses the default sample.
 *
 * Returns a pointer to the table which will
 * need to be freed after use.
 */
int *PlSeedPerlin( const int *hashTable ) {
	int *s = PL_NEW_( int, PERLIN_NOISE_SAMPLE );
	if ( hashTable == NULL ) {
		hashTable = perlinHash;
	}

	for ( unsigned int i = 0; i < PERLIN_NOISE_SAMPLE / 2; ++i ) {
		s[ ( PERLIN_NOISE_SAMPLE / 2 ) + i ] = s[ i ] = hashTable[ i ];
	}
	return s;
}

static double PerlinNoiseFade( double t ) {
	return t * t * t * ( t * ( t * 6 - 15 ) + 10 );
}

static double PerlinNoiseLerp( double t, double a, double b ) {
	return a + t * ( b - a );
}

static double PerlinNoiseGrad( int hash, double x, double y, double z ) {
	// Convert lo 4 bits of hash code into 12 gradient directions
	int h = hash & 15;
	double u = h < 8 ? x : y,
	       v = h < 4 ? y : h == 12 || h == 14 ? x
	                                          : z;
	return ( ( h & 1 ) == 0 ? u : -u ) + ( ( h & 2 ) == 0 ? v : -v );
}

double PlGeneratePerlinNoise( int *seed, double x, double y, double z ) {
	// Find unit cube that contains point
	int X = ( int ) floor( x ) & 255;
	int Y = ( int ) floor( y ) & 255;
	int Z = ( int ) floor( z ) & 255;

	// Find relative X, Y and Z of point in cube
	x -= floor( x );
	y -= floor( y );
	z -= floor( z );

	// Compute fade curves for each of X, Y and Z
	double u = PerlinNoiseFade( x );
	double v = PerlinNoiseFade( y );
	double w = PerlinNoiseFade( z );

	// Hash coordinates of the 8 cube corners
	int A = seed[ X ] + Y, AA = seed[ A ] + Z, AB = seed[ A + 1 ] + Z,
	    B = seed[ X + 1 ] + Y, BA = seed[ B ] + Z, BB = seed[ B + 1 ] + Z;

	// And add blended results from 8 corners of cube
	return PerlinNoiseLerp( w, PerlinNoiseLerp( v, PerlinNoiseLerp( u, PerlinNoiseGrad( seed[ AA ], x, y, z ), PerlinNoiseGrad( seed[ BA ], x - 1, y, z ) ), PerlinNoiseLerp( u, PerlinNoiseGrad( seed[ AB ], x, y - 1, z ), PerlinNoiseGrad( seed[ BB ], x - 1, y - 1, z ) ) ),
	                        PerlinNoiseLerp( v, PerlinNoiseLerp( u, PerlinNoiseGrad( seed[ AA + 1 ], x, y, z - 1 ), PerlinNoiseGrad( seed[ BA + 1 ], x - 1, y, z - 1 ) ),
	                                         PerlinNoiseLerp( u, PerlinNoiseGrad( seed[ AB + 1 ], x, y - 1, z - 1 ),
	                                                          PerlinNoiseGrad( seed[ BB + 1 ], x - 1, y - 1, z - 1 ) ) ) );
}
