// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>

#pragma once

#define PL_COMPILE_PLUGIN 1
#include <plgraphics/plg_driver_interface.h>

extern const PLGDriverExportTable *gInterface;

extern int glLogLevel;

typedef struct XglTexture
{
	unsigned int id;
	unsigned int target;
	unsigned int faceIndex;// for cubemaps

	bool hasStorage;
} XglTexture;

typedef enum XglShaderUniformType : unsigned int
{
	XGL_SHADER_UNIFORM_TYPE_MODEL_MATRIX,
	XGL_SHADER_UNIFORM_TYPE_VIEW_MATRIX,
	XGL_SHADER_UNIFORM_TYPE_PROJECTION_MATRIX,
	XGL_SHADER_UNIFORM_TYPE_TEXTURE_MATRIX,
	XGL_SHADER_UNIFORM_TYPE_CLIP_PLANE,
	XGL_SHADER_UNIFORM_TYPE_CLIP_PLANE_MATRIX,
	XGL_SHADER_UNIFORM_TYPE_MAX
} XglShaderUniformType;

typedef enum XglShaderAttributeType : unsigned int
{
	XGL_SHADER_ATTRIBUTE_TYPE_POSITION,
	XGL_SHADER_ATTRIBUTE_TYPE_NORMAL,
	XGL_SHADER_ATTRIBUTE_TYPE_COLOUR,
	XGL_SHADER_ATTRIBUTE_TYPE_TANGENT,
	XGL_SHADER_ATTRIBUTE_TYPE_BITANGENT,
	XGL_SHADER_ATTRIBUTE_TYPE_UV0,
	XGL_SHADER_ATTRIBUTE_TYPE_UV1,
	XGL_SHADER_ATTRIBUTE_TYPE_UV2,
	XGL_SHADER_ATTRIBUTE_TYPE_UV3,

	XGL_SHADER_ATTRIBUTE_TYPE_MAX
} XglShaderAttributeType;

typedef struct XglShaderProgram
{
	unsigned int id;

	XglShaderUniformType   defaultUniforms[ XGL_SHADER_UNIFORM_TYPE_MAX ];
	XglShaderAttributeType defaultAttributes[ XGL_SHADER_ATTRIBUTE_TYPE_MAX ];
} XglShaderProgram;

enum
{
	XGL_MESH_BUFFER_VERTEX = 0,
	XGL_MESH_BUFFER_ELEMENT,

	XGL_MESH_BUFFER_MAX
};

typedef struct XglMesh
{
	unsigned int buffers[ XGL_MESH_BUFFER_MAX ];
	unsigned int vao;
} XglMesh;
