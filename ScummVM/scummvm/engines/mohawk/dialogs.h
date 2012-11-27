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

#ifndef MOHAWK_DIALOGS_H
#define MOHAWK_DIALOGS_H

#include "mohawk/mohawk.h"

#include "common/events.h"
#include "common/str.h"
#include "gui/dialog.h"
#include "gui/options.h"
#include "gui/widget.h"
#include "gui/widgets/list.h"

namespace Mohawk {

class MohawkEngine;

class InfoDialog : public GUI::Dialog {
protected:
	MohawkEngine *_vm;
	Common::String _message;
	GUI::StaticTextWidget *_text;

public:
	InfoDialog(MohawkEngine *vm, const Common::String &message);

	void setInfoText(const Common::String &message);

	virtual void handleMouseDown(int x, int y, int button, int clickCount) {
		setResult(0);
		close();
	}

	virtual void handleKeyDown(Common::KeyState state) {
		setResult(state.ascii);
		close();
	}

	virtual void reflowLayout();
};

class PauseDialog : public InfoDialog {
public:
	PauseDialog(MohawkEngine* vm, const Common::String &message);
	virtual void handleKeyDown(Common::KeyState state);
};

#ifdef ENABLE_MYST

class MohawkEngine_Myst;

class MystOptionsDialog : public GUI::OptionsDialog {
public:
	MystOptionsDialog(MohawkEngine_Myst *vm);
	~MystOptionsDialog();
	void open();

	virtual void handleCommand(GUI::CommandSender*, uint32, uint32);
private:
	MohawkEngine_Myst *_vm;
	GUI::CheckboxWidget *_zipModeCheckbox;
	GUI::CheckboxWidget *_transitionsCheckbox;
	GUI::ButtonWidget *_dropPageButton;
	GUI::ButtonWidget *_showMapButton;
	GUI::ButtonWidget *_returnToMenuButton;
};

#endif

#ifdef ENABLE_RIVEN

class MohawkEngine_Riven;

class RivenOptionsDialog : public GUI::OptionsDialog {
public:
	RivenOptionsDialog(MohawkEngine_Riven *vm);
	~RivenOptionsDialog();
	void open();

	virtual void handleCommand(GUI::CommandSender*, uint32, uint32);
private:
	MohawkEngine_Riven *_vm;
	GUI::CheckboxWidget *_zipModeCheckbox;
	GUI::CheckboxWidget *_waterEffectCheckbox;
};

#endif

} // End of namespace Mohawk

#endif
