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

#ifndef CHOOSER_DIALOG_H
#define CHOOSER_DIALOG_H

#include "common/array.h"
#include "common/str.h"
#include "gui/dialog.h"

namespace GUI {

class ButtonWidget;
class CommandSender;
class ListWidget;

/*
 * A dialog that allows the user to choose between a selection of items
 */

class ChooserDialog : public Dialog {
	typedef Common::String String;
	typedef Common::Array<Common::String> StringArray;
protected:
	ListWidget		*_list;
	ButtonWidget	*_chooseButton;

public:
	ChooserDialog(const String &title, String dialogId = "Browser");

	void setList(const StringArray& list);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
};

} // End of namespace GUI

#endif
