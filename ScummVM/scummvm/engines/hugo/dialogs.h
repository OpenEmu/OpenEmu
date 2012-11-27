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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HUGO_DIALOGS_H
#define HUGO_DIALOGS_H

#include "gui/dialog.h"
#include "gui/widgets/edittext.h"

namespace Hugo {

enum MenuOption {
	kMenuWhat = 0,
	kMenuMusic,
	kMenuSoundFX,
	kMenuSave,
	kMenuLoad,
	kMenuRecall,
	kMenuTurbo,
	kMenuLook,
	kMenuInventory
};

enum {
	kMenuWidth = 320,
	kMenuHeight = 24,
	kMenuX = 5,
	kMenuY = 1,
	kButtonWidth = 20,
	kButtonHeight = 20,
	kButtonPad = 1,
	kButtonSpace = 5
};

enum {
	// TopMenu commands
	kCmdWhat = 'WHAT',
	kCmdMusic = 'MUZK',
	kCmdSoundFX = 'SOUN',
	kCmdSave = 'SAVE',
	kCmdLoad = 'LOAD',
	kCmdRecall = 'RECL',
	kCmdTurbo = 'TURB',
	kCmdLook = 'LOOK',
	kCmdInvent = 'INVT',

	// EntryDialog commands
	kCmdButton = 'BTNP',
	kCmdFinishEdit = 'FNSH'
};

class TopMenu : public GUI::Dialog {
public:
	TopMenu(HugoEngine *vm);
	~TopMenu();

	void reflowLayout();
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data);
	void handleMouseUp(int x, int y, int button, int clickCount);

	void loadBmpArr(Common::SeekableReadStream &in);

protected:
	void init();

	HugoEngine *_vm;

	GUI::PicButtonWidget *_whatButton;
	GUI::PicButtonWidget *_musicButton;
	GUI::PicButtonWidget *_soundFXButton;
	GUI::PicButtonWidget *_loadButton;
	GUI::PicButtonWidget *_saveButton;
	GUI::PicButtonWidget *_recallButton;
	GUI::PicButtonWidget *_turboButton;
	GUI::PicButtonWidget *_lookButton;
	GUI::PicButtonWidget *_inventButton;

	Graphics::Surface **_arrayBmp;
	uint16 _arraySize;
};

class EntryDialog : public GUI::Dialog {
public:
	EntryDialog(const Common::String &title, const Common::String &buttonLabel, const Common::String &defaultValue);
	virtual ~EntryDialog();

	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data);

	const Common::String &getEditString() const	{ return _text->getEditString(); }

protected:
	GUI::EditTextWidget *_text;
};

}

#endif // HUGO_DIALOGS_H
