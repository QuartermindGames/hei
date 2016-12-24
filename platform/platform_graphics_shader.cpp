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

/*===========================
	SHADER
===========================*/

pl::graphics::Shader::Shader(ShaderType type) : type_(type) {
#if defined(PL_MODE_OPENGL)
    id_ = glCreateShader(_plTranslateShaderType(type_));
    if(id_ == 0) {
        // todo, failed to create shader.
    }
#endif
}

pl::graphics::Shader::~Shader() {
#if defined(PL_MODE_OPENGL)
    if(id_ != 0) {
        glDeleteShader(id_);
    }
#endif
}

PLresult pl::graphics::Shader::Load(std::string path) {
#if defined(PL_MODE_OPENGL)
    // Ensure we use the correct path and shader.
    std::string full_path;
    switch(type_) {
        case pl::graphics::ShaderType::SHADER_FRAGMENT:
            full_path = path + "_fragment.shader";
            break;
        case pl::graphics::ShaderType::SHADER_VERTEX:
            full_path = path + "_vertex.shader";
            break;
        default:    return PL_RESULT_FILETYPE;
    }

    std::ifstream file;
    file.open(full_path, std::ios::in);
    if(!file.is_open()) {
        return PL_RESULT_FILEREAD;
    }
    file.seekg(0, file.end);
    long size = file.tellg();
    file.seekg(0, file.beg);
    char *buf = new char[size];
    file.read(buf, size);

    const char *full_source[] = {
#if defined (VL_MODE_OPENGL)
        //"#version 110\n",	// OpenGL 2.0
		"#version 120\n",	// OpenGL 2.1
		//"#version 130\n",	// OpenGL 3.0
		//"#version 140\n",	// OpenGL 3.1
		//"#version 150\n",	// OpenGL 3.2
		//"#version 450\n",	// OpenGL 4.5
#elif defined (VL_MODE_OPENGL_ES)
        "#version 100\n",	// OpenGL ES 2.0
#endif
        buf
    };
    // todo, introduce pre-processor to catch any custom ones (like include).
    glShaderSource(id_, 2, full_source, NULL);
    glCompileShader(id_);

    delete[] buf;

    int status;
    glGetObjectParameterivARB(id_, GL_COMPILE_STATUS, &status);
    if(!status) {
        int length;
        glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &length);
        if(length > 1) {
            char *log = new char[length];
            glGetShaderInfoLog(id_, length, NULL, log);
            plGraphicsLog(log);
            delete[] log;
        }

        return PL_RESULT_SHADERCOMPILE;
    }

    return PL_RESULT_SUCCESS;
#else
    return PL_RESULT_SUCCESS;
#endif
}

/*===========================
	SHADER PROGRAM
===========================*/

pl::graphics::ShaderProgram::ShaderProgram(std::string name) : name_(name) {
#if defined(PL_MODE_OPENGL)
    id_ = glCreateProgram();
    if(id_ == 0) {
        // todo, failed to create shader program.
    }
#endif
}

pl::graphics::ShaderProgram::~ShaderProgram() {
#if defined(PL_MODE_OPENGL)
    if(id_ != 0) {
        glDeleteProgram(id_);
    }
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
