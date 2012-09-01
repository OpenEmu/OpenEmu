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
 * Shader.h
 * 
 * OpenGL shader management functions.
 */

#ifndef INCLUDED_SHADER_H
#define INCLUDED_SHADER_H

#include "Pkgs/glew.h"

/*
 * LoadShaderProgram(shaderProgramPtr, vertexShaderPtr, fragmentShaderPtr, 
 *					 vsFile, fsFile, vsString, fsString):
 *
 * Loads and creates an OpenGL shader program.
 *
 * Parameters:
 *		shaderProgramPtr	Pointer to where OpenGL shader program handle is 
 *							written.
 *		vertexShaderPtr		Pointer to where vertex shader handle is written.
 *		fragmentShaderPtr	Pointer for fragment shader.
 *		vsFile				Vertex shader file path. If this is not NULL, the
 *							vertex shader is loaded from the file.
 *		fsFile				Fragment shader file path. If this is not NULL, the
 *							fragment shader is loaded from the file.
 *		vsString			String containing the vertex shader.
 *		fsString			String containing the fragment shader.
 *
 * Returns:
 *		OKAY is successfully loaded, otherwise FAIL. Prints own error messages.
 */
extern bool LoadShaderProgram(GLuint *shaderProgramPtr, GLuint *vertexShaderPtr, 
							  GLuint *fragmentShaderPtr, const char *vsFile, 
							  const char *fsFile, const char *vsString,
							  const char *fsString);

/*
 * DestroyShaderProgram(shaderProgram, vertexShader, fragmentShader):
 *
 * Removes shaders and returns to the fixed function OpenGL pipeline.
 *
 * Parameters:
 *		shaderProgram	Handle for shader program.
 *		vertexShader	Handle for vertex shader.
 *		fragmentShader	Handle for fragment shader.
 */
extern void DestroyShaderProgram(GLuint shaderProgram, GLuint vertexShader, 
								 GLuint fragmentShader);


#endif	// INCLUDED_SHADER_H
