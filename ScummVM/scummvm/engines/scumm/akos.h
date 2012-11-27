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

#ifndef SCUMM_AKOS_H
#define SCUMM_AKOS_H

#include "scumm/base-costume.h"

namespace Scumm {

struct CostumeData;
struct AkosHeader;
struct AkosOffset;

class AkosCostumeLoader : public BaseCostumeLoader {
protected:
	const byte *_akos;

public:
	AkosCostumeLoader(ScummEngine *vm) : BaseCostumeLoader(vm) {}

	void loadCostume(int id);
	byte increaseAnims(Actor *a);
	void costumeDecodeData(Actor *a, int frame, uint usemask);

	//void animateLimb(int limb, int f);
	bool hasManyDirections(int id) {
		loadCostume(id);
		return hasManyDirections();
	}

protected:
	bool hasManyDirections();
};

class AkosRenderer : public BaseCostumeRenderer {
protected:
	uint16 _codec;

	// actor _palette
	uint16 _palette[256];
	bool _useBompPalette;

	// pointer to various parts of the costume resource
	const AkosHeader *akhd;	// header

	const byte *akpl;		// palette data
	const byte *akci;		// CostumeInfo table
	const byte *aksq;		// command sequence
	const AkosOffset *akof;	// offsets into ci and cd table
	const byte *akcd;		// costume data (contains the data for the codecs)

	const byte *akct;		// HE specific: condition table
	const byte *rgbs;		// HE specific: RGB table
	const uint8 *xmap;		// HE specific: shadow color table

	struct {
		bool repeatMode;
		int repeatCount;
		byte mask;
		byte color;
		byte shift;
		uint16 bits;
		byte numbits;
		const byte *dataptr;
		byte buffer[336];
	} _akos16;

public:
	AkosRenderer(ScummEngine *scumm) : BaseCostumeRenderer(scumm) {
		_useBompPalette = false;
		akhd = 0;
		akpl = 0;
		akci = 0;
		aksq = 0;
		akof = 0;
		akcd = 0;
		akct = 0;
		rgbs = 0;
		xmap = 0;
		_actorHitMode = false;
	}

	bool _actorHitMode;
	int16 _actorHitX, _actorHitY;
	bool _actorHitResult;

	void setPalette(uint16 *_palette);
	void setFacing(const Actor *a);
	void setCostume(int costume, int shadow);

protected:
	byte drawLimb(const Actor *a, int limb);

	byte codec1(int xmoveCur, int ymoveCur);
	void codec1_genericDecode(Codec1 &v1);
	byte codec5(int xmoveCur, int ymoveCur);
	byte codec16(int xmoveCur, int ymoveCur);
	byte codec32(int xmoveCur, int ymoveCur);
	void akos16SetupBitReader(const byte *src);
	void akos16SkipData(int32 numskip);
	void akos16DecodeLine(byte *buf, int32 numbytes, int32 dir);
	void akos16Decompress(byte *dest, int32 pitch, const byte *src, int32 t_width, int32 t_height, int32 dir, int32 numskip_before, int32 numskip_after, byte transparency, int maskLeft, int maskTop, int zBuf);

	void markRectAsDirty(Common::Rect rect);
};

} // End of namespace Scumm

#endif
