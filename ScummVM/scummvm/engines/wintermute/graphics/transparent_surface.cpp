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
 */

#include "common/algorithm.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "graphics/primitives.h"
#include "engines/wintermute/graphics/transparent_surface.h"

namespace Wintermute {

byte *TransparentSurface::_lookup = NULL;

void TransparentSurface::destroyLookup() {
	delete _lookup;
	_lookup = NULL;
}

TransparentSurface::TransparentSurface() : Surface(), _enableAlphaBlit(true) {}

TransparentSurface::TransparentSurface(const Surface &surf, bool copyData) : Surface(), _enableAlphaBlit(true) {
	if (copyData) {
		copyFrom(surf);
	} else {
		w = surf.w;
		h = surf.h;
		pitch = surf.pitch;
		format = surf.format;
		pixels = surf.pixels;
	}
}

void doBlitOpaque(byte *ino, byte* outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {
	byte *in, *out;

#ifdef SCUMM_LITTLE_ENDIAN
	const int aIndex = 3;
#else
	const int aIndex = 0;
#endif

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		memcpy(out, in, width * 4);
		for (uint32 j = 0; j < width; j++) {
			out[aIndex] = 0xFF;
			out += 4;
		}
		outo += pitch;
		ino += inoStep;
	}
}

void TransparentSurface::generateLookup() {
	_lookup = new byte[256 * 256];
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			_lookup[(i << 8) + j] = (i * j) >> 8;
		}
	}
}

void TransparentSurface::doBlitAlpha(byte *ino, byte* outo, uint32 width, uint32 height, uint32 pitch, int32 inStep, int32 inoStep) {
	byte *in, *out;

	if (!_lookup) {
		generateLookup();
	}

#ifdef SCUMM_LITTLE_ENDIAN
	const int aIndex = 3;
	const int bIndex = 0;
	const int gIndex = 1;
	const int rIndex = 2;
#else
	const int aIndex = 0;
	const int bIndex = 3;
	const int gIndex = 2;
	const int rIndex = 1;
#endif

	const int bShift = 0;//img->format.bShift;
	const int gShift = 8;//img->format.gShift;
	const int rShift = 16;//img->format.rShift;
	const int aShift = 24;//img->format.aShift;

	const int bShiftTarget = 0;//target.format.bShift;
	const int gShiftTarget = 8;//target.format.gShift;
	const int rShiftTarget = 16;//target.format.rShift;

	for (uint32 i = 0; i < height; i++) {
		out = outo;
		in = ino;
		for (uint32 j = 0; j < width; j++) {
			uint32 pix = *(uint32 *)in;
			uint32 oPix = *(uint32 *) out;
			int b = (pix >> bShift) & 0xff;
			int g = (pix >> gShift) & 0xff;
			int r = (pix >> rShift) & 0xff;
			int a = (pix >> aShift) & 0xff;
			int outb, outg, outr, outa;
			in += inStep;

			switch (a) {
				case 0: // Full transparency
					out += 4;
					break;
				case 255: // Full opacity
					outb = b;
					outg = g;
					outr = r;
					outa = a;

					out[aIndex] = outa;
					out[bIndex] = outb;
					out[gIndex] = outg;
					out[rIndex] = outr;
					out += 4;
					break;

				default: // alpha blending
					outa = 255;

					outb = _lookup[(((oPix >> bShiftTarget) & 0xff)) + ((255 - a) << 8)];
					outg = _lookup[(((oPix >> gShiftTarget) & 0xff)) + ((255 - a) << 8)];
					outr = _lookup[(((oPix >> rShiftTarget) & 0xff)) + ((255 - a) << 8)];
					outb += _lookup[b + (a << 8)];
					outg += _lookup[g + (a << 8)];
					outr += _lookup[r + (a << 8)];

					out[aIndex] = outa;
					out[bIndex] = outb;
					out[gIndex] = outg;
					out[rIndex] = outr;
					out += 4;
			}
		}
		outo += pitch;
		ino += inoStep;
	}
}


Common::Rect TransparentSurface::blit(Graphics::Surface &target, int posX, int posY, int flipping, Common::Rect *pPartRect, uint color, int width, int height) {
	int ca = (color >> 24) & 0xff;

	Common::Rect retSize;
	retSize.top = 0;
	retSize.left = 0;
	retSize.setWidth(0);
	retSize.setHeight(0);
	// Check if we need to draw anything at all
	if (ca == 0)
		return retSize;

	int cr = (color >> 16) & 0xff;
	int cg = (color >> 8) & 0xff;
	int cb = (color >> 0) & 0xff;

	// Compensate for transparency. Since we're coming
	// down to 255 alpha, we just compensate for the colors here
	if (ca != 255) {
		cr = cr * ca >> 8;
		cg = cg * ca >> 8;
		cb = cb * ca >> 8;
	}

	// Create an encapsulating surface for the data
	TransparentSurface srcImage(*this, false);
	// TODO: Is the data really in the screen format?
	if (format.bytesPerPixel != 4) {
		warning("TransparentSurface can only blit 32 bpp images");
		return retSize;
	}

	if (pPartRect) {
		srcImage.pixels = &((char *)pixels)[pPartRect->top * srcImage.pitch + pPartRect->left * 4];
		srcImage.w = pPartRect->width();
		srcImage.h = pPartRect->height();

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping,
		      pPartRect->left,  pPartRect->top, pPartRect->width(), pPartRect->height(), color, width, height);
	} else {

		debug(6, "Blit(%d, %d, %d, [%d, %d, %d, %d], %08x, %d, %d)", posX, posY, flipping, 0, 0,
		      srcImage.w, srcImage.h, color, width, height);
	}

	if (width == -1)
		width = srcImage.w;
	if (height == -1)
		height = srcImage.h;

#ifdef SCALING_TESTING
	// Hardcode scaling to 66% to test scaling
	width = width * 2 / 3;
	height = height * 2 / 3;
#endif

	Graphics::Surface *img;
	Graphics::Surface *imgScaled = NULL;
	byte *savedPixels = NULL;
	if ((width != srcImage.w) || (height != srcImage.h)) {
		// Scale the image
		img = imgScaled = srcImage.scale(width, height);
		savedPixels = (byte *)img->pixels;
	} else {
		img = &srcImage;
	}

	// Handle off-screen clipping
	if (posY < 0) {
		img->h = MAX(0, (int)img->h - -posY);
		img->pixels = (byte *)img->pixels + img->pitch * -posY;
		posY = 0;
	}

	if (posX < 0) {
		img->w = MAX(0, (int)img->w - -posX);
		img->pixels = (byte *)img->pixels + (-posX * 4);
		posX = 0;
	}

	img->w = CLIP((int)img->w, 0, (int)MAX((int)target.w - posX, 0));
	img->h = CLIP((int)img->h, 0, (int)MAX((int)target.h - posY, 0));

	if ((img->w > 0) && (img->h > 0)) {
		int xp = 0, yp = 0;

		int inStep = 4;
		int inoStep = img->pitch;
		if (flipping & TransparentSurface::FLIP_V) {
			inStep = -inStep;
			xp = img->w - 1;
		}

		if (flipping & TransparentSurface::FLIP_H) {
			inoStep = -inoStep;
			yp = img->h - 1;
		}

		byte *ino = (byte *)img->getBasePtr(xp, yp);
		byte *outo = (byte *)target.getBasePtr(posX, posY);
		byte *in, *out;

#ifdef SCUMM_LITTLE_ENDIAN
		const int aIndex = 3;
		const int bIndex = 0;
		const int gIndex = 1;
		const int rIndex = 2;
#else
		const int aIndex = 0;
		const int bIndex = 3;
		const int gIndex = 2;
		const int rIndex = 1;
#endif
		const int bShift = 0;//img->format.bShift;
		const int gShift = 8;//img->format.gShift;
		const int rShift = 16;//img->format.rShift;
		const int aShift = 24;//img->format.aShift;

		const int bShiftTarget = 0;//target.format.bShift;
		const int gShiftTarget = 8;//target.format.gShift;
		const int rShiftTarget = 16;//target.format.rShift;

		if (ca == 255 && cb == 255 && cg == 255 && cr == 255) {
			if (_enableAlphaBlit) {
				doBlitAlpha(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
			} else {
				doBlitOpaque(ino, outo, img->w, img->h, target.pitch, inStep, inoStep);
			}
		} else {
			for (int i = 0; i < img->h; i++) {
				out = outo;
				in = ino;
				for (int j = 0; j < img->w; j++) {
					uint32 pix = *(uint32 *)in;
					uint32 o_pix = *(uint32 *) out;
					int b = (pix >> bShift) & 0xff;
					int g = (pix >> gShift) & 0xff;
					int r = (pix >> rShift) & 0xff;
					int a = (pix >> aShift) & 0xff;
					int outb, outg, outr, outa;
					in += inStep;

					if (ca != 255) {
						a = a * ca >> 8;
					}

					switch (a) {
					case 0: // Full transparency
						out += 4;
						break;
					case 255: // Full opacity
						if (cb != 255)
							outb = (b * cb) >> 8;
						else
							outb = b;

						if (cg != 255)
							outg = (g * cg) >> 8;
						else
							outg = g;

						if (cr != 255)
							outr = (r * cr) >> 8;
						else
							outr = r;
						outa = a;
						out[aIndex] = outa;
						out[bIndex] = outb;
						out[gIndex] = outg;
						out[rIndex] = outr;
						out += 4;
						break;

					default: // alpha blending
						outa = 255;
						outb = (o_pix >> bShiftTarget) & 0xff;
						outg = (o_pix >> gShiftTarget) & 0xff;
						outr = (o_pix >> rShiftTarget) & 0xff;
						if (cb == 0)
							outb = 0;
						else if (cb != 255)
							outb += ((b - outb) * a * cb) >> 16;
						else
							outb += ((b - outb) * a) >> 8;
						if (cg == 0)
							outg = 0;
						else if (cg != 255)
							outg += ((g - outg) * a * cg) >> 16;
						else
							outg += ((g - outg) * a) >> 8;
						if (cr == 0)
							outr = 0;
						else if (cr != 255)
							outr += ((r - outr) * a * cr) >> 16;
						else
							outr += ((r - outr) * a) >> 8;
						out[aIndex] = outa;
						out[bIndex] = outb;
						out[gIndex] = outg;
						out[rIndex] = outr;
						out += 4;
					}
				}
				outo += target.pitch;
				ino += inoStep;
			}
		}
	}

	if (imgScaled) {
		imgScaled->pixels = savedPixels;
		imgScaled->free();
		delete imgScaled;
	}

	retSize.setWidth(img->w);
	retSize.setHeight(img->h);
	return retSize;
}

TransparentSurface *TransparentSurface::scale(uint16 newWidth, uint16 newHeight) const {
	Common::Rect srcRect(0, 0, (int16)w, (int16)h);
	Common::Rect dstRect(0, 0, (int16)newWidth, (int16)newHeight);
	return scale(srcRect, dstRect);
}

// Copied from clone2727's https://github.com/clone2727/scummvm/blob/pegasus/engines/pegasus/surface.cpp#L247
TransparentSurface *TransparentSurface::scale(const Common::Rect &srcRect, const Common::Rect &dstRect) const {
	// I'm doing simple linear scaling here
	// dstRect(x, y) = srcRect(x * srcW / dstW, y * srcH / dstH);
	TransparentSurface *target = new TransparentSurface();

	int srcW = srcRect.width();
	int srcH = srcRect.height();
	int dstW = dstRect.width();
	int dstH = dstRect.height();

	target->create((uint16)dstW, (uint16)dstH, this->format);

	for (int y = 0; y < dstH; y++) {
		for (int x = 0; x < dstW; x++) {
			uint32 color = READ_UINT32((const byte *)getBasePtr(x * srcW / dstW + srcRect.left,
														  y * srcH / dstH + srcRect.top));
			WRITE_UINT32((byte *)target->getBasePtr(x + dstRect.left, y + dstRect.top), color);
		}
	}
	return target;

}

/**
 * Writes a color key to the alpha channel of the surface
 * @param rKey  the red component of the color key
 * @param gKey  the green component of the color key
 * @param bKey  the blue component of the color key
 * @param overwriteAlpha if true, all other alpha will be set fully opaque
 */
void TransparentSurface::applyColorKey(uint8 rKey, uint8 gKey, uint8 bKey, bool overwriteAlpha) {
	assert(format.bytesPerPixel == 4);
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			uint32 pix = ((uint32 *)pixels)[i * w + j];
			uint8 r, g, b, a;
			format.colorToARGB(pix, a, r, g, b);
			if (r == rKey && g == gKey && b == bKey) {
				a = 0;
				((uint32 *)pixels)[i * w + j] = format.ARGBToColor(a, r, g, b);
			} else if (overwriteAlpha) {
				a = 255;
				((uint32 *)pixels)[i * w + j] = format.ARGBToColor(a, r, g, b);
			}
		}
	}
}

} // End of namespace Graphics
