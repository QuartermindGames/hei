/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#define PL_COMPILE_PLUGIN 1
#include <plgraphics/plg_driver_interface.h>

extern const PLGDriverExportTable *gInterface;

extern int glLogLevel;

typedef struct GLTexture {
	unsigned int id;
	unsigned int target;
} GLTexture;

typedef enum OGLDefaultUniform {
	OGL_DEFAULT_UNIFORM_MODEL_MATRIX,
	OGL_DEFAULT_UNIFORM_VIEW_MATRIX,
	OGL_DEFAULT_UNIFORM_PROJECTION_MATRIX,
	OGL_DEFAULT_UNIFORM_TEXTURE_MATRIX,

	OGL_DEFAULT_UNIFORM_CLIP_PLANE,
	OGL_DEFAULT_UNIFORM_CLIP_PLANE_MATRIX,

	OGL_MAX_DEFAULT_UNIFORMS
} OGLDefaultUniform;

typedef struct OGLShaderProgram {
	unsigned int defaultUniforms[ OGL_MAX_DEFAULT_UNIFORMS ];
} OGLShaderProgram;
