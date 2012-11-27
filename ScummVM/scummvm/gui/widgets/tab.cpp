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

#include "common/util.h"
#include "gui/widgets/tab.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

enum {
	kCmdLeft  = 'LEFT',
	kCmdRight = 'RGHT'
};

TabWidget::TabWidget(GuiObject *boss, int x, int y, int w, int h)
	: Widget(boss, x, y, w, h), _bodyBackgroundType(GUI::ThemeEngine::kDialogBackgroundDefault) {
	init();
}

TabWidget::TabWidget(GuiObject *boss, const String &name)
	: Widget(boss, name), _bodyBackgroundType(GUI::ThemeEngine::kDialogBackgroundDefault) {
	init();
}

void TabWidget::init() {
	setFlags(WIDGET_ENABLED);
	_type = kTabWidget;
	_activeTab = -1;
	_firstVisibleTab = 0;

	_tabWidth = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Width");
	_tabHeight = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Height");
	_titleVPad = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Padding.Top");

	_bodyTP = g_gui.xmlEval()->getVar("Globals.TabWidget.Body.Padding.Top");
	_bodyBP = g_gui.xmlEval()->getVar("Globals.TabWidget.Body.Padding.Bottom");
	_bodyLP = g_gui.xmlEval()->getVar("Globals.TabWidget.Body.Padding.Left");
	_bodyRP = g_gui.xmlEval()->getVar("Globals.TabWidget.Body.Padding.Right");

	_butRP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButtonPadding.Right", 0);
	_butTP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Padding.Top", 0);
	_butW = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Width", 10);
	_butH = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Height", 10);

	int x = _w - _butRP - _butW * 2 - 2;
	int y = _butTP - _tabHeight;
	_navLeft = new ButtonWidget(this, x, y, _butW, _butH, "<", 0, kCmdLeft);
	_navRight = new ButtonWidget(this, x + _butW + 2, y, _butW, _butH, ">", 0, kCmdRight);
}

TabWidget::~TabWidget() {
	_firstWidget = 0;
	for (uint i = 0; i < _tabs.size(); ++i) {
		delete _tabs[i].firstWidget;
		_tabs[i].firstWidget = 0;
	}
	_tabs.clear();
	delete _navRight;
}

int16 TabWidget::getChildY() const {
	return getAbsY() + _tabHeight;
}

int TabWidget::addTab(const String &title) {
	// Add a new tab page
	Tab newTab;
	newTab.title = title;
	newTab.firstWidget = 0;

	_tabs.push_back(newTab);

	int numTabs = _tabs.size();

	// HACK: Nintendo DS uses a custom config dialog. This dialog does not work with
	// our default "Globals.TabWidget.Tab.Width" setting.
	//
	// TODO: Add proper handling in the theme layout for such cases.
	//
	// There are different solutions to this problem:
	//  - offer a "Tab.Width" setting per tab widget and thus let the Ninteno DS
	//    backend set a default value for its special dialog.
	//
	//  - change our themes to use auto width calculaction by default
	//
	//  - change "Globals.TabWidget.Tab.Width" to be the minimal tab width setting and
	//    rename it accordingly.
	//    Actually this solution is pretty similar to our HACK for the Nintendo DS
	//    backend. This hack enables auto width calculation by default with the
	//    "Globals.TabWidget.Tab.Width" value as minimal width for the tab buttons.
	//
	//  - we might also consider letting every tab button having its own width.
	//
	//  - other solutions you can think of, which are hopefully less evil ;-).
	//
	// Of course also the Ninteno DS' dialog should be in our layouting engine, instead
	// of being hard coded like it is right now.
	//
	// There are checks for __DS__ all over this source file to take care of the
	// aforemnetioned problem.
#ifdef __DS__
	if (true) {
#else
	if (g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Width") == 0) {
#endif
		if (_tabWidth == 0)
			_tabWidth = 40;
		// Determine the new tab width
		int newWidth = g_gui.getStringWidth(title) + 2 * 3;
		if (_tabWidth < newWidth)
			_tabWidth = newWidth;
		int maxWidth = _w / numTabs;
		if (_tabWidth > maxWidth)
			_tabWidth = maxWidth;
	}

	// Activate the new tab
	setActiveTab(numTabs - 1);

	return _activeTab;
}

void TabWidget::removeTab(int tabID) {
	assert(0 <= tabID && tabID < (int)_tabs.size());

	// Deactive the tab if it's currently the active one
	if (tabID == _activeTab) {
		_tabs[tabID].firstWidget = _firstWidget;
		releaseFocus();
		_firstWidget = 0;
	}

	// Dispose the widgets in that tab and then the tab itself
	delete _tabs[tabID].firstWidget;
	_tabs.remove_at(tabID);

	// Adjust _firstVisibleTab if necessary
	if (_firstVisibleTab >= (int)_tabs.size()) {
		_firstVisibleTab = MAX(0, (int)_tabs.size() - 1);
	}

	// The active tab was removed, so select a new active one (if any remains)
	if (tabID == _activeTab) {
		_activeTab = -1;
		if (tabID >= (int)_tabs.size())
			tabID = _tabs.size() - 1;
		if (tabID >= 0)
			setActiveTab(tabID);
	}

	// Finally trigger a redraw
	_boss->draw();
}

void TabWidget::setActiveTab(int tabID) {
	assert(0 <= tabID && tabID < (int)_tabs.size());
	if (_activeTab != tabID) {
		// Exchange the widget lists, and switch to the new tab
		if (_activeTab != -1) {
			_tabs[_activeTab].firstWidget = _firstWidget;
			releaseFocus();
		}
		_activeTab = tabID;
		_firstWidget = _tabs[tabID].firstWidget;
		_boss->draw();
	}
}


void TabWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	Widget::handleCommand(sender, cmd, data);

	switch (cmd) {
	case kCmdLeft:
		if (_firstVisibleTab) {
			_firstVisibleTab--;
			draw();
		}
		break;

	case kCmdRight:
		if (_firstVisibleTab + _w / _tabWidth < (int)_tabs.size()) {
			_firstVisibleTab++;
			draw();
		}
		break;
	}
}

void TabWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	assert(y < _tabHeight);

	// Determine which tab was clicked
	int tabID = -1;
	if (x >= 0 && (x % _tabWidth) < _tabWidth) {
		tabID = x / _tabWidth;
		if (tabID >= (int)_tabs.size())
			tabID = -1;
	}

	// If a tab was clicked, switch to that pane
	if (tabID >= 0 && tabID + _firstVisibleTab < (int)_tabs.size()) {
		setActiveTab(tabID + _firstVisibleTab);
	}
}

bool TabWidget::handleKeyDown(Common::KeyState state) {
	// TODO: maybe there should be a way to switch between tabs
	// using the keyboard? E.g. Alt-Shift-Left/Right-Arrow or something
	// like that.
	return Widget::handleKeyDown(state);
}

void TabWidget::reflowLayout() {
	Widget::reflowLayout();

	for (uint i = 0; i < _tabs.size(); ++i) {
		Widget *w = _tabs[i].firstWidget;
		while (w) {
			w->reflowLayout();
			w = w->next();
		}
	}

	_tabHeight = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Height");
	_tabWidth = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Width");
	_titleVPad = g_gui.xmlEval()->getVar("Globals.TabWidget.Tab.Padding.Top");

	if (_tabWidth == 0) {
		_tabWidth = 40;
#ifdef __DS__
	}
	if (true) {
#endif
		int maxWidth = _w / _tabs.size();

		for (uint i = 0; i < _tabs.size(); ++i) {
			// Determine the new tab width
			int newWidth = g_gui.getStringWidth(_tabs[i].title) + 2 * 3;
			if (_tabWidth < newWidth)
				_tabWidth = newWidth;
			if (_tabWidth > maxWidth)
				_tabWidth = maxWidth;
		}
	}

	_butRP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.PaddingRight", 0);
	_butTP = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Padding.Top", 0);
	_butW = g_gui.xmlEval()->getVar("GlobalsTabWidget.NavButton.Width", 10);
	_butH = g_gui.xmlEval()->getVar("Globals.TabWidget.NavButton.Height", 10);

	int x = _w - _butRP - _butW * 2 - 2;
	int y = _butTP - _tabHeight;
	_navLeft->resize(x, y, _butW, _butH);
	_navRight->resize(x + _butW + 2, y, _butW, _butH);
}

void TabWidget::drawWidget() {
	Common::Array<Common::String> tabs;
	for (int i = _firstVisibleTab; i < (int)_tabs.size(); ++i) {
		tabs.push_back(_tabs[i].title);
	}
	g_gui.theme()->drawDialogBackground(Common::Rect(_x + _bodyLP, _y + _bodyTP, _x+_w-_bodyRP, _y+_h-_bodyBP), _bodyBackgroundType);

	g_gui.theme()->drawTab(Common::Rect(_x, _y, _x+_w, _y+_h), _tabHeight, _tabWidth, tabs, _activeTab - _firstVisibleTab, 0, _titleVPad);
}

void TabWidget::draw() {
	Widget::draw();

	if (_tabWidth * _tabs.size() > _w) {
		_navLeft->draw();
		_navRight->draw();
	}
}

Widget *TabWidget::findWidget(int x, int y) {
	if (y < _tabHeight) {
		if (_tabWidth * _tabs.size() > _w) {
			if (y >= _butTP && y < _butTP + _butH) {
				if (x >= _w - _butRP - _butW * 2 - 2 && x < _w - _butRP - _butW - 2)
					return _navLeft;
				if (x >= _w - _butRP - _butW &&  x < _w - _butRP)
					return _navRight;
			}
		}

		// Click was in the tab area
		return this;
	} else {
		// Iterate over all child widgets and find the one which was clicked
		return Widget::findWidgetInChain(_firstWidget, x, y - _tabHeight);
	}
}

} // End of namespace GUI
