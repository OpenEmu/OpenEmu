/*
   4xGLSLScale shader
   
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
	vec3 dt = vec3(1.0,1.0,1.0);

	float ko1=dot(abs(o1-c),dt);
	float ko2=dot(abs(o2-c),dt);
	float ko3=dot(abs(o3-c),dt);
	float ko4=dot(abs(o4-c),dt);

	float k1=min(dot(abs(i1-i3),dt),dot(abs(o1-o3),dt));
	float k2=min(dot(abs(i2-i4),dt),dot(abs(o2-o4),dt));

	float w1 = k2; if(ko3<ko1) w1 = 0.0;
	float w2 = k1; if(ko4<ko2) w2 = 0.0;
	float w3 = k2; if(ko1<ko3) w3 = 0.0;
	float w4 = k1; if(ko2<ko4) w4 = 0.0;

	gl_FragColor.xyz = (w1*o1+w2*o2+w3*o3+w4*o4+0.0001*c)/(w1+w2+w3+w4+0.0001);
	gl_FragColor.a = 1.0;
}




