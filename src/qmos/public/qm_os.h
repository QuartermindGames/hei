// Copyright © 2020-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

/////////////////////////////////////////////////////////////////////////////////////
// Random
/////////////////////////////////////////////////////////////////////////////////////

unsigned int qm_os_random_seed_initialize();

int qm_os_random_int( unsigned int *seed );

float qm_os_random_float( unsigned int *seed, float max );
float qm_os_random_uniform_float( unsigned int *seed, float minMax );
