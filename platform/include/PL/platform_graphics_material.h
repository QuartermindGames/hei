
#pragma once

#include <PL/platform_graphics.h>
#include <PL/platform_graphics_texture.h>
#include <PL/platform_math.h>

typedef enum PLMaterialTextureType {
    PLMATERIAL_TEXTURE_DIFFUSE,	    // Basic diffuse layer.
    PLMATERIAL_TEXTURE_SPHERE,		// Spherical mapping.
    PLMATERIAL_TEXTURE_FULLBRIGHT,	// Adds highlights to the texture.
    PLMATERIAL_TEXTURE_NORMAL,		// Defines the direction of a pixel's normal.
    PLMATERIAL_TEXTURE_DETAIL,		// Detail map is blended with other layers to make textures appear more detailed.
    PLMATERIAL_TEXTURE_LIGHTMAP,

    PLMATERIAL_TEXTURE_MAX
} PLMaterialTextureType;

#define	PLMATERIAL_FLAG_ALPHA		(1 << 1)	// Declares that the given texture has an alpha channel.
#define	PLMATERIAL_FLAG_BLEND		(1 << 2)	// Ditto to the above, but tells us to use blending rather than alpha-test.
#define	PLMATERIAL_FLAG_ANIMATED	(1 << 3)	// This is a global flag; tells the material system to scroll through all skins.
#define	PLMATERIAL_FLAG_MIRROR		(1 << 4)	// Must be GLOBAL!
#define	PLMATERIAL_FLAG_NEAREST	    (1 << 5)	// Forces the texture to be loaded with nearest filtering.
#define	PLMATERIAL_FLAG_WATER		(1 << 6)	// Must be GLOBAL!
#define	PLMATERIAL_FLAG_ADDITIVE	(1 << 7)	// Renders the current skin as an additive.
#define	PLMATERIAL_FLAG_ALPHATRICK	(1 << 8)	// Skips/enables the alpha trick (handy for UI elements!)
#define	PLMATERIAL_FLAG_PRESERVE	(1 << 9)	// Preserves the material during clear outs.

/////////////////////////////////////////////////////////////

#define PLMATERIAL_MAX_TEXTURES     16
#define PLMATERIAL_MAX_SKINS        128

typedef struct PLMaterialTexture {
    PLTexture *texture;

    PLVector2D scroll;
    float rotate, scale;

    PLMaterialTextureType type;
} PLMaterialTexture;

typedef struct PLMaterialSkin {
    PLMaterialTexture textures[PLMATERIAL_MAX_TEXTURES];
    unsigned int num_textures;

    // todo, shader

    unsigned int flags;

    // Animation
    float           animation_speed;    // Speed to scroll through skins, if animation is enabled.
    unsigned int    animation_frame;    // Current frame for animation.
    double          animation_time;     // Time until we animate again.
} PLMaterialSkin;

typedef struct PLMaterial {
    char path[PL_SYSTEM_MAX_PATH];
    char name[128];

    PLMaterialSkin skins[PLMATERIAL_MAX_SKINS];
    unsigned int num_skins;
} PLMaterial;

