#include <plcore/pl.h>
#include <plcore/pl_compression.h>

const char *string = "The quick brown fox jumped over the lazy dog! !2345678 "
                     "The quick brown fox jumped over the lazy dog! !2345678 "
                     "The quick brown fox jumped over the lazy dog! !2345678 "
                     "The quick brown fox jumped over the lazy dog! !2345678 "
                     "The quick brown fox jumped over the lazy dog! !2345678 "
                     "The quick brown fox jumped over the lazy dog! !2345678 "
                     "The quick brown fox jumped over the lazy dog! !2345678 "
                     "The quick brown fox jumped over the lazy dog! !2345678 ";

int main( int argc, char **argv ) {
	PlInitialize( argc, argv );

	size_t originalLength = strlen( string ) + 1;

	size_t srcLength = originalLength;
	size_t dstLength;
	void *dst = PlCompress_LZRW1( string, srcLength, &dstLength );
	if ( dst == NULL || ( dstLength >= srcLength ) || dstLength == 0 ) {
		printf( "Failed to compress - lzrw1: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	printf( "Compressed from %u bytes to %u bytes\nDecompressing...\n", srcLength, dstLength );

	char *src = PlDecompress_LZRW1( dst, dstLength, &srcLength );
	if ( src == NULL || ( srcLength <= dstLength ) || ( srcLength != originalLength ) || srcLength == 0 ) {
		printf( "Failed to decompress - lzrw1: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	printf( "SOURCE: %s\n", string );
	printf( "DEST:   %s\n", src );

	if ( strcmp( string, src ) != 0 ) {
		printf( "Decompressed result doesn't match original!\n" );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}