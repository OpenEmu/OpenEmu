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
 * Shader.cpp
 * 
 * OpenGL shader management.
 *
 * To-Do List
 * ----------
 * - Mesa crashes because, evidently, the function pointers are invalid. Mesa
 *   returns the following information:
 *		                   Vendor: Mesa project: www.mesa3d.org
 *		                 Renderer: Mesa GLX Indirect
 *		                  Version: 1.2 (1.5 Mesa 6.5.1)
 *		 Shading Language Version: (null)
 *		Maximum Vertex Array Size: -1 vertices
 *		     Maximum Texture Size: 2048 texels
 *		Maximum Vertex Attributes: 16
 *		  Maximum Vertex Uniforms: 16
 * - Check for OpenGL 2.0 and perhaps check some of the function pointers,
 *   which will be NULL, if GL 2.0 and shaders are not supported.
 * - Keep in mind that all these checks should probably go somewhere else...
 * - Turn this into a class.
 */

#include <new>
#include <stdio.h>
#include "Pkgs/glew.h"
#include "Supermodel.h"


// Load a source file. Pointer returned must be freed by caller. Returns NULL if failed.
static char *LoadShaderSource(const char *file)
{
	FILE	*fp;
	char	*buf;
	int		size;

	// Open shader and get the file size
	fp = fopen(file, "r");
	if (NULL == fp)
	{
		ErrorLog("Unable to open shader source file: %s", file);
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);

	// Allocate memory and read it in
	buf = new(std::nothrow) char[size+1];
	if (NULL == buf)
	{
		ErrorLog("Insufficient memory to load shader source file: %s", file);
		fclose(fp);
		return NULL;
	}
	buf[size] = '\0';	// for safety, actual size might be smaller once newline characters are converted
	size = fread(buf, sizeof(char), size, fp);
	buf[size] = '\0';
	
	fclose(fp);
	return buf;
}

bool LoadShaderProgram(GLuint *shaderProgramPtr, GLuint *vertexShaderPtr, GLuint *fragmentShaderPtr, const char *vsFile, const char *fsFile, const char *vsString, const char *fsString)
{
	char		infoLog[2048];
	const char	*vsSource, *fsSource;	// source code
	GLuint		shaderProgram, vertexShader, fragmentShader;
	GLint		result, len;
	bool		ret = OKAY;
	
	// Load shaders from files if specified
	if (vsFile != NULL)
		vsSource = LoadShaderSource(vsFile);
	else
		vsSource = vsString;
	if (fsFile != NULL)
		fsSource = LoadShaderSource(fsFile);
	else
		fsSource = fsString;
	if (vsSource == NULL || fsSource == NULL)
	{
		ret = FAIL;
		goto Quit;
	}

	// Ensure that shader support exists
	if ((glCreateProgram==NULL) || (glCreateShader==NULL) || (glShaderSource==NULL) || (glCompileShader==NULL))
	{
		ret = FAIL;
		ErrorLog("OpenGL 2.x does not appear to be present. Unable to proceed.");
		goto Quit;
	}
	
	// Create the shaders and shader program
	shaderProgram	= glCreateProgram();
	vertexShader	= glCreateShader(GL_VERTEX_SHADER);
	fragmentShader 	= glCreateShader(GL_FRAGMENT_SHADER);
	*shaderProgramPtr	= shaderProgram;
	*vertexShaderPtr 	= vertexShader;
	*fragmentShaderPtr 	= fragmentShader;
	
	// Attempt to compile vertex shader
	glShaderSource(vertexShader, 1, (const GLchar **) &vsSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)	// failed to compile
	{
		glGetShaderInfoLog(vertexShader, 2048, &len, infoLog);
		ErrorLog("Vertex shader failed to compile. Your OpenGL driver said:\n%s", infoLog);
		ret = FAIL;	// error
	}
	
	// Attempt to compile fragment shader
	glShaderSource(fragmentShader, 1, (const GLchar **) &fsSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)	// failed to compile
	{
		glGetShaderInfoLog(fragmentShader, 2048, &len, infoLog);
		ErrorLog("Fragment shader failed to compile. Your OpenGL driver said:\n%s", infoLog);
		ret = FAIL;	// error
	}
	
	// Link
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &result);
	if (result == GL_FALSE)
	{
		glGetProgramInfoLog(shaderProgram, 2048, &len, infoLog);
		ErrorLog("Failed to link shader objects. Your OpenGL driver said:\n%s\n", infoLog);
		ret = FAIL;	// error
	}

	// Enable the shader (if no errors)
	if (ret == OKAY)
		glUseProgram(shaderProgram);

	// Clean up and quit 
Quit:
	if ((vsSource != NULL) && (vsFile != NULL))	// loaded from file, must delete
		delete [] vsSource;
	if ((fsSource != NULL) && (fsFile != NULL))	// ""
		delete [] fsSource;
	return ret;
}

void DestroyShaderProgram(GLuint shaderProgram, GLuint vertexShader, GLuint fragmentShader)
{
	// In case LoadShaderProgram() failed above due to lack of OpenGL 2.0+ functions...
	if ((glUseProgram==NULL) || (glDeleteShader==NULL) || (glDeleteProgram==NULL))
		return;

	glUseProgram(0);	// return to fixed function pipeline
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(shaderProgram);
}
