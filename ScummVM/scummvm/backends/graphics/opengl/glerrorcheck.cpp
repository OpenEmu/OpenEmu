/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#if defined(DEBUG) && defined(USE_OPENGL)

#include "backends/graphics/opengl/glerrorcheck.h"
#include "common/textconsole.h"
#include "common/str.h"

#ifdef WIN32
#if defined(ARRAYSIZE) && !defined(_WINDOWS_)
#undef ARRAYSIZE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE
#endif

#if defined(USE_GLES)
#include <GLES/gl.h>
#elif defined(MACOSX)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

static Common::String getGlErrStr(GLenum error) {
	switch (error) {
	case GL_NO_ERROR:           return "GL_NO_ERROR";
	case GL_INVALID_ENUM:       return "GL_INVALID_ENUM";
	case GL_INVALID_OPERATION:  return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW:     return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:    return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:      return "GL_OUT_OF_MEMORY";
	}

	return Common::String::format("(Unknown GL error code 0x%x)", error);
}

void checkGlError(const char *file, int line) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		warning("%s:%d: GL error: %s", file, line, getGlErrStr(error).c_str());
}

#endif
