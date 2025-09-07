// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

/////////////////////////////////////////////////////////////////////////////////////
// Random
// Please note that these are no way reliable or secure!
/////////////////////////////////////////////////////////////////////////////////////

#if defined( __cplusplus )
extern "C"
{
#endif

	/**
	 * Initialize and return an initial seed value.
	 *
	 * @return Returns a new seed value, based on the current time.
	 */
	unsigned int qm_os_random_seed_initialize();

	/**
	 * Returns a randomized integer value.
	 * Essentially operates the same as rand, but takes a seed.
	 *
	 * @param seed Your seed for the randomization.
	 * @return Randomized integer value.
	 */
	int qm_os_random_int( unsigned int *seed );

	/**
	 * Returns a randomized floating-point value.
	 *
	 * @param seed Your seed for the randomization.
	 * @param max Maximum value, from 0 to max.
	 * @return Randomized floating-point value.
	 */
	float qm_os_random_float( unsigned int *seed, float max );

	/**
	 * Returns a randomized floating-point value, from negative to positive.
	 *
	 * @param seed Your seed for the randomization.
	 * @param minMax Minimum and maximum value.
	 * @return Randomized floating-point value, from -minMax to minMax.
	 */
	float qm_os_random_uniform_float( unsigned int *seed, float minMax );

#if defined( __cplusplus )
};
#endif
