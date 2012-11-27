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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_OPENGL_GLTEXTURE_H
#define BACKENDS_GRAPHICS_OPENGL_GLTEXTURE_H

#include "common/scummsys.h"

#ifdef WIN32
#if defined(ARRAYSIZE) && !defined(_WINDOWS_)
#undef ARRAYSIZE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE
#endif

// HACK: At this point in Windows platforms, common/util.h has been included
// via common/rect.h (from backends/graphics/sdl/sdl-graphics.h), via
// backends/graphics/openglsdl/openglsdl-graphics.h. Thus, we end up with
// COMMON_UTIL_H defined, and ARRAYSIZE undefined (bad!). Therefore,
// ARRAYSIZE is undefined in openglsdl-graphics.cpp. This is a temporary
// hackish solution fo fix compilation under Windows.
#if !defined(ARRAYSIZE) && defined(COMMON_UTIL_H)
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))
#endif

#if defined(BADA)
#include <FGraphicsOpengl.h>
using namespace Osp::Graphics::Opengl;
#elif defined(USE_GLES)
#include <GLES/gl.h>
#elif defined(SDL_BACKEND)
#include <SDL_opengl.h>
#else
#include <GL/gl.h>
#endif

#include "graphics/surface.h"

/**
 * OpenGL texture manager class
 */
class GLTexture {
public:
	/**
	 * Initialize OpenGL Extensions
	 */
	static void initGLExtensions();

	GLTexture(byte bpp, GLenum internalFormat, GLenum format, GLenum type);
	~GLTexture();

	/**
	 * Refresh the texture after a context change. The
	 * process will be completed on next allocBuffer call.
	 */
	void refresh();

	/**
	 * Allocates memory needed for the given size.
	 */
	void allocBuffer(GLuint width, GLuint height);

	/**
	 * Updates the texture pixels.
	 */
	void updateBuffer(const void *buf, int pitch, GLuint x, GLuint y,
		GLuint w, GLuint h);

	/**
	 * Draws the texture to the screen buffer.
	 */
	void drawTexture(GLshort x, GLshort y, GLshort w, GLshort h);

	/**
	 * Get the texture width.
	 */
	GLuint getWidth() const { return _realWidth; }

	/**
	 * Get the texture height.
	 */
	GLuint getHeight() const { return _realHeight; }

	/**
	 * Get the bytes per pixel.
	 */
	uint getBytesPerPixel() const { return _bytesPerPixel; }

	/**
	 * Set the texture filter.
	 * @filter the filter type, GL_NEAREST or GL_LINEAR
	 */
	void setFilter(GLint filter) { _filter = filter; }

private:
	const byte _bytesPerPixel;
	const GLenum _internalFormat;
	const GLenum _glFormat;
	const GLenum _glType;

	GLuint _realWidth;
	GLuint _realHeight;
	GLuint _textureName;
	GLuint _textureWidth;
	GLuint _textureHeight;
	GLint _filter;
	bool _refresh;
};

#endif
