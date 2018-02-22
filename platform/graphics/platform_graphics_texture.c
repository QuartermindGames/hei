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

#include "graphics_private.h"

#include <PL/platform_image.h>
#include <PL/platform_console.h>
#include <PL/platform_graphics_texture.h>
#include <PL/platform_filesystem.h>

PLConsoleVariable pl_texture_anisotropy = { "gr_texture_anisotropy", "16", pl_int_var, NULL };

#define FREE_TEXTURE    ((unsigned int)-1)

void InitTextures(void) {
    gfx_state.tmu = (PLTextureMappingUnit*)calloc(plGetMaxTextureUnits(), sizeof(PLTextureMappingUnit));
    memset(gfx_state.tmu, 0, sizeof(PLTextureMappingUnit));
    for (unsigned int i = 0; i < plGetMaxTextureUnits(); i++) {
        gfx_state.tmu[i].current_envmode = PL_TEXTUREMODE_REPLACE;
    }

    gfx_state.max_textures  = 1024;
    gfx_state.textures      = (PLTexture**)malloc(sizeof(PLTexture*) * gfx_state.max_textures);
    memset(gfx_state.textures, 0, sizeof(PLTexture*) * gfx_state.max_textures);
    gfx_state.num_textures  = 0;

    plRegisterConsoleVariable(NULL, NULL, pl_float_var, 0, NULL);
}

void ShutdownTextures(void) {
    if(gfx_state.tmu) {
        free(gfx_state.tmu);
    }

    if(gfx_state.textures) {
        for(PLTexture **texture = gfx_state.textures;
            texture < gfx_state.textures + gfx_state.num_textures; ++texture) {
            if ((*texture)) {
                plDeleteTexture((*texture), true);
            }
        }
        free(gfx_state.textures);
    }
}

/////////////////////////////////////////////////////

unsigned int plGetMaxTextureSize(void) {
    if (gfx_state.hw_maxtexturesize != 0) {
        return gfx_state.hw_maxtexturesize;
    }

#if defined (PL_MODE_OPENGL) || defined (PL_MODE_OPENGL_CORE)
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&gfx_state.hw_maxtexturesize);
#elif defined (PL_MODE_GLIDE)
    grGet(GR_MAX_TEXTURE_SIZE, sizeof(gfx_state.hw_maxtexturesize), &gfx_state.hw_maxtexturesize);
#else // Software
    gfx_state.hw_maxtexturesize = 4096;
#endif

    return gfx_state.hw_maxtexturesize;
}

/////////////////////////////////////////////////////

#if defined(PL_MODE_OPENGL)

unsigned int _plTranslateTextureUnit(unsigned int target) {
    unsigned int out = GL_TEXTURE0 + target;
    if (out > (GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS - 1)) {
        GfxLog("Attempted to select an invalid texture image unit! (%i)\n", target);
    }
    return out;
}

unsigned int _plTranslateTextureTarget(PLTextureTarget target) {
    switch (target) {
        default:
        case PL_TEXTURE_2D: return GL_TEXTURE_2D;
        case PL_TEXTURE_1D: return GL_TEXTURE_1D;
        case PL_TEXTURE_3D: return GL_TEXTURE_3D;
    }
}

unsigned int _plTranslateColourFormat(PLColourFormat format) {
    switch(format) {
#if defined(PL_MODE_OPENGL) || defined(VL_MODE_OPENGL_CORE)
        default:
        case PL_COLOURFORMAT_RGB:   return GL_RGB;
        case PL_COLOURFORMAT_RGBA:  return GL_RGBA;
        case PL_COLOURFORMAT_BGR:   return GL_BGR;
        case PL_COLOURFORMAT_BGRA:  return GL_BGRA;
#elif defined(VL_MODE_GLIDE)
        default:
        case PL_COLOURFORMAT_RGBA:  return GR_COLORFORMAT_RGBA;
        case PL_COLOURFORMAT_BGRA:  return GR_COLORFORMAT_BGRA;
        case PL_COLOURFORMAT_ARGB:  return GR_COLORFORMAT_ARGB;
        case PL_COLOURFORMAT_ABGR:  return GR_COLORFORMAT_ABGR;
#endif
    }
}

unsigned int _plTranslateTextureFormat(PLImageFormat format) {
    switch (format) {
        default:
        case PL_IMAGEFORMAT_RGB4:         return GL_RGB4;
        case PL_IMAGEFORMAT_RGBA4:        return GL_RGBA4;
        case PL_IMAGEFORMAT_RGB5:         return GL_RGB5;
        case PL_IMAGEFORMAT_RGB5A1:       return GL_RGB5_A1;
#if defined(PL_MODE_OPENGL_CORE)
        case PL_IMAGEFORMAT_RGB565:       return GL_RGB565;
#endif
        case PL_IMAGEFORMAT_RGB8:         return GL_RGB;
        case PL_IMAGEFORMAT_RGBA8:        return GL_RGBA8;
        case PL_IMAGEFORMAT_RGBA12:       return GL_RGBA12;
        case PL_IMAGEFORMAT_RGBA16:       return GL_RGBA16;
#if defined(PL_MODE_OPENGL_CORE)
        case PL_IMAGEFORMAT_RGBA16F:      return GL_RGBA16F;
#endif
        case PL_IMAGEFORMAT_RGBA_DXT1:    return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case PL_IMAGEFORMAT_RGB_DXT1:     return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case PL_IMAGEFORMAT_RGBA_DXT3:    return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case PL_IMAGEFORMAT_RGBA_DXT5:    return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
}

unsigned int _plTranslateTextureStorageFormat(PLDataFormat format) {
    switch (format) {
        default:
        case PL_UNSIGNED_BYTE:              return GL_UNSIGNED_BYTE;
        case PL_UNSIGNED_INT_8_8_8_8_REV:   return GL_UNSIGNED_INT_8_8_8_8_REV;
    }
}

unsigned int _plTranslateTextureEnvironmentMode(PLTextureEnvironmentMode mode) {
    switch (mode) {
        default:
        case PL_TEXTUREMODE_ADD:        return GL_ADD;
        case PL_TEXTUREMODE_MODULATE:   return GL_MODULATE;
        case PL_TEXTUREMODE_DECAL:      return GL_DECAL;
        case PL_TEXTUREMODE_BLEND:      return GL_BLEND;
        case PL_TEXTUREMODE_REPLACE:    return GL_REPLACE;
        case PL_TEXTUREMODE_COMBINE:    return GL_COMBINE;
    }
}

int _plTranslateColourChannel(int channel) {
    switch(channel) {
        case PL_RED:    return GL_RED;
        case PL_GREEN:  return GL_GREEN;
        case PL_BLUE:   return GL_BLUE;
        case PL_ALPHA:  return GL_ALPHA;
        default:        return channel;
    }
}

#endif

/////////////////////////////////////////////////////

PLTexture *plCreateTexture(void) {
#if 0
    PLTexture **texture = gfx_state.textures; unsigned int slot;
    for(slot = 0; texture < gfx_state.textures + gfx_state.max_textures; ++texture, ++slot) {
        if(!(*texture)) {
            (*texture) = (PLTexture*)malloc(sizeof(PLTexture));
            if(!(*texture)) {
                ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate memory for Texture object, %d\n", sizeof(PLTexture));
                return NULL;
            }

            memset((*texture), 0, sizeof(PLTexture));
            break;
        }

        if((*texture)->internal.id == FREE_TEXTURE) {
            break;
        }
    }

    if(texture >= gfx_state.textures + gfx_state.max_textures) { // ran out of available slots... ?
        PLTexture **old_mem = gfx_state.textures;
        gfx_state.textures = (PLTexture**)realloc(
                gfx_state.textures,
                (gfx_state.max_textures += 512) * sizeof(PLTexture)
        );
        if(!gfx_state.textures) {
            ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate %d bytes!\n",
                           gfx_state.max_textures * sizeof(PLTexture));
            gfx_state.textures = old_mem;
            gfx_state.max_textures -= 512;
        }
        memset(gfx_state.textures + (sizeof(PLTexture) * (gfx_state.max_textures - 512)), 0, sizeof(PLTexture*) * 512);
        return plCreateTexture();
    }

    (*texture)->internal.id = slot;
    (*texture)->format      = PL_IMAGEFORMAT_RGBA8;
    (*texture)->w           = 8;
    (*texture)->h           = 8;

    CallGfxFunction(CreateTexture, (*texture));

    return (*texture);
#else
    PLTexture *texture = malloc(sizeof(PLTexture));
    if(texture == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate %d bytes!\n", sizeof(PLTexture));
        return NULL;
    }

    texture->format = PL_IMAGEFORMAT_RGBA8;
    texture->w = 8;
    texture->h = 8;

    CallGfxFunction(CreateTexture, texture);
    return texture;
#endif
}

void plDeleteTexture(PLTexture *texture, bool force) {
    plAssert(texture);

    CallGfxFunction(DeleteTexture, texture);

#if 0
    if(!force) {
        memset(texture, 0, sizeof(PLTexture));
        return;
    }

    gfx_state.textures[texture->internal.id] = NULL;
#endif

    free(texture);
}

/* automatically loads in an image and uploads it as a texture */
PLTexture *plLoadTextureImage(const char *path, PLTextureFilter filter_mode) {
    if(path[0] == ' ' || path[0] == '\0') {
        return NULL;
    }

    PLImage image;
    if(plLoadImage(path, &image) != PL_RESULT_SUCCESS) {
        return NULL;
    }

    PLTexture *texture = plCreateTexture();
    if(texture != NULL) {
        texture->filter = filter_mode;
        plUploadTextureImage(texture, &image);
    }

    plFreeImage(&image);

    return texture;
}

/////////////////////////////////////////////////////

PLTexture *plGetCurrentTexture(unsigned int tmu) {
    for(PLTexture **texture = gfx_state.textures;
        texture < gfx_state.textures + gfx_state.num_textures; ++texture) {
        if(gfx_state.tmu[tmu].current_texture == (*texture)->internal.id) {
            return (*texture);
        }
    }
    return NULL;
}

unsigned int plGetMaxTextureUnits(void) {
    if (gfx_state.hw_maxtextureunits != 0) {
        return gfx_state.hw_maxtextureunits;
    }

#ifdef PL_MODE_OPENGL
#elif defined (VL_MODE_GLIDE)
    grGet(GR_NUM_TMU, sizeof(param), (FxI32*)graphics_state.hw_maxtextureunits);
#endif

    CallGfxFunction(GetMaxTextureUnits, &gfx_state.hw_maxtextureunits);
    return gfx_state.hw_maxtextureunits;
}

unsigned int plGetCurrentTextureUnit(void) {
    return gfx_state.current_textureunit;
}

void plSetTextureUnit(unsigned int target) {
    if (target == gfx_state.current_textureunit)
        return;

    if (target > plGetMaxTextureUnits()) {
        GfxLog("Attempted to select a texture image unit beyond what's supported by your hardware! (%i)\n",
                      target);
        return;
    }

#if defined (PL_MODE_OPENGL)
    glActiveTexture(_plTranslateTextureUnit(target));
#endif

    gfx_state.current_textureunit = target;
}

unsigned int plGetMaxTextureAnistropy(void) {
    if (gfx_state.hw_maxtextureanistropy != 0) {
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
void plSetTextureAnisotropy(PLTexture *texture, unsigned int amount) {
#if defined (PL_MODE_OPENGL) && !defined(PL_MODE_OPENGL_CORE)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (int) amount);
#endif
}

void BindTexture(const PLTexture *texture) {
    // allow us to pass null texture instances
    // as it will give us an opportunity to unbind
    // them on the GPU upon request
    unsigned int id = 0;
    if(texture != NULL) {
        id = texture->internal.id;
    }

    if (id == gfx_state.tmu[plGetCurrentTextureUnit()].current_texture) {
        return;
    }

    CallGfxFunction(BindTexture, texture);

    gfx_state.tmu[plGetCurrentTextureUnit()].current_texture = id;
}

void plSetTextureFlags(PLTexture *texture, unsigned int flags) {
    texture->flags = flags;
}

void plSetTextureEnvironmentMode(PLTextureEnvironmentMode mode) {
    if (gfx_state.tmu[plGetCurrentTextureUnit()].current_envmode == mode)
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

    gfx_state.tmu[plGetCurrentTextureUnit()].current_envmode = mode;
}

/////////////////////

bool plUploadTextureImage(PLTexture *texture, const PLImage *upload) {
    plAssert(texture);

    BindTexture(texture);

    texture->w          = upload->width;
    texture->h          = upload->height;
    texture->format     = upload->format;
    texture->size       = upload->size;
    texture->levels     = upload->levels;

    const char *file_name = plGetFileName(upload->path);
    if(file_name == NULL || file_name[0] == '\0') {
        strcpy(texture->name, "null");
    } else {
        strncpy(texture->name, file_name, sizeof(texture->name));
    }

    CallGfxFunction(UploadTexture, texture, upload);

    BindTexture(NULL);

    return true;
}

void plSwizzleTexture(PLTexture *texture, int r, int g, int b, int a) {
    plAssert(texture);

    BindTexture(texture);
#if 0
    if(_PLGL_VERSION(3,3)) {
        int swizzle[] = {
                _plTranslateColourChannel(r),
                _plTranslateColourChannel(g),
                _plTranslateColourChannel(b),
                _plTranslateColourChannel(a)
        };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
    } else {
        // todo, software implementation
    }
#endif
}