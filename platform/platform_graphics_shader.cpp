
#include "platform_graphics.h"

PLuint _plTranslateShaderType(pl::graphics::ShaderType type) {
    switch(type) {
#if defined(PL_MODE_OPENGL)
        default:                                            return 0;   // todo, urgh, handle these cases better!
        case pl::graphics::ShaderType::SHADER_FRAGMENT:     return GL_FRAGMENT_SHADER;
        case pl::graphics::ShaderType::SHADER_GEOMETRY:     return GL_GEOMETRY_SHADER;
        case pl::graphics::ShaderType::SHADER_VERTEX:       return GL_VERTEX_SHADER;
#else
        default:    return 0;
#endif
    }
}

// Shader Uniform

pl::graphics::ShaderProgram::ShaderProgram(std::string name) : name_(name) {
#if defined(PL_MODE_OPENGL)
    id_ = glCreateProgram();
    if(id_ == 0) {
        // todo, handle error as gracefully as possible...
    }
#endif
}

pl::graphics::ShaderProgram::~ShaderProgram() {
#if defined(PL_MODE_OPENGL)
    if(id_ == 0) {
        return;
    }

    glDeleteProgram(id_);
#endif

    attributes_.clear();
    uniforms_.clear();
    shaders_.clear();
}

int pl::graphics::ShaderProgram::GetUniformLocation(std::string name) {
#if defined(PL_MODE_OPENGL)
    int location = glGetUniformLocation(id_, name.c_str());
    if(location == -1) {
        // todo, failed to get uniform location.
    }

    return location;
#else
    return 0;
#endif
}
