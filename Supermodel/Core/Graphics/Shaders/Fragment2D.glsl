/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson 
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * Fragment2D.glsl
 *
 * Fragment shader for 2D tilemap rendering.
 */

#version 120

// Global uniforms
uniform sampler2D	textureMap;		// 512x512 layer surface
uniform vec3		colorOffset;	// color offset for this layer

/*
 * main():
 *
 * Fragment shader entry point.
 */

void main(void)
{	
	gl_FragColor = texture2D(textureMap, gl_TexCoord[0].st);
	gl_FragColor.rgb = clamp(gl_FragColor.rgb+colorOffset,0.0,1.0);
}
