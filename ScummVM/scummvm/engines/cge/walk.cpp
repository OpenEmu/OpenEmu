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

#include "cge/walk.h"
#include "cge/cge_main.h"

namespace CGE {

Cluster::Cluster(CGEEngine *vm, int16 a, int16 b) : _vm(vm) {
	_pt = Common::Point(a, b);
}

Cluster::Cluster(CGEEngine *vm) : _vm(vm) {
	_pt = Common::Point(-1, -1);
}

uint8 &Cluster::cell() {
	return _vm->_clusterMap[_pt.y][_pt.x];
}

bool Cluster::isValid() const {
	return (_pt.x >= 0) && (_pt.x < kMapXCnt) && (_pt.y >= 0) && (_pt.y < kMapZCnt);
}

Walk::Walk(CGEEngine *vm, BitmapPtr *shpl)
	: Sprite(vm, shpl), _dir(kDirNone), _tracePtr(-1), _level(0), _target(-1, -1), _findLevel(-1), _here(vm), _vm(vm) {
	for (int i = 0; i < kMaxFindLevel; i++) {
		Cluster *tmpClust = new Cluster(_vm);
		_trace.push_back(tmpClust);
	}
}

Walk::~Walk() {
	for (uint idx = 0; idx < _trace.size(); ++idx)
		delete _trace[idx];
}

void Walk::tick() {
	if (_flags._hide)
		return;

	_here = _vm->XZ(_x + _w / 2, _y + _h);

	if (_dir != kDirNone) {
		_vm->_sys->funTouch();
		for (Sprite *spr = _vm->_vga->_showQ->first(); spr; spr = spr->_next) {
			if (distance(spr) < 2) {
				if (!spr->_flags._near) {
					_vm->feedSnail(spr, kNear);
					spr->_flags._near = true;
				}
			} else {
				spr->_flags._near = false;
			}
		}
	}

	if (_flags._hold || _tracePtr < 0) {
		park();
	} else {
		if (_here._pt == _trace[_tracePtr]->_pt) {
			if (--_tracePtr < 0)
				park();
		} else {
			Common::Point tmpPoint = _trace[_tracePtr]->_pt - _here._pt;
			int16 dx = tmpPoint.x;
			int16 dz = tmpPoint.y;
			Dir d = (dx) ? ((dx > 0) ? kDirEast : kDirWest) : ((dz > 0) ? kDirSouth : kDirNorth);
			turn(d);
		}
	}

	step();

	if ((_dir == kDirWest  && _x <= 0) ||
	    (_dir == kDirEast  && _x + _w >= kScrWidth) ||
	    (_dir == kDirSouth && _y + _w >= kWorldHeight - 2)) {
		park();
	} else {
		// take current Z position
		_z = _here._pt.y;
		_vm->_commandHandlerTurbo->addCommand(kCmdZTrim, -1, 0, this);    // update Hero's pos in show queue
	}
}

int Walk::distance(Sprite *spr) {
	int dx = spr->_x - (_x + _w - kWalkSide);
	if (dx < 0)
		dx = (_x + kWalkSide) - (spr->_x + spr->_w);

	if (dx < 0)
		dx = 0;

	dx /= kMapGridX;
	int dz = spr->_z - _z;
	if (dz < 0)
		dz = - dz;

	dx = dx * dx + dz * dz;
	for (dz = 1; dz * dz < dx; dz++)
		;

	return dz - 1;
}
/**
 * Turn the character to a given direction
 * @param d			  Direction
 */
void Walk::turn(Dir d) {
	Dir dir = (_dir == kDirNone) ? kDirSouth : _dir;
	if (d != _dir) {
		step((d == dir) ? (1 + dir + dir) : (9 + 4 * dir + d));
		_dir = d;
	}
}

/**
 * Stop the character and reset his direction
 */
void Walk::park() {
	if (_time == 0)
		_time++;

	if (_dir != kDirNone) {
		step(9 + 4 * _dir + _dir);
		_dir = kDirNone;
		_tracePtr = -1;
	}
}

void Walk::findWay(Cluster c) {
	if (c._pt == _here._pt)
		return;

	for (_findLevel = 1; _findLevel <= kMaxFindLevel; _findLevel++) {
		_target = _here._pt;
		int16 x = c._pt.x;
		int16 z = c._pt.y;

		if (find1Way(Cluster(_vm, x, z)))
			break;
	}

	_tracePtr = (_findLevel > kMaxFindLevel) ? -1 : (_findLevel - 1);
	if (_tracePtr < 0)
		noWay();
	_time = 1;
}

void Walk::findWay(Sprite *spr) {
	if (!spr || spr == this)
		return;

	int x = spr->_x;
	int z = spr->_z;
	if (spr->_flags._east)
		x += spr->_w + _w / 2 - kWalkSide;
	else
		x -= _w / 2 - kWalkSide;

	findWay(Cluster(_vm, (x / kMapGridX),
	                ((z < kMapZCnt - kDistMax) ? (z + 1)
	                 : (z - 1))));
}

bool Walk::lower(Sprite *spr) {
	return (spr->_y > _y + (_h * 3) / 5);
}

void Walk::reach(Sprite *spr, int mode) {
	if (spr) {
		_vm->_hero->findWay(spr);
		if (mode < 0) {
			mode = spr->_flags._east;
			if (lower(spr))
				mode += 2;
		}
	}
	// note: insert SNAIL commands in reverse order
	_vm->_commandHandler->insertCommand(kCmdPause, -1, 64, NULL);
	_vm->_commandHandler->insertCommand(kCmdSeq, -1, kTSeq + mode, this);
	if (spr) {
		_vm->_commandHandler->insertCommand(kCmdWait, -1, -1, _vm->_hero);
		//SNINSERT(SNWALK, -1, -1, spr);
	}
	// sequence is not finished,
	// now it is just at sprite appear (disappear) point
}

void Walk::noWay() {
	_vm->trouble(kSeqNoWay, kNoWay);
}

bool Cluster::chkBar() const {
	assert(_vm->_now <= kSceneMax);
	return (_pt.x == _vm->_barriers[_vm->_now]._horz) || (_pt.y == _vm->_barriers[_vm->_now]._vert);
}

bool Walk::find1Way(Cluster c) {
	const Cluster tab[4] = { Cluster(_vm, -1, 0), Cluster(_vm, 1, 0), Cluster(_vm, 0, -1), Cluster(_vm, 0, 1)};
	const int tabLen = 4;

	if (c._pt == _target)
		// Found destination
		return true;

	if (_level >= _findLevel)
		// Nesting limit
		return false;

	// Look for barriers
	if (c.chkBar())
		return false;

	if (c.cell())
		// Location is occupied
		return false;

	// Loop through each direction
	Cluster start = c;
	for (int i = 0; i < tabLen; i++) {
		// Reset to starting position
		c = start;

		do {
			c._pt += tab[i]._pt;
			if (!c.isValid())
				// Break to check next direction
				break;

			// Recursively check for further paths
			++_level;
			++start.cell();
			bool foundPath = find1Way(c);
			--start.cell();
			--_level;

			if (foundPath) {
				// Set route point
				_trace[_level]->_pt = start._pt;
				return true;
			}
		} while (!c.chkBar() && !c.cell());
	}

	return false;
}

} // End of namespace CGE
