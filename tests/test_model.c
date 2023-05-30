// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plmodel/plm.h>

#include "tests.h"

FUNC_TEST( plm_format_ply ) {
	PLMModel *model = PlmLoadModel( "testdata/models/test_box.ply" );
	if ( model == NULL ) {
		RETURN_FAILURE( "Failed on load model: %s\n", PlGetError() );
	}
	PlmWriteModel( "out/test_box_ply.smd", model, PLM_MODEL_OUTPUT_DEFAULT );
	PlmDestroyModel( model );
	return TEST_RETURN_SUCCESS;
}
FUNC_TEST_END()

int main( int argc, char **argv ) {
	if ( PlInitialize( argc, argv ) != PL_RESULT_SUCCESS ) {
		fprintf( stderr, "Failed to initialize Hei: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	PlmRegisterStandardModelLoaders( PLM_MODEL_FILEFORMAT_ALL );

	TEST_RUN_INIT

	CALL_FUNC_TEST( plm_format_ply )

	TEST_RUN_END
}
