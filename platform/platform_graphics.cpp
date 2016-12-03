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

#include "platform_graphics.h"
#include "platform_log.h"
#include "platform_image.h"

/*	Graphics	*/

/*	todo,
		replace glew with our own internal solution.
		make everything more scalable depending on current hw.
		decide if we're going to cut support for earlier versions of OpenGL.
		are we going to continue with Glide implementation?
		internal lighting system?? should not be shader-based; just cheap CPU solution.
		how are we going to deal with the fact that Glide doesn't support shaders? Funky SW-based shader solution?
		support for SW rendering? could be neat.
*/

typedef struct PLTextureMappingUnit {
    PLbool active;

    PLuint current_texture;
    PLuint current_capabilities;

    PLTextureEnvironmentMode current_envmode;
} PLTextureMappingUnit;

typedef struct PLGraphicsState {
    PLuint num_cards;        // Number of video cards.

    VLCullMode current_cullmode;

    PLColour current_clearcolour;
    PLColour current_colour;            // Current global colour.

    PLuint current_capabilities;    // Enabled capabilities.
    PLuint current_textureunit;

    // Texture states.
    PLTextureMappingUnit *tmu;

    // Shader states.
    PLShaderProgram current_program;

    // Hardware / Driver information

    const PLchar *hw_vendor;
    const PLchar *hw_renderer;
    const PLchar *hw_version;
    const PLchar *hw_extensions;

    PLuint hw_maxtexturesize;
    PLuint hw_maxtextureunits;
    PLuint hw_maxtextureanistropy;

    ////////////////////////////////////////

    PLint viewport_x, viewport_y;
    PLuint viewport_width, viewport_height;

    PLbool mode_debug;
} PLGraphicsState;

PLGraphicsState pl_graphics_state;

#ifdef _DEBUG
#	define plGraphicsLog(...) plWriteLog("pl_graphics", __VA_ARGS__)
#else
#	define plGraphicsLog(...)
#endif

/*	TODO:
- Add somewhere we can store tracking
data for each of these functions
- Do this in another thread if possible
- Display that data as an overlay
*/
#define    _PL_GRAPHICS_TRACK()                                    \
    {                                                            \
        unsigned static int _t = 0;                                \
        if(pl_graphics_state.mode_debug)                        \
        {                                                        \
            plGraphicsLog(" " PL_FUNCTION "\n");                \
            _t++;                                                \
        }                                                        \
    }

/*===========================
	INITIALIZATION
===========================*/

#if defined (VL_MODE_GLIDE)

#elif defined (VL_MODE_DIRECT3D)

#elif defined (VL_MODE_OPENGL)

PLbool pl_gl_generate_mipmap = PL_FALSE;
PLbool pl_gl_depth_texture = PL_FALSE;
PLbool pl_gl_shadow = PL_FALSE;
PLbool pl_gl_vertex_buffer_object = PL_FALSE;
PLbool pl_gl_texture_compression = PL_FALSE;
PLbool pl_gl_texture_compression_s3tc = PL_FALSE;
PLbool pl_gl_multitexture = PL_FALSE;
PLbool pl_gl_texture_env_combine = PL_FALSE;
PLbool pl_gl_texture_env_add = PL_FALSE;
PLbool pl_gl_vertex_program = PL_FALSE;
PLbool pl_gl_fragment_program = PL_FALSE;

PLuint pl_gl_version_major = 0;
PLuint pl_gl_version_minor = 0;

#define PL_GL_VERSION(maj, min)    ((maj == pl_gl_version_major && min <= pl_gl_version_minor) || maj < pl_gl_version_major)

void _plInitOpenGL() {
    _PL_GRAPHICS_TRACK();

    PLuint err = glewInit();
    if (err != GLEW_OK)
        plGraphicsLog("Failed to initialize GLEW!\n%s\n", glewGetErrorString(err));

    // Check that the required capabilities are supported.
    if (GLEW_ARB_multitexture) pl_gl_multitexture = PL_TRUE;
    else
        plGraphicsLog("Video hardware incapable of multi-texturing!\n");
    if (GLEW_ARB_texture_env_combine || GLEW_EXT_texture_env_combine) pl_gl_texture_env_combine = PL_TRUE;
    else
        plGraphicsLog("ARB/EXT_texture_env_combine isn't supported by your hardware!\n");
    if (GLEW_ARB_texture_env_add || GLEW_EXT_texture_env_add) pl_gl_texture_env_add = PL_TRUE;
    else
        plGraphicsLog("ARB/EXT_texture_env_add isn't supported by your hardware!\n");
    if (GLEW_ARB_vertex_program || GLEW_ARB_fragment_program) {
        pl_gl_vertex_program = PL_TRUE;
        pl_gl_fragment_program = PL_TRUE;
    }
    else
        plGraphicsLog("Shaders aren't supported by this hardware!\n");
    if (GLEW_SGIS_generate_mipmap) pl_gl_generate_mipmap = PL_TRUE;
    else
        plGraphicsLog("Hardware mipmap generation isn't supported!\n");
    if (GLEW_ARB_depth_texture) pl_gl_depth_texture = PL_TRUE;
    else
        plGraphicsLog("ARB_depth_texture isn't supported by your hardware!\n");
    if (GLEW_ARB_shadow) pl_gl_shadow = PL_TRUE;
    else
        plGraphicsLog("ARB_shadow isn't supported by your hardware!\n");
    if (GLEW_ARB_vertex_buffer_object) pl_gl_vertex_buffer_object = PL_TRUE;
    else
        plGraphicsLog("Hardware doesn't support Vertex Buffer Objects!\n");

    // If HW compression isn't supported then we'll need to do
    // all of this in software later.
    if (GLEW_ARB_texture_compression) {
        if (GLEW_EXT_texture_compression_s3tc) pl_gl_texture_compression_s3tc = PL_TRUE;
    }

    const PLchar *version = _plGetHWVersion();
    pl_gl_version_major = (unsigned int) atoi(&version[0]);
    pl_gl_version_minor = (unsigned int) atoi(&version[2]);
}

void _plShutdownOpenGL() {}

#endif

PLresult plInitGraphics(void) {
    _PL_GRAPHICS_TRACK();

    plGraphicsLog("Initializing graphics abstraction layer...\n");

    memset(&pl_graphics_state, 0, sizeof(PLGraphicsState));

#if defined (VL_MODE_OPENGL)
    _plInitOpenGL();
#elif defined (VL_MODE_GLIDE)
    _plInitGlide();
#elif defined (VL_MODE_DIRECT3D)
    _plInitDirect3D();
#endif

    pl_graphics_state.tmu = (PLTextureMappingUnit *) calloc(plGetMaxTextureUnits(), sizeof(PLTextureMappingUnit));
    if (!pl_graphics_state.tmu) {
        plSetError("Failed to allocate memory for texture units! (%i)\n", plGetMaxTextureUnits());
        return PL_RESULT_MEMORYALLOC;
    }
    memset(pl_graphics_state.tmu, 0, sizeof(PLTextureMappingUnit));
    for (PLuint i = 0; i < plGetMaxTextureUnits(); i++)
        pl_graphics_state.tmu[i].current_envmode = PL_TEXTUREMODE_REPLACE;

    // Get any information that will be presented later.
    pl_graphics_state.hw_extensions = _plGetHWExtensions();
    pl_graphics_state.hw_renderer = _plGetHWRenderer();
    pl_graphics_state.hw_vendor = _plGetHWVendor();
    pl_graphics_state.hw_version = _plGetHWVersion();
    plGraphicsLog(" HARDWARE/DRIVER INFORMATION\n");
    plGraphicsLog("  RENDERER: %s\n", pl_graphics_state.hw_renderer);
    plGraphicsLog("  VENDOR:   %s\n", pl_graphics_state.hw_vendor);
    plGraphicsLog("  VERSION:  %s\n\n", pl_graphics_state.hw_version);

    return PL_RESULT_SUCCESS;
}

void plShutdownGraphics(void) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL)
#elif defined (VL_MODE_GLIDE)
#elif defined (VL_MODE_DIRECT3D)
    _plShutdownDirect3D();
#endif
}

/*===========================
	HARDWARE INFORMATION
===========================*/

const PLchar *_plGetHWExtensions(void) {
    _PL_GRAPHICS_TRACK();

#if defined(VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    return (const PLchar *) glGetString(GL_EXTENSIONS);
    // TODO: this works differently in core; use glGetStringi instead!
#elif defined (VL_MODE_GLIDE)
    return grGetString(GR_EXTENSION);
#else
    return "";
#endif
}

const PLchar *_plGetHWRenderer(void) {
    _PL_GRAPHICS_TRACK();

#if defined(VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    return (const PLchar *) glGetString(GL_RENDERER);
#elif defined (VL_MODE_GLIDE)
    return grGetString(GR_RENDERER);
#else
    return "";
#endif
}

const PLchar *_plGetHWVendor(void) {
    _PL_GRAPHICS_TRACK();

#if defined(VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    return (const PLchar *) glGetString(GL_VENDOR);
#elif defined (VL_MODE_GLIDE)
    return grGetString(GR_VENDOR);
#else
    return "";
#endif
}

const PLchar *_plGetHWVersion(void) {
    _PL_GRAPHICS_TRACK();

#if defined(VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    return (const PLchar *) glGetString(GL_VERSION);
#elif defined (VL_MODE_GLIDE)
    return grGetString(GR_VERSION);
#else
    return "";
#endif
}

PLbool plHWSupportsMultitexture(void) {
    _PL_GRAPHICS_TRACK();

#if defined(VL_MODE_OPENGL)
    return pl_gl_multitexture;
#elif defined(VL_MODE_GLIDE)
    return true;
#else
    return false;
#endif
}

PLbool plHWSupportsShaders(void) {
    _PL_GRAPHICS_TRACK();

#if defined(VL_MODE_OPENGL)
    return (pl_gl_fragment_program && pl_gl_vertex_program);
#else
    return false;
#endif
}

/*===========================
	FRAMEBUFFERS
===========================*/

void plSetClearColour3f(PLfloat r, PLfloat g, PLfloat b) {
    _PL_GRAPHICS_TRACK();

    plSetClearColour4f(r, g, b, pl_graphics_state.current_clearcolour.a);
}

void plSetClearColour4f(PLfloat r, PLfloat g, PLfloat b, PLfloat a) {
    _PL_GRAPHICS_TRACK();

    plSetClearColour(PLColour(r, g, b, a));
}

void plSetClearColour4fv(PLColourf rgba) {
    plSetClearColour4f(rgba[0], rgba[1], rgba[2], rgba[3]);
}

void plSetClearColour(PLColour rgba) {
    if (rgba == pl_graphics_state.current_clearcolour)
        return;

#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    glClearColor(rgba.r / 255, rgba.g / 255, rgba.b / 255, rgba.a / 255);
#elif defined (VL_MODE_DIRECT3D)
    // Don't need to do anything specific here, colour is set on clear call.
#else // Software
#endif

    pl_graphics_state.current_clearcolour = rgba;
}

void plClearBuffers(PLuint buffers) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    // Rather ugly, but translate it over to GL.
    PLuint glclear = 0;
    if(buffers & PL_BUFFER_COLOUR)  glclear |= GL_COLOR_BUFFER_BIT;
    if(buffers & PL_BUFFER_DEPTH)   glclear |= GL_DEPTH_BUFFER_BIT;
    if(buffers & PL_BUFFER_STENCIL) glclear |= GL_STENCIL_BUFFER_BIT;

    glClear(buffers);
#elif defined (VL_MODE_GLIDE)
    // Glide only supports clearing a single buffer.
    grBufferClear(
        // Convert buffer_clearcolour to something that works with Glide.
        _plConvertColour4fv(VL_COLOURFORMAT_RGBA, graphics_state.buffer_clearcolour),
        1, 1);
#elif defined (VL_MODE_DIRECT3D)
    pl_d3d_context->lpVtbl->ClearRenderTargetView(pl_d3d_context,
        pl_d3d_backbuffer,
        graphics_state.current_clearcolour
    );
#else // Software
#endif
}

/*===========================
	CAPABILITIES
===========================*/

typedef struct _PLGraphicsCapabilities {
    unsigned int pl_parm, to_parm;

    const char *ident;
} _PLGraphicsCapabilities;

_PLGraphicsCapabilities graphics_capabilities[] =
        {
#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
                {PL_CAPABILITY_ALPHA_TEST, GL_ALPHA_TEST, "ALPHA_TEST"},
                {PL_CAPABILITY_BLEND, GL_BLEND, "BLEND"},
                {VL_CAPABILITY_DEPTH_TEST, GL_DEPTH_TEST, "DEPTH_TEST"},
                {PL_CAPABILITY_TEXTURE_2D, GL_TEXTURE_2D, "TEXTURE_2D"},
                {VL_CAPABILITY_TEXTURE_GEN_S, GL_TEXTURE_GEN_S, "TEXTURE_GEN_S"},
                {VL_CAPABILITY_TEXTURE_GEN_T, GL_TEXTURE_GEN_T, "TEXTURE_GEN_T"},
                {VL_CAPABILITY_CULL_FACE, GL_CULL_FACE, "CULL_FACE"},
                {VL_CAPABILITY_STENCIL_TEST, GL_STENCIL_TEST, "STENCIL_TEST"},
                {VL_CAPABILITY_MULTISAMPLE, GL_MULTISAMPLE, "MULTISAMPLE"},
                {PL_CAPABILITY_SCISSORTEST, GL_SCISSOR_TEST, "SCISSOR_TEST"},

                {PL_CAPABILITY_GENERATEMIPMAP, 0, "GENERATE_MIPMAP"},
#else
        { PL_CAPABILITY_ALPHA_TEST, 0, "ALPHA_TEST" },
        { VL_CAPABILITY_BLEND, 0, "BLEND" },
        { VL_CAPABILITY_DEPTH_TEST, 0, "DEPTH_TEST" },
        { VL_CAPABILITY_TEXTURE_2D, 0, "TEXTURE_2D" },
        { VL_CAPABILITY_TEXTURE_GEN_S, 0, "TEXTURE_GEN_S" },
        { VL_CAPABILITY_TEXTURE_GEN_T, 0, "TEXTURE_GEN_T" },
        { VL_CAPABILITY_CULL_FACE, 0, "CULL_FACE" },
        { VL_CAPABILITY_STENCIL_TEST, 0, "STENCIL_TEST" },
        { VL_CAPABILITY_MULTISAMPLE, 0, "MULTISAMPLE" },
#endif

                {0}
        };

PLbool plIsGraphicsStateEnabled(PLuint flags) {
    _PL_GRAPHICS_TRACK();

    if (flags & pl_graphics_state.current_capabilities)
        return PL_TRUE;

    return PL_FALSE;
}

void plEnableGraphicsStates(PLuint flags) {
    _PL_GRAPHICS_TRACK();

    if (plIsGraphicsStateEnabled(flags))
        return;

    for (PLint i = 0; i < sizeof(graphics_capabilities); i++) {
        if (graphics_capabilities[i].pl_parm == 0) break;

        if (pl_graphics_state.mode_debug)
            plGraphicsLog("Enabling %s\n", graphics_capabilities[i].ident);

        if (flags & PL_CAPABILITY_TEXTURE_2D)
            pl_graphics_state.tmu[pl_graphics_state.current_textureunit].active = PL_TRUE;
#if defined (VL_MODE_GLIDE)
        if (flags & VL_CAPABILITY_FOG)
            // TODO: need to check this is supported...
            grFogMode(GR_FOG_WITH_TABLE_ON_FOGCOORD_EXT);
        if (flags & VL_CAPABILITY_DEPTH_TEST)
            grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
        if (flags & VL_CAPABILITY_CULL_FACE)
            grCullMode(graphics_state.current_cullmode);
#endif

        if ((flags & graphics_capabilities[i].pl_parm) && (graphics_capabilities[i].to_parm != 0))
#if defined (VL_MODE_OPENGL) || (VL_MODE_OPENGL_CORE)
            glEnable(graphics_capabilities[i].to_parm);
#elif defined (VL_MODE_GLIDE)
        grEnable(graphics_capabilities[i].to_parm);
#endif

        pl_graphics_state.current_capabilities |= graphics_capabilities[i].pl_parm;
    }
}

void plDisableGraphicsStates(PLuint flags) {
    _PL_GRAPHICS_TRACK();

    if (!plIsGraphicsStateEnabled(flags))
        return;

    for (PLint i = 0; i < sizeof(graphics_capabilities); i++) {
        if (graphics_capabilities[i].pl_parm == 0) break;

        if (pl_graphics_state.mode_debug)
            plGraphicsLog("Disabling %s\n", graphics_capabilities[i].ident);

        if (flags & PL_CAPABILITY_TEXTURE_2D)
            pl_graphics_state.tmu[pl_graphics_state.current_textureunit].active = PL_FALSE;
#if defined (VL_MODE_GLIDE)
        if (flags & VL_CAPABILITY_FOG)
            grFogMode(GR_FOG_DISABLE);
        if (flags & VL_CAPABILITY_DEPTH_TEST)
            grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
        if (flags & VL_CAPABILITY_CULL_FACE)
            grCullMode(graphics_state.current_cullmode);
#endif

        if ((flags & graphics_capabilities[i].pl_parm) && (graphics_capabilities[i].to_parm != 0))
#if defined (VL_MODE_OPENGL) || (VL_MODE_OPENGL_CORE)
            glDisable(graphics_capabilities[i].to_parm);
#elif defined (VL_MODE_GLIDE)
        grDisable(graphics_capabilities[i].to_parm);
#endif

        pl_graphics_state.current_capabilities &= ~graphics_capabilities[i].pl_parm;
    }
}

/*===========================
	DRAW
===========================*/

void plSetBlendMode(PLBlend modea, PLBlend modeb) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    glBlendFunc(modea, modeb);
#elif defined (VL_MODE_GLIDE)
    grAlphaBlendFunction(modea, modeb, modea, modeb);
#endif
}

void plSetCullMode(VLCullMode mode) {
    _PL_GRAPHICS_TRACK();

    if (mode == pl_graphics_state.current_cullmode)
        return;
#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    glCullFace(GL_BACK);
    switch (mode) {
        default:
        case VL_CULL_NEGATIVE:
            glFrontFace(GL_CW);
            break;
        case VL_CULL_POSTIVE:
            glFrontFace(GL_CCW);
            break;
    }
#elif defined (VL_MODE_DIRECT3D)
    // todo, create new render state and somehow get the properties of the
    // current but update them to reflect the new cull mode.

    vl_d3d_context->lpVtbl->RSSetState(vl_d3d_context, vl_d3d_state);
#endif
    pl_graphics_state.current_cullmode = mode;
}

typedef struct PLTranslatePrimitive {
    PLPrimitive mode;

    PLuint target;

    const PLchar *name;
} PLTranslatePrimitive;

PLTranslatePrimitive _pl_primitives[] = {
#if defined (VL_MODE_OPENGL) || (VL_MODE_OPENGL_CORE)
        {VL_PRIMITIVE_LINES, GL_LINES, "LINES"},
        {VL_PRIMITIVE_POINTS, GL_POINTS, "POINTS"},
        {VL_PRIMITIVE_TRIANGLES, GL_TRIANGLES, "TRIANGLES"},
        {VL_PRIMITIVE_TRIANGLE_FAN, GL_TRIANGLE_FAN, "TRIANGLE_FAN"},
        {VL_PRIMITIVE_TRIANGLE_FAN_LINE, GL_LINES, "TRIANGLE_FAN_LINE"},
        {VL_PRIMITIVE_TRIANGLE_STRIP, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP"},
        {VL_PRIMITIVE_QUADS, GL_QUADS, "QUADS"}
#elif defined (VL_MODE_GLIDE)
{ VL_PRIMITIVE_LINES,					GR_LINES,			"LINES" },
    { VL_PRIMITIVE_LINE_STRIP,				GR_LINE_STRIP,		"LINE_STRIP" },
    { VL_PRIMITIVE_POINTS,					GR_POINTS,			"POINTS" },
    { VL_PRIMITIVE_TRIANGLES,				GR_TRIANGLES,		"TRIANGLES" },
    { VL_PRIMITIVE_TRIANGLE_FAN,			GR_TRIANGLE_FAN,	"TRIANGLE_FAN" },
    { VL_PRIMITIVE_TRIANGLE_FAN_LINE,		GR_LINES,			"TRIANGLE_FAN_LINE" },
    { VL_PRIMITIVE_TRIANGLE_STRIP,			GR_TRIANGLE_STRIP,	"TRIANGLE_STRIP" },
    { VL_PRIMITIVE_QUADS,					0,					"QUADS" }
#elif defined (VL_MODE_DIRECT3D)
#elif defined (VL_MODE_VULKAN)
    { VL_PRIMITIVE_LINES,					VK_PRIMITIVE_TOPOLOGY_LINE_LIST,		"LINES" },
    { VL_PRIMITIVE_POINTS,					VK_PRIMITIVE_TOPOLOGY_POINT_LIST,		"POINTS" },
    { VL_PRIMITIVE_TRIANGLES,				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,	"TRIANGLES" },
    { VL_PRIMITIVE_TRIANGLE_FAN,			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,		"TRIANGLE_FAN" },
    { VL_PRIMITIVE_TRIANGLE_FAN_LINE,		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,		"TRIANGLE_FAN_LINE" },
    { VL_PRIMITIVE_TRIANGLE_STRIP,			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,	"TRIANGLE_STRIP" },
    { VL_PRIMITIVE_QUADS,					0,										"QUADS" }
#else
    { 0 }
#endif
};

PLuint _plTranslatePrimitiveMode(PLPrimitive mode) {
    _PL_GRAPHICS_TRACK();

    for (PLint i = 0; i < plArrayElements(_pl_primitives); i++)
        if (mode == _pl_primitives[i].mode)
            return _pl_primitives[i].target;

    // Hacky, but just return initial otherwise.
    return _pl_primitives[0].target;
}

void _plDrawArrays(PLPrimitive mode, PLuint first, PLuint count) {
    _PL_GRAPHICS_TRACK();

    if ((count == 0) || (first > count)) return;
#if defined(VL_MODE_OPENGL)
    glDrawArrays(_plTranslatePrimitiveMode(mode), first, count);
#endif
}

void _plDrawElements(PLPrimitive mode, PLuint count, PLuint type, const PLvoid *indices) {
    _PL_GRAPHICS_TRACK();

    if ((count == 0) || !indices) return;
#if defined(VL_MODE_OPENGL)
    glDrawElements(_plTranslatePrimitiveMode(mode), count, type, indices);
#endif
}

void plDraw(PLDraw *draw) {
    _PL_GRAPHICS_TRACK();
#if 0 // update this :(
    if(!draw)
        return;
    else if(draw->numverts == 0)
        return;

#if defined(VL_MODE_GLIDE)
    // todo, glide needs its own setup here...
#elif defined(VL_MODE_OPENGL)
#if 0
    if(draw->_gl_vbo[_VL_BUFFER_VERTICES] != 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, draw->_gl_vbo[_VL_BUFFER_VERTICES]);
        glVertexPointer(3, GL_FLOAT, sizeof(PLVertex), NULL);

        glBindBuffer(GL_ARRAY_BUFFER, draw->_gl_vbo[_VL_BUFFER_TEXCOORDS]);
        glTexCoordPointer(2, GL_FLOAT, sizeof(PLVertex), NULL);

        // todo, switch to using glInterleavedArrays?

        if (draw->primitive == VL_PRIMITIVE_TRIANGLES)
            _vlDrawElements(
                draw->primitive,
                draw->numtriangles * 3,
                GL_UNSIGNED_BYTE,
                draw->indices
            );
        else
            _vlDrawArrays(draw->primitive, 0, draw->numverts);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
#endif
    {
#if !defined(VL_MODE_OPENGL_CORE)   // Immediate mode isn't support in core!
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        PLVertex *vert = &draw->vertices[0];
        glVertexPointer(3, GL_FLOAT, sizeof(PLVertex), vert->position);
        glColorPointer(4, GL_FLOAT, sizeof(PLVertex), vert->colour);
        glNormalPointer(GL_FLOAT, sizeof(PLVertex), vert->normal);
        for(PLint i = 0; i < plGetMaxTextureUnits(); i++)
            if(pl_graphics_state.tmu[i].active)
            {
                glClientActiveTexture((GLenum)GL_TEXTURE0 + i);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, sizeof(PLVertex), vert->ST[i]);
            }

        if(draw->primitive == VL_PRIMITIVE_TRIANGLES)
            _plDrawElements
            (
                    draw->primitive,
                    draw->numtriangles * 3,
                    GL_UNSIGNED_BYTE,
                    draw->indices
            );
        else
            _plDrawArrays(draw->primitive, 0, draw->numverts);

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        for(PLint i = 0; i < plGetMaxTextureUnits(); i++)
            if(pl_graphics_state.tmu[i].active)
            {
                glClientActiveTexture((GLenum)GL_TEXTURE0 + i);
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
#endif
    }
#endif
#endif
}

void plDrawVertexNormals(PLDraw *draw) {
    if (draw->primitive == VL_PRIMITIVE_LINES)
        return;

    PLVector3D endpos;
    for (PLuint i = 0; i < draw->numverts; i++) {
        endpos = (draw->vertices[i].normal * 2.0f) + draw->vertices[i].position;
        //plDrawLine blah
    }
}

/*===========================
	SHADERS
===========================*/

void plCreateShader(PLShader *shader, PLShaderType type) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL)
    *shader = glCreateShader(type);
#endif
}

void plDeleteShader(PLShader *shader) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL)
    glDeleteShader(*shader);
#endif
}

void plCreateShaderProgram(PLShaderProgram *program) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL)
    *program = glCreateProgram();
#endif
}

void plDeleteShaderProgram(PLShaderProgram *program) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL)
    glDeleteProgram(*program);
#endif
}

PLShaderProgram plGetCurrentShaderProgram(void) {
    _PL_GRAPHICS_TRACK();

    return pl_graphics_state.current_program;
}

void plSetShaderProgram(PLShaderProgram program) {
    _PL_GRAPHICS_TRACK();

    if (program == pl_graphics_state.current_program)
        return;

#if defined (VL_MODE_OPENGL)
    glUseProgram(program);
#endif

    pl_graphics_state.current_program = program;
}

/*  Platform Texture Manager    */

PLuint _plTranslateTextureUnit(PLuint target) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL) || defined(VL_MODE_OPENGL_CORE)
    PLuint out = GL_TEXTURE0 + target;
    if (out > (GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS - 1))
        plGraphicsLog("Attempted to select an invalid texture image unit! (%i)\n", target);
    return out;
#else
    return 0;
#endif
}

PLuint _plTranslateTextureStorageFormat(PLDataFormat format) {
    _PL_GRAPHICS_TRACK();

#if defined(VL_MODE_OPENGL) || defined(VL_MODE_OPENGL_CORE)
    switch (format) {
        default:
        case PL_UNSIGNED_BYTE:              return GL_UNSIGNED_BYTE;
        case PL_UNSIGNED_INT_8_8_8_8_REV:   return GL_UNSIGNED_INT_8_8_8_8_REV;
    }
#else
    return format;
#endif
}

PLuint _plTranslateTextureTarget(PLTextureTarget target) {
    _PL_GRAPHICS_TRACK();

#if defined(VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    switch (target) {
        default:
        case PL_TEXTURE_2D: return GL_TEXTURE_2D;
        case PL_TEXTURE_1D: return GL_TEXTURE_1D;
        case PL_TEXTURE_3D: return GL_TEXTURE_3D;
    }
#else
    return (PLuint)format;
#endif
}

PLuint _plTranslateTextureEnvironmentMode(PLTextureEnvironmentMode mode) {
#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    switch (mode) {
        default:
        case PL_TEXTUREMODE_ADD:        return GL_ADD;
        case PL_TEXTUREMODE_MODULATE:   return GL_MODULATE;
        case PL_TEXTUREMODE_DECAL:      return GL_DECAL;
        case PL_TEXTUREMODE_BLEND:      return GL_BLEND;
        case PL_TEXTUREMODE_REPLACE:    return GL_REPLACE;
        case PL_TEXTUREMODE_COMBINE:    return GL_COMBINE;
    }
#elif defined (VL_MODE_GLIDE)
#elif defined (VL_MODE_DIRECT3D)
#else
    // No translation if we're doing this in SW.
    return (PLuint)format;
#endif
}

PLuint _plTranslateTextureFormat(PLTextureFormat format) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    switch (format) {
        default:
        case PL_TEXTUREFORMAT_RGB4:         return GL_RGB4;
        case PL_TEXTUREFORMAT_RGBA4:        return GL_RGBA4;
        case PL_TEXTUREFORMAT_RGB5:         return GL_RGB5;
        case PL_TEXTUREFORMAT_RGB5A1:       return GL_RGB5_A1;
        case PL_TEXTUREFORMAT_RGB565:       return GL_RGB565;
        case VL_TEXTUREFORMAT_RGB8:         return GL_RGB8;
        case PL_TEXTUREFORMAT_RGBA8:        return GL_RGBA8;
        case PL_TEXTUREFORMAT_RGBA12:       return GL_RGBA12;
        case PL_TEXTUREFORMAT_RGBA16:       return GL_RGBA16;
        case PL_TEXTUREFORMAT_RGBA16F:      return GL_RGBA16F;
        case VL_TEXTUREFORMAT_RGBA_DXT1:    return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        case VL_TEXTUREFORMAT_RGB_DXT1:     return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case VL_TEXTUREFORMAT_RGBA_DXT3:    return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        case VL_TEXTUREFORMAT_RGBA_DXT5:    return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case VL_TEXTUREFORMAT_RGB_FXT1:     return GL_COMPRESSED_RGB_FXT1_3DFX;
    }
#elif defined (VL_MODE_GLIDE)
#elif defined (VL_MODE_DIRECT3D)
#else
    // No translation if we're doing this in SW.
    return (PLuint)format;
#endif
}

PLbool _plIsCompressedTextureFormat(PLTextureFormat format) {
    _PL_GRAPHICS_TRACK();

    switch (format) {
        default:return PL_FALSE;
        case VL_TEXTUREFORMAT_RGBA_DXT1:
        case VL_TEXTUREFORMAT_RGBA_DXT3:
        case VL_TEXTUREFORMAT_RGBA_DXT5:
        case VL_TEXTUREFORMAT_RGB_DXT1:
        case VL_TEXTUREFORMAT_RGB_FXT1:
            return PL_TRUE;
    }
}

std::map<PLuint, PLTexture*> pl_graphics_textures;

PLTexture *plCreateTexture(void) {
    _PL_GRAPHICS_TRACK();

    PLTexture *texture = new PLTexture;
    memset(texture, 0, sizeof(PLTexture));
    texture->format = PL_TEXTUREFORMAT_RGBA8;
    texture->width = 8;
    texture->height = 8;

#if defined(VL_MODE_OPENGL)
    glGenTextures(1, &texture->id);
#else
    texture->id = pl_graphics_textures.count + 1;
#endif

    pl_graphics_textures.emplace(texture->id, texture);

    return texture;
}

void plDeleteTexture(PLTexture *texture, PLbool force) {
    _PL_GRAPHICS_TRACK();

    if(!texture || ((texture->flags & PL_TEXTUREFLAG_PRESERVE) && !force)) {
        return;
    }

    auto tex = pl_graphics_textures.begin();
    while(tex != pl_graphics_textures.end()) {
        if(tex->second == texture) {
#if defined(VL_MODE_OPENGL)
            glDeleteTextures(1, &texture->id);
#endif

            delete tex->second;
            pl_graphics_textures.erase(tex);
            return;
        }
        ++tex;
    }
}

#define _PL_TEXTURE_LEVELS  4   // Default number of mipmap levels.

PLresult plUploadTextureImage(PLTexture *texture, const PLImage *upload) {
    _PL_GRAPHICS_TRACK();

    plSetTexture(texture);

#if defined(VL_MODE_OPENGL) || defined(VL_MODE_OPENGL_CORE)
    PLuint levels = upload->levels;
    if(plIsGraphicsStateEnabled(PL_CAPABILITY_GENERATEMIPMAP) && (levels <= 1)) {
        levels = _PL_TEXTURE_LEVELS;
    }

    PLuint format = _plTranslateTextureFormat(upload->format);
    glTexStorage2D(GL_TEXTURE_2D, levels, format, upload->width, upload->height);

    // Check the format, to see if we're getting a compressed
    // format type.
    if (_plIsCompressedTextureFormat(upload->format)) {
        glCompressedTexSubImage2D
                (
                        GL_TEXTURE_2D,
                        0,
                        upload->x, upload->y,
                        upload->width, upload->height,
                        format,
                        upload->size,
                        upload->data
                );
    } else {
        glTexSubImage2D
                (
                        GL_TEXTURE_2D,
                        0,
                        upload->x, upload->y,
                        upload->width, upload->height,
                        upload->colour_format,
                        GL_UNSIGNED_BYTE,
                        upload->data
                );
    }

    if(plIsGraphicsStateEnabled(PL_CAPABILITY_GENERATEMIPMAP) && (upload->levels <= 1))
        glGenerateMipmap(GL_TEXTURE_2D);
#endif

    return PL_RESULT_SUCCESS;
}

PLresult plUploadTexture(PLTexture *texture, const PLTextureInfo *upload) {
    _PL_GRAPHICS_TRACK();

    plSetTexture(texture);

#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    PLuint storage = _plTranslateTextureStorageFormat(upload->storage_type);
    PLuint format = _plTranslateTextureFormat(upload->format);

    PLuint levels = upload->levels;
    if (plIsGraphicsStateEnabled(PL_CAPABILITY_GENERATEMIPMAP))
        if (levels <= 1) levels = _PL_TEXTURE_LEVELS;

    if (upload->initial)
        glTexStorage2D(GL_TEXTURE_2D, levels, format, upload->width, upload->height);

    // Check the format, to see if we're getting a compressed
    // format type.
    if (_plIsCompressedTextureFormat(upload->format))
        glCompressedTexSubImage2D
                (
                        GL_TEXTURE_2D,
                        0,
                        upload->x, upload->y,
                        upload->width, upload->height,
                        format,
                        upload->size,
                        upload->data
                );
    else
        glTexSubImage2D
                (
                        GL_TEXTURE_2D,
                        0,
                        upload->x, upload->y,
                        upload->width, upload->height,
                        upload->pixel_format,
                        storage,
                        upload->data
                );

    if (plIsGraphicsStateEnabled(PL_CAPABILITY_GENERATEMIPMAP))
        glGenerateMipmap(GL_TEXTURE_2D);
#elif defined (VL_MODE_GLIDE)
#elif defined (VL_MODE_DIRECT3D)
#endif

    return PL_RESULT_SUCCESS;
}

PLTexture *plGetCurrentTexture(PLuint tmu) {
    _PL_GRAPHICS_TRACK();

    return pl_graphics_textures.find(pl_graphics_state.tmu[tmu].current_texture)->second;
}

PLuint plGetCurrentTextureUnit(void) {
    _PL_GRAPHICS_TRACK();

    return pl_graphics_state.current_textureunit;
}

#if defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#endif

PLuint plGetMaxTextureSize(void) {
    _PL_GRAPHICS_TRACK();
    if (pl_graphics_state.hw_maxtexturesize != 0)
        return pl_graphics_state.hw_maxtexturesize;

#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    PLint num_size = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &num_size);
    pl_graphics_state.hw_maxtexturesize = (PLuint) num_size;
#elif defined (VL_MODE_GLIDE)
    grGet(GR_MAX_TEXTURE_SIZE, sizeof(pl_graphics_state.hw_maxtexturesize), &pl_graphics_state.hw_maxtexturesize);
#endif

    return pl_graphics_state.hw_maxtexturesize;
}

PLuint plGetMaxTextureUnits(void) {
    _PL_GRAPHICS_TRACK();
    if (pl_graphics_state.hw_maxtextureunits != 0)
        return pl_graphics_state.hw_maxtextureunits;

#ifdef VL_MODE_OPENGL
    PLint num_units = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &num_units);
    pl_graphics_state.hw_maxtextureunits = (PLuint) num_units;
#elif defined (VL_MODE_GLIDE)
    grGet(GR_NUM_TMU, sizeof(param), (FxI32*)graphics_state.hw_maxtextureunits);
#endif

    return pl_graphics_state.hw_maxtextureunits;
}

PLuint plGetMaxTextureAnistropy(void) {
    _PL_GRAPHICS_TRACK();
    if (pl_graphics_state.hw_maxtextureanistropy != 0)
        return pl_graphics_state.hw_maxtextureanistropy;

#if defined (VL_MODE_OPENGL)
    PLfloat num_anisotropy = 0;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &num_anisotropy);
    pl_graphics_state.hw_maxtextureanistropy = (PLuint) num_anisotropy;
#endif

    return pl_graphics_state.hw_maxtextureanistropy;
}

#if defined(__GNUC__) || defined(__GNUG__)
#   pragma GCC diagnostic pop
#endif

PLresult plSetTexture(PLTexture *texture) {
    _PL_GRAPHICS_TRACK();

    if (texture->id == pl_graphics_state.tmu[plGetCurrentTextureUnit()].current_texture) {
        return PL_RESULT_SUCCESS;
    }

#if defined (VL_MODE_OPENGL)
    glBindTexture(GL_TEXTURE_2D, texture->id);
#endif

    pl_graphics_state.tmu[plGetCurrentTextureUnit()].current_texture = texture->id;

    return PL_RESULT_SUCCESS;
}

void plSetTextureUnit(PLuint target) {
    _PL_GRAPHICS_TRACK();
    if (target == pl_graphics_state.current_textureunit)
        return;
    else if (target > plGetMaxTextureUnits()) {
        plGraphicsLog("Attempted to select a texture image unit beyond what's supported by your hardware! (%i)\n",
                      target);
        return;
    }

#if defined (VL_MODE_OPENGL)
    glActiveTexture(_plTranslateTextureUnit(target));
#endif

    pl_graphics_state.current_textureunit = target;
}

PLresult plSetTextureAnisotropy(PLTexture *texture, PLuint amount) {
    _PL_GRAPHICS_TRACK();

    plSetTexture(texture);

#if defined (VL_MODE_OPENGL)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (PLint) amount);
#endif

    return PL_RESULT_SUCCESS;
}

PLresult plSetTextureFilter(PLTexture *texture, PLTextureFilter filter) {
    _PL_GRAPHICS_TRACK();

    plSetTexture(texture);

#ifdef VL_MODE_OPENGL
    PLuint filtermin, filtermax;
    switch (filter) {
        default:
        case PL_TEXTUREFILTER_NEAREST:
            filtermax = GL_NEAREST;
            filtermin = GL_NEAREST;
            break;
        case PL_TEXTUREFILTER_LINEAR:
            filtermax = GL_LINEAR;
            filtermin = GL_LINEAR;
            break;
        case PL_TEXTUREFILTER_MIPMAP_LINEAR:
            filtermax = GL_LINEAR;
            filtermin = GL_LINEAR_MIPMAP_LINEAR;
            break;
        case PL_TEXTUREFILTER_MIPMAP_NEAREST:
            filtermax = GL_NEAREST;
            filtermin = GL_NEAREST_MIPMAP_NEAREST;
            break;
        case PL_TEXTUREFILTER_MIPMAP_LINEAR_NEAREST:
            filtermax = GL_LINEAR;
            filtermin = GL_LINEAR_MIPMAP_NEAREST;
            break;
        case PL_TEXTUREFILTER_MIPMAP_NEAREST_LINEAR:
            filtermax = GL_NEAREST;
            filtermin = GL_NEAREST_MIPMAP_LINEAR;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtermin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtermax);
#elif defined(VL_MODE_GLIDE)
    // NEAREST > GR_TEXTUREFILTER_POINT_SAMPLED
    // LINEAR > GR_TEXTUREFILTER_BILINEAR
    // todo, glide implementation
#endif

    return PL_RESULT_SUCCESS;
}

void plSetTextureEnvironmentMode(PLTextureEnvironmentMode mode) {
    _PL_GRAPHICS_TRACK();

    if (pl_graphics_state.tmu[plGetCurrentTextureUnit()].current_envmode == mode)
        return;

#if defined(VL_MODE_OPENGL)
    glTexEnvi
            (
                    GL_TEXTURE_ENV,
                    GL_TEXTURE_ENV_MODE,
                    _plTranslateTextureEnvironmentMode(mode)
            );
#endif

    pl_graphics_state.tmu[plGetCurrentTextureUnit()].current_envmode = mode;
}

/*===========================
	LIGHTING
===========================*/

void plApplyLighting(PLDraw *object, PLLight *light, PLVector3f position) {
#if 0
    // Calculate the distance.
    PLVector3f distvec = { 0 };
    plVectorSubtract3fv(position, light->position, distvec);
    float distance = (light->radius - plLengthf(distvec)) / 100.0f;

    for(PLuint i = 0; i < object->numverts; i++)
    {
        float x = object->vertices[i].normal[0];
        float y = object->vertices[i].normal[1];
        float z = object->vertices[i].normal[2];

        float angle = (distance * ((x * distvec[0]) + (y * distvec[1]) + (z * distvec[2])));
        if(angle < 0)
            object->vertices[i].colour.Clear();
        else
        {
            object->vertices[i].colour[PL_RED]      = light->colour[PL_RED] * angle;
            object->vertices[i].colour[PL_GREEN]    = light->colour[PL_GREEN] * angle;
            object->vertices[i].colour[PL_BLUE]     = light->colour[PL_BLUE] * angle;
        }

        /*
        x = Object->Vertices_normalStat[count].x;
        y = Object->Vertices_normalStat[count].y;
        z = Object->Vertices_normalStat[count].z;

        angle = (LightDist*((x * Object->Spotlight.x) + (y * Object->Spotlight.y) + (z * Object->Spotlight.z) ));
        if (angle<0 )
        {
        Object->Vertices_screen[count].r = 0;
        Object->Vertices_screen[count].b = 0;
        Object->Vertices_screen[count].g = 0;
        }
        else
        {
        Object->Vertices_screen[count].r = Object->Vertices_local[count].r * angle;
        Object->Vertices_screen[count].b = Object->Vertices_local[count].b * angle;
        Object->Vertices_screen[count].g = Object->Vertices_local[count].g * angle;
        }
        */
    }
#endif
}

/*===========================
	UTILITY FUNCTIONS
===========================*/

void plViewport(PLint x, PLint y, PLuint width, PLuint height) {
    _PL_GRAPHICS_TRACK();

    if (((x == pl_graphics_state.viewport_x) &&
         (y == pl_graphics_state.viewport_y)) &&
        ((width == pl_graphics_state.viewport_width) &&
         (height == pl_graphics_state.viewport_height)))
        return;

#if defined (VL_MODE_OPENGL)
    glViewport(x, y, width, height);

    pl_graphics_state.viewport_x = x;
    pl_graphics_state.viewport_y = y;
    pl_graphics_state.viewport_width = width;
    pl_graphics_state.viewport_height = height;
#elif defined (VL_MODE_DIRECT3D)
    D3D11_VIEWPORT viewport;
    memset(&viewport, 0, sizeof(D3D11_VIEWPORT));

    pl_graphics_state.viewport_x = viewport.TopLeftX = x;
    pl_graphics_state.viewport_y = viewport.TopLeftY = y;
    pl_graphics_state.viewport_width = viewport.Width = width;
    pl_graphics_state.viewport_height = viewport.Height = height;

    vl_d3d_context->lpVtbl->RSSetViewports(vl_d3d_context, 1, &viewport);
#endif
}

void plScissor(PLint x, PLint y, PLuint width, PLuint height) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    glScissor(x, y, width, height);
#elif defined (VL_MODE_DIRECT3D)
    D3D11_RECT scissor_region;
    memset(&scissor_region, 0, sizeof(D3D11_RECT));
    scissor_region.bottom	= height;
    scissor_region.right	= width;
    vl_d3d_context->lpVtbl->RSSetScissorRects(vl_d3d_context, 0, &scissor_region);
#endif
}

void plFinish(void) {
    _PL_GRAPHICS_TRACK();

#if defined (VL_MODE_OPENGL) || defined (VL_MODE_OPENGL_CORE)
    glFinish();
#elif defined (VL_MODE_GLIDE)
    grFinish();
#elif defined (VL_MODE_DIRECT3D)
    // Not supported, or rather, we don't need this.
#endif
}