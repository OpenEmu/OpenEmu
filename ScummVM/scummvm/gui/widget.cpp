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

#include "common/scummsys.h"
#include "common/system.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "graphics/pixelformat.h"
#include "gui/widget.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

#include "gui/dialog.h"

namespace GUI {

Widget::Widget(GuiObject *boss, int x, int y, int w, int h, const char *tooltip)
	: GuiObject(x, y, w, h), _type(0), _boss(boss), _tooltip(tooltip),
	  _id(0), _flags(0), _hasFocus(false), _state(ThemeEngine::kStateEnabled) {
	init();
}

Widget::Widget(GuiObject *boss, const Common::String &name, const char *tooltip)
	: GuiObject(name), _type(0), _boss(boss), _tooltip(tooltip),
	  _id(0), _flags(0), _hasFocus(false), _state(ThemeEngine::kStateDisabled) {
	init();
}

void Widget::init() {
	// Insert into the widget list of the boss
	_next = _boss->_firstWidget;
	_boss->_firstWidget = this;
}

Widget::~Widget() {
	delete _next;
	_next = 0;
}

void Widget::resize(int x, int y, int w, int h) {
	_x = x;
	_y = y;
	_w = w;
	_h = h;
}

void Widget::setFlags(int flags) {
	updateState(_flags, _flags | flags);
	_flags |= flags;
}

void Widget::clearFlags(int flags) {
	updateState(_flags, _flags & ~flags);
	_flags &= ~flags;
}

void Widget::updateState(int oldFlags, int newFlags) {
	if (newFlags & WIDGET_ENABLED) {
		_state = ThemeEngine::kStateEnabled;
		if (newFlags & WIDGET_HILITED)
			_state = ThemeEngine::kStateHighlight;
		if (newFlags & WIDGET_PRESSED)
			_state = ThemeEngine::kStatePressed;
	} else {
		_state = ThemeEngine::kStateDisabled;
	}
}

void Widget::draw() {
	if (!isVisible() || !_boss->isVisible())
		return;

	int oldX = _x, oldY = _y;

	// Account for our relative position in the dialog
	_x = getAbsX();
	_y = getAbsY();

	// Draw border
	if (_flags & WIDGET_BORDER) {
		g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x+_w, _y+_h), 0, ThemeEngine::kWidgetBackgroundBorder);
		_x += 4;
		_y += 4;
		_w -= 8;
		_h -= 8;
	}

	// Now perform the actual widget draw
	drawWidget();

	// Restore x/y
	if (_flags & WIDGET_BORDER) {
		_x -= 4;
		_y -= 4;
		_w += 8;
		_h += 8;
	}

	_x = oldX;
	_y = oldY;

	// Draw all children
	Widget *w = _firstWidget;
	while (w) {
		w->draw();
		w = w->_next;
	}
}

Widget *Widget::findWidgetInChain(Widget *w, int x, int y) {
	while (w) {
		// Stop as soon as we find a widget that contains the point (x,y)
		if (x >= w->_x && x < w->_x + w->_w && y >= w->_y && y < w->_y + w->_h)
			break;
		w = w->_next;
	}
	if (w)
		w = w->findWidget(x - w->_x, y - w->_y);
	return w;
}

Widget *Widget::findWidgetInChain(Widget *w, const char *name) {
	while (w) {
		if (w->_name == name) {
			return w;
		}
		w = w->_next;
	}
	return 0;
}

void Widget::setEnabled(bool e) {
	if ((_flags & WIDGET_ENABLED) != e) {
		if (e)
			setFlags(WIDGET_ENABLED);
		else
			clearFlags(WIDGET_ENABLED);

		_boss->draw();
	}
}

bool Widget::isEnabled() const {
	if (g_gui.xmlEval()->getVar("Dialog." + _name + ".Enabled", 1) == 0) {
		return false;
	}
	return ((_flags & WIDGET_ENABLED) != 0);
}

void Widget::setVisible(bool e) {
	if (e)
		clearFlags(WIDGET_INVISIBLE);
	else
		setFlags(WIDGET_INVISIBLE);
}

bool Widget::isVisible() const {
	if (g_gui.xmlEval()->getVar("Dialog." + _name + ".Visible", 1) == 0)
		return false;

	return !(_flags & WIDGET_INVISIBLE);
}

uint8 Widget::parseHotkey(const Common::String &label) {
	if (!label.contains('~'))
		return 0;

	int state = 0;
	uint8 hotkey = 0;

	for (uint i = 0; i < label.size() && state != 3; i++) {
		switch (state) {
		case 0:
			if (label[i] == '~')
				state = 1;
			break;
		case 1:
			if (label[i] != '~') {
				state = 2;
				hotkey = label[i];
			} else
				state = 0;
			break;
		case 2:
			if (label[i] == '~')
				state = 3;
			else
				state = 0;
			break;
		}
	}

	if (state == 3)
		return hotkey;

	return 0;
}

Common::String Widget::cleanupHotkey(const Common::String &label) {
	Common::String res;

	for (uint i = 0; i < label.size(); i++)
		if (label[i] != '~')
			res = res + label[i];

	return res;
}

#pragma mark -

StaticTextWidget::StaticTextWidget(GuiObject *boss, int x, int y, int w, int h, const Common::String &text, Graphics::TextAlign align, const char *tooltip)
	: Widget(boss, x, y, w, h, tooltip), _align(align) {
	setFlags(WIDGET_ENABLED);
	_type = kStaticTextWidget;
	_label = text;
}

StaticTextWidget::StaticTextWidget(GuiObject *boss, const Common::String &name, const Common::String &text, const char *tooltip)
	: Widget(boss, name, tooltip) {
	setFlags(WIDGET_ENABLED);
	_type = kStaticTextWidget;
	_label = text;

	_align = g_gui.xmlEval()->getWidgetTextHAlign(name);
}

void StaticTextWidget::setValue(int value) {
	_label = Common::String::format("%d", value);
}

void StaticTextWidget::setLabel(const Common::String &label) {
	if (_label != label) {
		_label = label;

		// when changing the label, add the CLEARBG flag
		// so the widget is completely redrawn, otherwise
		// the new text is drawn on top of the old one.
		setFlags(WIDGET_CLEARBG);
		draw();
		clearFlags(WIDGET_CLEARBG);
	}
}

void StaticTextWidget::setAlign(Graphics::TextAlign align) {
	_align = align;
	// TODO: We should automatically redraw when the alignment is changed.
	// See setLabel() for more insights.
}


void StaticTextWidget::drawWidget() {
	g_gui.theme()->drawText(Common::Rect(_x, _y, _x+_w, _y+_h), _label, _state, _align);
}

#pragma mark -

ButtonWidget::ButtonWidget(GuiObject *boss, int x, int y, int w, int h, const Common::String &label, const char *tooltip, uint32 cmd, uint8 hotkey)
	: StaticTextWidget(boss, x, y, w, h, cleanupHotkey(label), Graphics::kTextAlignCenter, tooltip), CommandSender(boss),
	  _cmd(cmd), _hotkey(hotkey), _lastTime(0) {

	if (hotkey == 0)
		_hotkey = parseHotkey(label);

	setFlags(WIDGET_ENABLED/* | WIDGET_BORDER*/ | WIDGET_CLEARBG);
	_type = kButtonWidget;
}

ButtonWidget::ButtonWidget(GuiObject *boss, const Common::String &name, const Common::String &label, const char *tooltip, uint32 cmd, uint8 hotkey)
	: StaticTextWidget(boss, name, cleanupHotkey(label), tooltip), CommandSender(boss),
	  _cmd(cmd), _lastTime(0) {
	if (hotkey == 0)
		_hotkey = parseHotkey(label);
	setFlags(WIDGET_ENABLED/* | WIDGET_BORDER*/ | WIDGET_CLEARBG);
	_type = kButtonWidget;
}

void ButtonWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h) {
		startAnimatePressedState();
		sendCommand(_cmd, 0);
	}
}

void ButtonWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	setPressedState();
}

void ButtonWidget::drawWidget() {
	g_gui.theme()->drawButton(Common::Rect(_x, _y, _x+_w, _y+_h), _label, _state, getFlags());
}

void ButtonWidget::setLabel(const Common::String &label) {
	StaticTextWidget::setLabel(cleanupHotkey(label));
}

ButtonWidget *addClearButton(GuiObject *boss, const Common::String &name, uint32 cmd, int x, int y, int w, int h) {
	ButtonWidget *button;

#ifndef DISABLE_FANCY_THEMES
	if (g_gui.xmlEval()->getVar("Globals.ShowSearchPic") == 1 && g_gui.theme()->supportsImages()) {
		if (!name.empty())
			button = new PicButtonWidget(boss, name, _("Clear value"), cmd);
		else
			button = new PicButtonWidget(boss, x, y, w, h, _("Clear value"), cmd);
		((PicButtonWidget *)button)->useThemeTransparency(true);
		((PicButtonWidget *)button)->setGfx(g_gui.theme()->getImageSurface(ThemeEngine::kImageEraser));
	} else
#endif
		if (!name.empty())
			button = new ButtonWidget(boss, name, "C", _("Clear value"), cmd);
		else
			button = new ButtonWidget(boss, x, y, w, h, "C", _("Clear value"), cmd);

	return button;
}

void ButtonWidget::setHighLighted(bool enable) {
	(enable) ? setFlags(WIDGET_HILITED) : clearFlags(WIDGET_HILITED);
	draw();
}

void ButtonWidget::handleTickle() {
	if (_lastTime) {
		uint32 curTime = g_system->getMillis();
		if (curTime - _lastTime > kPressedButtonTime) {
			stopAnimatePressedState();
		}
	}
}

void ButtonWidget::setPressedState() {
	wantTickle(true);
	setFlags(WIDGET_PRESSED);
	draw();
}

void ButtonWidget::stopAnimatePressedState() {
	wantTickle(false);
	_lastTime = 0;
	clearFlags(WIDGET_PRESSED);
	draw();
}

void ButtonWidget::startAnimatePressedState() {
	_lastTime = g_system->getMillis();
}

void ButtonWidget::wantTickle(bool tickled) {
	if (tickled)
		((GUI::Dialog *)_boss)->setTickleWidget(this);
	else
		((GUI::Dialog *)_boss)->unSetTickleWidget();
}

#pragma mark -

PicButtonWidget::PicButtonWidget(GuiObject *boss, int x, int y, int w, int h, const char *tooltip, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, "", tooltip, cmd, hotkey),
	  _gfx(), _alpha(256), _transparency(false) {

	setFlags(WIDGET_ENABLED/* | WIDGET_BORDER*/ | WIDGET_CLEARBG);
	_type = kButtonWidget;
}

PicButtonWidget::PicButtonWidget(GuiObject *boss, const Common::String &name, const char *tooltip, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, name, "", tooltip, cmd, hotkey),
	  _gfx(), _alpha(256), _transparency(false) {
	setFlags(WIDGET_ENABLED/* | WIDGET_BORDER*/ | WIDGET_CLEARBG);
	_type = kButtonWidget;
}

PicButtonWidget::~PicButtonWidget() {
	_gfx.free();
}

void PicButtonWidget::setGfx(const Graphics::Surface *gfx) {
	_gfx.free();

	if (!gfx || !gfx->pixels)
		return;

	if (gfx->format.bytesPerPixel == 1) {
		warning("PicButtonWidget::setGfx got paletted surface passed");
		return;
	}


	if (gfx->w > _w || gfx->h > _h) {
		warning("PicButtonWidget has size %dx%d, but a surface with %dx%d is to be set", _w, _h, gfx->w, gfx->h);
		return;
	}

	_gfx.copyFrom(*gfx);
}

void PicButtonWidget::setGfx(int w, int h, int r, int g, int b) {
	if (w == -1)
		w = _w;
	if (h == -1)
		h = _h;

	const Graphics::PixelFormat &requiredFormat = g_gui.theme()->getPixelFormat();

	_gfx.free();
	_gfx.create(w, h, requiredFormat);
	_gfx.fillRect(Common::Rect(0, 0, w, h), _gfx.format.RGBToColor(r, g, b));
}

void PicButtonWidget::drawWidget() {
	g_gui.theme()->drawButton(Common::Rect(_x, _y, _x+_w, _y+_h), "", _state, getFlags());

	if (_gfx.pixels) {
		// Check whether the set up surface needs to be converted to the GUI
		// color format.
		const Graphics::PixelFormat &requiredFormat = g_gui.theme()->getPixelFormat();
		if (_gfx.format != requiredFormat) {
			_gfx.convertToInPlace(requiredFormat);
		}

		const int x = _x + (_w - _gfx.w) / 2;
		const int y = _y + (_h - _gfx.h) / 2;

		g_gui.theme()->drawSurface(Common::Rect(x, y, x + _gfx.w,  y + _gfx.h), _gfx, _state, _alpha, _transparency);
	}
}

#pragma mark -

CheckboxWidget::CheckboxWidget(GuiObject *boss, int x, int y, int w, int h, const Common::String &label, const char *tooltip, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, tooltip, cmd, hotkey), _state(false) {
	setFlags(WIDGET_ENABLED);
	_type = kCheckboxWidget;
}

CheckboxWidget::CheckboxWidget(GuiObject *boss, const Common::String &name, const Common::String &label, const char *tooltip, uint32 cmd, uint8 hotkey)
	: ButtonWidget(boss, name, label, tooltip, cmd, hotkey), _state(false) {
	setFlags(WIDGET_ENABLED);
	_type = kCheckboxWidget;
}

void CheckboxWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h) {
		toggleState();
	}
}

void CheckboxWidget::setState(bool state) {
	if (_state != state) {
		_state = state;
		//_flags ^= WIDGET_INV_BORDER;
		draw();
	}
	sendCommand(_cmd, _state);
}

void CheckboxWidget::drawWidget() {
	g_gui.theme()->drawCheckbox(Common::Rect(_x, _y, _x+_w, _y+_h), _label, _state, Widget::_state);
}

#pragma mark -
RadiobuttonGroup::RadiobuttonGroup(GuiObject *boss, uint32 cmd) : CommandSender(boss) {
	_value = -1;
	_cmd = cmd;
}

void RadiobuttonGroup::setValue(int value) {
	Common::Array<RadiobuttonWidget *>::iterator button = _buttons.begin();
	while (button != _buttons.end()) {
		(*button)->setState((*button)->getValue() == value, false);

		button++;
	}

	_value = value;

	sendCommand(_cmd, _value);
}

void RadiobuttonGroup::setEnabled(bool ena) {
	Common::Array<RadiobuttonWidget *>::iterator button = _buttons.begin();
	while (button != _buttons.end()) {
		(*button)->setEnabled(ena);

		button++;
	}
}

#pragma mark -

RadiobuttonWidget::RadiobuttonWidget(GuiObject *boss, int x, int y, int w, int h, RadiobuttonGroup *group, int value, const Common::String &label, const char *tooltip, uint8 hotkey)
	: ButtonWidget(boss, x, y, w, h, label, tooltip, 0, hotkey), _state(false), _value(value), _group(group) {
	setFlags(WIDGET_ENABLED);
	_type = kRadiobuttonWidget;
	_group->addButton(this);
}

RadiobuttonWidget::RadiobuttonWidget(GuiObject *boss, const Common::String &name, RadiobuttonGroup *group, int value, const Common::String &label, const char *tooltip, uint8 hotkey)
	: ButtonWidget(boss, name, label, tooltip, 0, hotkey), _state(false), _value(value), _group(group) {
	setFlags(WIDGET_ENABLED);
	_type = kRadiobuttonWidget;
	_group->addButton(this);
}

void RadiobuttonWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && x >= 0 && x < _w && y >= 0 && y < _h) {
		toggleState();
	}
}

void RadiobuttonWidget::setState(bool state, bool setGroup) {
	if (setGroup) {
		_group->setValue(_value);
		return;
	}

	if (_state != state) {
		_state = state;
		//_flags ^= WIDGET_INV_BORDER;
		draw();
	}
	sendCommand(_cmd, _state);
}

void RadiobuttonWidget::drawWidget() {
	g_gui.theme()->drawRadiobutton(Common::Rect(_x, _y, _x+_w, _y+_h), _label, _state, Widget::_state);
}

#pragma mark -

SliderWidget::SliderWidget(GuiObject *boss, int x, int y, int w, int h, const char *tooltip, uint32 cmd)
	: Widget(boss, x, y, w, h, tooltip), CommandSender(boss),
	  _cmd(cmd), _value(0), _oldValue(0), _valueMin(0), _valueMax(100), _isDragging(false) {
	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG);
	_type = kSliderWidget;
}

SliderWidget::SliderWidget(GuiObject *boss, const Common::String &name, const char *tooltip, uint32 cmd)
	: Widget(boss, name, tooltip), CommandSender(boss),
	  _cmd(cmd), _value(0), _oldValue(0), _valueMin(0), _valueMax(100), _isDragging(false) {
	setFlags(WIDGET_ENABLED | WIDGET_TRACK_MOUSE | WIDGET_CLEARBG);
	_type = kSliderWidget;
}

void SliderWidget::handleMouseMoved(int x, int y, int button) {
	if (isEnabled() && _isDragging) {
		int newValue = posToValue(x);
		if (newValue < _valueMin)
			newValue = _valueMin;
		else if (newValue > _valueMax)
			newValue = _valueMax;

		if (newValue != _value) {
			_value = newValue;
			draw();
			sendCommand(_cmd, _value);	// FIXME - hack to allow for "live update" in sound dialog
		}
	}
}

void SliderWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (isEnabled()) {
		_isDragging = true;
		handleMouseMoved(x, y, button);
	}
}

void SliderWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	if (isEnabled() && _isDragging) {
		sendCommand(_cmd, _value);
	}
	_isDragging = false;
}

void SliderWidget::handleMouseWheel(int x, int y, int direction) {
	if (isEnabled() && !_isDragging) {
		// Increment or decrement one position
		int newValue = posToValue(valueToPos(_value) - 1 * direction);

		if (newValue < _valueMin)
			newValue = _valueMin;
		else if (newValue > _valueMax)
			newValue = _valueMax;

		if (newValue != _value) {
			_value = newValue;
			draw();
			sendCommand(_cmd, _value);	// FIXME - hack to allow for "live update" in sound dialog
		}
	}
}

void SliderWidget::drawWidget() {
	g_gui.theme()->drawSlider(Common::Rect(_x, _y, _x + _w, _y + _h), valueToBarWidth(_value), _state);
}

int SliderWidget::valueToBarWidth(int value) {
	return (_w * (value - _valueMin) / (_valueMax - _valueMin));
}

int SliderWidget::valueToPos(int value) {
	return ((_w - 1) * (value - _valueMin + 1) / (_valueMax - _valueMin));
}

int SliderWidget::posToValue(int pos) {
	return (pos) * (_valueMax - _valueMin) / (_w - 1) + _valueMin;
}

#pragma mark -

GraphicsWidget::GraphicsWidget(GuiObject *boss, int x, int y, int w, int h, const char *tooltip)
	: Widget(boss, x, y, w, h, tooltip), _gfx(), _alpha(256), _transparency(false) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG);
	_type = kGraphicsWidget;
}

GraphicsWidget::GraphicsWidget(GuiObject *boss, const Common::String &name, const char *tooltip)
	: Widget(boss, name, tooltip), _gfx(), _alpha(256), _transparency(false) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG);
	_type = kGraphicsWidget;
}

GraphicsWidget::~GraphicsWidget() {
	_gfx.free();
}

void GraphicsWidget::setGfx(const Graphics::Surface *gfx) {
	_gfx.free();

	if (!gfx || !gfx->pixels)
		return;

	if (gfx->format.bytesPerPixel == 1) {
		warning("GraphicsWidget::setGfx got paletted surface passed");
		return;
	}

	if (gfx->w > _w || gfx->h > _h) {
		warning("GraphicsWidget has size %dx%d, but a surface with %dx%d is to be set", _w, _h, gfx->w, gfx->h);
		return;
	}

	_gfx.copyFrom(*gfx);
}

void GraphicsWidget::setGfx(int w, int h, int r, int g, int b) {
	if (w == -1)
		w = _w;
	if (h == -1)
		h = _h;

	const Graphics::PixelFormat &requiredFormat = g_gui.theme()->getPixelFormat();

	_gfx.free();
	_gfx.create(w, h, requiredFormat);
	_gfx.fillRect(Common::Rect(0, 0, w, h), _gfx.format.RGBToColor(r, g, b));
}

void GraphicsWidget::drawWidget() {
	if (_gfx.pixels) {
		// Check whether the set up surface needs to be converted to the GUI
		// color format.
		const Graphics::PixelFormat &requiredFormat = g_gui.theme()->getPixelFormat();
		if (_gfx.format != requiredFormat) {
			_gfx.convertToInPlace(requiredFormat);
		}

		const int x = _x + (_w - _gfx.w) / 2;
		const int y = _y + (_h - _gfx.h) / 2;

		g_gui.theme()->drawSurface(Common::Rect(x, y, x + _gfx.w,  y + _gfx.h), _gfx, _state, _alpha, _transparency);
	}
}

#pragma mark -

ContainerWidget::ContainerWidget(GuiObject *boss, int x, int y, int w, int h) : Widget(boss, x, y, w, h) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG);
	_type = kContainerWidget;
}

ContainerWidget::ContainerWidget(GuiObject *boss, const Common::String &name) : Widget(boss, name) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG);
	_type = kContainerWidget;
}

ContainerWidget::~ContainerWidget() {
	// We also remove the widget from the boss to avoid segfaults, when the
	// deleted widget is an active widget in the boss.
	for (Widget *w = _firstWidget; w; w = w->next()) {
		_boss->removeWidget(w);
	}
}

Widget *ContainerWidget::findWidget(int x, int y) {
	return findWidgetInChain(_firstWidget, x, y);
}

void ContainerWidget::removeWidget(Widget *widget) {
	// We also remove the widget from the boss to avoid a reference to a
	// widget not in the widget chain anymore.
	_boss->removeWidget(widget);

	Widget::removeWidget(widget);
}

void ContainerWidget::drawWidget() {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h), 0, ThemeEngine::kWidgetBackgroundBorder);
}

} // End of namespace GUI
