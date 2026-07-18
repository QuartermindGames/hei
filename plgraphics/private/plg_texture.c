// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>
// Purpose: Texture API
// Author:  Mark E. Sowden

#include "plg_private.h"

static void texture_destroy( void *ptr )
{
	CallGfxFunction( DeleteTexture, ptr );
}

QmGfxTexture *qm_gfx_texture_create( const QmGfxTextureType type )
{
	QmGfxTexture *texture = QM_OS_MEMORY_NEW_D( QmGfxTexture, texture_destroy );
	texture->type         = type;
	texture->format       = PL_IMAGEFORMAT_RGBA8;
	texture->w            = 8;
	texture->h            = 8;

	CallGfxFunction( CreateTexture, texture );
	return texture;
}

void qm_gfx_texture_set( const QmGfxTexture *self, unsigned int tmu )
{
	CallGfxFunction( BindTexture, self, tmu );
}

// todo, hook this up with var
void qm_gfx_texture_set_anisotropy( QmGfxTexture *texture, unsigned int amount )
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

void qm_gfx_texture_set_flags( QmGfxTexture *self, unsigned int flags )
{
	self->flags = flags;
}

bool qm_gfx_texture_upload( QmGfxTexture *self, const QmImage *upload )
{
	self->w      = upload->width;
	self->h      = upload->height;
	self->format = upload->format;
	self->pixel  = upload->colour_format;
	self->size   = upload->size;
	self->levels = upload->levels;

	if ( self->flags & PLG_TEXTURE_FLAG_NOMIPS &&
	     self->filter != PLG_TEXTURE_FILTER_LINEAR && self->filter != PLG_TEXTURE_FILTER_NEAREST )
	{
		GfxLog( "Invalid filter mode for texture - if specifying nomips, use either linear or nearest!" );
		self->filter = PLG_TEXTURE_FILTER_NEAREST;
	}

	CallGfxFunction( UploadTexture, self, upload );

	return true;
}
