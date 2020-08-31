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

#include <PL/platform.h>
#include <PL/platform_console.h>
#include <PL/platform_image.h>

/**
 * Command line utility to interface with the platform lib.
 **/

static void ConvertImage( const char *path, const char *destination ) {
	PLImage *image = plLoadImage( path );
	if ( image == NULL ) {
		printf( "Error: %s\n", plGetError() );
		return;
	}

	/* ensure it's a valid format before we write it out */
	plConvertPixelFormat( image, PL_IMAGEFORMAT_RGBA8 );

	if ( plWriteImage( image, destination ) ) {
		printf( "Wrote \"%s\"\n", destination );
	} else {
		printf( "Error: %s\n", plGetError() );
	}

	plDestroyImage( image );
}

static void ConvertImageCallback( const char *path, void *userData ) {
	char *outDir = ( char * ) userData;

	if ( !plCreateDirectory( outDir ) ) {
		printf( "Error: %s\n", plGetError() );
		return;
	}

	const char *fileName = plGetFileName( path );
	if ( fileName == NULL ) {
		printf( "Error: %s\n", plGetError() );
		return;
	}

	char outPath[ PL_SYSTEM_MAX_PATH ];
	snprintf( outPath, sizeof( outPath ), "%s%s.png", outDir, fileName );

	ConvertImage( path, outPath );
}

static void Cmd_IMGConvert( unsigned int argc, char **argv ) {
	if ( argc < 2 ) {
		return;
	}

	const char *outPath = "./out.png";
	if ( argc >= 3 ) {
		outPath = argv[ 2 ];
	}

	ConvertImage( argv[ 1 ], outPath );
}

static void Cmd_IMGBulkConvert( unsigned int argc, char **argv ) {
	if ( argc < 3 ) {
		return;
	}

	char outDir[ PL_SYSTEM_MAX_PATH ];
	if ( argc >= 4 ) {
		snprintf( outDir, sizeof( outDir ), "%s/", argv[ 3 ] );
	} else {
		snprintf( outDir, sizeof( outDir ), "out/" );
	}

	plScanDirectory( argv[ 1 ], argv[ 2 ], ConvertImageCallback, false, outDir );

	printf( "Done!\n" );
}

static bool isRunning = true;

static void Cmd_Exit( unsigned int argc, char **argv ) {
	/* we don't use these here, and ommitting var names is a C2x feature :( */
	(void)( argc );
	(void)( argv );

	isRunning = false;
}

#define MAX_COMMAND_LENGTH 256
static char cmdLine[ MAX_COMMAND_LENGTH ];
int main( int argc, char **argv ) {
#if defined( _WIN32 )
	/* stop buffering stdout! */
	setvbuf( stdout, NULL, _IONBF, 0 );
#endif

	plInitialize( argc, argv );

	plRegisterStandardImageLoaders( PL_IMAGE_FILEFORMAT_ALL );

	//plRegisterPlugins( "plugins/" );

	/* register all our custom console commands */
	plRegisterConsoleCommand( "exit", Cmd_Exit, "Exit the application." );
	plRegisterConsoleCommand( "quit", Cmd_Exit, "Exit the application." );
	plRegisterConsoleCommand( "img_convert", Cmd_IMGConvert,
	                          "Convert the given image.\n"
	                          "Usage: img_convert ./image.bmp [./out.png]" );
	plRegisterConsoleCommand( "img_bulkconvert", Cmd_IMGBulkConvert,
	                          "Bulk convert images in the given directory.\n"
	                          "Usage: img_bulkconvert ./path bmp [./outpath]" );

	while( isRunning ) {
		printf( "> " );

		int i;
		char *p = cmdLine;
		while ( ( i = getchar() ) != '\n' ) {
			*p++ = ( char ) i;

			unsigned int numChars = p - cmdLine;
			if ( numChars >= MAX_COMMAND_LENGTH ) {
				printf( "Hit character limit!\n" );
				break;
			}
		}

		plParseConsoleString( cmdLine );

		memset( cmdLine, 0, sizeof( cmdLine ) );
	}

	plShutdown();

	return EXIT_SUCCESS;
}
