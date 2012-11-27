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


#include "scumm/scumm.h"
#include "scumm/akos.h"
#include "scumm/bomp.h"
#include "scumm/util.h"


namespace Scumm {

static int32 setupBompScale(byte *scaling, int32 size, byte scale);

static void bompScaleFuncX(byte *line_buffer, byte *scaling_x_ptr, byte skip, int32 size);

static void bompApplyShadow0(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency, bool HE7Check);
static void bompApplyShadow1(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency);
static void bompApplyShadow3(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency);
static void bompApplyActorPalette(uint16 *actorPalette, byte *line_buffer, int32 size);



void decompressBomp(byte *dst, const byte *src, int w, int h) {
	assert(w > 0);
	assert(h > 0);

	do {
		bompDecodeLine(dst, src + 2, w);
		src += READ_LE_UINT16(src) + 2;
		dst += w;
	} while (--h);
}

void bompDecodeLine(byte *dst, const byte *src, int len) {
	assert(len > 0);

	int num;
	byte code, color;

	while (len > 0) {
		code = *src++;
		num = (code >> 1) + 1;
		if (num > len)
			num = len;
		len -= num;
		if (code & 1) {
			color = *src++;
			memset(dst, color, num);
		} else {
			memcpy(dst, src, num);
			src += num;
		}
		dst += num;
	}
}

void bompDecodeLineReverse(byte *dst, const byte *src, int len) {
	assert(len > 0);

	dst += len;

	int num;
	byte code, color;

	while (len > 0) {
		code = *src++;
		num = (code >> 1) + 1;
		if (num > len)
			num = len;
		len -= num;
		dst -= num;
		if (code & 1) {
			color = *src++;
			memset(dst, color, num);
		} else {
			memcpy(dst, src, num);
			src += num;
		}
	}
}

void bompApplyMask(byte *line_buffer, byte *mask, byte maskbit, int32 size, byte transparency) {
	while (1) {
		do {
			if (size-- == 0)
				return;
			if (*mask & maskbit) {
				*line_buffer = transparency;
			}
			line_buffer++;
			maskbit >>= 1;
		} while	(maskbit);
		mask++;
		maskbit = 128;
	}
}

void bompApplyShadow(int shadowMode, const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency, bool HE7Check) {
	assert(size > 0);
	switch (shadowMode) {
	case 0:
		bompApplyShadow0(shadowPalette, line_buffer, dst, size, transparency, HE7Check);
		break;
	case 1:
		bompApplyShadow1(shadowPalette, line_buffer, dst, size, transparency);
		break;
	case 3:
		bompApplyShadow3(shadowPalette, line_buffer, dst, size, transparency);
		break;
	default:
		error("Unknown shadow mode %d", shadowMode);
	}
}
void bompApplyShadow0(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency, bool HE7Check) {
	while (size-- > 0) {
		byte tmp = *line_buffer++;
		if (tmp != transparency) {
			if (HE7Check)
				*dst = shadowPalette[tmp];
			else
				*dst = tmp;
		}
		dst++;
	}
}

void bompApplyShadow1(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency) {
	while (size-- > 0) {
		byte tmp = *line_buffer++;
		if (tmp != transparency) {
			if (tmp == 13) {
				tmp = shadowPalette[*dst];
			}
			*dst = tmp;
		}
		dst++;
	}
}

void bompApplyShadow3(const byte *shadowPalette, const byte *line_buffer, byte *dst, int32 size, byte transparency) {
	while (size-- > 0) {
		byte tmp = *line_buffer++;
		if (tmp != transparency) {
			if (tmp < 8) {
				tmp = shadowPalette[*dst + (tmp << 8)];
			}
			*dst = tmp;
		}
		dst++;
	}
}

void bompApplyActorPalette(uint16 *actorPalette, byte *line_buffer, int32 size) {
	actorPalette[255] = 255;
	while (size-- > 0) {
		*line_buffer = actorPalette[*line_buffer];
		line_buffer++;
	}
}

void bompScaleFuncX(byte *line_buffer, byte *scaling_x_ptr, byte skip, int32 size) {
	byte *line_ptr1 = line_buffer;
	byte *line_ptr2 = line_buffer;

	byte tmp = *scaling_x_ptr++;

	while (size--) {
		if ((skip & tmp) == 0) {
			*line_ptr1++ = *line_ptr2;
		}
		line_ptr2++;
		skip >>= 1;
		if (skip == 0) {
			skip = 128;
			tmp = *scaling_x_ptr++;
		}
	}
}

void drawBomp(const BompDrawData &bd) {
	const byte *src;
	byte *dst;
	byte *mask = 0;
	Common::Rect clip;
	byte *scalingYPtr = 0;
	byte skip_y_bits = 0x80;
	byte skip_y_new = 0;
	byte tmp;
	byte bomp_scaling_x[64];
	byte bomp_scaling_y[64];

	if (bd.x < 0) {
		clip.left = -bd.x;
	} else {
		clip.left = 0;
	}

	if (bd.y < 0) {
		clip.top = -bd.y;
	} else {
		clip.top = 0;
	}

	clip.right = bd.srcwidth;
	if (clip.right > bd.dst.w - bd.x) {
		clip.right = bd.dst.w - bd.x;
	}

	clip.bottom = bd.srcheight;
	if (clip.bottom > bd.dst.h - bd.y) {
		clip.bottom = bd.dst.h - bd.y;
	}

	src = bd.src;
	dst = (byte *)bd.dst.pixels + bd.y * bd.dst.pitch + (bd.x + clip.left);

	const byte maskbit = revBitMask((bd.x + clip.left) & 7);

	// Mask against any additionally imposed mask
	if (bd.maskPtr) {
		mask = bd.maskPtr + (bd.y * bd.numStrips) + ((bd.x + clip.left) / 8);
	}

	// Setup vertical scaling
	if (bd.scale_y != 255) {
		int scaleBottom = setupBompScale(bomp_scaling_y, bd.srcheight, bd.scale_y);
		scalingYPtr = bomp_scaling_y;

		skip_y_new = *scalingYPtr++;
		skip_y_bits = 0x80;

		if (clip.bottom > scaleBottom) {
			clip.bottom = scaleBottom;
		}
	}

	// Setup horizontal scaling
	if (bd.scale_x != 255) {
		int scaleRight = setupBompScale(bomp_scaling_x, bd.srcwidth, bd.scale_x);

		if (clip.right > scaleRight) {
			clip.right = scaleRight;
		}
	}

	const int width = clip.right - clip.left;

	if (width <= 0)
		return;

	int pos_y = 0;
	byte line_buffer[1024];

	byte *line_ptr = line_buffer + clip.left;

	// Loop over all lines
	while (pos_y < clip.bottom) {
		// Decode a single (bomp encoded) line, reversed if we are in mirror mode
		if (bd.mirror)
			bompDecodeLineReverse(line_buffer, src + 2, bd.srcwidth);
		else
			bompDecodeLine(line_buffer, src + 2, bd.srcwidth);
		src += READ_LE_UINT16(src) + 2;

		// If vertical scaling is enabled, do it
		if (bd.scale_y != 255) {
			// A bit set means we should skip this line...
			tmp = skip_y_new & skip_y_bits;

			// Advance the scale-skip bit mask, if it's 0, get the next scale-skip byte
			skip_y_bits /= 2;
			if (skip_y_bits == 0) {
				skip_y_bits = 0x80;
				skip_y_new = *scalingYPtr++;
			}

			// Skip the current line if the above check tells us to
			if (tmp != 0)
				continue;
		}

		// Perform horizontal scaling
		if (bd.scale_x != 255) {
			bompScaleFuncX(line_buffer, bomp_scaling_x, 0x80, bd.srcwidth);
		}

		// The first clip.top lines are to be clipped, i.e. not drawn
		if (clip.top > 0) {
			clip.top--;
		} else {
			// Replace the parts of the line which are masked with the transparency color
			if (bd.maskPtr)
				bompApplyMask(line_ptr, mask, maskbit, width, 255);

			// Apply custom color map, if available
			if (bd.actorPalette)
				bompApplyActorPalette(bd.actorPalette, line_ptr, width);

			// Finally, draw the decoded, scaled, masked and recolored line onto
			// the target surface, using the specified shadow mode
			bompApplyShadow(bd.shadowMode, bd.shadowPalette, line_ptr, dst, width, 255);
		}

		// Advance to the next line
		pos_y++;
		mask += bd.numStrips;
		dst += bd.dst.pitch;
	}
}

int32 setupBompScale(byte *scaling, int32 size, byte scale) {
	static const int offsets[8] = { 3, 2, 1, 0, 7, 6, 5, 4 };
	int32 count;
	byte bitsCount = 0;

	count = (256 - size / 2);
	assert(0 <= count && count < 768);
	const byte *scaleTable = bigCostumeScaleTable + count;

	count = (size + 7) / 8;
	while (count--) {
		byte scaleMask = 0;
		for (int i = 0; i < 8; i++) {
			byte scaleTest = *(scaleTable + offsets[i]);
			scaleMask <<= 1;
			if (scale < scaleTest) {
				scaleMask |= 1;
			} else {
				bitsCount++;
			}
		}
		scaleTable += 8;

		*scaling++ = scaleMask;
	}
	size &= 7;
	if (size != 0) {
		--scaling;
		if ((*scaling & revBitMask(size)) == 0) {
			*scaling |= revBitMask(size);
			bitsCount--;
		}
	}

	return bitsCount;
}

} // End of namespace Scumm
