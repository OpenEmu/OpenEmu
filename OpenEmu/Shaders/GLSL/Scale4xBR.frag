/*
    Hyllian's 4xBR Shader

    Copyright (C) 2011 Hyllian/Jararaca - sergiogdb@gmail.com

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

uniform sampler2DRect OETexture;

const vec3 dtt = vec3(65536.0, 255.0, 1.0);

const vec2 dx = vec2(0.0, -1.0);
const vec2 dy = vec2(-1.0, 0.0);

float reduce(vec3 color)
{ 
    return dot(color, dtt);
}

void main()
{
	vec2 fp = fract(gl_TexCoord[0].st);

	vec2 g1 = dx * (step(0.5,fp.x) + step(0.5, fp.y) -1.0) + dy * (step(0.5,fp.x) - step(0.5, fp.y)     );
	vec2 g2 = dx * (step(0.5,fp.y) - step(0.5, fp.x)     ) + dy * (step(0.5,fp.x) + step(0.5, fp.y) -1.0);

	vec3 B = texture2DRect(OETexture, gl_TexCoord[0].st +g1     ).xyz;
	vec3 C = texture2DRect(OETexture, gl_TexCoord[0].st +g1 - g2).xyz;
	vec3 D = texture2DRect(OETexture, gl_TexCoord[0].st      +g2).xyz;
	vec3 E = texture2DRect(OETexture, gl_TexCoord[0].st         ).xyz;
	vec3 F = texture2DRect(OETexture, gl_TexCoord[0].st      -g2).xyz;
	vec3 G = texture2DRect(OETexture, gl_TexCoord[0].st -g1 + g2).xyz;
	vec3 H = texture2DRect(OETexture, gl_TexCoord[0].st -g1     ).xyz;
	vec3 I = texture2DRect(OETexture, gl_TexCoord[0].st -g1 - g2).xyz;

	vec3 E11 = E;
	vec3 E15 = E;

	float b = reduce(B);
	float c = reduce(C);
	float d = reduce(D);
	float e = reduce(E);
	float f = reduce(F);
	float g = reduce(G);
	float h = reduce(H);
	float i = reduce(I);	

	if (h==f && h!=e && ( e==g && (h==i || e==d) || e==c && (h==i || e==b) ))
	{
		E11 = E11*0.5+F*0.5;
		E15 = F;
	}

	vec3 res;

	res = (fp.x < 0.50) ? ((fp.x < 0.25) ? ((fp.y < 0.25) ? E15: (fp.y < 0.50) ? E11: (fp.y < 0.75) ? E11: E15) : ((fp.y < 0.25) ? E11: (fp.y < 0.50) ? E  : (fp.y < 0.75) ? E  : E11)) : ((fp.x < 0.75) ? ((fp.y < 0.25) ? E11: (fp.y < 0.50) ? E  : (fp.y < 0.75) ? E   : E11) : ((fp.y < 0.25) ? E15: (fp.y < 0.50) ? E11: (fp.y < 0.75) ? E11 : E15));

	gl_FragColor = vec4(res, 1.0);
}
