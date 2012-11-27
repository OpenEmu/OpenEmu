/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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

#include "common/endian.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/screen.h"

namespace Sword2 {

/**
 * This function takes a sprite and creates a mirror image of it.
 * @param dst destination buffer
 * @param src source buffer
 * @param w width of the sprite
 * @param h height of the sprite
 */

void Screen::mirrorSprite(byte *dst, byte *src, int16 w, int16 h) {
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			*dst++ = *(src + w - x - 1);
		}
		src += w;
	}
}

/**
 * This function takes a compressed frame of a sprite with up to 256 colors
 * and decompresses it.
 * @param dst destination buffer
 * @param src source buffer
 * @param decompSize the expected size of the decompressed sprite
 */

int32 Screen::decompressRLE256(byte *dst, byte *src, int32 decompSize) {
	// PARAMETERS:
	// source	points to the start of the sprite data for input
	// decompSize	gives size of decompressed data in bytes
	// dest		points to start of destination buffer for decompressed
	//		data

	byte headerByte;			// block header byte
	byte *endDest = dst + decompSize;	// pointer to byte after end of decomp buffer
	int32 rv;

	while (1) {
		// FLAT block
		// read FLAT block header & increment 'scan' to first pixel
		// of block
		headerByte = *src++;

		// if this isn't a zero-length block
		if (headerByte) {
			if (dst + headerByte > endDest) {
				rv = 1;
				break;
			}

			// set the next 'headerByte' pixels to the next color
			// at 'source'
			memset(dst, *src, headerByte);

			// increment destination pointer to just after this
			// block
			dst += headerByte;

			// increment source pointer to just after this color
			src++;

			// if we've decompressed all of the data
			if (dst == endDest) {
				rv = 0;		// return "OK"
				break;
			}
		}

		// RAW block
		// read RAW block header & increment 'scan' to first pixel of
		// block
		headerByte = *src++;

		// if this isn't a zero-length block
		if (headerByte) {
			if (dst + headerByte > endDest) {
				rv = 1;
				break;
			}

			// copy the next 'headerByte' pixels from source to
			// destination
			memcpy(dst, src, headerByte);

			// increment destination pointer to just after this
			// block
			dst += headerByte;

			// increment source pointer to just after this block
			src += headerByte;

			// if we've decompressed all of the data
			if (dst == endDest) {
				rv = 0;		// return "OK"
				break;
			}
		}
	}

	return rv;
}

/**
 * Unwinds a run of 16-color data into 256-color palette data.
 */

void Screen::unwindRaw16(byte *dst, byte *src, uint16 blockSize, byte *colTable) {
	// for each pair of pixels
	while (blockSize > 1) {

		if (Sword2Engine::isPsx()) {
			// 1st color = number in table at position given by upper
			// nibble of source byte
			*dst++ = colTable[(*src) & 0x0f];

			// 2nd color = number in table at position given by lower
			// nibble of source byte
			*dst++ = colTable[(*src) >> 4];
		} else {
			*dst++ = colTable[(*src) >> 4];
			*dst++ = colTable[(*src) & 0x0f];
		}


		// point to next source byte
		src++;

		// decrement count of how many pixels left to read
		blockSize -= 2;
	}

	// if there's a final odd pixel
	if (blockSize) {
		// color = number in table at position given by upper nibble
		// of source byte
		*dst++ = colTable[(*src) >> 4];
	}
}

/**
 * This function takes a compressed frame of a sprite (with up to 16 colors)
 * and decompresses it.
 * @param dst destination buffer
 * @param src source buffer
 * @param decompSize the expected size of the uncompressed sprite
 * @param colTable mapping from the 16 encoded colors to the current palette
 */

int32 Screen::decompressRLE16(byte *dst, byte *src, int32 decompSize, byte *colTable) {
	byte headerByte;			// block header byte
	byte *endDest = dst + decompSize;	// pointer to byte after end of decomp buffer
	int32 rv;

	while (1) {
		// FLAT block
		// read FLAT block header & increment 'scan' to first pixel
		// of block
		headerByte = *src++;

		// if this isn't a zero-length block
		if (headerByte) {
			if (dst + headerByte > endDest) {
				rv = 1;
				break;
			}

			// set the next 'headerByte' pixels to the next
			// color at 'source'
			memset(dst, *src, headerByte);

			// increment destination pointer to just after this
			// block
			dst += headerByte;

			// increment source pointer to just after this color
			src++;

			// if we've decompressed all of the data
			if (dst == endDest) {
				rv = 0;		// return "OK"
				break;
			}
		}

		// RAW block
		// read RAW block header & increment 'scan' to first pixel of
		// block
		headerByte = *src++;

		// if this isn't a zero-length block
		if (headerByte) {
			if (dst + headerByte > endDest) {
				rv = 1;
				break;
			}

			// copy the next 'headerByte' pixels from source to
			// destination (NB. 2 pixels per byte)
			unwindRaw16(dst, src, headerByte, colTable);

			// increment destination pointer to just after this
			// block
			dst += headerByte;

			// increment source pointer to just after this block
			// (NB. headerByte gives pixels, so /2 for bytes)
			src += (headerByte + 1) / 2;

			// if we've decompressed all of the data
			if (dst >= endDest) {
				rv = 0;		// return "OK"
				break;
			}
		}
	}

	return rv;
}

/**
 * This function takes a compressed HIF image and decompresses it.
 * Used for PSX version sprites.
 * @param dst destination buffer
 * @param src source buffer
 * @param skipData if pointer != NULL, value of pointed var
 * is set to number of bytes composing the compressed data.
 */

uint32 Screen::decompressHIF(byte *src, byte *dst, uint32 *skipData) {
	uint32 decompSize = 0;
	uint32 readByte = 0;

	for (;;) { // Main loop
		byte control_byte = *src++;
		readByte++;
		uint32 byte_count = 0;
		while (byte_count < 8) {
			if (control_byte & 0x80) {
				uint16 info_word = READ_BE_UINT16(src); // Read the info word
				src += 2;
				readByte += 2;
				if (info_word == 0xFFFF) { // Got 0xFFFF code, finished.
					if (skipData != NULL) *(skipData) = readByte;
					return decompSize;
				}

				int32 repeat_count = (info_word >> 12) + 2; // How many time data needs to be refetched
				while (repeat_count >= 0) {
					uint16 refetchData = (info_word & 0xFFF) + 1;
					if (refetchData > decompSize) return 0; // We have a problem here...
					uint8 *old_data_src = dst - refetchData;
					*dst++ = *old_data_src;
					decompSize++;
					repeat_count--;
				}
			} else {
				*dst++ = *src++;
				readByte++;
				decompSize++;
			}
			byte_count++;
			control_byte <<= 1; // Shifting left the control code one bit
		}
	}
}

// Double line image to keep aspect ratio.
// Used in PSX version.
void Screen::resizePsxSprite(byte *dst, byte *src, uint16 destW, uint16 destH) {
	for (int i = 0; i < destH / 2; i++) {
		memcpy(dst + i * destW * 2, src + i * destW, destW);
		memcpy(dst + i * destW * 2 + destW, src + i * destW, destW);
	}
}

// Sprites wider than 254px in PSX version are divided
// into slices, this recomposes the image.
void Screen::recomposePsxSprite(SpriteInfo *s) {
	if (!s)
		return;

	uint16 noStripes = (s->w / 254) + ((s->w % 254) ? 1 : 0);
	uint16 lastStripeSize = (s->w % 254) ? s->w % 254 : 254;
	byte *buffer = (byte *)malloc(s->w * s->h / 2);

	memset(buffer, 0, s->w * s->h / 2);

	for (int idx = 0; idx < noStripes; idx++) {
		uint16 stripeSize = (idx == noStripes - 1) ? lastStripeSize : 254;
		for (int line = 0; line < s->h / 2; line++) {
			memcpy(buffer + idx * 254 + line * s->w, s->data, stripeSize);
			s->data += stripeSize;
		}
	}

	s->data = buffer;

}

// Recomposes sprites wider than 254 pixels but also
// compressed with HIF.
// Used in PSX version.
void Screen::recomposeCompPsxSprite(SpriteInfo *s) {
	if (!s)
		return;

	uint16 noStripes = (s->w / 254) + ((s->w % 254) ? 1 : 0);
	uint16 lastStripeSize = (s->w % 254) ? s->w % 254 : 254;
	byte *buffer = (byte *)malloc(s->w * s->h / 2);
	byte *stripeBuffer = (byte *)malloc(254 * s->h);

	memset(buffer, 0, s->w * s->h / 2);
	uint32 skipData = 0;
	uint32 compBytes = 0;

	for (int idx = 0; idx < noStripes; idx++) {
		uint16 stripeSize = (idx == noStripes - 1) ? lastStripeSize : 254;

		decompressHIF((s->data) + skipData, stripeBuffer, &compBytes);
		skipData += compBytes;

		for (int line = 0; line < s->h / 2; line++) {
			memcpy(buffer + idx * 254 + line * s->w, stripeBuffer + line * stripeSize, stripeSize);
		}
	}

	free(stripeBuffer);
	s->data = buffer;

}

/**
 * Creates a sprite surface. Sprite surfaces are used by the in-game dialogs
 * and for displaying cutscene subtitles, which makes them much easier to draw
 * than standard sprites.
 * @param s information about how to decode the sprite
 * @param sprite the buffer that will be created to store the surface
 * @return RD_OK, or an error code
 */

int32 Screen::createSurface(SpriteInfo *s, byte **sprite) {
	*sprite = (byte *)malloc(s->w * s->h);
	if (!*sprite)
		return RDERR_OUTOFMEMORY;

	// Surfaces are either uncompressed or RLE256-compressed. No need to
	// test for anything else.

	if (s->type & RDSPR_NOCOMPRESSION) {
		memcpy(*sprite, s->data, s->w * s->h);
	} else if (decompressRLE256(*sprite, s->data, s->w * s->h)) {
		free(*sprite);
		return RDERR_DECOMPRESSION;
	}

	return RD_OK;
}

/**
 * Draws the sprite surface created earlier.
 * @param s information about how to place the sprite
 * @param surface pointer to the surface created earlier
 * @param clipRect the clipping rectangle
 */

void Screen::drawSurface(SpriteInfo *s, byte *surface, Common::Rect *clipRect) {
	Common::Rect rd, rs;
	uint16 x, y;
	byte *src, *dst;

	rs.left = 0;
	rs.right = s->w;
	rs.top = 0;
	rs.bottom = s->h;

	rd.left = s->x;
	rd.right = rd.left + rs.right;
	rd.top = s->y;
	rd.bottom = rd.top + rs.bottom;

	Common::Rect defClipRect(0, 0, _screenWide, _screenDeep);

	if (!clipRect) {
		clipRect = &defClipRect;
	}

	if (clipRect->left > rd.left) {
		rs.left += (clipRect->left - rd.left);
		rd.left = clipRect->left;
	}

	if (clipRect->top > rd.top) {
		rs.top += (clipRect->top - rd.top);
		rd.top = clipRect->top;
	}

	if (clipRect->right < rd.right) {
		rd.right = clipRect->right;
	}

	if (clipRect->bottom < rd.bottom) {
		rd.bottom = clipRect->bottom;
	}

	if (rd.width() <= 0 || rd.height() <= 0)
		return;

	src = surface + rs.top * s->w + rs.left;
	dst = _buffer + _screenWide * rd.top + rd.left;

	// Surfaces are always transparent.

	for (y = 0; y < rd.height(); y++) {
		for (x = 0; x < rd.width(); x++) {
			if (src[x])
				dst[x] = src[x];
		}
		src += s->w;
		dst += _screenWide;
	}

	updateRect(&rd);
}

/**
 * Destroys a surface.
 */

void Screen::deleteSurface(byte *surface) {
	free(surface);
}

/**
 * Draws a sprite onto the screen. The type of the sprite can be a combination
 * of the following flags, some of which are mutually exclusive:
 * RDSPR_DISPLAYALIGN	The sprite is drawn relative to the top left corner
 *			of the screen
 * RDSPR_FLIP		The sprite is mirrored
 * RDSPR_TRANS		The sprite has a transparent color zero
 * RDSPR_BLEND		The sprite is translucent
 * RDSPR_SHADOW		The sprite is affected by the light mask. (Scaled
 *			sprites always are.)
 * RDSPR_NOCOMPRESSION	The sprite data is not compressed
 * RDSPR_RLE16		The sprite data is a 16-color compressed sprite
 * RDSPR_RLE256		The sprite data is a 256-color compressed sprite
 * @param s all the information needed to draw the sprite
 * @warning Sprites will only be drawn onto the background, not over menubar
 * areas.
 */

// FIXME: I'm sure this could be optimized. There's plenty of data copying and
// mallocing here.

int32 Screen::drawSprite(SpriteInfo *s) {
	byte *src, *dst;
	byte *sprite, *newSprite;
	uint16 scale;
	int16 i, j;
	uint16 srcPitch;
	bool freeSprite = false;
	Common::Rect rd, rs;

	// -----------------------------------------------------------------
	// Decompression and mirroring
	// -----------------------------------------------------------------
	if (s->type & RDSPR_NOCOMPRESSION) {
		if (Sword2Engine::isPsx()) { // PSX Uncompressed sprites
			if (s->w > 254 && !s->isText) { // We need to recompose these frames
				recomposePsxSprite(s);
			}

			// If the height is not an even value, fix it.
			// Apparently it's a problem in the data of a few sprites
			// of the PSX version. This should fix an evident problem
			// in the foyer at the beginning of the game, where a line
			// of pixels is missing near the stairs. But it should also
			// fix a more subtle one in the glease gallery and in quaramonte
			// police office.
			if (s->h % 2)
				s->h++;

			freeSprite = true;
			byte *tempBuf = (byte *)malloc(s->w * s->h * 2);
			memset(tempBuf, 0, s->w * s->h * 2);
			resizePsxSprite(tempBuf, s->data, s->w, s->h);

			if (s->w > 254 && !s->isText) {
				free(s->data);
			}

			sprite = tempBuf;
		} else { // PC Uncompressed sprites
			sprite = s->data;
		}
	} else {
		freeSprite = true;

		if ((s->type & 0xff00) == RDSPR_RLE16) {
			if (Sword2Engine::isPsx()) { // PSX HIF16 sprites
				uint32 decompData;
				byte *tempBuf = (byte *)malloc(s->w * s->h);
				memset(tempBuf, 0, s->w * s->h);

				decompData = decompressHIF(s->data, tempBuf);

				// Check that we correctly decompressed data
				if (!decompData) {
					free(tempBuf);

					return RDERR_DECOMPRESSION;
				}

				s->w = (decompData / (s->h / 2)) * 2;
				byte *tempBuf2 = (byte *)malloc(s->w * s->h * 10);
				memset(tempBuf2, 0, s->w * s->h * 2);

				unwindRaw16(tempBuf2, tempBuf, (s->w * (s->h / 2)), s->colorTable);
				sprite = (byte *)malloc(s->w * s->h);

				if (!sprite) {
					free(tempBuf2);
					free(tempBuf);
					return RDERR_OUTOFMEMORY;
				}

				resizePsxSprite(sprite, tempBuf2, s->w, s->h);

				free(tempBuf2);
				free(tempBuf);
			} else { // PC RLE16 sprites
				sprite = (byte *)malloc(s->w * s->h);

				if (!sprite)
					return RDERR_OUTOFMEMORY;

				if (decompressRLE16(sprite, s->data, s->w * s->h, s->colorTable)) {
					free(sprite);
					return RDERR_DECOMPRESSION;
				}
			}
		} else {
			if (Sword2Engine::isPsx()) { // PSX HIF256 sprites
				if (s->w > 255) {
					sprite = (byte *)malloc(s->w * s->h);
					recomposeCompPsxSprite(s);
					resizePsxSprite(sprite, s->data, s->w, s->h);
					free(s->data);
				} else {
					byte *tempBuf = (byte *)malloc(s->w * s->h);
					uint32 decompData = decompressHIF(s->data, tempBuf);

					// Check that we correctly decompressed data
					if (!decompData) {
						free(tempBuf);

						return RDERR_DECOMPRESSION;
					}

					s->w = (decompData / (s->h / 2));
					sprite = (byte *)malloc(s->w * s->h);

					if (!sprite)
						return RDERR_OUTOFMEMORY;

					resizePsxSprite(sprite, tempBuf, s->w, s->h);
					free(tempBuf);
				}
			} else { // PC RLE256 sprites
				sprite = (byte *)malloc(s->w * s->h);

				if (!sprite)
					return RDERR_OUTOFMEMORY;

				if (decompressRLE256(sprite, s->data, s->w * s->h)) {
					free(sprite);
					return RDERR_DECOMPRESSION;
				}
			}
		}
	}

	if (s->type & RDSPR_FLIP) {
		newSprite = (byte *)malloc(s->w * s->h);
		if (newSprite == NULL) {
			if (freeSprite)
				free(sprite);
			return RDERR_OUTOFMEMORY;
		}
		mirrorSprite(newSprite, sprite, s->w, s->h);
		if (freeSprite)
			free(sprite);
		sprite = newSprite;
		freeSprite = true;
	}

	// -----------------------------------------------------------------
	// Positioning and clipping.
	// -----------------------------------------------------------------

	int16 spriteX = s->x;
	int16 spriteY = s->y;

	if (!(s->type & RDSPR_DISPLAYALIGN)) {
		spriteX += _parallaxScrollX;
		spriteY += _parallaxScrollY;
	}

	spriteY += MENUDEEP;

	// A scale factor 0 or 256 means don't scale. Why do they use two
	// different values to mean the same thing? Normalize it here for
	// convenience.

	scale = (s->scale == 0) ? 256 : s->scale;

	rs.top = 0;
	rs.left = 0;

	if (scale != 256) {
		rs.right = s->scaledWidth;
		rs.bottom = s->scaledHeight;
		srcPitch = s->scaledWidth;
	} else {
		rs.right = s->w;
		rs.bottom = s->h;
		srcPitch = s->w;
	}

	rd.top = spriteY;
	rd.left = spriteX;

	if (!(s->type & RDSPR_DISPLAYALIGN)) {
		rd.top -= _scrollY;
		rd.left -= _scrollX;
	}

	rd.right = rd.left + rs.right;
	rd.bottom = rd.top + rs.bottom;

	// Check if the sprite would end up completely outside the screen.

	if (rd.left > RENDERWIDE || rd.top > RENDERDEEP + MENUDEEP || rd.right < 0 || rd.bottom < MENUDEEP) {
		if (freeSprite)
			free(sprite);
		return RD_OK;
	}

	if (rd.top < MENUDEEP) {
		rs.top = MENUDEEP - rd.top;
		rd.top = MENUDEEP;
	}
	if (rd.bottom > RENDERDEEP + MENUDEEP) {
		rd.bottom = RENDERDEEP + MENUDEEP;
		rs.bottom = rs.top + (rd.bottom - rd.top);
	}
	if (rd.left < 0) {
		rs.left = -rd.left;
		rd.left = 0;
	}
	if (rd.right > RENDERWIDE) {
		rd.right = RENDERWIDE;
		rs.right = rs.left + (rd.right - rd.left);
	}

	// -----------------------------------------------------------------
	// Scaling
	// -----------------------------------------------------------------

	if (scale != 256) {
		if (s->scaledWidth > SCALE_MAXWIDTH || s->scaledHeight > SCALE_MAXHEIGHT) {
			if (freeSprite)
				free(sprite);
			return RDERR_NOTIMPLEMENTED;
		}

		newSprite = (byte *)malloc(s->scaledWidth * s->scaledHeight);
		if (newSprite == NULL) {
			if (freeSprite)
				free(sprite);
			return RDERR_OUTOFMEMORY;
		}

		// We cannot use good scaling for PSX version, as we are missing
		// some required data.
		if (_renderCaps & RDBLTFX_EDGEBLEND && !Sword2Engine::isPsx())
			scaleImageGood(newSprite, s->scaledWidth, s->scaledWidth, s->scaledHeight, sprite, s->w, s->w, s->h, _buffer, rd.left, rd.top);
		else
			scaleImageFast(newSprite, s->scaledWidth, s->scaledWidth, s->scaledHeight, sprite, s->w, s->w, s->h);

		if (freeSprite)
			free(sprite);
		sprite = newSprite;
		freeSprite = true;
	}

	// -----------------------------------------------------------------
	// Light masking
	// -----------------------------------------------------------------

	// The light mask is an optional layer that covers the entire room
	// and which is used to simulate light and shadows. Scaled sprites
	// (actors, presumably) are always affected.
	// Light masking makes use of palette match table, so it's unavailable
	// in PSX version.

	if ((_renderCaps & RDBLTFX_SHADOWBLEND) && _lightMask && (scale != 256 || ((s->type & RDSPR_SHADOW) && !Sword2Engine::isPsx()) )) {
		byte *lightMap;

		// Make sure that we never apply the shadow to the original
		// resource data. This could only ever happen in the
		// RDSPR_NOCOMPRESSION case.

		if (!freeSprite) {
			newSprite = (byte *)malloc(s->w * s->h);
			memcpy(newSprite, sprite, s->w * s->h);
			sprite = newSprite;
			freeSprite = true;
		}

		src = sprite + rs.top * srcPitch + rs.left;
		lightMap = _lightMask + (rd.top + _scrollY - MENUDEEP) * _locationWide + rd.left + _scrollX;

		for (i = 0; i < rs.height(); i++) {
			for (j = 0; j < rs.width(); j++) {
				if (src[j] && lightMap[j]) {
					uint8 r = ((32 - lightMap[j]) * _palette[src[j] * 3 + 0]) >> 5;
					uint8 g = ((32 - lightMap[j]) * _palette[src[j] * 3 + 1]) >> 5;
					uint8 b = ((32 - lightMap[j]) * _palette[src[j] * 3 + 2]) >> 5;
					src[j] = quickMatch(r, g, b);
				}
			}
			src += srcPitch;
			lightMap += _locationWide;
		}
	}

	// -----------------------------------------------------------------
	// Drawing
	// -----------------------------------------------------------------

	src = sprite + rs.top * srcPitch + rs.left;
	dst = _buffer + _screenWide * rd.top + rd.left;

	if (s->type & RDSPR_BLEND) {
		// The original code had two different blending cases. One for
		// s->blend & 0x01 and one for s->blend & 0x02. However, the
		// only values that actually appear in the cluster files are
		// 0, 513 and 1025 so the s->blend & 0x02 case was never used.
		// Which is just as well since that code made no sense to me.

		// TODO: In PSX version, blending is done through hardware transparency.
		// The only correct way to simulate this would be using 16-bit mode.
		// As this is not yet available for this engine, fake transparency is used
		// as placeholder.
		if (!(_renderCaps & RDBLTFX_SPRITEBLEND) || Sword2Engine::isPsx()) {
			for (i = 0; i < rs.height(); i++) {
				for (j = 0; j < rs.width(); j++) {
					if (src[j] && ((i & 1) == (j & 1)))
						dst[j] = src[j];
				}
				src += srcPitch;
				dst += _screenWide;
			}
		} else {
			uint8 n = s->blend >> 8;

			for (i = 0; i < rs.height(); i++) {
				for (j = 0; j < rs.width(); j++) {
					if (src[j]) {
						uint8 r1 = _palette[src[j] * 3 + 0];
						uint8 g1 = _palette[src[j] * 3 + 1];
						uint8 b1 = _palette[src[j] * 3 + 2];
						uint8 r2 = _palette[dst[j] * 3 + 0];
						uint8 g2 = _palette[dst[j] * 3 + 1];
						uint8 b2 = _palette[dst[j] * 3 + 2];

						uint8 r = (r1 * n + r2 * (8 - n)) >> 3;
						uint8 g = (g1 * n + g2 * (8 - n)) >> 3;
						uint8 b = (b1 * n + b2 * (8 - n)) >> 3;
						dst[j] = quickMatch(r, g, b);
					}
				}
				src += srcPitch;
				dst += _screenWide;
			}
		}
	} else {
		if (s->type & RDSPR_TRANS) {
			for (i = 0; i < rs.height(); i++) {
				for (j = 0; j < rs.width(); j++) {
					if (src[j])
						dst[j] = src[j];
				}
				src += srcPitch;
				dst += _screenWide;
			}
		} else {
			for (i = 0; i < rs.height(); i++) {
				memcpy(dst, src, rs.width());
				src += srcPitch;
				dst += _screenWide;
			}
		}
	}

	if (freeSprite)
		free(sprite);

	markAsDirty(rd.left, rd.top, rd.right - 1, rd.bottom - 1);
	return RD_OK;
}

/**
 * Opens the light masking sprite for a room.
 */

int32 Screen::openLightMask(SpriteInfo *s) {
	// FIXME: The light mask is only needed on higher graphics detail
	// settings, so to save memory we could simply ignore it on lower
	// settings. But then we need to figure out how to ensure that it
	// is properly loaded if the user changes the settings in mid-game.

	if (_lightMask)
		return RDERR_NOTCLOSED;

	_lightMask = (byte *)malloc(s->w * s->h);
	if (!_lightMask)
		return RDERR_OUTOFMEMORY;

	if (s->data == NULL) // Check, as there's no mask in psx version
		return RDERR_NOTOPEN;

	if (decompressRLE256(_lightMask, s->data, s->w * s->h))
		return RDERR_DECOMPRESSION;

	return RD_OK;
}

/**
 * Closes the light masking sprite for a room.
 */

int32 Screen::closeLightMask() {
	if (!_lightMask)
		return RDERR_NOTOPEN;

	free(_lightMask);
	_lightMask = NULL;
	return RD_OK;
}

} // End of namespace Sword2
