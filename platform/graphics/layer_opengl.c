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

#if defined(PL_SUPPORT_OPENGL)

#include <PL/platform_console.h>

#include "graphics_private.h"

#include <GL/glew.h>

#include <PL/platform_mesh.h>
#include <PL/platform_graphics.h>
#include <PL/platform_graphics_camera.h>
#include <PL/platform_graphics_texture.h>

#define DEBUG_GL

struct {
	bool generate_mipmap;
	bool depth_texture;
	bool shadow;
	bool vertex_buffer_object;
	bool texture_compression;
	bool texture_compression_s3tc;
	bool multitexture;
	bool texture_env_combine;
	bool texture_env_add;
	bool vertex_program;
	bool fragment_program;
	bool direct_state_access;
} gl_capabilities;

static int gl_version_major = 0;
static int gl_version_minor = 0;

#define GLVersion( maj, min ) (((maj) == gl_version_major && (min) <= gl_version_minor) || (maj) < gl_version_major)

unsigned int gl_num_extensions = 0;

static GLuint VAO[1];

///////////////////////////////////////////
// Debug

static void GLInsertDebugMarker( const char *msg ) {
	if ( !GLVersion( 4, 3 ) ) {
		return;
	}

	glDebugMessageInsert( GL_DEBUG_SOURCE_APPLICATION,
						  GL_DEBUG_TYPE_MARKER,
						  0,
						  GL_DEBUG_SEVERITY_NOTIFICATION,
						  -1,
						  msg );
}

static void GLPushDebugGroupMarker( const char *msg ) {
	if ( !GLVersion( 4, 3 ) ) {
		return;
	}

	glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0, -1, msg );
}

static void GLPopDebugGroupMarker( void ) {
	if ( !GLVersion( 4, 3 ) ) {
		return;
	}

	glPopDebugGroup();
}

static void ClearBoundTextures( void ) {
	for ( unsigned int i = 0; i < gfx_state.hw_maxtextureunits; ++i ) {
		glActiveTexture( GL_TEXTURE0 + i );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
	glActiveTexture( GL_TEXTURE0 );
}

static void ClearBoundBuffers( void ) {
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glBindRenderbuffer( GL_RENDERBUFFER, 0 );
}

/////////////////////////////////////////////////////////////

static bool GLHWSupportsShaders( void ) {
	return ( GLVersion( 2, 1 ) || ( gl_capabilities.fragment_program && gl_capabilities.vertex_program ) );
}

static bool GLHWSupportsMultitexture( void ) {
	return gl_capabilities.multitexture;
}

static void GLGetMaxTextureUnits( unsigned int *num_units ) {
	glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, ( GLint * ) num_units );
}

static void GLGetMaxTextureSize( unsigned int *s ) {
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, ( GLint * ) s );
}

/////////////////////////////////////////////////////////////

static void GLSetClearColour( PLColour rgba ) {
	glClearColor(
		plByteToFloat( rgba.r ),
		plByteToFloat( rgba.g ),
		plByteToFloat( rgba.b ),
		plByteToFloat( rgba.a )
	);
}

static void GLClearBuffers( unsigned int buffers ) {
	// Rather ugly, but translate it over to GL.
	unsigned int glclear = 0;
	if ( buffers & PL_BUFFER_COLOUR ) glclear |= GL_COLOR_BUFFER_BIT;
	if ( buffers & PL_BUFFER_DEPTH ) glclear |= GL_DEPTH_BUFFER_BIT;
	if ( buffers & PL_BUFFER_STENCIL ) glclear |= GL_STENCIL_BUFFER_BIT;
	glClear( glclear );
}

static void GLSetDepthBufferMode( unsigned int mode ) {
	switch ( mode ) {
		default:
			GfxLog( "Unknown depth buffer mode, %d\n", mode );
			break;

		case PL_DEPTHBUFFER_DISABLE:
			glDisable( GL_DEPTH_TEST );
			break;

		case PL_DEPTHBUFFER_ENABLE:
			glEnable( GL_DEPTH_TEST );
			break;
	}
}

static void GLSetDepthMask( bool enable ) {
	glDepthMask( enable );
}

/////////////////////////////////////////////////////////////

static unsigned int TranslateBlendFunc( PLBlend blend ) {
	switch ( blend ) {
		default:
		case PL_BLEND_ONE: return GL_ONE;
		case PL_BLEND_ZERO: return GL_ZERO;
		case PL_BLEND_SRC_COLOR: return GL_SRC_COLOR;
		case PL_BLEND_ONE_MINUS_SRC_COLOR: return GL_ONE_MINUS_SRC_COLOR;
		case PL_BLEND_SRC_ALPHA: return GL_SRC_ALPHA;
		case PL_BLEND_ONE_MINUS_SRC_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
		case PL_BLEND_DST_ALPHA: return GL_DST_ALPHA;
		case PL_BLEND_ONE_MINUS_DST_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
		case PL_BLEND_DST_COLOR: return GL_DST_COLOR;
		case PL_BLEND_ONE_MINUS_DST_COLOR: return GL_ONE_MINUS_DST_COLOR;
		case PL_BLEND_SRC_ALPHA_SATURATE: return GL_SRC_ALPHA_SATURATE;
	}
}

static void GLSetBlendMode( PLBlend a, PLBlend b ) {
	if ( a == PL_BLEND_NONE && b == PL_BLEND_NONE ) {
		glDisable( GL_BLEND );
	} else {
		glEnable( GL_BLEND );
	}

	glBlendFunc( TranslateBlendFunc( a ), TranslateBlendFunc( b ) );
}

static void GLSetCullMode( PLCullMode mode ) {
	if ( mode == gfx_state.current_cullmode ) {
		return;
	}

	if ( mode == PL_CULL_NONE ) {
		glDisable( GL_CULL_FACE );
	} else {
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		switch ( mode ) {
			default:
			case PL_CULL_NEGATIVE:
				glFrontFace( GL_CW );
				break;

			case PL_CULL_POSTIVE:
				glFrontFace( GL_CCW );
				break;
		}
	}

	gfx_state.current_cullmode = mode;
}



/////////////////////////////////////////////////////////////
// Framebuffer

static unsigned int TranslateFrameBufferBinding( PLFBOTarget targetBinding ) {
	switch ( targetBinding ) {
		case PL_FRAMEBUFFER_DEFAULT: return GL_FRAMEBUFFER;
		case PL_FRAMEBUFFER_DRAW: return GL_DRAW_FRAMEBUFFER;
		case PL_FRAMEBUFFER_READ: return GL_READ_FRAMEBUFFER;
		default: return 0;
	}
}

static void GLCreateFrameBuffer( PLFrameBuffer *buffer ) {
	glGenFramebuffers( 1, &buffer->fbo );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, buffer->fbo );
	//GfxLog( "Created framebuffer %dx%d", buffer->width, buffer->height );

	if ( buffer->flags & PL_BUFFER_COLOUR ) {
		glGenRenderbuffers( 1, &buffer->rbo_colour );
		glBindRenderbuffer( GL_RENDERBUFFER, buffer->rbo_colour );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA, buffer->width, buffer->height );
		glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, buffer->rbo_colour );
		//GfxLog( "Created colour renderbuffer %dx%d", buffer->width, buffer->height );
	}

	if ( buffer->flags & PL_BUFFER_DEPTH ) {
		glGenRenderbuffers( 1, &buffer->rbo_depth );
		glBindRenderbuffer( GL_RENDERBUFFER, buffer->rbo_depth );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, buffer->width, buffer->height );
		glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer->rbo_depth );
		//GfxLog( "Created depth renderbuffer %dx%d", buffer->width, buffer->height );
	}

	if ( buffer->flags & PL_BUFFER_STENCIL ) {
		GfxLog( "Stencil renderbuffer not supported yet!" );
	}
}

static void GLDeleteFrameBuffer( PLFrameBuffer *buffer ) {
	if ( buffer ) {
		glDeleteFramebuffers( 1, &buffer->fbo );
		if ( buffer->rbo_colour ) {
			glDeleteRenderbuffers( 1, &buffer->rbo_colour );
		}
		if ( buffer->rbo_depth ) {
			glDeleteRenderbuffers( 1, &buffer->rbo_depth );
		}
	}
}

static void GLBindFrameBuffer( PLFrameBuffer *buffer, PLFBOTarget target_binding ) {
	GLuint binding = TranslateFrameBufferBinding( target_binding );
	if ( buffer ) {
		glBindFramebuffer( binding, buffer->fbo );
	} else {
		glBindFramebuffer( binding, 0 ); //Bind default backbuffer
	}
}

static void GLBlitFrameBuffers( PLFrameBuffer *src_buffer,
								unsigned int src_w,
								unsigned int src_h,
								PLFrameBuffer *dst_buffer,
								unsigned int dst_w,
								unsigned int dst_h,
								bool linear ) {
	if ( src_buffer ) {
		glBindFramebuffer( GL_READ_FRAMEBUFFER, src_buffer->fbo );
	} else {
		glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 ); //Bind default backbuffer
	}

	if ( dst_buffer ) {
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, dst_buffer->fbo );
	} else {
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ); //Bind default backbuffer
	}

	glBlitFramebuffer( 0, 0, src_w, src_h, 0, 0, dst_w, dst_h, GL_COLOR_BUFFER_BIT, linear ? GL_LINEAR : GL_NEAREST );
}

static PLTexture *GLGetFrameBufferTextureAttachment( PLFrameBuffer *buffer ) {
	PLTexture *texture = plCreateTexture();
	if ( texture == NULL ) {
		return NULL;
	}

	/* all of this is going to change later...
	 * this is just the bare minimum to get things going */

	_plBindTexture( texture );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, buffer->width, buffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->internal.id, 0 );

	_plBindTexture( NULL );

	return texture;
}

/////////////////////////////////////////////////////////////
// Texture

static unsigned int TranslateImageFormat( PLImageFormat format ) {
	switch ( format ) {
		case PL_IMAGEFORMAT_RGB8: return GL_RGB8;
		case PL_IMAGEFORMAT_RGBA8: return GL_RGBA8;
		case PL_IMAGEFORMAT_RGB4: return GL_RGB4;
		case PL_IMAGEFORMAT_RGBA4: return GL_RGBA4;
		case PL_IMAGEFORMAT_RGB5: return GL_RGB5;
		case PL_IMAGEFORMAT_RGB5A1: return GL_RGB5_A1;

		case PL_IMAGEFORMAT_RGB_DXT1: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		case PL_IMAGEFORMAT_RGBA_DXT1: return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		case PL_IMAGEFORMAT_RGB_FXT1: return GL_COMPRESSED_RGB_FXT1_3DFX;

		default: return 0;
	}
}

static unsigned int TranslateStorageFormat( PLDataFormat format ) {
	switch ( format ) {
		case PL_UNSIGNED_BYTE: return GL_UNSIGNED_BYTE;
		case PL_UNSIGNED_INT_8_8_8_8_REV: return GL_UNSIGNED_INT_8_8_8_8_REV;
		default:plAssert( 0 );
			return 0; /* todo */
	}
}

static unsigned int TranslateImageColourFormat( PLColourFormat format ) {
	switch ( format ) {
		default:
		case PL_COLOURFORMAT_RGBA: return GL_RGBA;
		case PL_COLOURFORMAT_RGB: return GL_RGB;
	}
}

static void GLCreateTexture( PLTexture *texture ) {
	glGenTextures( 1, &texture->internal.id );
}

static void GLDeleteTexture( PLTexture *texture ) {
	glDeleteTextures( 1, &texture->internal.id );
}

static void GLBindTexture( const PLTexture *texture ) {
	if ( texture == NULL ) {
		glBindTexture( GL_TEXTURE_2D, 0 );
		return;
	}
	glBindTexture( GL_TEXTURE_2D, texture->internal.id );
}

static void GL_TranslateTextureFilterFormat( PLTextureFilter filterMode, unsigned int *min, unsigned int *mag ) {
	switch ( filterMode ) {
		case PL_TEXTURE_FILTER_LINEAR:
			*min = *mag = GL_LINEAR;
			break;
		default:
		case PL_TEXTURE_FILTER_NEAREST:
			*min = *mag = GL_NEAREST;
			break;
		case PL_TEXTURE_FILTER_MIPMAP_LINEAR:
			*min = GL_LINEAR_MIPMAP_LINEAR;
			*mag = GL_LINEAR;
			break;
		case PL_TEXTURE_FILTER_MIPMAP_LINEAR_NEAREST:
			*min = GL_LINEAR_MIPMAP_NEAREST;
			*mag = GL_LINEAR;
			break;
		case PL_TEXTURE_FILTER_MIPMAP_NEAREST:
			*min = GL_NEAREST_MIPMAP_NEAREST;
			*mag = GL_NEAREST;
			break;
		case PL_TEXTURE_FILTER_MIPMAP_NEAREST_LINEAR:
			*min = GL_NEAREST_MIPMAP_LINEAR;
			*mag = GL_NEAREST;
			break;
	}
}

static void GLUploadTexture( PLTexture *texture, const PLImage *upload ) {
	/* was originally GL_CLAMP; deprecated in GL3+, though some drivers
	 * still seem to accept it anyway except for newer Intel GPUs apparently */
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	unsigned int min, mag;
	GL_TranslateTextureFilterFormat( texture->filter, &min, &mag );
	if ( texture->filter == PL_TEXTURE_FILTER_LINEAR || texture->filter == PL_TEXTURE_FILTER_NEAREST ) {
		texture->flags |= PL_TEXTURE_FLAG_NOMIPS;
	}

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min );

	unsigned int levels = upload->levels;
	if ( levels == 0 ) {
		levels = 1;
	}

	unsigned int image_format = TranslateImageFormat( upload->format );
	unsigned int colour_format = TranslateImageColourFormat( upload->colour_format );
	unsigned int storage_format = TranslateStorageFormat( texture->storage );

	for ( unsigned int i = 0; i < levels; ++i ) {
		GLsizei w = texture->w / ( unsigned int ) pow( 2, i );
		GLsizei h = texture->h / ( unsigned int ) pow( 2, i );
		if ( plIsCompressedImageFormat( upload->format ) ) {
			glCompressedTexImage2D(
				GL_TEXTURE_2D,
				i,
				image_format,
				w, h,
				0,
				( GLsizei ) upload->size,
				upload->data[ 0 ]
			);
		} else {
			glTexImage2D(
				GL_TEXTURE_2D,
				i,
				image_format,
				w, h,
				0,
				colour_format,
				storage_format,
				upload->data[ 0 ]
			);
		}
	}

	if ( levels == 1 && !( texture->flags & PL_TEXTURE_FLAG_NOMIPS ) ) {
		glGenerateMipmap( GL_TEXTURE_2D );
	}
}

static void GLSetTextureAnisotropy( PLTexture *texture, uint32_t value ) {
	plSetTexture( texture, gfx_state.current_textureunit );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, ( int ) value );
}

static void GLActiveTexture( unsigned int target ) {
	glActiveTexture( GL_TEXTURE0 + target );
}

/* Swizzle texture channels */

static int TranslateColourChannel( int channel ) {
	switch ( channel ) {
		case PL_RED: return GL_RED;
		case PL_GREEN: return GL_GREEN;
		case PL_BLUE: return GL_BLUE;
		case PL_ALPHA: return GL_ALPHA;
		default: return channel;
	}
}

static void GLSwizzleTexture( PLTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
	GLBindTexture( texture );
	if ( GLVersion( 3, 3 ) ) {
		int swizzle[] = {
			TranslateColourChannel( r ),
			TranslateColourChannel( g ),
			TranslateColourChannel( b ),
			TranslateColourChannel( a )
		};
		glTexParameteriv( GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle );
	} else {
		ReportError( PL_RESULT_UNSUPPORTED, "missing software implementation" );
	}
}

/////////////////////////////////////////////////////////////
// Mesh

typedef struct MeshTranslatePrimitive {
	PLMeshPrimitive mode;
	unsigned int target;
	const char *name;
} MeshTranslatePrimitive;

static MeshTranslatePrimitive primitives[] = {
	{ PL_MESH_LINES, GL_LINES, "LINES" },
	{ PL_MESH_LINE_LOOP, GL_LINE_LOOP, "LINE_LOOP" },
	{ PL_MESH_POINTS, GL_POINTS, "POINTS" },
	{ PL_MESH_TRIANGLES, GL_TRIANGLES, "TRIANGLES" },
	{ PL_MESH_TRIANGLE_FAN, GL_TRIANGLE_FAN, "TRIANGLE_FAN" },
	{ PL_MESH_TRIANGLE_FAN_LINE, GL_LINES, "TRIANGLE_FAN_LINE" },
	{ PL_MESH_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP" },
	{ PL_MESH_QUADS, GL_TRIANGLES, "QUADS" }   // todo, translate
};

static unsigned int TranslatePrimitiveMode( PLMeshPrimitive mode ) {
	for ( unsigned int i = 0; i < plArrayElements( primitives ); i++ ) {
		if ( mode == primitives[ i ].mode )
			return primitives[ i ].target;
	}

	// Hacky, but just return initial otherwise.
	return primitives[ 0 ].target;
}

static unsigned int TranslateDrawMode( PLMeshDrawMode mode ) {
	switch ( mode ) {
		case PL_DRAW_DYNAMIC: return GL_DYNAMIC_DRAW;
		case PL_DRAW_STATIC: return GL_STATIC_DRAW;
		case PL_DRAW_STREAM: return GL_STREAM_DRAW;
		default: return 0;
	}
}

enum {
	BUFFER_VERTEX_DATA,
	BUFFER_ELEMENT_DATA,
};

static void GLCreateMesh( PLMesh *mesh ) {
	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	// Create VBO
	glGenBuffers( 1, &mesh->internal.buffers[ BUFFER_VERTEX_DATA ] );

	// Create the EBO
	if ( mesh->num_indices > 0 ) {
		glGenBuffers( 1, &mesh->internal.buffers[ BUFFER_ELEMENT_DATA ] );
	}
}

static void GLUploadMesh( PLMesh *mesh ) {
	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	PLShaderProgram *program = gfx_state.current_program;
	if ( program == NULL ) {
		return;
	}

	//Bind VBO
	glBindBuffer( GL_ARRAY_BUFFER, mesh->internal.buffers[ BUFFER_VERTEX_DATA ] );

	//Write the current CPU vertex data into the VBO
	glBufferData( GL_ARRAY_BUFFER, sizeof( PLVertex ) * mesh->num_verts, &mesh->vertices[ 0 ],
				  TranslateDrawMode( mesh->mode ) );

	//Point to the different substreams of the interleaved BVO
	//Args: Index, Size, Type, (Normalized), Stride, StartPtr

	if ( program->internal.v_position != -1 ) {
		glEnableVertexAttribArray( program->internal.v_position );
		glVertexAttribPointer( program->internal.v_position, 3, GL_FLOAT, GL_FALSE, sizeof( PLVertex ),
							   ( const GLvoid * ) pl_offsetof( PLVertex, position ) );
	}

	if ( program->internal.v_normal != -1 ) {
		glEnableVertexAttribArray( program->internal.v_normal );
		glVertexAttribPointer( program->internal.v_normal, 3, GL_FLOAT, GL_FALSE, sizeof( PLVertex ),
							   ( const GLvoid * ) pl_offsetof( PLVertex, normal ) );
	}

	if ( program->internal.v_uv != -1 ) {
		glEnableVertexAttribArray( program->internal.v_uv );
		glVertexAttribPointer( program->internal.v_uv, 2, GL_FLOAT, GL_FALSE, sizeof( PLVertex ),
							   ( const GLvoid * ) pl_offsetof( PLVertex, st ) );
	}

	if ( program->internal.v_colour != -1 ) {
		glEnableVertexAttribArray( program->internal.v_colour );
		glVertexAttribPointer( program->internal.v_colour, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof( PLVertex ),
							   ( const GLvoid * ) pl_offsetof( PLVertex, colour ) );
	}

	if ( mesh->internal.buffers[ BUFFER_ELEMENT_DATA ] != 0 ) {
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->internal.buffers[ BUFFER_ELEMENT_DATA ] );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,
					  sizeof( unsigned int ) * mesh->num_indices,
					  &mesh->indices[ 0 ],
					  GL_STATIC_DRAW );
	}
}

static void GLDeleteMesh( PLMesh *mesh ) {
	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	glDeleteBuffers( 1, &mesh->internal.buffers[ BUFFER_VERTEX_DATA ] );
	glDeleteBuffers( 1, &mesh->internal.buffers[ BUFFER_ELEMENT_DATA ] );
}

static void GLDrawMesh( PLMesh *mesh ) {
	if ( gfx_state.current_program == NULL ) {
		GfxLog( "no shader assigned!\n" );
		return;
	}

	/* anything less and we'll just fallback to immediate */
	if ( !GLVersion( 2, 0 ) ) {
		/* todo... */
		for ( unsigned int i = 0; i < gfx_state.current_program->num_stages; ++i ) {
			PLShaderStage *stage = gfx_state.current_program->stages[ i ];
			if ( stage->SWFallback == NULL ) {
				continue;
			}

			stage->SWFallback( gfx_state.current_program, stage->type );
		}
		return;
	}

	if ( mesh->internal.buffers[ BUFFER_VERTEX_DATA ] == 0 ) {
		GfxLog( "invalid buffer provided, skipping draw!\n" );
		return;
	}

	//Write camera matrices to shader shared uniforms
	GLuint view_loc = glGetUniformLocation( gfx_state.current_program->internal.id, "pl_view" );
	GLuint proj_loc = glGetUniformLocation( gfx_state.current_program->internal.id, "pl_proj" );
	glUniformMatrix4fv( view_loc, 1, GL_FALSE, gfx_state.view_matrix.m );
	glUniformMatrix4fv( proj_loc, 1, GL_FALSE, gfx_state.projection_matrix.m );

	//Ensure VAO/VBO/EBO are bound
	if ( GLVersion( 3, 0 ) ) {
		glBindVertexArray( VAO[ 0 ] );
		/* todo: fallback for legacy... */
	}

	glBindBuffer( GL_ARRAY_BUFFER, mesh->internal.buffers[ BUFFER_VERTEX_DATA ] );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mesh->internal.buffers[ BUFFER_ELEMENT_DATA ] );

	//draw
	GLuint mode = TranslatePrimitiveMode( mesh->primitive );
	if ( mesh->num_indices > 0 ) {
		glDrawElements( mode, mesh->num_indices, GL_UNSIGNED_INT, 0 );
	} else {
		glDrawArrays( mode, 0, mesh->num_verts );
	}
}

/////////////////////////////////////////////////////////////
// Camera

enum {
	VIEWPORT_FRAMEBUFFER,
	VIEWPORT_RENDERBUFFER_DEPTH,
	VIEWPORT_RENDERBUFFER_COLOUR,
};

static void GLCreateCamera( PLCamera *camera ) {
	plAssert( camera );
}

static void GLDestroyCamera( PLCamera *camera ) {
	plAssert( camera );
}

static void GLSetupCamera( PLCamera *camera ) {
	plAssert( camera );

	if ( camera->viewport.auto_scale && GLVersion( 3, 0 ) ) {
		GLint bound_rbo_w, bound_rbo_h;
		glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &bound_rbo_w );
		glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &bound_rbo_h );
		camera->viewport.x = 0;
		camera->viewport.y = 0;
		camera->viewport.w = bound_rbo_w;
		camera->viewport.h = bound_rbo_h;
	}

	glViewport( camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h );
	glScissor( camera->viewport.x, camera->viewport.y, camera->viewport.w, camera->viewport.h );
}

/////////////////////////////////////////////////////////////
// Shader

#define SHADER_INVALID_TYPE ((uint32_t)0 - 1)

static GLenum TranslateShaderStageType( PLShaderStageType type ) {
	switch ( type ) {
		case PL_SHADER_TYPE_VERTEX: return GL_VERTEX_SHADER;
		case PL_SHADER_TYPE_COMPUTE: return GL_COMPUTE_SHADER;
		case PL_SHADER_TYPE_FRAGMENT: return GL_FRAGMENT_SHADER;
		case PL_SHADER_TYPE_GEOMETRY: return GL_GEOMETRY_SHADER;
		default: return SHADER_INVALID_TYPE;
	}
}

static const char *GetGLShaderStageDescriptor( GLenum type ) {
	switch ( type ) {
		case GL_VERTEX_SHADER: return "GL_VERTEX_SHADER";
		case GL_COMPUTE_SHADER: return "GL_COMPUTE_SHADER";
		case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
		case GL_GEOMETRY_SHADER: return "GL_GEOMETRY_SHADER";
		default: return "unknown";
	}
}

static GLenum TranslateShaderUniformType( PLShaderUniformType type ) {
	switch ( type ) {
		case PL_UNIFORM_BOOL: return GL_BOOL;
		case PL_UNIFORM_DOUBLE: return GL_DOUBLE;
		case PL_UNIFORM_FLOAT: return GL_FLOAT;
		case PL_UNIFORM_INT: return GL_INT;
		case PL_UNIFORM_UINT: return GL_UNSIGNED_INT;

		case PL_UNIFORM_SAMPLER1D: return GL_SAMPLER_1D;
		case PL_UNIFORM_SAMPLER1DSHADOW: return GL_SAMPLER_1D_SHADOW;
		case PL_UNIFORM_SAMPLER2D: return GL_SAMPLER_2D;
		case PL_UNIFORM_SAMPLER2DSHADOW: return GL_SAMPLER_2D_SHADOW;
		case PL_UNIFORM_SAMPLER3D: return GL_SAMPLER_3D;
		case PL_UNIFORM_SAMPLERCUBE: return GL_SAMPLER_CUBE;

		case PL_UNIFORM_VEC2: return GL_FLOAT_VEC2;
		case PL_UNIFORM_VEC3: return GL_FLOAT_VEC3;
		case PL_UNIFORM_VEC4: return GL_FLOAT_VEC4;

		case PL_UNIFORM_MAT3: return GL_FLOAT_MAT3;

		default: return SHADER_INVALID_TYPE;
	}
}

static unsigned int TranslateGLShaderUniformType( GLenum type ) {
	switch ( type ) {
		case GL_BOOL: return PL_UNIFORM_BOOL;
		case GL_DOUBLE: return PL_UNIFORM_DOUBLE;
		case GL_FLOAT: return PL_UNIFORM_FLOAT;
		case GL_INT: return PL_UNIFORM_INT;
		case GL_UNSIGNED_INT: return PL_UNIFORM_UINT;

		case GL_SAMPLER_1D: return PL_UNIFORM_SAMPLER1D;
		case GL_SAMPLER_1D_SHADOW: return PL_UNIFORM_SAMPLER1DSHADOW;
		case GL_SAMPLER_2D: return PL_UNIFORM_SAMPLER2D;
		case GL_SAMPLER_2D_SHADOW: return PL_UNIFORM_SAMPLER2DSHADOW;
		case GL_SAMPLER_3D: return PL_UNIFORM_SAMPLER3D;
		case GL_SAMPLER_CUBE: return PL_UNIFORM_SAMPLERCUBE;

		case GL_FLOAT_VEC2: return PL_UNIFORM_VEC2;
		case GL_FLOAT_VEC3: return PL_UNIFORM_VEC3;
		case GL_FLOAT_VEC4: return PL_UNIFORM_VEC4;

		case GL_FLOAT_MAT3: return PL_UNIFORM_MAT3;

		default: return SHADER_INVALID_TYPE;
	}
}

static void GLCreateShaderProgram( PLShaderProgram *program ) {
	if ( !GLVersion( 2, 0 ) ) {
		GfxLog( "HW shaders unsupported on platform, relying on SW fallback\n" );
		return;
	}

	program->internal.id = glCreateProgram();
	if ( program->internal.id == 0 ) {
		GfxLog( "Failed to generate shader program!\n" );
		return;
	}
}

static void GLDestroyShaderProgram( PLShaderProgram *program ) {
	if ( program->internal.id == 0 ) {
		return;
	}

	if ( GLVersion( 2, 0 ) ) {
		glDeleteProgram( program->internal.id );
	}

	program->internal.id = 0;
}

static void GLCreateShaderStage( PLShaderStage *stage ) {
	if ( !GLVersion( 2, 0 ) ) {
		GfxLog( "HW shaders unsupported on platform, relying on SW fallback\n" );
		return;
	}

	GLenum type = TranslateShaderStageType( stage->type );
	if ( type == SHADER_INVALID_TYPE ) {
		ReportError( PL_RESULT_INVALID_SHADER_TYPE, "%u", type );
		return;
	}

	if ( type == GL_GEOMETRY_SHADER && !GLVersion( 3, 0 ) ) {
		ReportError( PL_RESULT_UNSUPPORTED_SHADER_TYPE, "%s", GetGLShaderStageDescriptor( type ) );
		return;
	}

	if ( type == GL_COMPUTE_SHADER && !GLVersion( 4, 3 ) ) {
		ReportError( PL_RESULT_UNSUPPORTED_SHADER_TYPE, "%s", GetGLShaderStageDescriptor( type ) );
		return;
	}

	stage->internal.id = glCreateShader( type );
	if ( stage->internal.id == 0 ) {
		ReportError( PL_RESULT_INVALID_SHADER_TYPE, "%u", type );
		return;
	}
}

static void GLDestroyShaderStage( PLShaderStage *stage ) {
	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	if ( stage->program != NULL ) {
		glDetachShader( stage->program->internal.id, stage->internal.id );
		stage->program = NULL;
	}

	glDeleteShader( stage->internal.id );
	stage->internal.id = 0;
}

static void GLAttachShaderStage( PLShaderProgram *program, PLShaderStage *stage ) {
	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	glAttachShader( program->internal.id, stage->internal.id );
}

static void GLCompileShaderStage( PLShaderStage *stage, const char *buf, size_t length ) {
	plUnused( length );

	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	glShaderSource( stage->internal.id, 1, &buf, NULL );

	GfxLog( "COMPILING SHADER STAGE...\n" );
	glCompileShader( stage->internal.id );

	int status;
	glGetShaderiv( stage->internal.id, GL_COMPILE_STATUS, &status );
	if ( status == 0 ) {
		int s_length;
		glGetShaderiv( stage->internal.id, GL_INFO_LOG_LENGTH, &s_length );
		if ( s_length > 1 ) {
			char *log = pl_calloc( ( size_t ) s_length, sizeof( char ) );
			glGetShaderInfoLog( stage->internal.id, s_length, NULL, log );
			GfxLog( " COMPILE ERROR:\n%s\n", log );
			ReportError( PL_RESULT_SHADER_COMPILE, "%s", log );
			pl_free( log );
		}
	} else {
		GfxLog( " COMPLETED SUCCESSFULLY!\n" );
	}
}

static void GLSetShaderUniformMatrix4( PLShaderProgram *program, int slot, PLMatrix4 value, bool transpose ) {
	plUnused( program );

	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	GLuint loc = ( GLuint ) slot;
	glUniformMatrix4fv( loc, 1, transpose ? GL_TRUE : GL_FALSE, value.m );
}

static void GLLinkShaderProgram( PLShaderProgram *program ) {
	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	GfxLog( "LINKING SHADER PROGRAM...\n" );
	glLinkProgram( program->internal.id );

	int status;
	glGetProgramiv( program->internal.id, GL_LINK_STATUS, &status );
	if ( status == 0 ) {
		int s_length;
		glGetProgramiv( program->internal.id, GL_INFO_LOG_LENGTH, &s_length );
		if ( s_length > 1 ) {
			char *log = pl_calloc( ( size_t ) s_length, sizeof( char ) );
			glGetProgramInfoLog( program->internal.id, s_length, NULL, log );
			GfxLog( " LINK ERROR:\n%s\n", log );
			pl_free( log );

			ReportError( PL_RESULT_SHADER_COMPILE, "%s", log );
		} else {
			GfxLog( " UNKNOWN LINK ERROR!\n" );
		}

		return;
	}

	GfxLog( " LINKED SUCCESSFULLY!\n" );
	program->is_linked = true;
}

static void GLSetShaderProgram( PLShaderProgram *program ) {
	if ( !GLVersion( 2, 0 ) ) {
		return;
	}

	unsigned int id = 0;
	if ( program != NULL ) {
		id = program->internal.id;
	}

	glUseProgram( id );
}

/////////////////////////////////////////////////////////////
// Generic State Management

unsigned int TranslateGraphicsState( PLGraphicsState state ) {
	switch ( state ) {
		default:return 0;
		case PL_GFX_STATE_FOG:
			if ( GLVersion( 3, 0 ) ) {
				return 0;
			}
			return GL_FOG;
		case PL_GFX_STATE_ALPHATEST:
			if ( GLVersion( 3, 0 ) ) {
				return 0;
			}
			return GL_ALPHA_TEST;
		case PL_GFX_STATE_BLEND:return GL_BLEND;
		case PL_GFX_STATE_DEPTHTEST:return GL_DEPTH_TEST;
		case PL_GFX_STATE_STENCILTEST:return GL_STENCIL_TEST;
		case PL_GFX_STATE_MULTISAMPLE:return GL_MULTISAMPLE;
		case PL_GFX_STATE_SCISSORTEST:return GL_SCISSOR_TEST;
		case PL_GFX_STATE_ALPHATOCOVERAGE:return GL_SAMPLE_ALPHA_TO_COVERAGE;
	}
}

void GLEnableState( PLGraphicsState state ) {
	unsigned int gl_state = TranslateGraphicsState( state );
	if ( !gl_state ) {
		/* probably unsupported */
		return;
	}

	glEnable( gl_state );

	gfx_state.current_capabilities[ state ] = true;
}

void GLDisableState( PLGraphicsState state ) {
	unsigned int gl_state = TranslateGraphicsState( state );
	if ( !gl_state ) {
		/* probably unsupported */
		return;
	}

	glDisable( gl_state );

	gfx_state.current_capabilities[ state ] = false;
}

/////////////////////////////////////////////////////////////

static char gl_extensions[4096][4096] = { { '\0' } };

#if defined(DEBUG_GL)
static void MessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	void *param ) {
	plUnused( source );
	plUnused( id );
	plUnused( length );
	plUnused( param );

	const char *s_severity;
	switch ( severity ) {
		case GL_DEBUG_SEVERITY_HIGH: {
			s_severity = "HIGH";
		}
			break;

		case GL_DEBUG_SEVERITY_MEDIUM: {
			s_severity = "MEDIUM";
		}
			break;

		case GL_DEBUG_SEVERITY_LOW: {
			s_severity = "LOW";
		}
			break;

		default:return;
	}

	const char *s_type;
	switch ( type ) {
		case GL_DEBUG_TYPE_ERROR: {
			s_type = "ERROR";
		}
			break;

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
			s_type = "DEPRECATED";
		}
			break;

		case GL_DEBUG_TYPE_MARKER: {
			s_type = "MARKER";
		}
			break;

		case GL_DEBUG_TYPE_PERFORMANCE: {
			s_type = "PERFORMANCE";
		}
			break;

		case GL_DEBUG_TYPE_PORTABILITY: {
			s_type = "PORTABILITY";
		}
			break;

		default: {
			s_type = "OTHER";
		}
			break;
	}

	if ( message != NULL && message[ 0 ] != '\0' ) {
		GfxLog( "(%s) %s - %s\n", s_type, s_severity, message );
	}
}
#endif

void plInitOpenGL( void ) {
	glewExperimental = true;
	GLenum err = glewInit();
	if ( err != GLEW_OK ) {
		ReportError( PL_RESULT_GRAPHICSINIT, "failed to initialize glew, %s", glewGetErrorString( err ) );
		return;
	}

	// Get any information that will be presented later.
	gfx_state.hw_extensions = ( const char * ) glGetString( GL_EXTENSIONS );
	gfx_state.hw_renderer = ( const char * ) glGetString( GL_RENDERER );
	gfx_state.hw_vendor = ( const char * ) glGetString( GL_VENDOR );
	gfx_state.hw_version = ( const char * ) glGetString( GL_VERSION );

	memset( &gl_capabilities, 0, sizeof( gl_capabilities ) );

	gl_version_major = ( gfx_state.hw_version[ 0 ] - '0' );
	gl_version_minor = ( gfx_state.hw_version[ 2 ] - '0' );

	if ( GLVersion( 3, 0 ) ) {
		int minor, major;
		glGetIntegerv( GL_MINOR_VERSION, &major );
		glGetIntegerv( GL_MAJOR_VERSION, &minor );
		if ( major > 0 ) {
			gl_version_major = major;
			gl_version_minor = minor;
		} else {
			GfxLog( "failed to get OpenGL version, expect some functionality not to work!\n" );
		}
	}

	GfxLog( " OpenGL %d.%d\n", gl_version_major, gl_version_minor );
	GfxLog( "  renderer:   %s\n", gfx_state.hw_renderer );
	GfxLog( "  vendor:     %s\n", gfx_state.hw_vendor );
	GfxLog( "  version:    %s\n", gfx_state.hw_version );
	GfxLog( "  extensions:\n" );

	/* this is kind of gross, but manually change the version,
	 * otherwise there's no other way we can do it on newer
	 * hardware...
	 */
	if ( GLVersion( 1, 0 ) && gfx_layer.mode == PL_GFX_MODE_OPENGL_1_0 ) {
		gl_version_major = 1;
		gl_version_minor = 0;
	}

	if ( GLVersion( 3, 0 ) ) {
		glGetIntegerv( GL_NUM_EXTENSIONS, ( GLint * ) ( &gl_num_extensions ) );
		for ( unsigned int i = 0; i < gl_num_extensions; ++i ) {
			const uint8_t *extension = glGetStringi( GL_EXTENSIONS, i );
			snprintf( gl_extensions[ i ], sizeof( gl_extensions[ i ] ), "%s", extension );
			GfxLog( "    %s\n", extension );
		}
	} else {
		// todo
	}

#if defined(DEBUG_GL)
	if ( GLVersion( 4, 3 ) ) {
		glEnable( GL_DEBUG_OUTPUT );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );

		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE );
		glDebugMessageCallback( ( GLDEBUGPROC ) MessageCallback, NULL );
	}
#endif

	if ( GLEW_ARB_direct_state_access || GLVersion( 4, 5 ) ) {
		gl_capabilities.direct_state_access = true;
	}

	// Init vertex attributes
	if ( GLVersion( 3, 0 ) ) {
		glGenVertexArrays( 1, VAO );
		glBindVertexArray( VAO[ 0 ] );
	}

	/* in OpenGL, multisample is automatically enabled per spec */
	gfx_state.current_capabilities[ PL_GFX_STATE_MULTISAMPLE ] = true;

	// setup the gfx layer
	gfx_layer.InsertDebugMarker = GLInsertDebugMarker;
	gfx_layer.PushDebugGroupMarker = GLPushDebugGroupMarker;
	gfx_layer.PopDebugGroupMarker = GLPopDebugGroupMarker;

	gfx_layer.HWSupportsShaders = GLHWSupportsShaders;
	gfx_layer.HWSupportsMultitexture = GLHWSupportsMultitexture;
	gfx_layer.GetMaxTextureUnits = GLGetMaxTextureUnits;
	gfx_layer.GetMaxTextureSize = GLGetMaxTextureSize;

	gfx_layer.SetBlendMode = GLSetBlendMode;
	gfx_layer.SetCullMode = GLSetCullMode;

	gfx_layer.SetClearColour = GLSetClearColour;
	gfx_layer.ClearBuffers = GLClearBuffers;

	gfx_layer.SetDepthBufferMode = GLSetDepthBufferMode;
	gfx_layer.SetDepthMask = GLSetDepthMask;

	gfx_layer.CreateFrameBuffer = GLCreateFrameBuffer;
	gfx_layer.DeleteFrameBuffer = GLDeleteFrameBuffer;
	gfx_layer.BindFrameBuffer = GLBindFrameBuffer;
	gfx_layer.BlitFrameBuffers = GLBlitFrameBuffers;
	gfx_layer.GetFrameBufferTextureAttachment = GLGetFrameBufferTextureAttachment;

	gfx_layer.CreateTexture = GLCreateTexture;
	gfx_layer.DeleteTexture = GLDeleteTexture;
	gfx_layer.BindTexture = GLBindTexture;
	gfx_layer.UploadTexture = GLUploadTexture;
	gfx_layer.SetTextureAnisotropy = GLSetTextureAnisotropy;
	gfx_layer.ActiveTexture = GLActiveTexture;
	gfx_layer.SwizzleTexture = GLSwizzleTexture;

	gfx_layer.CreateMesh = GLCreateMesh;
	gfx_layer.DeleteMesh = GLDeleteMesh;
	gfx_layer.DrawMesh = GLDrawMesh;
	gfx_layer.UploadMesh = GLUploadMesh;

	gfx_layer.CreateCamera = GLCreateCamera;
	gfx_layer.DestroyCamera = GLDestroyCamera;
	gfx_layer.SetupCamera = GLSetupCamera;

	gfx_layer.CreateShaderProgram = GLCreateShaderProgram;
	gfx_layer.DestroyShaderProgram = GLDestroyShaderProgram;
	gfx_layer.SetShaderProgram = GLSetShaderProgram;
	gfx_layer.LinkShaderProgram = GLLinkShaderProgram;
	gfx_layer.CreateShaderStage = GLCreateShaderStage;
	gfx_layer.DestroyShaderStage = GLDestroyShaderStage;
	gfx_layer.AttachShaderStage = GLAttachShaderStage;
	gfx_layer.CompileShaderStage = GLCompileShaderStage;

	gfx_layer.SetShaderUniformMatrix4 = GLSetShaderUniformMatrix4;

	gfx_layer.EnableState = GLEnableState;
	gfx_layer.DisableState = GLDisableState;
}

void plShutdownOpenGL( void ) {
#if defined(DEBUG_GL)
	if ( GLVersion( 4, 3 ) ) {
		glDisable( GL_DEBUG_OUTPUT );
		glDisable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
	}
#endif
}

#endif
