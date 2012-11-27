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
 *
 */

#ifndef THEME_LAYOUT_H
#define THEME_LAYOUT_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/font.h"

#ifdef LAYOUT_DEBUG_DIALOG
namespace Graphics {
class Surface;
}
#endif

namespace GUI {

class ThemeLayout {
	friend class ThemeLayoutMain;
	friend class ThemeLayoutStacked;
	friend class ThemeLayoutSpacing;
	friend class ThemeLayoutWidget;
public:
	enum LayoutType {
		kLayoutMain,
		kLayoutVertical,
		kLayoutHorizontal,
		kLayoutWidget
	};

	ThemeLayout(ThemeLayout *p) :
		_parent(p), _x(0), _y(0), _w(-1), _h(-1),
		_centered(false), _defaultW(-1), _defaultH(-1),
		_textHAlign(Graphics::kTextAlignInvalid) {}

	virtual ~ThemeLayout() {
		for (uint i = 0; i < _children.size(); ++i)
			delete _children[i];
	}

	virtual void reflowLayout() = 0;

	virtual void resetLayout() { _x = 0; _y = 0; _w = _defaultW; _h = _defaultH; }

	void addChild(ThemeLayout *child) { _children.push_back(child); }

	void setPadding(int8 left, int8 right, int8 top, int8 bottom) {
		_padding.left = left;
		_padding.right = right;
		_padding.top = top;
		_padding.bottom = bottom;
	}

protected:
	int16 getWidth() { return _w; }
	int16 getHeight() { return _h; }

	void offsetX(int newX) {
		_x += newX;
		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->offsetX(newX);
	}

	void offsetY(int newY) {
		_y += newY;
		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->offsetY(newY);
	}

	void setWidth(int16 width) { _w = width; }
	void setHeight(int16 height) { _h = height; }
	void setTextHAlign(Graphics::TextAlign align) { _textHAlign = align; }

	virtual LayoutType getLayoutType() = 0;

	virtual ThemeLayout *makeClone(ThemeLayout *newParent) = 0;

public:
	virtual bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h);

	virtual Graphics::TextAlign getWidgetTextHAlign(const Common::String &name);

	void importLayout(ThemeLayout *layout);

	Graphics::TextAlign getTextHAlign() { return _textHAlign; }

#ifdef LAYOUT_DEBUG_DIALOG
	void debugDraw(Graphics::Surface *screen, const Graphics::Font *font);

	virtual const char *getName() const = 0;
#endif

protected:
	ThemeLayout *_parent;
	int16 _x, _y, _w, _h;
	Common::Rect _padding;
	Common::Array<ThemeLayout *> _children;
	bool _centered;
	int16 _defaultW, _defaultH;
	Graphics::TextAlign _textHAlign;
};

class ThemeLayoutMain : public ThemeLayout {
public:
	ThemeLayoutMain(int16 x, int16 y, int16 w, int16 h) : ThemeLayout(0) {
		_w = _defaultW = w;
		_h = _defaultH = h;
		_x = _defaultX = x;
		_y = _defaultY = y;
	}
	void reflowLayout();

	void resetLayout() {
		ThemeLayout::resetLayout();
		_x = _defaultX;
		_y = _defaultY;
	}

#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const { return "Global Layout"; }
#endif

protected:
	LayoutType getLayoutType() { return kLayoutMain; }
	ThemeLayout *makeClone(ThemeLayout *newParent) { assert(!"Do not copy Main Layouts!"); return 0; }

	int16 _defaultX;
	int16 _defaultY;
};

class ThemeLayoutStacked : public ThemeLayout {
public:
	ThemeLayoutStacked(ThemeLayout *p, LayoutType type, int spacing, bool center) :
		ThemeLayout(p), _type(type) {
		assert((type == kLayoutVertical) || (type == kLayoutHorizontal));
		_spacing = spacing;
		_centered = center;
	}

	void reflowLayout() {
		if (_type == kLayoutVertical)
			reflowLayoutVertical();
		else
			reflowLayoutHorizontal();
	}
	void reflowLayoutHorizontal();
	void reflowLayoutVertical();

#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const {
		return (_type == kLayoutVertical)
			? "Vertical Layout" : "Horizontal Layout";
	}
#endif

protected:
	int16 getParentWidth();
	int16 getParentHeight();

	LayoutType getLayoutType() { return _type; }

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayoutStacked *n = new ThemeLayoutStacked(*this);
		n->_parent = newParent;

		for (uint i = 0; i < n->_children.size(); ++i)
			n->_children[i] = n->_children[i]->makeClone(n);

		return n;
	}

	const LayoutType _type;
	int8 _spacing;
};

class ThemeLayoutWidget : public ThemeLayout {
public:
	ThemeLayoutWidget(ThemeLayout *p, const Common::String &name, int16 w, int16 h, Graphics::TextAlign align) : ThemeLayout(p), _name(name) {
		_w = _defaultW = w;
		_h = _defaultH = h;

		setTextHAlign(align);
	}

	bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h);
	Graphics::TextAlign getWidgetTextHAlign(const Common::String &name);

	void reflowLayout() {}

#ifdef LAYOUT_DEBUG_DIALOG
	virtual const char *getName() const { return _name.c_str(); }
#endif

protected:
	LayoutType getLayoutType() { return kLayoutWidget; }

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayout *n = new ThemeLayoutWidget(*this);
		n->_parent = newParent;
		return n;
	}

	Common::String _name;
};

class ThemeLayoutSpacing : public ThemeLayout {
public:
	ThemeLayoutSpacing(ThemeLayout *p, int size) : ThemeLayout(p) {
		if (p->getLayoutType() == kLayoutHorizontal) {
			_w = _defaultW = size;
			_h = _defaultH = 1;
		} else if (p->getLayoutType() == kLayoutVertical) {
			_w = _defaultW = 1;
			_h = _defaultH = size;
		}
	}

	bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) { return false; }
	void reflowLayout() {}
#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const { return "SPACE"; }
#endif

protected:
	LayoutType getLayoutType() { return kLayoutWidget; }

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayout *n = new ThemeLayoutSpacing(*this);
		n->_parent = newParent;
		return n;
	}
};

}

#endif
