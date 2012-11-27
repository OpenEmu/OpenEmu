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

#ifndef CGE_VMENU_H
#define CGE_VMENU_H

#include "cge/talk.h"

namespace CGE {

#define kMenuBarVM       1
#define kMenuBarHM       3
#define kMenuBarLT       kVgaColLightGray
#define kMenuBarRB       kVgaColDarkGray


struct Choice {
	const char *_text;
	void (CGEEngine::*Proc)();
};


class MenuBar : public Talk {
public:
	MenuBar(CGEEngine *vm, uint16 w);
private:
	CGEEngine *_vm;
};

class Vmenu : public Talk {
public:
	static Vmenu *_addr;
	static int _recent;
	MenuBar *_bar;
	Vmenu(CGEEngine *vm, Choice *list, int x, int y);
	~Vmenu();
	virtual void touch(uint16 mask, int x, int y, Common::KeyCode keyCode);
private:
	char *_vmgt;
	CGEEngine *_vm;
	uint16 _items;
	Choice *_menu;

	char *VMGather(Choice *list);
};

} // End of namespace CGE

#endif
