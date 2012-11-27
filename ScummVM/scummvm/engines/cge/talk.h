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

#ifndef CGE_TALK_H
#define CGE_TALK_H

#include "cge/general.h"
#include "cge/vga13h.h"

namespace CGE {

#define kTextColFG         kVgaColDark              // foreground color
#define kTextColBG         kVgaColGray              // background color
#define kTextHMargin      (6&~1)                    // EVEN horizontal margins!
#define kTextVMargin       5                        // vertical margins
#define kTextLineSpace     2                        // line spacing
#define kTextRoundCorner   3                        // rounded corners
#define kWidSize           256
#define kPosSize           256
#define kMapSize          (256*8)
#define kFontHigh          8
#define kFontExt           ".CFT"

enum TextBoxStyle { kTBPure, kTBRect, kTBRound };

class Talk : public Sprite {
protected:
	TextBoxStyle _mode;
	BitmapPtr *_ts;
	Bitmap *box(uint16 w, uint16 h);
	bool _wideSpace;
public:
	Talk(CGEEngine *vm, const char *text, TextBoxStyle mode, bool wideSpace = false);
	Talk(CGEEngine *vm);

	virtual void update(const char *text);
private:
	CGEEngine *_vm;
};

class InfoLine : public Talk {
	const char *_oldText;
public:
	InfoLine(CGEEngine *vm, uint16 wid);
	void update(const char *text);
private:
	CGEEngine *_vm;
};

} // End of namespace CGE

#endif
