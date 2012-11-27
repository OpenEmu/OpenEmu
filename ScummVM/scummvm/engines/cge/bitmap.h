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

#ifndef CGE_BITMAP_H
#define CGE_BITMAP_H

#include "cge/general.h"
#include "common/file.h"

namespace CGE {

class CGEEngine;
class EncryptedStream;

#define kMaxPath  128
enum {
	kBmpEOI = 0x0000,
	kBmpSKP = 0x4000,
	kBmpREP = 0x8000,
	kBmpCPY = 0xC000
};

#include "common/pack-start.h"

struct HideDesc {
	uint16 _skip;
	uint16 _hide;
} PACKED_STRUCT;

#include "common/pack-end.h"

class Bitmap {
	CGEEngine *_vm;
	char *forceExt(char *buf, const char *name, const char *ext);
	bool loadVBM(EncryptedStream *f);
public:
	uint16 _w;
	uint16 _h;
	uint8 *_m;
	uint8 *_v;
	int32 _map;
	HideDesc *_b;

	Bitmap(CGEEngine *vm, const char *fname);
	Bitmap(CGEEngine *vm, uint16 w, uint16 h, uint8 *map);
	Bitmap(CGEEngine *vm, uint16 w, uint16 h, uint8 fill);
	Bitmap(CGEEngine *vm, const Bitmap &bmp);
	~Bitmap();

	Bitmap *code();
	Bitmap &operator=(const Bitmap &bmp);
	void hide(int16 x, int16 y);
	void show(int16 x, int16 y);
	void xShow(int16 x, int16 y);
	bool solidAt(int16 x, int16 y);
};


typedef Bitmap *BitmapPtr;

} // End of namespace CGE

#endif
