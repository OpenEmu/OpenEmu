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

#include "cge/vmenu.h"
#include "cge/events.h"
#include "cge/cge_main.h"

namespace CGE {

MenuBar::MenuBar(CGEEngine *vm, uint16 w) : Talk(vm), _vm(vm) {
	int h = kFontHigh + 2 * kMenuBarVM;
	int i = (w += 2 * kMenuBarHM) * h;
	uint8 *p = (uint8 *)malloc(sizeof(uint8) * i);

	memset(p + w, kPixelTransp, i - 2 * w);
	memset(p, kMenuBarLT, w);
	memset(p + i - w, kMenuBarRB, w);
	uint8 *p1 = p;
	uint8 *p2 = p + i - 1;
	for (int cpt = 0; cpt < h; cpt++) {
		*p1 = kMenuBarLT;
		*p2 = kMenuBarRB;
		p1 += w;
		p2 -= w;
	}

	_ts = new BitmapPtr[2];
	_ts[0] = new Bitmap(_vm, w, h, p);
	_ts[1] = NULL;
	setShapeList(_ts);

	_flags._slav = true;
	_flags._tran = true;
	_flags._kill = true;
	_flags._bDel = true;
}

Vmenu *Vmenu::_addr = NULL;
int Vmenu::_recent = -1;

Vmenu::Vmenu(CGEEngine *vm, Choice *list, int x, int y)
	: Talk(vm, VMGather(list), kTBRect), _menu(list), _bar(NULL), _vmgt(NULL), _vm(vm) {
	Choice *cp;

	_addr = this;
	delete[] _vmgt;
	_items = 0;
	for (cp = list; cp->_text; cp++)
		_items++;
	_flags._bDel = true;
	_flags._kill = true;
	if (x < 0 || y < 0)
		center();
	else
		gotoxy(x - _w / 2, y - (kTextVMargin + kFontHigh / 2));
	_vm->_vga->_showQ->insert(this, _vm->_vga->_showQ->last());
	_bar = new MenuBar(_vm, _w - 2 * kTextHMargin);
	_bar->gotoxy(_x + kTextHMargin - kMenuBarHM, _y + kTextVMargin - kMenuBarVM);
	_vm->_vga->_showQ->insert(_bar, _vm->_vga->_showQ->last());
}

Vmenu::~Vmenu() {
	_addr = NULL;
}

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

void Vmenu::touch(uint16 mask, int x, int y, Common::KeyCode keyCode) {
	if (!_items)
		return;

	Sprite::touch(mask, x, y, keyCode);

	y -= kTextVMargin - 1;
	int n = 0;
	bool ok = false;
	uint16 h = kFontHigh + kTextLineSpace;

	if (y >= 0) {
		n = y / h;
		if (n < _items)
			ok = (x >= kTextHMargin && x < _w - kTextHMargin/* && y % h < FONT_HIG*/);
		else
			n = _items - 1;
	}

	_bar->gotoxy(_x + kTextHMargin - kMenuBarHM, _y + kTextVMargin + n * h - kMenuBarVM);

	if (ok && (mask & kMouseLeftUp)) {
		_items = 0;
		_vm->_commandHandlerTurbo->addCommand(kCmdKill, -1, 0, this);
		_recent = n;
		assert(_menu[n].Proc);
		CALL_MEMBER_FN(*_vm, _menu[n].Proc)();
	}
}

char *Vmenu::VMGather(Choice *list) {
	Choice *cp;
	int len = 0, h = 0;

	for (cp = list; cp->_text; cp++) {
		len += strlen(cp->_text);
		h++;
	}
	_vmgt = new char[len + h];
	if (_vmgt) {
		*_vmgt = '\0';
		for (cp = list; cp->_text; cp++) {
			if (*_vmgt)
				strcat(_vmgt, "|");
			strcat(_vmgt, cp->_text);
			h++;
		}
	}
	return _vmgt;
}
} // End of namespace CGE
