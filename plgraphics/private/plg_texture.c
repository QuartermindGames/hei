// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>
// Purpose: Texture API
// Author:  Mark E. Sowden

#include "plg_private.h"

static void texture_destroy( void *ptr )
{
	CallGfxFunction( DeleteTexture, ptr );
}

QmGfxTexture *qm_gfx_texture_create()
{
	QmGfxTexture *texture = QM_OS_MEMORY_NEW_D( QmGfxTexture, texture_destroy );
	texture->format       = PL_IMAGEFORMAT_RGBA8;
	texture->w            = 8;
	texture->h            = 8;

	CallGfxFunction( CreateTexture, texture );
	return texture;
}

static void set_texture_unit( unsigned int target )
{
	if ( target == gfx_state.current_textureunit )
	{
		return;
	}

	if ( target > qm_gfx_get_max_texture_units() )
	{
		GfxLog( "Attempted to select a texture image unit beyond what's supported by your hardware! (%i)\n",
		        target );
		return;
	}

	CallGfxFunction( ActiveTexture, target );

	gfx_state.current_textureunit = target;
}

void qm_gfx_texture_set( const QmGfxTexture *self, unsigned int tmu )
{
	set_texture_unit( tmu );

	CallGfxFunction( BindTexture, self );

	set_texture_unit( 0 );
}

// todo, hook this up with var
void PlgSetTextureAnisotropy( QmGfxTexture *texture, unsigned int amount )
{
	CallGfxFunction( SetTextureAnisotropy, texture, amount );
}

void qm_gfx_texture_set_filter( QmGfxTexture *self, QmGfxTextureFilter filter )
{
	if ( self->filter == filter )
	{
		return;
	}

	CallGfxFunction( SetTextureFilter, self, filter );
}

void qm_gfx_texture_set_wrap_mode( QmGfxTexture *self, QmGfxTextureWrapMode wrapMode )
{
	if ( self->wrapMode == wrapMode )
	{
		return;
	}

	CallGfxFunction( SetTextureWrapMode, self, wrapMode );
}

/* todo: kill this, favor SetTexture */
static void BindTexture( const QmGfxTexture *texture )
{
	unsigned int unitIndex = gfx_state.current_textureunit;
	if ( unitIndex >= qm_gfx_get_max_texture_units() )
	{
		/* todo: should we let user know ... ? */
		return;
	}

	CallGfxFunction( BindTexture, texture );
}

void qm_gfx_texture_set_flags( QmGfxTexture *self, unsigned int flags )
{
	self->flags = flags;
}

bool qm_gfx_texture_upload( QmGfxTexture *self, const PLImage *upload )
{
	self->w      = upload->width;
	self->h      = upload->height;
	self->format = upload->format;
	self->pixel  = upload->colour_format;
	self->size   = upload->size;
	self->levels = upload->levels;

	/* store the path, so we can easily reload the image if we need to */
	strncpy( self->path, upload->path, sizeof( self->path ) );

	if ( self->flags & PLG_TEXTURE_FLAG_NOMIPS &&
	     self->filter != PLG_TEXTURE_FILTER_LINEAR && self->filter != PLG_TEXTURE_FILTER_NEAREST )
	{
		GfxLog( "Invalid filter mode for texture - if specifying nomips, use either linear or nearest!" );
		self->filter = PLG_TEXTURE_FILTER_NEAREST;
	}

	BindTexture( self );

	CallGfxFunction( UploadTexture, self, upload );

	BindTexture( nullptr );

	return true;
}

void qm_gfx_texture_swizzle( QmGfxTexture *self, uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	CallGfxFunction( SwizzleTexture, self, r, g, b, a );
}
