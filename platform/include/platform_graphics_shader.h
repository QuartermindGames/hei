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

#pragma once

typedef enum PLShaderType {
    PL_SHADER_VERTEX,   // GL_VERTEX_SHADER
    PL_SHADER_FRAGMENT, // GL_FRAGMENT_SHADER
    PL_SHADER_GEOMETRY, // GL_GEOMETRY_SHADER
    PL_SHADER_COMPUTE,  // GL_COMPUTE_SHADER
} PLShaderType;

typedef enum PLShaderUniformType {
    PL_UNIFORM_FLOAT,
    PL_UNIFORM_INT,
    PL_UNIFORM_UINT,
    PL_UNIFORM_BOOL,
    PL_UNIFORM_DOUBLE,

    // Textures

    PL_UNIFORM_SAMPLER1D,
    PL_UNIFORM_SAMPLER2D,
    PL_UNIFORM_SAMPLER3D,
    PL_UNIFORM_SAMPLERCUBE,
    PL_UNIFORM_SAMPLER1DSHADOW,
    PL_UNIFORM_SAMPLER2DSHADOW,

    // Vectors

    PL_UNIFORM_VEC2,
    PL_UNIFORM_VEC3,
    PL_UNIFORM_VEC4,

    // Matrices

    PL_UNIFORM_MAT3
} ShaderUniformType;

typedef int PLSampler1D, PLSampler2D, PLSampler3D;
typedef int PLSamplerCube;
typedef int PLSampler1DShadow, PLSampler2DShadow;

#ifdef __cplusplus

namespace pl {
    namespace graphics {

        class Shader {
        public:
            Shader(PLShaderType type);
            ~Shader();

            PLresult LoadFile(std::string path);

            unsigned int GetInstance() { return id_; }

        protected:
        private:
            unsigned int id_;

            PLShaderType type_;
        };

        class ShaderProgram;

        class ShaderUniform {
        public:
            ShaderUniform(ShaderProgram *parent, std::string name);

            void Set(float x);
            void Set(float x, float y);
            void Set(float x, float y, float z);
            void Set(float x, float y, float z, float w);
            void Set(int x);
            void Set(int x, int y);
            void Set(int x, int y, int z);

        protected:
        private:
            unsigned int id_;

            ShaderProgram *parent_;
        };

        class ShaderAttribute {
        public:
            ShaderAttribute(ShaderProgram *parent, std::string name);

            void Set(float x);
            void Set(float x, float y);
            void Set(float x, float y, float z);
            void Set(float x, float y, float z, float w);
            void Set(int x);
            void Set(int x, int y);
            void Set(int x, int y, int z);

        protected:
        private:
            unsigned int id_;

            ShaderProgram *parent_;
        };

        class ShaderProgram {
        public:
            ShaderProgram();
            ~ShaderProgram();

            void RegisterUniform(std::string name);
            void RegisterAttribute(std::string name);

            void LoadShaders(std::string vertex, std::string fragment);

            void AttachShader(Shader *shader);

            bool IsEnabled() { return (plGetCurrentShaderProgram() == id_); }

            void Enable();
            void Disable();

            ShaderAttribute *GetAttribute(std::string name) {
                auto attribute = attributes.find(name);
                if(attribute != attributes.end()) {
                    return &attribute->second;
                }

                return nullptr;
            }

            ShaderUniform *GetUniform(std::string name) {
                auto uniform = uniforms.find(name);
                if(uniform != uniforms.end()) {
                    return &uniform->second;
                }

                return nullptr;
            }

            unsigned int GetInstance() { return id_; }

        protected:
        private:
            unsigned int id_;

            std::vector<Shader*>							    shaders;
            std::unordered_map<std::string, ShaderAttribute>	attributes;
            std::unordered_map<std::string, ShaderUniform>		uniforms;
        };

    }
}

#endif