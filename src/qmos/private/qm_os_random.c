// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Randomisation methods.
// Author:  Mark E. Sowden

#include <time.h>
#include <stdlib.h>

#include "qmos/public/qm_os.h"

#if defined( _WIN32 )
static int rand_r( unsigned int *seed )
{
	*seed = *seed * 1103515245 + 12345;
	return *seed / 65536 % ( RAND_MAX - 1 );
}
#endif

unsigned int qm_os_random_seed_initialize()
{
	return ( unsigned int ) time( nullptr ) + ( unsigned int ) clock();
}

int qm_os_random_int( unsigned int *seed )
{
	return rand_r( seed );
}

float qm_os_random_float( unsigned int *seed, float max )
{
	return ( float ) rand_r( seed ) / ( RAND_MAX / max );
}

// http://stackoverflow.com/questions/7978759/generate-float-random-values-also-negative
static float uniform_0_to_1_random( unsigned int *seed )
{
	return rand_r( seed ) / ( ( float ) RAND_MAX + 1 );
}

float qm_os_random_uniform_float( unsigned int *seed, float minMax )
{
	return minMax * 2.0f * uniform_0_to_1_random( seed ) - minMax;
}
