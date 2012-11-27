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

#include "mohawk/mohawk.h"
#include "mohawk/dialogs.h"

#include "gui/gui-manager.h"
#include "gui/ThemeEngine.h"
#include "gui/widget.h"
#include "common/system.h"
#include "common/translation.h"

#ifdef ENABLE_MYST
#include "mohawk/myst.h"
#endif

#ifdef ENABLE_RIVEN
#include "mohawk/riven.h"
#endif

namespace Mohawk {

// This used to have GUI::Dialog("MohawkDummyDialog"), but that doesn't work with the gui branch merge :P (Sorry, Tanoku!)
InfoDialog::InfoDialog(MohawkEngine *vm, const Common::String &message) : _vm(vm), GUI::Dialog(0, 0, 1, 1), _message(message) {
	_backgroundType = GUI::ThemeEngine::kDialogBackgroundSpecial;

	_text = new GUI::StaticTextWidget(this, 0, 0, 10, 10, _message, Graphics::kTextAlignCenter);
}

void InfoDialog::setInfoText(const Common::String &message) {
	_message = message;
	_text->setLabel(_message);
}

void InfoDialog::reflowLayout() {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int width = g_gui.getStringWidth(_message) + 16;
	int height = g_gui.getFontHeight() + 8;

	_w = width;
	_h = height;
	_x = (screenW - width) / 2;
	_y = (screenH - height) / 2;

	_text->setSize(_w, _h);
}

PauseDialog::PauseDialog(MohawkEngine *vm, const Common::String &message) : InfoDialog(vm, message) {
}

void PauseDialog::handleKeyDown(Common::KeyState state) {
	if (state.ascii == ' ')
		close();
	else
		InfoDialog::handleKeyDown(state);
}

enum {
	kZipCmd = 'ZIPM',
	kTransCmd = 'TRAN',
	kWaterCmd = 'WATR',
	kDropCmd = 'DROP',
	kMapCmd = 'SMAP',
	kMenuCmd = 'MENU'
};

#ifdef ENABLE_MYST

MystOptionsDialog::MystOptionsDialog(MohawkEngine_Myst* vm) : GUI::OptionsDialog("", 120, 120, 360, 200), _vm(vm) {
	// I18N: Option for fast scene switching
	_zipModeCheckbox = new GUI::CheckboxWidget(this, 15, 10, 300, 15, _("~Z~ip Mode Activated"), 0, kZipCmd);
	_transitionsCheckbox = new GUI::CheckboxWidget(this, 15, 30, 300, 15, _("~T~ransitions Enabled"), 0, kTransCmd);
	// I18N: Drop book page
	_dropPageButton = new GUI::ButtonWidget(this, 15, 60, 100, 25, _("~D~rop Page"), 0, kDropCmd);

	// Myst ME only has maps
	if (_vm->getFeatures() & GF_ME)
		_showMapButton = new GUI::ButtonWidget(this, 15, 95, 100, 25, _("~S~how Map"), 0, kMapCmd);
	else
		_showMapButton = 0;

	// Myst demo only has a menu
	if (_vm->getFeatures() & GF_DEMO)
		_returnToMenuButton = new GUI::ButtonWidget(this, 15, 95, 100, 25, _("~M~ain Menu"), 0, kMenuCmd);
	else
		_returnToMenuButton = 0;

	new GUI::ButtonWidget(this, 95, 160, 120, 25, _("~O~K"), 0, GUI::kOKCmd);
	new GUI::ButtonWidget(this, 225, 160, 120, 25, _("~C~ancel"), 0, GUI::kCloseCmd);
}

MystOptionsDialog::~MystOptionsDialog() {
}

void MystOptionsDialog::open() {
	Dialog::open();

	_dropPageButton->setEnabled(_vm->_gameState->_globals.heldPage != 0);

	if (_showMapButton)
		_showMapButton->setEnabled(_vm->_scriptParser &&
				_vm->_scriptParser->getMap());

	// Return to menu button is not enabled on the menu
	if (_returnToMenuButton)
		_returnToMenuButton->setEnabled(_vm->_scriptParser &&
				_vm->getCurStack() != kDemoStack);

	// Zip mode is disabled in the demo
	if (_vm->getFeatures() & GF_DEMO)
		_zipModeCheckbox->setEnabled(false);

	_zipModeCheckbox->setState(_vm->_gameState->_globals.zipMode);
	_transitionsCheckbox->setState(_vm->_gameState->_globals.transitions);
}

void MystOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kZipCmd:
		_vm->_gameState->_globals.zipMode = _zipModeCheckbox->getState();
		break;
	case kTransCmd:
		_vm->_gameState->_globals.transitions = _transitionsCheckbox->getState();
		break;
	case kDropCmd:
		_vm->_needsPageDrop = true;
		close();
		break;
	case kMapCmd:
		_vm->_needsShowMap = true;
		close();
	break;
	case kMenuCmd:
		_vm->_needsShowDemoMenu = true;
		close();
	break;
	case GUI::kCloseCmd:
		close();
		break;
	default:
		GUI::OptionsDialog::handleCommand(sender, cmd, data);
	}
}

#endif

#ifdef ENABLE_RIVEN

RivenOptionsDialog::RivenOptionsDialog(MohawkEngine_Riven* vm) : GUI::OptionsDialog("", 120, 120, 360, 200), _vm(vm) {
	_zipModeCheckbox = new GUI::CheckboxWidget(this, 15, 10, 300, 15, _("~Z~ip Mode Activated"), 0, kZipCmd);
	_waterEffectCheckbox = new GUI::CheckboxWidget(this, 15, 30, 300, 15, _("~W~ater Effect Enabled"), 0, kWaterCmd);

	new GUI::ButtonWidget(this, 95, 160, 120, 25, _("~O~K"), 0, GUI::kOKCmd);
	new GUI::ButtonWidget(this, 225, 160, 120, 25, _("~C~ancel"), 0, GUI::kCloseCmd);
}

RivenOptionsDialog::~RivenOptionsDialog() {
}

void RivenOptionsDialog::open() {
	Dialog::open();

	_zipModeCheckbox->setState(_vm->_vars["azip"] != 0);
	_waterEffectCheckbox->setState(_vm->_vars["waterenabled"] != 0);
}

void RivenOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kZipCmd:
		_vm->_vars["azip"] = _zipModeCheckbox->getState() ? 1 : 0;
		break;
	case kWaterCmd:
		_vm->_vars["waterenabled"] = _waterEffectCheckbox->getState() ? 1 : 0;
		break;
	case GUI::kCloseCmd:
		close();
		break;
	default:
		GUI::OptionsDialog::handleCommand(sender, cmd, data);
	}
}

#endif

} // End of namespace Mohawk
