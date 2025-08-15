// Copyright © 2020-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

// technically MSVC isn't supported right now by this library, 
// at least not until C23 is formally supported, however these 
// *may* help it compile...
#if defined( _MSC_VER ) && !defined( __cplusplus )
#	define nullptr   NULL
#	define constexpr const
#endif

#if defined( __cplusplus )
extern "C"
{
#endif

	/////////////////////////////////////////////////////////////////////////////////////
	// Random
	// Please note that these are no way reliable or secure!
	/////////////////////////////////////////////////////////////////////////////////////

	unsigned int qm_os_random_seed_initialize();

	int qm_os_random_int( unsigned int *seed );

	float qm_os_random_float( unsigned int *seed, float max );
	float qm_os_random_uniform_float( unsigned int *seed, float minMax );

#if defined( __cplusplus )
};
#endif
