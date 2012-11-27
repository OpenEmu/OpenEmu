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

#include "common/substream.h"
#include "graphics/decoders/bmp.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"

#include "hugo/hugo.h"
#include "hugo/display.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"
#include "hugo/sound.h"
#include "hugo/util.h"

namespace Hugo {

TopMenu::TopMenu(HugoEngine *vm) : Dialog(0, 0, kMenuWidth, kMenuHeight), _arrayBmp(0), _arraySize(0),
	_vm(vm) {
	init();
}

TopMenu::~TopMenu() {
	for (int i = 0; i < _arraySize; i++) {
		_arrayBmp[i * 2]->free();
		delete _arrayBmp[i * 2];
		_arrayBmp[i * 2 + 1]->free();
		delete _arrayBmp[i * 2 + 1];
	}
	delete[] _arrayBmp;
}

void TopMenu::init() {
	int x = kMenuX;
	int y = kMenuY;

	_whatButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "What is it?", kCmdWhat);
	_musicButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Music", kCmdMusic);
	_soundFXButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Sound FX", kCmdSoundFX);
	_saveButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Save game", kCmdSave);
	_loadButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Load game", kCmdLoad);
	_recallButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Recall last command", kCmdRecall);
	_turboButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Turbo", kCmdTurbo);
	_lookButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Description of the scene", kCmdLook);
	_inventButton = new GUI::PicButtonWidget(this, x, y, kButtonWidth, kButtonHeight, "Inventory", kCmdInvent);
}

void TopMenu::reflowLayout() {
	_w = g_system->getOverlayWidth();

	int scale = (_w > 320 ? 2 : 1);

	_h = kMenuHeight * scale;

	int x = kMenuX * scale;
	int y = kMenuY * scale;

	_whatButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	_musicButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	_soundFXButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_saveButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	_loadButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_recallButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	_turboButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	x += kButtonSpace;

	_lookButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	_inventButton->resize(x * scale, y * scale, kButtonWidth * scale, kButtonHeight * scale);
	x += kButtonWidth + kButtonPad;

	// Set the graphics to the 'on' buttons, except for the variable ones
	_whatButton->setGfx(_arrayBmp[4 * kMenuWhat + scale - 1]);
	_musicButton->setGfx(_arrayBmp[4 * kMenuMusic + scale - 1 + ((_vm->_config._musicFl) ? 0 : 2)]);
	_soundFXButton->setGfx(_arrayBmp[4 * kMenuSoundFX + scale - 1 + ((_vm->_config._soundFl) ? 0 : 2)]);
	_saveButton->setGfx(_arrayBmp[4 * kMenuSave + scale - 1]);
	_loadButton->setGfx(_arrayBmp[4 * kMenuLoad + scale - 1]);
	_recallButton->setGfx(_arrayBmp[4 * kMenuRecall + scale - 1]);
	_turboButton->setGfx(_arrayBmp[4 * kMenuTurbo + scale - 1 + ((_vm->_config._turboFl) ? 0 : 2)]);
	_lookButton->setGfx(_arrayBmp[4 * kMenuLook + scale - 1]);
	_inventButton->setGfx(_arrayBmp[4 * kMenuInventory + scale - 1]);
}

void TopMenu::loadBmpArr(Common::SeekableReadStream &in) {
	_arraySize = in.readUint16BE();

	delete _arrayBmp;
	_arrayBmp = new Graphics::Surface *[_arraySize * 2];
	for (int i = 0; i < _arraySize; i++) {
		uint16 bmpSize = in.readUint16BE();
		uint32 filPos = in.pos();
		Common::SeekableSubReadStream stream(&in, filPos, filPos + bmpSize);

		Graphics::BitmapDecoder bitmapDecoder;
		if (!bitmapDecoder.loadStream(stream))
			error("TopMenu::loadBmpArr(): Could not load bitmap");

		const Graphics::Surface *bitmapSrc = bitmapDecoder.getSurface();
		if (bitmapSrc->format.bytesPerPixel == 1)
			error("TopMenu::loadBmpArr(): Unhandled paletted image");

		_arrayBmp[i * 2] = bitmapSrc->convertTo(g_system->getOverlayFormat());
		_arrayBmp[i * 2 + 1] = new Graphics::Surface();
		_arrayBmp[i * 2 + 1]->create(_arrayBmp[i * 2]->w * 2, _arrayBmp[i * 2]->h * 2, g_system->getOverlayFormat());
		byte *src = (byte *)_arrayBmp[i * 2]->pixels;
		byte *dst = (byte *)_arrayBmp[i * 2 + 1]->pixels;

		for (int j = 0; j < _arrayBmp[i * 2]->h; j++) {
			src = (byte *)_arrayBmp[i * 2]->getBasePtr(0, j);
			dst = (byte *)_arrayBmp[i * 2 + 1]->getBasePtr(0, j * 2);
			for (int k = _arrayBmp[i * 2]->w; k > 0; k--) {
				for (int m = _arrayBmp[i * 2]->format.bytesPerPixel; m > 0; m--) {
					*dst++ = *src++;
				}
				src -= _arrayBmp[i * 2]->format.bytesPerPixel;

				for (int m = _arrayBmp[i * 2]->format.bytesPerPixel; m > 0; m--) {
					*dst++ = *src++;
				}
			}
			src = (byte *)_arrayBmp[i * 2 + 1]->getBasePtr(0, j * 2);
			dst = (byte *)_arrayBmp[i * 2 + 1]->getBasePtr(0, j * 2 + 1);
			for (int k = _arrayBmp[i * 2 + 1]->pitch; k > 0; k--) {
				*dst++ = *src++;
			}
		}

		in.seek(filPos + bmpSize);
	}
}

void TopMenu::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	switch (command) {
	case kCmdWhat:
		close();
		_vm->getGameStatus()._helpFl = true;

		break;
	case kCmdMusic:
		_vm->_sound->toggleMusic();
		_musicButton->setGfx(_arrayBmp[4 * kMenuMusic + (g_system->getOverlayWidth() > 320 ? 2 : 1) - 1 + ((_vm->_config._musicFl) ? 0 : 2)]);
		_musicButton->draw();
		g_gui.theme()->updateScreen();
		g_system->updateScreen();
		g_system->delayMillis(500);
		close();
		break;
	case kCmdSoundFX:
		_vm->_sound->toggleSound();
		reflowLayout();
		_soundFXButton->draw();
		g_gui.theme()->updateScreen();
		g_system->updateScreen();
		g_system->delayMillis(500);
		close();
		break;
	case kCmdSave:
		close();
		if (_vm->getGameStatus()._viewState == kViewPlay) {
			if (_vm->getGameStatus()._gameOverFl)
				_vm->gameOverMsg();
			else
				_vm->_file->saveGame(-1, Common::String());
		}
		break;
	case kCmdLoad:
		close();
		_vm->_file->restoreGame(-1);
		break;
	case kCmdRecall:
		close();
		_vm->getGameStatus()._recallFl = true;
		break;
	case kCmdTurbo:
		_vm->_parser->switchTurbo();
		reflowLayout();
		_turboButton->draw();
		g_gui.theme()->updateScreen();
		g_system->updateScreen();
		g_system->delayMillis(500);
		close();
		break;
	case kCmdLook:
		close();
		_vm->_parser->command("look around");
		break;
	case kCmdInvent:
		close();
		_vm->_parser->showInventory();
		break;
	default:
		Dialog::handleCommand(sender, command, data);
	}
}

void TopMenu::handleMouseUp(int x, int y, int button, int clickCount) {
	if (y > _h)
		close();
	else
		Dialog::handleMouseUp(x, y, button, clickCount);
}

EntryDialog::EntryDialog(const Common::String &title, const Common::String &buttonLabel, const Common::String &defaultValue) : GUI::Dialog(20, 20, 100, 50) {
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	Common::Array<Common::String> lines;
	int lineCount, buttonPos;
	int maxlineWidth = g_gui.getFont().wordWrapText(title, screenW - 2 * 30, lines);

	// Calculate the desired dialog size (maxing out at 300*180 for now)
	_w = MAX(maxlineWidth, buttonWidth) + 20;

	lineCount = lines.size();

	_h = 16 + buttonHeight + 8;

	// Limit the number of lines so that the dialog still fits on the screen.
	if (lineCount > (screenH - 20 - _h) / kLineHeight) {
		lineCount = (screenH - 20 - _h) / kLineHeight;
	}
	_h += lineCount * kLineHeight;

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	// Each line is represented by one static text item.
	for (int i = 0; i < lineCount; i++) {
		new GUI::StaticTextWidget(this, 10, 10 + i * kLineHeight, maxlineWidth, kLineHeight,
								lines[i], Graphics::kTextAlignCenter);
	}

	_text = new GUI::EditTextWidget(this, 10, 10 + lineCount * (kLineHeight + 1), _w - 20, kLineHeight, "", "", 0, kCmdFinishEdit);
	_text->setEditString(defaultValue);

	_h += kLineHeight + 5;

	buttonPos = (_w - buttonWidth) / 2;

	new GUI::ButtonWidget(this, buttonPos, _h - buttonHeight - 8, buttonWidth, buttonHeight, buttonLabel, 0, kCmdButton, Common::ASCII_RETURN);	// Confirm dialog

}

EntryDialog::~EntryDialog() {
}

void EntryDialog::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	switch (command) {
	case kCmdButton:
	case kCmdFinishEdit:
		close();
		break;
	default:
		Dialog::handleCommand(sender, command, data);
	}
}

} // End of namespace Hugo
