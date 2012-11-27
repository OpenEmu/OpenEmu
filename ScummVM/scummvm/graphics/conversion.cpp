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

#include "graphics/conversion.h"
#include "graphics/pixelformat.h"

#include "common/endian.h"

namespace Graphics {

// TODO: YUV to RGB conversion function

namespace {

template<typename SrcColor, typename DstColor, bool backward>
inline void crossBlitLogic(byte *dst, const byte *src, const uint w, const uint h,
                           const PixelFormat &srcFmt, const PixelFormat &dstFmt,
                           const uint srcDelta, const uint dstDelta) {
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			const uint32 color = *(const SrcColor *)src;
			byte a, r, g, b;
			srcFmt.colorToARGB(color, a, r, g, b);
			*(DstColor *)dst = dstFmt.ARGBToColor(a, r, g, b);

			if (backward) {
				src -= sizeof(SrcColor);
				dst -= sizeof(DstColor);
			} else {
				src += sizeof(SrcColor);
				dst += sizeof(DstColor);
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
		}
	}
}

template<typename DstColor, bool backward>
inline void crossBlitLogic3BppSource(byte *dst, const byte *src, const uint w, const uint h,
                                     const PixelFormat &srcFmt, const PixelFormat &dstFmt,
                                     const uint srcDelta, const uint dstDelta) {
	uint32 color;
	byte r, g, b, a;
	uint8 *col = (uint8 *)&color;
#ifdef SCUMM_BIG_ENDIAN
	col++;
#endif
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			memcpy(col, src, 3);
			srcFmt.colorToARGB(color, a, r, g, b);
			*(DstColor *)dst = dstFmt.ARGBToColor(a, r, g, b);

			if (backward) {
				src -= 3;
				dst -= sizeof(DstColor);
			} else {
				src += 3;
				dst += sizeof(DstColor);
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
		}
	}
}

} // End of anonymous namespace

// Function to blit a rect from one color format to another
bool crossBlit(byte *dst, const byte *src,
               const uint dstPitch, const uint srcPitch,
               const uint w, const uint h,
               const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			 || (dstFmt.bytesPerPixel == 3)
			 || (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	// Don't perform unnecessary conversion
	if (srcFmt == dstFmt) {
		if (dst != src) {
			if (dstPitch == srcPitch && ((w * dstFmt.bytesPerPixel) == dstPitch)) {
				memcpy(dst, src, dstPitch * h);
			} else {
				for (uint i = 0; i < h; ++i) {
					memcpy(dst, src, w * dstFmt.bytesPerPixel);
					dst += dstPitch;
					src += srcPitch;
				}
			}
		}

		return true;
	}

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * srcFmt.bytesPerPixel);
	const uint dstDelta = (dstPitch - w * dstFmt.bytesPerPixel);

	// TODO: optimized cases for dstDelta of 0
	if (dstFmt.bytesPerPixel == 2) {
		if (srcFmt.bytesPerPixel == 2) {
			crossBlitLogic<uint16, uint16, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		} else if (srcFmt.bytesPerPixel == 3) {
			crossBlitLogic3BppSource<uint16, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		} else {
			crossBlitLogic<uint32, uint16, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		}
	} else if (dstFmt.bytesPerPixel == 4) {
		if (srcFmt.bytesPerPixel == 2) {
			// We need to blit the surface from bottom right to top left here.
			// This is neeeded, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			crossBlitLogic<uint16, uint32, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		} else if (srcFmt.bytesPerPixel == 3) {
			// We need to blit the surface from bottom right to top left here.
			// This is neeeded, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			crossBlitLogic3BppSource<uint32, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		} else {
			crossBlitLogic<uint32, uint32, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		}
	} else {
		return false;
	}
	return true;
}

} // End of namespace Graphics
