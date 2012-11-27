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

#include "common/array.h"
#include "common/rect.h"
#include "graphics/palette.h"
#include "cge/general.h"
#include "cge/vga13h.h"
#include "cge/bitmap.h"
#include "cge/text.h"
#include "cge/cge_main.h"
#include "cge/cge.h"

namespace CGE {

Seq *getConstantSeq(bool seqFlag) {
	const Seq seq1[] = { { 0, 0, 0, 0, 0 } };
	const Seq seq2[] = { { 0, 1, 0, 0, 0 }, { 1, 0, 0, 0, 0 } };

	Seq *seq;
	if (seqFlag) {
		seq = (Seq *)malloc(1 * sizeof(Seq));
		*seq = seq1[0];
	} else {
		seq = (Seq *)malloc(2 * sizeof(Seq));
		seq[0] = seq2[0];
		seq[1] = seq2[1];
	}

	return seq;
}

Sprite::Sprite(CGEEngine *vm, BitmapPtr *shpP)
	: _x(0), _y(0), _z(0), _nearPtr(0), _takePtr(0),
	  _next(NULL), _prev(NULL), _seqPtr(kNoSeq), _time(0),
	  _ext(NULL), _ref(-1), _scene(0), _vm(vm) {
	memset(_file, 0, sizeof(_file));
	memset(&_flags, 0, sizeof(_flags));
	_ref = 0;
	_x = _y = 0;
	_w = _h = 0;
	_time = 0;
	_seqPtr = 0;
	_shpCnt = 0;
	_prev = _next = NULL;

	setShapeList(shpP);
}

Sprite::~Sprite() {
	if (_vm->_sprite == this)
		_vm->_sprite = NULL;

	contract();
}

BitmapPtr Sprite::shp() {
	SprExt *e = _ext;
	if (!e || !e->_seq)
		return NULL;

	int i = e->_seq[_seqPtr]._now;
	if (i >= _shpCnt)
		error("Invalid PHASE in SPRITE::Shp() %s", _file);
	return e->_shpList[i];
}

BitmapPtr *Sprite::setShapeList(BitmapPtr *shpP) {
	BitmapPtr *r = (_ext) ? _ext->_shpList : NULL;

	_shpCnt = 0;
	_w = 0;
	_h = 0;

	if (shpP) {
		BitmapPtr *p;
		for (p = shpP; *p; p++) {
			BitmapPtr b = (*p); // ->Code();
			if (b->_w > _w)
				_w = b->_w;
			if (b->_h > _h)
				_h = b->_h;
			_shpCnt++;
		}
		expand();
		_ext->_shpList = shpP;
		_flags._bDel = true;
		if (!_ext->_seq)
			setSeq(getConstantSeq(_shpCnt < 2));
	}
	return r;
}

bool Sprite::works(Sprite *spr) {
	if (!spr || !spr->_ext)
		return false;

	CommandHandler::Command *c = spr->_ext->_take;
	if (c != NULL) {
		c += spr->_takePtr;
		if (c->_ref == _ref)
			if (c->_commandType != kCmdLabel || (c->_val == 0 || c->_val == _vm->_now))
				return true;
	}

	return false;
}

Seq *Sprite::setSeq(Seq *seq) {
	if (_ext) {
		free(_ext->_seq);
		_ext->_seq = NULL;
	}

	expand();

	Seq *s = _ext->_seq;
	_ext->_seq = seq;
	if (_seqPtr == kNoSeq)
		step(0);
	else if (_time == 0)
		step(_seqPtr);
	return s;
}

bool Sprite::seqTest(int n) {
	if (n >= 0)
		return (_seqPtr == n);
	if (_ext)
		return (_ext->_seq[_seqPtr]._next == _seqPtr);
	return true;
}

CommandHandler::Command *Sprite::snList(SnList type) {
	SprExt *e = _ext;
	if (e)
		return (type == kNear) ? e->_near : e->_take;
	return NULL;
}

void Sprite::setName(char *newName) {
	if (!_ext)
		return;

	if (_ext->_name) {
		delete[] _ext->_name;
		_ext->_name = NULL;
	}
	if (newName) {
		_ext->_name = new char[strlen(newName) + 1];
		assert(_ext->_name != NULL);
		strcpy(_ext->_name, newName);
	}
}

Sprite *Sprite::expand() {
	if (_ext)
		return this;

	_ext = new SprExt;
	assert(_ext != NULL);
	if (!*_file)
		return this;

	static const char *Comd[] = { "Name", "Phase", "Seq", "Near", "Take", NULL };
	char fname[kPathMax];

	Common::Array<BitmapPtr> shplist;
	for (int i = 0; i < _shpCnt + 1; ++i)
		shplist.push_back(NULL);

	Seq *seq = NULL;
	int shapeCount = 0,
	    seqCount = 0,
	    nearCount = 0,
	    takeCount = 0,
	    maxnow = 0,
	    maxnxt = 0;

	CommandHandler::Command *nearList = NULL;
	CommandHandler::Command *takeList = NULL;
	_vm->mergeExt(fname, _file, kSprExt);
	if (_vm->_resman->exist(fname)) { // sprite description file exist
		EncryptedStream sprf(_vm, fname);
		if (sprf.err())
			error("Bad SPR [%s]", fname);
		Common::String line;
		char tmpStr[kLineMax + 1];
		int len = 0, lcnt = 0;

		for (line = sprf.readLine(); !sprf.eos(); line = sprf.readLine()) {
			len = line.size();
			strcpy(tmpStr, line.c_str());
			lcnt++;
			if (len == 0 || *tmpStr == '.')
				continue;

			CommandHandler::Command *c;
			switch (_vm->takeEnum(Comd, strtok(tmpStr, " =\t"))) {
			case 0:
				// Name
				setName(strtok(NULL, ""));
				break;
			case 1:
				// Phase
				// In case the shape index gets too high, increase the array size
				while ((shapeCount + 1) >= (int)shplist.size()) {
					shplist.push_back(NULL);
					++_shpCnt;
				}
				shplist[shapeCount++] = new Bitmap(_vm, strtok(NULL, " \t,;/"));
				break;
			case 2:
				// Seq
				seq = (Seq *)realloc(seq, (seqCount + 1) * sizeof(*seq));
				assert(seq != NULL);
				Seq *s;
				s = &seq[seqCount++];
				s->_now = atoi(strtok(NULL, " \t,;/"));
				if (s->_now > maxnow)
					maxnow = s->_now;
				s->_next = atoi(strtok(NULL, " \t,;/"));
				switch (s->_next) {
				case 0xFF:
					s->_next = seqCount;
					break;
				case 0xFE:
					s->_next = seqCount - 1;
					break;
				}
				if (s->_next > maxnxt)
					maxnxt = s->_next;
				s->_dx = atoi(strtok(NULL, " \t,;/"));
				s->_dy = atoi(strtok(NULL, " \t,;/"));
				s->_dly = atoi(strtok(NULL, " \t,;/"));
				break;
			case 3:
				// Near
				if (_nearPtr == kNoPtr)
					break;
				nearList = (CommandHandler::Command *)realloc(nearList, (nearCount + 1) * sizeof(*nearList));
				assert(nearList != NULL);
				c = &nearList[nearCount++];
				if ((c->_commandType = (CommandType)_vm->takeEnum(CommandHandler::_commandText, strtok(NULL, " \t,;/"))) < 0)
					error("Bad NEAR in %d [%s]", lcnt, fname);
				c->_ref = atoi(strtok(NULL, " \t,;/"));
				c->_val = atoi(strtok(NULL, " \t,;/"));
				c->_spritePtr = NULL;
				break;
			case 4:
				// Take
				if (_takePtr == kNoPtr)
					break;
				takeList = (CommandHandler::Command *)realloc(takeList, (takeCount + 1) * sizeof(*takeList));
				assert(takeList != NULL);
				c = &takeList[takeCount++];
				if ((c->_commandType = (CommandType)_vm->takeEnum(CommandHandler::_commandText, strtok(NULL, " \t,;/"))) < 0)
					error("Bad NEAR in %d [%s]", lcnt, fname);
				c->_ref = atoi(strtok(NULL, " \t,;/"));
				c->_val = atoi(strtok(NULL, " \t,;/"));
				c->_spritePtr = NULL;
				break;
			}
		}
	} else {
		// no sprite description: try to read immediately from .BMP
		shplist[shapeCount++] = new Bitmap(_vm, _file);
	}

	shplist[shapeCount] = NULL;
	if (seq) {
		if (maxnow >= shapeCount)
			error("Bad PHASE in SEQ [%s]", fname);
		if (maxnxt >= seqCount)
			error("Bad JUMP in SEQ [%s]", fname);
		setSeq(seq);
	} else
		setSeq(getConstantSeq(_shpCnt == 1));

	// Set the shape list
	BitmapPtr *shapeList = new BitmapPtr[shplist.size()];
	for (uint i = 0; i < shplist.size(); ++i)
		shapeList[i] = shplist[i];

	setShapeList(shapeList);

	if (nearList)
		nearList[nearCount - 1]._spritePtr = _ext->_near = nearList;
	else
		_nearPtr = kNoPtr;
	if (takeList)
		takeList[takeCount - 1]._spritePtr = _ext->_take = takeList;
	else
		_takePtr = kNoPtr;

	return this;
}

Sprite *Sprite::contract() {
	SprExt *e = _ext;
	if (!e)
		return this;

	if (e->_name)
		delete[] e->_name;
	if (_flags._bDel && e->_shpList) {
		for (int i = 0; e->_shpList[i]; i++)
			delete e->_shpList[i];
		delete[] e->_shpList;
	}

	free(e->_seq);
	free(e->_near);
	free(e->_take);

	delete e;
	_ext = NULL;

	return this;
}

Sprite *Sprite::backShow(bool fast) {
	expand();
	show(2);
	show(1);
	if (fast)
		show(0);
	contract();
	return this;
}

void Sprite::step(int nr) {
	if (nr >= 0)
		_seqPtr = nr;
	if (_ext) {
		Seq *seq;
		if (nr < 0)
			_seqPtr = _ext->_seq[_seqPtr]._next;
		seq = _ext->_seq + _seqPtr;
		if (seq->_dly >= 0) {
			gotoxy(_x + (seq->_dx), _y + (seq->_dy));
			_time = seq->_dly;
		}
	}
}

void Sprite::tick() {
	step();
}

void Sprite::makeXlat(uint8 *x) {
	if (!_ext)
		return;

	if (_flags._xlat)
		killXlat();
	for (BitmapPtr *b = _ext->_shpList; *b; b++)
		(*b)->_m = x;
	_flags._xlat = true;
}

void Sprite::killXlat() {
	if (!_flags._xlat || !_ext)
		return;

	uint8 *m = (*_ext->_shpList)->_m;
	free(m);

	for (BitmapPtr *b = _ext->_shpList; *b; b++)
		(*b)->_m = NULL;
	_flags._xlat = false;
}

void Sprite::gotoxy(int x, int y) {
	int xo = _x, yo = _y;
	if (_x < kScrWidth) {
		if (x < 0)
			x = 0;
		if (x + _w > kScrWidth)
			x = (kScrWidth - _w);
		_x = x;
	}
	if (_h < kScrHeight) {
		if (y < 0)
			y = 0;
		if (y + _h > kScrHeight)
			y = (kScrHeight - _h);
		_y = y;
	}
	if (_next)
		if (_next->_flags._slav)
			_next->gotoxy(_next->_x - xo + _x, _next->_y - yo + _y);
	if (_flags._shad)
		_prev->gotoxy(_prev->_x - xo + _x, _prev->_y - yo + _y);
}

void Sprite::center() {
	gotoxy((kScrWidth - _w) / 2, (kScrHeight - _h) / 2);
}

void Sprite::show() {
	SprExt *e;
	e = _ext;
	e->_x0 = e->_x1;
	e->_y0 = e->_y1;
	e->_b0 = e->_b1;
	e->_x1 = _x;
	e->_y1 = _y;
	e->_b1 = shp();
	if (!_flags._hide) {
		if (_flags._xlat)
			e->_b1->xShow(e->_x1, e->_y1);
		else
			e->_b1->show(e->_x1, e->_y1);
	}
}

void Sprite::show(uint16 pg) {
	Graphics::Surface *a = _vm->_vga->_page[1];
	_vm->_vga->_page[1] = _vm->_vga->_page[pg & 3];
	shp()->show(_x, _y);
	_vm->_vga->_page[1] = a;
}

void Sprite::hide() {
	SprExt *e = _ext;
	if (e->_b0)
		e->_b0->hide(e->_x0, e->_y0);
}

BitmapPtr Sprite::ghost() {
	SprExt *e = _ext;
	if (!e->_b1)
		return NULL;

	BitmapPtr bmp = new Bitmap(_vm, 0, 0, (uint8 *)NULL);
	assert(bmp != NULL);
	bmp->_w = e->_b1->_w;
	bmp->_h = e->_b1->_h;
	bmp->_b = new HideDesc[bmp->_h];
	assert(bmp->_b != NULL);
	bmp->_v = (uint8 *) memcpy(bmp->_b, e->_b1->_b, sizeof(HideDesc) * bmp->_h);
	bmp->_map = (e->_y1 << 16) + e->_x1;
	return bmp;
}

void Sprite::sync(Common::Serializer &s) {
	uint16 unused = 0;

	s.syncAsUint16LE(unused);
	s.syncAsUint16LE(unused);	// _ext
	s.syncAsUint16LE(_ref);
	s.syncAsByte(_scene);

	// bitfield in-memory storage is unpredictable, so to avoid
	// any issues, pack/unpack everything manually
	uint16 flags = 0;
	if (s.isLoading()) {
		s.syncAsUint16LE(flags);
		_flags._hide = flags & 0x0001 ? true : false;
		_flags._near = flags & 0x0002 ? true : false;
		_flags._drag = flags & 0x0004 ? true : false;
		_flags._hold = flags & 0x0008 ? true : false;
		_flags._dummy = flags & 0x0010 ? true : false;
		_flags._slav = flags & 0x0020 ? true : false;
		_flags._syst = flags & 0x0040 ? true : false;
		_flags._kill = flags & 0x0080 ? true : false;
		_flags._xlat = flags & 0x0100 ? true : false;
		_flags._port = flags & 0x0200 ? true : false;
		_flags._kept = flags & 0x0400 ? true : false;
		_flags._east = flags & 0x0800 ? true : false;
		_flags._shad = flags & 0x1000 ? true : false;
		_flags._back = flags & 0x2000 ? true : false;
		_flags._bDel = flags & 0x4000 ? true : false;
		_flags._tran = flags & 0x8000 ? true : false;
	} else {
		flags = (flags << 1) | _flags._tran;
		flags = (flags << 1) | _flags._bDel;
		flags = (flags << 1) | _flags._back;
		flags = (flags << 1) | _flags._shad;
		flags = (flags << 1) | _flags._east;
		flags = (flags << 1) | _flags._kept;
		flags = (flags << 1) | _flags._port;
		flags = (flags << 1) | _flags._xlat;
		flags = (flags << 1) | _flags._kill;
		flags = (flags << 1) | _flags._syst;
		flags = (flags << 1) | _flags._slav;
		flags = (flags << 1) | _flags._dummy;
		flags = (flags << 1) | _flags._hold;
		flags = (flags << 1) | _flags._drag;
		flags = (flags << 1) | _flags._near;
		flags = (flags << 1) | _flags._hide;
		s.syncAsUint16LE(flags);
	}

	s.syncAsUint16LE(_x);
	s.syncAsUint16LE(_y);
	s.syncAsByte(_z);
	s.syncAsUint16LE(_w);
	s.syncAsUint16LE(_h);
	s.syncAsUint16LE(_time);
	s.syncAsByte(_nearPtr);
	s.syncAsByte(_takePtr);
	s.syncAsSint16LE(_seqPtr);
	s.syncAsUint16LE(_shpCnt);
	s.syncBytes((byte *)&_file[0], 9);
	_file[8] = '\0';

	s.syncAsUint16LE(unused);	// _prev
	s.syncAsUint16LE(unused);	// _next
}

Queue::Queue(bool show) : _head(NULL), _tail(NULL), _show(show) {
}

Queue::~Queue() {
	clear();
}

void Queue::clear() {
	while (_head) {
		Sprite *s = remove(_head);
		if (s->_flags._kill)
			delete s;
	}
}

void Queue::append(Sprite *spr) {
	if (_tail) {
		spr->_prev = _tail;
		_tail->_next = spr;
	} else
		_head = spr;
	_tail = spr;
	if (_show)
		spr->expand();
	else
		spr->contract();
}

void Queue::insert(Sprite *spr, Sprite *nxt) {
	if (nxt == _head) {
		spr->_next = _head;
		_head = spr;
		if (!_tail)
			_tail = spr;
	} else {
		assert(nxt);
		spr->_next = nxt;
		spr->_prev = nxt->_prev;
		if (spr->_prev)
			spr->_prev->_next = spr;
	}
	if (spr->_next)
		spr->_next->_prev = spr;
	if (_show)
		spr->expand();
	else
		spr->contract();
}

void Queue::insert(Sprite *spr) {
	Sprite *s;
	for (s = _head; s; s = s->_next)
		if (s->_z > spr->_z)
			break;
	if (s)
		insert(spr, s);
	else
		append(spr);
	if (_show)
		spr->expand();
	else
		spr->contract();
}

template<typename T>
inline bool contains(const Common::List<T> &l, const T &v) {
	return (Common::find(l.begin(), l.end(), v) != l.end());
}

Sprite *Queue::remove(Sprite *spr) {
	if (spr == _head)
		_head = spr->_next;
	if (spr == _tail)
		_tail = spr->_prev;
	if (spr->_next)
		spr->_next->_prev = spr->_prev;
	if (spr->_prev)
		spr->_prev->_next = spr->_next;
	spr->_prev = NULL;
	spr->_next = NULL;
	return spr;
}

Sprite *Queue::locate(int ref) {
	for (Sprite *spr = _head; spr; spr = spr->_next) {
		if (spr->_ref == ref)
			return spr;
	}
	return NULL;
}

Vga::Vga(CGEEngine *vm) : _frmCnt(0), _msg(NULL), _name(NULL), _setPal(false), _mono(0), _vm(vm) {
	_oldColors = NULL;
	_newColors = NULL;
	_showQ = new Queue(true);
	_spareQ = new Queue(false);
	_sysPal = new Dac[kPalCount];

	for (int idx = 0; idx < 4; idx++) {
		_page[idx] = new Graphics::Surface();
		_page[idx]->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	}

	_oldColors = (Dac *)malloc(sizeof(Dac) * kPalCount);
	_newColors = (Dac *)malloc(sizeof(Dac) * kPalCount);
	getColors(_oldColors);
	sunset();
	setColors();
	clear(0);
}

Vga::~Vga() {
	_mono = 0;

	Common::String buffer = "";
/*
	clear(0);
	setMode(_oldMode);
	setColors();
	restoreScreen(_oldScreen);
	sunrise(_oldColors);
*/
	free(_oldColors);
	free(_newColors);
	if (_msg)
		buffer = Common::String(_msg);
	if (_name)
		buffer = buffer + " [" + _name + "]";

	debugN("%s", buffer.c_str());

	delete _showQ;
	delete _spareQ;
	delete[] _sysPal;

	for (int idx = 0; idx < 4; idx++) {
		_page[idx]->free();
		delete _page[idx];
	}
}

void Vga::waitVR() {
	// Since some of the game parts rely on using vertical sync as a delay mechanism,
	// we're introducing a short delay to simulate it
	g_system->delayMillis(5);
}

void Vga::getColors(Dac *tab) {
	byte palData[kPalSize];
	g_system->getPaletteManager()->grabPalette(palData, 0, kPalCount);
	palToDac(palData, tab);
}

uint8 Vga::closest(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB) {
#define f(col, lum) ((((uint16)(col)) << 8) / lum)
	uint16 i, dif = 0xFFFF, found = 0;
	uint16 L = colR + colG + colB;
	if (!L)
		L++;
	uint16 R = f(colR, L), G = f(colG, L), B = f(colB, L);
	for (i = 0; i < 256; i++) {
		uint16 l = pal[i]._r + pal[i]._g + pal[i]._b;
		if (!l)
			l++;
		int  r = f(pal[i]._r, l), g = f(pal[i]._g, l), b = f(pal[i]._b, l);
		uint16 D = ((r > R) ? (r - R) : (R - r)) +
		           ((g > G) ? (g - G) : (G - g)) +
		           ((b > B) ? (b - B) : (B - b)) +
		           ((l > L) ? (l - L) : (L - l)) * 10 ;

		if (D < dif) {
			found = i;
			dif = D;
			if (D == 0)
				break;    // exact!
		}
	}
	return found;
#undef f
}

uint8 *Vga::glass(Dac *pal, const uint8 colR, const uint8 colG, const uint8 colB) {
	uint8 *x = (uint8 *)malloc(256);
	if (x) {
		uint16 i;
		for (i = 0; i < 256; i++) {
			x[i] = closest(pal, ((uint16)(pal[i]._r) * colR) / 255,
			                    ((uint16)(pal[i]._g) * colG) / 255,
			                    ((uint16)(pal[i]._b) * colB) / 255);
		}
	}
	return x;
}

void Vga::palToDac(const byte *palData, Dac *tab) {
	const byte *colP = palData;
	for (int idx = 0; idx < kPalCount; idx++, colP += 3) {
		tab[idx]._r = *colP >> 2;
		tab[idx]._g = *(colP + 1) >> 2;
		tab[idx]._b = *(colP + 2) >> 2;
	}
}

void Vga::dacToPal(const Dac *tab, byte *palData) {
	for (int idx = 0; idx < kPalCount; idx++, palData += 3) {
		*palData = tab[idx]._r << 2;
		*(palData + 1) = tab[idx]._g << 2;
		*(palData + 2) = tab[idx]._b << 2;
	}
}

void Vga::setColors(Dac *tab, int lum) {
	Dac *palP = tab, *destP = _newColors;
	for (int idx = 0; idx < kPalCount; idx++, palP++, destP++) {
		destP->_r = (palP->_r * lum) >> 6;
		destP->_g = (palP->_g * lum) >> 6;
		destP->_b = (palP->_b * lum) >> 6;
	}

	if (_mono) {
		destP = _newColors;
		for (int idx = 0; idx < kPalCount; idx++, destP++) {
			// Form a greyscalce colour from 30% R, 59% G, 11% B
			uint8 intensity = (((int)destP->_r * 77) + ((int)destP->_g * 151) + ((int)destP->_b * 28)) >> 8;
			destP->_r = intensity;
			destP->_g = intensity;
			destP->_b = intensity;
		}
	}

	_setPal = true;
}

void Vga::setColors() {
	memset(_newColors, 0, kPalSize);
	updateColors();
}

void Vga::sunrise(Dac *tab) {
	for (int i = 0; i <= 64; i += kFadeStep) {
		setColors(tab, i);
		waitVR();
		updateColors();
	}
}

void Vga::sunset() {
	Dac tab[256];
	getColors(tab);
	for (int i = 64; i >= 0; i -= kFadeStep) {
		setColors(tab, i);
		waitVR();
		updateColors();
	}
}

void Vga::show() {
	for (Sprite *spr = _showQ->first(); spr; spr = spr->_next)
		spr->show();
	update();
	for (Sprite *spr = _showQ->first(); spr; spr = spr->_next)
		spr->hide();

	_frmCnt++;
}

void Vga::updateColors() {
	byte palData[kPalSize];
	dacToPal(_newColors, palData);
	g_system->getPaletteManager()->setPalette(palData, 0, 256);
}

void Vga::update() {
	SWAP(Vga::_page[0], Vga::_page[1]);

	if (_setPal) {
		updateColors();
		_setPal = false;
	}
	if (_vm->_showBoundariesFl) {
		Vga::_page[0]->hLine(0, 200 - kPanHeight, 320, 0xee);
		if (_vm->_barriers[_vm->_now]._horz != 255) {
			for (int i = 0; i < 8; i++)
				Vga::_page[0]->vLine((_vm->_barriers[_vm->_now]._horz * 8) + i, 0, 200, 0xff);
		}
		if (_vm->_barriers[_vm->_now]._vert != 255) {
			for (int i = 0; i < 4; i++)
				Vga::_page[0]->hLine(0, 80 + (_vm->_barriers[_vm->_now]._vert * 4) + i, 320, 0xff);
		}
	}

	g_system->copyRectToScreen(Vga::_page[0]->getBasePtr(0, 0), kScrWidth, 0, 0, kScrWidth, kScrHeight);
	g_system->updateScreen();
}

void Vga::clear(uint8 color) {
	for (int paneNum = 0; paneNum < 4; paneNum++)
		_page[paneNum]->fillRect(Common::Rect(0, 0, kScrWidth, kScrHeight), color);
}

void Vga::copyPage(uint16 d, uint16 s) {
	_page[d]->copyFrom(*_page[s]);
}

//--------------------------------------------------------------------------

void Bitmap::xShow(int16 x, int16 y) {
	debugC(4, kCGEDebugBitmap, "Bitmap::xShow(%d, %d)", x, y);

	const byte *srcP = (const byte *)_v;
	byte *destEndP = (byte *)_vm->_vga->_page[1]->pixels + (kScrWidth * kScrHeight);
	byte *lookupTable = _m;

	// Loop through processing data for each plane. The game originally ran in plane mapped mode, where a
	// given plane holds each fourth pixel sequentially. So to handle an entire picture, each plane's data
	// must be decompressed and inserted into the surface
	for (int planeCtr = 0; planeCtr < 4; planeCtr++) {
		byte *destP = (byte *)_vm->_vga->_page[1]->getBasePtr(x + planeCtr, y);

		for (;;) {
			uint16 v = READ_LE_UINT16(srcP);
			srcP += 2;
			int cmd = v >> 14;
			int count = v & 0x3FFF;

			if (cmd == 0) {
				// End of image
				break;
			}

			assert(destP < destEndP);

			if (cmd == 2)
				srcP++;
			else if (cmd == 3)
				srcP += count;

			// Handle a set of pixels
			while (count-- > 0) {
				// Transfer operation
				switch (cmd) {
				case 1:
					// SKIP
					break;
				case 2:
				case 3:
					// TINT
					*destP = lookupTable[*destP];
					break;
				}

				// Move to next dest position
				destP += 4;
			}
		}
	}
}


void Bitmap::show(int16 x, int16 y) {
	debugC(5, kCGEDebugBitmap, "Bitmap::show(%d, %d)", x, y);

	const byte *srcP = (const byte *)_v;
	byte *destEndP = (byte *)_vm->_vga->_page[1]->pixels + (kScrWidth * kScrHeight);

	// Loop through processing data for each plane. The game originally ran in plane mapped mode, where a
	// given plane holds each fourth pixel sequentially. So to handle an entire picture, each plane's data
	// must be decompressed and inserted into the surface
	for (int planeCtr = 0; planeCtr < 4; planeCtr++) {
		byte *destP = (byte *)_vm->_vga->_page[1]->getBasePtr(x + planeCtr, y);

		for (;;) {
			uint16 v = READ_LE_UINT16(srcP);
			srcP += 2;
			int cmd = v >> 14;
			int count = v & 0x3FFF;

			if (cmd == 0) {
				// End of image
				break;
			}

			assert(destP < destEndP);

			// Handle a set of pixels
			while (count-- > 0) {
				// Transfer operation
				switch (cmd) {
				case 1:
					// SKIP
					break;
				case 2:
					// REPEAT
					*destP = *srcP;
					break;
				case 3:
					// COPY
					*destP = *srcP++;
					break;
				}

				// Move to next dest position
				destP += 4;
			}

			if (cmd == 2)
				srcP++;
		}
	}
}


void Bitmap::hide(int16 x, int16 y) {
	debugC(5, kCGEDebugBitmap, "Bitmap::hide(%d, %d)", x, y);

	for (int yp = y; yp < y + _h; yp++) {
		const byte *srcP = (const byte *)_vm->_vga->_page[2]->getBasePtr(x, yp);
		byte *destP = (byte *)_vm->_vga->_page[1]->getBasePtr(x, yp);

		Common::copy(srcP, srcP + _w, destP);
	}
}

/*--------------------------------------------------------------------------*/

HorizLine::HorizLine(CGEEngine *vm) : Sprite(vm, NULL), _vm(vm) {
	// Set the sprite list
	BitmapPtr *HL = new BitmapPtr[2];
	HL[0] = new Bitmap(_vm, "HLINE");
	HL[1] = NULL;

	setShapeList(HL);
}

SceneLight::SceneLight(CGEEngine *vm) : Sprite(vm, NULL), _vm(vm) {
	// Set the sprite list
	BitmapPtr *PR = new BitmapPtr[2];
	PR[0] = new Bitmap(_vm, "PRESS");
	PR[1] = NULL;

	setShapeList(PR);
}

Speaker::Speaker(CGEEngine *vm): Sprite(vm, NULL), _vm(vm) {
	// Set the sprite list
	BitmapPtr *SP = new BitmapPtr[3];
	SP[0] = new Bitmap(_vm, "SPK_L");
	SP[1] = new Bitmap(_vm, "SPK_R");
	SP[2] = NULL;

	setShapeList(SP);
}

PocLight::PocLight(CGEEngine *vm): Sprite(vm, NULL), _vm(vm) {
	// Set the sprite list
	BitmapPtr *LI = new BitmapPtr[5];
	LI[0] = new Bitmap(_vm, "LITE0");
	LI[1] = new Bitmap(_vm, "LITE1");
	LI[2] = new Bitmap(_vm, "LITE2");
	LI[3] = new Bitmap(_vm, "LITE3");
	LI[4] = NULL;

	setShapeList(LI);

	_flags._kill = false;
}

} // End of namespace CGE
