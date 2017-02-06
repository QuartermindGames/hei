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
#include "platform_filesystem.h"

using namespace pl::graphics;

#define SHADER_INVALID_TYPE ((PLuint)0 - 1)

unsigned int _plTranslateShaderType(ShaderType type) {
    switch(type) {
        case SHADER_VERTEX:      return GL_VERTEX_SHADER;
        case SHADER_FRAGMENT:    return GL_FRAGMENT_SHADER;
        case SHADER_GEOMETRY:    return GL_GEOMETRY_SHADER;
#ifndef __APPLE__
        case SHADER_COMPUTE:     return GL_COMPUTE_SHADER;
#endif
        default: return SHADER_INVALID_TYPE;
    }
}

/*===========================
	SHADER
===========================*/

Shader::Shader(ShaderType type, std::string path) : type_(type) {
    PLuint ntype = _plTranslateShaderType(type_);
    if(ntype == SHADER_INVALID_TYPE) {
        std::runtime_error("invalid shader type received!");
    }

    id_ = glCreateShader(_plTranslateShaderType(type_));
    if(id_ == 0) {
        throw std::runtime_error("failed to create shader");
    } else if(!path.empty() && (LoadFile(path) != PL_RESULT_SUCCESS)) {
        throw std::runtime_error("failed to load shader");
    }
}

Shader::~Shader() {
    glDeleteShader(id_);
}

PLresult Shader::LoadFile(std::string path) {
    // Ensure we use the correct path and shader.
    std::string full_path;
    switch(type_) {
        case SHADER_FRAGMENT:
            full_path = path + "_fragment.shader";
            break;
        case SHADER_VERTEX:
            full_path = path + "_vertex.shader";
            break;
        default:    return PL_RESULT_FILETYPE;
    }

    std::ifstream file(full_path, std::ios::in);
    if(!file.is_open()) {
        return PL_RESULT_FILEREAD;
    }
    file.seekg(0, file.end);
    long size = file.tellg();
    file.seekg(0, file.beg);
    char *buf = new char[size];
    file.read(buf, size);

    const char *full_source[] = {
#if 0
#if defined(PL_MODE_OPENGL)
        //"#version 110\n",	// OpenGL 2.0
        "#version 120\n",	// OpenGL 2.1
        //"#version 130\n",	// OpenGL 3.0
        //"#version 140\n",	// OpenGL 3.1
        //"#version 150\n",	// OpenGL 3.2
        //"#version 330\n", // OpenGL 3.3
        //"#version 450\n",	// OpenGL 4.5
#elif defined(PL_MODE_OPENGL_ES)
        "#version 100\n",	// OpenGL ES 2.0
#endif
#endif
        buf
    };
    // todo, introduce pre-processor to catch any custom ones (like include).
    glShaderSource(id_, 2, full_source, NULL);
    glCompileShader(id_);

    delete[] buf;

    int status;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
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
}

/*===========================
	SHADER UNIFORM
===========================*/

ShaderUniform::ShaderUniform(ShaderProgram *parent, std::string name) {
    if(!parent) {
        throw std::runtime_error("invalid shader program");
    } else if(parent->GetInstance() == 0) {
        throw std::runtime_error("invalid shader program instance, might not yet have been created");
    }
    parent_ = parent;

    if(name.empty()) {
        throw std::runtime_error("received invalid uniform name");
    }

    int location = glGetUniformLocation(parent_->GetInstance(), name.c_str());
    if(location == -1) {
        throw std::runtime_error("failed to get uniform location");
    }
    id_ = static_cast<unsigned int>(location);
}

void ShaderUniform::Set(float x) {
    glUniform1f(id_, x);
}

void ShaderUniform::Set(float x, float y) {
    glUniform2f(id_, x, y);
}

void ShaderUniform::Set(float x, float y, float z) {
    glUniform3f(id_, x, y, z);
}

void ShaderUniform::Set(float x, float y, float z, float w) {
    glUniform4f(id_, x, y, z, w);
}

void ShaderUniform::Set(int x) {
    glUniform1i(id_, x);
}

void ShaderUniform::Set(int x, int y) {
    glUniform2i(id_, x, y);
}

void ShaderUniform::Set(int x, int y, int z) {
    glUniform3i(id_, x, y, z);
}

/*===========================
	SHADER ATTRIBUTE
===========================*/

ShaderAttribute::ShaderAttribute(ShaderProgram *parent, std::string name) {
    if(!parent) {
        throw std::runtime_error("invalid shader program");
    } else if(parent->GetInstance() == 0) {
        throw std::runtime_error("invalid shader program instance, might not yet have been created");
    }
    parent_ = parent;

    if(name.empty()) { // warning?
        throw std::runtime_error("received invalid attribute name");
    }

    int location = glGetAttribLocation(parent_->GetInstance(), name.c_str());
    if(location == -1) { // warning?
        throw std::runtime_error("failed to get attribute location");
    }
    id_ = static_cast<unsigned int>(location);
}

void ShaderAttribute::Set(float x) {

}

void ShaderAttribute::Set(float x, float y) {

}

void ShaderAttribute::Set(float x, float y, float z) {

}

void ShaderAttribute::Set(float x, float y, float z, float w) {

}

void ShaderAttribute::Set(int x) {

}

void ShaderAttribute::Set(int x, int y) {

}

void ShaderAttribute::Set(int x, int y, int z) {

}

/*===========================
	SHADER PROGRAM
===========================*/

ShaderProgram::ShaderProgram() {
    int id = glCreateProgram();
    if(id == 0) {
        throw std::runtime_error("failed to create shader program");
    }

    id_ = static_cast<unsigned int>(id);
}

ShaderProgram::~ShaderProgram() {
    if(id_ == 0) {
        return;
    }

    glDeleteProgram(id_);
}

void ShaderProgram::Enable() {
    plEnableShaderProgram(id_);
}

void ShaderProgram::Disable() {
    plDisableShaderProgram(id_);
}

void ShaderProgram::RegisterUniform(std::string name) {
    if(name.empty()) {
        plGraphicsLog("Attempted to register a uniform with an invalid name!\n");
        return;
    }

    // Ensure it's not registered already.
    ShaderUniform *euni = GetUniform(name);
    if(euni) {
        return;
    }

    uniforms.emplace(name, ShaderUniform(this, name));
}

void ShaderProgram::RegisterAttribute(std::string name) {
    if(name.empty()) {
        plGraphicsLog("Attempted to register an attribute with an invalid name!\n");
        return;
    }

    // Ensure it's not registered already.
    ShaderAttribute *eati = GetAttribute(name);
    if(eati) {
        return;
    }

    attributes.emplace(name, ShaderAttribute(this, name));
}

void ShaderProgram::AttachShader(Shader *shader) {
    if(!shader) {
        plGraphicsLog("Attempted to attach an invalid shader!\n");
        return;
    } else if(shader->GetInstance() == 0) {
        plGraphicsLog("Attempted to attach a shader with an invalid instance!\n");
        return;
    }

    glAttachShader(id_, shader->GetInstance());

    shaders.push_back(shader);
}

void ShaderProgram::LoadShaders(std::string vertex, std::string fragment) {
    if(!plFileExists(vertex.c_str()) || !plFileExists(fragment.c_str())) {
        plGraphicsLog("Invalid shader path! (%s / %s)\n", fragment.c_str(), vertex.c_str());
        return;
    }

    AttachShader(new Shader(SHADER_FRAGMENT, fragment));
    AttachShader(new Shader(SHADER_VERTEX, vertex));
}

