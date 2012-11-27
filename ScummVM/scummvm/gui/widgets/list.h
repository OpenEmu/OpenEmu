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

#ifndef GUI_WIDGETS_LIST_H
#define GUI_WIDGETS_LIST_H

#include "gui/widgets/editable.h"
#include "common/str.h"

#include "gui/ThemeEngine.h"

namespace GUI {

class ScrollBarWidget;

enum NumberingMode {
	kListNumberingOff	= -1,
	kListNumberingZero	= 0,
	kListNumberingOne	= 1
};

/// Some special commands
enum {
	kListItemDoubleClickedCmd	= 'LIdb',	///< double click on item - 'data' will be item index
	kListItemActivatedCmd		= 'LIac',	///< item activated by return/enter - 'data' will be item index
	kListItemRemovalRequestCmd	= 'LIrm',	///< request to remove the item with the delete/backspace keys - 'data' will be item index
	kListSelectionChangedCmd	= 'Lsch'	///< selection changed - 'data' will be item index
};

/* ListWidget */
class ListWidget : public EditableWidget {
public:
	typedef Common::String String;
	typedef Common::Array<Common::String> StringArray;
	typedef Common::Array<ThemeEngine::FontColor> ColorList;
protected:
	StringArray		_list;
	StringArray		_dataList;
	ColorList		_listColors;
	Common::Array<int>		_listIndex;
	bool			_editable;
	bool			_editMode;
	NumberingMode	_numberingMode;
	int				_currentPos;
	int				_entriesPerPage;
	int				_selectedItem;
	ScrollBarWidget	*_scrollBar;
	int				_currentKeyDown;

	String			_quickSelectStr;
	uint32			_quickSelectTime;

	int				_hlLeftPadding;
	int				_hlRightPadding;
	int				_leftPadding;
	int				_rightPadding;
	int				_topPadding;
	int				_bottomPadding;
	int				_scrollBarWidth;

	String			_filter;
	bool			_quickSelect;

	uint32			_cmd;

	ThemeEngine::FontColor _editColor;

public:
	ListWidget(Dialog *boss, const String &name, const char *tooltip = 0, uint32 cmd = 0);
	ListWidget(Dialog *boss, int x, int y, int w, int h, const char *tooltip = 0, uint32 cmd = 0);
	virtual ~ListWidget();

	virtual Widget *findWidget(int x, int y);

	void setList(const StringArray &list, const ColorList *colors = 0);
	const StringArray &getList()	const			{ return _dataList; }

	void append(const String &s, ThemeEngine::FontColor color = ThemeEngine::kFontColorNormal);

	void setSelected(int item);
	int getSelected() const						{ return (_filter.empty() || _selectedItem == -1) ? _selectedItem : _listIndex[_selectedItem]; }

	const String &getSelectedString() const		{ return _list[_selectedItem]; }
	ThemeEngine::FontColor getSelectionColor() const;

	void setNumberingMode(NumberingMode numberingMode)	{ _numberingMode = numberingMode; }

	void scrollTo(int item);
	void scrollToEnd();
	int getCurrentScrollPos() const { return _currentPos; }

	void enableQuickSelect(bool enable) 		{ _quickSelect = enable; }
	String getQuickSelectString() const 		{ return _quickSelectStr; }

	bool isEditable() const						{ return _editable; }
	void setEditable(bool editable)				{ _editable = editable; }
	void setEditColor(ThemeEngine::FontColor color) { _editColor = color; }

	// Made startEditMode/endEditMode for SaveLoadChooser
	void startEditMode();
	void endEditMode();

	void setFilter(const String &filter, bool redraw = true);

	virtual void handleTickle();
	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleMouseUp(int x, int y, int button, int clickCount);
	virtual void handleMouseWheel(int x, int y, int direction);
	virtual bool handleKeyDown(Common::KeyState state);
	virtual bool handleKeyUp(Common::KeyState state);
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	virtual void reflowLayout();

	virtual bool wantsFocus() { return true; }

protected:
	void drawWidget();

	/// Finds the item at position (x,y). Returns -1 if there is no item there.
	int findItem(int x, int y) const;
	void scrollBarRecalc();

	void abortEditMode();

	Common::Rect getEditRect() const;

	void receivedFocusWidget();
	void lostFocusWidget();
	void scrollToCurrent();

	int *_textWidth;
};

} // End of namespace GUI

#endif
