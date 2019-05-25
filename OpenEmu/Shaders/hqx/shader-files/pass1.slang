#version 450

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	float trY;
	float trU;
	float trV;
} registers;

layout(std140, set = 0, binding = 0) uniform UBO
{
   mat4 MVP;
} global;

/*
* Copyright (C) 2003 Maxim Stepin ( maxst@hiend3d.com )
*
* Copyright (C) 2010 Cameron Zemek ( grom@zeminvaders.net )
*
* Copyright (C) 2014 Jules Blok ( jules@aerix.nl )
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#pragma parameter trY "Y Threshold" 48.0 0.0 255.0 1.0
#pragma parameter trU "U Threshold" 7.0 0.0 255.0 1.0
#pragma parameter trV "V Threshold" 6.0 0.0 255.0 1.0

vec3 yuv_threshold = vec3(registers.trY/255.0, registers.trU/255.0, registers.trV/255.0);
const mat3 yuv = mat3(0.299, -0.169, 0.5, 0.587, -0.331, -0.419, 0.114, 0.5, -0.081);
const vec3 yuv_offset = vec3(0.0, 0.5, 0.5);

bool diff(vec3 yuv1, vec3 yuv2) {
	bvec3 res = greaterThan(abs((yuv1 + yuv_offset) - (yuv2 + yuv_offset)) , yuv_threshold);
	return res.x || res.y || res.z;
}

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec4 t1;
layout(location = 2) out vec4 t2;
layout(location = 3) out vec4 t3;

void main()
{
	gl_Position	=	global.MVP * Position;
	vTexCoord	=	TexCoord;
	float	dx	=	registers.SourceSize.z;
	float	dy	=	registers.SourceSize.w;
	//   +----+----+----+
	//   |    |    |    |
	//   | w1 | w2 | w3 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w4 | w5 | w6 |
	//   +----+----+----+
	//   |    |    |    |
	//   | w7 | w8 | w9 |
	//   +----+----+----+
	t1	=	vTexCoord.xxxy + vec4(-dx, 0, dx, -dy); //  w1 | w2 | w3;
	t2	=	vTexCoord.xxxy + vec4(-dx, 0, dx,   0); //  w4 | w5 | w6;
	t3	=	vTexCoord.xxxy + vec4(-dx, 0, dx,  dy); //  w7 | w8 | w9;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec4 t1;
layout(location = 2) in vec4 t2;
layout(location = 3) in vec4 t3;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

void main()
{
	vec3	w1	=	yuv * texture(Source, t1.xw).rgb;
	vec3	w2	=	yuv * texture(Source, t1.yw).rgb;
	vec3	w3	=	yuv * texture(Source, t1.zw).rgb;

	vec3	w4	=	yuv * texture(Source, t2.xw).rgb;
	vec3	w5	=	yuv * texture(Source, t2.yw).rgb;
	vec3	w6	=	yuv * texture(Source, t2.zw).rgb;

	vec3	w7	=	yuv * texture(Source, t3.xw).rgb;
	vec3	w8	=	yuv * texture(Source, t3.yw).rgb;
	vec3	w9	=	yuv * texture(Source, t3.zw).rgb;

	vec3	pattern_1	=	vec3(diff(w5, w1),	diff(w5, w2),	diff(w5, w3));
	vec3	pattern_2	=	vec3(diff(w5, w4),	false,	diff(w5, w6));
	vec3	pattern_3	=	vec3(diff(w5, w7),	diff(w5, w8),	diff(w5, w9));
	vec4	cross		=	vec4(diff(w4, w2),	diff(w2, w6),	diff(w8, w4),	diff(w6, w8));
	
	vec2	index;
	index.x	=	dot(pattern_1, vec3(1, 2, 4)) +
				dot(pattern_2, vec3(8, 0, 16)) +
				dot(pattern_3, vec3(32, 64, 128));
	index.y	=	dot(cross, vec4(1, 2, 4, 8));
	
	FragColor	=	vec4(index / vec2(255.0, 15.0), 0.0, 1.0);
}