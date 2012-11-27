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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#if defined(USE_OPENGL)

#include "backends/graphics/opengl/gltexture.h"
#include "backends/graphics/opengl/glerrorcheck.h"

#include "common/rect.h"
#include "common/array.h"
#include "common/util.h"
#include "common/tokenizer.h"

// Supported GL extensions
static bool npot_supported = false;
static bool glext_inited = false;

/*static inline GLint xdiv(int numerator, int denominator) {
	assert(numerator < (1 << 16));
	return (numerator << 16) / denominator;
}*/

static GLuint nextHigher2(GLuint v) {
	if (v == 0)
		return 1;
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return ++v;
}

void GLTexture::initGLExtensions() {

	// Return if extensions were already checked
	if (glext_inited)
		return;

	// Get a string with all extensions
	const char *ext_string = (const char *)glGetString(GL_EXTENSIONS);
	CHECK_GL_ERROR();
	Common::StringTokenizer tokenizer(ext_string, " ");
	// Iterate all string tokens
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();
		if (token == "GL_ARB_texture_non_power_of_two")
			npot_supported = true;
	}

	glext_inited = true;
}

GLTexture::GLTexture(byte bpp, GLenum internalFormat, GLenum format, GLenum type)
	:
	_bytesPerPixel(bpp),
	_internalFormat(internalFormat),
	_glFormat(format),
	_glType(type),
	_textureWidth(0),
	_textureHeight(0),
	_realWidth(0),
	_realHeight(0),
	_refresh(false),
	_filter(GL_NEAREST) {

	// Generate the texture ID
	glGenTextures(1, &_textureName); CHECK_GL_ERROR();
}

GLTexture::~GLTexture() {
	// Delete the texture
	glDeleteTextures(1, &_textureName); CHECK_GL_ERROR();
}

void GLTexture::refresh() {
	// Delete previous texture
	glDeleteTextures(1, &_textureName); CHECK_GL_ERROR();

	// Generate the texture ID
	glGenTextures(1, &_textureName); CHECK_GL_ERROR();
	_refresh = true;
}

void GLTexture::allocBuffer(GLuint w, GLuint h) {
	_realWidth = w;
	_realHeight = h;

	if (w <= _textureWidth && h <= _textureHeight && !_refresh)
		// Already allocated a sufficiently large buffer
		return;

	if (npot_supported) {
		_textureWidth = w;
		_textureHeight = h;
	} else {
		_textureWidth = nextHigher2(w);
		_textureHeight = nextHigher2(h);
	}

	// Select this OpenGL texture
	glBindTexture(GL_TEXTURE_2D, _textureName); CHECK_GL_ERROR();

	// Set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filter); CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filter); CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); CHECK_GL_ERROR();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); CHECK_GL_ERROR();

	// Allocate room for the texture
	glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat,
	             _textureWidth, _textureHeight, 0, _glFormat, _glType, NULL); CHECK_GL_ERROR();

	_refresh = false;
}

void GLTexture::updateBuffer(const void *buf, int pitch, GLuint x, GLuint y, GLuint w, GLuint h) {
	// Skip empty updates.
	if (w * h == 0)
		return;

	// Select this OpenGL texture
	glBindTexture(GL_TEXTURE_2D, _textureName); CHECK_GL_ERROR();

	// Check if the buffer has its data contiguously
	if ((int)w * _bytesPerPixel == pitch) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
		                _glFormat, _glType, buf); CHECK_GL_ERROR();
	} else {
		// Update the texture row by row
		const byte *src = (const byte *)buf;
		do {
			glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,
			                w, 1, _glFormat, _glType, src); CHECK_GL_ERROR();
			++y;
			src += pitch;
		} while (--h);
	}
}

void GLTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	// Select this OpenGL texture
	glBindTexture(GL_TEXTURE_2D, _textureName); CHECK_GL_ERROR();

	// Calculate the texture rect that will be drawn
	const GLfloat texWidth = (GLfloat)_realWidth / _textureWidth;//xdiv(_surface.w, _textureWidth);
	const GLfloat texHeight = (GLfloat)_realHeight / _textureHeight;//xdiv(_surface.h, _textureHeight);
	const GLfloat texcoords[] = {
		0, 0,
		texWidth, 0,
		0, texHeight,
		texWidth, texHeight,
	};
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords); CHECK_GL_ERROR();

	// Calculate the screen rect where the texture will be drawn
	const GLshort vertices[] = {
		x,      y,
		x + w,  y,
		x,      y + h,
		x + w,  y + h,
	};
	glVertexPointer(2, GL_SHORT, 0, vertices); CHECK_GL_ERROR();

	// Draw the texture to the screen buffer
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); CHECK_GL_ERROR();
}

#endif
