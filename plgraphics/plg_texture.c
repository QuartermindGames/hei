/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plgraphics/plg_driver_interface.h>

#include "plg_private.h"
#include "qmos/public/qm_os_memory.h"

static void CheckTMUStates( void ) {
	if ( gfx_state.tmu != NULL ) {
		return;
	}

	gfx_state.tmu = ( PLGTextureMappingUnit * ) QM_OS_MEMORY_CALLOC( PlgGetMaxTextureUnits(), sizeof( PLGTextureMappingUnit ) );
	for ( unsigned int i = 0; i < PlgGetMaxTextureUnits(); i++ ) {
		gfx_state.tmu[ i ].current_envmode = PLG_TEXTUREMODE_REPLACE;
	}
}

void PlgShutdownTextures( void ) {
	qm_os_memory_free( gfx_state.tmu );
}

/* todo: move into generic GET handler */
unsigned int PlgGetMaxTextureSize( void ) {
	if ( gfx_state.hw_maxtexturesize != 0 ) {
		return gfx_state.hw_maxtexturesize;
	}

	CallGfxFunction( GetMaxTextureSize, &gfx_state.hw_maxtexturesize );

	return gfx_state.hw_maxtexturesize;
}

PLGTexture *PlgCreateTexture( void ) {
	PLGTexture *texture = QM_OS_MEMORY_CALLOC( 1, sizeof( PLGTexture ) );
	texture->format = PL_IMAGEFORMAT_RGBA8;
	texture->w = 8;
	texture->h = 8;

	CallGfxFunction( CreateTexture, texture );
	return texture;
}

void PlgDestroyTexture( PLGTexture *texture ) {
	if ( texture == NULL ) {
		return;
	}

	CallGfxFunction( DeleteTexture, texture );

	qm_os_memory_free( texture );
}

/* todo: move into generic GET handler */
unsigned int PlgGetMaxTextureUnits( void ) {
	if ( gfx_state.hw_maxtextureunits != 0 ) {
		return gfx_state.hw_maxtextureunits;
	}

	CallGfxFunction( GetMaxTextureUnits, &gfx_state.hw_maxtextureunits );
	return gfx_state.hw_maxtextureunits;
}

unsigned int PlgGetCurrentTextureUnit( void ) {
	return gfx_state.current_textureunit;
}

static void SetTextureUnit( unsigned int target ) {
	if ( target == gfx_state.current_textureunit ) {
		return;
	}

	if ( target > PlgGetMaxTextureUnits() ) {
		GfxLog( "Attempted to select a texture image unit beyond what's supported by your hardware! (%i)\n",
		        target );
		return;
	}

	CallGfxFunction( ActiveTexture, target );

	gfx_state.current_textureunit = target;
}

void PlgSetTexture( PLGTexture *texture, unsigned int tmu ) {
	SetTextureUnit( tmu );

	CallGfxFunction( BindTexture, texture );

	SetTextureUnit( 0 );
}

/* todo: move into generic GET handler */
unsigned int PlgGetMaxTextureAnistropy( void ) {
	if ( gfx_state.hw_maxtextureanistropy != 0 ) {
		return gfx_state.hw_maxtextureanistropy;
	}

#if defined( PL_MODE_OPENGL )
	glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, ( GLfloat * ) &gfx_state.hw_maxtextureanistropy );
#else
	gfx_state.hw_maxtextureanistropy = 1;
#endif

	return gfx_state.hw_maxtextureanistropy;
}

// todo, hook this up with var
void PlgSetTextureAnisotropy( PLGTexture *texture, unsigned int amount ) {
	CallGfxFunction( SetTextureAnisotropy, texture, amount );
}

void PlgSetTextureFilter( PLGTexture *texture, PLGTextureFilter filter ) {
	if ( texture->filter == filter ) {
		return;
	}

	CallGfxFunction( SetTextureFilter, texture, filter );
}

void PlgSetTextureWrapMode( PLGTexture *texture, PLGTextureWrapMode wrapMode ) {
	if ( texture->wrapMode == wrapMode ) {
		return;
	}

	CallGfxFunction( SetTextureWrapMode, texture, wrapMode );
}

/* todo: kill this, favor SetTexture */
static void BindTexture( const PLGTexture *texture ) {
	unsigned int unitIndex = PlgGetCurrentTextureUnit();
	if ( unitIndex >= PlgGetMaxTextureUnits() ) {
		/* todo: should we let user know ... ? */
		return;
	}

	CallGfxFunction( BindTexture, texture );
}

void PlgSetTextureFlags( PLGTexture *texture, unsigned int flags ) {
	texture->flags = flags;
}

void PlgSetTextureEnvironmentMode( PLGTextureEnvironmentMode mode ) {
	if ( gfx_state.tmu[ PlgGetCurrentTextureUnit() ].current_envmode == mode )
		return;

#if defined( PL_MODE_OPENGL ) && !defined( PL_MODE_OPENGL_CORE )
	glTexEnvi(
	        GL_TEXTURE_ENV,
	        GL_TEXTURE_ENV_MODE,
	        _plTranslateTextureEnvironmentMode( mode ) );
#elif defined( PL_MODE_OPENGL_CORE )
	// todo
#endif

	gfx_state.tmu[ PlgGetCurrentTextureUnit() ].current_envmode = mode;
}

/////////////////////

bool PlgUploadTextureImage( PLGTexture *texture, const PLImage *upload ) {
	assert( texture );

	texture->w = upload->width;
	texture->h = upload->height;
	texture->format = upload->format;
	texture->pixel = upload->colour_format;
	texture->size = upload->size;
	texture->levels = upload->levels;

	/* store the path, so we can easily reload the image if we need to */
	strncpy( texture->path, upload->path, sizeof( texture->path ) );

	if ( texture->flags & PLG_TEXTURE_FLAG_NOMIPS &&
	     ( texture->filter != PLG_TEXTURE_FILTER_LINEAR && texture->filter != PLG_TEXTURE_FILTER_NEAREST ) ) {
		GfxLog( "Invalid filter mode for texture - if specifying nomips, use either linear or nearest!" );
		texture->filter = PLG_TEXTURE_FILTER_NEAREST;
	}

	const char *file_name = PlGetFileName( upload->path );
	if ( file_name == NULL || file_name[ 0 ] == '\0' ) {
		strncpy( texture->name, "null", sizeof( texture->name ) );
	} else {
		snprintf( texture->name, sizeof( texture->name ), "%s", file_name );
	}

	BindTexture( texture );
	CallGfxFunction( UploadTexture, texture, upload );
	BindTexture( NULL );

	return true;
}

void PlgSwizzleTexture( PLGTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
	CallGfxFunction( SwizzleTexture, texture, r, g, b, a );
}