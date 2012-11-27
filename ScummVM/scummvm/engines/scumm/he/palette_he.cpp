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

#ifdef ENABLE_HE

#include "common/system.h"
#include "graphics/palette.h"
#include "scumm/scumm.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/util.h"

namespace Scumm {

void ScummEngine_v71he::remapHEPalette(const uint8 *src, uint8 *dst) {
	int r, g, b, sum, bestitem, bestsum;
	int ar, ag, ab;
	uint8 *palPtr;
	src += 30;

	if (_game.heversion >= 99) {
		palPtr = _hePalettes + _hePaletteSlot + 30;
	} else {
		palPtr = _currentPalette + 30;
	}

	for (int j = 10; j < 246; j++) {
		bestitem = 0xFFFF;
		bestsum = 0xFFFF;

		r = *src++;
		g = *src++;
		b = *src++;

		uint8 *curPal = palPtr;

		for (int k = 10; k < 246; k++) {
			ar = r - *curPal++;
			ag = g - *curPal++;
			ab = b - *curPal++;

			sum = (ar * ar) + (ag * ag) + (ab * ab);

			if (bestitem == 0xFFFF || sum <= bestsum) {
				bestitem = k;
				bestsum = sum;
			}
		}

		dst[j] = bestitem;
	}
}

uint8 *ScummEngine_v90he::getHEPaletteIndex(int palSlot) {
	if (palSlot) {
		assert(palSlot >= 1 && palSlot <= _numPalettes);
		return _hePalettes + palSlot * _hePaletteSlot;
	} else {
		return _hePalettes + _hePaletteSlot;
	}
}

int ScummEngine_v90he::getHEPaletteSimilarColor(int palSlot, int red, int green, int start, int end) {
	assertRange(1, palSlot, _numPalettes, "palette");
	assertRange(0, start, 255, "start palette slot");
	assertRange(0, end, 255, "pend alette slot");

	uint8 *pal = _hePalettes + palSlot * _hePaletteSlot + start * 3;

	int bestsum = 0x7FFFFFFF;
	int bestitem = start;

	for (int i = start; i <= end; i++) {
		int dr = red - pal[0];
		int dg = green - pal[1];
		int sum = dr * dr + dg * dg * 2;
		if (sum == 0) {
			return i;
		}
		if (sum < bestsum) {
			bestsum = sum;
			bestitem = i;
		}
		pal += 3;
	}
	return bestitem;
}

int ScummEngine_v90he::getHEPalette16BitColorComponent(int component, int type) {
	uint16 col;
	if (type == 2) {
		col = (((component & 0xFFFF) >>  0) & 0x1F) << 3;
	} else if (type == 1) {
		col = (((component & 0xFFFF) >>  5) & 0x1F) << 3;
	} else {
		col = (((component & 0xFFFF) >> 10) & 0x1F) << 3;
	}
	return col;
}

int ScummEngine_v90he::getHEPaletteColorComponent(int palSlot, int color, int component) {
	assertRange(1, palSlot, _numPalettes, "palette");
	assertRange(0, color, 255, "palette slot");

	return _hePalettes[palSlot * _hePaletteSlot + color * 3 + component % 3];
}

int ScummEngine_v90he::getHEPaletteColor(int palSlot, int color) {
	assertRange(1, palSlot, _numPalettes, "palette");
	assertRange(0, color, 255, "palette slot");

	if (_game.features & GF_16BIT_COLOR)
		return READ_LE_UINT16(_hePalettes + palSlot * _hePaletteSlot + 768 + color * 2);
	else
		return _hePalettes[palSlot * _hePaletteSlot + 768 + color];
}

void ScummEngine_v90he::setHEPaletteColor(int palSlot, uint8 color, uint8 r, uint8 g, uint8 b) {
	debug(7, "setHEPaletteColor(%d, %d, %d, %d, %d)", palSlot, color, r, g, b);
	assertRange(1, palSlot, _numPalettes, "palette");

	uint8 *p = _hePalettes + palSlot * _hePaletteSlot + color * 3;
	*(p + 0) = r;
	*(p + 1) = g;
	*(p + 2) = b;
	if (_game.features & GF_16BIT_COLOR) {
		WRITE_LE_UINT16(_hePalettes + palSlot * _hePaletteSlot + 768 + color * 2, get16BitColor(r, g, b));
	} else {
		_hePalettes[palSlot * _hePaletteSlot + 768 + color] = color;
	}
}

void ScummEngine_v90he::setHEPaletteFromPtr(int palSlot, const uint8 *palData) {
	assertRange(1, palSlot, _numPalettes, "palette");

	uint8 *pc = _hePalettes + palSlot * _hePaletteSlot;
	uint8 *pi = pc + 768;
	if (_game.features & GF_16BIT_COLOR) {
		for (int i = 0; i < 256; ++i) {
			uint8 r = *pc++ = *palData++;
			uint8 g = *pc++ = *palData++;
			uint8 b = *pc++ = *palData++;
			WRITE_LE_UINT16(pi, get16BitColor(r, g, b)); pi += 2;
		}
	} else {
		for (int i = 0; i < 256; ++i) {
			*pc++ = *palData++;
			*pc++ = *palData++;
			*pc++ = *palData++;
			*pi++ = i;
		}
	}

	int i;
	uint8 *palPtr = _hePalettes + palSlot * _hePaletteSlot + 768;
	if (_game.features & GF_16BIT_COLOR) {
		for (i = 0; i < 10; ++i)
			WRITE_LE_UINT16(palPtr + i * 2, i);
		for (i = 246; i < 256; ++i)
			WRITE_LE_UINT16(palPtr + i * 2, i);
	} else {
		for (i = 0; i < 10; ++i)
			*(palPtr + i) = i;
		for (i = 246; i < 256; ++i)
			*(palPtr + i) = i;
	}
}

void ScummEngine_v90he::setHEPaletteFromCostume(int palSlot, int resId) {
	debug(7, "setHEPaletteFromCostume(%d, %d)", palSlot, resId);
	assertRange(1, palSlot, _numPalettes, "palette");
	const uint8 *data = getResourceAddress(rtCostume, resId);
	assert(data);
	const uint8 *rgbs = findResourceData(MKTAG('R','G','B','S'), data);
	assert(rgbs);
	setHEPaletteFromPtr(palSlot, rgbs);
}

void ScummEngine_v90he::setHEPaletteFromImage(int palSlot, int resId, int state) {
	debug(7, "setHEPaletteFromImage(%d, %d, %d)", palSlot, resId, state);
	assertRange(1, palSlot, _numPalettes, "palette");
	uint8 *data = getResourceAddress(rtImage, resId);
	assert(data);
	const uint8 *rgbs = findWrappedBlock(MKTAG('R','G','B','S'), data, state, 0);
	if (rgbs)
		setHEPaletteFromPtr(palSlot, rgbs);
}

void ScummEngine_v90he::setHEPaletteFromRoom(int palSlot, int resId, int state) {
	debug(7, "setHEPaletteFromRoom(%d, %d, %d)", palSlot, resId, state);
	assertRange(1, palSlot, _numPalettes, "palette");
	const uint8 *data = getResourceAddress(rtRoom, resId);
	assert(data);
	const uint8 *pals = findResourceData(MKTAG('P','A','L','S'), data);
	assert(pals);
	const uint8 *rgbs = findPalInPals(pals, state);
	assert(rgbs);
	setHEPaletteFromPtr(palSlot, rgbs);
}

void ScummEngine_v90he::restoreHEPalette(int palSlot) {
	debug(7, "restoreHEPalette(%d)", palSlot);
	assertRange(1, palSlot, _numPalettes, "palette");

	if (palSlot != 1) {
		memcpy(_hePalettes + palSlot * _hePaletteSlot, _hePalettes + _hePaletteSlot, _hePaletteSlot);
	}
}

void ScummEngine_v90he::copyHEPalette(int dstPalSlot, int srcPalSlot) {
	debug(7, "copyHEPalette(%d, %d)", dstPalSlot, srcPalSlot);
	assert(dstPalSlot >= 1 && dstPalSlot <= _numPalettes);
	assert(srcPalSlot >= 1 && srcPalSlot <= _numPalettes);

	if (dstPalSlot != srcPalSlot) {
		memcpy(_hePalettes + dstPalSlot * _hePaletteSlot, _hePalettes + srcPalSlot * _hePaletteSlot, _hePaletteSlot);
	}
}

void ScummEngine_v90he::copyHEPaletteColor(int palSlot, uint8 dstColor, uint16 srcColor) {
	debug(7, "copyHEPaletteColor(%d, %d, %d)", palSlot, dstColor, srcColor);
	assertRange(1, palSlot, _numPalettes, "palette");

	uint8 *dstPal = _hePalettes + palSlot * _hePaletteSlot + dstColor * 3;
	uint8 *srcPal = _hePalettes + _hePaletteSlot + srcColor * 3;
	if (_game.features & GF_16BIT_COLOR) {
		dstPal[0] = (srcColor >> 10) << 3;
		dstPal[1] = (srcColor >>  5) << 3;
		dstPal[2] = (srcColor >>  0) << 3;
		WRITE_LE_UINT16(_hePalettes + palSlot * _hePaletteSlot + 768 + dstColor * 2, srcColor);
	} else {
		memcpy(dstPal, srcPal, 3);
		_hePalettes[palSlot * _hePaletteSlot + 768 + dstColor] = srcColor;
	}
}

void ScummEngine_v99he::setPaletteFromPtr(const byte *ptr, int numcolor) {
	int i;
	byte *dest, r, g, b;

	if (numcolor < 0) {
		numcolor = getResourceDataSize(ptr) / 3;
	}

	assertRange(0, numcolor, 256, "setPaletteFromPtr: numcolor");

	dest = _hePalettes + _hePaletteSlot;

	for (i = 0; i < numcolor; i++) {
		r = *ptr++;
		g = *ptr++;
		b = *ptr++;

		if (i == 15 || r < 252 || g < 252 || b < 252) {
			*dest++ = r;
			*dest++ = g;
			*dest++ = b;

			if (_game.features & GF_16BIT_COLOR) {
				WRITE_LE_UINT16(_hePalettes + 2048 + i * 2, get16BitColor(r, g, b));
			} else {
				_hePalettes[1792 + i] = i;
			}
		} else {
			dest += 3;
		}
	}

	memcpy(_hePalettes, _hePalettes + _hePaletteSlot, 768);

	if (_game.features & GF_16BIT_COLOR) {
		for (i = 0; i < 10; ++i)
			WRITE_LE_UINT16(_hePalettes + 2048 + i * 2, i);
		for (i = 246; i < 256; ++i)
			WRITE_LE_UINT16(_hePalettes + 2048 + i * 2, i);
	} else {
		for (i = 0; i < 10; ++i)
			_hePalettes[1792 + i] = i;
		for (i = 246; i < 256; ++i)
			_hePalettes[1792 + i] = i;
	}
	setDirtyColors(0, numcolor - 1);
}

void ScummEngine_v99he::darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor) {
	uint8 *src, *dst;
	int j, r, g, b;

	src = _hePalettes + startColor * 3;
	dst = _hePalettes + _hePaletteSlot + startColor * 3;
	for (j = startColor; j <= endColor; j++) {
		r = *src++;
		r = r * redScale / 0xFF;
		if (r > 255)
			r = 255;
		*dst++ = r;

		g = *src++;
		g = g * greenScale / 0xFF;
		if (g > 255)
			g = 255;
		*dst++ = g;

		b = *src++;
		b = b * blueScale / 0xFF;
		if (b > 255)
			b = 255;
		*dst++ = b;

		if (_game.features & GF_16BIT_COLOR) {
			WRITE_LE_UINT16(_hePalettes + 2048 + j * 2, get16BitColor(r, g, b));
		} else {
			_hePalettes[1792 + j] = j;
		}
		setDirtyColors(j, endColor);
	}
}

void ScummEngine_v99he::copyPalColor(int dst, int src) {
	byte *dp, *sp;

	if ((uint) dst >= 256 || (uint) src >= 256)
		error("copyPalColor: invalid values, %d, %d", dst, src);

	dp = &_hePalettes[_hePaletteSlot + dst * 3];
	sp = &_hePalettes[_hePaletteSlot + src * 3];

	dp[0] = sp[0];
	dp[1] = sp[1];
	dp[2] = sp[2];

	if (_game.features & GF_16BIT_COLOR) {
		WRITE_LE_UINT16(_hePalettes + 2048 + dst * 2, get16BitColor(sp[0], sp[1], sp[2]));
	} else {
		_hePalettes[1792 + dst] = dst;
	}

	setDirtyColors(dst, dst);
}

void ScummEngine_v99he::setPalColor(int idx, int r, int g, int b) {
	_hePalettes[_hePaletteSlot + idx * 3 + 0] = r;
	_hePalettes[_hePaletteSlot + idx * 3 + 1] = g;
	_hePalettes[_hePaletteSlot + idx * 3 + 2] = b;

	if (_game.features & GF_16BIT_COLOR) {
		WRITE_LE_UINT16(_hePalettes + 2048 + idx * 2, get16BitColor(r, g, b));
	} else {
		_hePalettes[1792 + idx] = idx;
	}
	setDirtyColors(idx, idx);
}

void ScummEngine_v99he::updatePalette() {
	if (_game.features & GF_16BIT_COLOR)
		return;

	if (_palDirtyMax == -1)
		return;

	int num = _palDirtyMax - _palDirtyMin + 1;

	_system->getPaletteManager()->setPalette(_hePalettes + 1024 + _palDirtyMin * 3, _palDirtyMin, num);

	_palDirtyMax = -1;
	_palDirtyMin = 256;
}

} // End of namespace Scumm

#endif // ENABLE_HE
