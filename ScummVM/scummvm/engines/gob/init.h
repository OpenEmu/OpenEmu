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

#ifndef GOB_INIT_H
#define GOB_INIT_H

#include "gob/gob.h"
#include "gob/video.h"

namespace Gob {

class Init {
public:
	Init(GobEngine *vm);
	virtual ~Init();

	virtual void initGame();

	virtual void initVideo() = 0;

	virtual void updateConfig();

protected:
	Video::PalDesc *_palDesc;
	static const char *const _fontNames[4];
	GobEngine *_vm;

	void cleanup();
	void doDemo();
};

class Init_v1 : public Init {
public:
	Init_v1(GobEngine *vm);
	~Init_v1();

	void initVideo();
};

class Init_Geisha : public Init_v1 {
public:
	Init_Geisha(GobEngine *vm);
	~Init_Geisha();

	void initVideo();
};

class Init_v2 : public Init_v1 {
public:
	Init_v2(GobEngine *vm);
	~Init_v2();

	void initVideo();
};

class Init_v3 : public Init_v2 {
public:
	Init_v3(GobEngine *vm);
	~Init_v3();

	void initVideo();
	void updateConfig();
};

class Init_v4 : public Init_v3 {
public:
	Init_v4(GobEngine *vm);
	~Init_v4();

	void updateConfig();
};

class Init_v6 : public Init_v3 {
public:
	Init_v6(GobEngine *vm);
	~Init_v6();

	void initGame();
};

class Init_Fascination : public Init_v2 {
public:
	Init_Fascination(GobEngine *vm);
	~Init_Fascination();

	void updateConfig();
	void initGame();
};

class Init_v7 : public Init_v2 {
public:
	Init_v7(GobEngine *vm);
	~Init_v7();

	void initGame();
};

} // End of namespace Gob

#endif // GOB_INIT_H
