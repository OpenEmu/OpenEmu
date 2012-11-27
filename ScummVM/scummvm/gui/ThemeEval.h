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

#ifndef GUI_THEME_EVAL_H
#define GUI_THEME_EVAL_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "graphics/font.h"

#include "gui/ThemeLayout.h"

namespace GUI {

class ThemeEval {

	typedef Common::HashMap<Common::String, int> VariablesMap;
	typedef Common::HashMap<Common::String, ThemeLayout *> LayoutsMap;

public:
	ThemeEval() {
		buildBuiltinVars();
	}

	~ThemeEval();

	void buildBuiltinVars();

	int getVar(const Common::String &s) {
		if (_vars.contains(s))
			return _vars[s];

		if (_builtin.contains(s))
			return _builtin[s];

		error("CRITICAL: Missing variable: '%s'", s.c_str());
		return -13375; //EVAL_UNDEF_VAR
	}

	int getVar(const Common::String &s, int def) {
		if (_vars.contains(s))
			return _vars[s];

		if (_builtin.contains(s))
			return _builtin[s];

		return def;
	}

	void setVar(const Common::String &name, int val) { _vars[name] = val; }

	bool hasVar(const Common::String &name) { return _vars.contains(name) || _builtin.contains(name); }

	void addDialog(const Common::String &name, const Common::String &overlays, bool enabled = true, int inset = 0);
	void addLayout(ThemeLayout::LayoutType type, int spacing, bool center = false);
	void addWidget(const Common::String &name, int w, int h, const Common::String &type, bool enabled = true, Graphics::TextAlign align = Graphics::kTextAlignLeft);
	bool addImportedLayout(const Common::String &name);
	void addSpace(int size);

	void addPadding(int16 l, int16 r, int16 t, int16 b) { _curLayout.top()->setPadding(l, r, t, b); }

	void closeLayout() { _curLayout.pop(); }
	void closeDialog() { _curLayout.pop()->reflowLayout(); _curDialog.clear(); }

	bool getWidgetData(const Common::String &widget, int16 &x, int16 &y, uint16 &w, uint16 &h);

	Graphics::TextAlign getWidgetTextHAlign(const Common::String &widget);

#ifdef LAYOUT_DEBUG_DIALOG
	void debugDraw(Graphics::Surface *screen, const Graphics::Font *font) {
		_layouts[LAYOUT_DEBUG_DIALOG]->debugDraw(screen, font);
	}
#endif

	void reset();

private:
	VariablesMap _vars;
	VariablesMap _builtin;

	LayoutsMap _layouts;
	Common::Stack<ThemeLayout *> _curLayout;
	Common::String _curDialog;
};

} // End of namespace GUI

#endif
