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

#ifndef CGE_TEXT_H
#define CGE_TEXT_H

#include "cge/talk.h"

namespace CGE {

#define kSayExt         ".SAY"
#define kSysTextMax     1000
#define kTextNoMouse    95
#define kInfName        101
#define kSayName        102
#define kInfRef         301
#define kSayRef         302


class Text {
	struct Handler {
		int _ref;
		char *_text;
	} *_cache;
	int _size;
	char _fileName[kPathMax];
	void load();
	int16 count();
public:
	Text(CGEEngine *vm, const char *fname);
	~Text();
	void clear();
	char *getText(int ref);
	void say(const char *text, Sprite *spr);
	void sayTime(Sprite *spr);
private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
