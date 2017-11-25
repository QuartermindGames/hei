
#include "graphics_private.h"

#if !defined(PL_MODE_OPENGL_CORE)
bool pl_gl_generate_mipmap              = false;
bool pl_gl_depth_texture                = false;
bool pl_gl_shadow                       = false;
bool pl_gl_vertex_buffer_object         = false;
bool pl_gl_texture_compression          = false;
bool pl_gl_texture_compression_s3tc     = false;
bool pl_gl_multitexture                 = false;
bool pl_gl_texture_env_combine          = false;
bool pl_gl_texture_env_add              = false;
bool pl_gl_vertex_program               = false;
bool pl_gl_fragment_program             = false;
#endif

int pl_gl_version_major = 0;
int pl_gl_version_minor = 0;

int pl_gl_num_extensions = 0;

/////////////////////////////////////////////////////////////

bool GLHWSupportsShaders(void) {
    return (pl_gl_fragment_program && pl_gl_vertex_program);
}

bool GLHWSupportsMultitexture(void) {
    return pl_gl_multitexture;
}

/////////////////////////////////////////////////////////////

void GLSetBlendMode(PLBlend a, PLBlend b) {
    glBlendFunc(a, b);
}

void GLSetCullMode(PLCullMode mode) {
    if(mode == gfx_state.current_cullmode) {
        return;
    }

    glCullFace(GL_BACK);
    switch(mode) {
        default:
        case PL_CULL_NEGATIVE: {
            glFrontFace(GL_CW);
        } break;

        case PL_CULL_POSTIVE: {
            glFrontFace(GL_CCW);
        } break;
    }
    gfx_state.current_cullmode = mode;
}

/////////////////////////////////////////////////////////////

char pl_gl_extensions[4096][4096] = { { '\0' } };

void InitOpenGL(void) {
#if 0
    #if !defined(PL_MODE_OPENGL_CORE)
    PLuint err = glewInit();
    if (err != GLEW_OK) {
        plGraphicsLog("Failed to initialize GLEW!\n%s\n", glewGetErrorString(err));
    }

    // Check that the required capabilities are supported.
    if (GLEW_ARB_multitexture) pl_gl_multitexture = true;
    else plGraphicsLog("Video hardware incapable of multi-texturing!\n");
    if (GLEW_ARB_texture_env_combine || GLEW_EXT_texture_env_combine) pl_gl_texture_env_combine = true;
    else plGraphicsLog("ARB/EXT_texture_env_combine isn't supported by your hardware!\n");
    if (GLEW_ARB_texture_env_add || GLEW_EXT_texture_env_add) pl_gl_texture_env_add = true;
    else plGraphicsLog("ARB/EXT_texture_env_add isn't supported by your hardware!\n");
    if (GLEW_ARB_vertex_program || GLEW_ARB_fragment_program) {
        pl_gl_vertex_program = true;
        pl_gl_fragment_program = true;
    }
    else plGraphicsLog("Shaders aren't supported by this hardware!\n");
    if (GLEW_SGIS_generate_mipmap) pl_gl_generate_mipmap = true;
    else plGraphicsLog("Hardware mipmap generation isn't supported!\n");
    if (GLEW_ARB_depth_texture) pl_gl_depth_texture = true;
    else plGraphicsLog("ARB_depth_texture isn't supported by your hardware!\n");
    if (GLEW_ARB_shadow) pl_gl_shadow = true;
    else plGraphicsLog("ARB_shadow isn't supported by your hardware!\n");
    if (GLEW_ARB_vertex_buffer_object) pl_gl_vertex_buffer_object = true;
    else plGraphicsLog("Hardware doesn't support Vertex Buffer Objects!\n");

    // If HW compression isn't supported then we'll need to do
    // all of this in software later.
    if (GLEW_ARB_texture_compression) {
        if (GLEW_EXT_texture_compression_s3tc) {
            pl_gl_texture_compression_s3tc = true;
        }
    }
#endif
#endif

    // setup the gfx layer
    gfx_layer.HWSupportsShaders         = GLHWSupportsShaders;
    gfx_layer.HWSupportsMultitexture    = GLHWSupportsMultitexture;
    gfx_layer.SetBlendMode              = GLSetBlendMode;
    gfx_layer.SetCullMode               = GLSetCullMode;

    // Get any information that will be presented later.
    gfx_state.hw_extensions     = (const char *) glGetString(GL_EXTENSIONS);
    gfx_state.hw_renderer       = (const char *) glGetString(GL_RENDERER);
    gfx_state.hw_vendor         = (const char *) glGetString(GL_VENDOR);
    gfx_state.hw_version        = (const char *) glGetString(GL_VERSION);

    glGetIntegerv(GL_NUM_EXTENSIONS, &pl_gl_num_extensions);

    glGetIntegerv(GL_MINOR_VERSION, &pl_gl_version_minor);
    glGetIntegerv(GL_MAJOR_VERSION, &pl_gl_version_major);
    plGraphicsLog(" OpenGL %d.%d\n", pl_gl_version_major, pl_gl_version_minor);
    plGraphicsLog(" renderer:   %s\n", gfx_state.hw_renderer);
    plGraphicsLog(" vendor:     %s\n", gfx_state.hw_vendor);
    plGraphicsLog(" version:    %s\n", gfx_state.hw_version);
    plGraphicsLog(" extensions:\n");
    for(unsigned int i = 0; i < pl_gl_num_extensions; ++i) {
        const GLubyte *extension = glGetStringi(GL_EXTENSIONS, i);
        plGraphicsLog("    %s\n", extension);
    }
}

void ShutdownOpenGL() {

}