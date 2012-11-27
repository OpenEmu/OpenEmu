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

#ifndef BROWSER_DIALOG_H
#define BROWSER_DIALOG_H

#include "gui/dialog.h"
#include "common/fs.h"

namespace GUI {

class ListWidget;
class StaticTextWidget;
class CommandSender;

class BrowserDialog : public Dialog {
public:
	BrowserDialog(const char *title, bool dirBrowser);

#ifdef MACOSX
	~BrowserDialog();
	virtual int runModal();
#else
	virtual void open();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
#endif

	const Common::FSNode	&getResult() { return _choice; }

protected:
#ifdef MACOSX
	const void *_titleRef;
	const void *_chooseRef;
#else
	ListWidget		*_fileList;
	StaticTextWidget	*_currentPath;
	Common::FSNode	_node;
	Common::FSList			_nodeContent;
#endif
	Common::FSNode	_choice;
	bool			_isDirBrowser;

#ifndef MACOSX
	void updateListing();
#endif
};

} // End of namespace GUI

#endif
