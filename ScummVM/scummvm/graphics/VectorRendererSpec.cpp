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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/util.h"
#include "common/system.h"
#include "common/frac.h"

#include "graphics/surface.h"
#include "graphics/colormasks.h"

#include "gui/ThemeEngine.h"
#include "graphics/VectorRenderer.h"
#include "graphics/VectorRendererSpec.h"

#define VECTOR_RENDERER_FAST_TRIANGLES

/** Fixed point SQUARE ROOT **/
inline frac_t fp_sqroot(uint32 x) {
#if 0
	// Use the FPU to compute the square root and then convert it to fixed
	// point data. On systems with a fast FPU, this can be a lot faster than
	// the integer based code below - on my system roughly 50x! However, on
	// systems without an FPU, the converse might be true.
	// For now, we only use the integer based code.
	return doubleToFrac(sqrt((double)x));
#else
	// The code below wants to use a lot of registers, which is not good on
	// x86 processors. By taking advantage of the fact the the input value is
	// an integer, it might be possible to improve this. Furthermore, we could
	// take advantage of the fact that we call this function several times on
	// decreasing values. By feeding it the sqrt of the previous old x, as well
	// as the old x, it should be possible to compute the correct sqrt with far
	// fewer than 23 iterations.
	register uint32 root, remHI, remLO, testDIV, count;

	root = 0;
	remHI = 0;
	remLO = x << 16;
	count = 23;

	do {
		remHI = (remHI << 2) | (remLO >> 30);
		remLO <<= 2;
		root <<= 1;
		testDIV = (root << 1) + 1;

		if (remHI >= testDIV) {
			remHI -= testDIV;
			root++;
		}
	} while (count--);

	return root;
#endif
}

/*
	HELPER MACROS for Bresenham's circle drawing algorithm
	Note the proper spelling on this header.
*/
#define BE_ALGORITHM() do { \
	if (f >= 0) { \
		y--; \
		ddF_y += 2; \
		f += ddF_y; \
		py -= pitch; \
	} \
	px += pitch; \
	ddF_x += 2; \
	f += ddF_x + 1; \
} while(0)

#define BE_DRAWCIRCLE_TOP(ptr1,ptr2,x,y,px,py) do { \
	*(ptr1 + (y) - (px)) = color; \
	*(ptr1 + (x) - (py)) = color; \
	*(ptr2 - (x) - (py)) = color; \
	*(ptr2 - (y) - (px)) = color; \
} while (0)

#define BE_DRAWCIRCLE_BOTTOM(ptr3,ptr4,x,y,px,py) do { \
	*(ptr3 - (y) + (px)) = color; \
	*(ptr3 - (x) + (py)) = color; \
	*(ptr4 + (x) + (py)) = color; \
	*(ptr4 + (y) + (px)) = color; \
} while (0)

#define BE_DRAWCIRCLE(ptr1,ptr2,ptr3,ptr4,x,y,px,py) do { \
	BE_DRAWCIRCLE_TOP(ptr1,ptr2,x,y,px,py); \
	BE_DRAWCIRCLE_BOTTOM(ptr3,ptr4,x,y,px,py); \
} while (0)

#define BE_DRAWCIRCLE_BCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py) do { \
	*(ptr1 + (y) - (px)) = color1; \
	*(ptr1 + (x) - (py)) = color1; \
	*(ptr2 - (x) - (py)) = color1; \
	*(ptr2 - (y) - (px)) = color1; \
	*(ptr3 - (y) + (px)) = color1; \
	*(ptr3 - (x) + (py)) = color1; \
	*(ptr4 + (x) + (py)) = color2; \
	*(ptr4 + (y) + (px)) = color2; \
} while (0)

#define BE_DRAWCIRCLE_XCOLOR_TOP(ptr1,ptr2,x,y,px,py) do { \
	*(ptr1 + (y) - (px)) = color1; \
	*(ptr1 + (x) - (py)) = color2; \
	*(ptr2 - (x) - (py)) = color2; \
	*(ptr2 - (y) - (px)) = color1; \
} while (0)

#define BE_DRAWCIRCLE_XCOLOR_BOTTOM(ptr3,ptr4,x,y,px,py) do { \
	*(ptr3 - (y) + (px)) = color3; \
	*(ptr3 - (x) + (py)) = color4; \
	*(ptr4 + (x) + (py)) = color4; \
	*(ptr4 + (y) + (px)) = color3; \
} while (0)

#define BE_DRAWCIRCLE_XCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py) do { \
	BE_DRAWCIRCLE_XCOLOR_TOP(ptr1,ptr2,x,y,px,py); \
	BE_DRAWCIRCLE_XCOLOR_BOTTOM(ptr3,ptr4,x,y,px,py); \
} while (0)


#define BE_RESET() do { \
	f = 1 - r; \
	ddF_x = 0; ddF_y = -2 * r; \
	x = 0; y = r; px = 0; py = pitch * r; \
} while (0)

#define TRIANGLE_MAINX() \
		if (error_term >= 0) { \
			ptr_right += pitch; \
			ptr_left += pitch; \
			error_term += dysub; \
		} else { \
			error_term += ddy; \
		} \
		ptr_right++; \
		ptr_left--;

#define TRIANGLE_MAINY() \
		if (error_term >= 0) { \
			ptr_right++; \
			ptr_left--; \
			error_term += dxsub; \
		} else { \
			error_term += ddx; \
		} \
		ptr_right += pitch; \
		ptr_left += pitch;

/** HELPER MACROS for WU's circle drawing algorithm **/
#define WU_DRAWCIRCLE_TOP(ptr1,ptr2,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr1 + (y) - (px), color, a); \
	this->blendPixelPtr(ptr1 + (x) - (py), color, a); \
	this->blendPixelPtr(ptr2 - (x) - (py), color, a); \
	this->blendPixelPtr(ptr2 - (y) - (px), color, a); \
} while (0)

#define WU_DRAWCIRCLE_BOTTOM(ptr3,ptr4,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr3 - (y) + (px), color, a); \
	this->blendPixelPtr(ptr3 - (x) + (py), color, a); \
	this->blendPixelPtr(ptr4 + (x) + (py), color, a); \
	this->blendPixelPtr(ptr4 + (y) + (px), color, a); \
} while (0)

#define WU_DRAWCIRCLE(ptr1,ptr2,ptr3,ptr4,x,y,px,py,a) do { \
	WU_DRAWCIRCLE_TOP(ptr1,ptr2,x,y,px,py,a); \
	WU_DRAWCIRCLE_BOTTOM(ptr3,ptr4,x,y,px,py,a); \
} while (0)


// Color depending on y
// Note: this is only for the outer pixels
#define WU_DRAWCIRCLE_XCOLOR_TOP(ptr1,ptr2,x,y,px,py,a,func) do { \
	this->func(ptr1 + (y) - (px), color1, a); \
	this->func(ptr1 + (x) - (py), color2, a); \
	this->func(ptr2 - (x) - (py), color2, a); \
	this->func(ptr2 - (y) - (px), color1, a); \
} while (0)

#define WU_DRAWCIRCLE_XCOLOR_BOTTOM(ptr3,ptr4,x,y,px,py,a,func) do { \
	this->func(ptr3 - (y) + (px), color3, a); \
	this->func(ptr3 - (x) + (py), color4, a); \
	this->func(ptr4 + (x) + (py), color4, a); \
	this->func(ptr4 + (y) + (px), color3, a); \
} while (0)

#define WU_DRAWCIRCLE_XCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py,a,func) do { \
	WU_DRAWCIRCLE_XCOLOR_TOP(ptr1,ptr2,x,y,px,py,a,func); \
	WU_DRAWCIRCLE_XCOLOR_BOTTOM(ptr3,ptr4,x,y,px,py,a,func); \
} while (0)

// Color depending on corner (tl,tr,bl: color1, br: color2)
// Note: this is only for the outer pixels
#define WU_DRAWCIRCLE_BCOLOR(ptr1,ptr2,ptr3,ptr4,x,y,px,py,a) do { \
	this->blendPixelPtr(ptr1 + (y) - (px), color1, a); \
	this->blendPixelPtr(ptr1 + (x) - (py), color1, a); \
	this->blendPixelPtr(ptr2 - (x) - (py), color1, a); \
	this->blendPixelPtr(ptr2 - (y) - (px), color1, a); \
	this->blendPixelPtr(ptr3 - (y) + (px), color1, a); \
	this->blendPixelPtr(ptr3 - (x) + (py), color1, a); \
	this->blendPixelPtr(ptr4 + (x) + (py), color2, a); \
	this->blendPixelPtr(ptr4 + (y) + (px), color2, a); \
} while (0)


// optimized Wu's algorithm
#define WU_ALGORITHM() do { \
	oldT = T; \
	T = fp_sqroot(rsq - y*y) ^ 0xFFFF; \
	py += pitch; \
	if (T < oldT) { \
		x--; px -= pitch; \
	} \
	a2 = (T >> 8); \
	a1 = ~a2;   \
} while (0)


namespace Graphics {

/**
 * Fills several pixels in a row with a given color.
 *
 * This is a replacement function for Common::fill, using an unrolled
 * loop to maximize performance on most architectures.
 * This function may (and should) be overloaded in any child renderers
 * for portable platforms with platform-specific assembly code.
 *
 * This fill operation is extensively used throughout the renderer, so this
 * counts as one of the main bottlenecks. Please replace it with assembly
 * when possible!
 *
 * @param first Pointer to the first pixel to fill.
 * @param last Pointer to the last pixel to fill.
 * @param color Color of the pixel
 */
template<typename PixelType>
void colorFill(PixelType *first, PixelType *last, PixelType color) {
	register int count = (last - first);
	if (!count)
		return;
	register int n = (count + 7) >> 3;
	switch (count % 8) {
	case 0: do {
				*first++ = color;
	case 7:		*first++ = color;
	case 6:		*first++ = color;
	case 5:		*first++ = color;
	case 4:		*first++ = color;
	case 3:		*first++ = color;
	case 2:		*first++ = color;
	case 1:		*first++ = color;
			} while (--n > 0);
	}
}


VectorRenderer *createRenderer(int mode) {
#ifdef DISABLE_FANCY_THEMES
	assert(mode == GUI::ThemeEngine::kGfxStandard16bit);
#endif

	PixelFormat format = g_system->getOverlayFormat();
	switch (mode) {
	case GUI::ThemeEngine::kGfxStandard16bit:
		return new VectorRendererSpec<OverlayColor>(format);
#ifndef DISABLE_FANCY_THEMES
	case GUI::ThemeEngine::kGfxAntialias16bit:
		return new VectorRendererAA<OverlayColor>(format);
#endif
	default:
		break;
	}

	return 0;
}

template<typename PixelType>
VectorRendererSpec<PixelType>::
VectorRendererSpec(PixelFormat format) :
	_format(format),
	_redMask((0xFF >> format.rLoss) << format.rShift),
	_greenMask((0xFF >> format.gLoss) << format.gShift),
	_blueMask((0xFF >> format.bLoss) << format.bShift),
	_alphaMask((0xFF >> format.aLoss) << format.aShift) {

	_bitmapAlphaColor = _format.RGBToColor(255, 0, 255);
}

/****************************
 * Gradient-related methods *
 ****************************/

template<typename PixelType>
void VectorRendererSpec<PixelType>::
setGradientColors(uint8 r1, uint8 g1, uint8 b1, uint8 r2, uint8 g2, uint8 b2) {
	_gradientEnd = _format.RGBToColor(r2, g2, b2);
	_gradientStart = _format.RGBToColor(r1, g1, b1);

	_gradientBytes[0] = (_gradientEnd & _redMask) - (_gradientStart & _redMask);
	_gradientBytes[1] = (_gradientEnd & _greenMask) - (_gradientStart & _greenMask);
	_gradientBytes[2] = (_gradientEnd & _blueMask) - (_gradientStart & _blueMask);
}

template<typename PixelType>
inline PixelType VectorRendererSpec<PixelType>::
calcGradient(uint32 pos, uint32 max) {
	PixelType output = 0;
	pos = (MIN(pos * Base::_gradientFactor, max) << 12) / max;

	output |= ((_gradientStart & _redMask) + ((_gradientBytes[0] * pos) >> 12)) & _redMask;
	output |= ((_gradientStart & _greenMask) + ((_gradientBytes[1] * pos) >> 12)) & _greenMask;
	output |= ((_gradientStart & _blueMask) + ((_gradientBytes[2] * pos) >> 12)) & _blueMask;
	output |= _alphaMask;

	return output;
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
precalcGradient(int h) {
	PixelType prevcolor = 0, color;

	_gradCache.resize(0);
	_gradIndexes.resize(0);

	for (int i = 0; i < h + 2; i++) {
		color = calcGradient(i, h);
		if (color != prevcolor || i == 0 || i > h - 1) {
			prevcolor = color;
			_gradCache.push_back(color);
			_gradIndexes.push_back(i);
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
gradientFill(PixelType *ptr, int width, int x, int y) {
	bool ox = ((y & 1) == 1);
	int stripSize;
	int curGrad = 0;

	while (_gradIndexes[curGrad + 1] <= y)
		curGrad++;

	stripSize = _gradIndexes[curGrad + 1] - _gradIndexes[curGrad];

	int grad = (((y - _gradIndexes[curGrad]) % stripSize) << 2) / stripSize;

	// Dithering:
	//   +--+ +--+ +--+ +--+
	//   |  | |  | | *| | *|
	//   |  | | *| |* | |**|
	//   +--+ +--+ +--+ +--+
	//     0    1    2    3
	if (grad == 0 ||
		_gradCache[curGrad] == _gradCache[curGrad + 1] || // no color change
		stripSize < 2) { // the stip is small
		colorFill<PixelType>(ptr, ptr + width, _gradCache[curGrad]);
	} else if (grad == 3 && ox) {
		colorFill<PixelType>(ptr, ptr + width, _gradCache[curGrad + 1]);
	} else {
		for (int j = x; j < x + width; j++, ptr++) {
			bool oy = ((j & 1) == 1);

			if ((ox && oy) ||
				((grad == 2 || grad == 3) && ox && !oy) ||
				(grad == 3 && oy))
				*ptr = _gradCache[curGrad + 1];
			else
				*ptr = _gradCache[curGrad];
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
fillSurface() {
	byte *ptr = (byte *)_activeSurface->getBasePtr(0, 0);

	int h = _activeSurface->h;
	int pitch = _activeSurface->pitch;

	if (Base::_fillMode == kFillBackground) {
		colorFill<PixelType>((PixelType *)ptr, (PixelType *)(ptr + pitch * h), _bgColor);
	} else if (Base::_fillMode == kFillForeground) {
		colorFill<PixelType>((PixelType *)ptr, (PixelType *)(ptr + pitch * h), _fgColor);
	} else if (Base::_fillMode == kFillGradient) {
		precalcGradient(h);

		for (int i = 0; i < h; i++) {
			gradientFill((PixelType *)ptr, _activeSurface->w, 0, i);

			ptr += pitch;
		}
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
copyFrame(OSystem *sys, const Common::Rect &r) {

	sys->copyRectToOverlay(
		_activeSurface->getBasePtr(r.left, r.top),
		_activeSurface->pitch,
	    r.left, r.top, r.width(), r.height()
	);
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
blitSurface(const Graphics::Surface *source, const Common::Rect &r) {
	assert(source->w == _activeSurface->w && source->h == _activeSurface->h);

	byte *dst_ptr = (byte *)_activeSurface->getBasePtr(r.left, r.top);
	const byte *src_ptr = (const byte *)source->getBasePtr(r.left, r.top);

	const int dst_pitch = _activeSurface->pitch;
	const int src_pitch = source->pitch;

	int h = r.height();
	const int w = r.width() * sizeof(PixelType);

	while (h--) {
		memcpy(dst_ptr, src_ptr, w);
		dst_ptr += dst_pitch;
		src_ptr += src_pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
blitSubSurface(const Graphics::Surface *source, const Common::Rect &r) {
	byte *dst_ptr = (byte *)_activeSurface->getBasePtr(r.left, r.top);
	const byte *src_ptr = (const byte *)source->getBasePtr(0, 0);

	const int dst_pitch = _activeSurface->pitch;
	const int src_pitch = source->pitch;

	int h = r.height();
	const int w = r.width() * sizeof(PixelType);

	while (h--) {
		memcpy(dst_ptr, src_ptr, w);
		dst_ptr += dst_pitch;
		src_ptr += src_pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
blitAlphaBitmap(const Graphics::Surface *source, const Common::Rect &r) {
	int16 x = r.left;
	int16 y = r.top;

	if (r.width() > source->w)
		x = x + (r.width() >> 1) - (source->w >> 1);

	if (r.height() > source->h)
		y = y + (r.height() >> 1) - (source->h >> 1);

	PixelType *dst_ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
	const PixelType *src_ptr = (const PixelType *)source->getBasePtr(0, 0);

	int dst_pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int src_pitch = source->pitch / source->format.bytesPerPixel;

	int w, h = source->h;

	while (h--) {
		w = source->w;

		while (w--) {
			if (*src_ptr != _bitmapAlphaColor)
				*dst_ptr = *src_ptr;

			dst_ptr++;
			src_ptr++;
		}

		dst_ptr = dst_ptr - source->w + dst_pitch;
		src_ptr = src_ptr - source->w + src_pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
applyScreenShading(GUI::ThemeEngine::ShadingStyle shadingStyle) {
	int pixels = _activeSurface->w * _activeSurface->h;
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(0, 0);
	uint8 r, g, b;
	uint lum;

	// Mask to clear the last bit of every color component and all unused bits
	const uint32 colorMask = ~((1 << _format.rShift) | (1 << _format.gShift) | (1 << _format.bShift) // R/G/B components
			| (_format.aLoss == 8 ? 0 : (1 << _format.aShift)) // Alpha component
			| ~(_alphaMask | _redMask | _greenMask | _blueMask)); // All unused bits

	if (shadingStyle == GUI::ThemeEngine::kShadingDim) {

		// TODO: Check how this interacts with kFeatureOverlaySupportsAlpha
		for (int i = 0; i < pixels; ++i) {
			*ptr = ((*ptr & colorMask) >> 1) | _alphaMask;
			++ptr;
		}

	} else if (shadingStyle == GUI::ThemeEngine::kShadingLuminance) {
		while (pixels--) {
			_format.colorToRGB(*ptr, r, g, b);
			lum = (r >> 2) + (g >> 1) + (b >> 3);
			*ptr++ = _format.RGBToColor(lum, lum, lum);
		}
	}
}

template<typename PixelType>
inline void VectorRendererSpec<PixelType>::
blendPixelPtr(PixelType *ptr, PixelType color, uint8 alpha) {
	int idst = *ptr;
	int isrc = color;

	*ptr = (PixelType)(
		(_redMask & ((idst & _redMask) +
		((int)(((int)(isrc & _redMask) -
		(int)(idst & _redMask)) * alpha) >> 8))) |
		(_greenMask & ((idst & _greenMask) +
		((int)(((int)(isrc & _greenMask) -
		(int)(idst & _greenMask)) * alpha) >> 8))) |
		(_blueMask & ((idst & _blueMask) +
		((int)(((int)(isrc & _blueMask) -
		(int)(idst & _blueMask)) * alpha) >> 8))) |
		(idst & _alphaMask));
}

template<typename PixelType>
inline void VectorRendererSpec<PixelType>::
blendPixelDestAlphaPtr(PixelType *ptr, PixelType color, uint8 alpha) {
	int idst = *ptr;
	// This function is only used for corner pixels in rounded rectangles, so
	// the performance hit of this if shouldn't be too high.
	// We're also ignoring the cases where dst has intermediate alpha.
	if ((idst & _alphaMask) == 0) {
		// set color and alpha channels
		*ptr = (PixelType)(color & (_redMask | _greenMask | _blueMask)) |
		                  ((alpha >> _format.aLoss) << _format.aShift);
	} else {
		// blend color with background
		blendPixelPtr(ptr, color, alpha);
	}
}

template<typename PixelType>
inline void VectorRendererSpec<PixelType>::
darkenFill(PixelType *ptr, PixelType *end) {
	PixelType mask = (PixelType)((3 << _format.rShift) | (3 << _format.gShift) | (3 << _format.bShift));

	if (!g_system->hasFeature(OSystem::kFeatureOverlaySupportsAlpha)) {
		// !kFeatureOverlaySupportsAlpha (but might have alpha bits)

		while (ptr != end) {
			*ptr = ((*ptr & ~mask) >> 2) | _alphaMask;
			++ptr;
		}
	} else {
		// kFeatureOverlaySupportsAlpha
		// assuming at least 3 alpha bits

		mask |= 3 << _format.aShift;
		PixelType addA = (PixelType)(255 >> _format.aLoss) << _format.aShift;
		addA -= (addA >> 2);

		while (ptr != end) {
			// Darken the colour, and increase the alpha
			// (0% -> 75%, 100% -> 100%)
			*ptr = (PixelType)(((*ptr & ~mask) >> 2) + addA);
			++ptr;
		}
	}
}

/********************************************************************
 ********************************************************************
 * Primitive shapes drawing - Public API calls - VectorRendererSpec *
 ********************************************************************
 ********************************************************************/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawString(const Graphics::Font *font, const Common::String &text, const Common::Rect &area,
			Graphics::TextAlign alignH, GUI::ThemeEngine::TextAlignVertical alignV, int deltax, bool ellipsis) {

	int offset = area.top;

	if (font->getFontHeight() < area.height()) {
		switch (alignV) {
			case GUI::ThemeEngine::kTextAlignVCenter:
				offset = area.top + ((area.height() - font->getFontHeight()) >> 1);
				break;
			case GUI::ThemeEngine::kTextAlignVBottom:
				offset = area.bottom - font->getFontHeight();
				break;
			default:
				break;
		}
	}

	font->drawString(_activeSurface, text, area.left, offset, area.width() - deltax, _fgColor, alignH, deltax, ellipsis);
}

/** LINES **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawLine(int x1, int y1, int x2, int y2) {
	x1 = CLIP(x1, 0, (int)Base::_activeSurface->w);
	x2 = CLIP(x2, 0, (int)Base::_activeSurface->w);
	y1 = CLIP(y1, 0, (int)Base::_activeSurface->h);
	y2 = CLIP(y2, 0, (int)Base::_activeSurface->h);

	// we draw from top to bottom
	if (y2 < y1) {
		SWAP(x1, x2);
		SWAP(y1, y2);
	}

	int dx = ABS(x2 - x1);
	int dy = ABS(y2 - y1);

	// this is a point, not a line. stoopid.
	if (dy == 0 && dx == 0)
		return;

	if (Base::_strokeWidth == 0)
		return;

	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int st = Base::_strokeWidth >> 1;

	if (dy == 0) { // horizontal lines
		// these can be filled really fast with a single memset.
		colorFill<PixelType>(ptr, ptr + dx + 1, (PixelType)_fgColor);

		for (int i = 0, p = pitch; i < st; ++i, p += pitch) {
			colorFill<PixelType>(ptr + p, ptr + dx + 1 + p, (PixelType)_fgColor);
			colorFill<PixelType>(ptr - p, ptr + dx + 1 - p, (PixelType)_fgColor);
		}

	} else if (dx == 0) { // vertical lines
		// these ones use a static pitch increase.
		while (y1++ <= y2) {
			colorFill<PixelType>(ptr - st, ptr + st, (PixelType)_fgColor);
			ptr += pitch;
		}

	} else if (ABS(dx) == ABS(dy)) { // diagonal lines
		// these ones also use a fixed pitch increase
		pitch += (x2 > x1) ? 1 : -1;

		while (dy--) {
			colorFill<PixelType>(ptr - st, ptr + st, (PixelType)_fgColor);
			ptr += pitch;
		}

	} else { // generic lines, use the standard algorithm...
		drawLineAlg(x1, y1, x2, y2, dx, dy, (PixelType)_fgColor);
	}
}

/** CIRCLES **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawCircle(int x, int y, int r) {
	if (x + r > Base::_activeSurface->w || y + r > Base::_activeSurface->h ||
		x - r < 0 || y - r < 0 || x == 0 || y == 0 || r <= 0)
		return;

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset
		&& x + r + Base::_shadowOffset < Base::_activeSurface->w
		&& y + r + Base::_shadowOffset < Base::_activeSurface->h) {
		drawCircleAlg(x + Base::_shadowOffset + 1, y + Base::_shadowOffset + 1, r, 0, kFillForeground);
	}

	switch (Base::_fillMode) {
	case kFillDisabled:
		if (Base::_strokeWidth)
			drawCircleAlg(x, y, r, _fgColor, kFillDisabled);
		break;

	case kFillForeground:
		drawCircleAlg(x, y, r, _fgColor, kFillForeground);
		break;

	case kFillBackground:
		if (Base::_strokeWidth > 1) {
			drawCircleAlg(x, y, r, _fgColor, kFillForeground);
			drawCircleAlg(x, y, r - Base::_strokeWidth, _bgColor, kFillBackground);
		} else {
			drawCircleAlg(x, y, r, _bgColor, kFillBackground);
			drawCircleAlg(x, y, r, _fgColor, kFillDisabled);
		}
		break;

	case kFillGradient:
		break;
	}
}

/** SQUARES **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawSquare(int x, int y, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0)
		return;

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset
		&& x + w + Base::_shadowOffset < Base::_activeSurface->w
		&& y + h + Base::_shadowOffset < Base::_activeSurface->h) {
		drawSquareShadow(x, y, w, h, Base::_shadowOffset);
	}

	switch (Base::_fillMode) {
	case kFillDisabled:
		if (Base::_strokeWidth)
			drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		break;

	case kFillForeground:
		drawSquareAlg(x, y, w, h, _fgColor, kFillForeground);
		break;

	case kFillBackground:
		drawSquareAlg(x, y, w, h, _bgColor, kFillBackground);
		drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		break;

	case kFillGradient:
		VectorRendererSpec::drawSquareAlg(x, y, w, h, 0, kFillGradient);
		if (Base::_strokeWidth)
			drawSquareAlg(x, y, w, h, _fgColor, kFillDisabled);
		break;
	}
}

/** ROUNDED SQUARES **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawRoundedSquare(int x, int y, int r, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0 || r <= 0)
		return;

	if ((r * 2) > w || (r * 2) > h)
		r = MIN(w /2, h / 2);

	if (r <= 0)
		return;

	if (Base::_fillMode != kFillDisabled && Base::_shadowOffset
		&& x + w + Base::_shadowOffset + 1 < Base::_activeSurface->w
		&& y + h + Base::_shadowOffset + 1 < Base::_activeSurface->h) {
		drawRoundedSquareShadow(x, y, r, w, h, Base::_shadowOffset);
	}

	drawRoundedSquareAlg(x, y, r, w, h, _fgColor, Base::_fillMode);
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTab(int x, int y, int r, int w, int h) {
	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h ||
		w <= 0 || h <= 0 || x < 0 || y < 0 || r > w || r > h)
		return;

	if (r == 0 && Base::_bevel > 0) {
		drawBevelTabAlg(x, y, w, h, Base::_bevel, _bevelColor, _fgColor, (Base::_dynamicData >> 16), (Base::_dynamicData & 0xFFFF));
		return;
	}

	if (r == 0) return;

	switch (Base::_fillMode) {
		case kFillDisabled:
			// FIXME: Implement this
			return;

		case kFillGradient:
		case kFillBackground:
			// FIXME: This is broken for the AA renderer.
			// See the rounded rect alg for how to fix it. (The border should
			// be drawn before the interior, both inside drawTabAlg.)
			drawTabAlg(x, y, w, h, r, (Base::_fillMode == kFillBackground) ? _bgColor : _fgColor, Base::_fillMode);
			if (Base::_strokeWidth)
				drawTabAlg(x, y, w, h, r, _fgColor, kFillDisabled, (Base::_dynamicData >> 16), (Base::_dynamicData & 0xFFFF));
			break;

		case kFillForeground:
			drawTabAlg(x, y, w, h, r, (Base::_fillMode == kFillBackground) ? _bgColor : _fgColor, Base::_fillMode);
			break;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTriangle(int x, int y, int w, int h, TriangleOrientation orient) {

	if (x + w > Base::_activeSurface->w || y + h > Base::_activeSurface->h)
		return;

	PixelType color = 0;

	if (Base::_strokeWidth <= 1) {
		if (Base::_fillMode == kFillForeground)
			color = _fgColor;
		else if (Base::_fillMode == kFillBackground)
			color = _bgColor;
	} else {
		if (Base::_fillMode == kFillDisabled)
			return;
		color = _fgColor;
	}

	if (Base::_dynamicData != 0)
		orient = (TriangleOrientation)Base::_dynamicData;

	if (w == h) {
		int newW = w;

		switch (orient) {
		case kTriangleUp:
		case kTriangleDown:
			//drawTriangleFast(x, y, newW, (orient == kTriangleDown), color, Base::_fillMode);
			drawTriangleVertAlg(x, y, newW, newW, (orient == kTriangleDown), color, Base::_fillMode);
			break;

		case kTriangleLeft:
		case kTriangleRight:
		case kTriangleAuto:
			break;
		}

		if (Base::_strokeWidth > 0)
			if (Base::_fillMode == kFillBackground || Base::_fillMode == kFillGradient) {
				//drawTriangleFast(x, y, newW, (orient == kTriangleDown), _fgColor, kFillDisabled);
				drawTriangleVertAlg(x, y, newW, newW, (orient == kTriangleDown), color, Base::_fillMode);
			}
	} else {
		int newW = w;
		int newH = h;

		switch (orient) {
		case kTriangleUp:
		case kTriangleDown:
			drawTriangleVertAlg(x, y, newW, newH, (orient == kTriangleDown), color, Base::_fillMode);
			break;

		case kTriangleLeft:
		case kTriangleRight:
		case kTriangleAuto:
			break;
		}

		if (Base::_strokeWidth > 0) {
			if (Base::_fillMode == kFillBackground || Base::_fillMode == kFillGradient) {
				drawTriangleVertAlg(x, y, newW, newH, (orient == kTriangleDown), _fgColor, kFillDisabled);
			}
		}
	}
}





/********************************************************************
 ********************************************************************
 * Aliased Primitive drawing ALGORITHMS - VectorRendererSpec
 ********************************************************************
 ********************************************************************/
/** TAB ALGORITHM - NON AA */
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTabAlg(int x1, int y1, int w, int h, int r, PixelType color, VectorRenderer::FillMode fill_m, int baseLeft, int baseRight) {
	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int sw  = 0, sp = 0, hp = 0;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;
	int short_h = h - r + 2;
	int long_h = h;

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			colorFill<PixelType>(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			colorFill<PixelType>(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
			sp += pitch;

			BE_RESET();
			r--;

			while (x++ < y) {
				BE_ALGORITHM();
				BE_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px, py);

				if (Base::_strokeWidth > 1)
					BE_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px - pitch, py);
			}
		}

		ptr_fill += pitch * real_radius;
		while (short_h--) {
			colorFill<PixelType>(ptr_fill, ptr_fill + Base::_strokeWidth, color);
			colorFill<PixelType>(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}

		if (baseLeft) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1 + h + 1);
			while (sw++ < Base::_strokeWidth) {
				colorFill<PixelType>(ptr_fill - baseLeft, ptr_fill, color);
				ptr_fill += pitch;
			}
		}

		if (baseRight) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w, y1 + h + 1);
			while (sw++ < Base::_strokeWidth) {
				colorFill<PixelType>(ptr_fill, ptr_fill + baseRight, color);
				ptr_fill += pitch;
			}
		}
	} else {
		BE_RESET();

		precalcGradient(long_h);

		PixelType color1, color2;
		color1 = color2 = color;

		while (x++ < y) {
			BE_ALGORITHM();

			if (fill_m == kFillGradient) {
				color1 = calcGradient(real_radius - x, long_h);
				color2 = calcGradient(real_radius - y, long_h);

				gradientFill(ptr_tl - x - py, w - 2 * r + 2 * x, x1 + r - x - y, real_radius - y);
				gradientFill(ptr_tl - y - px, w - 2 * r + 2 * y, x1 + r - y - x, real_radius - x);

				BE_DRAWCIRCLE_XCOLOR_TOP(ptr_tr, ptr_tl, x, y, px, py);
			} else {
				colorFill<PixelType>(ptr_tl - x - py, ptr_tr + x - py, color);
				colorFill<PixelType>(ptr_tl - y - px, ptr_tr + y - px, color);

				BE_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px, py);
			}
		}

		ptr_fill += pitch * r;
		while (short_h--) {
			if (fill_m == kFillGradient) {
				gradientFill(ptr_fill, w + 1, x1, real_radius++);
			} else {
				colorFill<PixelType>(ptr_fill, ptr_fill + w + 1, color);
			}
			ptr_fill += pitch;
		}
	}
}


/** BEVELED TABS FOR CLASSIC THEME **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawBevelTabAlg(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color, int baseLeft, int baseRight) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int i, j;

	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);

	i = bevel;
	while (i--) {
		colorFill<PixelType>(ptr_left, ptr_left + w, top_color);
		ptr_left += pitch;
	}

	if (baseLeft > 0) {
		i = h - bevel;
		ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);
		while (i--) {
			colorFill<PixelType>(ptr_left, ptr_left + bevel, top_color);
			ptr_left += pitch;
		}
	}

	i = h - bevel;
	j = bevel - 1;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y);
	while (i--) {
		colorFill<PixelType>(ptr_left + j, ptr_left + bevel, bottom_color);
		if (j > 0) j--;
		ptr_left += pitch;
	}

	i = bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y + h - bevel);
	while (i--) {
		colorFill<PixelType>(ptr_left, ptr_left + baseRight + bevel, bottom_color);

		if (baseLeft)
			colorFill<PixelType>(ptr_left - w - baseLeft + bevel, ptr_left - w + bevel + bevel, top_color);
		ptr_left += pitch;
	}
}

/** SQUARE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawSquareAlg(int x, int y, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x, y);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int max_h = h;

	if (fill_m != kFillDisabled) {
		while (h--) {
			if (fill_m == kFillGradient)
				color = calcGradient(max_h - h, max_h);

			colorFill<PixelType>(ptr, ptr + w, color);
			ptr += pitch;
		}
	} else {
		int sw = Base::_strokeWidth, sp = 0, hp = pitch * (h - 1);

		while (sw--) {
			colorFill<PixelType>(ptr + sp, ptr + w + sp, color);
			colorFill<PixelType>(ptr + hp - sp, ptr + w + hp - sp, color);
			sp += pitch;
		}

		while (h--) {
			colorFill<PixelType>(ptr, ptr + Base::_strokeWidth, color);
			colorFill<PixelType>(ptr + w - Base::_strokeWidth, ptr + w, color);
			ptr += pitch;
		}
	}
}

/** SQUARE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawBevelSquareAlg(int x, int y, int w, int h, int bevel, PixelType top_color, PixelType bottom_color, bool fill) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	int height = h;
	PixelType *ptr_fill = (PixelType *)_activeSurface->getBasePtr(x, y);

	if (fill) {
		assert((_bgColor & ~_alphaMask) == 0); // only support black
		while (height--) {
			darkenFill(ptr_fill, ptr_fill + w);
			ptr_fill += pitch;
		}
	}

	int i, j;

	x = MAX(x - bevel, 0);
	y = MAX(y - bevel, 0);

	w = MIN(w + (bevel * 2), (int)_activeSurface->w);
	h = MIN(h + (bevel * 2), (int)_activeSurface->h);

	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y);

	i = bevel;
	while (i--) {
		colorFill<PixelType>(ptr_left, ptr_left + w, top_color);
		ptr_left += pitch;
	}

	i = h - bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y + bevel);
	while (i--) {
		colorFill<PixelType>(ptr_left, ptr_left + bevel, top_color);
		ptr_left += pitch;
	}

	i = bevel;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x, y + h - bevel);
	while (i--) {
		colorFill<PixelType>(ptr_left + i, ptr_left + w, bottom_color);
		ptr_left += pitch;
	}

	i = h - bevel;
	j = bevel - 1;
	ptr_left = (PixelType *)_activeSurface->getBasePtr(x + w - bevel, y);
	while (i--) {
		colorFill<PixelType>(ptr_left + j, ptr_left + bevel, bottom_color);
		if (j > 0) j--;
		ptr_left += pitch;
	}
}

/** GENERIC LINE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy, PixelType color) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int xdir = (x2 > x1) ? 1 : -1;

	*ptr = (PixelType)color;

	if (dx > dy) {
		int ddy = dy * 2;
		int dysub = ddy - (dx * 2);
		int error_term = ddy - dx;

		while (dx--) {
			if (error_term >= 0) {
				ptr += pitch;
				error_term += dysub;
			} else {
				error_term += ddy;
			}

			ptr += xdir;
			*ptr = (PixelType)color;
		}
	} else {
		int ddx = dx * 2;
		int dxsub = ddx - (dy * 2);
		int error_term = ddx - dy;

		while (dy--) {
			if (error_term >= 0) {
				ptr += xdir;
				error_term += dxsub;
			} else {
				error_term += ddx;
			}

			ptr += pitch;
			*ptr = (PixelType)color;
		}
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x2, y2);
	*ptr = (PixelType)color;
}

/** VERTICAL TRIANGLE DRAWING ALGORITHM **/
/**
	FIXED POINT ARITHMETIC
**/

#define FIXED_POINT 1

#if FIXED_POINT
#define ipart(x) ((x) & ~0xFF)
// This is not really correct since gradient is not percentage, but [0..255]
#define rfpart(x) ((0x100 - ((x) & 0xFF)) * 100 >> 8)
//#define rfpart(x) (0x100 - ((x) & 0xFF))
#else
#define ipart(x) ((int)x)
#define round(x) (ipart(x + 0.5))
#define fpart(x) (x - ipart(x))
#define rfpart(x) (int)((1 - fpart(x)) * 100)
#endif

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTriangleVertAlg(int x1, int y1, int w, int h, bool inverted, PixelType color, VectorRenderer::FillMode fill_m) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int gradient_h = 0;
	if (!inverted) {
		pitch = -pitch;
		y1 += h;
	}

	PixelType *ptr_right = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	PixelType *floor = ptr_right - 1;
	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x1 + w, y1);

	int x2 = x1 + w / 2;
	int y2 = y1 + h;

#if FIXED_POINT
	int dx = (x2 - x1) << 8;
	int dy = (y2 - y1) << 8;

	if (abs(dx) > abs(dy)) {
#else
	double dx = (double)x2 - (double)x1;
	double dy = (double)y2 - (double)y1;

	if (fabs(dx) > fabs(dy)) {
#endif
		while (floor++ != ptr_left)
			blendPixelPtr(floor, color, 50);

#if FIXED_POINT
		int gradient = (dy << 8) / dx;
		int intery = (y1 << 8) + gradient;
#else
		double gradient = dy / dx;
		double intery = y1 + gradient;
#endif

		for (int x = x1 + 1; x < x2; x++) {
#if FIXED_POINT
			if (intery + gradient > ipart(intery) + 0x100) {
#else
			if (intery + gradient > ipart(intery) + 1) {
#endif
				ptr_right++;
				ptr_left--;
			}

			ptr_left += pitch;
			ptr_right += pitch;

			intery += gradient;

			switch (fill_m) {
			case kFillDisabled:
				*ptr_left = *ptr_right = color;
				break;
			case kFillForeground:
			case kFillBackground:
				colorFill<PixelType>(ptr_right + 1, ptr_left, color);
				blendPixelPtr(ptr_right, color, rfpart(intery));
				blendPixelPtr(ptr_left, color, rfpart(intery));
				break;
			case kFillGradient:
				colorFill<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h));
				blendPixelPtr(ptr_right, color, rfpart(intery));
				blendPixelPtr(ptr_left, color, rfpart(intery));
				break;
			}
		}

		return;
	}

#if FIXED_POINT
	if (abs(dx) < abs(dy)) {
#else
	if (fabs(dx) < fabs(dy)) {
#endif
		ptr_left--;
		while (floor++ != ptr_left)
			blendPixelPtr(floor, color, 50);

#if FIXED_POINT
		int gradient = (dx << 8) / (dy + 0x100);
		int interx = (x1 << 8) + gradient;
#else
		double gradient = dx / (dy+1);
		double interx = x1 + gradient;
#endif

		for (int y = y1 + 1; y < y2; y++) {
#if FIXED_POINT
			if (interx + gradient > ipart(interx) + 0x100) {
#else
			if (interx + gradient > ipart(interx) + 1) {
#endif
				ptr_right++;
				ptr_left--;
			}

			ptr_left += pitch;
			ptr_right += pitch;

			interx += gradient;

			switch (fill_m) {
			case kFillDisabled:
				*ptr_left = *ptr_right = color;
				break;
			case kFillForeground:
			case kFillBackground:
				colorFill<PixelType>(ptr_right + 1, ptr_left, color);
				blendPixelPtr(ptr_right, color, rfpart(interx));
				blendPixelPtr(ptr_left, color, rfpart(interx));
				break;
			case kFillGradient:
				colorFill<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h));
				blendPixelPtr(ptr_right, color, rfpart(interx));
				blendPixelPtr(ptr_left, color, rfpart(interx));
				break;
			}
		}

		return;
	}

	ptr_left--;

	while (floor++ != ptr_left)
		blendPixelPtr(floor, color, 50);

#if FIXED_POINT
	int gradient = (dx / dy) << 8;
	int interx = (x1 << 8) + gradient;
#else
	double gradient = dx / dy;
	double interx = x1 + gradient;
#endif

	for (int y = y1 + 1; y < y2; y++) {
		ptr_right++;
		ptr_left--;

		ptr_left += pitch;
		ptr_right += pitch;

		interx += gradient;

		switch (fill_m) {
		case kFillDisabled:
			*ptr_left = *ptr_right = color;
			break;
		case kFillForeground:
		case kFillBackground:
			colorFill<PixelType>(ptr_right + 1, ptr_left, color);
			blendPixelPtr(ptr_right, color, rfpart(interx));
			blendPixelPtr(ptr_left, color, rfpart(interx));
			break;
		case kFillGradient:
			colorFill<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, h));
			blendPixelPtr(ptr_right, color, rfpart(interx));
			blendPixelPtr(ptr_left, color, rfpart(interx));
			break;
		}
	}

}

/** VERTICAL TRIANGLE DRAWING - FAST VERSION FOR SQUARED TRIANGLES */
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawTriangleFast(int x1, int y1, int size, bool inverted, PixelType color, VectorRenderer::FillMode fill_m) {
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	if (!inverted) {
		pitch = -pitch;
		y1 += size;
	}

	int gradient_h = 0;
	PixelType *ptr_right = (PixelType *)_activeSurface->getBasePtr(x1, y1);
	PixelType *ptr_left = (PixelType *)_activeSurface->getBasePtr(x1 + size, y1);
	int x2 = x1 + size / 2;
	int y2 = y1 + size;
	int deltaX = abs(x2 - x1);
	int deltaY = abs(y2 - y1);
	int signX = x1 < x2 ? 1 : -1;
	int signY = y1 < y2 ? 1 : -1;
	int error = deltaX - deltaY;

	colorFill<PixelType>(ptr_right, ptr_left, color);

	while (1) {
		switch (fill_m) {
		case kFillDisabled:
			*ptr_left = *ptr_right = color;
			break;
		case kFillForeground:
		case kFillBackground:
			colorFill<PixelType>(ptr_right, ptr_left, color);
			break;
		case kFillGradient:
			colorFill<PixelType>(ptr_right, ptr_left, calcGradient(gradient_h++, size));
			break;
		}

		if (x1 == x2 && y1 == y2)
			break;

		int error2 = error * 2;

		if (error2 > -deltaY) {
			error -= deltaY;
			x1 += signX;
			ptr_right += signX;
			ptr_left += -signX;
		}

		if (error2 < deltaX) {
			error += deltaX;
			y1 += signY;
			ptr_right += pitch;
			ptr_left += pitch;
		}
	}
}

/** ROUNDED SQUARE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;

	// TODO: Split this up into border, bevel and interior functions

	if (fill_m != kFillDisabled) {
		PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
		PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
		PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
		PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
		PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

		int real_radius = r;
		int short_h = h - (2 * r) + 2;
		int long_h = h;

		BE_RESET();

		PixelType color1 = color;
		if (fill_m == kFillBackground)
			color1 = _bgColor;

		if (fill_m == kFillGradient) {
			PixelType color2, color3, color4;
			precalcGradient(long_h);

			while (x++ < y) {
				BE_ALGORITHM();

				color1 = calcGradient(real_radius - x, long_h);
				color2 = calcGradient(real_radius - y, long_h);
				color3 = calcGradient(long_h - r + x, long_h);
				color4 = calcGradient(long_h - r + y, long_h);

				gradientFill(ptr_tl - x - py, w - 2 * r + 2 * x, x1 + r - x - y, real_radius - y);
				gradientFill(ptr_tl - y - px, w - 2 * r + 2 * y, x1 + r - y - x, real_radius - x);

				gradientFill(ptr_bl - x + py, w - 2 * r + 2 * x, x1 + r - x - y, long_h - r + y);
				gradientFill(ptr_bl - y + px, w - 2 * r + 2 * y, x1 + r - y - x, long_h - r + x);

				BE_DRAWCIRCLE_XCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);
			}
		} else {
			while (x++ < y) {
				BE_ALGORITHM();

				colorFill<PixelType>(ptr_tl - x - py, ptr_tr + x - py, color1);
				colorFill<PixelType>(ptr_tl - y - px, ptr_tr + y - px, color1);

				colorFill<PixelType>(ptr_bl - x + py, ptr_br + x + py, color1);
				colorFill<PixelType>(ptr_bl - y + px, ptr_br + y + px, color1);

				// do not remove - messes up the drawing at lower resolutions
				BE_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);
			}
		}

		ptr_fill += pitch * r;
		while (short_h--) {
			if (fill_m == kFillGradient) {
				gradientFill(ptr_fill, w + 1, x1, real_radius++);
			} else {
				colorFill<PixelType>(ptr_fill, ptr_fill + w + 1, color1);
			}
			ptr_fill += pitch;
		}
	}


	if (Base::_strokeWidth) {
		int sw = 0, sp = 0, hp = h * pitch;

		PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
		PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
		PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
		PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
		PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

		int real_radius = r;
		int short_h = h - (2 * r) + 2;

		// TODO: A gradient effect on the bevel
		PixelType color1, color2;
		color1 = Base::_bevel ? _bevelColor : color;
		color2 = color;

		while (sw++ < Base::_strokeWidth) {
			colorFill<PixelType>(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color1);
			colorFill<PixelType>(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color2);
			sp += pitch;

			BE_RESET();
			r--;

			while (x++ < y) {
				BE_ALGORITHM();
				BE_DRAWCIRCLE_BCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py);

				if (Base::_strokeWidth > 1) {
					BE_DRAWCIRCLE_BCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x - 1, y, px, py);
					BE_DRAWCIRCLE_BCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px - pitch, py);
				}
			}
		}

		ptr_fill += pitch * real_radius;
		while (short_h--) {
			colorFill<PixelType>(ptr_fill, ptr_fill + Base::_strokeWidth, color1);
			colorFill<PixelType>(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color2);
			ptr_fill += pitch;
		}
	}
}

/** CIRCLE ALGORITHM **/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawCircleAlg(int x1, int y1, int r, PixelType color, VectorRenderer::FillMode fill_m) {
	int f, ddF_x, ddF_y;
	int x, y, px, py, sw = 0;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			BE_RESET();
			r--;

			*(ptr + y) = color;
			*(ptr - y) = color;
			*(ptr + py) = color;
			*(ptr - py) = color;

			while (x++ < y) {
				BE_ALGORITHM();
				BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py);

				if (Base::_strokeWidth > 1) {
					BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x - 1, y, px, py);
					BE_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px - pitch, py);
				}
			}
		}
	} else {
		colorFill<PixelType>(ptr - r, ptr + r, color);
		BE_RESET();

		while (x++ < y) {
			BE_ALGORITHM();
			colorFill<PixelType>(ptr - x + py, ptr + x + py, color);
			colorFill<PixelType>(ptr - x - py, ptr + x - py, color);
			colorFill<PixelType>(ptr - y + px, ptr + y + px, color);
			colorFill<PixelType>(ptr - y - px, ptr + y - px, color);
		}
	}
}





/********************************************************************
 ********************************************************************
 * SHADOW drawing algorithms - VectorRendererSpec *******************
 ********************************************************************
 ********************************************************************/
template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawSquareShadow(int x, int y, int w, int h, int blur) {
	PixelType *ptr = (PixelType *)_activeSurface->getBasePtr(x + w - 1, y + blur);
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int i, j;

	i = h - blur;

	while (i--) {
		j = blur;
		while (j--)
			blendPixelPtr(ptr + j, 0, ((blur - j) << 8) / blur);
		ptr += pitch;
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x + blur, y + h - 1);

	while (i++ < blur) {
		j = w - blur;
		while (j--)
			blendPixelPtr(ptr + j, 0, ((blur - i) << 8) / blur);
		ptr += pitch;
	}

	ptr = (PixelType *)_activeSurface->getBasePtr(x + w, y + h);

	i = 0;
	while (i++ < blur) {
		j = blur - 1;
		while (j--)
			blendPixelPtr(ptr + j, 0, (((blur - j) * (blur - i)) << 8) / (blur * blur));
		ptr += pitch;
	}
}

template<typename PixelType>
void VectorRendererSpec<PixelType>::
drawRoundedSquareShadow(int x1, int y1, int r, int w, int h, int blur) {
	int f, ddF_x, ddF_y;
	int x, y, px, py;
	int pitch = _activeSurface->pitch / _activeSurface->format.bytesPerPixel;
	int alpha = 102;

	x1 += blur;
	y1 += blur;

	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
	PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - blur, y1 + r);

	int short_h = h - (2 * r) + 1;

	BE_RESET();

	// HACK: As we are drawing circles exploting 8-axis symmetry,
	// there are 4 pixels on each circle which are drawn twice.
	// this is ok on filled circles, but when blending on surfaces,
	// we cannot let it blend twice. awful.
	uint32 hb = 0;

	while (x++ < y) {
		BE_ALGORITHM();

		if (((1 << x) & hb) == 0) {
			blendFill(ptr_tr - px - r, ptr_tr + y - px, 0, alpha);
			blendFill(ptr_bl - y + px, ptr_br + y + px, 0, alpha);
			hb |= (1 << x);
		}

		if (((1 << y) & hb) == 0) {
			blendFill(ptr_tr - r - py, ptr_tr + x - py, 0, alpha);
			blendFill(ptr_bl - x + py, ptr_br + x + py, 0, alpha);
			hb |= (1 << y);
		}
	}

	while (short_h--) {
		blendFill(ptr_fill - r, ptr_fill + blur, 0, alpha);
		ptr_fill += pitch;
	}
}


/******************************************************************************/



#ifndef DISABLE_FANCY_THEMES

/********************************************************************
 * ANTIALIASED PRIMITIVES drawing algorithms - VectorRendererAA
 ********************************************************************/
/** LINES **/
template<typename PixelType>
void VectorRendererAA<PixelType>::
drawLineAlg(int x1, int y1, int x2, int y2, int dx, int dy, PixelType color) {

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);
	int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int xdir = (x2 > x1) ? 1 : -1;
	uint16 error_tmp, error_acc, gradient;
	uint8 alpha;

	*ptr = (PixelType)color;

	if (dx > dy) {
		gradient = (uint32)(dy << 16) / (uint32)dx;
		error_acc = 0;

		while (--dx) {
			error_tmp = error_acc;
			error_acc += gradient;

			if (error_acc <= error_tmp)
				ptr += pitch;

			ptr += xdir;
			alpha = (error_acc >> 8);

			this->blendPixelPtr(ptr, color, ~alpha);
			this->blendPixelPtr(ptr + pitch, color, alpha);
		}
	} else {
		gradient = (uint32)(dx << 16) / (uint32)dy;
		error_acc = 0;

		while (--dy) {
			error_tmp = error_acc;
			error_acc += gradient;

			if (error_acc <= error_tmp)
				ptr += xdir;

			ptr += pitch;
			alpha = (error_acc >> 8);

			this->blendPixelPtr(ptr, color, ~alpha);
			this->blendPixelPtr(ptr + xdir, color, alpha);
		}
	}

	Base::putPixel(x2, y2, color);
}

/** TAB ALGORITHM */
template<typename PixelType>
void VectorRendererAA<PixelType>::
drawTabAlg(int x1, int y1, int w, int h, int r, PixelType color, VectorRenderer::FillMode fill_m, int baseLeft, int baseRight) {
	int x, y, px, py;
	int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int sw = 0, sp = 0, hp = 0;

	frac_t T = 0, oldT;
	uint8 a1, a2;
	uint32 rsq = r*r;

	PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
	PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
	PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	int real_radius = r;

	if (fill_m == Base::kFillDisabled) {
		color = 0;
		while (sw++ < Base::_strokeWidth) {
			colorFill<PixelType>(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color);
			colorFill<PixelType>(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color);
			sp += pitch;

			x = r - (sw - 1);
			y = 0;
			T = 0;
			px = pitch * x;
			py = 0;


			while (x > y++) {
				WU_ALGORITHM();

				// sw == 1: outside, sw = _strokeWidth: inside
				if (sw != Base::_strokeWidth)
					a2 = 255;

				// inner arc
				WU_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px, py, a2);

				if (sw == 1) // outer arc
					WU_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px - pitch, py, a1);
			}
		}

		int short_h = h - r + 2;

		ptr_fill += pitch * real_radius;
		while (short_h--) {
			colorFill<PixelType>(ptr_fill, ptr_fill + Base::_strokeWidth, color);
			colorFill<PixelType>(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color);
			ptr_fill += pitch;
		}

		if (baseLeft) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1 + h + 1);
			while (sw++ < Base::_strokeWidth) {
				colorFill<PixelType>(ptr_fill - baseLeft, ptr_fill, color);
				ptr_fill += pitch;
			}
		}

		if (baseRight) {
			sw = 0;
			ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w, y1 + h + 1);
			while (sw++ < Base::_strokeWidth) {
				colorFill<PixelType>(ptr_fill, ptr_fill + baseRight, color);
				ptr_fill += pitch;
			}
		}
	} else {
		PixelType color1, color2;
		color1 = color2 = color;

		int long_h = h;
		int short_h = h - real_radius + 2;
		x = real_radius;
		y = 0;
		T = 0;
		px = pitch * x;
		py = 0;

		Base::precalcGradient(long_h);

		while (x > y++) {
			WU_ALGORITHM();

			if (fill_m == Base::kFillGradient) {
				color1 = Base::calcGradient(real_radius - x, long_h);
				color2 = Base::calcGradient(real_radius - y, long_h);

				Base::gradientFill(ptr_tl - x - py + 1, w - 2 * r + 2 * x - 1, x1 + r - x - y + 1, real_radius - y);

				// Only fill each horizontal line once (or we destroy
				// the gradient effect at the edges)
				if (T < oldT || y == 1)
					Base::gradientFill(ptr_tl - y - px + 1, w - 2 * r + 2 * y - 1, x1 + r - y - x + 1, real_radius - x);

				WU_DRAWCIRCLE_XCOLOR_TOP(ptr_tr, ptr_tl, x, y, px, py, a1, Base::blendPixelPtr);
			} else {
				colorFill<PixelType>(ptr_tl - x - py + 1, ptr_tr + x - py, color);
				if (T < oldT || y == 1)
					colorFill<PixelType>(ptr_tl - y - px + 1, ptr_tr + y - px, color);

				WU_DRAWCIRCLE_TOP(ptr_tr, ptr_tl, x, y, px, py, a1);
			}
		}

		ptr_fill += pitch * r;
		while (short_h--) {
			if (fill_m == Base::kFillGradient) {
				Base::gradientFill(ptr_fill, w + 1, x1, real_radius++);
			} else {
				colorFill<PixelType>(ptr_fill, ptr_fill + w + 1, color);
			}
			ptr_fill += pitch;
		}
	}
}


/** ROUNDED SQUARES **/
template<typename PixelType>
void VectorRendererAA<PixelType>::
drawRoundedSquareAlg(int x1, int y1, int r, int w, int h, PixelType color, VectorRenderer::FillMode fill_m) {
	int x, y;
	const int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int px, py;

	uint32 rsq = r*r;
	frac_t T = 0, oldT;
	uint8 a1, a2;

	// TODO: Split this up into border, bevel and interior functions

	if (Base::_strokeWidth) {
		PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
		PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
		PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
		PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
		PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

		int sw = 0, sp = 0;
		int short_h = h - 2 * r;
		int hp = h * pitch;

		int strokeWidth = Base::_strokeWidth;
		// If we're going to fill the inside, draw a slightly thicker border
		// so we can blend the inside on top of it.
		if (fill_m != Base::kFillDisabled) strokeWidth++;

		// TODO: A gradient effect on the bevel
		PixelType color1, color2;
		color1 = Base::_bevel ? Base::_bevelColor : color;
		color2 = color;


		while (sw++ < strokeWidth) {
			colorFill<PixelType>(ptr_fill + sp + r, ptr_fill + w + 1 + sp - r, color1);
			colorFill<PixelType>(ptr_fill + hp - sp + r, ptr_fill + w + hp + 1 - sp - r, color2);
			sp += pitch;

			x = r - (sw - 1);
			y = 0;
			T = 0;
			px = pitch * x;
			py = 0;

			while (x > y++) {
				WU_ALGORITHM();

				// sw == 1: outside, sw = _strokeWidth: inside
				// We always draw the outer edge AAed, but the inner edge
				// only when the inside isn't filled
				if (sw != strokeWidth || fill_m != Base::kFillDisabled)
					a2 = 255;

				// inner arc
				WU_DRAWCIRCLE_BCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, (x - 1), y, (px - pitch), py, a2);

				if (sw == 1) // outer arc
					WU_DRAWCIRCLE_BCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1);
			}
		}

		ptr_fill += pitch * r;
		while (short_h-- >= 0) {
			colorFill<PixelType>(ptr_fill, ptr_fill + Base::_strokeWidth, color1);
			colorFill<PixelType>(ptr_fill + w - Base::_strokeWidth + 1, ptr_fill + w + 1, color2);
			ptr_fill += pitch;
		}
	}

	r -= Base::_strokeWidth;
	x1 += Base::_strokeWidth;
	y1 += Base::_strokeWidth;
	w -= 2*Base::_strokeWidth;
	h -= 2*Base::_strokeWidth;
	rsq = r*r;

	if (w <= 0 || h <= 0)
		return; // Only border is visible

	if (fill_m != Base::kFillDisabled) {
		if (fill_m == Base::kFillBackground)
			color = Base::_bgColor;

		PixelType *ptr_tl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + r);
		PixelType *ptr_tr = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + r);
		PixelType *ptr_bl = (PixelType *)Base::_activeSurface->getBasePtr(x1 + r, y1 + h - r);
		PixelType *ptr_br = (PixelType *)Base::_activeSurface->getBasePtr(x1 + w - r, y1 + h - r);
		PixelType *ptr_fill = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

		int short_h = h - 2 * r;
		x = r;
		y = 0;
		T = 0;
		px = pitch * x;
		py = 0;

		if (fill_m == Base::kFillGradient) {

			Base::precalcGradient(h);

			PixelType color1, color2, color3, color4;
			while (x > y++) {
				WU_ALGORITHM();

				color1 = Base::calcGradient(r - x, h);
				color2 = Base::calcGradient(r - y, h);
				color3 = Base::calcGradient(h - r + x, h);
				color4 = Base::calcGradient(h - r + y, h);

				Base::gradientFill(ptr_tl - x - py + 1, w - 2 * r + 2 * x - 1, x1 + r - x - y + 1, r - y);

				// Only fill each horizontal line once (or we destroy
				// the gradient effect at the edges)
				if (T < oldT || y == 1)
					Base::gradientFill(ptr_tl - y - px + 1, w - 2 * r + 2 * y - 1, x1 + r - y - x + 1, r - x);

				Base::gradientFill(ptr_bl - x + py + 1, w - 2 * r + 2 * x - 1, x1 + r - x - y + 1, h - r + y);

				// Only fill each horizontal line once (or we destroy
				// the gradient effect at the edges)
				if (T < oldT || y == 1)
					Base::gradientFill(ptr_bl - y + px + 1, w - 2 * r + 2 * y - 1, x1 + r - y - x + 1, h - r + x);

				// This shape is used for dialog backgrounds.
				// If we're drawing on top of an empty overlay background,
				// and the overlay supports alpha, we have to do AA by
				// setting the dest alpha channel, instead of blending with
				// dest color channels.
				if (!g_system->hasFeature(OSystem::kFeatureOverlaySupportsAlpha))
					WU_DRAWCIRCLE_XCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1, blendPixelPtr);
				else
					WU_DRAWCIRCLE_XCOLOR(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1, blendPixelDestAlphaPtr);
			}

			ptr_fill += pitch * r;
			while (short_h-- >= 0) {
				Base::gradientFill(ptr_fill, w + 1, x1, r++);
				ptr_fill += pitch;
			}

		} else {

			while (x > 1 + y++) {
				WU_ALGORITHM();

				colorFill<PixelType>(ptr_tl - x - py + 1, ptr_tr + x - py, color);
				if (T < oldT || y == 1)
					colorFill<PixelType>(ptr_tl - y - px + 1, ptr_tr + y - px, color);

				colorFill<PixelType>(ptr_bl - x + py + 1, ptr_br + x + py, color);
				if (T < oldT || y == 1)
					colorFill<PixelType>(ptr_bl - y + px + 1, ptr_br + y + px, color);

				WU_DRAWCIRCLE(ptr_tr, ptr_tl, ptr_bl, ptr_br, x, y, px, py, a1);
			}

			ptr_fill += pitch * r;
			while (short_h-- >= 0) {
				colorFill<PixelType>(ptr_fill, ptr_fill + w + 1, color);
				ptr_fill += pitch;
			}
		}
	}
}

/** CIRCLES **/
template<typename PixelType>
void VectorRendererAA<PixelType>::
drawCircleAlg(int x1, int y1, int r, PixelType color, VectorRenderer::FillMode fill_m) {
	int x, y, sw = 0;
	const int pitch = Base::_activeSurface->pitch / Base::_activeSurface->format.bytesPerPixel;
	int px, py;

	uint32 rsq = r*r;
	frac_t T = 0, oldT;
	uint8 a1, a2;

	PixelType *ptr = (PixelType *)Base::_activeSurface->getBasePtr(x1, y1);

	if (fill_m == VectorRenderer::kFillDisabled) {
		while (sw++ < Base::_strokeWidth) {
			x = r - (sw - 1);
			y = 0;
			T = 0;
			px = pitch * x;
			py = 0;

			*(ptr + x) = (PixelType)color;
			*(ptr - x) = (PixelType)color;
			*(ptr + px) = (PixelType)color;
			*(ptr - px) = (PixelType)color;

			while (x > y++) {
				WU_ALGORITHM();

				if (sw != 1 && sw != Base::_strokeWidth)
					a2 = a1 = 255;

				WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, (x - 1), y, (px - pitch), py, a2);
				WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py, a1);
			}
		}
	} else {
		colorFill<PixelType>(ptr - r, ptr + r + 1, color);
		x = r;
		y = 0;
		T = 0;
		px = pitch * x;
		py = 0;

		while (x > y++) {
			WU_ALGORITHM();

			colorFill<PixelType>(ptr - x + py, ptr + x + py, color);
			colorFill<PixelType>(ptr - x - py, ptr + x - py, color);
			colorFill<PixelType>(ptr - y + px, ptr + y + px, color);
			colorFill<PixelType>(ptr - y - px, ptr + y - px, color);

			WU_DRAWCIRCLE(ptr, ptr, ptr, ptr, x, y, px, py, a1);
		}
	}
}

#endif

}
