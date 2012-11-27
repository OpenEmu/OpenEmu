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

#ifndef CGE_WALK_H
#define CGE_WALK_H

#include "common/rect.h"
#include "cge/vga13h.h"
#include "cge/events.h"

namespace CGE {

#define kMapArrSize    (kMapZCnt * kMapXCnt)
#define kMapHig        80
#define kMapGridX      (kScrWidth / kMapXCnt)
#define kMapGridZ      (kMapHig / kMapZCnt)
#define kMaxFindLevel  3

enum Dir { kDirNone = -1, kDirNorth, kDirEast, kDirSouth, kDirWest };

class Cluster {
public:
	CGEEngine *_vm;
	Common::Point _pt;
public:
	uint8 &cell();
	Cluster(CGEEngine *vm, int16 a, int16 b);
	Cluster(CGEEngine *vm);
	bool chkBar() const;
	bool isValid() const;
};

class Walk : public Sprite {
private:
	CGEEngine *_vm;
public:
	Cluster _here;
	int _tracePtr;
	int _level;
	int _findLevel;
	Common::Point _target;
	Common::Array<Cluster *> _trace;

	Dir _dir;
	Walk(CGEEngine *vm, BitmapPtr *shpl);
	~Walk();
	void tick();
	void findWay(Cluster c);
	void findWay(Sprite *spr);
	int distance(Sprite *spr);
	void turn(Dir d);
	void park();
	bool lower(Sprite *spr);
	void reach(Sprite *spr, int mode = -1);

	void noWay();
	bool find1Way(Cluster c);
};

} // End of namespace CGE

#endif
