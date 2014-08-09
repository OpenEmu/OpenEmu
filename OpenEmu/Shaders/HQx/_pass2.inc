/* COMPATIBILITY 
   - HLSL compilers
   - Cg   compilers
*/


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

struct orig
{
	float2 tex_coord;
	uniform float2 texture_size;
	uniform sampler2D texture;
};

struct input
{
	half2 video_size;
	float2 texture_size;
	half2 output_size;
};

struct out_vertex {
	float4 position : POSITION;
	float4 color    : COLOR;
	float2 texCoord : TEXCOORD0;
};

/*    VERTEX_SHADER    */
out_vertex main_vertex
(
	float4 position	: POSITION,
	float4 color	: COLOR,
	float2 texCoord : TEXCOORD0,

   	uniform half4x4 modelViewProj,
	uniform input IN
)
{
	out_vertex OUT;

	OUT.position = mul(modelViewProj, position);
	OUT.color = color;

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

	OUT.texCoord = texCoord;

	return OUT;
}


/*    FRAGMENT SHADER    */
float4 main_fragment(in out_vertex VAR, uniform sampler2D decal : TEXUNIT0, orig ORIG, uniform input IN, uniform sampler2D LUT : TEXUNIT1) : COLOR
{
	float2 fp = frac(VAR.texCoord*IN.texture_size);
	half2 quad = sign(-0.5 + fp);

	float2 ps = 1.0 / ORIG.texture_size;
	float dx = ps.x;
	float dy = ps.y;

	float3 p1  = tex2D(ORIG.texture, VAR.texCoord).rgb;
	float3 p2  = tex2D(ORIG.texture, VAR.texCoord + float2(dx, dy) * quad).rgb;
	float3 p3  = tex2D(ORIG.texture, VAR.texCoord + float2(dx, 0) * quad).rgb;
	float3 p4  = tex2D(ORIG.texture, VAR.texCoord + float2(0, dy) * quad).rgb;
	float4x3 pixels = half4x3(p1, p2, p3, p4);

	half2 index = tex2D(decal, VAR.texCoord).xy * half2(255.0, 15.0 * (SCALE * SCALE));
	index.y += dot(floor(fp * SCALE), half2(1, SCALE));

	half2 step = 1.0 / half2(256.0, 16.0 * (SCALE * SCALE));
	half2 offset = step / 2.0;
	half4 weights = tex2D(LUT, index * step + offset);
	half sum = dot(weights, half4(1));
	float3 res = mul(transpose(pixels), weights / sum);

	return float4(res, 1.0);
}
