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

#include "tucker/graphics.h"

namespace Tucker {

int Graphics::encodeRLE(const uint8 *src, uint8 *dst, int w, int h) {
	int sz = 0;
	int count = 0;
	int code = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			code = src[x];
			if (code == 0) {
				++count;
				if (count > 200) {
					dst[sz++] = 0;
					dst[sz++] = count;
					count = 0;
				}
			} else {
				if (count > 0) {
					dst[sz++] = 0;
					dst[sz++] = count;
					count = 0;
				}
				dst[sz++] = code;
			}
		}
		src += 320;
	}
	if (count > 0) {
		dst[sz++] = 0;
		dst[sz++] = count;
	}
	return sz;
}

int Graphics::encodeRAW(const uint8 *src, uint8 *dst, int w, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(dst, src, w);
		dst += w;
		src += 320;
	}
	return w * h;
}

void Graphics::decodeRLE(uint8 *dst, const uint8 *src, int w, int h) {
	int code = 0;
	int color = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (code == 0) {
				color = *src++;
				if (color == 0) {
					code = *src++;
				}
			}
			if (color != 0) {
				dst[x] = color;
			} else {
				--code;
			}
		}
		dst += 640;
	}
}

void Graphics::decodeRLE_224(uint8 *dst, const uint8 *src, int w, int h) {
	int code = 0;
	int color = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (code == 0) {
				color = *src++;
				if (color == 0) {
					code = *src++;
				}
			}
			if (color != 0) {
				if (dst[x] < 0xE0) {
					dst[x] = color;
				}
			} else {
				--code;
			}
		}
		dst += 640;
	}
}

void Graphics::decodeRLE_248(uint8 *dst, const uint8 *src, int w, int h, int y1, int y2, bool xflip, bool color248Only) {
	int code = 0;
	int color = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const int offset = xflip ? (w - 1 - x) : x;
			if (code == 0) {
				color = *src++;
				if (color == 0) {
					code = *src++;
				}
			}
			if (color != 0) {
				if ((color248Only || dst[offset] < 0xE0 || y + y1 < y2) && dst[offset] < 0xF8) {
					dst[offset] = color;
				}
			} else {
				--code;
			}
		}
		dst += 640;
	}
}

void Graphics::decodeRLE_320(uint8 *dst, const uint8 *src, int w, int h) {
	int code = 0;
	int color = 0;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			if (code == 0) {
				color = *src++;
				if (color == 0) {
					code = *src++;
				}
			}
			if (code == 0) {
				dst[x] = color;
			} else {
				--code;
			}
		}
		dst += 320;
	}
}

void Graphics::copyRect(uint8 *dst, int dstPitch, uint8 *src, int srcPitch, int w, int h) {
	for (int y = 0; y < h; ++y) {
		memcpy(dst, src, w);
		dst += dstPitch;
		src += srcPitch;
	}
}

void Graphics::drawStringChar(uint8 *dst, int xDst, int yDst, int pitch, uint8 chr, uint8 chrColor, const uint8 *src) {
	if (chr < 32 || chr - 32 >= _charset.xCount * _charset.yCount) {
		return;
	}
	const int h = MIN(_charset.charH, 200 - yDst);
	const int w = MIN(_charset.charW, pitch - xDst);
	dst += yDst * pitch + xDst;
	int offset = (chr - 32) * _charset.charH * _charset.charW;
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			const int color = src[offset++];
			if (color != 0) {
				if (_charsetType == kCharsetTypeCredits) {
					dst[x] = color;
				} else {
					dst[x] = (color == 128) ? color : chrColor;
				}
			}
		}
		dst += pitch;
	}
}

void Graphics::setCharset(CharsetType type) {
	_charsetType = type;
	switch (type) {
	case kCharsetTypeDefault:
		_charset.charW = 10;
		_charset.charH = 10;
		_charset.xCount = 32;
		_charset.yCount = 7;
		break;
	case kCharsetTypeEng:
		_charset.charW = 10;
		_charset.charH = 8;
		_charset.xCount = 32;
		_charset.yCount = 3;
		break;
	case kCharsetTypeCredits:
		_charset.charW = 19;
		_charset.charH = 10;
		_charset.xCount = 16;
		_charset.yCount = 7;
		break;
	}
}

Charset Graphics::_charset;

CharsetType Graphics::_charsetType;

} // namespace Tucker
