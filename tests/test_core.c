// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "tests.h"

#include "plcore/pl_package.h"

FUNC_TEST( pl_array_vector ) {
	PLVectorArray *vec;

	/* attempt to create vector of size 0 */
	vec = PlCreateVectorArray( 0 );
	if ( vec == NULL ) {
		RETURN_FAILURE( "Failed to create vector array: %s\n", PlGetError() );
	}
	if ( PlGetMaxVectorArrayElements( vec ) != 0 ) {
		RETURN_FAILURE( "Maximum elements not equal to 0\n" );
	}
	PlDestroyVectorArray( vec );

	/* now vector of size 100 */
	vec = PlCreateVectorArray( 100 );
	if ( vec == NULL ) {
		RETURN_FAILURE( "Failed to create vector array w/ max 100 elements: %s\n", PlGetError() );
	}
	if ( PlGetMaxVectorArrayElements( vec ) != 100 ) {
		RETURN_FAILURE( "Maximum elements not equal to 100\n" );
	}
	if ( !PlIsVectorArrayEmpty( vec ) ) {
		RETURN_FAILURE( "Vector should be empty - query returned false!\n" );
	}

	/* fill it with data */
	static unsigned int data[ 100 ];
	for ( unsigned int i = 0; i < 100; ++i ) {
		data[ i ] = i;
		PlPushBackVectorArrayElement( vec, &data[ i ] );
	}
	/* verify */
	unsigned int *v = PlGetVectorArrayElementAt( vec, 5 );
	if ( *v != 5 ) {
		RETURN_FAILURE( "Unexpected data in vector!\n" );
	}
	v = PlGetVectorArrayBack( vec );
	if ( *v != 99 ) {
		RETURN_FAILURE( "Unexpected back element (%u) in vector!\n", *v );
	}

	/* remove several elements */
	for ( unsigned int i = 0; i < 50; ++i ) {
		PlEraseVectorArrayElement( vec, 0 );
		v = PlGetVectorArrayFront( vec );
		if ( *v != ( i + 1 ) ) {
			RETURN_FAILURE( "Unexpected data (%u) in vector at %u!\n", *v, i );
		}
	}
	if ( PlGetNumVectorArrayElements( vec ) != 50 ) {
		RETURN_FAILURE( "Vector not of expected size!\n" );
	}
	for ( unsigned int i = 0; i < 50; ++i ) {
		v = PlGetVectorArrayElementAt( vec, i );
		if ( *v != ( i + 50 ) ) {
			RETURN_FAILURE( "Unexpected data (%u) in vector at %u!\n", *v, i );
		}
	}
	/* ensure the last element is still as expected */
	v = PlGetVectorArrayBack( vec );
	if ( *v != 99 ) {
		RETURN_FAILURE( "Unexpected back element (%u) in vector, expected '99'!\n", *v );
	}
	/* and try popping the last element */
	PlPopVectorArrayBack( vec );
	v = PlGetVectorArrayBack( vec );
	if ( *v != 98 ) {
		RETURN_FAILURE( "Unexpected back element (%u) in vector, expected '98'!\n", *v );
	}

	/* invalid get request */
	if ( PlGetVectorArrayElementAt( vec, 80 ) != NULL ) {
		RETURN_FAILURE( "Invalid GetVectorArrayElementAt request succeeded!\n" );
	}

	/* make sure shrinking works */
	unsigned int maxElements = PlGetMaxVectorArrayElements( vec );
	PlShrinkVectorArray( vec );
	if ( PlGetMaxVectorArrayElements( vec ) >= maxElements ) {
		RETURN_FAILURE( "Shrink failed!\n" );
	}

	if ( PlGetVectorArrayData( vec ) == NULL ) {
		RETURN_FAILURE( "Returned null array!\n" );
	}

	PlDestroyVectorArray( vec );
}
FUNC_TEST_END()

FUNC_TEST( pl_filesystem ) {
	PLPath tmp;
	PlSetupPath( tmp, true, "testing123" );
	if ( strcmp( tmp, "testing123" ) != 0 ) {
		RETURN_FAILURE( "Failed to set path!\n" );
	}

	PlAppendPath( tmp, "_again.txt", true );
	if ( strcmp( tmp, "testing123_again.txt" ) != 0 ) {
		RETURN_FAILURE( "Failed to append path!\n" );
	}

	PlPrefixPath( tmp, "im_", true );
	if ( strcmp( tmp, "im_testing123_again.txt" ) != 0 ) {
		RETURN_FAILURE( "Failed to prefix path!\n" );
	}

	static const char *overflow = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	                              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	                              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	                              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
	                              "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

	if ( PlAppendPath( tmp, overflow, false ) != NULL ) {
		RETURN_FAILURE( "Failed to catch path overflow for append!\n" );
	}
	if ( PlPrefixPath( tmp, overflow, false ) != NULL ) {
		RETURN_FAILURE( "Failed to catch path overflow for prefix!\n" );
	}

	PlAppendPath( tmp, overflow, true );
	if ( tmp[ strlen( tmp ) - 1 ] != 'x' ) {
		RETURN_FAILURE( "Failed to append path with overflow!\n" );
	}

	PlPrefixPath( tmp, overflow, true );
	if ( *tmp != 'x' ) {
		RETURN_FAILURE( "Failed to prefix path with overflow!\n" );
	}
}
FUNC_TEST_END()

FUNC_TEST( pl_package ) {
	PLPackage *packageHandle = PlCreatePackageHandle( "testdata/blob.bin", 0, NULL );
	if ( packageHandle == NULL ) {
		RETURN_FAILURE( "Failed to create package handle!\n" );
	}

	PlAppendPackageFromFile( packageHandle, NULL, "testdata/images/dice.png", PL_COMPRESSION_GZIP )
}
FUNC_TEST_END()

/*============================================================
 * CONSOLE
 ===========================================================*/

static bool test_cmd_called = false;
void CB_test_cmd( unsigned int argc, char **argv ) {
	test_cmd_called = true;
}

FUNC_TEST( RegisterConsoleCommand ) {
	PlRegisterConsoleCommand( "test_cmd", "testing", 0, CB_test_cmd );
	PlParseConsoleString( "test_cmd" );
	if ( !test_cmd_called ) {
		RETURN_FAILURE( "Failed to call \"test_cmd\" command!\n" );
	}
	test_cmd_called = false;
}
FUNC_TEST_END()

FUNC_TEST( GetConsoleCommands ) {
	PLConsoleCommand **cmds;
	size_t numCmds;
	PlGetConsoleCommands( &cmds, &numCmds );
	if ( numCmds < 1 ) {
		RETURN_FAILURE( "Did not receive any commands!\n" );
	}
	if ( cmds == NULL ) {
		RETURN_FAILURE( "Returned null cmds list!\n" );
	}
	bool foundResult = false;
	for ( unsigned int i = 0; i < numCmds; ++i ) {
		const PLConsoleCommand *cmd = cmds[ i ];
		if ( strcmp( "test_cmd", cmd->name ) != 0 ) {
			continue;
		}
		foundResult = true;
	}
	if ( !foundResult ) {
		RETURN_FAILURE( "Failed to find test_cmd!\n" );
	}
}
FUNC_TEST_END()

FUNC_TEST( StringChunk ) {
	static const char *testString = "thisthisthisthisthisthis";
	char *c = pl_strchunksplit( testString, 4, " " );
	if ( c == NULL ) {
		RETURN_FAILURE( "Returned null!\n" );
	}
	bool status = ( strcmp( c, "this this this this this this " ) == 0 );
	PlFree( c );
	if ( !status ) {
		RETURN_FAILURE( "Failed to split string!\n" );
	}
}
FUNC_TEST_END()

/*============================================================
 * COMPRESSION
 ===========================================================*/

#include <plcore/pl_compression.h>

FUNC_TEST( pl_compression ) {
	static const char *string = "The quick brown fox jumped over the lazy dog! !2345678 "
	                            "The quick brown fox jumped over the lazy dog! !3456789 "
	                            "The quick brown fox jumped over the lazy dog! !4567890 "
	                            "The quick brown fox jumped over the lazy dog! !5678901 "
	                            "The quick brown fox jumped over the lazy dog! !6789012 "
	                            "The quick brown fox jumped over the lazy dog! !7890123 "
	                            "The quick brown fox jumped over the lazy dog! !8901234 "
	                            "The quick brown fox jumped over the lazy dog! !9012345 ";

	size_t originalLength = strlen( string ) + 1;
	size_t srcLength = originalLength;
	size_t dstLength;
	void *dst = PlCompress_LZRW1( string, srcLength, &dstLength );
	if ( dst == NULL || ( dstLength >= srcLength ) || dstLength == 0 ) {
		RETURN_FAILURE( "Failed to compress - lzrw1: %s\n", PlGetError() );
	}

	//printf( "Compressed from %lu bytes to %lu bytes\nDecompressing...\n", srcLength, dstLength );

	char *src = PlDecompress_LZRW1( dst, dstLength, &srcLength );
	if ( src == NULL || ( srcLength <= dstLength ) || ( srcLength != originalLength ) || srcLength == 0 ) {
		RETURN_FAILURE( "Failed to decompress - lzrw1: %s\n", PlGetError() );
	}

	//printf( "SOURCE: %s\n", string );
	//printf( "DEST:   %s\n", src );

	if ( strcmp( string, src ) != 0 ) {
		RETURN_FAILURE( "Decompressed result doesn't match original!\n" );
	}
}
FUNC_TEST_END()

/*============================================================
 * LINKED LIST
 ===========================================================*/

#include <plcore/pl_linkedlist.h>

FUNC_TEST( LinkedList ) {
	PLLinkedList *list = PlCreateLinkedList();
	PlInsertLinkedListNode( list, "Hello A" );
	PlInsertLinkedListNode( list, "Hello B" );
	PlInsertLinkedListNode( list, "Hello C" );

	PLLinkedListNode *node;
	node = PlGetFirstNode( list );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello A" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetNextLinkedListNode( node );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello B" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetNextLinkedListNode( node );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello C" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetNextLinkedListNode( node );
	if ( node != NULL ) {
		return TEST_RETURN_FAILURE;
	}

	node = PlGetFirstNode( list );
	PlMoveLinkedListNodeToBack( node );

	node = PlGetFirstNode( list );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello B" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetNextLinkedListNode( node );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello C" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetNextLinkedListNode( node );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello A" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}

	node = PlGetLastNode( list );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello A" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetPrevLinkedListNode( node );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello C" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetPrevLinkedListNode( node );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello B" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}

	node = PlGetLastNode( list );
	PlMoveLinkedListNodeToFront( node );
	node = PlGetFirstNode( list );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello A" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetNextLinkedListNode( node );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello B" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetNextLinkedListNode( node );
	if ( strcmp( PlGetLinkedListNodeUserData( node ), "Hello C" ) != 0 ) {
		return TEST_RETURN_FAILURE;
	}
	node = PlGetNextLinkedListNode( node );
	if ( node != NULL ) {
		return TEST_RETURN_FAILURE;
	}

	PlDestroyLinkedList( list );
}
FUNC_TEST_END()

/*============================================================
 * MEMORY
 ===========================================================*/

FUNC_TEST( HeapMemory ) {
	const char *src = "The quick brown fox jumped over the lazy dog!";
	size_t srcLength = strlen( src ) + 1;

	PLMemoryHeap *heap = PlCreateHeap( 64 );
	if ( PlGetAvailableHeapSize( heap ) != 64 ) {
		RETURN_FAILURE( "PlGetAvailableHeapSize didn't return size allocated!\n" );
	}
	char *dst = PL_HNEW_( heap, char, srcLength );
	snprintf( dst, srcLength, "%s", src );

	if ( strcmp( dst, src ) != 0 ) {
		RETURN_FAILURE( "Source doesn't match destination!\n" );
	}

	if ( PlGetAvailableHeapSize( heap ) != 18 ) {
		RETURN_FAILURE( "Unexpected remaining heap size!\n" );
	}

	/* this should fail due to an overrun */
	dst = PL_HNEW_( heap, char, srcLength );
	if ( dst != NULL ) {
		RETURN_FAILURE( "Heap allocation should've failed due to overrun!\n" );
	}

	PlDestroyHeap( heap );
}
FUNC_TEST_END()

FUNC_TEST( GroupMemory ) {
	PLMemoryGroup *group = PlCreateMemoryGroup();

	size_t a = PlGetTotalAllocatedMemory();
	PL_GNEW_( group, char, 256 );
	PL_GNEW_( group, char, 256 );
	PL_GNEW_( group, char, 256 );
	PL_GNEW_( group, char, 256 );
	PL_GNEW_( group, char, 256 );
	if ( PlGetMemoryGroupSize( group ) != 1280 ) {
		RETURN_FAILURE( "GetMemoryGroupSize returned an expected value!\n" );
	}

	if ( PlGetTotalAllocatedMemory() == a ) {
		RETURN_FAILURE( "GetTotalAllocatedMemory did not track allocations under MemoryGroup!\n" );
	}

	PlFlushMemoryGroup( group );
	size_t c = PlGetTotalAllocatedMemory();
	if ( a != c ) {
		RETURN_FAILURE( "FlushMemoryGroup did not clear memory!\n" );
	}
	if ( PlGetMemoryGroupSize( group ) != 0 ) {
		RETURN_FAILURE( "GetMemoryGroupSize returned an expected value!\n" );
	}

	PlDestroyMemoryGroup( group );
}
FUNC_TEST_END()

/*============================================================
 ===========================================================*/

int main( int argc, char **argv ) {
	if ( PlInitialize( argc, argv ) != PL_RESULT_SUCCESS ) {
		fprintf( stderr, "Failed to initialize Hei: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	TEST_RUN_INIT

	CALL_FUNC_TEST( pl_array_vector )
	CALL_FUNC_TEST( pl_filesystem )

	CALL_FUNC_TEST( RegisterConsoleCommand )
	CALL_FUNC_TEST( GetConsoleCommands )

	/* string */
	CALL_FUNC_TEST( StringChunk )

	CALL_FUNC_TEST( pl_compression )

	CALL_FUNC_TEST( LinkedList )

	/* memory */
	CALL_FUNC_TEST( HeapMemory )
	CALL_FUNC_TEST( GroupMemory )

	TEST_RUN_END
}
