/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include <plcore/pl.h>
#include <plcore/pl_console.h>
#include <plcore/pl_array_vector.h>
#include <plcore/pl_filesystem.h>

enum {
	TEST_RETURN_SUCCESS,
	TEST_RETURN_FAILURE,
	TEST_RETURN_FATAL,
};

#define FUNC_TEST( NAME )         \
	uint8_t test_##NAME( void ) { \
		printf( " " #NAME "... " );
#define FUNC_TEST_END()         \
	printf( "OK\n" );           \
	return TEST_RETURN_SUCCESS; \
	}

FUNC_TEST( pl_array_vector ) {
	PLVectorArray *vec;

	/* attempt to create vector of size 0 */
	vec = PlCreateVectorArray( 0 );
	if ( vec == NULL ) {
		printf( "Failed to create vector array: %s\n", PlGetError() );
		return TEST_RETURN_FAILURE;
	}
	if ( PlGetMaxVectorArrayElements( vec ) != 0 ) {
		printf( "Maximum elements not equal to 0\n" );
		return TEST_RETURN_FAILURE;
	}
	PlDestroyVectorArray( vec );

	/* now vector of size 100 */
	vec = PlCreateVectorArray( 100 );
	if ( vec == NULL ) {
		printf( "Failed to create vector array w/ max 100 elements: %s\n", PlGetError() );
		return TEST_RETURN_FAILURE;
	}
	if ( PlGetMaxVectorArrayElements( vec ) != 100 ) {
		printf( "Maximum elements not equal to 100\n" );
		return TEST_RETURN_FAILURE;
	}
	if ( !PlIsVectorArrayEmpty( vec ) ) {
		printf( "Vector should be empty - query returned false!\n" );
		return TEST_RETURN_FAILURE;
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
		printf( "Unexpected data in vector!\n" );
		return TEST_RETURN_FAILURE;
	}
	v = PlGetVectorArrayBack( vec );
	if ( *v != 99 ) {
		printf( "Unexpected back element (%u) in vector!\n", *v );
		return TEST_RETURN_FAILURE;
	}

	/* remove several elements */
	for ( unsigned int i = 0; i < 50; ++i ) {
		PlEraseVectorArrayElement( vec, 0 );
		v = PlGetVectorArrayFront( vec );
		if ( *v != ( i + 1 ) ) {
			printf( "Unexpected data (%u) in vector at %u!\n", *v, i );
			return TEST_RETURN_FAILURE;
		}
	}
	if ( PlGetNumVectorArrayElements( vec ) != 50 ) {
		printf( "Vector not of expected size!\n" );
		return TEST_RETURN_FAILURE;
	}
	for ( unsigned int i = 0; i < 50; ++i ) {
		v = PlGetVectorArrayElementAt( vec, i );
		if ( *v != ( i + 50 ) ) {
			printf( "Unexpected data (%u) in vector at %u!\n", *v, i );
			return TEST_RETURN_FAILURE;
		}
	}
	/* ensure the last element is still as expected */
	v = PlGetVectorArrayBack( vec );
	if ( *v != 99 ) {
		printf( "Unexpected back element (%u) in vector, expected '99'!\n", *v );
		return TEST_RETURN_FAILURE;
	}
	/* and try popping the last element */
	PlPopVectorArrayBack( vec );
	v = PlGetVectorArrayBack( vec );
	if ( *v != 98 ) {
		printf( "Unexpected back element (%u) in vector, expected '98'!\n", *v );
		return TEST_RETURN_FAILURE;
	}

	/* invalid get request */
	if ( PlGetVectorArrayElementAt( vec, 80 ) != NULL ) {
		printf( "Invalid GetVectorArrayElementAt request succeeded!\n" );
		return TEST_RETURN_FAILURE;
	}

	/* make sure shrinking works */
	unsigned int maxElements = PlGetMaxVectorArrayElements( vec );
	PlShrinkVectorArray( vec );
	if ( PlGetMaxVectorArrayElements( vec ) >= maxElements ) {
		printf( "Shrink failed!\n" );
		return TEST_RETURN_FAILURE;
	}

	if ( PlGetVectorArrayData( vec ) == NULL ) {
		printf( "Returned null array!\n" );
		return TEST_RETURN_FAILURE;
	}

	PlDestroyVectorArray( vec );
}
FUNC_TEST_END()

FUNC_TEST( pl_filesystem ) {
	PLPath tmp;
	PlSetPath( "testing123", tmp, true );
	if ( strcmp( tmp, "testing123" ) != 0 ) {
		printf( "Failed to set path!\n" );
		return TEST_RETURN_FAILURE;
	}

	PlAppendPath( "_again.txt", tmp, true );
	if ( strcmp( tmp, "testing123_again.txt" ) != 0 ) {
		printf( "Failed to append path!\n" );
		return TEST_RETURN_FAILURE;
	}

	PlPrefixPath( "im_", tmp, true );
	if ( strcmp( tmp, "im_testing123_again.txt" ) != 0 ) {
		printf( "Failed to prefix path!\n" );
		return TEST_RETURN_FAILURE;
	}
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
		printf( "Failed to call \"test_cmd\" command!\n" );
		return TEST_RETURN_FAILURE;
	}
	test_cmd_called = false;
}
FUNC_TEST_END()

FUNC_TEST( GetConsoleCommands ) {
	PLConsoleCommand **cmds;
	size_t numCmds;
	PlGetConsoleCommands( &cmds, &numCmds );
	if ( numCmds < 1 ) {
		printf( "Did not receive any commands!\n" );
		return TEST_RETURN_FAILURE;
	}
	if ( cmds == NULL ) {
		printf( "Returned null cmds list!\n" );
		return TEST_RETURN_FAILURE;
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
		printf( "Failed to find test_cmd!\n" );
		return TEST_RETURN_FAILURE;
	}
}
FUNC_TEST_END()

FUNC_TEST( StringChunk ) {
	static const char *testString = "thisthisthisthisthisthis";
	char *c = pl_strchunksplit( testString, 4, " " );
	if ( c == NULL ) {
		printf( "Returned null!\n" );
		return TEST_RETURN_FAILURE;
	}
	bool status = ( strcmp( c, "this this this this this this " ) == 0 );
	PlFree( c );
	if ( !status ) {
		printf( "Failed to split string!\n" );
		return TEST_RETURN_FAILURE;
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
		printf( "Failed to compress - lzrw1: %s\n", PlGetError() );
		return TEST_RETURN_FAILURE;
	}

	//printf( "Compressed from %lu bytes to %lu bytes\nDecompressing...\n", srcLength, dstLength );

	char *src = PlDecompress_LZRW1( dst, dstLength, &srcLength );
	if ( src == NULL || ( srcLength <= dstLength ) || ( srcLength != originalLength ) || srcLength == 0 ) {
		printf( "Failed to decompress - lzrw1: %s\n", PlGetError() );
		return TEST_RETURN_FAILURE;
	}

	//printf( "SOURCE: %s\n", string );
	//printf( "DEST:   %s\n", src );

	if ( strcmp( string, src ) != 0 ) {
		printf( "Decompressed result doesn't match original!\n" );
		return TEST_RETURN_FAILURE;
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
		printf( "PlGetAvailableHeapSize didn't return size allocated!\n" );
		return TEST_RETURN_FAILURE;
	}
	char *dst = PL_HNEW_( heap, char, srcLength );
	snprintf( dst, srcLength, "%s", src );

	if ( strcmp( dst, src ) != 0 ) {
		printf( "Source doesn't match destination!\n" );
		return TEST_RETURN_FAILURE;
	}

	if ( PlGetAvailableHeapSize( heap ) != 18 ) {
		printf( "Unexpected remaining heap size!\n" );
		return TEST_RETURN_FAILURE;
	}

	/* this should fail due to an overrun */
	dst = PL_HNEW_( heap, char, srcLength );
	if ( dst != NULL ) {
		printf( "Heap allocation should've failed due to overrun!\n" );
		return TEST_RETURN_FAILURE;
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
		printf( "GetMemoryGroupSize returned an expected value!\n" );
		return TEST_RETURN_FAILURE;
	}

	if ( PlGetTotalAllocatedMemory() == a ) {
		printf( "GetTotalAllocatedMemory did not track allocations under MemoryGroup!\n" );
		return TEST_RETURN_FAILURE;
	}

	PlFlushMemoryGroup( group );
	size_t c = PlGetTotalAllocatedMemory();
	if ( a != c ) {
		printf( "FlushMemoryGroup did not clear memory!\n" );
		return TEST_RETURN_FAILURE;
	}
	if ( PlGetMemoryGroupSize( group ) != 0 ) {
		printf( "GetMemoryGroupSize returned an expected value!\n" );
		return TEST_RETURN_FAILURE;
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

	bool allPassed = true;
#define CALL_FUNC_TEST( NAME )                                       \
	{                                                                \
		int ret = test_##NAME();                                     \
		if ( ret != TEST_RETURN_SUCCESS ) {                          \
			fprintf( stderr, "Failed on " #NAME "!\n" );             \
			if ( ret == TEST_RETURN_FATAL ) { return EXIT_FAILURE; } \
			allPassed = false;                                       \
		}                                                            \
	}

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

	if ( allPassed )
		printf( "\nAll tests passed successfully!\n" );
	else
		printf( "\nReached end but some tests failed!\n" );

	return EXIT_SUCCESS;
}
