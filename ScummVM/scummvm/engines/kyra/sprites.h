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

#ifndef KYRA_SPRITES_H
#define KYRA_SPRITES_H

#include "kyra/kyra_lok.h"
#include "common/random.h"

namespace Kyra {

#define MAX_NUM_ANIMS 11

struct Sprite {
	uint16 x;
	uint16 y;
	uint16 width;
	uint16 height;
};

struct Anim {
	uint8 *script;
	uint8 *curPos;
	uint16 length;
	int16 x;
	int16 y;
	bool flipX;
	int8 sprite;
	uint8 *loopStart;
	uint16 loopsLeft;
	uint32 nextRun;
	uint32 lastRefresh;
	bool play;
	uint16 width;
	uint16 height;
	uint16 width2;
	uint16 height2;
	uint16 unk1;
	uint16 drawY;
	uint16 unk2;
	uint8 *background;
	bool disable;
};

class KyraEngine_LoK;

class Sprites {
public:
	Sprites(KyraEngine_LoK *vm, OSystem *system);
	~Sprites();

	void updateSceneAnims();
	void setupSceneAnims();
	void loadDat(const char *filename, SceneExits &exits);
	void loadSceneShapes();

	Anim _anims[MAX_NUM_ANIMS];
	uint8 *_sceneShapes[50];

	void refreshSceneAnimObject(uint8 animNum, uint8 shapeNum, uint16 x, uint16 y, bool flipX, bool unkFlag);

	int getDrawLayer(int y);

	int _sceneAnimatorBeaconFlag;
protected:
	void freeSceneShapes();

	KyraEngine_LoK *_vm;
	Resource *_res;
	OSystem *_system;
	Screen *_screen;
	uint8 *_dat;
	Common::RandomSource _rnd;
	uint8 *_spriteDefStart;
	uint8 _drawLayerTable[8];
};

} // End of namespace Kyra

#endif
