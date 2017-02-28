
#pragma once

// Texture Environment Modes
typedef enum PLTextureEnvironmentMode {
    PL_TEXTUREMODE_ADD,
    PL_TEXTUREMODE_MODULATE,
    PL_TEXTUREMODE_DECAL,
    PL_TEXTUREMODE_BLEND,
    PL_TEXTUREMODE_REPLACE,
    PL_TEXTUREMODE_COMBINE
} PLTextureEnvironmentMode;

typedef enum PLTextureClamp {
#if defined (PL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    VL_TEXTURECLAMP_CLAMP = GL_CLAMP_TO_EDGE,
    VL_TEXTURECLAMP_WRAP = GL_REPEAT,
#elif defined (VL_MODE_GLIDE)
    VL_TEXTURECLAMP_CLAMP	= GR_TEXTURECLAMP_CLAMP,
    VL_TEXTURECLAMP_WRAP	= GR_TEXTURECLAMP_WRAP,
#else
    VL_TEXTURECLAMP_CLAMP,
    VL_TEXTURECLAMP_WRAP,
#endif
} PLTextureClamp;

typedef enum PLTextureFilter {
    PL_TEXTUREFILTER_MIPMAP_NEAREST,        // GL_NEAREST_MIPMAP_NEAREST
    PL_TEXTUREFILTER_MIPMAP_LINEAR,         // GL_LINEAR_MIPMAP_LINEAR

    PL_TEXTUREFILTER_MIPMAP_LINEAR_NEAREST,    // GL_LINEAR_MIPMAP_NEAREST
    PL_TEXTUREFILTER_MIPMAP_NEAREST_LINEAR,    // GL_NEAREST_MIPMAP_LINEAR

    PL_TEXTUREFILTER_NEAREST,       // Nearest filtering
    PL_TEXTUREFILTER_LINEAR         // Linear filtering
} PLTextureFilter;

typedef enum PLTextureTarget {
    PL_TEXTURE_1D,
    PL_TEXTURE_2D,
    PL_TEXTURE_3D
} PLTextureTarget;

enum PLTextureFlag {
    PL_TEXTUREFLAG_PRESERVE = (1 << 0)
};

typedef struct PLTextureMappingUnit {
    PLbool active;

    PLuint current_texture;
    PLuint current_capabilities;

    PLTextureEnvironmentMode current_envmode;
} PLTextureMappingUnit;

typedef struct PLTextureInfo {
    PLbyte *data;

    PLuint x, y;
    PLuint width, height;
    PLuint size;
    PLuint levels;

    PLColourFormat pixel_format;
    PLDataFormat storage_type;
    PLImageFormat format;

    PLbool initial;

    PLuint flags;
} PLTextureInfo;

typedef struct PLTexture {
    PLuint id;

    PLuint flags;
    PLuint width, height;

    PLuint size;

    PLImageFormat format;

#ifdef __cplusplus

    PL_INLINE PLuint GetFlags() const {
        return flags;
    }

    PL_INLINE void AddFlags(PLuint f) {
        flags |= f;
    }

    PL_INLINE void SetFlags(PLuint f) {
        flags = f;
    }

    PL_INLINE void RemoveFlags(PLuint f) {
        flags &= ~f;
    }

    PL_INLINE void ClearFlags() {
        flags = 0;
    }

#endif
} PLTexture;

PL_EXTERN_C

PL_EXTERN void plDeleteTexture(PLTexture *texture, PLbool force);

PL_EXTERN const PLchar *plPrintTextureMemoryUsage(void);

PL_EXTERN_C_END