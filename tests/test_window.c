// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plwindow/plw.h>

#include "tests.h"

FUNC_TEST( CreateWindow ) {
	PLWWindow *window = PlwCreateWindow( "Hello World", 800, 600 );
	if ( window == NULL ) {
		RETURN_FAILURE( "Failed on creating window: %s\n", PlwGetLastErrorMessage() );
	}

	PlwDestroyWindow( window );
}
FUNC_TEST_END()

int main( int argc, char **argv ) {
	if ( PlInitialize( argc, argv ) != PL_RESULT_SUCCESS ) {
		fprintf( stderr, "Failed to initialize Hei: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	TEST_RUN_INIT

	CALL_FUNC_TEST( CreateWindow )

	TEST_RUN_END
}
