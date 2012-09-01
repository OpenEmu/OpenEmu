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
 * Shaders2D.h
 * 
 * Header file containing the 2D vertex and fragment shaders.
 */

#ifndef INCLUDED_SHADERS2D_H
#define INCLUDED_SHADERS2D_H

// Vertex shader
static const char vertexShaderSource[] =
{
"/**																				\n"
" ** Supermodel																		\n"
" ** A Sega Model 3 Arcade Emulator.												\n"
" ** Copyright 2011 Bart Trzynadlowski 												\n"
" **																				\n"
" ** This file is part of Supermodel.												\n"
" **																				\n"
" ** Supermodel is free software: you can redistribute it and/or modify it under	\n"
" ** the terms of the GNU General Public License as published by the Free 			\n"
" ** Software Foundation, either version 3 of the License, or (at your option)		\n"
" ** any later version.																\n"
" **																				\n"
" ** Supermodel is distributed in the hope that it will be useful, but WITHOUT		\n"
" ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or			\n"
" ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for		\n"
" ** more details.																	\n"
" **																				\n"
" ** You should have received a copy of the GNU General Public License along		\n"
" ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.					\n"
" **/																				\n"
"\n"
"/*															\n"
" * Vertex2D.glsl											\n"
" *															\n"
" * Vertex shader for 2D tilemap rendering.					\n"
" */														\n"
" 															\n"
"#version 120												\n"
"\n"
"void main(void)											\n"
"{															\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;						\n"
"	gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;	\n"
"}\n"
};

// Fragment shader
static const char fragmentShaderSource[] = 
{
"/**																				\n"
" ** Supermodel																		\n"
" ** A Sega Model 3 Arcade Emulator.												\n"
" ** Copyright 2011 Bart Trzynadlowski 												\n"
" **																				\n"
" ** This file is part of Supermodel.												\n"
" **																				\n"
" ** Supermodel is free software: you can redistribute it and/or modify it under	\n"
" ** the terms of the GNU General Public License as published by the Free 			\n"
" ** Software Foundation, either version 3 of the License, or (at your option)		\n"
" ** any later version.																\n"
" **																				\n"
" ** Supermodel is distributed in the hope that it will be useful, but WITHOUT		\n"
" ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or			\n"
" ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for		\n"
" ** more details.																	\n"
" **																				\n"
" ** You should have received a copy of the GNU General Public License along		\n"
" ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.					\n"
" **/																				\n"
"\n"
"/*																		\n"
" * Fragment2D.glsl														\n"
" *																		\n"
" * Fragment shader for 2D tilemap rendering.							\n"
" */																	\n"
"\n"
"#version 120															\n"
"\n"
"// Global uniforms														\n"
"uniform sampler2D	textureMap;		// 512x512 layer surface			\n"
"uniform vec3		colorOffset;	// color offset for this layer		\n"
"\n"
"/*																		\n"
" * main():																\n"
" *																		\n"
" * Fragment shader entry point.										\n"
" */																	\n"
"\n"
"void main(void)														\n"
"{																		\n"
"	gl_FragColor = texture2D(textureMap, gl_TexCoord[0].st);			\n"
"	gl_FragColor.rgb = clamp(gl_FragColor.rgb+colorOffset,0.0,1.0);		\n"
"}\n"
};

#endif	// INCLUDED_SHADERS2D_H
