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

#ifndef REMAP_DIALOG_H
#define REMAP_DIALOG_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/keymapper.h"
#include "gui/dialog.h"

namespace GUI {
class ButtonWidget;
class PopUpWidget;
class ScrollBarWidget;
class StaticTextWidget;
}

namespace Common {

class RemapDialog : public GUI::Dialog {
public:
	RemapDialog();
	virtual ~RemapDialog();
	virtual void open();
	virtual void close();
	virtual void reflowLayout();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyDown(Common::KeyState state);
	virtual void handleKeyUp(Common::KeyState state);
	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleTickle();
	virtual void handleOtherEvent(Common::Event ev);

protected:
	struct ActionWidgets {
		GUI::StaticTextWidget *actionText;
		GUI::ButtonWidget *keyButton;
		GUI::ButtonWidget *clearButton;
	};
	struct ActionInfo {
		Action *action;
		bool inherited;
		String description;
	};

	void loadKeymap();
	void refreshKeymap();
	void clearMapping(uint i);
	void startRemapping(uint i);
	void stopRemapping(bool force = false);

	Keymapper *_keymapper;
	Keymap** _keymapTable;

	Array<ActionInfo> _currentActions;
	int _topAction;

	Rect _keymapArea;

	GUI::StaticTextWidget *_kmPopUpDesc;
	GUI::PopUpWidget *_kmPopUp;
	//GUI::ContainerWidget *_container;
	GUI::ScrollBarWidget *_scrollBar;

	uint _rowCount;

	Array<ActionWidgets> _keymapWidgets;
	uint32 _remapTimeout;
	static const uint32 kRemapTimeoutDelay = 3000;

	bool _changes;

	bool _topKeymapIsGui;

};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef REMAP_DIALOG_H
