// Copyright © 2020-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Randomisation methods.
// Author:  Mark E. Sowden

#include <time.h>
#include <stdlib.h>

#include "qmos/public/qm_os.h"

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
static inline float uniform_0_to_1_random( unsigned int *seed )
{
	return rand_r( seed ) / ( ( float ) RAND_MAX + 1 );
}

float qm_os_random_uniform_float( unsigned int *seed, float minMax )
{
	return minMax * 2.0f * uniform_0_to_1_random( seed ) - minMax;
}
