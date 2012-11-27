/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/rect.h"
#include "common/system.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/controls.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/screen.h"
#include "sword2/sound.h"

#define	MAX_STRING_LEN    64	// 20 was too low; better to be safe ;)
#define CHARACTER_OVERLAP 2	// overlap characters by 3 pixels

// our fonts start on SPACE character (32)
#define SIZE_OF_CHAR_SET  (256 - 32)

namespace Sword2 {

static int baseSlot = 0;

class Widget;

/**
 * Base class for all widgets.
 */

class Widget {
protected:
	Sword2Engine *_vm;
	Dialog *_parent;

	SpriteInfo *_sprites;

	struct WidgetSurface {
		byte *_surface;
		bool _original;
	};

	WidgetSurface *_surfaces;
	int _numStates;
	int _state;

	Common::Rect _hitRect;

public:
	Widget(Dialog *parent, int states);

	virtual ~Widget();

	void createSurfaceImage(int state, uint32 res, int x, int y, uint32 pc);
	void linkSurfaceImage(Widget *from, int state, int x, int y);

	void createSurfaceImages(uint32 res, int x, int y);
	void linkSurfaceImages(Widget *from, int x, int y);

	void setHitRect(int x, int y, int width, int height);
	bool isHit(int16 x, int16 y);

	void setState(int state);
	int getState();

	virtual void paint(Common::Rect *clipRect = NULL);

	virtual void onMouseEnter() {}
	virtual void onMouseExit() {}
	virtual void onMouseMove(int x, int y) {}
	virtual void onMouseDown(int x, int y) {}
	virtual void onMouseUp(int x, int y) {}
	virtual void onWheelUp(int x, int y) {}
	virtual void onWheelDown(int x, int y) {}
	virtual void onKey(KeyboardEvent *ke) {}
	virtual void onTick() {}

	virtual void releaseMouse(int x, int y) {}
};

/**
 * This class is used to draw text in dialogs, buttons, etc.
 */

class FontRendererGui {
private:
	Sword2Engine *_vm;

	struct Glyph {
		byte *_data;
		int _width;
		int _height;
	};

	Glyph _glyph[SIZE_OF_CHAR_SET];

	int _fontId;

public:
	enum {
		kAlignLeft,
		kAlignRight,
		kAlignCenter
	};

	FontRendererGui(Sword2Engine *vm, int fontId);
	~FontRendererGui();

	void fetchText(uint32 textId, byte *buf);

	int getCharWidth(byte c);
	int getCharHeight(byte c);

	int getTextWidth(byte *text);
	int getTextWidth(uint32 textId);

	void drawText(byte *text, int x, int y, int alignment = kAlignLeft);
	void drawText(uint32 textId, int x, int y, int alignment = kAlignLeft);
};

FontRendererGui::FontRendererGui(Sword2Engine *vm, int fontId)
	: _vm(vm), _fontId(fontId) {
	byte *font = _vm->_resman->openResource(fontId);
	SpriteInfo sprite;

	sprite.type = RDSPR_NOCOMPRESSION | RDSPR_TRANS;

	for (int i = 0; i < SIZE_OF_CHAR_SET; i++) {
		byte *frame = _vm->fetchFrameHeader(font, i);

		FrameHeader frame_head;

		frame_head.read(frame);

		sprite.data = frame + FrameHeader::size();
		sprite.w = frame_head.width;
		sprite.h = frame_head.height;
		_vm->_screen->createSurface(&sprite, &_glyph[i]._data);
		_glyph[i]._width = frame_head.width;
		_glyph[i]._height = frame_head.height;
	}

	_vm->_resman->closeResource(fontId);
}

FontRendererGui::~FontRendererGui() {
	for (int i = 0; i < SIZE_OF_CHAR_SET; i++)
		_vm->_screen->deleteSurface(_glyph[i]._data);
}

void FontRendererGui::fetchText(uint32 textId, byte *buf) {
	byte *data = _vm->fetchTextLine(_vm->_resman->openResource(textId / SIZE), textId & 0xffff);
	int i;

	if (buf) {
		for (i = 0; data[i + 2]; i++)
			buf[i] = data[i + 2];
		buf[i] = 0;
	}

	_vm->_resman->closeResource(textId / SIZE);
}

int FontRendererGui::getCharWidth(byte c) {
	if (c < 32)
		return 0;
	return _glyph[c - 32]._width;
}

int FontRendererGui::getCharHeight(byte c) {
	if (c < 32)
		return 0;
	return _glyph[c - 32]._height;
}

int FontRendererGui::getTextWidth(byte *text) {
	int textWidth = 0;

	for (int i = 0; text[i]; i++)
		if (text[i] >= ' ')
			textWidth += (getCharWidth(text[i]) - CHARACTER_OVERLAP);
	return textWidth;
}

int FontRendererGui::getTextWidth(uint32 textId) {
	byte text[MAX_STRING_LEN];

	fetchText(textId, text);
	return getTextWidth(text);
}

void FontRendererGui::drawText(byte *text, int x, int y, int alignment) {
	SpriteInfo sprite;
	int i;

	if (alignment != kAlignLeft) {
		int textWidth = getTextWidth(text);

		switch (alignment) {
		case kAlignRight:
			x -= textWidth;
			break;
		case kAlignCenter:
			x -= (textWidth / 2);
			break;
		}
	}

	sprite.x = x;
	sprite.y = y;

	for (i = 0; text[i]; i++) {
		if (text[i] >= ' ') {
			sprite.w = getCharWidth(text[i]);
			sprite.h = getCharHeight(text[i]);

			_vm->_screen->drawSurface(&sprite, _glyph[text[i] - 32]._data);

			sprite.x += (getCharWidth(text[i]) - CHARACTER_OVERLAP);
		}
	}
}

void FontRendererGui::drawText(uint32 textId, int x, int y, int alignment) {
	byte text[MAX_STRING_LEN];

	fetchText(textId, text);
	drawText(text, x, y, alignment);
}

//
// Dialog class functions
//

Dialog::Dialog(Sword2Engine *vm)
	: _numWidgets(0), _finish(false), _result(0), _vm(vm) {
	_vm->_screen->setFullPalette(CONTROL_PANEL_PALETTE);
	_vm->_screen->clearScene();
	_vm->_screen->updateDisplay();

	// Usually the mouse pointer will already be "normal", but not always.
	_vm->_mouse->setMouse(NORMAL_MOUSE_ID);

	// Force mouse mode as system menu: normally not needed,
	// but value is not correct in case of game start dialog
	// (when asking to restart or load a game).
	// This is forced to avoid GMM loading/saving being enabled
	// during initial dialog.
	_vm->_mouse->setMouseMode(MOUSE_system_menu);
}

Dialog::~Dialog() {
	for (int i = 0; i < _numWidgets; i++)
		delete _widgets[i];
	_vm->_screen->clearScene();
	_vm->_screen->updateDisplay();
}

void Dialog::registerWidget(Widget *widget) {
	if (_numWidgets < MAX_WIDGETS)
		_widgets[_numWidgets++] = widget;
}

void Dialog::paint() {
	_vm->_screen->clearScene();
	for (int i = 0; i < _numWidgets; i++)
		_widgets[i]->paint();
}

void Dialog::setResult(int result) {
	_result = result;
	_finish = true;
}

int Dialog::runModal() {
	uint32 oldFilter = _vm->setInputEventFilter(0);

	int i;

	paint();

	int oldMouseX = -1;
	int oldMouseY = -1;

	while (!_finish) {
		// So that the menu icons will reach their full size
		_vm->_mouse->processMenu();
		_vm->_screen->updateDisplay(false);

		int newMouseX, newMouseY;

		_vm->_mouse->getPos(newMouseX, newMouseY);

		newMouseY += 40;

		MouseEvent *me = _vm->mouseEvent();
		KeyboardEvent *ke = _vm->keyboardEvent();

		if (ke) {
			if (ke->kbd.keycode == Common::KEYCODE_ESCAPE)
				setResult(0);
			else if (ke->kbd.keycode == Common::KEYCODE_RETURN || ke->kbd.keycode == Common::KEYCODE_KP_ENTER)
				setResult(1);
		}

		int oldHit = -1;
		int newHit = -1;

		// Find out which widget the mouse was over the last time, and
		// which it is currently over. This assumes the widgets do not
		// overlap.

		for (i = 0; i < _numWidgets; i++) {
			if (_widgets[i]->isHit(oldMouseX, oldMouseY))
				oldHit = i;
			if (_widgets[i]->isHit(newMouseX, newMouseY))
				newHit = i;
		}

		// Was the mouse inside a widget the last time?

		if (oldHit >= 0) {
			if (newHit != oldHit)
				_widgets[oldHit]->onMouseExit();
		}

		// Is the mouse currently in a widget?

		if (newHit >= 0) {
			if (newHit != oldHit)
				_widgets[newHit]->onMouseEnter();

			if (me) {
				switch (me->buttons) {
				case RD_LEFTBUTTONDOWN:
					_widgets[newHit]->onMouseDown(newMouseX, newMouseY);
					break;
				case RD_LEFTBUTTONUP:
					_widgets[newHit]->onMouseUp(newMouseX, newMouseY);
					break;
				case RD_WHEELUP:
					_widgets[newHit]->onWheelUp(newMouseX, newMouseY);
					break;
				case RD_WHEELDOWN:
					_widgets[newHit]->onWheelDown(newMouseX, newMouseY);
					break;
				}
			}
		}

		// Some events are passed to the widgets regardless of where
		// the mouse cursor is.

		for (i = 0; i < _numWidgets; i++) {
			if (me && me->buttons == RD_LEFTBUTTONUP) {
				// So that slider widgets will know when the
				// user releases the mouse button, even if the
				// cursor is outside of the slider's hit area.
				_widgets[i]->releaseMouse(newMouseX, newMouseY);
			}

			// This is to make it easier to drag the slider widget

			if (newMouseX != oldMouseX || newMouseY != oldMouseY)
				_widgets[i]->onMouseMove(newMouseX, newMouseY);

			if (ke)
				_widgets[i]->onKey(ke);

			_widgets[i]->onTick();
		}

		oldMouseX = newMouseX;
		oldMouseY = newMouseY;

		_vm->_system->delayMillis(20);

		if (_vm->shouldQuit())
			setResult(0);
	}

	_vm->setInputEventFilter(oldFilter);
	return _result;
}

//
// Widget functions
//

Widget::Widget(Dialog *parent, int states)
	: _vm(parent->_vm), _parent(parent), _numStates(states), _state(0) {
	_sprites = (SpriteInfo *)calloc(states, sizeof(SpriteInfo));
	_surfaces = (WidgetSurface *)calloc(states, sizeof(WidgetSurface));

	_hitRect.left = _hitRect.right = _hitRect.top = _hitRect.bottom = -1;
}

Widget::~Widget() {
	for (int i = 0; i < _numStates; i++) {
		if (_surfaces[i]._original)
			_vm->_screen->deleteSurface(_surfaces[i]._surface);
	}
	free(_sprites);
	free(_surfaces);
}

void Widget::createSurfaceImage(int state, uint32 res, int x, int y, uint32 pc) {
	byte *file, *colTablePtr = NULL;
	AnimHeader anim_head;
	FrameHeader frame_head;
	CdtEntry cdt_entry;
	uint32 spriteType = RDSPR_TRANS;

	// open anim resource file, point to base
	file = _vm->_resman->openResource(res);

	byte *frame = _vm->fetchFrameHeader(file, pc);

	anim_head.read(_vm->fetchAnimHeader(file));
	cdt_entry.read(_vm->fetchCdtEntry(file, pc));
	frame_head.read(frame);

	// If the frame is flipped. (Only really applicable to frames using
	// offsets.)

	if (cdt_entry.frameType & FRAME_FLIPPED)
		spriteType |= RDSPR_FLIP;

	// Which compression was used?

	switch (anim_head.runTimeComp) {
	case NONE:
		spriteType |= RDSPR_NOCOMPRESSION;
		break;
	case RLE256:
		spriteType |= RDSPR_RLE256;
		break;
	case RLE16:
		spriteType |= RDSPR_RLE256;
		// Points to just after last cdt_entry, i.e. start of color
		// table
		colTablePtr = _vm->fetchAnimHeader(file) + AnimHeader::size()
			+ anim_head.noAnimFrames * CdtEntry::size();
		break;
	}

	_sprites[state].x = x;
	_sprites[state].y = y;
	_sprites[state].w = frame_head.width;
	_sprites[state].h = frame_head.height;
	_sprites[state].scale = 0;
	_sprites[state].type = spriteType;
	_sprites[state].blend = anim_head.blend;

	// Points to just after frame header, ie. start of sprite data
	_sprites[state].data = frame + FrameHeader::size();
	_sprites[state].colorTable = colTablePtr;
	_sprites[state].isText = false;

	_vm->_screen->createSurface(&_sprites[state], &_surfaces[state]._surface);
	_surfaces[state]._original = true;

	// Release the anim resource
	_vm->_resman->closeResource(res);
}

void Widget::linkSurfaceImage(Widget *from, int state, int x, int y) {
	_sprites[state].x = x;
	_sprites[state].y = y;
	_sprites[state].w = from->_sprites[state].w;
	_sprites[state].h = from->_sprites[state].h;
	_sprites[state].scale = from->_sprites[state].scale;
	_sprites[state].type = from->_sprites[state].type;
	_sprites[state].blend = from->_sprites[state].blend;

	_surfaces[state]._surface = from->_surfaces[state]._surface;
	_surfaces[state]._original = false;
}

void Widget::createSurfaceImages(uint32 res, int x, int y) {
	for (int i = 0; i < _numStates; i++)
		createSurfaceImage(i, res, x, y, i);
}

void Widget::linkSurfaceImages(Widget *from, int x, int y) {
	for (int i = 0; i < from->_numStates; i++)
		linkSurfaceImage(from, i, x, y);
}

void Widget::setHitRect(int x, int y, int width, int height) {
	_hitRect.left = x;
	_hitRect.right = x + width;
	_hitRect.top = y;
	_hitRect.bottom = y + height;
}

bool Widget::isHit(int16 x, int16 y) {
	return _hitRect.left >= 0 && _hitRect.contains(x, y);
}

void Widget::setState(int state) {
	if (state != _state) {
		_state = state;
		paint();
	}
}

int Widget::getState() {
	return _state;
}

void Widget::paint(Common::Rect *clipRect) {
	_vm->_screen->drawSurface(&_sprites[_state], _surfaces[_state]._surface, clipRect);
}

/**
 * Standard button class.
 */

class Button : public Widget {
public:
	Button(Dialog *parent, int x, int y, int w, int h)
		: Widget(parent, 2) {
		setHitRect(x, y, w, h);
	}

	virtual void onMouseExit() {
		setState(0);
	}

	virtual void onMouseDown(int x, int y) {
		setState(1);
	}

	virtual void onMouseUp(int x, int y) {
		if (getState() != 0) {
			setState(0);
			_parent->onAction(this);
		}
	}
};

/**
 * Scroll buttons are used to scroll the savegame list. The difference between
 * this and a normal button is that we want this to repeat.
 */

class ScrollButton : public Widget {
private:
	uint32 _holdCounter;

public:
	ScrollButton(Dialog *parent, int x, int y, int w, int h)
		: Widget(parent, 2), _holdCounter(0) {
		setHitRect(x, y, w, h);
	}

	virtual void onMouseExit() {
		setState(0);
	}

	virtual void onMouseDown(int x, int y) {
		setState(1);
		_parent->onAction(this);
		_holdCounter = 0;
	}

	virtual void onMouseUp(int x, int y) {
		setState(0);
	}

	virtual void onTick() {
		if (getState() != 0) {
			_holdCounter++;
			if (_holdCounter > 16 && (_holdCounter % 4) == 0)
				_parent->onAction(this);
		}
	}
};

/**
 * A switch is a button that changes state when clicked, and keeps that state
 * until clicked again.
 */

class Switch : public Widget {
private:
	bool _holding, _value;
	int _upState, _downState;

public:
	Switch(Dialog *parent, int x, int y, int w, int h)
		: Widget(parent, 2), _holding(false), _value(false),
		  _upState(0), _downState(1) {
		setHitRect(x, y, w, h);
	}

	// The sound mute switches have 0 as their "down" state and 1 as
	// their "up" state, so this function is needed to get consistent
	// behavior.

	void reverseStates() {
		_upState = 1;
		_downState = 0;
	}

	void setValue(bool value) {
		_value = value;
		if (_value)
			setState(_downState);
		else
			setState(_upState);
	}

	bool getValue() {
		return _value;
	}

	virtual void onMouseExit() {
		if (_holding && !_value)
			setState(_upState);
		_holding = false;
	}

	virtual void onMouseDown(int x, int y) {
		_holding = true;
		setState(_downState);
	}

	virtual void onMouseUp(int x, int y) {
		if (_holding) {
			_holding = false;
			_value = !_value;
			if (_value)
				setState(_downState);
			else
				setState(_upState);
			_parent->onAction(this, getState());
		}
	}
};

/**
 * A slider is used to specify a value within a pre-defined range.
 */

class Slider : public Widget {
private:
	Widget *_background;
	bool _dragging;
	int _value, _targetValue;
	int _maxValue;
	int _valueStep;
	int _dragOffset;

	int posFromValue(int value) {
		return _hitRect.left + (value * (_hitRect.width() - 38)) / _maxValue;
	}

	int valueFromPos(int x) {
		return (int)((double)(_maxValue * (x - _hitRect.left)) / (double)(_hitRect.width() - 38) + 0.5);
	}

public:
	Slider(Dialog *parent, Widget *background, int max,
		int x, int y, int w, int h, int step, Widget *base = NULL)
		: Widget(parent, 1), _background(background),
		  _dragging(false), _value(0), _targetValue(0),
		  _maxValue(max), _valueStep(step) {
		setHitRect(x, y, w, h);

		if (_valueStep <= 0)
			_valueStep = 1;

		if (base)
			linkSurfaceImages(base, x, y);
		else
			createSurfaceImages(3406, x, y);
	}

	virtual void paint(Common::Rect *clipRect = NULL) {
		// This will redraw a bit more than is strictly necessary,
		// but I doubt that will make any noticeable difference.

		_background->paint(&_hitRect);
		Widget::paint(clipRect);
	}

	void setValue(int value) {
		_value = value;
		_targetValue = value;
		_sprites[0].x = posFromValue(_value);
		paint();
	}

	int getValue() {
		return _value;
	}

	virtual void onMouseMove(int x, int y) {
		if (_dragging) {
			int newX = x - _dragOffset;
			int newValue;

			if (newX < _hitRect.left)
				newX = _hitRect.left;
			else if (newX + 38 > _hitRect.right)
				newX = _hitRect.right - 38;

			_sprites[0].x = newX;

			newValue = valueFromPos(newX);
			if (newValue != _value) {
				_value = newValue;
				_targetValue = newValue;
				_parent->onAction(this, newValue);
			}

			paint();
		}
	}

	virtual void onMouseDown(int x, int y) {
		if (x >= _sprites[0].x && x < _sprites[0].x + 38) {
			_dragging = true;
			_dragOffset = x - _sprites[0].x;
		} else if (x < _sprites[0].x) {
			if (_targetValue >= _valueStep)
				_targetValue -= _valueStep;
			else
				_targetValue = 0;
		} else {
			if (_targetValue < _maxValue - _valueStep)
				_targetValue += _valueStep;
			else
				_targetValue = _maxValue;
		}
	}

	virtual void releaseMouse(int x, int y) {
		if (_dragging)
			_dragging = false;
	}

	virtual void onTick() {
		if (!_dragging) {
			int target = posFromValue(_targetValue);

			if (target != _sprites[0].x) {
				if (target < _sprites[0].x) {
					_sprites[0].x -= 4;
					if (_sprites[0].x < target)
						_sprites[0].x = target;
				} else if (target > _sprites[0].x) {
					_sprites[0].x += 4;
					if (_sprites[0].x > target)
						_sprites[0].x = target;
				}

				int newValue = valueFromPos(_sprites[0].x);
				if (newValue != _value) {
					_value = newValue;
					_parent->onAction(this, newValue);
				}

				paint();
			}
		}
	}
};

/**
 * The "mini" dialog.
 */

MiniDialog::MiniDialog(Sword2Engine *vm, uint32 headerTextId, uint32 okTextId, uint32 cancelTextId) : Dialog(vm) {
	_headerTextId = headerTextId;
	_okTextId = okTextId;
	_cancelTextId = cancelTextId;

	_fr = new FontRendererGui(_vm, _vm->_controlsFontId);

	_panel = new Widget(this, 1);
	_panel->createSurfaceImages(1996, 203, 104);

	_okButton = new Button(this, 243, 214, 24, 24);
	_okButton->createSurfaceImages(2002, 243, 214);

	_cancelButton = new Button(this, 243, 276, 24, 24);
	_cancelButton->linkSurfaceImages(_okButton, 243, 276);

	registerWidget(_panel);
	registerWidget(_okButton);
	registerWidget(_cancelButton);
}

MiniDialog::~MiniDialog() {
	delete _fr;
}

void MiniDialog::paint() {
	Dialog::paint();

	if (_headerTextId)
		_fr->drawText(_headerTextId, 310, 134, FontRendererGui::kAlignCenter);
	_fr->drawText(_okTextId, 270, 214);
	_fr->drawText(_cancelTextId, 270, 276);
}

void MiniDialog::onAction(Widget *widget, int result) {
	if (widget == _okButton)
		setResult(1);
	else if (widget == _cancelButton)
		setResult(0);
}

StartDialog::StartDialog(Sword2Engine *vm) : MiniDialog(vm, 0) {}

int StartDialog::runModal() {
	while (1) {
		MiniDialog startDialog(_vm, 0, TEXT_RESTART, TEXT_RESTORE);

		if (startDialog.runModal())
			return 1;

		if (_vm->shouldQuit())
			return 0;

		RestoreDialog restoreDialog(_vm);

		if (restoreDialog.runModal())
			return 0;

		if (_vm->shouldQuit())
			return 0;
	}

	return 1;
}

/**
 * The restart dialog.
 */

RestartDialog::RestartDialog(Sword2Engine *vm) : MiniDialog(vm, TEXT_RESTART) {}

int RestartDialog::runModal() {
	int result = MiniDialog::runModal();

	if (result)
		_vm->restartGame();

	return result;
}

/**
 * The quit dialog.
 */

QuitDialog::QuitDialog(Sword2Engine *vm) : MiniDialog(vm, TEXT_QUIT) {}

int QuitDialog::runModal() {
	int result = MiniDialog::runModal();

	if (result)
		_vm->quitGame();

	return result;
}

/**
 * The game settings dialog.
 */

OptionsDialog::OptionsDialog(Sword2Engine *vm) : Dialog(vm) {
	_fr = new FontRendererGui(_vm, _vm->_controlsFontId);

	_mixer = _vm->_mixer;

	_panel = new Widget(this, 1);
	_panel->createSurfaceImages(3405, 0, 40);

	_objectLabelsSwitch = new Switch(this, 304, 100, 53, 32);
	_objectLabelsSwitch->createSurfaceImages(3687, 304, 100);

	_subtitlesSwitch = new Switch(this, 510, 100, 53, 32);
	_subtitlesSwitch->linkSurfaceImages(_objectLabelsSwitch, 510, 100);

	_reverseStereoSwitch = new Switch(this, 304, 293, 53, 32);
	_reverseStereoSwitch->linkSurfaceImages(_objectLabelsSwitch, 304, 293);

	_musicSwitch = new Switch(this, 516, 157, 40, 32);
	_musicSwitch->createSurfaceImages(3315, 516, 157);
	_musicSwitch->reverseStates();

	_speechSwitch = new Switch(this, 516, 205, 40, 32);
	_speechSwitch->linkSurfaceImages(_musicSwitch, 516, 205);
	_speechSwitch->reverseStates();

	_fxSwitch = new Switch(this, 516, 250, 40, 32);
	_fxSwitch->linkSurfaceImages(_musicSwitch, 516, 250);
	_fxSwitch->reverseStates();

	int volStep = Audio::Mixer::kMaxMixerVolume / 10;

	_musicSlider = new Slider(this, _panel, Audio::Mixer::kMaxMixerVolume, 309, 161, 170, 27, volStep);
	_speechSlider = new Slider(this, _panel, Audio::Mixer::kMaxMixerVolume, 309, 208, 170, 27, volStep, _musicSlider);
	_fxSlider = new Slider(this, _panel, Audio::Mixer::kMaxMixerVolume, 309, 254, 170, 27, volStep, _musicSlider);
	_gfxSlider = new Slider(this, _panel, 3, 309, 341, 170, 27, 1, _musicSlider);

	_gfxPreview = new Widget(this, 4);
	_gfxPreview->createSurfaceImages(256, 495, 310);

	_okButton = new Button(this, 203, 382, 53, 32);
	_okButton->createSurfaceImages(901, 203, 382);

	_cancelButton = new Button(this, 395, 382, 53, 32);
	_cancelButton->linkSurfaceImages(_okButton, 395, 382);

	registerWidget(_panel);
	registerWidget(_objectLabelsSwitch);
	registerWidget(_subtitlesSwitch);
	registerWidget(_reverseStereoSwitch);
	registerWidget(_musicSwitch);
	registerWidget(_speechSwitch);
	registerWidget(_fxSwitch);
	registerWidget(_musicSlider);
	registerWidget(_speechSlider);
	registerWidget(_fxSlider);
	registerWidget(_gfxSlider);
	registerWidget(_gfxPreview);
	registerWidget(_okButton);
	registerWidget(_cancelButton);

	_objectLabelsSwitch->setValue(_vm->_mouse->getObjectLabels());
	_subtitlesSwitch->setValue(_vm->getSubtitles());
	_reverseStereoSwitch->setValue(_vm->_sound->isReverseStereo());
	_musicSwitch->setValue(!_vm->_sound->isMusicMute());
	_speechSwitch->setValue(!_vm->_sound->isSpeechMute());
	_fxSwitch->setValue(!_vm->_sound->isFxMute());

	_musicSlider->setValue(_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
	_speechSlider->setValue(_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType));
	_fxSlider->setValue(_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType));

	_gfxSlider->setValue(_vm->_screen->getRenderLevel());
	_gfxPreview->setState(_vm->_screen->getRenderLevel());
}

OptionsDialog::~OptionsDialog() {
	delete _fr;
}

void OptionsDialog::paint() {
	Dialog::paint();

	int maxWidth = 0;
	int width;

	uint32 alignTextIds[] = {
		TEXT_OBJECT_LABELS,
		TEXT_MUSIC_VOLUME,
		TEXT_SPEECH_VOLUME,
		TEXT_FX_VOLUME,
		TEXT_GFX_QUALITY,
		TEXT_REVERSE_STEREO
	};

	for (int i = 0; i < ARRAYSIZE(alignTextIds); i++) {
		width = _fr->getTextWidth(alignTextIds[i]);
		if (width > maxWidth)
			maxWidth = width;
	}

	_fr->drawText(TEXT_OPTIONS, 321, 55, FontRendererGui::kAlignCenter);
	_fr->drawText(TEXT_SUBTITLES, 500, 103, FontRendererGui::kAlignRight);
	_fr->drawText(TEXT_OBJECT_LABELS, 299 - maxWidth, 103);
	_fr->drawText(TEXT_MUSIC_VOLUME, 299 - maxWidth, 161);
	_fr->drawText(TEXT_SPEECH_VOLUME, 299 - maxWidth, 208);
	_fr->drawText(TEXT_FX_VOLUME, 299 - maxWidth, 254);
	_fr->drawText(TEXT_REVERSE_STEREO, 299 - maxWidth, 296);
	_fr->drawText(TEXT_GFX_QUALITY, 299 - maxWidth, 341);
	_fr->drawText(TEXT_OK, 193, 382, FontRendererGui::kAlignRight);
	_fr->drawText(TEXT_CANCEL, 385, 382, FontRendererGui::kAlignRight);
}

void OptionsDialog::onAction(Widget *widget, int result) {
	// Since there is music playing while the dialog is displayed we need
	// to update music volume immediately.

	if (widget == _musicSwitch) {
		_vm->_sound->muteMusic(result != 0);
	} else if (widget == _musicSlider) {
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, result);
		_vm->_sound->muteMusic(result == 0);
		_musicSwitch->setValue(result != 0);
	} else if (widget == _speechSlider) {
		_speechSwitch->setValue(result != 0);
	} else if (widget == _fxSlider) {
		_fxSwitch->setValue(result != 0);
	} else if (widget == _gfxSlider) {
		_gfxPreview->setState(result);
		_vm->_screen->setRenderLevel(result);
	} else if (widget == _okButton) {
		// Apply the changes
		_vm->setSubtitles(_subtitlesSwitch->getValue());
		_vm->_mouse->setObjectLabels(_objectLabelsSwitch->getValue());
		_vm->_sound->muteMusic(!_musicSwitch->getValue());
		_vm->_sound->muteSpeech(!_speechSwitch->getValue());
		_vm->_sound->muteFx(!_fxSwitch->getValue());
		_vm->_sound->setReverseStereo(_reverseStereoSwitch->getValue());
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, _musicSlider->getValue());
		_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, _speechSlider->getValue());
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, _fxSlider->getValue());
		_vm->_screen->setRenderLevel(_gfxSlider->getValue());

		_vm->writeSettings();
		setResult(1);
	} else if (widget == _cancelButton) {
		// Revert the changes
		_vm->readSettings();
		setResult(0);
	}
}

// Slot button actions. Note that keyboard input generates positive actions

enum {
	kSelectSlot = -1,
	kDeselectSlot = -2,
	kWheelDown = -3,
	kWheelUp = -4,
	kStartEditing = -5,
	kCursorTick = -6
};

class Slot : public Widget {
private:
	int _mode;
	FontRendererGui *_fr;
	byte _text[SAVE_DESCRIPTION_LEN];
	bool _clickable;
	bool _editable;

public:
	Slot(Dialog *parent, int x, int y, int w, int h)
		: Widget(parent, 2), _clickable(false), _editable(false) {
		setHitRect(x, y, w, h);
		_text[0] = 0;
	}

	void setMode(int mode) {
		_mode = mode;
	}

	void setClickable(bool clickable) {
		_clickable = clickable;
	}

	void setEditable(bool editable) {
		_editable = editable;
		_vm->_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, editable);
	}

	bool isEditable() {
		return _editable;
	}

	void setText(FontRendererGui *fr, int slot, byte *text) {
		_fr = fr;
		if (text)
			sprintf((char *)_text, "%d.  %s", slot, text);
		else
			sprintf((char *)_text, "%d.  ", slot);
	}

	byte *getText() {
		return &_text[0];
	}

	virtual void paint(Common::Rect *clipRect = NULL) {
		Widget::paint();

		// HACK: The main dialog is responsible for drawing the text
		// when in editing mode.

		if (!_editable)
			_fr->drawText(_text, _sprites[0].x + 16, _sprites[0].y + 4 + 2 * getState());
	}

	virtual void onMouseDown(int x, int y) {
		if (_clickable) {
			if (getState() == 0) {
				setState(1);
				_parent->onAction(this, kSelectSlot);
				if (_mode == kSaveDialog)
					_parent->onAction(this, kStartEditing);
			} else if (_mode == kRestoreDialog) {
				setState(0);
				_parent->onAction(this, kDeselectSlot);
			}
		}
	}

	virtual void onWheelUp(int x, int y) {
		_parent->onAction(this, kWheelUp);
	}

	virtual void onWheelDown(int x, int y) {
		_parent->onAction(this, kWheelDown);
	}

	virtual void onKey(KeyboardEvent *ke) {
		if (_editable) {
			if (ke->kbd.keycode == Common::KEYCODE_BACKSPACE)
				_parent->onAction(this, Common::KEYCODE_BACKSPACE);
			else if (ke->kbd.ascii >= ' ' && ke->kbd.ascii <= 255) {
				// Accept the character if the font renderer
				// has what looks like a valid glyph for it.
				if (_fr->getCharWidth(ke->kbd.ascii))
					_parent->onAction(this, ke->kbd.ascii);
			}
		}
	}

	virtual void onTick() {
		if (_editable)
			_parent->onAction(this, kCursorTick);
	}

	void setY(int y) {
		for (int i = 0; i < _numStates; i++)
			_sprites[i].y = y;
		setHitRect(_hitRect.left, y, _hitRect.width(), _hitRect.height());
	}

	int getY() {
		return _sprites[0].y;
	}
};

SaveRestoreDialog::SaveRestoreDialog(Sword2Engine *vm, int mode) : Dialog(vm) {
	int i;

	_mode = mode;
	_selectedSlot = -1;

	// FIXME: The "control font" and the "red font" are currently always
	// the same font, so one should be eliminated.

	_fr1 = new FontRendererGui(_vm, _vm->_controlsFontId);
	_fr2 = new FontRendererGui(_vm, _vm->_redFontId);

	_panel = new Widget(this, 1);
	_panel->createSurfaceImages(2016, 0, 40);

	for (i = 0; i < 4; i++) {
		_slotButton[i] = new Slot(this, 114, 0, 384, 36);
		_slotButton[i]->createSurfaceImages(2006 + i, 114, 0);
		_slotButton[i]->setMode(mode);
		_slotButton[i + 4] = new Slot(this, 114, 0, 384, 36);
		_slotButton[i + 4]->linkSurfaceImages(_slotButton[i], 114, 0);
		_slotButton[i + 4]->setMode(mode);
	}

	updateSlots();

	_zupButton = new ScrollButton(this, 516, 65, 17, 17);
	_zupButton->createSurfaceImages(1982, 516, 65);

	_upButton = new ScrollButton(this, 516, 85, 17, 17);
	_upButton->createSurfaceImages(2067, 516, 85);

	_downButton = new ScrollButton(this, 516, 329, 17, 17);
	_downButton->createSurfaceImages(1986, 516, 329);

	_zdownButton = new ScrollButton(this, 516, 350, 17, 17);
	_zdownButton->createSurfaceImages(1988, 516, 350);

	_okButton = new Button(this, 130, 377, 24, 24);
	_okButton->createSurfaceImages(2002, 130, 377);

	_cancelButton = new Button(this, 350, 377, 24, 24);
	_cancelButton->linkSurfaceImages(_okButton, 350, 377);

	registerWidget(_panel);

	for (i = 0; i < 8; i++)
		registerWidget(_slotButton[i]);

	registerWidget(_zupButton);
	registerWidget(_upButton);
	registerWidget(_downButton);
	registerWidget(_zdownButton);
	registerWidget(_okButton);
	registerWidget(_cancelButton);
}

SaveRestoreDialog::~SaveRestoreDialog() {
	delete _fr1;
	delete _fr2;
}

// There aren't really a hundred different button objects of course, there are
// only eight. Re-arrange them to simulate scrolling.

void SaveRestoreDialog::updateSlots() {
	for (int i = 0; i < 8; i++) {
		Slot *slot = _slotButton[(baseSlot + i) % 8];
		FontRendererGui *fr;
		byte description[SAVE_DESCRIPTION_LEN];

		slot->setY(72 + i * 36);

		if (baseSlot + i == _selectedSlot) {
			slot->setEditable(_mode == kSaveDialog);
			slot->setState(1);
			fr = _fr2;
		} else {
			slot->setEditable(false);
			slot->setState(0);
			fr = _fr1;
		}

		if (_vm->getSaveDescription(baseSlot + i, description) == SR_OK) {
			slot->setText(fr, baseSlot + i, description);
			slot->setClickable(true);
		} else {
			slot->setText(fr, baseSlot + i, NULL);
			slot->setClickable(_mode == kSaveDialog);
		}

		if (slot->isEditable())
			drawEditBuffer(slot);
		else
			slot->paint();
	}
}

void SaveRestoreDialog::drawEditBuffer(Slot *slot) {
	if (_selectedSlot == -1)
		return;

	// This will redraw a bit more than is strictly necessary, but I doubt
	// that will make any noticeable difference.

	slot->paint();
	_fr2->drawText(_editBuffer, 130, 78 + (_selectedSlot - baseSlot) * 36);
}

void SaveRestoreDialog::onAction(Widget *widget, int result) {
	if (widget == _zupButton) {
		if (baseSlot > 0) {
			if (baseSlot >= 8)
				baseSlot -= 8;
			else
				baseSlot = 0;
			updateSlots();
		}
	} else if (widget == _upButton) {
		if (baseSlot > 0) {
			baseSlot--;
			updateSlots();
		}
	} else if (widget == _downButton) {
		if (baseSlot < 92) {
			baseSlot++;
			updateSlots();
		}
	} else if (widget == _zdownButton) {
		if (baseSlot < 92) {
			if (baseSlot <= 84)
				baseSlot += 8;
			else
				baseSlot = 92;
			updateSlots();
		}
	} else if (widget == _okButton) {
		setResult(1);
	} else if (widget == _cancelButton) {
		setResult(0);
	} else {
		Slot *slot = (Slot *)widget;
		int textWidth;
		byte tmp;
		int i;
		int j;

		switch (result) {
		case kWheelUp:
			onAction(_upButton);
			break;
		case kWheelDown:
			onAction(_downButton);
			break;
		case kSelectSlot:
		case kDeselectSlot:
			if (result == kSelectSlot)
				_selectedSlot = baseSlot + (slot->getY() - 72) / 35;
			else if (result == kDeselectSlot)
				_selectedSlot = -1;

			for (i = 0; i < 8; i++)
				if (widget == _slotButton[i])
					break;

			for (j = 0; j < 8; j++) {
				if (j != i) {
					_slotButton[j]->setEditable(false);
					_slotButton[j]->setState(0);
				}
			}
			break;
		case kStartEditing:
			if (_selectedSlot >= 10)
				_firstPos = 5;
			else
				_firstPos = 4;

			strcpy((char *)_editBuffer, (char *)slot->getText());
			_editPos = strlen((char *)_editBuffer);
			_cursorTick = 0;
			_editBuffer[_editPos] = '_';
			_editBuffer[_editPos + 1] = 0;
			slot->setEditable(true);
			drawEditBuffer(slot);
			break;
		case kCursorTick:
			_cursorTick++;
			if (_cursorTick == 7) {
				_editBuffer[_editPos] = ' ';
				drawEditBuffer(slot);
			} else if (_cursorTick == 14) {
				_cursorTick = 0;
				_editBuffer[_editPos] = '_';
				drawEditBuffer(slot);
			}
			break;
		case Common::KEYCODE_BACKSPACE:
			if (_editPos > _firstPos) {
				_editBuffer[_editPos - 1] = _editBuffer[_editPos];
				_editBuffer[_editPos--] = 0;
				drawEditBuffer(slot);
			}
			break;
		default:
			tmp = _editBuffer[_editPos];
			_editBuffer[_editPos] = 0;
			textWidth = _fr2->getTextWidth(_editBuffer);
			_editBuffer[_editPos] = tmp;

			if (textWidth < 340 && _editPos < SAVE_DESCRIPTION_LEN - 2) {
				_editBuffer[_editPos + 1] = _editBuffer[_editPos];
				_editBuffer[_editPos + 2] = 0;
				_editBuffer[_editPos++] = result;
				drawEditBuffer(slot);
			}
			break;
		}
	}
}

void SaveRestoreDialog::paint() {
	Dialog::paint();

	_fr1->drawText((_mode == kRestoreDialog) ? TEXT_RESTORE : TEXT_SAVE, 165, 377);
	_fr1->drawText(TEXT_CANCEL, 382, 377);
}

void SaveRestoreDialog::setResult(int result) {
	if (result) {
		if (_selectedSlot == -1)
			return;

		if (_mode == kSaveDialog) {
			if (_editPos <= _firstPos)
				return;
		}
	}

	Dialog::setResult(result);
}

int SaveRestoreDialog::runModal() {
	int result = Dialog::runModal();

	if (result) {
		switch (_mode) {
		case kSaveDialog:
			// Remove the cursor character from the savegame name
			_editBuffer[_editPos] = 0;

			if (_vm->saveGame(_selectedSlot, (byte *)&_editBuffer[_firstPos]) != SR_OK)
				result = 0;
			break;
		case kRestoreDialog:
			if (_vm->restoreGame(_selectedSlot) != SR_OK)
				result = 0;
			break;
		}
	}

	return result;
}

} // End of namespace Sword2
