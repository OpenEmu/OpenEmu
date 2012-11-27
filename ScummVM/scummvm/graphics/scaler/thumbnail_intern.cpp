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

#include "common/endian.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "graphics/colormasks.h"
#include "graphics/scaler.h"
#include "graphics/scaler/intern.h"
#include "graphics/palette.h"

template<int bitFormat>
uint16 quadBlockInterpolate(const uint8 *src, uint32 srcPitch) {
	uint16 colorx1y1 = *(((const uint16 *)src));
	uint16 colorx2y1 = *(((const uint16 *)src) + 1);

	uint16 colorx1y2 = *(((const uint16 *)(src + srcPitch)));
	uint16 colorx2y2 = *(((const uint16 *)(src + srcPitch)) + 1);

	return interpolate16_1_1_1_1<Graphics::ColorMasks<bitFormat> >(colorx1y1, colorx2y1, colorx1y2, colorx2y2);
}

template<int bitFormat>
void createThumbnail_2(const uint8 *src, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	assert(width % 2 == 0);
	assert(height % 2 == 0);
	for (int y = 0; y < height; y += 2) {
		for (int x = 0; x < width; x += 2, dstPtr += 2) {
			*((uint16 *)dstPtr) = quadBlockInterpolate<bitFormat>(src + 2 * x, srcPitch);
		}
		dstPtr += (dstPitch - 2 * width / 2);
		src += 2 * srcPitch;
	}
}

template<int bitFormat>
void createThumbnail_4(const uint8 *src, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	assert(width % 4 == 0);
	assert(height % 4 == 0);
	for (int y = 0; y < height; y += 4) {
		for (int x = 0; x < width; x += 4, dstPtr += 2) {
			uint16 upleft = quadBlockInterpolate<bitFormat>(src + 2 * x, srcPitch);
			uint16 upright = quadBlockInterpolate<bitFormat>(src + 2 * (x + 2), srcPitch);
			uint16 downleft = quadBlockInterpolate<bitFormat>(src + srcPitch * 2 + 2 * x, srcPitch);
			uint16 downright = quadBlockInterpolate<bitFormat>(src + srcPitch * 2 + 2 * (x + 2), srcPitch);

			*((uint16 *)dstPtr) = interpolate16_1_1_1_1<Graphics::ColorMasks<bitFormat> >(upleft, upright, downleft, downright);
		}
		dstPtr += (dstPitch - 2 * width / 4);
		src += 4 * srcPitch;
	}
}

static void createThumbnail(const uint8 *src, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	// only 1/2 and 1/4 downscale supported
	if (width != 320 && width != 640)
		return;

	int downScaleMode = (width == 320) ? 2 : 4;

	if (downScaleMode == 2) {
		createThumbnail_2<565>(src, srcPitch, dstPtr, dstPitch, width, height);
	} else if (downScaleMode == 4) {
		createThumbnail_4<565>(src, srcPitch, dstPtr, dstPitch, width, height);
	}
}


/**
 * Copies the current screen contents to a new surface, using RGB565 format.
 * WARNING: surf->free() must be called by the user to avoid leaking.
 *
 * @param surf		the surface to store the data in it
 */
static bool grabScreen565(Graphics::Surface *surf) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return false;

	assert(screen->format.bytesPerPixel == 1 || screen->format.bytesPerPixel == 2);
	assert(screen->pixels != 0);

	Graphics::PixelFormat screenFormat = g_system->getScreenFormat();

	surf->create(screen->w, screen->h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	byte *palette = 0;
	if (screenFormat.bytesPerPixel == 1) {
		palette = new byte[256 * 3];
		assert(palette);
		g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	}

	for (uint y = 0; y < screen->h; ++y) {
		for (uint x = 0; x < screen->w; ++x) {
			byte r = 0, g = 0, b = 0;

			if (screenFormat.bytesPerPixel == 1) {
				r = palette[((uint8 *)screen->pixels)[y * screen->pitch + x] * 3];
				g = palette[((uint8 *)screen->pixels)[y * screen->pitch + x] * 3 + 1];
				b = palette[((uint8 *)screen->pixels)[y * screen->pitch + x] * 3 + 2];
			} else if (screenFormat.bytesPerPixel == 2) {
				uint16 col = READ_UINT16(screen->getBasePtr(x, y));
				screenFormat.colorToRGB(col, r, g, b);
			}

			((uint16 *)surf->pixels)[y * surf->w + x] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(r, g, b);
		}
	}

	delete[] palette;

	g_system->unlockScreen();
	return true;
}

static bool createThumbnail(Graphics::Surface &out, Graphics::Surface &in) {
	uint16 width = in.w;
	uint16 inHeight = in.h;

	if (width < 320) {
		// Special case to handle MM NES (uses a screen width of 256)
		width = 320;

		// center MM NES screen
		Graphics::Surface newscreen;
		newscreen.create(width, in.h, in.format);

		uint8 *dst = (uint8 *)newscreen.getBasePtr((320 - in.w) / 2, 0);
		const uint8 *src = (const uint8 *)in.getBasePtr(0, 0);
		uint16 height = in.h;

		while (height--) {
			memcpy(dst, src, in.pitch);
			dst += newscreen.pitch;
			src += in.pitch;
		}

		in.free();
		in = newscreen;
	} else if (width == 720) {
		// Special case to handle Hercules mode
		//
		// NOTE: This code is pretty SCUMM specific.
		// For other games this code might cut off
		// not only the menu, but also other graphics.
		width = 640;
		inHeight = 400;

		// cut off menu and so on..
		Graphics::Surface newscreen;
		newscreen.create(width, 400, in.format);

		uint8 *dst = (uint8 *)newscreen.getBasePtr(0, (400 - 240) / 2);
		const uint8 *src = (const uint8 *)in.getBasePtr(41, 28);

		for (int y = 0; y < 240; ++y) {
			memcpy(dst, src, 640 * in.format.bytesPerPixel);
			dst += newscreen.pitch;
			src += in.pitch;
		}

		in.free();
		in = newscreen;
	} else if (width == 640 && inHeight == 440) {
		// Special case to handle KQ6 Windows: resize the screen to 640x480,
		// adding a black band in the bottom.
		inHeight = 480;

		Graphics::Surface newscreen;
		newscreen.create(width, 480, in.format);

		memcpy(newscreen.getBasePtr(0, 0), in.getBasePtr(0, 0), width * 440 * in.format.bytesPerPixel);

		in.free();
		in = newscreen;
	}

	uint16 newHeight = !(inHeight % 240) ? kThumbnailHeight2 : kThumbnailHeight1;

	out.create(kThumbnailWidth, newHeight, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	createThumbnail((const uint8 *)in.pixels, width * sizeof(uint16), (uint8 *)out.pixels, out.pitch, width, inHeight);

	in.free();

	return true;
}

bool createThumbnailFromScreen(Graphics::Surface* surf) {
	assert(surf);

	Graphics::Surface screen;

	if (!grabScreen565(&screen))
		return false;

	return createThumbnail(*surf, screen);
}

bool createThumbnail(Graphics::Surface *surf, const uint8 *pixels, int w, int h, const uint8 *palette) {
	assert(surf);

	Graphics::Surface screen;
	screen.create(w, h, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	for (uint y = 0; y < screen.h; ++y) {
		for (uint x = 0; x < screen.w; ++x) {
			byte r, g, b;
			r = palette[pixels[y * w + x] * 3];
			g = palette[pixels[y * w + x] * 3 + 1];
			b = palette[pixels[y * w + x] * 3 + 2];

			((uint16 *)screen.pixels)[y * screen.w + x] = Graphics::RGBToColor<Graphics::ColorMasks<565> >(r, g, b);
		}
	}

	return createThumbnail(*surf, screen);
}
