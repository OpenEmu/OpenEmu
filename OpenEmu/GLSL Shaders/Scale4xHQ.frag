/*
   4xGLSLHqFilter shader
   
   Copyright (C) 2005 guest(r) - guest.r@gmail.com

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


uniform sampler2DRect OGL2Texture;

const float mx = 1.00;     // start smoothing wt.
const float k = -1.10;     // wt. decrease factor
const float max_w = 0.75;  // max filter weigth
const float min_w = 0.03;   // min filter weigth
const float lum_add = 0.33; // effects smoothing

void main()
{
	vec3 c  = texture2DRect(OGL2Texture, gl_TexCoord[0].xy).xyz;
	vec3 i1 = texture2DRect(OGL2Texture, gl_TexCoord[1].xy).xyz; 
	vec3 i2 = texture2DRect(OGL2Texture, gl_TexCoord[2].xy).xyz; 
	vec3 i3 = texture2DRect(OGL2Texture, gl_TexCoord[3].xy).xyz; 
	vec3 i4 = texture2DRect(OGL2Texture, gl_TexCoord[4].xy).xyz; 
	vec3 o1 = texture2DRect(OGL2Texture, gl_TexCoord[5].xy).xyz; 
	vec3 o3 = texture2DRect(OGL2Texture, gl_TexCoord[6].xy).xyz; 
	vec3 o2 = texture2DRect(OGL2Texture, gl_TexCoord[5].zw).xyz;
	vec3 o4 = texture2DRect(OGL2Texture, gl_TexCoord[6].zw).xyz;
	vec3 s1 = texture2DRect(OGL2Texture, gl_TexCoord[1].zw).xyz; 
	vec3 s2 = texture2DRect(OGL2Texture, gl_TexCoord[2].zw).xyz; 
	vec3 s3 = texture2DRect(OGL2Texture, gl_TexCoord[3].zw).xyz; 
	vec3 s4 = texture2DRect(OGL2Texture, gl_TexCoord[4].zw).xyz;  
	vec3 dt = vec3(1.0,1.0,1.0);

	float ko1=dot(abs(o1-c),dt);
	float ko2=dot(abs(o2-c),dt);
	float ko3=dot(abs(o3-c),dt);
	float ko4=dot(abs(o4-c),dt);

	float k1=min(dot(abs(i1-i3),dt),max(ko1,ko3));
	float k2=min(dot(abs(i2-i4),dt),max(ko2,ko4));

	float w1 = k2; if(ko3<ko1) w1*=ko3/ko1;
	float w2 = k1; if(ko4<ko2) w2*=ko4/ko2;
	float w3 = k2; if(ko1<ko3) w3*=ko1/ko3;
	float w4 = k1; if(ko2<ko4) w4*=ko2/ko4;

	c=(w1*o1+w2*o2+w3*o3+w4*o4+0.001*c)/(w1+w2+w3+w4+0.001);

	w1 = k*dot(abs(i1-c)+abs(i3-c),dt)/(0.125*dot(i1+i3,dt)+lum_add);
	w2 = k*dot(abs(i2-c)+abs(i4-c),dt)/(0.125*dot(i2+i4,dt)+lum_add);
	w3 = k*dot(abs(s1-c)+abs(s3-c),dt)/(0.125*dot(s1+s3,dt)+lum_add);
	w4 = k*dot(abs(s2-c)+abs(s4-c),dt)/(0.125*dot(s2+s4,dt)+lum_add);

	w1 = clamp(w1+mx,min_w,max_w); 
	w2 = clamp(w2+mx,min_w,max_w);
	w3 = clamp(w3+mx,min_w,max_w); 
	w4 = clamp(w4+mx,min_w,max_w);

	gl_FragColor.xyz=(w1*(i1+i3)+w2*(i2+i4)+w3*(s1+s3)+w4*(s2+s4)+c)/(2.0*(w1+w2+w3+w4)+1.0);
	gl_FragColor.a = 1.0;
}




