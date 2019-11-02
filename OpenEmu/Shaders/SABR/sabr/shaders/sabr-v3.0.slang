#version 450

/*
	SABR v3.0 Shader
	Joshua Street
	
	Portions of this algorithm were taken from Hyllian's 5xBR v3.7c
	shader.
	
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

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
} params;

layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
} global;

/*
	Constants
*/
/*
	Inequation coefficients for interpolation
Equations are in the form: Ay + Bx = C
45, 30, and 60 denote the angle from x each line the cooeficient variable set builds
*/
const vec4 Ai  = vec4( 1.0, -1.0, -1.0,  1.0);
const vec4 B45 = vec4( 1.0,  1.0, -1.0, -1.0);
const vec4 C45 = vec4( 1.5,  0.5, -0.5,  0.5);
const vec4 B30 = vec4( 0.5,  2.0, -0.5, -2.0);
const vec4 C30 = vec4( 1.0,  1.0, -0.5,  0.0);
const vec4 B60 = vec4( 2.0,  0.5, -2.0, -0.5);
const vec4 C60 = vec4( 2.0,  0.0, -1.0,  0.5);

const vec4 M45 = vec4(0.4, 0.4, 0.4, 0.4);
const vec4 M30 = vec4(0.2, 0.4, 0.2, 0.4);
const vec4 M60 = M30.yxwz;
const vec4 Mshift = vec4(0.2);

// Coefficient for weighted edge detection
const float coef = 2.0;
// Threshold for if luminance values are "equal"
const vec4 threshold = vec4(0.32);

// Conversion from RGB to Luminance (from GIMP)
const vec3 lum = vec3(0.21, 0.72, 0.07);

// Performs same logic operation as && for vectors
bvec4 _and_(bvec4 A, bvec4 B) {
	return bvec4(A.x && B.x, A.y && B.y, A.z && B.z, A.w && B.w);
}

// Performs same logic operation as || for vectors
bvec4 _or_(bvec4 A, bvec4 B) {
	return bvec4(A.x || B.x, A.y || B.y, A.z || B.z, A.w || B.w);
}

// Converts 4 3-color vectors into 1 4-value luminance vector
vec4 lum_to(vec3 v0, vec3 v1, vec3 v2, vec3 v3) {
	return vec4(dot(lum, v0), dot(lum, v1), dot(lum, v2), dot(lum, v3));
}

// Gets the difference between 2 4-value luminance vectors
vec4 lum_df(vec4 A, vec4 B) {
	return abs(A - B);
}

// Determines if 2 4-value luminance vectors are "equal" based on threshold
bvec4 lum_eq(vec4 A, vec4 B) {
	return lessThan(lum_df(A, B), threshold);
}

vec4 lum_wd(vec4 a, vec4 b, vec4 c, vec4 d, vec4 e, vec4 f, vec4 g, vec4 h) {
	return lum_df(a, b) + lum_df(a, c) + lum_df(d, e) + lum_df(d, f) + 4.0 * lum_df(g, h);
}

// Gets the difference between 2 3-value rgb colors
float c_df(vec3 c1, vec3 c2) {
	vec3 df = abs(c1 - c2);
	return df.r + df.g + df.b;
}

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 tc;
layout(location = 1) out vec4 xyp_1_2_3;
layout(location = 2) out vec4 xyp_5_10_15;
layout(location = 3) out vec4 xyp_6_7_8;
layout(location = 4) out vec4 xyp_9_14_9;
layout(location = 5) out vec4 xyp_11_12_13;
layout(location = 6) out vec4 xyp_16_17_18;
layout(location = 7) out vec4 xyp_21_22_23;

void main()
{
   gl_Position = global.MVP * Position;
   
   	float x = params.SourceSize.z;
	float y = params.SourceSize.w;
	
	tc = TexCoord * vec2(1.0004, 1.0);
	xyp_1_2_3    = tc.xxxy + vec4(      -x, 0.0,   x, -2.0 * y);
	xyp_6_7_8    = tc.xxxy + vec4(      -x, 0.0,   x,       -y);
	xyp_11_12_13 = tc.xxxy + vec4(      -x, 0.0,   x,      0.0);
	xyp_16_17_18 = tc.xxxy + vec4(      -x, 0.0,   x,        y);
	xyp_21_22_23 = tc.xxxy + vec4(      -x, 0.0,   x,  2.0 * y);
	xyp_5_10_15  = tc.xyyy + vec4(-2.0 * x,  -y, 0.0,        y);
	xyp_9_14_9   = tc.xyyy + vec4( 2.0 * x,  -y, 0.0,        y);
}

#pragma stage fragment
layout(location = 0) in vec2 tc;
layout(location = 1) in vec4 xyp_1_2_3;
layout(location = 2) in vec4 xyp_5_10_15;
layout(location = 3) in vec4 xyp_6_7_8;
layout(location = 4) in vec4 xyp_9_14_9;
layout(location = 5) in vec4 xyp_11_12_13;
layout(location = 6) in vec4 xyp_16_17_18;
layout(location = 7) in vec4 xyp_21_22_23;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

void main()
{
/*
Mask for algorhithm
+-----+-----+-----+-----+-----+
|     |  1  |  2  |  3  |     |
+-----+-----+-----+-----+-----+
|  5  |  6  |  7  |  8  |  9  |
+-----+-----+-----+-----+-----+
| 10  | 11  | 12  | 13  | 14  |
+-----+-----+-----+-----+-----+
| 15  | 16  | 17  | 18  | 19  |
+-----+-----+-----+-----+-----+
|     | 21  | 22  | 23  |     |
+-----+-----+-----+-----+-----+
	*/
	// Get mask values by performing texture lookup with the uniform sampler
	vec3 P1  = texture(Source, xyp_1_2_3.xw   ).rgb;
	vec3 P2  = texture(Source, xyp_1_2_3.yw   ).rgb;
	vec3 P3  = texture(Source, xyp_1_2_3.zw   ).rgb;
	
	vec3 P6  = texture(Source, xyp_6_7_8.xw   ).rgb;
	vec3 P7  = texture(Source, xyp_6_7_8.yw   ).rgb;
	vec3 P8  = texture(Source, xyp_6_7_8.zw   ).rgb;
	
	vec3 P11 = texture(Source, xyp_11_12_13.xw).rgb;
	vec3 P12 = texture(Source, xyp_11_12_13.yw).rgb;
	vec3 P13 = texture(Source, xyp_11_12_13.zw).rgb;
	
	vec3 P16 = texture(Source, xyp_16_17_18.xw).rgb;
	vec3 P17 = texture(Source, xyp_16_17_18.yw).rgb;
	vec3 P18 = texture(Source, xyp_16_17_18.zw).rgb;
	
	vec3 P21 = texture(Source, xyp_21_22_23.xw).rgb;
	vec3 P22 = texture(Source, xyp_21_22_23.yw).rgb;
	vec3 P23 = texture(Source, xyp_21_22_23.zw).rgb;
	
	vec3 P5  = texture(Source, xyp_5_10_15.xy ).rgb;
	vec3 P10 = texture(Source, xyp_5_10_15.xz ).rgb;
	vec3 P15 = texture(Source, xyp_5_10_15.xw ).rgb;
	
	vec3 P9  = texture(Source, xyp_9_14_9.xy  ).rgb;
	vec3 P14 = texture(Source, xyp_9_14_9.xz  ).rgb;
	vec3 P19 = texture(Source, xyp_9_14_9.xw  ).rgb;
	
	// Store luminance values of each point in groups of 4
	// so that we may operate on all four corners at once
	vec4 p7  = lum_to(P7,  P11, P17, P13);
	vec4 p8  = lum_to(P8,  P6,  P16, P18);
	vec4 p11 = p7.yzwx;                      // P11, P17, P13, P7
	vec4 p12 = lum_to(P12, P12, P12, P12);
	vec4 p13 = p7.wxyz;                      // P13, P7,  P11, P17
	vec4 p14 = lum_to(P14, P2,  P10, P22);
	vec4 p16 = p8.zwxy;                      // P16, P18, P8,  P6
	vec4 p17 = p7.zwxy;                      // P17, P13, P7,  P11
	vec4 p18 = p8.wxyz;                      // P18, P8,  P6,  P16
	vec4 p19 = lum_to(P19, P3,  P5,  P21);
	vec4 p22 = p14.wxyz;                     // P22, P14, P2,  P10
	vec4 p23 = lum_to(P23, P9,  P1,  P15);
	
	// Scale current texel coordinate to [0..1]
	vec2 fp = fract(tc * params.SourceSize.xy);
	
	// Determine amount of "smoothing" or mixing that could be done on texel corners
	vec4 ma45 = smoothstep(C45 - M45, C45 + M45, Ai * fp.y + B45 * fp.x);
	vec4 ma30 = smoothstep(C30 - M30, C30 + M30, Ai * fp.y + B30 * fp.x);
	vec4 ma60 = smoothstep(C60 - M60, C60 + M60, Ai * fp.y + B60 * fp.x);
	vec4 marn = smoothstep(C45 - M45 + Mshift, C45 + M45 + Mshift, Ai * fp.y + B45 * fp.x);
	
	// Perform edge weight calculations
	vec4 e45   = lum_wd(p12, p8, p16, p18, p22, p14, p17, p13);
	vec4 econt = lum_wd(p17, p11, p23, p13, p7, p19, p12, p18);
	vec4 e30   = lum_df(p13, p16);
	vec4 e60   = lum_df(p8, p17);
	
	// Calculate rule results for interpolation
	bvec4 r45_1   = _and_(notEqual(p12, p13), notEqual(p12, p17));
	bvec4 r45_2   = _and_(not(lum_eq(p13, p7)), not(lum_eq(p13, p8)));
	bvec4 r45_3   = _and_(not(lum_eq(p17, p11)), not(lum_eq(p17, p16)));
	bvec4 r45_4_1 = _and_(not(lum_eq(p13, p14)), not(lum_eq(p13, p19)));
	bvec4 r45_4_2 = _and_(not(lum_eq(p17, p22)), not(lum_eq(p17, p23)));
	bvec4 r45_4   = _and_(lum_eq(p12, p18), _or_(r45_4_1, r45_4_2));
	bvec4 r45_5   = _or_(lum_eq(p12, p16), lum_eq(p12, p8));
	bvec4 r45     = _and_(r45_1, _or_(_or_(_or_(r45_2, r45_3), r45_4), r45_5));
	bvec4 r30 = _and_(notEqual(p12, p16), notEqual(p11, p16));
	bvec4 r60 = _and_(notEqual(p12, p8), notEqual(p7, p8));
	
	// Combine rules with edge weights
	bvec4 edr45 = _and_(lessThan(e45, econt), r45);
	bvec4 edrrn = lessThanEqual(e45, econt);
	bvec4 edr30 = _and_(lessThanEqual(coef * e30, e60), r30);
	bvec4 edr60 = _and_(lessThanEqual(coef * e60, e30), r60);
	
	// Finalize interpolation rules and cast to float (0.0 for false, 1.0 for true)
	vec4 final45 = vec4(_and_(_and_(not(edr30), not(edr60)), edr45));
	vec4 final30 = vec4(_and_(_and_(edr45, not(edr60)), edr30));
	vec4 final60 = vec4(_and_(_and_(edr45, not(edr30)), edr60));
	vec4 final36 = vec4(_and_(_and_(edr60, edr30), edr45));
	vec4 finalrn = vec4(_and_(not(edr45), edrrn));
	
	// Determine the color to mix with for each corner
	vec4 px = step(lum_df(p12, p17), lum_df(p12, p13));
	
	// Determine the mix amounts by combining the final rule result and corresponding
	// mix amount for the rule in each corner
	vec4 mac = final36 * max(ma30, ma60) + final30 * ma30 + final60 * ma60 + final45 * ma45 + finalrn * marn;
	
/*
Calculate the resulting color by traversing clockwise and counter-clockwise around
the corners of the texel

Finally choose the result that has the largest difference from the texel's original
color
*/
	vec3 res1 = P12;
	res1 = mix(res1, mix(P13, P17, px.x), mac.x);
	res1 = mix(res1, mix(P7, P13, px.y), mac.y);
	res1 = mix(res1, mix(P11, P7, px.z), mac.z);
	res1 = mix(res1, mix(P17, P11, px.w), mac.w);
	
	vec3 res2 = P12;
	res2 = mix(res2, mix(P17, P11, px.w), mac.w);
	res2 = mix(res2, mix(P11, P7, px.z), mac.z);
	res2 = mix(res2, mix(P7, P13, px.y), mac.y);
	res2 = mix(res2, mix(P13, P17, px.x), mac.x);
	
	FragColor = vec4(mix(res1, res2, step(c_df(P12, res1), c_df(P12, res2))), 1.0);
}