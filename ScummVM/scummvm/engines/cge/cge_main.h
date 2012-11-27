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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE_CGEMAIN_H
#define CGE_CGEMAIN_H

#include "cge/events.h"

namespace CGE {

#define kSceneX        4
#define kSceneY        166
#define kSceneSX       0
#define kSceneSY       0
#define kInfoX         177
#define kInfoY         164
#define kInfoW         140
#define kButtonX       151
#define kButtonY       164
#define kMiniX         86
#define kMiniY         162
#define kLineMax       512
#define kDistMax       3
#define kLgoExt        ".LGO"
#define kSvgExt        ".SVG"
#define kPaylistExt    ".X00"
#define kWinkExt       ".X01"
#define kIntroExt      ".X02"
#define kEndgExt       ".X03"
#define kWalkSide      10
#define kBusyRef       500
#define kSystemRate    6   // 12 Hz
#define kHeroFun0      (40 * 12)
#define kHeroFun1      ( 2 * 12)
#define kShowScummVMVersion 15
#define kTSeq          96
#define kBadSVG        99
#define kSeqHTalk      (kTSeq + 4)
#define kSeqTooFar     (kTSeq + 5)
#define kSeqNoWay      (kTSeq + 5)
#define kSeqPocketFull (kTSeq + 5)
#define kSeqOffUse     (kTSeq + 6)
#define kQuitTitle     200
#define kQuit          201
#define kNoQuit        202
#define kDemo          300
#define kOffUseCount   600
#define kOffUse        601
#define kNoWay         671
#define kTooFar        681
#define kPocketFull    691
#define kPanHeight     40
#define kScrWidth      320
#define kScrHeight     200
#define kWorldHeight   (kScrHeight - kPanHeight)
#define kStackSize     2048
#define kSavegameCheckSum   (1956 + _now + _oldLev + _game + _music + kDemo)
#define kSavegame0Name ("{{INIT}}" kSvgExt)
#define kSavegameStrSize 11
#define kGameFrameDelay (1000 / 50)
#define kGameTickDelay  (1000 / 62)

class System : public Sprite {
public:
	int _funDel;

	System(CGEEngine *vm);

	void setPal();
	void funTouch();
	virtual void touch(uint16 mask, int x, int y, Common::KeyCode keyCode);
	void tick();
private:
	CGEEngine *_vm;
};

class Square : public Sprite {
public:
	Square(CGEEngine *vm);
	virtual void touch(uint16 mask, int x, int y, Common::KeyCode keyCode);
private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
