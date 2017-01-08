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

#version 120

varying vec4 v_diffusecolour;
varying vec4 v_normalcolour;

uniform sampler2D	u_diffusemap;
uniform sampler2D	u_detailmap;
uniform sampler2D	u_fullbrightmap;
uniform sampler2D	u_normalmap;
uniform sampler2D	u_spheremap;

uniform vec4	u_lightcolour;
uniform vec3	u_lightposition;

uniform	float	u_alphaclamp;
uniform	bool	u_alphatest;

void main() {
	vec4 diffuse = texture2D(u_diffusemap, gl_TexCoord[0].st);

	// Alpha-testing.
	if (u_alphatest == true) {
		if (diffuse.a < u_alphaclamp) {
			discard;
		}
	}

	gl_FragColor = diffuse;
	//gl_FragColor = vec4(v_diffusecolour.xyz, 1.0) * diffuse;
	//gl_FragColor = normalize(v_normalcolour);
}
