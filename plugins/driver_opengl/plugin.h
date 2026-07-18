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

typedef enum XglBuiltInUniform
{
	XGL_UNIFORM_MODEL_MATRIX,
	XGL_UNIFORM_VIEW_MATRIX,
	XGL_UNIFORM_PROJECTION_MATRIX,
	XGL_UNIFORM_TEXTURE_MATRIX,

	XGL_UNIFORM_CLIP_PLANE,
	XGL_UNIFORM_CLIP_PLANE_MATRIX,

	XGL_MAX_BUILTIN_UNIFORMS
} XglBuiltInUniform;

typedef struct XglShaderProgram
{
	unsigned int id;
	unsigned int builtInUniforms[ XGL_MAX_BUILTIN_UNIFORMS ];
} XglShaderProgram;

enum
{
	XGL_MESH_BUFFER_VERTEX = 0,
	XGL_MESH_BUFFER_ELEMENT,

	XGL_MAX_GPU_MESH_BUFFERS
};

typedef struct XglMesh
{
	unsigned int buffers[ XGL_MAX_GPU_MESH_BUFFERS ];
	unsigned int vao;
} XglMesh;
