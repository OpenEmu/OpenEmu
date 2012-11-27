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

#include "cge/general.h"
#include "cge/text.h"
#include "cge/talk.h"
#include "cge/game.h"
#include "cge/snail.h"
#include "cge/cge_main.h"
#include "common/str.h"

namespace CGE {

Text::Text(CGEEngine *vm, const char *fname) : _vm(vm) {
	_vm->mergeExt(_fileName, fname, kSayExt);
	if (!_vm->_resman->exist(_fileName))
		error("No talk (%s)", _fileName);
	int16 txtCount = count() + 1;
	if (!txtCount)
		error("Unable to read dialog file %s", _fileName);

	_cache = new Handler[txtCount];
	for (_size = 0; _size < txtCount; _size++) {
		_cache[_size]._ref = 0;
		_cache[_size]._text = NULL;
	}
	load();
}

Text::~Text() {
	clear();
	delete[] _cache;
}

int16 Text::count() {
	EncryptedStream tf(_vm, _fileName);
	if (tf.err())
		return -1;

	Common::String line;
	char tmpStr[kLineMax + 1];

	int counter = 0;

	for (line = tf.readLine(); !tf.eos(); line = tf.readLine()) {
		char *s;

		strcpy(tmpStr, line.c_str());
		if ((s = strtok(tmpStr, " =,;/\t\n")) == NULL)
			continue;
		if (!Common::isDigit(*s))
			continue;

		counter++;
	}
	return counter;
}

void Text::clear() {
	for (Handler *p = _cache, *q = p + _size; p < q; p++) {
		if (p->_ref) {
			p->_ref = 0;
			delete[] p->_text;
			p->_text = NULL;
		}
	}
}

void Text::load() {
	EncryptedStream tf(_vm, _fileName);
	assert(!tf.err());

	Common::String line;
	char tmpStr[kLineMax + 1];
	int idx;

	for (idx = 0, line = tf.readLine(); !tf.eos(); line = tf.readLine()) {
		int n = line.size();
		char *s;

		strcpy(tmpStr, line.c_str());
		if ((s = strtok(tmpStr, " =,;/\t\n")) == NULL)
			continue;
		if (!Common::isDigit(*s))
			continue;

		int r = atoi(s);

		s += strlen(s);
		if (s < tmpStr + n)
			++s;

		_cache[idx]._ref = r;
		_cache[idx]._text = new char[strlen(s) + 1];
		strcpy(_cache[idx]._text, s);
		idx++;
	}
}

char *Text::getText(int ref) {
	int i;
	for (i = 0; (i < _size) && (_cache[i]._ref != ref); i++)
		;

	if (i < _size)
		return _cache[i]._text;

	warning("getText: Unable to find ref %d", ref);
	return NULL;
}

void Text::say(const char *text, Sprite *spr) {
	_vm->killText();

	if (!text)
		return;

	if (*text == 0)
		return;

	_vm->_talk = new Talk(_vm, text, kTBRound);
	if (!_vm->_talk)
		return;

	bool east = spr->_flags._east;
	int x = (east) ? (spr->_x + spr->_w - 2) : (spr->_x + 2);
	int y = spr->_y + 2;
	Speaker *speaker = new Speaker(_vm);
	uint16 sw = speaker->_w;

	if (east) {
		if (x + sw + kTextRoundCorner + 5 >= kScrWidth)
			east = false;
	} else {
		if (x <= 5 + kTextRoundCorner + sw)
			east = true;
	}
	x = (east) ? (spr->_x + spr->_w - 2) : (spr->_x + 2 - sw);
	if (spr->_ref == 1)
		x += ((east) ? -10 : 10); // Hero

	_vm->_talk->_flags._kill = true;
	_vm->_talk->_flags._bDel = true;
	_vm->_talk->setName(_vm->_text->getText(kSayName));
	_vm->_talk->gotoxy(x - (_vm->_talk->_w - sw) / 2 - 3 + 6 * east, y - speaker->_h - _vm->_talk->_h + 1);
	_vm->_talk->_z = 125;
	_vm->_talk->_ref = kSayRef;

	speaker->gotoxy(x, _vm->_talk->_y + _vm->_talk->_h - 1);
	speaker->_z = 126;
	speaker->_flags._slav = true;
	speaker->_flags._kill = true;
	speaker->setName(_vm->_text->getText(kSayName));
	speaker->step(east);
	speaker->_ref = kSayRef;

	_vm->_vga->_showQ->insert(_vm->_talk, _vm->_vga->_showQ->last());
	_vm->_vga->_showQ->insert(speaker, _vm->_vga->_showQ->last());
}

void CGEEngine::inf(const char *text, bool wideSpace) {
	debugC(1, kCGEDebugEngine, "CGEEngine::inf(%s)", text);
	if (!text)
		return;

	if (*text == 0)
		return;

	killText();
	_talk = new Talk(this, text, kTBRect, wideSpace);
	if (!_talk)
		return;

	_talk->_flags._kill = true;
	_talk->_flags._bDel = true;
	_talk->setName(_text->getText(kInfName));
	_talk->center();
	_talk->gotoxy(_talk->_x, _talk->_y - 20);
	_talk->_z = 126;
	_talk->_ref = kInfRef;
	_vga->_showQ->insert(_talk, _vga->_showQ->last());
}

void Text::sayTime(Sprite *spr) {
	TimeDate curTime;
	_vm->_system->getTimeAndDate(curTime);

	char t[6];
	sprintf(t, "%d:%02d", curTime.tm_hour, curTime.tm_min);
	say(t, spr);
}

} // End of namespace CGE
