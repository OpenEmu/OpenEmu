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

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;

void main()
{
	gl_Position	=	global.MVP * Position;
	vTexCoord	=	TexCoord;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;
layout(set = 0, binding = 3) uniform sampler2D LUT;
layout(set = 0, binding = 4) uniform sampler2D Original;

void main()
{
	vec2 fp = fract(vTexCoord * registers.SourceSize.xy);
	vec2 quad = sign(-0.5 + fp);

	float dx = registers.SourceSize.z;
	float dy = registers.SourceSize.w;

	vec3 p1 = texture(Original, vTexCoord).rgb;
	vec3 p2 = texture(Original, vTexCoord + vec2(dx, dy) * quad).rgb;
	vec3 p3 = texture(Original, vTexCoord + vec2(dx, 0.0) * quad).rgb;
	vec3 p4 = texture(Original, vTexCoord + vec2(0.0, dy) * quad).rgb;
	vec3 pixels[4];
	pixels[0]	=	p1.xyz;
	pixels[1]	=	p2.xyz;
	pixels[2]	=	p3.xyz;
	pixels[3]	=	p4.xyz;

	vec2 index = texture(Source, vTexCoord).xy * vec2(255.0, 15.0 * (SCALE * SCALE));
	index.y += dot(floor(fp * SCALE), vec2(1.0, SCALE));

	vec2 step = 1.0 / vec2(256.0, 16.0 * (SCALE * SCALE));
	vec2 offset = step / 2.0;
	vec4 weights = texture(LUT, index * step + offset);
	float sum = dot(weights, vec4(1.0));
	vec4 tmp = vec4(float((weights/sum).x), float((weights/sum).y), float((weights/sum).z), float((weights/sum).w));
	vec3 res = tmp.x * pixels[0];
	res = res + tmp.y * pixels[1];
	res = res + tmp.z * pixels[2];
	res = res + tmp.w * pixels[3];

	FragColor = vec4(res.xyz, 1.0);
}
