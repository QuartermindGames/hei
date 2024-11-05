// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>
// Purpose: Example application for using console API

#include <plcore/pl_array_vector.h>
#include <plcore/pl_image.h>
#include <plcore/pl_package.h>
#include <plcore/pl_console.h>

#include <plmodel/plm.h>

#include "pl_extra_okre.h"

/**
 * Command line utility to interface with the platform lib.
 **/

#define Error( ... ) fprintf( stderr, __VA_ARGS__ )

static void ConvertImage( const char *path, const char *destination ) {
	PLImage *image = PlLoadImage( path );
	if ( image == NULL ) {
		printf( "Failed to load \"%s\"! (%s)\n", path, PlGetError() );
		return;
	}

	/* ensure it's a valid format before we write it out */
	PlConvertPixelFormat( image, PL_IMAGEFORMAT_RGBA8 );

	if ( PlWriteImage( image, destination, 100 ) ) {
		printf( "Wrote \"%s\"\n", destination );
	} else {
		printf( "Failed to write \"%s\"! (%s)\n", destination, PlGetError() );
	}

	PlDestroyImage( image );
}

static void ConvertImageCallback( const char *path, void *userData ) {
	char *outDir = ( char * ) userData;

	if ( !PlCreateDirectory( outDir ) ) {
		Error( "Error: %s\n", PlGetError() );
		return;
	}

	const char *fileName = PlGetFileName( path );
	if ( fileName == NULL ) {
		Error( "Error: %s\n", PlGetError() );
		return;
	}

	char tmp[ 128 ];
	PlStripExtension( tmp, sizeof( tmp ), fileName );

	char outPath[ PL_SYSTEM_MAX_PATH ];
	snprintf( outPath, sizeof( outPath ), "%s%s.png", outDir, tmp );

	ConvertImage( path, outPath );
}

static void ConvertImageCommand( unsigned int argc, char **argv ) {
	if ( argc < 2 ) {
		return;
	}

	const char *outPath = "./out.png";
	if ( argc >= 3 ) {
		outPath = argv[ 2 ];
	}

	ConvertImage( argv[ 1 ], outPath );
}

static void BulkConvertImagesCommand( unsigned int argc, char **argv ) {
	if ( argc < 3 ) {
		return;
	}

	char outDir[ PL_SYSTEM_MAX_PATH ];
	if ( argc >= 4 ) {
		snprintf( outDir, sizeof( outDir ), "%s/", argv[ 3 ] );
	} else {
		snprintf( outDir, sizeof( outDir ), "out/" );
	}

	bool recursive = false;
	for ( unsigned int i = 1; i < argc; ++i ) {
		if ( *argv[ i ] == '/' && *( argv[ i ] + 1 ) == 'r' ) {
			recursive = true;
		}
	}

	PlScanDirectory( argv[ 1 ], argv[ 2 ], ConvertImageCallback, recursive, outDir );

	printf( "Done!\n" );
}

static void ConvertModel( const char *path, const char *destination ) {
	PLMModel *model = PlmLoadModel( path );
	if ( model == NULL ) {
		printf( "Failed to load model: %s\n", PlGetError() );
		return;
	}

	PlmWriteModel( destination, model, PLM_MODEL_OUTPUT_SMD );

	PlmDestroyModel( model );
}

static void Cmd_ConvertModel( unsigned int argc, char **argv ) {
	if ( argc < 2 ) {
		return;
	}

	PLPath outPath;
	if ( argc >= 3 ) {
		snprintf( outPath, sizeof( outPath ), "%s", argv[ 2 ] );
	} else {
		const char *fn = PlGetFileName( argv[ 1 ] );
		size_t ns = strlen( fn );
		const char *fe = PlGetFileExtension( fn );
		size_t es = strlen( fe );

		char *name = PL_NEW_( char, ns );
		snprintf( name, ns - es, "%s", fn );
		snprintf( outPath, sizeof( outPath ), "./%s.smd", name );
		PL_DELETE( name );
	}

	ConvertModel( argv[ 1 ], outPath );
}

static void ConvertModelCallback( const char *path, void *userData ) {
	char *outDir = ( char * ) userData;

	if ( !PlCreateDirectory( outDir ) ) {
		Error( "Error: %s\n", PlGetError() );
		return;
	}

	const char *fn = PlGetFileName( path );
	size_t ns = strlen( fn );
	const char *fe = PlGetFileExtension( fn );
	size_t es = strlen( fe );

	char *name = PL_NEW_( char, ns );
	snprintf( name, ns - es, "%s", fn );
	char outPath[ PL_SYSTEM_MAX_PATH ];
	snprintf( outPath, sizeof( outPath ), "%s%s.smd", outDir, name );
	PL_DELETE( name );

	printf( "Converting %s to %s\n", path, outPath );

	ConvertModel( path, outPath );
}

static void Cmd_BulkConvertModel( unsigned int argc, char **argv ) {
	if ( argc < 3 ) {
		return;
	}

	char outDir[ PL_SYSTEM_MAX_PATH ];
	if ( argc >= 4 ) {
		snprintf( outDir, sizeof( outDir ), "%s/", argv[ 3 ] );
	} else {
		snprintf( outDir, sizeof( outDir ), "out/" );
	}

	PlScanDirectory( argv[ 1 ], argv[ 2 ], ConvertModelCallback, true, outDir );

	printf( "Done!\n" );
}

static void BulkExportCallback( const char *path, void *user ) {
	/* just throw it through the console command */
	char *tmp = pl_strjoin( "pkgext ", path );
	PlParseConsoleString( tmp );
	PL_DELETE( tmp );
}

static void Cmd_BulkExportPackages( unsigned int argc, char **argv ) {
	PLPath path;
	snprintf( path, sizeof( path ), "%s", argv[ 1 ] );
	if ( !PlPathExists( path ) ) {
		printf( "Path doesn't exist: %s\n", PlGetError() );
		return;
	}

	char extension[ 16 ];
	snprintf( extension, sizeof( extension ), "%s", argv[ 2 ] );
	if ( extension[ 0 ] == '\0' || extension[ 0 ] == '.' ) {
		printf( "Invalid extension provided; example 'clu'\n" );
		return;
	}

	PlScanDirectory( path, extension, BulkExportCallback, false, NULL );
}

static void list_packages_callback( const char *path, PL_UNUSED void *user ) {
	/* just throw it through the console command */
	char *tmp = pl_strjoin( "pkglst ", path );
	PlParseConsoleString( tmp );
	PL_DELETE( tmp );
}

static void list_packages_command( PL_UNUSED unsigned int argc, char **argv ) {
	PLPath path;
	snprintf( path, sizeof( path ), "%s", argv[ 1 ] );
	if ( !PlPathExists( path ) ) {
		printf( "Path doesn't exist: %s\n", PlGetError() );
		return;
	}

	char extension[ 16 ];
	snprintf( extension, sizeof( extension ), "%s", argv[ 2 ] );
	if ( extension[ 0 ] == '\0' || extension[ 0 ] == '.' ) {
		printf( "Invalid extension provided; example 'clu'\n" );
		return;
	}

	PlScanDirectory( path, extension, list_packages_callback, false, NULL );
}

static void Cmd_Exit( unsigned int argc, char **argv ) {
	/* we don't use these here, and ommitting var names is a C2x feature :( */
	( void ) ( argc );
	( void ) ( argv );

	PlShutdown();

	exit( 0 );
}

/*************************************************************/


/*************************************************************/

PL_NORETURN( static void MainLoop( void ) ) {
#define MAX_COMMAND_LENGTH 256
	static char cmdLine[ MAX_COMMAND_LENGTH ];
	while ( true ) {
		printf( "> " );

		int i;
		char *p = cmdLine;
		while ( ( i = getchar() ) != '\n' ) {
			*p++ = ( char ) i;

			unsigned int numChars = p - cmdLine;
			if ( numChars >= MAX_COMMAND_LENGTH - 1 ) {
				Error( "Hit character limit!\n" );
				break;
			}
		}

		PlParseConsoleString( cmdLine );

		PL_ZERO_( cmdLine );
	}
}

static void register_image_formats( void ) {
	PlRegisterStandardImageLoaders( PL_IMAGE_FILEFORMAT_ALL );

	PlRegisterImageLoader( "tga", PlParseOkreTexture );
	PlRegisterImageLoader( "bmDDS", PlParseOkreTexture );
}

int main( int argc, char **argv ) {
	/* no buffering stdout! */
	setvbuf( stdout, NULL, _IONBF, 0 );

	PlInitialize( argc, argv );
	PlInitializeSubSystems( PL_SUBSYSTEM_IO );

	PlmRegisterStandardModelLoaders( PLM_MODEL_FILEFORMAT_ALL );

	PlRegisterStandardPackageLoaders( PL_PACKAGE_LOAD_FORMAT_ALL );

	void PlxRegisterHavenPackageFormat( void );// package_haven_dat.c
	PlxRegisterHavenPackageFormat();

	PLPackage *IStorm_LST_LoadFile( const char *path );
	PlRegisterPackageLoader( "lst", IStorm_LST_LoadFile, NULL );

	PLPackage *AITD_PAK_LoadPackage( const char *path );
	PlRegisterPackageLoader( "pak", AITD_PAK_LoadPackage, NULL );

	PLPackage *FTactics_PAK_LoadFile( const char *path );
	PlRegisterPackageLoader( "pak", FTactics_PAK_LoadFile, NULL );

	PLPackage *Sentient_VSR_LoadFile( const char *path );
	PlRegisterPackageLoader( "vsr", Sentient_VSR_LoadFile, NULL );

	PLPackage *Mortyr_HAL_LoadFile( const char *path );
	PlRegisterPackageLoader( "hal", Mortyr_HAL_LoadFile, NULL );

	PLPackage *Eradicator_RID_LoadFile( const char *path );
	PlRegisterPackageLoader( "rid", Eradicator_RID_LoadFile, NULL );
	PlRegisterPackageLoader( "rim", Eradicator_RID_LoadFile, NULL );

	PLPackage *Outwars_FF_LoadFile( const char *path );
	PlRegisterPackageLoader( "ff", Outwars_FF_LoadFile, NULL );

	PLPackage *PlParseIce3DDatPackage( PLFile * file );
	PlRegisterPackageLoader( "dat", NULL, PlParseIce3DDatPackage );

	PLPackage *asa_format_tre_load( const char *path );
	PlRegisterPackageLoader( "tre", asa_format_tre_load, NULL );

	PlRegisterPackageLoader( "wad", NULL, PlParseOkreWadPackage );
	PlRegisterPackageLoader( "dir", NULL, PlParseOkreDirPackage );

	//PlRegisterPackageLoader( "wad", NULL, PlParseKriPackage );

	register_image_formats();

	PLPath exeDir;
	if ( PlGetExecutableDirectory( exeDir, sizeof( PLPath ) ) != NULL ) {
		PLPath pluginsDir;
		PlSetupPath( pluginsDir, true, PlPathEndsInSlash( exeDir ) ? "%splugins" : "%s/plugins", exeDir );
		PlRegisterPlugins( pluginsDir );
	}

	/* register all our custom console commands */
	PlRegisterConsoleCommand( "exit", "Exit the application.", 0, Cmd_Exit );
	PlRegisterConsoleCommand( "quit", "Exit the application.", 0, Cmd_Exit );
	PlRegisterConsoleCommand( "mdlconv",
	                          "Convert the specified image. 'mdlconv ./model.mdl [./out.mdl]'",
	                          1, Cmd_ConvertModel );
	PlRegisterConsoleCommand( "mdlconvdir",
	                          "Bulk convert images.",
	                          1, Cmd_BulkConvertModel );
	PlRegisterConsoleCommand( "imgconv",
	                          "Convert the given image. 'img_convert ./image.bmp [./out.png]'",
	                          -1, ConvertImageCommand );
	PlRegisterConsoleCommand( "imgconvdir",
	                          "Bulk convert images in the given directory. 'img_bulkconvert ./path bmp [./outpath]'",
	                          -1, BulkConvertImagesCommand );
	PlRegisterConsoleCommand( "export_packages",
	                          "Attempt to export from all packages found in directory. "
	                          "<directory> <extension>",
	                          2, Cmd_BulkExportPackages );
	PlRegisterConsoleCommand( "list_packages",
	                          "Attempts to bulk list the contents from all packages found in a directory.",
	                          2, list_packages_command );

	void asa_register_commands( void );
	asa_register_commands();

	PlInitializePlugins();

	/* allow us to just push a command via the command line if we want */
	const char *arg = PlGetCommandLineArgumentValue( "-cmd" );
	if ( arg != NULL ) {
		PlParseConsoleString( arg );
		PlShutdown();
		return EXIT_SUCCESS;
	}

	MainLoop();
}
