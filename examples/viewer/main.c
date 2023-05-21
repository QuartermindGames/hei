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
#include <plcore/pl_package.h>
#include <plcore/pl_console.h>

#include <plgraphics/plg.h>
#include <plgraphics/plg_camera.h>

#include <plmodel/plm.h>

#include <plwindow/plw.h>

#include "../shared.h"

#define TITLE "Model Viewer"

#define VERSION_MAJOR 0
#define VERSION_MINOR 3

#define WIDTH  800
#define HEIGHT 600

#define CENTER_X WIDTH / 2
#define CENTER_Y HEIGHT / 2

static PLGCamera *mainCamera = NULL;
static PLWWindow *mainWindow = NULL;

enum {
	VIEW_MODE_LIT,
	VIEW_MODE_WIREFRAME,
	VIEW_MODE_POINTS,
	VIEW_MODE_WEIGHTS,
	VIEW_MODE_SKELETON
};
static unsigned int viewMode = VIEW_MODE_LIT;

static void ProcessKeyboard( void ) {
#if 0
	const uint8_t *state = SDL_GetKeyboardState(NULL);
    if(state[SDL_SCANCODE_1]) {
        view_mode = VIEW_MODE_LIT;
    } else if(state[SDL_SCANCODE_2]) {
        view_mode = VIEW_MODE_WIREFRAME;
    } else if(state[SDL_SCANCODE_3]) {
        view_mode = VIEW_MODE_POINTS;
    } else if(state[SDL_SCANCODE_4]) {
        view_mode = VIEW_MODE_WEIGHTS;
    } else if( state[ SDL_SCANCODE_5 ] ) {
        view_mode = VIEW_MODE_SKELETON;
    }

    if( state[ SDL_SCANCODE_A ] || state[ SDL_SCANCODE_LEFT ] ) {
        main_camera->angles.x -= 4.f;
    } else if( state[ SDL_SCANCODE_D ] || state[ SDL_SCANCODE_RIGHT ] ) {
        main_camera->angles.x += 4.f;
    }

    PLVector3 left, up, forward;
    plAnglesAxes( main_camera->angles, &left, &up, &forward );
    if( state[ SDL_SCANCODE_W ] || state[ SDL_SCANCODE_UP ] ) {
        main_camera->position = plVector3Add( main_camera->position, main_camera->forward );
        //main_camera->position = plVector3Scale(main_camera->position, PLVector3(0.5f, 0.5f, 4.f));
    } else if( state[ SDL_SCANCODE_D ] || state[ SDL_SCANCODE_DOWN ] ) {
        main_camera->position = plVector3Subtract( main_camera->position, main_camera->forward );
        //main_camera->position = plVector3Scale(main_camera->position, PLVector3(4.f, 4.f, 4.f));
    }

    static unsigned int toggle_delay = 0;
    if( toggle_delay == 0 ) {
        if( state[ SDL_SCANCODE_Q ] ) {
            use_mouse_look = !use_mouse_look;
            SDL_ShowCursor( !use_mouse_look );
            main_camera->position = PLVector3( 0, 2, -50 );
            main_camera->angles = PLVector3( 0, 0, 0 );

            toggle_delay = 10;
        }

        if( state[ SDL_SCANCODE_C ] ) {
            static bool cull = false;
            if( cull ) {
                plSetCullMode( PL_CULL_NONE );
            } else {
                plSetCullMode( PL_CULL_NEGATIVE );
            }

            toggle_delay = 10;
        }
    } else {
        toggle_delay--;
    }

    if( state[ SDL_SCANCODE_ESCAPE ] ) {
        exit( EXIT_SUCCESS );
    }
#endif
}

// loads a model in and then frees it
static void TempModelLoad( const char *path, PL_UNUSED void *userData ) {
	PLMModel *model = PlmLoadModel( path );
	if ( model != NULL ) {
		PlmDestroyModel( model );
	}
}

int main( int argc, char **argv ) {
	PRINT( "\n " TITLE " : Version %d.%d (" __DATE__ ")\n", VERSION_MAJOR, VERSION_MINOR );
	PRINT( " Developed by...\n" );
	PRINT( "   Mark \"hogsy\" Sowden <hogsy@oldtimes-software.com>\n" );
	PRINT( "\n" );
	PRINT( " Usage:\n" );
	PRINT( "  Left   - rotate model\n" );
	PRINT( "  Right  - move model backward / forward\n" );
	PRINT( "  Middle - move model up, down, left and right\n\n" );
	PRINT( "  WASD   - move camera\n" );
	PRINT( "\n-------------------------------------------------------------------------\n\n" );

	if ( PlInitialize( argc, argv ) != PL_RESULT_SUCCESS ) {
		PRINT( "Failed to initialize plcore: %s\n", PlGetError() );
		return EXIT_FAILURE;
	}

	PlSetupLogOutput( "viewer.log" );

	PlRegisterStandardPackageLoaders();
	PlRegisterStandardImageLoaders( PL_IMAGE_FILEFORMAT_ALL );
	PlmRegisterStandardModelLoaders( PLM_MODEL_FILEFORMAT_ALL );

	if ( argc < 2 ) {
		PRINT( " viewer -<optional mode> <model path>\n" );
		PRINT( "  -smd    : write model out to an SMD\n" );
		PRINT( "  -scan   : scans through a directory, must provide extension as follow-up argument to model path\n" );
		return EXIT_SUCCESS;
	}

	char model_path[ PL_SYSTEM_MAX_PATH ] = { '\0' };
	char model_extension[ 12 ] = { '\0' };

	bool exportModel = false;
	bool scanDirectory = false;
	bool useMouseLook = false;

	if ( PlHasCommandLineArgument( "-smd" ) ) {
		exportModel = true;
	} else if ( PlHasCommandLineArgument( "-scan" ) ) {
		scanDirectory = true;
	}

	for ( int i = 1; i < argc; ++i ) {
		if ( argv[ i ] == NULL || argv[ i ][ 0 ] == '\0' ) {
			continue;
		}

		if ( argv[ i ][ 0 ] != '-' ) {// probably model path, probably
			if ( model_path[ 0 ] == '\0' ) {
				strncpy( model_path, argv[ i ], sizeof( model_path ) );
			} else {// probably extension, probably
				strncpy( model_extension, argv[ i ], sizeof( model_extension ) );
			}
		}
	}

	if ( model_path[ 0 ] == '\0' ) {
		printf( "invalid path for model, aborting!\n" );
		return EXIT_FAILURE;
	}

	if ( scanDirectory ) {
		if ( model_extension[ 0 ] == '\0' ) {
			printf( "invalid extension for scan, aborting!\n" );
			return EXIT_FAILURE;
		}

		PlScanDirectory( model_path, model_extension, TempModelLoad, false, NULL );
		return EXIT_SUCCESS;
	}

	mainWindow = PlwCreateWindow( TITLE, WIDTH, HEIGHT );
	if ( mainWindow == NULL ) {
		PRINT_ERROR( "Failed to create window: %s\n", PlwGetLastErrorMessage() );
	}

	PlgInitializeGraphics();

	PLMModel *model = PlmLoadModel( model_path );
	if ( model == NULL ) {
		PRINT_ERROR( "Failed to load model \"%s\"!\n%s", model_path, PlGetError() );
	}

	if ( exportModel ) {
		if ( !PlmWriteModel( "output", model, PLM_MODEL_OUTPUT_SMD ) ) {
			PRINT_ERROR( "Failed to write model \"%s\"!\n%s\n", model->name, PlGetError() );
		}
		return EXIT_SUCCESS;
	}

	PlgSetClearColour( PLColour( 0, 0, 128, 255 ) );

	mainCamera = PlgCreateCamera();
	if ( mainCamera == NULL ) {
		PRINT_ERROR( "Failed to create camera!\n" );
	}
	mainCamera->mode = PLG_CAMERA_MODE_PERSPECTIVE;
	mainCamera->fov = 75.0f;
	mainCamera->position = PLVector3( 0, 2, -50 );

	PlgSetDepthBufferMode( PLG_DEPTHBUFFER_ENABLE );
	PlgSetCullMode( PLG_CULL_NONE );

	PLGLight lights[ 4 ];
	memset( &lights, 0, sizeof( PLGLight ) * 4 );
	lights[ 0 ].position = PLVector3( 0, 0, 0 );
	lights[ 0 ].colour = PlCreateColour4F( 1.5f, .5f, .5f, 128.f );
	lights[ 0 ].type = PLG_LIGHT_TYPE_OMNI;

	/* compile shaders */

	const char *vertex_stage = {
	        "void main() {"
	        "   gl_Position = ftransform();"
	        "}" };

	const char *fragment_stage = {
	        "void main() {"
	        "   gl_FragColor = vec4(1,1,1,1);"
	        "}" };

	PLGShaderProgram *program = PlgCreateShaderProgram();
	PlgRegisterShaderStageFromMemory( program, vertex_stage, strlen( vertex_stage ), PLG_SHADER_TYPE_VERTEX );
	PlgRegisterShaderStageFromMemory( program, fragment_stage, strlen( fragment_stage ), PLG_SHADER_TYPE_FRAGMENT );

	PlgLinkShaderProgram( program );
	PlgSetShaderProgram( program );

	/* done, now for main rendering loop! */

	while ( true ) {
		static PLVector3 object_angles = { 0, 0 };

#if 0 /* todo: windowing... */
		SDL_PumpEvents();

		// input handlers start..
		int xpos, ypos;
		unsigned int state = SDL_GetMouseState( &xpos, &ypos );

		if ( useMouseLook ) {
			object_angles = PLVector3( 0, 0, 0 );

			int n_pos[ 2 ] = { xpos - CENTER_X, ypos - CENTER_Y };
			mainCamera->angles.x += ( float ) n_pos[ 0 ] / 10;
			mainCamera->angles.y += ( float ) n_pos[ 1 ] / 10;
			mainCamera->angles.y = plClamp( -90, mainCamera->angles.y, 90 );

			SDL_WarpMouseInWindow( window, CENTER_X, CENTER_Y );
		} else {
			// Camera rotation
			static double old_left_pos[ 2 ] = { 0, 0 };
			if ( state & SDL_BUTTON( SDL_BUTTON_LEFT ) ) {
				double n_x_pos = xpos - old_left_pos[ 0 ];
				double n_y_pos = ypos - old_left_pos[ 1 ];
				object_angles.x += ( float ) n_x_pos / 50;
				object_angles.y += ( float ) n_y_pos / 50;
			} else {
				old_left_pos[ 0 ] = xpos;
				old_left_pos[ 1 ] = ypos;
			}

			// Zoom in and out thing...
			static int old_right_pos[ 2 ] = { 0, 0 };
			if ( state & SDL_BUTTON( SDL_BUTTON_RIGHT ) ) {
				int n_y_pos = ypos - old_right_pos[ 1 ];
				mainCamera->position.z += ( float ) n_y_pos / 100;
			} else {
				old_right_pos[ 0 ] = xpos;
				old_right_pos[ 1 ] = ypos;
			}

			// panning thing
			static int old_middle_pos[ 2 ] = { 0, 0 };
			if ( state & SDL_BUTTON( SDL_BUTTON_MIDDLE ) ) {
				int n_x_pos = xpos - old_middle_pos[ 0 ];
				int n_y_pos = ypos - old_middle_pos[ 1 ];
				mainCamera->position.y += ( float ) n_y_pos / 50;
				mainCamera->position.x -= ( float ) n_x_pos / 50;
			} else {
				old_middle_pos[ 0 ] = xpos;
				old_middle_pos[ 1 ] = ypos;
			}
		}
		// input handlers end...
#endif

		ProcessKeyboard();

		PlgSetupCamera( mainCamera );

		PlgClearBuffers( PLG_BUFFER_COLOUR | PLG_BUFFER_DEPTH );

		PlLoadIdentityMatrix();
		PlPushMatrix();
		PlRotateMatrix( object_angles.y, 1, 0, 0 );
		PlRotateMatrix( object_angles.x, 0, 1, 0 );
		PlRotateMatrix( object_angles.z, 0, 0, 1 );

		switch ( viewMode ) {
			default:
				break;

			case VIEW_MODE_LIT:
			case VIEW_MODE_WEIGHTS:
			case VIEW_MODE_WIREFRAME:
				PlmDrawModel( model );
				break;

			case VIEW_MODE_SKELETON:
				PlmDrawModelSkeleton( model );
				break;
		}

		PlwSwapBuffers( mainWindow );
	}

	PlmDestroyModel( model );
	PlgDestroyCamera( mainCamera );

	PlgShutdownGraphics();
	PlShutdown();

	return EXIT_SUCCESS;
}