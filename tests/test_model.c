// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plmodel/plm.h>

#include "tests.h"

int main( int argc, char **argv ) {
	if ( PlInitialize( argc, argv ) != PL_RESULT_SUCCESS ) {
		fprintf( stderr, "Failed to initialize Hei: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	bool allPassed = true;



	if ( allPassed )
		printf( "\nAll tests passed successfully!\n" );
	else
		printf( "\nReached end but some tests failed!\n" );

	return EXIT_SUCCESS;
}

