/*
   
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

uniform vec4 OGL2Param;
uniform vec4 OGL2Size;

void main()
{
	//float x = (OGL2Size.x/2048.0)*OGL2Param.x;
	//float y = (OGL2Size.y/1024.0)*OGL2Param.y;

	float x = 0.5;
	float y = 0.4;

	vec2 dg1 = vec2( x,y);  vec2 dg2 = vec2(-x,y);
	vec2 sd1 = dg1*0.5;     vec2 sd2 = dg2*0.5;
	vec2 ddx = vec2(x,0.0); vec2 ddy = vec2(0.0,y);

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1].xy = gl_TexCoord[0].xy - sd1;
	gl_TexCoord[2].xy = gl_TexCoord[0].xy - sd2;
	gl_TexCoord[3].xy = gl_TexCoord[0].xy + sd1;
	gl_TexCoord[4].xy = gl_TexCoord[0].xy + sd2;
	gl_TexCoord[5].xy = gl_TexCoord[0].xy - dg1;
	gl_TexCoord[6].xy = gl_TexCoord[0].xy + dg1;
	gl_TexCoord[5].zw = gl_TexCoord[0].xy - dg2;
	gl_TexCoord[6].zw = gl_TexCoord[0].xy + dg2;
	gl_TexCoord[1].zw = gl_TexCoord[0].xy - ddy;
	gl_TexCoord[2].zw = gl_TexCoord[0].xy + ddx;
	gl_TexCoord[3].zw = gl_TexCoord[0].xy + ddy;
	gl_TexCoord[4].zw = gl_TexCoord[0].xy - ddx;
}