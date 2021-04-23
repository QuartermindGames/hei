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

#include "plg_private.h"

void _InitTextures( void ) {
	gfx_state.tmu = ( PLGTextureMappingUnit * ) pl_calloc( PlgGetMaxTextureUnits(), sizeof( PLGTextureMappingUnit ) );
	for ( unsigned int i = 0; i < PlgGetMaxTextureUnits(); i++ ) {
		gfx_state.tmu[ i ].current_envmode = PLG_TEXTUREMODE_REPLACE;
	}

	gfx_state.max_textures = 1024;
	gfx_state.textures = ( PLGTexture ** ) pl_malloc( sizeof( PLGTexture * ) * gfx_state.max_textures );
	memset( gfx_state.textures, 0, sizeof( PLGTexture * ) * gfx_state.max_textures );
	gfx_state.num_textures = 0;

	PlRegisterConsoleVariable( NULL, NULL, pl_float_var, 0, NULL );
}

void PlgShutdownTextures( void ) {
	if ( gfx_state.tmu ) {
		pl_free( gfx_state.tmu );
	}

	if ( gfx_state.textures ) {
		for ( PLGTexture **texture = gfx_state.textures;
			  texture < gfx_state.textures + gfx_state.num_textures; ++texture ) {
			if ( ( *texture ) ) {
				PlgDestroyTexture( ( *texture ) );
			}
		}
		pl_free( gfx_state.textures );
	}
}

unsigned int PlgGetMaxTextureSize( void ) {
	if ( gfx_state.hw_maxtexturesize != 0 ) {
		return gfx_state.hw_maxtexturesize;
	}

	CallGfxFunction( GetMaxTextureSize, &gfx_state.hw_maxtexturesize );

	return gfx_state.hw_maxtexturesize;
}

PLGTexture *PlgCreateTexture( void ) {
	PLGTexture *texture = pl_calloc( 1, sizeof( PLGTexture ) );
	if ( texture == NULL ) {
		return NULL;
	}

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

	pl_free( texture );
}

/**
 * Automatically loads in an image and uploads it as a texture.
 */
PLGTexture *PlgLoadTextureFromImage( const char *path, PLGTextureFilter filter_mode ) {
	PLImage *image = PlLoadImage( path );
	if ( image == NULL ) {
		return NULL;
	}

	PLGTexture *texture = PlgCreateTexture();
	if ( texture != NULL ) {
		/* store the path, so we can easily reload the image if we need to */
		strncpy( texture->path, image->path, sizeof( texture->path ) );
	
		texture->filter = filter_mode;

		PlgUploadTextureImage( texture, image );
	}

	PlDestroyImage( image );

	return texture;
}

/////////////////////////////////////////////////////

PLGTexture *plGetCurrentTexture( unsigned int tmu ) {
	for ( PLGTexture **texture = gfx_state.textures; texture < gfx_state.textures + gfx_state.num_textures; ++texture ) {
		if ( gfx_state.tmu[ tmu ].current_texture == ( *texture )->internal.id ) {
			return ( *texture );
		}
	}
	return NULL;
}

unsigned int PlgGetMaxTextureUnits( void ) {
	if ( gfx_state.hw_maxtextureunits != 0 ) {
		return gfx_state.hw_maxtextureunits;
	}

	CallGfxFunction( GetMaxTextureUnits, &gfx_state.hw_maxtextureunits );
	return gfx_state.hw_maxtextureunits;
}

unsigned int plGetCurrentTextureUnit( void ) {
	return gfx_state.current_textureunit;
}

void PlgSetTexture( PLGTexture *texture, unsigned int tmu ) {
	PlgSetTextureUnit( tmu );

	if ( ( gfx_state.textures[ tmu ] != NULL ) && ( gfx_state.textures[ tmu ] == texture ) ) {
		return;
	}

	CallGfxFunction( BindTexture, texture );

	gfx_state.textures[ tmu ] = texture;

	PlgSetTextureUnit( 0 );
}

void PlgSetTextureUnit( unsigned int target ) {
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

unsigned int PlgGetMaxTextureAnistropy( void ) {
	if ( gfx_state.hw_maxtextureanistropy != 0 ) {
		return gfx_state.hw_maxtextureanistropy;
	}

#if defined (PL_MODE_OPENGL)
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, (GLfloat*)&gfx_state.hw_maxtextureanistropy);
#else
	gfx_state.hw_maxtextureanistropy = 1;
#endif

	return gfx_state.hw_maxtextureanistropy;
}

// todo, hook this up with var
void PlgSetTextureAnisotropy( PLGTexture *texture, unsigned int amount ) {
	CallGfxFunction( SetTextureAnisotropy, texture, amount );
}

void _plBindTexture( const PLGTexture *texture ) {
	// allow us to pass null texture instances
	// as it will give us an opportunity to unbind
	// them on the GPU upon request
	unsigned int id = 0;
	if ( texture != NULL ) {
		id = texture->internal.id;
	}

	PLGTextureMappingUnit *unit = &gfx_state.tmu[ plGetCurrentTextureUnit() ];
	if ( id == unit->current_texture ) {
		return;
	}

	CallGfxFunction( BindTexture, texture );

	unit->current_texture = id;
}

void PlgSetTextureFlags( PLGTexture *texture, unsigned int flags ) {
	texture->flags = flags;
}

void PlgSetTextureEnvironmentMode( PLGTextureEnvironmentMode mode ) {
	if ( gfx_state.tmu[ plGetCurrentTextureUnit() ].current_envmode == mode )
		return;

#if defined(PL_MODE_OPENGL) && !defined(PL_MODE_OPENGL_CORE)
	glTexEnvi
			(
					GL_TEXTURE_ENV,
					GL_TEXTURE_ENV_MODE,
					_plTranslateTextureEnvironmentMode(mode)
			);
#elif defined(PL_MODE_OPENGL_CORE)
	// todo
#endif

	gfx_state.tmu[ plGetCurrentTextureUnit() ].current_envmode = mode;
}

/////////////////////

bool PlgUploadTextureImage( PLGTexture *texture, const PLImage *upload ) {
	plAssert( texture );

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
		strncpy( texture->name, file_name, sizeof( texture->name ) );
	}

	_plBindTexture( texture );
	CallGfxFunction( UploadTexture, texture, upload );
	_plBindTexture( NULL );

	return true;
}

void plSwizzleTexture( PLGTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
	CallGfxFunction( SwizzleTexture, texture, r, g, b, a );
}