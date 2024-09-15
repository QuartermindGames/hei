// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include <SDL2/SDL.h>

#include <plcore/pl.h>

#include <plgraphics/plg.h>
#include <plgraphics/plg_driver_interface.h>
#include <plgraphics/plg_camera.h>

// This is a very simple example of how to use the plgraphics API
// Mind that it's very barebones with minimal error checking!

#define WIDTH  800
#define HEIGHT 600

int main( int argc, char **argv ) {
	// core library needs to be initialised first
	PlInitialize( argc, argv );

	// graphics library can be initialised before window creation
	PlgInitializeGraphics();

	// scan and register available drivers
	PlgScanForDrivers( "./" );

	// we can use the below to determine all the available interfaces if we want to
#if 0
	unsigned int numInterfaces;
	const PLGDriverDescription **interfaces = PlgGetAvailableDriverInterfaces( &numInterfaces );
	assert( numInterfaces > 0 );
	for ( unsigned int i = 0; i < numInterfaces; ++i ) {
		printf( "%s\n", interfaces[ i ]->identifier );
	}
#endif

	// now we start init of some SDL2 crap for our window etc.
	SDL_Init( SDL_INIT_EVERYTHING );
	SDL_Window *window = SDL_CreateWindow( "Hei Triangle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL );
	SDL_GLContext context = SDL_GL_CreateContext( window );
	SDL_GL_MakeCurrent( window, context );

	// for now we'll just use the opengl interface; this needs to be called after window and context creation (for now)
	if ( PlgSetDriver( "opengl" ) != PL_RESULT_SUCCESS ) {
		printf( "Failed to set OpenGL driver: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	static const char *vertBuf =
	        "void main() {"
	        "   gl_Position = ( pl_proj * pl_view * pl_model ) * vec4( pl_vposition, 1.0 );"
	        "}";
	static const char *fragBuf =
	        "void main() {"
	        "	pl_frag = vec4(1.0, 0.0, 1.0, 1.0);"
	        "}";

	PLGShaderProgram *shaderProgram = PlgCreateShaderProgram();
	PlgAttachShaderStage( shaderProgram,
	                      PlgParseShaderStage( PLG_SHADER_TYPE_VERTEX, vertBuf, strlen( vertBuf ) ) );
	PlgAttachShaderStage( shaderProgram,
	                      PlgParseShaderStage( PLG_SHADER_TYPE_FRAGMENT, fragBuf, strlen( fragBuf ) ) );
	PlgLinkShaderProgram( shaderProgram );

	PlgSetShaderProgram( shaderProgram );

	PlgSetViewport( 0, 0, WIDTH, HEIGHT );
	PlgSetClearColour( &PL_COLOURF32RGB( 0.5f, 0.0f, 0.5f ) );

	PlgSetCullMode( PLG_CULL_NONE );

	unsigned int maxFrames = 2000;
	for ( unsigned int i = 0; i < maxFrames; ++i ) {
		printf( "frame %u\n", i );

		PlgClearBuffers( PLG_BUFFER_COLOUR | PLG_BUFFER_DEPTH );

		PlgDrawTriangle( 0, 0, WIDTH, HEIGHT );

		SDL_GL_SwapWindow( window );
	}

	SDL_GL_MakeCurrent( window, NULL );
	SDL_GL_DeleteContext( context );
	SDL_DestroyWindow( window );

	return EXIT_SUCCESS;
}
