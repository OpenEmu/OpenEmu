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

#ifndef CGE_VGA13H_H
#define CGE_VGA13H_H

#include "common/serializer.h"
#include "common/events.h"
#include "graphics/surface.h"
#include "cge/general.h"
#include "cge/bitmap.h"
#include "cge/snail.h"
#include "cge/cge.h"

namespace CGE {

#define kFadeStep        2
#define kVgaColDark      207
#define kVgaColDarkGray  225 /*219*/
#define kVgaColGray      231
#define kVgaColLightGray 237
#define kPixelTransp     0xFE
#define kNoSeq           (-1)
#define kNoPtr           ((uint8)-1)
#define kSprExt          ".SPR"
#define kPalCount        256
#define kPalSize         (kPalCount * 3)


struct Seq {
	uint8 _now;
	uint8 _next;
	int8 _dx;
	int8 _dy;
	int _dly;
};

class SprExt {
public:
	int _x0;
	int _y0;
	int _x1;
	int _y1;
	BitmapPtr _b0;
	BitmapPtr _b1;
	BitmapPtr *_shpList;
	Seq *_seq;
	char *_name;
	CommandHandler::Command *_near;
	CommandHandler::Command *_take;
	SprExt() :
		_x0(0), _y0(0),
		_x1(0), _y1(0),
		_b0(NULL), _b1(NULL),
		_shpList(NULL), _seq(NULL),
		_name(NULL), _near(NULL), _take(NULL)
	{}
};

class Sprite {
protected:
	SprExt *_ext;
public:
	int _ref;
	signed char _scene;
	struct Flags {
		uint16 _hide : 1;       // general visibility switch
		uint16 _near : 1;       // Near action lock
		uint16 _drag : 1;       // sprite is moveable
		uint16 _hold : 1;       // sprite is held with mouse
		uint16 _dummy : 1;       // intrrupt driven animation
		uint16 _slav : 1;       // slave object
		uint16 _syst : 1;       // system object
		uint16 _kill : 1;       // dispose memory after remove
		uint16 _xlat : 1;       // 2nd way display: xlat table
		uint16 _port : 1;       // portable
		uint16 _kept : 1;       // kept in pocket
		uint16 _east : 1;       // talk to east (in opposite to west)
		uint16 _shad : 1;       // shadow
		uint16 _back : 1;       // 'send to background' request
		uint16 _bDel : 1;       // delete bitmaps in ~SPRITE
		uint16 _tran : 1;       // transparent (untouchable)
	} _flags;
	int _x;
	int _y;
	signed char _z;
	uint16 _w;
	uint16 _h;
	uint16 _time;
	uint8 _nearPtr;
	uint8 _takePtr;
	int _seqPtr;
	int _shpCnt;
	char _file[kMaxFile];
	Sprite *_prev;
	Sprite *_next;

	bool works(Sprite *spr);
	bool seqTest(int n);
	inline bool active() {
		return _ext != NULL;
	}

	Sprite(CGEEngine *vm, BitmapPtr *shp);
	virtual ~Sprite();
	BitmapPtr shp();
	BitmapPtr *setShapeList(BitmapPtr *shp);
	Sprite *expand();
	Sprite *contract();
	Sprite *backShow(bool fast = false);
	void setName(char *newName);
	inline char *name() {
		return (_ext) ? _ext->_name : NULL;
	}
	void gotoxy(int x, int y);
	void center();
	void show();
	void hide();
	BitmapPtr ghost();
	void show(uint16 pg);
	void makeXlat(uint8 *x);
	void killXlat();
	void step(int nr = -1);
	Seq *setSeq(Seq *seq);
	CommandHandler::Command *snList(SnList type);
	virtual void touch(uint16 mask, int x, int y, Common::KeyCode keyCode);
	virtual void tick();
	void sync(Common::Serializer &s);
private:
	CGEEngine *_vm;
};

class Queue {
	Sprite *_head;
	Sprite *_tail;
public:
	Queue(bool show);
	~Queue();

	bool _show;

	void append(Sprite *spr);
	void insert(Sprite *spr, Sprite *nxt);
	void insert(Sprite *spr);
	Sprite *remove(Sprite *spr);
	Sprite *first() {
		return _head;
	}
	Sprite *last() {
		return _tail;
	}
	Sprite *locate(int ref);
	void clear();
};

class Vga {
	CGEEngine *_vm;
	bool _setPal;
	Dac *_oldColors;
	Dac *_newColors;
	const char *_msg;
	const char *_name;

	void updateColors();
	void setColors();
	void waitVR();
	uint8 closest(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB);

public:
	uint32 _frmCnt;
	Queue *_showQ;
	Queue *_spareQ;
	int _mono;
	Graphics::Surface *_page[4];
	Dac *_sysPal;

	Vga(CGEEngine *vm);
	~Vga();

	uint8 *glass(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB);
	void getColors(Dac *tab);
	void setColors(Dac *tab, int lum);
	void clear(uint8 color);
	void copyPage(uint16 d, uint16 s);
	void sunrise(Dac *tab);
	void sunset();
	void show();
	void update();

	void palToDac(const byte *palData, Dac *tab);
	void dacToPal(const Dac *tab, byte *palData);
};

class HorizLine: public Sprite {
	CGEEngine *_vm;
public:
	HorizLine(CGEEngine *vm);
};

class SceneLight: public Sprite {
	CGEEngine *_vm;
public:
	SceneLight(CGEEngine *vm);
};

class Speaker: public Sprite {
	CGEEngine *_vm;
public:
	Speaker(CGEEngine *vm);
};

class PocLight: public Sprite {
	CGEEngine *_vm;
public:
	PocLight(CGEEngine *vm);
};

} // End of namespace CGE

#endif
