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

#include "common/util.h"
#include "common/system.h"

#include "gui/ThemeLayout.h"

#include "graphics/font.h"

#ifdef LAYOUT_DEBUG_DIALOG
#include "graphics/surface.h"
#endif

namespace GUI {

void ThemeLayout::importLayout(ThemeLayout *layout) {
	assert(layout->getLayoutType() == kLayoutMain);

	if (layout->_children.size() == 0)
		return;

	layout = layout->_children[0];

	if (getLayoutType() == layout->getLayoutType()) {
		for (uint i = 0; i < layout->_children.size(); ++i)
			_children.push_back(layout->_children[i]->makeClone(this));
	} else {
		_children.push_back(layout->makeClone(this));
	}
}

bool ThemeLayout::getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) {
	if (name.empty()) {
		assert(getLayoutType() == kLayoutMain);
		x = _x; y = _y;
		w = _w; h = _h;
		return true;
	}

	for (uint i = 0; i < _children.size(); ++i) {
		if (_children[i]->getWidgetData(name, x, y, w, h))
			return true;
	}

	return false;
}

Graphics::TextAlign ThemeLayout::getWidgetTextHAlign(const Common::String &name) {
	if (name.empty()) {
		assert(getLayoutType() == kLayoutMain);
		return _textHAlign;
	}

	Graphics::TextAlign res;

	for (uint i = 0; i < _children.size(); ++i) {
		if ((res = _children[i]->getWidgetTextHAlign(name)) != Graphics::kTextAlignInvalid)
			return res;
	}

	return Graphics::kTextAlignInvalid;
}

int16 ThemeLayoutStacked::getParentWidth() {
	ThemeLayout *p = _parent;
	int width = 0;

	while (p && p->getLayoutType() != kLayoutMain) {
		width += p->_padding.right + p->_padding.left;
		if (p->getLayoutType() == kLayoutHorizontal) {
			const int spacing = ((ThemeLayoutStacked *)p)->_spacing;
			for (uint i = 0; i < p->_children.size(); ++i)
				width += p->_children[i]->getWidth() + spacing;
		}
		// FIXME: Do we really want to assume that any layout type different
		// from kLayoutHorizontal corresponds to width 0 ?
		p = p->_parent;
	}

	assert(p && p->getLayoutType() == kLayoutMain);
	return p->getWidth() - width;
}

int16 ThemeLayoutStacked::getParentHeight() {
	ThemeLayout *p = _parent;
	int height = 0;

	while (p && p->getLayoutType() != kLayoutMain) {
		height += p->_padding.bottom + p->_padding.top;
		if (p->getLayoutType() == kLayoutVertical) {
			const int spacing = ((ThemeLayoutStacked *)p)->_spacing;
			for (uint i = 0; i < p->_children.size(); ++i)
				height += p->_children[i]->getHeight() + spacing;
		}
		// FIXME: Do we really want to assume that any layout type different
		// from kLayoutVertical corresponds to height 0 ?
		p = p->_parent;
	}

	assert(p && p->getLayoutType() == kLayoutMain);
	return p->getHeight() - height;
}

#ifdef LAYOUT_DEBUG_DIALOG
void ThemeLayout::debugDraw(Graphics::Surface *screen, const Graphics::Font *font) {
	uint16 color = 0xFFFF;
	font->drawString(screen, getName(), _x, _y, _w, color, Graphics::kTextAlignRight, 0, true);
	screen->hLine(_x, _y, _x + _w, color);
	screen->hLine(_x, _y + _h, _x + _w , color);
	screen->vLine(_x, _y, _y + _h, color);
	screen->vLine(_x + _w, _y, _y + _h, color);

	for (uint i = 0; i < _children.size(); ++i)
		_children[i]->debugDraw(screen, font);
}
#endif


bool ThemeLayoutWidget::getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) {
	if (name == _name) {
		x = _x; y = _y;
		w = _w; h = _h;
		return true;
	}

	return false;
}

Graphics::TextAlign ThemeLayoutWidget::getWidgetTextHAlign(const Common::String &name) {
	if (name == _name) {
		return _textHAlign;
	}

	return Graphics::kTextAlignInvalid;
}

void ThemeLayoutMain::reflowLayout() {
	assert(_children.size() <= 1);

	if (_children.size()) {
		_children[0]->resetLayout();
		_children[0]->setWidth(_w);
		_children[0]->setHeight(_h);
		_children[0]->reflowLayout();

		if (_w == -1)
			_w = _children[0]->getWidth();

		if (_h == -1)
			_h = _children[0]->getHeight();

		if (_y == -1)
			_y = (g_system->getOverlayHeight() >> 1) - (_h >> 1);

		if (_x == -1)
			_x = (g_system->getOverlayWidth() >> 1) - (_w >> 1);
	}
}

void ThemeLayoutStacked::reflowLayoutVertical() {
	int curX, curY;
	int resize[8];
	int rescount = 0;

	curX = _padding.left;
	curY = _padding.top;
	_h = _padding.top + _padding.bottom;

	for (uint i = 0; i < _children.size(); ++i) {

		_children[i]->resetLayout();
		_children[i]->reflowLayout();

		if (_children[i]->getWidth() == -1)
			_children[i]->setWidth((_w == -1 ? getParentWidth() : _w) - _padding.left - _padding.right);

		if (_children[i]->getHeight() == -1) {
			assert(rescount < ARRAYSIZE(resize));
			resize[rescount++] = i;
			_children[i]->setHeight(0);
		}

		_children[i]->offsetY(curY);

		// Center child if it this has been requested *and* the space permits it.
		if (_centered && _children[i]->getWidth() < _w && _w != -1) {
			_children[i]->offsetX((_w >> 1) - (_children[i]->getWidth() >> 1));
		} else
			_children[i]->offsetX(curX);

		// Advance the vertical offset by the height of the newest item, plus
		// the item spacing value.
		curY += _children[i]->getHeight() + _spacing;

		// Update width and height of this stack layout
		_w = MAX(_w, (int16)(_children[i]->getWidth() + _padding.left + _padding.right));
		_h += _children[i]->getHeight() + _spacing;
	}

	// If there are any children at all, then we added the spacing value once
	// too often. Correct that.
	if (!_children.empty())
		_h -= _spacing;

	// If there were any items with undetermined height, then compute and set
	// their height now. We do so by determining how much space is left, and
	// then distributing this equally over all items which need auto-resizing.
	if (rescount) {
		int newh = (getParentHeight() - _h - _padding.bottom) / rescount;

		for (int i = 0; i < rescount; ++i) {
			// Set the height of the item.
			_children[resize[i]]->setHeight(newh);
			// Increase the height of this ThemeLayoutStacked accordingly, and
			// then shift all subsequence children.
			_h += newh;
			for (uint j = resize[i] + 1; j < _children.size(); ++j)
				_children[j]->offsetY(newh);
		}
	}
}

void ThemeLayoutStacked::reflowLayoutHorizontal() {
	int curX, curY;
	int resize[8];
	int rescount = 0;

	curX = _padding.left;
	curY = _padding.top;
	_w = _padding.left + _padding.right;

	for (uint i = 0; i < _children.size(); ++i) {

		_children[i]->resetLayout();
		_children[i]->reflowLayout();

		if (_children[i]->getHeight() == -1)
			_children[i]->setHeight((_h == -1 ? getParentHeight() : _h) - _padding.top - _padding.bottom);

		if (_children[i]->getWidth() == -1) {
			assert(rescount < ARRAYSIZE(resize));
			resize[rescount++] = i;
			_children[i]->setWidth(0);
		}

		_children[i]->offsetX(curX);

		// Center child if it this has been requested *and* the space permits it.
		if (_centered && _children[i]->getHeight() < _h && _h != -1)
			_children[i]->offsetY((_h >> 1) - (_children[i]->getHeight() >> 1));
		else
			_children[i]->offsetY(curY);

		// Advance the horizontal offset by the width of the newest item, plus
		// the item spacing value.
		curX += (_children[i]->getWidth() + _spacing);

		// Update width and height of this stack layout
		_w += _children[i]->getWidth() + _spacing;
		_h = MAX(_h, (int16)(_children[i]->getHeight() + _padding.top + _padding.bottom));
	}

	// If there are any children at all, then we added the spacing value once
	// too often. Correct that.
	if (!_children.empty())
		_w -= _spacing;

	// If there were any items with undetermined width, then compute and set
	// their width now. We do so by determining how much space is left, and
	// then distributing this equally over all items which need auto-resizing.
	if (rescount) {
		int neww = (getParentWidth() - _w - _padding.right) / rescount;

		for (int i = 0; i < rescount; ++i) {
			// Set the width of the item.
			_children[resize[i]]->setWidth(neww);
			// Increase the width of this ThemeLayoutStacked accordingly, and
			// then shift all subsequence children.
			_w += neww;
			for (uint j = resize[i] + 1; j < _children.size(); ++j)
				_children[j]->offsetX(neww);
		}
	}
}

} // End of namespace GUI
