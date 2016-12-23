
#pragma once

#if 1

namespace pl {

    namespace graphics {

        typedef enum ShaderType {
            SHADER_FRAGMENT,
            SHADER_VERTEX,
            SHADER_GEOMETRY
        } ShaderType;

        typedef enum ShaderUniformType {
            PL_UNIFORM_FLOAT,
            PL_UNIFORM_INT,
            PL_UNIFORM_UINT,
            PL_UNIFORM_BOOL,
            PL_UNIFORM_DOUBLE,

            // Textures
            PL_UNIFORM_TEXTURE1D,
            PL_UNIFORM_TEXTURE2D,
            PL_UNIFORM_TEXTURE3D,
            PL_UNIFORM_TEXTURECUBE,
            PL_UNIFORM_TEXTUREBUFFER,

            // Vectors
            PL_UNIFORM_VEC2,
            PL_UNIFORM_VEC3,
            PL_UNIFORM_VEC4,

            // Matrices
            PL_UNIFORM_MAT3
        } ShaderUniformType;

        class Shader {
        public:
            PL_DLL Shader(ShaderType type);
            PL_DLL ~Shader();

            PL_DLL void Load(std::string path);

            PL_DLL unsigned int GetInstance() {
                return id_;
            }

            PL_DLL ShaderType GetType() {
                return type_;
            }

        private:
            unsigned int id_;

            std::string source_;
            std::string source_path_;

            ShaderType type_;
        };

        class ShaderUniform {};
        class ShaderAttribute {};

        class ShaderProgram {
        public:
            ShaderProgram(std::string name);
            ~ShaderProgram();

            PL_DLL virtual void RegisterShader(std::string path, ShaderType type);
            PL_DLL virtual void RegisterAttributes();

            PL_DLL void Attach(Shader *shader);
            PL_DLL void Enable();
            PL_DLL void Disable();
            PL_DLL void Link();

            PL_DLL int GetUniformLocation(std::string name);

            PL_DLL bool IsActive();

        protected:
        private:
            unsigned int id_;

            std::vector<Shader>                                 shaders_;
            std::unordered_map<std::string, ShaderAttribute>    attributes_;
            std::unordered_map<std::string, ShaderUniform>      uniforms_;

            std::string name_;
        };

    }

}

#else

typedef struct PLShaderUniform {
    PLuint id;

    PLShaderUniformType type;

    PLchar def[32];
} PLShaderUniform;

typedef struct PLShaderAttribute {
    PLuint id;
} PLShaderAttribute;

typedef struct PLShader {
    PLuint id;

    PLShaderType type;

    PLchar *source;
    PLchar source_path[PL_SYSTEM_MAX_PATH];
} PLShader;

typedef struct PLShaderProgram {
    PLuint id;

    PLShader            **shaders;
    PLShaderUniform     **uniforms;
    PLShaderAttribute   **attributes;
} PLShaderProgram;

PL_EXTERN_C

PL_EXTERN PLShader *plCreateShader(PLShaderType type);
PL_EXTERN void plDeleteShader(PLShader *shader);

PL_EXTERN PLShaderProgram *plCreateShaderProgram(void);
PL_EXTERN void plDeleteShaderProgram(PLShaderProgram *program);

PL_EXTERN PLShaderProgram *plGetCurrentShaderProgram(void);
PL_EXTERN void plSetShaderProgram(PLShaderProgram *program);

PL_EXTERN_C_END

#endif