// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_console.h>
#include <plcore/pl_array_vector.h>
#include <plcore/pl_filesystem.h>

enum {
	TEST_RETURN_SUCCESS,
	TEST_RETURN_FAILURE,
	TEST_RETURN_FATAL,
};

#define TEST_RUN_INIT \
	bool allPassed = true;
#define TEST_RUN_END                                        \
	if ( allPassed ) {                                      \
		printf( "\nAll tests passed successfully!\n" );     \
		return EXIT_SUCCESS;                                \
	} else {                                                \
		printf( "\nReached end but some tests failed!\n" ); \
		return EXIT_FAILURE;                                \
	}

#define FUNC_TEST( NAME )         \
	uint8_t test_##NAME( void ) { \
		printf( " " #NAME "... " );
#define FUNC_TEST_END()         \
	printf( "OK\n" );           \
	return TEST_RETURN_SUCCESS; \
	}
#define CALL_FUNC_TEST( NAME )                                       \
	{                                                                \
		int ret = test_##NAME();                                     \
		if ( ret != TEST_RETURN_SUCCESS ) {                          \
			fprintf( stderr, "Failed on " #NAME "!\n" );             \
			if ( ret == TEST_RETURN_FATAL ) { return EXIT_FAILURE; } \
			allPassed = false;                                       \
		}                                                            \
	}

#define RETURN_FAILURE( ... ) \
	printf( __VA_ARGS__ );    \
	return TEST_RETURN_FAILURE
