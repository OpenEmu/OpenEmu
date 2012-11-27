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

#include "common/file.h"
#include "common/util.h"
#include "common/savefile.h"
#include "common/events.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "graphics/palette.h"
#include "graphics/thumbnail.h"
#include "gui/message.h"

#include "sword1/control.h"
#include "sword1/logic.h"
#include "sword1/mouse.h"
#include "sword1/music.h"
#include "sword1/objectman.h"
#include "sword1/resman.h"
#include "sword1/sound.h"
#include "sword1/sword1.h"
#include "sword1/sworddefs.h"
#include "sword1/swordres.h"
#include "sword1/screen.h"

namespace Sword1 {

enum LangStrings {
	STR_PAUSED = 0,
	STR_INSERT_CD_A,
	STR_INSERT_CD_B,
	STR_INCORRECT_CD,
	STR_SAVE,
	STR_RESTORE,
	STR_RESTART,
	STR_START,
	STR_QUIT,
	STR_SPEED,
	STR_VOLUME,
	STR_TEXT,
	STR_DONE,
	STR_OK,
	STR_CANCEL,
	STR_MUSIC,
	STR_SPEECH,
	STR_FX,
	STR_THE_END,
	STR_DRIVE_FULL
};

enum ButtonIds {
	BUTTON_DONE = 1,
	BUTTON_MAIN_PANEL,
	BUTTON_SAVE_PANEL,
	BUTTON_RESTORE_PANEL,
	BUTTON_RESTART,
	BUTTON_QUIT,
	BUTTON_SPEED,
	BUTTON_VOLUME_PANEL,
	BUTTON_TEXT,
	BUTTON_CONFIRM,
//-
	BUTTON_SCROLL_UP_FAST,
	BUTTON_SCROLL_UP_SLOW,
	BUTTON_SCROLL_DOWN_SLOW,
	BUTTON_SCROLL_DOWN_FAST,
	BUTTON_SAVE_SELECT1,
	BUTTON_SAVE_SELECT2,
	BUTTON_SAVE_SELECT3,
	BUTTON_SAVE_SELECT4,
	BUTTON_SAVE_SELECT5,
	BUTTON_SAVE_SELECT6,
	BUTTON_SAVE_SELECT7,
	BUTTON_SAVE_SELECT8,
	BUTTON_SAVE_RESTORE_OKAY,
	BUTTON_SAVE_CANCEL,
//-
	CONFIRM_OKAY,
	CONFIRM_CANCEL
};

enum TextModes {
	TEXT_LEFT_ALIGN = 0,
	TEXT_CENTER,
	TEXT_RIGHT_ALIGN,
	TEXT_RED_FONT = 128
};

ControlButton::ControlButton(uint16 x, uint16 y, uint32 resId, uint8 id, uint8 flag, ResMan *pResMan, uint8 *screenBuf, OSystem *system) {
	_x = x;
	_y = y;
	_id = id;
	_flag = flag;
	_resId = resId;
	_resMan = pResMan;
	_frameIdx = 0;
	_resMan->resOpen(_resId);
	FrameHeader *tmp = _resMan->fetchFrame(_resMan->fetchRes(_resId), 0);
	_width = _resMan->getUint16(tmp->width);
	_width = (_width > SCREEN_WIDTH) ? SCREEN_WIDTH : _width;
	_height = _resMan->getUint16(tmp->height);
	if ((x == 0) && (y == 0)) { // center the frame (used for panels);
		_x = (((640 - _width) / 2) < 0) ? 0 : ((640 - _width) / 2);
		_y = (((480 - _height) / 2) < 0) ? 0 : ((480 - _height) / 2);
	}
	_dstBuf = screenBuf + _y * SCREEN_WIDTH + _x;
	_system = system;
}

ControlButton::~ControlButton() {
	_resMan->resClose(_resId);
}

bool ControlButton::isSaveslot() {
	return ((_resId >= SR_SLAB1) && (_resId <= SR_SLAB4));
}

void ControlButton::draw() {
	FrameHeader *fHead = _resMan->fetchFrame(_resMan->fetchRes(_resId), _frameIdx);
	uint8 *src = (uint8 *)fHead + sizeof(FrameHeader);
	uint8 *dst = _dstBuf;

	if (SwordEngine::isPsx() && _resId) {
		uint8 *HIFbuf = (uint8 *)malloc(_resMan->readUint16(&fHead->height) * _resMan->readUint16(&fHead->width));
		memset(HIFbuf, 0, _resMan->readUint16(&fHead->height) * _resMan->readUint16(&fHead->width));
		Screen::decompressHIF(src, HIFbuf);
		src = HIFbuf;

		if (_resMan->readUint16(&fHead->width) < 300)
			for (uint16 cnt = 0; cnt < _resMan->readUint16(&fHead->height); cnt++) {
				for (uint16 cntx = 0; cntx < _resMan->readUint16(&fHead->width); cntx++)
					if (src[cntx])
						dst[cntx] = src[cntx];

				dst += SCREEN_WIDTH;
				for (uint16 cntx = 0; cntx < _resMan->readUint16(&fHead->width); cntx++)
					if (src[cntx])
						dst[cntx] = src[cntx];

				dst += SCREEN_WIDTH;
				src += _resMan->readUint16(&fHead->width);
			}
		else if (_resId == SR_DEATHPANEL) { // Check for death panel psx version (which is 1/3 of original width)
			for (uint16 cnt = 0; cnt < _resMan->readUint16(&fHead->height) / 2; cnt++) {
				//Stretched panel is bigger than 640px, check we don't draw outside screen
				for (uint16 cntx = 0; (cntx < (_resMan->readUint16(&fHead->width)) / 3) && (cntx < (SCREEN_WIDTH - 3)); cntx++)
					if (src[cntx]) {
						dst[cntx * 3] = src[cntx];
						dst[cntx * 3 + 1] = src[cntx];
						dst[cntx * 3 + 2] = src[cntx];
					}
				dst += SCREEN_WIDTH;

				for (uint16 cntx = 0; cntx < (_resMan->readUint16(&fHead->width)) / 3; cntx++)
					if (src[cntx]) {
						dst[cntx * 3] = src[cntx];
						dst[cntx * 3 + 1] = src[cntx];
						dst[cntx * 3 + 2] = src[cntx];
					}
				dst += SCREEN_WIDTH;
				src += _resMan->readUint16(&fHead->width) / 3;
			}
		} else { //save slots needs to be multiplied by 2 in height
			for (uint16 cnt = 0; cnt < _resMan->readUint16(&fHead->height); cnt++) {
				for (uint16 cntx = 0; cntx < _resMan->readUint16(&fHead->width) / 2; cntx++)
					if (src[cntx]) {
						dst[cntx * 2] = src[cntx];
						dst[cntx * 2 + 1] = src[cntx];
					}

				dst += SCREEN_WIDTH;
				for (uint16 cntx = 0; cntx < _resMan->readUint16(&fHead->width) / 2; cntx++)
					if (src[cntx]) {
						dst[cntx * 2] = src[cntx];
						dst[cntx * 2 + 1] = src[cntx];
					}

				dst += SCREEN_WIDTH;
				src += _resMan->readUint16(&fHead->width) / 2;
			}
		}

		free(HIFbuf);
	} else
		for (uint16 cnt = 0; cnt < _resMan->readUint16(&fHead->height); cnt++) {
			for (uint16 cntx = 0; cntx < _resMan->readUint16(&fHead->width); cntx++)
				if (src[cntx])
					dst[cntx] = src[cntx];

			dst += SCREEN_WIDTH;
			src += _resMan->readUint16(&fHead->width);
		}

	_system->copyRectToScreen(_dstBuf, SCREEN_WIDTH, _x, _y, _width, _height);
}

bool ControlButton::wasClicked(uint16 mouseX, uint16 mouseY) {
	if ((_x <= mouseX) && (_y <= mouseY) && (_x + _width >= mouseX) && (_y + _height >= mouseY))
		return true;
	else
		return false;
}

void ControlButton::setSelected(uint8 selected) {
	_frameIdx = selected;
	draw();
}

Control::Control(Common::SaveFileManager *saveFileMan, ResMan *pResMan, ObjectMan *pObjMan, OSystem *system, Mouse *pMouse, Sound *pSound, Music *pMusic) {
	_saveFileMan = saveFileMan;
	_resMan = pResMan;
	_objMan = pObjMan;
	_system = system;
	_mouse = pMouse;
	_music = pMusic;
	_sound = pSound;
	_lStrings = _languageStrings + SwordEngine::_systemVars.language * 20;
	_selectedButton = 255;
	_panelShown = false;
}

void Control::askForCd() {
	_screenBuf = (uint8 *)malloc(640 * 480);
	uint32 fontId = SR_FONT;
	if (SwordEngine::_systemVars.language == BS1_CZECH)
		fontId = CZECH_SR_FONT;
	_font = (uint8 *)_resMan->openFetchRes(fontId);
	uint8 *pal = (uint8 *)_resMan->openFetchRes(SR_PALETTE);
	uint8 *palOut = (uint8 *)malloc(256 * 3);
	for (uint16 cnt = 1; cnt < 256; cnt++) {
		palOut[cnt * 3 + 0] = pal[cnt * 3 + 0] << 2;
		palOut[cnt * 3 + 1] = pal[cnt * 3 + 1] << 2;
		palOut[cnt * 3 + 2] = pal[cnt * 3 + 2] << 2;
	}
	palOut[0] = palOut[1] = palOut[2] = 0;
	_resMan->resClose(SR_PALETTE);
	_system->getPaletteManager()->setPalette(palOut, 0, 256);
	free(palOut);

	char fName[10];
	uint8 textA[50];
	sprintf(fName, "cd%d.id", SwordEngine::_systemVars.currentCD);
	sprintf((char *)textA, "%s%d", _lStrings[STR_INSERT_CD_A], SwordEngine::_systemVars.currentCD);
	bool notAccepted = true;
	bool refreshText = true;
	do {
		if (refreshText) {
			memset(_screenBuf, 0, 640 * 480);
			renderText(textA, 320, 220, TEXT_CENTER);
			renderText(_lStrings[STR_INSERT_CD_B], 320, 240, TEXT_CENTER);
			_system->copyRectToScreen(_screenBuf, 640, 0, 0, 640, 480);
		}
		delay(300);
		if (_keyPressed.keycode) {
			if (!Common::File::exists(fName)) {
				memset(_screenBuf, 0, 640 * 480);
				renderText(_lStrings[STR_INCORRECT_CD], 320, 230, TEXT_CENTER);
				_system->copyRectToScreen(_screenBuf, 640, 0, 0, 640, 480);
				delay(2000);
				refreshText = true;
			} else {
				notAccepted = false;
			}
		}
	} while (notAccepted && (!Engine::shouldQuit()));

	_resMan->resClose(fontId);
	free(_screenBuf);
}

static int volToBalance(int volL, int volR) {
	if (volL + volR == 0) {
		return 50;
	} else {
		return (100 * volL / (volL + volR));
	}
}

uint8 Control::runPanel() {
	_panelShown = true;
	_mouseDown = false;
	_restoreBuf = NULL;
	_keyPressed.reset();
	_numButtons = 0;
	_screenBuf = (uint8 *)malloc(640 * 480);
	memset(_screenBuf, 0, 640 * 480);
	_system->copyRectToScreen(_screenBuf, 640, 0, 0, 640, 480);
	_sound->quitScreen();

	uint32 fontId = SR_FONT, redFontId = SR_REDFONT;
	if (SwordEngine::_systemVars.language == BS1_CZECH) {
		fontId = CZECH_SR_FONT;
		redFontId = CZECH_SR_REDFONT;
	}
	_font = (uint8 *)_resMan->openFetchRes(fontId);
	_redFont = (uint8 *)_resMan->openFetchRes(redFontId);

	uint8 *pal = (uint8 *)_resMan->openFetchRes(SR_PALETTE);
	uint8 *palOut = (uint8 *)malloc(256 * 3);
	for (uint16 cnt = 1; cnt < 256; cnt++) {
		palOut[cnt * 3 + 0] = pal[cnt * 3 + 0] << 2;
		palOut[cnt * 3 + 1] = pal[cnt * 3 + 1] << 2;
		palOut[cnt * 3 + 2] = pal[cnt * 3 + 2] << 2;
	}
	palOut[0] = palOut[1] = palOut[2] = 0;
	_resMan->resClose(SR_PALETTE);
	_system->getPaletteManager()->setPalette(palOut, 0, 256);
	free(palOut);
	uint8 mode = 0, newMode = BUTTON_MAIN_PANEL;
	bool fullRefresh = false;
	_mouse->controlPanel(true);
	uint8 retVal = CONTROL_NOTHING_DONE;
	_music->startMusic(61, 1);

	do {
		if (newMode) {
			mode = newMode;
			fullRefresh = true;
			destroyButtons();
			memset(_screenBuf, 0, 640 * 480);
			if (mode != BUTTON_SAVE_PANEL)
				_cursorVisible = false;
		}
		switch (mode) {
		case BUTTON_MAIN_PANEL:
			if (fullRefresh)
				setupMainPanel();
			break;
		case BUTTON_SAVE_PANEL:
			if (fullRefresh) {
				setupSaveRestorePanel(true);
			}
			if (_selectedSavegame < 255) {
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
				bool visible = _cursorVisible;
				_cursorTick++;
				if (_cursorTick == 7)
					_cursorVisible = true;
				else if (_cursorTick == 14) {
					_cursorVisible = false;
					_cursorTick = 0;
				}
				if (_keyPressed.keycode)
					handleSaveKey(_keyPressed);
				else if (_cursorVisible != visible)
					showSavegameNames();
			} else {
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			}
			break;
		case BUTTON_RESTORE_PANEL:
			if (fullRefresh)
				setupSaveRestorePanel(false);
			break;
		case BUTTON_VOLUME_PANEL:
			if (fullRefresh)
				setupVolumePanel();
			break;
		default:
			break;
		}
		if (fullRefresh) {
			fullRefresh = false;
			_system->copyRectToScreen(_screenBuf, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, 480);
		}
		delay(1000 / 12);
		newMode = getClicks(mode, &retVal);
	} while ((newMode != BUTTON_DONE) && (retVal == 0) && (!Engine::shouldQuit()));

	if (SwordEngine::_systemVars.controlPanelMode == CP_NORMAL) {
		uint8 volL, volR;
		_music->giveVolume(&volL, &volR);
		ConfMan.setInt("music_volume", (int)((volR + volL) / 2));
		ConfMan.setInt("music_balance", volToBalance(volL, volR));

		_sound->giveSpeechVol(&volL, &volR);
		ConfMan.setInt("speech_volume", (int)((volR + volL) / 2));
		ConfMan.setInt("speech_balance", volToBalance(volL, volR));

		_sound->giveSfxVol(&volL, &volR);
		ConfMan.setInt("sfx_volume", (int)((volR + volL) / 2));
		ConfMan.setInt("sfx_balance", volToBalance(volL, volR));

		ConfMan.setBool("subtitles", SwordEngine::_systemVars.showText == 1);
		ConfMan.flushToDisk();
	}

	destroyButtons();
	_resMan->resClose(fontId);
	_resMan->resClose(redFontId);
	memset(_screenBuf, 0, 640 * 480);
	_system->copyRectToScreen(_screenBuf, 640, 0, 0, 640, 480);
	free(_screenBuf);
	_mouse->controlPanel(false);
	// Can also be used to end the control panel music.
	_music->startMusic(Logic::_scriptVars[CURRENT_MUSIC], 1);
	_sound->newScreen(Logic::_scriptVars[SCREEN]);
	_panelShown = false;
	return retVal;
}

uint8 Control::getClicks(uint8 mode, uint8 *retVal) {
	uint8 checkButtons = _numButtons;
	if (mode == BUTTON_VOLUME_PANEL) {
		handleVolumeClicks();
		checkButtons = 1;
	}

	uint8 flag = 0;
	if (_keyPressed.keycode == Common::KEYCODE_ESCAPE)
		flag = kButtonCancel;
	else if (_keyPressed.keycode == Common::KEYCODE_RETURN || _keyPressed.keycode == Common::KEYCODE_KP_ENTER)
		flag = kButtonOk;

	if (flag) {
		for (uint8 cnt = 0; cnt < checkButtons; cnt++)
			if (_buttons[cnt]->_flag == flag)
				return handleButtonClick(_buttons[cnt]->_id, mode, retVal);
	}

	if (!_mouseState)
		return 0;
	if (_mouseState & BS1L_BUTTON_DOWN)
		for (uint8 cnt = 0; cnt < checkButtons; cnt++)
			if (_buttons[cnt]->wasClicked(_mouseCoord.x, _mouseCoord.y)) {
				_selectedButton = cnt;
				_buttons[cnt]->setSelected(1);
				if (_buttons[cnt]->isSaveslot())
					showSavegameNames();
			}
	if (_mouseState & BS1L_BUTTON_UP) {
		for (uint8 cnt = 0; cnt < checkButtons; cnt++)
			if (_buttons[cnt]->wasClicked(_mouseCoord.x, _mouseCoord.y))
				if (_selectedButton == cnt) {
					// saveslots stay selected after clicking
					if (!_buttons[cnt]->isSaveslot())
						_buttons[cnt]->setSelected(0);
					_selectedButton = 255;
					return handleButtonClick(_buttons[cnt]->_id, mode, retVal);
				}
		if (_selectedButton < checkButtons) {
			_buttons[_selectedButton]->setSelected(0);
			if (_buttons[_selectedButton]->isSaveslot())
				showSavegameNames();
		}
		_selectedButton = 255;
	}
	if (_mouseState & BS1_WHEEL_UP) {
		for (uint8 cnt = 0; cnt < checkButtons; cnt++)
			if (_buttons[cnt]->_id == BUTTON_SCROLL_UP_SLOW)
				return handleButtonClick(_buttons[cnt]->_id, mode, retVal);
	}
	if (_mouseState & BS1_WHEEL_DOWN) {
		for (uint8 cnt = 0; cnt < checkButtons; cnt++)
			if (_buttons[cnt]->_id == BUTTON_SCROLL_DOWN_SLOW)
				return handleButtonClick(_buttons[cnt]->_id, mode, retVal);
	}
	return 0;
}

uint8 Control::handleButtonClick(uint8 id, uint8 mode, uint8 *retVal) {
	switch (mode) {
	case BUTTON_MAIN_PANEL:
		if (id == BUTTON_RESTART) {
			if (SwordEngine::_systemVars.controlPanelMode) // if player is dead or has just started, don't ask for confirmation
				*retVal |= CONTROL_RESTART_GAME;
			else if (getConfirm(_lStrings[STR_RESTART]))
				*retVal |= CONTROL_RESTART_GAME;
			else
				return mode;
		} else if ((id == BUTTON_RESTORE_PANEL) || (id == BUTTON_SAVE_PANEL) ||
		           (id == BUTTON_DONE) || (id == BUTTON_VOLUME_PANEL))
			return id;
		else if (id == BUTTON_TEXT) {
			SwordEngine::_systemVars.showText ^= 1;
			_buttons[5]->setSelected(SwordEngine::_systemVars.showText);
		} else if (id == BUTTON_QUIT) {
			if (getConfirm(_lStrings[STR_QUIT]))
				Engine::quitGame();
			return mode;
		}
		break;
	case BUTTON_SAVE_PANEL:
	case BUTTON_RESTORE_PANEL:
		if ((id >= BUTTON_SCROLL_UP_FAST) && (id <= BUTTON_SCROLL_DOWN_FAST))
			saveNameScroll(id, mode == BUTTON_SAVE_PANEL);
		else if ((id >= BUTTON_SAVE_SELECT1) && (id <= BUTTON_SAVE_SELECT8))
			saveNameSelect(id, mode == BUTTON_SAVE_PANEL);
		else if (id == BUTTON_SAVE_RESTORE_OKAY) {
			if (mode == BUTTON_SAVE_PANEL) {
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
				if (saveToFile()) // don't go back to main panel if save fails.
					return BUTTON_DONE;
			} else {
				if (restoreFromFile()) { // don't go back to main panel if restore fails.
					*retVal |= CONTROL_GAME_RESTORED;
					return BUTTON_MAIN_PANEL;
				}
			}
		} else if (id == BUTTON_SAVE_CANCEL) {
			_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			return BUTTON_MAIN_PANEL; // mode down to main panel
		}
		break;
	case BUTTON_VOLUME_PANEL:
		return id;
	}
	return 0;
}

void Control::deselectSaveslots() {
	for (uint8 cnt = 0; cnt < 8; cnt++)
		_buttons[cnt]->setSelected(0);
}

void Control::setupMainPanel() {
	uint32 panelId;

	if (SwordEngine::_systemVars.controlPanelMode == CP_DEATHSCREEN)
		panelId = SR_DEATHPANEL;
	else {
		if (SwordEngine::_systemVars.realLanguage == Common::EN_USA)
			panelId = SR_PANEL_AMERICAN;
		else if (SwordEngine::_systemVars.language <= BS1_SPANISH)
			panelId = SR_PANEL_ENGLISH + SwordEngine::_systemVars.language;
		else
			panelId = SR_PANEL_ENGLISH;
	}

	ControlButton *panel = new ControlButton(0, 0, panelId, 0, 0, _resMan, _screenBuf, _system);
	panel->draw();
	delete panel;

	if (SwordEngine::_systemVars.controlPanelMode != CP_NORMAL)
		createButtons(_deathButtons, 3);
	else {
		createButtons(_panelButtons, 7);
		_buttons[5]->setSelected(SwordEngine::_systemVars.showText);
	}

	if (SwordEngine::_systemVars.controlPanelMode == CP_THEEND) // end of game
		renderText(_lStrings[STR_THE_END], 480, 188 + 40, TEXT_RIGHT_ALIGN);

	if (SwordEngine::_systemVars.controlPanelMode == CP_NORMAL) { // normal panel
		renderText(_lStrings[STR_SAVE], 180, 188 + 40, TEXT_LEFT_ALIGN);
		renderText(_lStrings[STR_DONE], 460, 332 + 40, TEXT_RIGHT_ALIGN);
		renderText(_lStrings[STR_RESTORE], 180, 224 + 40, TEXT_LEFT_ALIGN);
		renderText(_lStrings[STR_RESTART], 180, 260 + 40, TEXT_LEFT_ALIGN);
		renderText(_lStrings[STR_QUIT], 180, 296 + 40, TEXT_LEFT_ALIGN);

		renderText(_lStrings[STR_VOLUME], 460, 188 + 40, TEXT_RIGHT_ALIGN);
		renderText(_lStrings[STR_TEXT], 460, 224 + 40, TEXT_RIGHT_ALIGN);
	} else {
		renderText(_lStrings[STR_RESTORE], 285, 224 + 40, TEXT_LEFT_ALIGN);
		if (SwordEngine::_systemVars.controlPanelMode == CP_NEWGAME) // just started game
			renderText(_lStrings[STR_START], 285, 260 + 40, TEXT_LEFT_ALIGN);
		else
			renderText(_lStrings[STR_RESTART], 285, 260 + 40, TEXT_LEFT_ALIGN);
		renderText(_lStrings[STR_QUIT], 285, 296 + 40, TEXT_LEFT_ALIGN);
	}
}

void Control::setupSaveRestorePanel(bool saving) {
	readSavegameDescriptions();

	FrameHeader *savePanel = _resMan->fetchFrame(_resMan->openFetchRes(SR_WINDOW), 0);
	uint16 panelX = (640 - _resMan->getUint16(savePanel->width)) / 2;
	uint16 panelY = (480 - _resMan->getUint16(savePanel->height)) / 2;
	ControlButton *panel = new ControlButton(panelX, panelY, SR_WINDOW, 0, 0, _resMan, _screenBuf, _system);
	panel->draw();
	delete panel;
	_resMan->resClose(SR_WINDOW);
	createButtons(_saveButtons, 14);
	renderText(_lStrings[STR_CANCEL], _saveButtons[13].x - 10, _saveButtons[13].y, TEXT_RIGHT_ALIGN);
	if (saving) {
		renderText(_lStrings[STR_SAVE], _saveButtons[12].x + 30, _saveButtons[13].y, TEXT_LEFT_ALIGN);
	} else {
		renderText(_lStrings[STR_RESTORE], _saveButtons[12].x + 30, _saveButtons[13].y, TEXT_LEFT_ALIGN);
	}
	readSavegameDescriptions();
	_selectedSavegame = 255;
	showSavegameNames();
}

void Control::setupVolumePanel() {
	ControlButton *panel = new ControlButton(0, 0, SR_VOLUME, 0, 0, _resMan, _screenBuf, _system);
	panel->draw();
	delete panel;

	renderText(_lStrings[STR_MUSIC], 149, 39 + 40, TEXT_LEFT_ALIGN);
	renderText(_lStrings[STR_SPEECH], 320, 39 + 40, TEXT_CENTER);
	renderText(_lStrings[STR_FX], 438, 39 + 40, TEXT_LEFT_ALIGN);

	createButtons(_volumeButtons, 4);
	renderText(_lStrings[STR_DONE], _volumeButtons[0].x - 10, _volumeButtons[0].y, TEXT_RIGHT_ALIGN);

	uint8 volL, volR;
	_music->giveVolume(&volL, &volR);
	renderVolumeBar(1, volL, volR);
	_sound->giveSpeechVol(&volL, &volR);
	renderVolumeBar(2, volL, volR);
	_sound->giveSfxVol(&volL, &volR);
	renderVolumeBar(3, volL, volR);
}

void Control::handleVolumeClicks() {
	if (_mouseDown) {
		uint8 clickedId = 0;
		for (uint8 cnt = 1; cnt < 4; cnt++)
			if (_buttons[cnt]->wasClicked(_mouseCoord.x, _mouseCoord.y))
				clickedId = cnt;
		if (clickedId) { // these are circle shaped, so check again if it was clicked.
			uint8 clickDest = 0;
			int16 mouseDiffX = _mouseCoord.x - (_volumeButtons[clickedId].x + 48);
			int16 mouseDiffY = _mouseCoord.y - (_volumeButtons[clickedId].y + 48);
			int16 mouseOffs = (int16)sqrt((double)(mouseDiffX * mouseDiffX + mouseDiffY * mouseDiffY));
			// check if the player really hit the button (but not the center).
			if ((mouseOffs <= 42) && (mouseOffs >= 8)) {
				if (mouseDiffX > 8) { // right part
					if (mouseDiffY < -8) // upper right
						clickDest = 2;
					else if (ABS(mouseDiffY) <= 8) // right
						clickDest = 3;
					else                 // lower right
						clickDest = 4;
				} else if (mouseDiffX < -8) { // left part
					if (mouseDiffY < -8) // upper left
						clickDest = 8;
					else if (ABS(mouseDiffY) <= 8) // left
						clickDest = 7;
					else                 // lower left
						clickDest = 6;
				} else { // middle
					if (mouseDiffY < -8)
						clickDest = 1; // upper
					else if (mouseDiffY > 8)
						clickDest = 5; // lower
				}
			}
			_buttons[clickedId]->setSelected(clickDest);
			changeVolume(clickedId, clickDest);
		}
	} else if (_mouseState & BS1L_BUTTON_UP) {
		_buttons[1]->setSelected(0);
		_buttons[2]->setSelected(0);
		_buttons[3]->setSelected(0);
	}
}

void Control::changeVolume(uint8 id, uint8 action) {
	// ids: 1 = music, 2 = speech, 3 = sfx
	uint8 volL = 0, volR = 0;
	if (id == 1)
		_music->giveVolume(&volL, &volR);
	else if (id == 2)
		_sound->giveSpeechVol(&volL, &volR);
	else if (id == 3)
		_sound->giveSfxVol(&volL, &volR);

	int8 direction = 0;
	if ((action >= 4) && (action <= 6)) // lower part of the button => decrease volume
		direction = -1;
	else if ((action == 8) || (action == 1) || (action == 2)) // upper part => increase volume
		direction = 1;
	else if ((action == 3) || (action == 7)) // middle part => pan volume
		direction = 1;
	int8 factorL = 8, factorR = 8;
	if ((action >= 6) && (action <= 8)) { // left part => left pan
		factorL = 8;
		factorR = (action == 7) ? -8 : 0;
	} else if ((action >= 2) && (action <= 4)) { // right part
		factorR = 8;
		factorL = (action == 3) ? -8 : 0;
	}
	int16 resVolL = volL + direction * factorL;
	int16 resVolR = volR + direction * factorR;

	volL = (uint8)MAX((int16)0, MIN(resVolL, (int16)255));
	volR = (uint8)MAX((int16)0, MIN(resVolR, (int16)255));

	if (id == 1)
		_music->setVolume(volL, volR);
	else if (id == 2)
		_sound->setSpeechVol(volL, volR);
	else if (id == 3)
		_sound->setSfxVol(volL, volR);

	renderVolumeBar(id, volL, volR);
}

bool Control::getConfirm(const uint8 *title) {
	ControlButton *panel = new ControlButton(0, 0, SR_CONFIRM, 0, 0, _resMan, _screenBuf, _system);
	panel->draw();
	delete panel;
	renderText(title, 320, 160, TEXT_CENTER);
	ControlButton *buttons[2];
	buttons[0] = new ControlButton(260, 192 + 40, SR_BUTTON, 0, 0, _resMan, _screenBuf, _system);
	renderText(_lStrings[STR_OK], 640 - 260, 192 + 40, TEXT_RIGHT_ALIGN);
	buttons[1] = new ControlButton(260, 256 + 40, SR_BUTTON, 0, 0, _resMan, _screenBuf, _system);
	renderText(_lStrings[STR_CANCEL], 640 - 260, 256 + 40, TEXT_RIGHT_ALIGN);
	uint8 retVal = 0;
	uint8 clickVal = 0;
	do {
		buttons[0]->draw();
		buttons[1]->draw();
		delay(1000 / 12);
		if (_keyPressed.keycode == Common::KEYCODE_ESCAPE)
			retVal = 2;
		else if (_keyPressed.keycode == Common::KEYCODE_RETURN || _keyPressed.keycode == Common::KEYCODE_KP_ENTER)
			retVal = 1;
		if (_mouseState & BS1L_BUTTON_DOWN) {
			if (buttons[0]->wasClicked(_mouseCoord.x, _mouseCoord.y))
				clickVal = 1;
			else if (buttons[1]->wasClicked(_mouseCoord.x, _mouseCoord.y))
				clickVal = 2;
			else
				clickVal = 0;
			if (clickVal)
				buttons[clickVal - 1]->setSelected(1);
		}
		if ((_mouseState & BS1L_BUTTON_UP) && (clickVal)) {
			if (buttons[clickVal - 1]->wasClicked(_mouseCoord.x, _mouseCoord.y))
				retVal = clickVal;
			else
				buttons[clickVal - 1]->setSelected(0);
			clickVal = 0;
		}
	} while (!retVal);
	delete buttons[0];
	delete buttons[1];
	return retVal == 1;
}

bool Control::keyAccepted(uint16 ascii) {
	static const char allowedSpecials[] = ",.:-()?! \"\'";
	if (((ascii >= 'A') && (ascii <= 'Z')) ||
	        ((ascii >= 'a') && (ascii <= 'z')) ||
	        ((ascii >= '0') && (ascii <= '9')) ||
	        strchr(allowedSpecials, ascii))
		return true;
	else
		return false;
}

void Control::handleSaveKey(Common::KeyState kbd) {
	if (_selectedSavegame < 255) {
		uint8 len = _saveNames[_selectedSavegame].size();
		if ((kbd.keycode == Common::KEYCODE_BACKSPACE) && len)  // backspace
			_saveNames[_selectedSavegame].deleteLastChar();
		else if (kbd.ascii && keyAccepted(kbd.ascii) && (len < 31)) {
			_saveNames[_selectedSavegame].insertChar(kbd.ascii, len);
		}
		showSavegameNames();
	}
}

bool Control::saveToFile() {
	if ((_selectedSavegame == 255) || _saveNames[_selectedSavegame].size() == 0)
		return false; // no saveslot selected or no name entered
	saveGameToFile(_selectedSavegame);
	return true;
}

bool Control::restoreFromFile() {
	if (_selectedSavegame < 255) {
		return restoreGameFromFile(_selectedSavegame);
	} else
		return false;
}

void Control::readSavegameDescriptions() {
	char saveName[40];
	Common::String pattern = "sword1.???";
	Common::StringArray filenames = _saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	_saveNames.clear();

	int num = 0;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		while (num < slotNum) {
			_saveNames.push_back("");
			num++;
		}

		if (slotNum >= 0 && slotNum <= 999) {
			num++;
			Common::InSaveFile *in = _saveFileMan->openForLoading(*file);
			if (in) {
				in->readUint32LE(); // header
				in->read(saveName, 40);
				_saveNames.push_back(saveName);
				delete in;
			}
		}
	}

	for (int i = _saveNames.size(); i < 1000; i++)
		_saveNames.push_back("");

	_saveScrollPos = 0;
	_selectedSavegame = 255;
	_saveFiles = _numSaves = _saveNames.size();
}

bool Control::isPanelShown() {
	return _panelShown;
}

int Control::displayMessage(const char *altButton, const char *message, ...) {
	char buf[STRINGBUFLEN];
	va_list va;

	va_start(va, message);
	vsnprintf(buf, STRINGBUFLEN, message, va);
	va_end(va);

	GUI::MessageDialog dialog(buf, "OK", altButton);
	int result = dialog.runModal();
	_mouse->setPointer(MSE_POINTER, 0);
	return result;
}

bool Control::savegamesExist() {
	Common::String pattern = "sword1.???";
	Common::StringArray saveNames = _saveFileMan->listSavefiles(pattern);
	return saveNames.size() > 0;
}

void Control::checkForOldSaveGames() {
	Common::InSaveFile *inf = _saveFileMan->openForLoading("SAVEGAME.INF");

	if (!inf) {
		delete inf;
		return;
	}

	GUI::MessageDialog dialog0(
	    _("ScummVM found that you have old savefiles for Broken Sword 1 that should be converted.\n"
	      "The old save game format is no longer supported, so you will not be able to load your games if you don't convert them.\n\n"
	      "Press OK to convert them now, otherwise you will be asked again the next time you start the game.\n"), _("OK"), _("Cancel"));

	int choice = dialog0.runModal();
	if (choice == GUI::kMessageCancel) {
		// user pressed cancel
		return;
	}

	// Convert every save slot we find in the index file to the new format
	uint8 saveName[32];
	uint8 slot = 0;
	uint8 ch;

	memset(saveName, 0, sizeof(saveName));

	do {
		uint8 pos = 0;
		do {
			ch = inf->readByte();
			if (pos < sizeof(saveName) - 1) {
				if ((ch == 10) || (ch == 255) || (inf->eos()))
					saveName[pos++] = '\0';
				else if (ch >= 32)
					saveName[pos++] = ch;
			}
		} while ((ch != 10) && (ch != 255) && (!inf->eos()));

		if (pos > 1)    // if the slot has a description
			convertSaveGame(slot, (char *)saveName);
		slot++;
	} while ((ch != 255) && (!inf->eos()));

	delete inf;

	// Delete index file
	_saveFileMan->removeSavefile("SAVEGAME.INF");
}

void Control::showSavegameNames() {
	for (uint8 cnt = 0; cnt < 8; cnt++) {
		_buttons[cnt]->draw();
		uint8 textMode = TEXT_LEFT_ALIGN;
		uint16 ycoord = _saveButtons[cnt].y + 2;
		uint8 str[40];
		sprintf((char *)str, "%d. %s", cnt + _saveScrollPos + 1, _saveNames[cnt + _saveScrollPos].c_str());
		if (cnt + _saveScrollPos == _selectedSavegame) {
			textMode |= TEXT_RED_FONT;
			ycoord += 2;
			if (_cursorVisible)
				strcat((char *)str, "_");
		}
		renderText(str, _saveButtons[cnt].x + 6, ycoord, textMode);
	}
}

void Control::saveNameSelect(uint8 id, bool saving) {
	deselectSaveslots();
	_buttons[id - BUTTON_SAVE_SELECT1]->setSelected(1);
	uint8 num = (id - BUTTON_SAVE_SELECT1) + _saveScrollPos;
	if (saving && (_selectedSavegame != 255)) // the player may have entered something, clear it again
		_saveNames[_selectedSavegame] = _oldName;
	if (num < _saveFiles) {
		_selectedSavegame = num;
		_oldName = _saveNames[num]; // save for later
	} else {
		if (!saving)
			_buttons[id - BUTTON_SAVE_SELECT1]->setSelected(0); // no save in slot, deselect it
		else {
			if (_saveFiles <= num)
				_saveFiles = num + 1;
			_selectedSavegame = num;
			_oldName.clear();
		}
	}
	if (_selectedSavegame < 255)
		_cursorTick = 0;
	showSavegameNames();
}

void Control::saveNameScroll(uint8 scroll, bool saving) {
	uint16 maxScroll;
	if (saving)
		maxScroll = 64;
	else
		maxScroll = _saveFiles; // for loading, we can only scroll as far as there are savegames
	if (scroll == BUTTON_SCROLL_UP_FAST) {
		if (_saveScrollPos >= 8)
			_saveScrollPos -= 8;
		else
			_saveScrollPos = 0;
	} else if (scroll == BUTTON_SCROLL_UP_SLOW) {
		if (_saveScrollPos >= 1)
			_saveScrollPos--;
	} else if (scroll == BUTTON_SCROLL_DOWN_SLOW) {
		if (_saveScrollPos + 8 < maxScroll)
			_saveScrollPos++;
	} else if (scroll == BUTTON_SCROLL_DOWN_FAST) {
		if (_saveScrollPos + 16 < maxScroll)
			_saveScrollPos += 8;
		else {
			if (maxScroll >= 8)
				_saveScrollPos = maxScroll - 8;
			else
				_saveScrollPos = 0;
		}
	}
	_selectedSavegame = 255; // deselect savegame
	deselectSaveslots();
	showSavegameNames();
}

void Control::createButtons(const ButtonInfo *buttons, uint8 num) {
	for (uint8 cnt = 0; cnt < num; cnt++) {
		_buttons[cnt] = new ControlButton(buttons[cnt].x, buttons[cnt].y, buttons[cnt].resId, buttons[cnt].id, buttons[cnt].flag, _resMan, _screenBuf, _system);
		_buttons[cnt]->draw();
	}
	_numButtons = num;
}

void Control::destroyButtons() {
	for (uint8 cnt = 0; cnt < _numButtons; cnt++)
		delete _buttons[cnt];
	_numButtons = 0;
}

uint16 Control::getTextWidth(const uint8 *str) {
	uint16 width = 0;
	while (*str) {
		width += _resMan->getUint16(_resMan->fetchFrame(_font, *str - 32)->width) - 3;
		str++;
	}
	return width;
}

void Control::renderText(const uint8 *str, uint16 x, uint16 y, uint8 mode) {
	uint8 *font = _font;
	if (mode & TEXT_RED_FONT) {
		mode &= ~TEXT_RED_FONT;
		font = _redFont;
	}

	if (mode == TEXT_RIGHT_ALIGN) // negative x coordinate means right-aligned.
		x -= getTextWidth(str);
	else if (mode == TEXT_CENTER)
		x -= getTextWidth(str) / 2;

	uint16 destX = x;
	while (*str) {
		uint8 *dst = _screenBuf + y * SCREEN_WIDTH + destX;

		FrameHeader *chSpr = _resMan->fetchFrame(font, *str - 32);
		uint8 *sprData = (uint8 *)chSpr + sizeof(FrameHeader);
		uint8 *HIFbuf = NULL;

		if (SwordEngine::isPsx()) { //Text fonts are compressed in psx version
			HIFbuf = (uint8 *)malloc(_resMan->getUint16(chSpr->height) * _resMan->getUint16(chSpr->width));
			memset(HIFbuf, 0, _resMan->getUint16(chSpr->height) * _resMan->getUint16(chSpr->width));
			Screen::decompressHIF(sprData, HIFbuf);
			sprData = HIFbuf;
		}

		for (uint16 cnty = 0; cnty < _resMan->getUint16(chSpr->height); cnty++) {
			for (uint16 cntx = 0; cntx < _resMan->getUint16(chSpr->width); cntx++) {
				if (sprData[cntx])
					dst[cntx] = sprData[cntx];
			}

			if (SwordEngine::isPsx()) { //On PSX version we need to double horizontal lines
				dst += SCREEN_WIDTH;
				for (uint16 cntx = 0; cntx < _resMan->getUint16(chSpr->width); cntx++)
					if (sprData[cntx])
						dst[cntx] = sprData[cntx];
			}

			sprData += _resMan->getUint16(chSpr->width);
			dst += SCREEN_WIDTH;
		}
		destX += _resMan->getUint16(chSpr->width) - 3;
		str++;

		free(HIFbuf);
	}

	_system->copyRectToScreen(_screenBuf + y * SCREEN_WIDTH + x, SCREEN_WIDTH, x, y, (destX - x) + 3, 28);
}

void Control::renderVolumeBar(uint8 id, uint8 volL, uint8 volR) {
	uint16 destX = _volumeButtons[id].x + 20;
	uint16 destY = _volumeButtons[id].y + 116;

	for (uint8 chCnt = 0; chCnt < 2; chCnt++) {
		uint8 vol = (chCnt == 0) ? volL : volR;
		FrameHeader *frHead = _resMan->fetchFrame(_resMan->openFetchRes(SR_VLIGHT), (vol + 15) >> 4);
		uint8 *destMem = _screenBuf + destY * SCREEN_WIDTH + destX;
		uint8 *srcMem = (uint8 *)frHead + sizeof(FrameHeader);
		uint16 barHeight = _resMan->getUint16(frHead->height);
		uint8 *psxVolBuf = NULL;

		if (SwordEngine::isPsx()) {
			psxVolBuf = (uint8 *)malloc(_resMan->getUint16(frHead->height) / 2 * _resMan->getUint16(frHead->width));
			memset(psxVolBuf, 0, _resMan->getUint16(frHead->height) / 2 * _resMan->getUint16(frHead->width));
			Screen::decompressHIF(srcMem, psxVolBuf);
			srcMem = psxVolBuf;
			barHeight /= 2;
		}

		for (uint16 cnty = 0; cnty < barHeight; cnty++) {
			memcpy(destMem, srcMem, _resMan->getUint16(frHead->width));

			if (SwordEngine::isPsx()) { //linedoubling
				destMem += SCREEN_WIDTH;
				memcpy(destMem, srcMem, _resMan->getUint16(frHead->width));
			}

			srcMem += _resMan->getUint16(frHead->width);
			destMem += SCREEN_WIDTH;
		}

		_system->copyRectToScreen(_screenBuf + destY * SCREEN_WIDTH + destX, SCREEN_WIDTH, destX, destY, _resMan->getUint16(frHead->width), _resMan->getUint16(frHead->height));
		_resMan->resClose(SR_VLIGHT);
		destX += 32;

		free(psxVolBuf);
	}
}

void Control::saveGameToFile(uint8 slot) {
	char fName[15];
	uint16 cnt;
	sprintf(fName, "sword1.%03d", slot);
	uint16 liveBuf[TOTAL_SECTIONS];
	Common::OutSaveFile *outf;
	outf = _saveFileMan->openForSaving(fName);
	if (!outf) {
		// Display an error message and do nothing
		displayMessage(0, "Unable to create file '%s'. (%s)", fName, _saveFileMan->popErrorDesc().c_str());
		return;
	}

	outf->writeUint32LE(SAVEGAME_HEADER);
	outf->write(_saveNames[slot].c_str(), 40);
	outf->writeByte(SAVEGAME_VERSION);

	if (!isPanelShown()) // Generate a thumbnail only if we are outside of game menu
		Graphics::saveThumbnail(*outf);

	// Date / time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = (curTime.tm_mday & 0xFF) << 24 | ((curTime.tm_mon + 1) & 0xFF) << 16 | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = (curTime.tm_hour & 0xFF) << 8 | ((curTime.tm_min) & 0xFF);

	outf->writeUint32BE(saveDate);
	outf->writeUint16BE(saveTime);

	outf->writeUint32BE(g_engine->getTotalPlayTime() / 1000);

	_objMan->saveLiveList(liveBuf);
	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		outf->writeUint16LE(liveBuf[cnt]);

	Object *cpt = _objMan->fetchObject(PLAYER);
	Logic::_scriptVars[CHANGE_DIR] = cpt->o_dir;
	Logic::_scriptVars[CHANGE_X] = cpt->o_xcoord;
	Logic::_scriptVars[CHANGE_Y] = cpt->o_ycoord;
	Logic::_scriptVars[CHANGE_STANCE] = STAND;
	Logic::_scriptVars[CHANGE_PLACE] = cpt->o_place;

	for (cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++)
		outf->writeUint32LE(Logic::_scriptVars[cnt]);

	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	uint32 *playerRaw = (uint32 *)cpt;
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++)
		outf->writeUint32LE(playerRaw[cnt2]);
	outf->finalize();
	if (outf->err())
		displayMessage(0, "Couldn't write to file '%s'. Device full? (%s)", fName, _saveFileMan->popErrorDesc().c_str());
	delete outf;
}

bool Control::restoreGameFromFile(uint8 slot) {
	char fName[15];
	uint16 cnt;
	sprintf(fName, "sword1.%03d", slot);
	Common::InSaveFile *inf;
	inf = _saveFileMan->openForLoading(fName);
	if (!inf) {
		// Display an error message, and do nothing
		displayMessage(0, "Can't open file '%s'. (%s)", fName, _saveFileMan->popErrorDesc().c_str());
		return false;
	}

	uint saveHeader = inf->readUint32LE();
	if (saveHeader != SAVEGAME_HEADER) {
		// Display an error message, and do nothing
		displayMessage(0, "Save game '%s' is corrupt", fName);
		return false;
	}

	inf->skip(40);      // skip description
	uint8 saveVersion = inf->readByte();

	if (saveVersion > SAVEGAME_VERSION) {
		warning("Different save game version");
		return false;
	}

	if (saveVersion < 2) // These older version of the savegames used a flag to signal presence of thumbnail
		inf->skip(1);

	Graphics::skipThumbnail(*inf);

	inf->readUint32BE();    // save date
	inf->readUint16BE();    // save time

	if (saveVersion < 2) { // Before version 2 we didn't had play time feature
		g_engine->setTotalPlayTime(0);
	} else {
		g_engine->setTotalPlayTime(inf->readUint32BE() * 1000);
	}

	_restoreBuf = (uint8 *)malloc(
	                  TOTAL_SECTIONS * 2 +
	                  NUM_SCRIPT_VARS * 4 +
	                  (sizeof(Object) - 12000));

	uint16 *liveBuf = (uint16 *)_restoreBuf;
	uint32 *scriptBuf = (uint32 *)(_restoreBuf + 2 * TOTAL_SECTIONS);
	uint32 *playerBuf = (uint32 *)(_restoreBuf + 2 * TOTAL_SECTIONS + 4 * NUM_SCRIPT_VARS);

	for (cnt = 0; cnt < TOTAL_SECTIONS; cnt++)
		liveBuf[cnt] = inf->readUint16LE();

	for (cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++)
		scriptBuf[cnt] = inf->readUint32LE();

	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++)
		playerBuf[cnt2] = inf->readUint32LE();

	if (inf->err() || inf->eos()) {
		displayMessage(0, "Can't read from file '%s'. (%s)", fName, _saveFileMan->popErrorDesc().c_str());
		delete inf;
		free(_restoreBuf);
		_restoreBuf = NULL;
		return false;
	}
	delete inf;
	return true;
}

bool Control::convertSaveGame(uint8 slot, char *desc) {
	char oldFileName[15];
	char newFileName[40];
	sprintf(oldFileName, "SAVEGAME.%03d", slot);
	sprintf(newFileName, "sword1.%03d", slot);
	uint8 *saveData;
	int dataSize;

	// Check if the new file already exists
	Common::InSaveFile *testSave = _saveFileMan->openForLoading(newFileName);

	if (testSave) {
		delete testSave;

		Common::String msg = Common::String::format(_("Target new save game already exists!\n"
		                     "Would you like to keep the old save game (%s) or the new one (%s)?\n"),
		                     oldFileName, newFileName);
		GUI::MessageDialog dialog0(msg, _("Keep the old one"), _("Keep the new one"));

		int choice = dialog0.runModal();
		if (choice == GUI::kMessageCancel) {
			// User chose to keep the new game, so delete the old one
			_saveFileMan->removeSavefile(oldFileName);
			return true;
		}
	}

	Common::InSaveFile *oldSave = _saveFileMan->openForLoading(oldFileName);
	if (!oldSave) {
		// Display a warning message and do nothing
		warning("Can't open file '%s'", oldFileName);
		return false;
	}

	// Read data from old type of save game
	dataSize = oldSave->size();
	saveData = new uint8[dataSize];
	oldSave->read(saveData, dataSize);
	delete oldSave;

	// Now write the save data to a new type of save game
	Common::OutSaveFile *newSave;
	newSave = _saveFileMan->openForSaving(newFileName);
	if (!newSave) {
		// Display a warning message and do nothing
		warning("Unable to create file '%s'. (%s)", newFileName, _saveFileMan->popErrorDesc().c_str());
		delete[] saveData;
		saveData = NULL;
		return false;
	}

	newSave->writeUint32LE(SAVEGAME_HEADER);
	newSave->write(desc, 40);
	newSave->writeByte(SAVEGAME_VERSION);

	// Date / time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = (curTime.tm_mday & 0xFF) << 24 | ((curTime.tm_mon + 1) & 0xFF) << 16 | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = (curTime.tm_hour & 0xFF) << 8 | ((curTime.tm_min) & 0xFF);

	newSave->writeUint32BE(saveDate);
	newSave->writeUint16BE(saveTime);
	newSave->writeUint32BE(0); // We don't have playtime info when converting, so we start from 0.

	newSave->write(saveData, dataSize);

	newSave->finalize();
	if (newSave->err())
		warning("Couldn't write to file '%s'. Device full?", newFileName);
	delete newSave;

	// Delete old save
	_saveFileMan->removeSavefile(oldFileName);

	// Cleanup
	delete[] saveData;
	saveData = NULL;
	return true;
}

void Control::doRestore() {
	uint8 *bufPos = _restoreBuf;
	_objMan->loadLiveList((uint16 *)bufPos);
	bufPos += TOTAL_SECTIONS * 2;
	for (uint16 cnt = 0; cnt < NUM_SCRIPT_VARS; cnt++) {
		Logic::_scriptVars[cnt] = *(uint32 *)bufPos;
		bufPos += 4;
	}
	uint32 playerSize = (sizeof(Object) - 12000) / 4;
	uint32 *playerRaw = (uint32 *)_objMan->fetchObject(PLAYER);
	Object *cpt = _objMan->fetchObject(PLAYER);
	for (uint32 cnt2 = 0; cnt2 < playerSize; cnt2++) {
		*playerRaw = *(uint32 *)bufPos;
		playerRaw++;
		bufPos += 4;
	}
	free(_restoreBuf);
	Logic::_scriptVars[CHANGE_DIR] = cpt->o_dir;
	Logic::_scriptVars[CHANGE_X] = cpt->o_xcoord;
	Logic::_scriptVars[CHANGE_Y] = cpt->o_ycoord;
	Logic::_scriptVars[CHANGE_STANCE] = STAND;
	Logic::_scriptVars[CHANGE_PLACE] = cpt->o_place;
	SwordEngine::_systemVars.justRestoredGame = 1;
	if (SwordEngine::_systemVars.isDemo)
		Logic::_scriptVars[PLAYINGDEMO] = 1;
}

void Control::delay(uint32 msecs) {
	Common::Event event;

	uint32 now = _system->getMillis();
	uint32 endTime = now + msecs;
	_keyPressed.reset();
	_mouseState = 0;

	do {
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				_keyPressed = event.kbd;
				// we skip the rest of the delay and return immediately
				// to handle keyboard input
				return;
			case Common::EVENT_MOUSEMOVE:
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_LBUTTONDOWN:
				_mouseDown = true;
				_mouseState |= BS1L_BUTTON_DOWN;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_LBUTTONUP:
				_mouseDown = false;
				_mouseState |= BS1L_BUTTON_UP;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_WHEELUP:
				_mouseDown = false;
				_mouseState |= BS1_WHEEL_UP;
				_mouseCoord = event.mouse;
				break;
			case Common::EVENT_WHEELDOWN:
				_mouseDown = false;
				_mouseState |= BS1_WHEEL_DOWN;
				break;
			default:
				break;
			}
		}

		_system->updateScreen();
		_system->delayMillis(10);
	} while (_system->getMillis() < endTime);
}

const ButtonInfo Control::_deathButtons[3] = {
	{250, 224 + 40, SR_BUTTON, BUTTON_RESTORE_PANEL, 0 },
	{250, 260 + 40, SR_BUTTON, BUTTON_RESTART, kButtonOk },
	{250, 296 + 40, SR_BUTTON, BUTTON_QUIT, kButtonCancel }
};

const ButtonInfo Control::_panelButtons[7] = {
	{145, 188 + 40, SR_BUTTON, BUTTON_SAVE_PANEL, 0 },
	{145, 224 + 40, SR_BUTTON, BUTTON_RESTORE_PANEL, 0 },
	{145, 260 + 40, SR_BUTTON, BUTTON_RESTART, 0 },
	{145, 296 + 40, SR_BUTTON, BUTTON_QUIT, kButtonCancel },
	{475, 188 + 40, SR_BUTTON, BUTTON_VOLUME_PANEL, 0 },
	{475, 224 + 40, SR_TEXT_BUTTON, BUTTON_TEXT, 0 },
	{475, 332 + 40, SR_BUTTON, BUTTON_DONE, kButtonOk }
};

const ButtonInfo Control::_saveButtons[16] = {
	{114,  32 + 40, SR_SLAB1, BUTTON_SAVE_SELECT1, 0 },
	{114,  68 + 40, SR_SLAB2, BUTTON_SAVE_SELECT2, 0 },
	{114, 104 + 40, SR_SLAB3, BUTTON_SAVE_SELECT3, 0 },
	{114, 140 + 40, SR_SLAB4, BUTTON_SAVE_SELECT4, 0 },
	{114, 176 + 40, SR_SLAB1, BUTTON_SAVE_SELECT5, 0 },
	{114, 212 + 40, SR_SLAB2, BUTTON_SAVE_SELECT6, 0 },
	{114, 248 + 40, SR_SLAB3, BUTTON_SAVE_SELECT7, 0 },
	{114, 284 + 40, SR_SLAB4, BUTTON_SAVE_SELECT8, 0 },

	{516,  25 + 40, SR_BUTUF, BUTTON_SCROLL_UP_FAST, 0 },
	{516,  45 + 40, SR_BUTUS, BUTTON_SCROLL_UP_SLOW, 0 },
	{516, 289 + 40, SR_BUTDS, BUTTON_SCROLL_DOWN_SLOW, 0 },
	{516, 310 + 40, SR_BUTDF, BUTTON_SCROLL_DOWN_FAST, 0 },

	{125, 338 + 40, SR_BUTTON, BUTTON_SAVE_RESTORE_OKAY, kButtonOk},
	{462, 338 + 40, SR_BUTTON, BUTTON_SAVE_CANCEL, kButtonCancel }
};

const ButtonInfo Control::_volumeButtons[4] = {
	{ 478, 338 + 40, SR_BUTTON, BUTTON_MAIN_PANEL, kButtonOk },
	{ 138, 135, SR_VKNOB, 0, 0 },
	{ 273, 135, SR_VKNOB, 0, 0 },
	{ 404, 135, SR_VKNOB, 0, 0 },
};

const uint8 Control::_languageStrings[8 * 20][43] = {
	// BS1_ENGLISH:
	"PAUSED",
	"PLEASE INSERT CD-",
	"THEN PRESS A KEY",
	"INCORRECT CD",
	"Save",
	"Restore",
	"Restart",
	"Start",
	"Quit",
	"Speed",
	"Volume",
	"Text",
	"Done",
	"OK",
	"Cancel",
	"Music",
	"Speech",
	"Fx",
	"The End",
	"DRIVE FULL!",
// BS1_FRENCH:
	"PAUSE",
	"INS\xC9REZ LE CD-",
	"ET APPUYES SUR UNE TOUCHE",
	"CD INCORRECT",
	"Sauvegarder",
	"Recharger",
	"Recommencer",
	"Commencer",
	"Quitter",
	"Vitesse",
	"Volume",
	"Texte",
	"Termin\xE9",
	"OK",
	"Annuler",
	"Musique",
	"Voix",
	"Fx",
	"Fin",
	"DISQUE PLEIN!",
//BS1_GERMAN:
	"PAUSE",
	"BITTE LEGEN SIE CD-",
	"EIN UND DR\xDC""CKEN SIE EINE BELIEBIGE TASTE",
	"FALSCHE CD",
	"Speichern",
	"Laden",
	"Neues Spiel",
	"Start",
	"Beenden",
	"Geschwindigkeit",
	"Lautst\xE4rke",
	"Text",
	"Fertig",
	"OK",
	"Abbrechen",
	"Musik",
	"Sprache",
	"Fx",
	"Ende",
	"DRIVE FULL!",
//BS1_ITALIAN:
	"PAUSA",
	"INSERITE IL CD-",
	"E PREMETE UN TASTO",
	"CD ERRATO",
	"Salva",
	"Ripristina",
	"Ricomincia",
	"Inizio",
	"Esci",
	"Velocit\xE0",
	"Volume",
	"Testo",
	"Fatto",
	"OK",
	"Annulla",
	"Musica",
	"Parlato",
	"Fx",
	"Fine",
	"DISCO PIENO!",
//BS1_SPANISH:
	"PAUSA",
	"POR FAVOR INTRODUCE EL CD-",
	"Y PULSA UNA TECLA",
	"CD INCORRECTO",
	"Guardar",
	"Recuperar",
	"Reiniciar",
	"Empezar",
	"Abandonar",
	"Velocidad",
	"Volumen",
	"Texto",
	"Hecho",
	"OK",
	"Cancelar",
	"M\xFAsica",
	"Di\xE1logo",
	"Fx",
	"Fin",
	"DISCO LLENO",
// BS1_CZECH:
	"\xAC\x41S SE ZASTAVIL",
	"VLO\xA6TE DO MECHANIKY CD DISK",
	"PAK STISKN\xB7TE LIBOVOLNOU KL\xB5VESU",
	"TO NEBUDE TO SPR\xB5VN\x90 CD",
	"Ulo\xA7it pozici",
	"Nahr\xA0t pozici",
	"Za\x9F\xA1t znovu",
	"Start",
	"Ukon\x9Fit hru",
	"Rychlost",
	"Hlasitost",
	"Titulky",
	"Souhlas\xA1m",
	"Ano",
	"Ne",
	"Hudba",
	"Mluven, slovo",
	"Zvuky",
	"Konec",
	"Disk pln\xEC",
//BS1_PORTUGESE:
	"PAUSA",
	"FAVOR INSERIR CD",
	"E DIGITAR UMA TECLA",
	"CD INCORRETO",
	"Salvar",
	"Restaurar",
	"Reiniciar",
	"Iniciar",
	"Sair",
	"Velocidade",
	"Volume",
	"Texto",
	"Feito",
	"OK",
	"Cancelar",
	"M\xFAsica",
	"Voz",
	"Efeitos",
	"Fim",
	"UNIDADE CHEIA!",
};

} // End of namespace Sword1
