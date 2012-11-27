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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_RINGWORLD_DIALOGS_H
#define TSAGE_RINGWORLD_DIALOGS_H

#include "gui/options.h"
#include "tsage/events.h"
#include "tsage/graphics.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/system.h"

namespace TsAGE {

namespace Ringworld {

class RightClickButton : public GfxButton {
private:
	GfxSurface *_savedButton;
public:
	int _buttonIndex;

	RightClickButton(int buttonIndex, int xp, int yp);
	~RightClickButton() { delete _savedButton; }

	virtual void highlight();
};

class RightClickDialog : public GfxDialog {
private:
	GfxSurface _surface;
	RightClickButton *_highlightedButton;
	int _selectedAction;
	RightClickButton _walkButton, _lookButton, _useButton, _talkButton, _inventoryButton, _optionsButton;

	RightClickButton *findButton(const Common::Point &pt);
public:
	RightClickDialog();
	~RightClickDialog();

	virtual void draw();
	virtual bool process(Event &event);
	void execute();
};

class OptionsDialog : public ModalDialog {
private:
	GfxButton _btnSave, _btnRestore, _btnRestart;
	GfxButton _btnQuit, _btnResume;
	GfxButton _btnSound;
	GfxMessage _gfxMessage;
public:
	OptionsDialog();
	virtual ~OptionsDialog() {}
	GfxButton *execute() { return GfxDialog::execute(&_btnResume); }

	static void show();
};

/*--------------------------------------------------------------------------*/

class GfxInvImage : public GfxImage {
public:
	InvObject *_invObject;
public:
	GfxInvImage() : GfxImage(), _invObject(NULL) {}

	virtual bool process(Event &event);
};

#define MAX_INVOBJECT_DISPLAY 20

class InventoryDialog : public ModalDialog {
private:
	Common::Array<GfxInvImage *> _images;
	GfxButton _btnOk, _btnLook;
public:
	InventoryDialog();
	virtual ~InventoryDialog();
	void execute();

	static void show();
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
