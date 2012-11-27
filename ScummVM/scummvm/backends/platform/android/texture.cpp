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

#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "base/main.h"
#include "graphics/surface.h"

#include "common/rect.h"
#include "common/array.h"
#include "common/util.h"
#include "common/tokenizer.h"

#include "backends/platform/android/texture.h"
#include "backends/platform/android/android.h"

// Supported GL extensions
static bool npot_supported = false;

static inline GLfixed xdiv(int numerator, int denominator) {
	assert(numerator < (1 << 16));
	return (numerator << 16) / denominator;
}

template<class T>
static T nextHigher2(T k) {
	if (k == 0)
		return 1;
	--k;

	for (uint i = 1; i < sizeof(T) * CHAR_BIT; i <<= 1)
		k = k | k >> i;

	return k + 1;
}

void GLESBaseTexture::initGLExtensions() {
	const char *ext_string =
		reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));

	LOGI("Extensions: %s", ext_string);

	Common::StringTokenizer tokenizer(ext_string, " ");
	while (!tokenizer.empty()) {
		Common::String token = tokenizer.nextToken();

		if (token == "GL_ARB_texture_non_power_of_two")
			npot_supported = true;
	}
}

GLESBaseTexture::GLESBaseTexture(GLenum glFormat, GLenum glType,
									Graphics::PixelFormat pixelFormat) :
	_glFormat(glFormat),
	_glType(glType),
	_glFilter(GL_NEAREST),
	_texture_name(0),
	_surface(),
	_texture_width(0),
	_texture_height(0),
	_draw_rect(),
	_all_dirty(false),
	_dirty_rect(),
	_pixelFormat(pixelFormat),
	_palettePixelFormat()
{
	GLCALL(glGenTextures(1, &_texture_name));
}

GLESBaseTexture::~GLESBaseTexture() {
	release();
}

void GLESBaseTexture::release() {
	if (_texture_name) {
		LOGD("Destroying texture %u", _texture_name);

		GLCALL(glDeleteTextures(1, &_texture_name));
		_texture_name = 0;
	}
}

void GLESBaseTexture::reinit() {
	GLCALL(glGenTextures(1, &_texture_name));

	initSize();

	setDirty();
}

void GLESBaseTexture::initSize() {
	// Allocate room for the texture now, but pixel data gets uploaded
	// later (perhaps with multiple TexSubImage2D operations).
	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
	GLCALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, _glFormat,
						_texture_width, _texture_height,
						0, _glFormat, _glType, 0));
}

void GLESBaseTexture::setLinearFilter(bool value) {
	if (value)
		_glFilter = GL_LINEAR;
	else
		_glFilter = GL_NEAREST;

	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glFilter));
	GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _glFilter));
}

void GLESBaseTexture::allocBuffer(GLuint w, GLuint h) {
	_surface.w = w;
	_surface.h = h;
	_surface.format = _pixelFormat;

	if (w == _texture_width && h == _texture_height)
		return;

	if (npot_supported) {
		_texture_width = _surface.w;
		_texture_height = _surface.h;
	} else {
		_texture_width = nextHigher2(_surface.w);
		_texture_height = nextHigher2(_surface.h);
	}

	initSize();
}

void GLESBaseTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

	const GLfixed tex_width = xdiv(_surface.w, _texture_width);
	const GLfixed tex_height = xdiv(_surface.h, _texture_height);
	const GLfixed texcoords[] = {
		0, 0,
		tex_width, 0,
		0, tex_height,
		tex_width, tex_height,
	};

	GLCALL(glTexCoordPointer(2, GL_FIXED, 0, texcoords));

	const GLshort vertices[] = {
		x, y,
		x + w, y,
		x, y + h,
		x + w, y + h,
	};

	GLCALL(glVertexPointer(2, GL_SHORT, 0, vertices));

	assert(ARRAYSIZE(vertices) == ARRAYSIZE(texcoords));
	GLCALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, ARRAYSIZE(vertices) / 2));

	clearDirty();
}

const Graphics::PixelFormat &GLESBaseTexture::getPixelFormat() const {
	return _pixelFormat;
}

GLESTexture::GLESTexture(GLenum glFormat, GLenum glType,
							Graphics::PixelFormat pixelFormat) :
	GLESBaseTexture(glFormat, glType, pixelFormat),
	_pixels(0),
	_buf(0) {
}

GLESTexture::~GLESTexture() {
	delete[] _buf;
	delete[] _pixels;
}

void GLESTexture::allocBuffer(GLuint w, GLuint h) {
	GLuint oldw = _surface.w;
	GLuint oldh = _surface.h;

	GLESBaseTexture::allocBuffer(w, h);

	_surface.pitch = w * _pixelFormat.bytesPerPixel;

	if (_surface.w == oldw && _surface.h == oldh) {
		fillBuffer(0);
		return;
	}

	delete[] _buf;
	delete[] _pixels;

	_pixels = new byte[w * h * _surface.format.bytesPerPixel];
	assert(_pixels);

	_surface.pixels = _pixels;

	fillBuffer(0);

	_buf = new byte[w * h * _surface.format.bytesPerPixel];
	assert(_buf);
}

void GLESTexture::updateBuffer(GLuint x, GLuint y, GLuint w, GLuint h,
								const void *buf, int pitch_buf) {
	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	const byte *src = (const byte *)buf;
	byte *dst = _pixels + y * _surface.pitch + x * _surface.format.bytesPerPixel;

	do {
		memcpy(dst, src, w * _surface.format.bytesPerPixel);
		dst += _surface.pitch;
		src += pitch_buf;
	} while (--h);
}

void GLESTexture::fillBuffer(uint32 color) {
	assert(_surface.pixels);

	if (_pixelFormat.bytesPerPixel == 1 ||
			((color & 0xff) == ((color >> 8) & 0xff)))
		memset(_pixels, color & 0xff, _surface.pitch * _surface.h);
	else
		Common::fill(_pixels, _pixels + _surface.pitch * _surface.h,
						(uint16)color);

	setDirty();
}

void GLESTexture::drawTexture(GLshort x, GLshort y, GLshort w, GLshort h) {
	if (_all_dirty) {
		_dirty_rect.top = 0;
		_dirty_rect.left = 0;
		_dirty_rect.bottom = _surface.h;
		_dirty_rect.right = _surface.w;

		_all_dirty = false;
	}

	if (!_dirty_rect.isEmpty()) {
		byte *_tex;

		int16 dwidth = _dirty_rect.width();
		int16 dheight = _dirty_rect.height();

		if (dwidth == _surface.w) {
			_tex = _pixels + _dirty_rect.top * _surface.pitch;
		} else {
			_tex = _buf;

			byte *src = _pixels + _dirty_rect.top * _surface.pitch +
						_dirty_rect.left * _surface.format.bytesPerPixel;
			byte *dst = _buf;

			uint16 l = dwidth * _surface.format.bytesPerPixel;

			for (uint16 i = 0; i < dheight; ++i) {
				memcpy(dst, src, l);
				src += _surface.pitch;
				dst += l;
			}
		}

		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));
		GLCALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0,
								_dirty_rect.left, _dirty_rect.top,
								dwidth, dheight, _glFormat, _glType, _tex));
	}

	GLESBaseTexture::drawTexture(x, y, w, h);
}

GLES4444Texture::GLES4444Texture() :
	GLESTexture(GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, pixelFormat()) {
}

GLES4444Texture::~GLES4444Texture() {
}

GLES5551Texture::GLES5551Texture() :
	GLESTexture(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pixelFormat()) {
}

GLES5551Texture::~GLES5551Texture() {
}

GLES565Texture::GLES565Texture() :
	GLESTexture(GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pixelFormat()) {
}

GLES565Texture::~GLES565Texture() {
}

GLESFakePaletteTexture::GLESFakePaletteTexture(GLenum glFormat, GLenum glType,
									Graphics::PixelFormat pixelFormat) :
	GLESBaseTexture(glFormat, glType, pixelFormat),
	_palette(0),
	_pixels(0),
	_buf(0)
{
	_palettePixelFormat = pixelFormat;
	_fake_format = Graphics::PixelFormat::createFormatCLUT8();

	_palette = new uint16[256];
	assert(_palette);

	memset(_palette, 0, 256 * 2);
}

GLESFakePaletteTexture::~GLESFakePaletteTexture() {
	delete[] _buf;
	delete[] _pixels;
	delete[] _palette;
}

void GLESFakePaletteTexture::allocBuffer(GLuint w, GLuint h) {
	GLuint oldw = _surface.w;
	GLuint oldh = _surface.h;

	GLESBaseTexture::allocBuffer(w, h);

	_surface.format = Graphics::PixelFormat::createFormatCLUT8();
	_surface.pitch = w;

	if (_surface.w == oldw && _surface.h == oldh) {
		fillBuffer(0);
		return;
	}

	delete[] _buf;
	delete[] _pixels;

	_pixels = new byte[w * h];
	assert(_pixels);

	// fixup surface, for the outside this is a CLUT8 surface
	_surface.pixels = _pixels;

	fillBuffer(0);

	_buf = new uint16[w * h];
	assert(_buf);
}

void GLESFakePaletteTexture::fillBuffer(uint32 color) {
	assert(_surface.pixels);
	memset(_surface.pixels, color & 0xff, _surface.pitch * _surface.h);
	setDirty();
}

void GLESFakePaletteTexture::updateBuffer(GLuint x, GLuint y, GLuint w,
											GLuint h, const void *buf,
											int pitch_buf) {
	setDirtyRect(Common::Rect(x, y, x + w, y + h));

	const byte *src = (const byte *)buf;
	byte *dst = _pixels + y * _surface.pitch + x;

	do {
		memcpy(dst, src, w);
		dst += _surface.pitch;
		src += pitch_buf;
	} while (--h);
}

void GLESFakePaletteTexture::drawTexture(GLshort x, GLshort y, GLshort w,
										GLshort h) {
	if (_all_dirty) {
		_dirty_rect.top = 0;
		_dirty_rect.left = 0;
		_dirty_rect.bottom = _surface.h;
		_dirty_rect.right = _surface.w;

		_all_dirty = false;
	}

	if (!_dirty_rect.isEmpty()) {
		int16 dwidth = _dirty_rect.width();
		int16 dheight = _dirty_rect.height();

		byte *src = _pixels + _dirty_rect.top * _surface.pitch +
					_dirty_rect.left;
		uint16 *dst = _buf;
		uint pitch_delta = _surface.pitch - dwidth;

		for (uint16 j = 0; j < dheight; ++j) {
			for (uint16 i = 0; i < dwidth; ++i)
				*dst++ = _palette[*src++];
			src += pitch_delta;
		}

		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_name));

		GLCALL(glTexSubImage2D(GL_TEXTURE_2D, 0,
								_dirty_rect.left, _dirty_rect.top,
								dwidth, dheight, _glFormat, _glType, _buf));
	}

	GLESBaseTexture::drawTexture(x, y, w, h);
}

const Graphics::PixelFormat &GLESFakePaletteTexture::getPixelFormat() const {
	return _fake_format;
}

GLESFakePalette565Texture::GLESFakePalette565Texture() :
	GLESFakePaletteTexture(GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
							GLES565Texture::pixelFormat()) {
}

GLESFakePalette565Texture::~GLESFakePalette565Texture() {
}

GLESFakePalette5551Texture::GLESFakePalette5551Texture() :
	GLESFakePaletteTexture(GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
							GLES5551Texture::pixelFormat()) {
}

GLESFakePalette5551Texture::~GLESFakePalette5551Texture() {
}

#endif
