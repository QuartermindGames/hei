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

uniform vec3	u_lightposition;
uniform vec4	u_lightcolour;

uniform float u_vertexscale;

void main()
{
	v_normalcolour = vec4(gl_Normal, 1.0);
	v_diffusecolour = vec4(1.0, 1.0, 1.0, 1.0);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	float scale = u_vertexscale;
	if(scale == 0)
		scale = 1.0;
		
	gl_Position = ftransform() * scale;

	// Sphere-mapping
	/*
	vec3 u = normalize(vec3(gl_ModelViewMatrix * gl_Vertex));
	vec3 n = normalize(gl_NormalMatrix * gl_Normal);
	vec3 r = reflect(u, n);
	float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z + 1.0)*(r.z + 1.0));
	gl_TexCoord[1].s = r.x / m + 0.5;
	gl_TexCoord[1].t = r.y / m + 0.5;
	*/

	/*
	vec3 viewVertex = normalize(gl_ModelViewMatrix * gl_Vertex);
	vec3 viewNormal = normalize(gl_NormalMatrix * gl_Normal);
	vec3 viewLightPosition = normalize(gl_NormalMatrix * u_lightposition);

	float dist = distance(viewNormal, viewLightPosition);

	v_diffusecolour = vec4((lightColour * 2.0) * dist,1.0) * max(dot(viewNormal, viewLightPosition), 0.0);
	*/
}
