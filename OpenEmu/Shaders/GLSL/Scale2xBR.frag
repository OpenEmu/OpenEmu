/*
    Hyllian's 2xBR v3.5 Shader

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

#version 120

const  float coef        = 2.0;
const  vec3 dtt          = vec3(65536,255,1);
const  float y_weight    = 48.0;
const  float u_weight    = 7.0;
const  float v_weight    = 6.0;
const  mat3 yuv          = mat3(0.299, 0.587, 0.114, -0.169, -0.331, 0.499, 0.499, -0.418, -0.0813);
const  mat3 yuv_weighted = mat3(y_weight * yuv[0], u_weight * yuv[1], v_weight * yuv[2]);

const vec2 dx = vec2(1.0, 0.0);
const vec2 dy = vec2(0.0, 1.0);

//const static vec3x3 yuv_weighted = vec3x3(14.352, 28.176, 5.472, -1.183, -2.317, 3.5, 3.0, -2.514, -0.486);

uniform sampler2DRect OETexture;

vec4 RGBtoYUV(mat4x3 mat_color)
{
//	float a = abs(mul(yuv_weighted, mat_color[0]));
//	float b = abs(mul(yuv_weighted, mat_color[1]));
//	float c = abs(mul(yuv_weighted, mat_color[2]));
//	float d = abs(mul(yuv_weighted, mat_color[3]));
	float a = abs((yuv_weighted * mat_color[0]).x);
	float b = abs((yuv_weighted * mat_color[1]).x);
	float c = abs((yuv_weighted * mat_color[2]).x);
	float d = abs((yuv_weighted * mat_color[3]).x);
    
	return vec4(a, b, c, d);
}

vec4 df(vec4 A, vec4 B)
{
	return vec4(abs(A-B));
}

vec4 weighted_distance(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h)
{
	return vec4(df(a,b) + df(a,c) + df(d,e) + df(d,f) + 4.0 * df(g,h));
}


void main(void)
{
    bvec4 edr, edr_left, edr_up, px; // px = pixel, edr = edge detection rule
	bvec4 interp_restriction_lv1, interp_restriction_lv2_left, interp_restriction_lv2_up;
    
	vec2 fp = fract(gl_TexCoord[0].st);
    
	//vec2 dx = VAR.t1.xy;
	//vec2 dy = VAR.t1.zw;
    
	vec3 A = texture2DRect(OETexture, gl_TexCoord[0].st -dx -dy).xyz;
	vec3 B = texture2DRect(OETexture, gl_TexCoord[0].st     -dy).xyz;
	vec3 C = texture2DRect(OETexture, gl_TexCoord[0].st +dx -dy).xyz;
	vec3 D = texture2DRect(OETexture, gl_TexCoord[0].st -dx    ).xyz;
	vec3 E = texture2DRect(OETexture, gl_TexCoord[0].st        ).xyz;
	vec3 F = texture2DRect(OETexture, gl_TexCoord[0].st +dx    ).xyz;
	vec3 G = texture2DRect(OETexture, gl_TexCoord[0].st -dx +dy).xyz;
	vec3 H = texture2DRect(OETexture, gl_TexCoord[0].st     +dy).xyz;
	vec3 I = texture2DRect(OETexture, gl_TexCoord[0].st +dx +dy).xyz;
    
	vec3  A1 = texture2DRect(OETexture, gl_TexCoord[0].st     -dx -2.0*dy).xyz;
	vec3  C1 = texture2DRect(OETexture, gl_TexCoord[0].st     +dx -2.0*dy).xyz;
	vec3  A0 = texture2DRect(OETexture, gl_TexCoord[0].st -2.0*dx     -dy).xyz;
	vec3  G0 = texture2DRect(OETexture, gl_TexCoord[0].st -2.0*dx     +dy).xyz;
	vec3  C4 = texture2DRect(OETexture, gl_TexCoord[0].st +2.0*dx     -dy).xyz;
	vec3  I4 = texture2DRect(OETexture, gl_TexCoord[0].st +2.0*dx     +dy).xyz;
	vec3  G5 = texture2DRect(OETexture, gl_TexCoord[0].st     -dx +2.0*dy).xyz;
	vec3  I5 = texture2DRect(OETexture, gl_TexCoord[0].st     +dx +2.0*dy).xyz;
	vec3  B1 = texture2DRect(OETexture, gl_TexCoord[0].st         -2.0*dy).xyz;
	vec3  D0 = texture2DRect(OETexture, gl_TexCoord[0].st -2.0*dx        ).xyz;
	vec3  H5 = texture2DRect(OETexture, gl_TexCoord[0].st         +2.0*dy).xyz;
	vec3  F4 = texture2DRect(OETexture, gl_TexCoord[0].st +2.0*dx        ).xyz;
    
	vec4 a = RGBtoYUV( mat4x3(A, G, I, C) );
	vec4 b = RGBtoYUV( mat4x3(B, D, H, F) );
	vec4 c = RGBtoYUV( mat4x3(C, A, G, I) );
	vec4 d = RGBtoYUV( mat4x3(D, H, F, B) );
	vec4 e = RGBtoYUV( mat4x3(E, E, E, E) );
	vec4 f = RGBtoYUV( mat4x3(F, B, D, H) );
	vec4 g = RGBtoYUV( mat4x3(G, I, C, A) );
	vec4 h = RGBtoYUV( mat4x3(H, F, B, D) );
	vec4 i = RGBtoYUV( mat4x3(I, C, A, G) );
    
	//vec4 a1 = RGBtoYUV( mat4x3(A1, G0, I5, C4) );
	//vec4 c1 = RGBtoYUV( mat4x3(C1, A0, G5, I4) );
	//vec4 a0 = RGBtoYUV( mat4x3(A0, G5, I4, C1) );
	//vec4 g0 = RGBtoYUV( mat4x3(G0, I5, C4, A1) );
	//vec4 c4 = RGBtoYUV( mat4x3(C4, A1, G0, I5) );
	vec4 i4 = RGBtoYUV( mat4x3(I4, C1, A0, G5) );
	//vec4 g5 = RGBtoYUV( mat4x3(G5, I4, C1, A0) );
	vec4 i5 = RGBtoYUV( mat4x3(I5, C4, A1, G0) );
	//vec4 b1 = RGBtoYUV( mat4x3(B1, D0, H5, F4) );
	//vec4 d0 = RGBtoYUV( mat4x3(D0, H5, F4, B1) );
	vec4 h5 = RGBtoYUV( mat4x3(H5, F4, B1, D0) );
	vec4 f4 = RGBtoYUV( mat4x3(F4, B1, D0, H5) );
    
	interp_restriction_lv1      = bvec4( ((e!=f) && (e!=h)) );
	interp_restriction_lv2_left = bvec4( ((e!=g) && (d!=g)) );
	interp_restriction_lv2_up   = bvec4( ((e!=c) && (b!=c)) );
    
//	edr      = (weighted_distance( e, c, g, i, h5, f4, h, f) < weighted_distance( h, d, i5, f, i4, b, e, i)) && interp_restriction_lv1;
	edr      = bvec4( not( notEqual(lessThan(weighted_distance( e, c, g, i, h5, f4, h, f), weighted_distance( h, d, i5, f, i4, b, e, i)) ,  interp_restriction_lv1)) );
	edr_left = bvec4( not( notEqual(lessThanEqual(coef * df(f,g), df(h,c)),  interp_restriction_lv2_left)));
	edr_up   = bvec4( not( notEqual(greaterThanEqual(df(f,g),  (coef*df(h,c))), interp_restriction_lv2_up)));
    
	vec3 E0 = E;
	vec3 E1 = E;
	vec3 E2 = E;
	vec3 E3 = E;
    
	px = lessThanEqual(df(e,f), df(e,h));
    
	vec3 P[4];
    
	P[0] = px.x ? F : H;
	P[1] = px.y ? B : F;
	P[2] = px.z ? D : B;
	P[3] = px.w ? H : D;
    
    
	if (edr.x)
	{
		if (edr_left.x && edr_up.x)
		{
			E3  = mix(E3 , P[0],  0.833333);
			E2  = mix(E2 , P[0],  0.25);
			E1  = mix(E1 , P[0],  0.25);
		}
		else if (edr_left.x)
		{
			E3  = mix(E3 , P[0],  0.75);
			E2  = mix(E2 , P[0],  0.25);
		}
		else if (edr_up.x)
		{
			E3  = mix(E3 , P[0],  0.75);
			E1  = mix(E1 , P[0],  0.25);
		}
		else
		{
			E3  = mix(E3 , P[0],  0.5);
		}
	}
    
	if (edr.y)
	{
		if (edr_left.y && edr_up.y)
		{
			E1  = mix(E1 , P[1],  0.833333);
			E3  = mix(E3 , P[1],  0.25);
			E0  = mix(E0 , P[1],  0.25);
		}
		else if (edr_left.y)
		{
			E1  = mix(E1 , P[1],  0.75);
			E3  = mix(E3 , P[1],  0.25);
		}
		else if (edr_up.y)
		{
			E1  = mix(E1 , P[1],  0.75);
			E0  = mix(E0 , P[1],  0.25);
		}
		else
		{
			E1  = mix(E1 , P[1],  0.5);
		}
	}
    
	if (edr.z)
	{
		if (edr_left.z && edr_up.z)
		{
			E0  = mix(E0 , P[2],  0.833333);
			E1  = mix(E1 , P[2],  0.25);
			E2  = mix(E2 , P[2],  0.25);
		}
		else if (edr_left.z)
		{
			E0  = mix(E0 , P[2],  0.75);
			E1  = mix(E1 , P[2],  0.25);
		}
		else if (edr_up.z)
		{
			E0  = mix(E0 , P[2],  0.75);
			E2  = mix(E2 , P[2],  0.25);
		}
		else
		{
			E0  = mix(E0 , P[2],  0.5);
		}
	}
    
	if (edr.w)
	{
		if (edr_left.w && edr_up.w)
		{
			E2  = mix(E2 , P[3],  0.833333);
			E0  = mix(E0 , P[3],  0.25);
			E3  = mix(E3 , P[3],  0.25);
		}
		else if (edr_left.w)
		{
			E2  = mix(E2 , P[3],  0.75);
			E0  = mix(E0 , P[3],  0.25);
		}
		else if (edr_up.w)
		{
			E2  = mix(E2 , P[3],  0.75);
			E3  = mix(E3 , P[3],  0.25);
		}
		else
		{
			E2  = mix(E2 , P[3],  0.5);
		}
	}
    
    
    
    
    
	vec3 res = (fp.x < 0.50) ? (fp.y < 0.50 ? E0 : E2) : (fp.y < 0.50 ? E1: E3);
    
	gl_FragColor = vec4(res, 1.0);

}
