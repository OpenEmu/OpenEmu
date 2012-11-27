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

#include "parallaction/graphics.h"
#include "parallaction/parallaction.h"

#include "common/tokenizer.h"

namespace Parallaction {

class WrappedLineFormatter {

protected:
	Common::String _line;
	Font *_font;
	uint16 _lines, _lineWidth;

	virtual void setup() = 0;
	virtual void action() = 0;
	virtual void end() = 0;
	virtual Common::String expand(const Common::String &token) { return token; }

	void textAccum(const Common::String &token, uint16 width) {
		if (token.empty()) {
			return;
		}

		_lineWidth += width;
		_line += token;
	}

	void textNewLine() {
		_lines++;
		_lineWidth = 0;
		_line.clear();
	}

public:
	WrappedLineFormatter(Font *font) : _font(font) { }
	virtual ~WrappedLineFormatter() { }

	virtual void calc(const Common::String &text, uint16 maxwidth) {
		setup();

		_lineWidth = 0;
		_line.clear();
		_lines = 0;

		Common::StringTokenizer	tokenizer(text, " ");
		Common::String token;
		Common::String blank(" ");

		uint16 blankWidth = _font->getStringWidth(" ");
		uint16 tokenWidth = 0;

		while (!tokenizer.empty()) {
			token = tokenizer.nextToken();
			token = expand(token);

			if (token == "/") {
				tokenWidth = 0;
				action();
				textNewLine();
			} else {
				// todo: expand '%'
				tokenWidth = _font->getStringWidth(token.c_str());

				if (_lineWidth == 0) {
					textAccum(token, tokenWidth);
				} else {
					if (_lineWidth + blankWidth + tokenWidth <= maxwidth) {
						textAccum(blank, blankWidth);
						textAccum(token, tokenWidth);
					} else {
						action();
						textNewLine();
						textAccum(token, tokenWidth);
					}
				}
			}
		}

		end();
	}
};

class StringExtent_NS : public WrappedLineFormatter {

	uint	_width, _height;

protected:
	virtual Common::String expand(const Common::String &token) {
		if (token.compareToIgnoreCase("%p") == 0) {
			return Common::String("/");
		}

		return token;
	}

	virtual void setup() {
		_width = _height = 0;

		_line.clear();
		_lines = 0;
		_width = 0;
	}

	virtual void action() {
		if (_lineWidth > _width) {
			_width = _lineWidth;
		}
		_height = _lines * _font->height();
	}

	virtual void end() {
		action();
	}

public:
	StringExtent_NS(Font *font) : WrappedLineFormatter(font) { }

	uint width() const { return _width; }
	uint height() const { return _height; }
};


class StringWriter_NS : public WrappedLineFormatter {
	Parallaction_ns *_vm;

	uint	_width, _height;
	byte	_color;

	Graphics::Surface	*_surf;

protected:
	virtual Common::String expand(const Common::String& token) {
		if (token.compareToIgnoreCase("%p") == 0) {
			Common::String t(_vm->_password);
			for (int i = t.size(); i < 7; i++) {
				t += '.';
			}
			return Common::String("> ") + t;
		} else
		if (token.compareToIgnoreCase("%s") == 0) {
			char buf[20];
			sprintf(buf, "%i", _vm->_score);
			return Common::String(buf);
		}

		return token;
	}

	virtual void setup() {
	}

	virtual void action() {
		if (_line.empty()) {
			return;
		}
		uint16 rx = 10;
		uint16 ry = 4 + _lines * _font->height();	// y

		byte *dst = (byte *)_surf->getBasePtr(rx, ry);
		_font->setColor(_color);
		_font->drawString(dst, _surf->w, _line.c_str());
	}

	virtual void end() {
		action();
	}

public:
	StringWriter_NS(Parallaction_ns *vm, Font *font) : WrappedLineFormatter(font), _vm(vm) { }

	void write(const Common::String &text, uint maxWidth, byte color, Graphics::Surface *surf) {
		StringExtent_NS	se(_font);
		se.calc(text, maxWidth);
		_width = se.width() + 10;
		_height = se.height() + 20;
		_color = color;
		_surf = surf;

		calc(text, maxWidth);
	}

};



#define	BALLOON_TRANSPARENT_COLOR_NS 2
#define BALLOON_TRANSPARENT_COLOR_BR 0

#define BALLOON_TAIL_WIDTH	12
#define BALLOON_TAIL_HEIGHT	10


byte _resBalloonTail[2][BALLOON_TAIL_WIDTH*BALLOON_TAIL_HEIGHT] = {
	{
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	},
	{
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x01, 0x01, 0x00, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x02, 0x02
	}
};

class BalloonManager_ns : public BalloonManager {

	Parallaction_ns *_vm;
	static int16 _dialogueBalloonX[5];

	byte _textColors[3];

	struct Balloon {
		Common::Rect outerBox;
		Common::Rect innerBox;
		Graphics::Surface *surface;
		GfxObj	*obj;
	} _intBalloons[5];

	uint	_numBalloons;

	int createBalloon(int16 w, int16 h, int16 winding, uint16 borderThickness);
	Balloon *getBalloon(uint id);

	StringWriter_NS _sw;
	StringExtent_NS	_se;

public:
	BalloonManager_ns(Parallaction_ns *vm, Font *font);
	~BalloonManager_ns();

	void reset();
	int setLocationBalloon(const Common::String &text, bool endGame);
	int setDialogueBalloon(const Common::String &text, uint16 winding, TextColor textColor);
	int setSingleBalloon(const Common::String &text, uint16 x, uint16 y, uint16 winding, TextColor textColor);
	void setBalloonText(uint id, const Common::String &text, TextColor textColor);
	int hitTestDialogueBalloon(int x, int y);
};

int16 BalloonManager_ns::_dialogueBalloonX[5] = { 80, 120, 150, 150, 150 };

BalloonManager_ns::BalloonManager_ns(Parallaction_ns *vm, Font *font) : _vm(vm), _numBalloons(0), _sw(vm, font), _se(font) {
	_textColors[kSelectedColor] = 0;
	_textColors[kUnselectedColor] = 3;
	_textColors[kNormalColor] = 0;
}

BalloonManager_ns::~BalloonManager_ns() {

}


BalloonManager_ns::Balloon* BalloonManager_ns::getBalloon(uint id) {
	assert(id < _numBalloons);
	return &_intBalloons[id];
}

int BalloonManager_ns::createBalloon(int16 w, int16 h, int16 winding, uint16 borderThickness) {
	assert(_numBalloons < 5);

	int id = _numBalloons;

	Balloon *balloon = &_intBalloons[id];

	int16 real_h = (winding == -1) ? h : h + 9;
	balloon->surface = new Graphics::Surface;
	balloon->surface->create(w, real_h, Graphics::PixelFormat::createFormatCLUT8());
	balloon->surface->fillRect(Common::Rect(w, real_h), BALLOON_TRANSPARENT_COLOR_NS);

	Common::Rect r(w, h);
	balloon->surface->fillRect(r, 0);
	balloon->outerBox = r;

	r.grow(-borderThickness);
	balloon->surface->fillRect(r, 1);
	balloon->innerBox = r;

	if (winding != -1) {
		// draws tail
		// TODO: this bitmap tail should only be used for Dos games. Amiga should use a polygon fill.
		winding = (winding == 0 ? 1 : 0);
		Common::Rect s(BALLOON_TAIL_WIDTH, BALLOON_TAIL_HEIGHT);
		s.moveTo(r.width()/2 - 5, r.bottom - 1);
		_vm->_gfx->blt(s, _resBalloonTail[winding], balloon->surface, LAYER_FOREGROUND, 100, BALLOON_TRANSPARENT_COLOR_NS);
	}

	_numBalloons++;

	return id;
}


int BalloonManager_ns::setSingleBalloon(const Common::String &text, uint16 x, uint16 y, uint16 winding, TextColor textColor) {

	int16 w, h;

	_se.calc(text, MAX_BALLOON_WIDTH);
	w = _se.width() + 14;
	h = _se.height() + 20;

	int id = createBalloon(w+5, h, winding, 1);
	Balloon *balloon = &_intBalloons[id];

	_sw.write(text, MAX_BALLOON_WIDTH, _textColors[textColor], balloon->surface);

	// TODO: extract some text to make a name for obj
	balloon->obj = _vm->_gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = x;
	balloon->obj->y = y;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_NS;

	return id;
}

int BalloonManager_ns::setDialogueBalloon(const Common::String &text, uint16 winding, TextColor textColor) {

	int16 w, h;

	_se.calc(text, MAX_BALLOON_WIDTH);
	w = _se.width() + 14;
	h = _se.height() + 20;


	int id = createBalloon(w+5, h, winding, 1);
	Balloon *balloon = &_intBalloons[id];

	_sw.write(text, MAX_BALLOON_WIDTH, _textColors[textColor], balloon->surface);

	// TODO: extract some text to make a name for obj
	balloon->obj = _vm->_gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = _dialogueBalloonX[id];
	balloon->obj->y = 10;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_NS;

	if (id > 0) {
		balloon->obj->y += _intBalloons[id - 1].obj->y + _intBalloons[id - 1].outerBox.height();
	}


	return id;
}

void BalloonManager_ns::setBalloonText(uint id, const Common::String &text, TextColor textColor) {
	Balloon *balloon = getBalloon(id);
	balloon->surface->fillRect(balloon->innerBox, 1);

	_sw.write(text, MAX_BALLOON_WIDTH, _textColors[textColor], balloon->surface);
}


int BalloonManager_ns::setLocationBalloon(const Common::String &text, bool endGame) {

	int16 w, h;

	_se.calc(text, MAX_BALLOON_WIDTH);
	w = _se.width() + 14;
	h = _se.height() + 20;

	int id = createBalloon(w+(endGame ? 5 : 10), h+5, -1, BALLOON_TRANSPARENT_COLOR_NS);
	Balloon *balloon = &_intBalloons[id];
	_sw.write(text, MAX_BALLOON_WIDTH, _textColors[kNormalColor], balloon->surface);

	// TODO: extract some text to make a name for obj
	balloon->obj = _vm->_gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = 5;
	balloon->obj->y = 5;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_NS;

	return id;
}

int BalloonManager_ns::hitTestDialogueBalloon(int x, int y) {

	Common::Point p;

	for (uint i = 0; i < _numBalloons; i++) {
		p.x = x - _intBalloons[i].obj->x;
		p.y = y - _intBalloons[i].obj->y;

		if (_intBalloons[i].innerBox.contains(p))
			return i;
	}

	return -1;
}

void BalloonManager_ns::reset() {
	for (uint i = 0; i < _numBalloons; i++) {
		_intBalloons[i].obj = 0;
		_intBalloons[i].surface = 0;	// no need to delete surface, since it is done by Gfx
	}
	_numBalloons = 0;
}











class StringExtent_BR : public WrappedLineFormatter {

	uint	_width, _height;

protected:
	virtual void setup() {
		_width = _height = 0;

		_line.clear();
		_lines = 0;
		_width = 0;
	}

	virtual void action() {
		if (_lineWidth > _width) {
			_width = _lineWidth;
		}
		_height = _lines * _font->height();
	}

	virtual void end() {
		action();
	}

public:
	StringExtent_BR(Font *font) : WrappedLineFormatter(font) { }

	uint width() const { return _width; }
	uint height() const { return _height; }
};


class StringWriter_BR : public WrappedLineFormatter {

	uint	_width, _height;
	byte	_color;
	uint	_x, _y;

	Graphics::Surface	*_surf;

protected:
	StringWriter_BR(Font *font, byte color) : WrappedLineFormatter(font) {

	}

	virtual void setup() {
	}

	virtual void action() {
		if (_line.empty()) {
			return;
		}
		uint16 rx = _x + (_surf->w - _lineWidth) / 2;
		uint16 ry = _y + _lines * _font->height();	// y

		byte *dst = (byte *)_surf->getBasePtr(rx, ry);
		_font->setColor(_color);
		_font->drawString(dst, _surf->w, _line.c_str());
	}

	virtual void end() {
		action();
	}

public:
	StringWriter_BR(Font *font) : WrappedLineFormatter(font) { }

	void write(const Common::String &text, uint maxWidth, byte color, Graphics::Surface *surf) {
		StringExtent_BR	se(_font);
		se.calc(text, maxWidth);
		_width = se.width() + 10;
		_height = se.height() + 12;
		_color = color;
		_surf = surf;

		_x = 0;
		_y = (_surf->h - _height) / 2;
		calc(text, maxWidth);
	}

};




class BalloonManager_br : public BalloonManager {

	Parallaction_br *_vm;
	byte _textColors[3];

	struct Balloon {
		Common::Rect box;
		Graphics::Surface *surface;
		GfxObj	*obj;
	} _intBalloons[3];

	uint	_numBalloons;

	Frames *_leftBalloon;
	Frames *_rightBalloon;

	void cacheAnims();
	int createBalloon(int16 w, int16 h, uint16 borderThickness);
	Balloon *getBalloon(uint id);
	Graphics::Surface *expandBalloon(Frames *data, int frameNum);

	StringWriter_BR	_sw;
	StringExtent_BR _se;

public:
	BalloonManager_br(Parallaction_br *vm, Font *font);
	~BalloonManager_br();

	void reset();
	int setLocationBalloon(const Common::String &text, bool endGame);
	int setDialogueBalloon(const Common::String &text, uint16 winding, TextColor textColor);
	int setSingleBalloon(const Common::String &text, uint16 x, uint16 y, uint16 winding, TextColor textColor);
	void setBalloonText(uint id, const Common::String &text, TextColor textColor);
	int hitTestDialogueBalloon(int x, int y);
};



BalloonManager_br::Balloon* BalloonManager_br::getBalloon(uint id) {
	assert(id < _numBalloons);
	return &_intBalloons[id];
}

Graphics::Surface *BalloonManager_br::expandBalloon(Frames *data, int frameNum) {

	Common::Rect rect;
	data->getRect(frameNum, rect);

	rect.translate(-rect.left, -rect.top);

	Graphics::Surface *surf = new Graphics::Surface;
	surf->create(rect.width(), rect.height(), Graphics::PixelFormat::createFormatCLUT8());

	_vm->_gfx->unpackBlt(rect, data->getData(frameNum), data->getRawSize(frameNum), surf, LAYER_FOREGROUND, 100, BALLOON_TRANSPARENT_COLOR_BR);

	return surf;
}

int BalloonManager_br::setSingleBalloon(const Common::String &text, uint16 x, uint16 y, uint16 winding, TextColor textColor) {
	cacheAnims();

	int id = _numBalloons;
	Frames *src = 0;
	int srcFrame = 0;

	Balloon *balloon = &_intBalloons[id];

	if (winding == 0) {
		src = _rightBalloon;
		srcFrame = 0;
	} else
	if (winding == 1) {
		src = _leftBalloon;
		srcFrame = 0;
	}

	assert(src);

	balloon->surface = expandBalloon(src, srcFrame);
	src->getRect(srcFrame, balloon->box);

	_sw.write(text, 216, _textColors[textColor], balloon->surface);

	// TODO: extract some text to make a name for obj
	balloon->obj = _vm->_gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = x + balloon->box.left;
	balloon->obj->y = y + balloon->box.top;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_BR;

	_numBalloons++;

	return id;
}

int BalloonManager_br::setDialogueBalloon(const Common::String &text, uint16 winding, TextColor textColor) {
	cacheAnims();

	int id = _numBalloons;
	Frames *src = 0;
	int srcFrame = 0;

	Balloon *balloon = &_intBalloons[id];

	if (winding == 0) {
		src = _rightBalloon;
		srcFrame = 0;
	} else
	if (winding == 1) {
		src = _leftBalloon;
		srcFrame = id;
	}

	assert(src);

	balloon->surface = expandBalloon(src, srcFrame);
	src->getRect(srcFrame, balloon->box);

	// TODO: fix text positioning in the Amiga version
	_sw.write(text, 216, _textColors[textColor], balloon->surface);

	// TODO: extract some text to make a name for obj
	balloon->obj = _vm->_gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = balloon->box.left;
	balloon->obj->y = balloon->box.top;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_BR;

	_numBalloons++;

	return id;
}

void BalloonManager_br::setBalloonText(uint id, const Common::String &text, TextColor textColor) {
	Balloon *balloon = getBalloon(id);
	_sw.write(text, 216, _textColors[textColor], balloon->surface);
}

int BalloonManager_br::createBalloon(int16 w, int16 h, uint16 borderThickness) {
	assert(_numBalloons < 5);

	int id = _numBalloons;
	Balloon *balloon = &_intBalloons[id];

	balloon->surface = new Graphics::Surface;
	balloon->surface->create(w, h, Graphics::PixelFormat::createFormatCLUT8());

	Common::Rect rect(w, h);
	balloon->surface->fillRect(rect, 1);
	rect.grow(-borderThickness);
	balloon->surface->fillRect(rect, 15);

	_numBalloons++;

	return id;
}

int BalloonManager_br::setLocationBalloon(const Common::String &text, bool endGame) {
	_se.calc(text, 240);

	int id = createBalloon(_se.width() + 20, _se.height() + 30, 2);
	Balloon *balloon = &_intBalloons[id];

	_sw.write(text, 240, kNormalColor, balloon->surface);

	balloon->obj = _vm->_gfx->registerBalloon(new SurfaceToFrames(balloon->surface), 0);
	balloon->obj->x = 5;
	balloon->obj->y = 5;
	balloon->obj->transparentKey = BALLOON_TRANSPARENT_COLOR_BR;

	return 0;
}

int BalloonManager_br::hitTestDialogueBalloon(int x, int y) {

	for (uint i = 0; i < _numBalloons; i++) {
		if (_intBalloons[i].box.contains(x, y)) {
			return i;
		}
	}

	return -1;
}

void BalloonManager_br::reset() {
	for (uint i = 0; i < _numBalloons; i++) {
		_intBalloons[i].obj = 0;
		_intBalloons[i].surface = 0;	// no need to delete surface, since it is done by Gfx
	}

	_numBalloons = 0;
}

void BalloonManager_br::cacheAnims() {
	if (!_leftBalloon) {
		_leftBalloon = _vm->_disk->loadFrames("fumetto.ani");
		_rightBalloon = _vm->_disk->loadFrames("fumdx.ani");
	}
}



BalloonManager_br::BalloonManager_br(Parallaction_br *vm, Font *font) : _vm(vm), _numBalloons(0),
	_leftBalloon(0), _rightBalloon(0), _sw(font), _se(font) {

	if (_vm->getPlatform() == Common::kPlatformPC) {
		_textColors[kSelectedColor] = 12;
		_textColors[kUnselectedColor] = 0;
		_textColors[kNormalColor] = 0;
	} else {
		_textColors[kSelectedColor] = 11;
		_textColors[kUnselectedColor] = 1;
		_textColors[kNormalColor] = 1;
	}
}

BalloonManager_br::~BalloonManager_br() {
	delete _leftBalloon;
	delete _rightBalloon;
}

void Parallaction_ns::setupBalloonManager() {
	_balloonMan = new BalloonManager_ns(this, _dialogueFont);
}

void Parallaction_br::setupBalloonManager() {
	_balloonMan = new BalloonManager_br(this, _dialogueFont);
}



} // namespace Parallaction
