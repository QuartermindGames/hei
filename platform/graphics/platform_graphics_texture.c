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

void _InitTextures(void) {
    gfx_state.tmu = (PLTextureMappingUnit*)pl_calloc(plGetMaxTextureUnits(), sizeof(PLTextureMappingUnit));
    for (unsigned int i = 0; i < plGetMaxTextureUnits(); i++) {
        gfx_state.tmu[i].current_envmode = PL_TEXTUREMODE_REPLACE;
    }

    gfx_state.max_textures  = 1024;
    gfx_state.textures      = (PLTexture**)pl_malloc(sizeof(PLTexture*) * gfx_state.max_textures);
    memset(gfx_state.textures, 0, sizeof(PLTexture*) * gfx_state.max_textures);
    gfx_state.num_textures  = 0;

    plRegisterConsoleVariable(NULL, NULL, pl_float_var, 0, NULL);
}

void plShutdownTextures(void) {
    if(gfx_state.tmu) {
        pl_free(gfx_state.tmu);
    }

    if(gfx_state.textures) {
        for(PLTexture **texture = gfx_state.textures;
            texture < gfx_state.textures + gfx_state.num_textures; ++texture) {
            if ((*texture)) {
                plDestroyTexture((*texture), true);
            }
        }
        pl_free(gfx_state.textures);
    }
}

unsigned int plGetMaxTextureSize(void) {
    if (gfx_state.hw_maxtexturesize != 0) {
        return gfx_state.hw_maxtexturesize;
    }

    if(gfx_layer.GetMaxTextureSize != NULL) {
        gfx_layer.GetMaxTextureSize(&gfx_state.hw_maxtexturesize);
    } else {
        gfx_state.hw_maxtexturesize = 4096;
    }

    return gfx_state.hw_maxtexturesize;
}

PLTexture *plCreateTexture(void) {
#if 0
    PLTexture **texture = gfx_state.textures; unsigned int slot;
    for(slot = 0; texture < gfx_state.textures + gfx_state.max_textures; ++texture, ++slot) {
        if(!(*texture)) {
            (*texture) = (PLTexture*)pl_malloc(sizeof(PLTexture));
            if(!(*texture)) {
                ReportError(PL_RESULT_MEMORY_ALLOCATION, "failed to allocate memory for Texture object, %d", sizeof(PLTexture));
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
            ReportError(PL_RESULT_MEMORY_ALLOCATION, "failed to allocate %d bytes",
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
    PLTexture *texture = pl_calloc(1, sizeof(PLTexture));
    if(texture == NULL) {
        return NULL;
    }

    texture->format = PL_IMAGEFORMAT_RGBA8;
    texture->w = 8;
    texture->h = 8;

    CallGfxFunction(CreateTexture, texture);
    return texture;
#endif
}

void plDestroyTexture(PLTexture *texture, bool force) {
    if(texture == NULL) {
        return;
    }

    CallGfxFunction(DeleteTexture, texture);

#if 0
    if(!force) {
        memset(texture, 0, sizeof(PLTexture));
        return;
    }

    gfx_state.textures[texture->internal.id] = NULL;
#endif

    pl_free(texture);
}

/* automatically loads in an image and uploads it as a texture */
PLTexture *plLoadTextureImage(const char *path, PLTextureFilter filter_mode) {
    if(path[0] == ' ' || path[0] == '\0') {
        return NULL;
    }

    PLImage image;
    if(!plLoadImage(path, &image)) {
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

void plSetTexture(PLTexture *texture, unsigned int tmu) {
    plSetTextureUnit(tmu);

    if((gfx_state.textures[tmu] != NULL) && (gfx_state.textures[tmu] == texture)) {
        return;
    }

    CallGfxFunction(BindTexture, texture);

    gfx_state.textures[tmu] = texture;

    plSetTextureUnit(0);
}

void plSetTextureUnit(unsigned int target) {
    if (target == gfx_state.current_textureunit) {
        return;
    }

    if (target > plGetMaxTextureUnits()) {
        GfxLog("Attempted to select a texture image unit beyond what's supported by your hardware! (%i)\n",
                      target);
        return;
    }

    CallGfxFunction(ActiveTexture, target);

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
    CallGfxFunction(SetTextureAnisotropy, texture, amount);
}

void _plBindTexture(const PLTexture *texture) {
    // allow us to pass null texture instances
    // as it will give us an opportunity to unbind
    // them on the GPU upon request
    unsigned int id = 0;
    if(texture != NULL) {
        id = texture->internal.id;
    }

    PLTextureMappingUnit *unit = &gfx_state.tmu[plGetCurrentTextureUnit()];
    if (id == unit->current_texture) {
        return;
    }

    CallGfxFunction(BindTexture, texture);

    unit->current_texture = id;
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

    texture->w          = upload->width;
    texture->h          = upload->height;
    texture->format     = upload->format;
    texture->pixel      = upload->colour_format;
    texture->size       = upload->size;
    texture->levels     = upload->levels;

    if(texture->flags & PL_TEXTURE_FLAG_NOMIPS &&
            (texture->filter != PL_TEXTURE_FILTER_LINEAR && texture->filter != PL_TEXTURE_FILTER_NEAREST)) {
        GfxLog("invalid filter mode for texture - if specifying nomips, use either linear or nearest!");
        texture->filter = PL_TEXTURE_FILTER_NEAREST;
    }

    const char *file_name = plGetFileName(upload->path);
    if(file_name == NULL || file_name[0] == '\0') {
        strcpy(texture->name, "null");
    } else {
        strncpy(texture->name, file_name, sizeof(texture->name));
    }

    _plBindTexture(texture);
    CallGfxFunction(UploadTexture, texture, upload);
    _plBindTexture(NULL);

    return true;
}

void plSwizzleTexture(PLTexture *texture, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    CallGfxFunction(SwizzleTexture, texture, r, g, b, a);
}