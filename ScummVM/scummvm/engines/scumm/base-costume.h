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

#ifndef SCUMM_BASE_COSTUME_H
#define SCUMM_BASE_COSTUME_H

#include "common/scummsys.h"
#include "scumm/actor.h"		// for CostumeData

namespace Scumm {

#include "common/pack-start.h"	// START STRUCT PACKING

struct CostumeInfo {
	uint16 width, height;
	int16 rel_x, rel_y;
	int16 move_x, move_y;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


extern const byte smallCostumeScaleTable[256];
extern const byte bigCostumeScaleTable[768];


class Actor;
class ScummEngine;
struct VirtScreen;

class BaseCostumeLoader {
protected:
	ScummEngine *_vm;

public:
	BaseCostumeLoader(ScummEngine *vm) : _vm(vm) {}
	virtual ~BaseCostumeLoader() {}

	virtual void loadCostume(int id) = 0;
	virtual byte increaseAnims(Actor *a) = 0;
	virtual void costumeDecodeData(Actor *a, int frame, uint usemask) = 0;

	bool hasManyDirections(int id) { return false; }
};


/**
 * Base class for both ClassicCostumeRenderer and AkosRenderer.
 */
class BaseCostumeRenderer {
public:
	Common::Rect _clipOverride;
	byte _actorID;

	byte _shadow_mode;
	byte *_shadow_table;

	int _actorX, _actorY;
	byte _zbuf;
	byte _scaleX, _scaleY;

	int _draw_top, _draw_bottom;
	byte _paletteNum;
	bool _skipLimbs;
	bool _actorDrawVirScr;


protected:
	ScummEngine *_vm;

	// Destination
	Graphics::Surface _out;
	int32 _numStrips;

	// Source pointer
	const byte *_srcptr;

	// current move offset
	int _xmove, _ymove;

	// whether to draw the actor mirrored
	bool _mirror;

	int _numBlocks;

	// width and height of cel to decode
	int _width, _height;

public:
	struct Codec1 {
		// Parameters for the original ("V1") costume codec.
		// These ones are accessed from ARM code. Don't reorder.
		int x;
		int y;
		const byte *scaletable;
		int skip_width;
		byte *destptr;
		const byte *mask_ptr;
		int scaleXstep;
		byte mask, shr;
		byte repcolor;
		byte replen;
		// These ones aren't accessed from ARM code.
		Common::Rect boundsRect;
		int scaleXindex, scaleYindex;
	};

	BaseCostumeRenderer(ScummEngine *scumm) {
		_actorID = 0;
		_shadow_mode = 0;
		_shadow_table = 0;
		_actorX = _actorY = 0;
		_zbuf = 0;
		_scaleX = _scaleY = 0;
		_draw_top = _draw_bottom = 0;

		_vm = scumm;
		_numStrips = -1;
		_srcptr = 0;
		_xmove = _ymove = 0;
		_mirror = false;
		_width = _height = 0;
		_skipLimbs = 0;
		_paletteNum = 0;
	}
	virtual ~BaseCostumeRenderer() {}

	virtual void setPalette(uint16 *palette) = 0;
	virtual void setFacing(const Actor *a) = 0;
	virtual void setCostume(int costume, int shadow) = 0;


	byte drawCostume(const VirtScreen &vs, int numStrips, const Actor *a, bool drawToBackBuf);

protected:
	virtual byte drawLimb(const Actor *a, int limb) = 0;

	void codec1_ignorePakCols(Codec1 &v1, int num);
};

} // End of namespace Scumm

#endif
