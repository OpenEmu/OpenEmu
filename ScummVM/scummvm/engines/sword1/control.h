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

#ifndef SWORD1_CONTROL_H
#define SWORD1_CONTROL_H

#include "common/scummsys.h"
#include "common/events.h"
#include "common/str-array.h"
#include "sword1/sworddefs.h"

class OSystem;
namespace Common {
class SaveFileManager;
}

namespace Sword1 {

class ObjectMan;
class ResMan;
class Mouse;
class Music;
class Sound;

#define SAVEGAME_HEADER MKTAG('B','S','_','1')
#define SAVEGAME_VERSION 2

#define MAX_BUTTONS 16

#define CONTROL_NOTHING_DONE 0
#define CONTROL_GAME_RESTORED 1
#define CONTROL_RESTART_GAME 2

class ControlButton {
public:
	ControlButton(uint16 x, uint16 y, uint32 resId, uint8 id, uint8 flag, ResMan *pResMan, uint8 *screenBuf, OSystem *system);
	~ControlButton();
	void draw();
	bool wasClicked(uint16 mouseX, uint16 mouseY);
	void setSelected(uint8 selected);
	bool isSaveslot();
	uint8 _id;
	uint8 _flag;
private:
	int _frameIdx;
	uint16 _x, _y;
	uint16 _width, _height;
	uint32 _resId;
	ResMan *_resMan;
	uint8 *_dstBuf;
	OSystem *_system;
};

enum {
	kButtonOk = 1,
	kButtonCancel = 2
};

struct ButtonInfo {
	uint16 x, y;
	uint32 resId, id;
	uint8 flag;
};

class Control {
public:
	Control(Common::SaveFileManager *saveFileMan, ResMan *pResMan, ObjectMan *pObjMan, OSystem *system, Mouse *pMouse, Sound *pSound, Music *pMusic);
	uint8 runPanel();
	void doRestore();
	void askForCd();
	bool savegamesExist();
	void readSavegameDescriptions();
	void saveGameToFile(uint8 slot);
	bool restoreGameFromFile(uint8 slot);
	void checkForOldSaveGames();
	bool isPanelShown();

	void setSaveDescription(int slot, const char *desc) {
		_saveNames[slot] = desc;
	}

private:
	int displayMessage(const char *altButton, const char *message, ...) GCC_PRINTF(3, 4);

	bool convertSaveGame(uint8 slot, char *desc);
	void showSavegameNames();
	void deselectSaveslots();
	uint8 *_restoreBuf;
	uint8 _saveFiles;
	uint8 _numSaves;
	uint8 _saveScrollPos;
	uint8 _selectedSavegame;
	Common::StringArray _saveNames;
	Common::String _oldName;
	uint8 _cursorTick;
	bool _cursorVisible;
	bool _panelShown;

	uint8 getClicks(uint8 mode, uint8 *retVal);
	uint8 handleButtonClick(uint8 id, uint8 mode, uint8 *retVal);
	void handleVolumeClicks();
	void changeVolume(uint8 id, uint8 action);

	void setupMainPanel();
	void setupSaveRestorePanel(bool saving);
	void setupVolumePanel();
	bool getConfirm(const uint8 *title);

	void saveNameScroll(uint8 scroll, bool saving);
	void saveNameSelect(uint8 id, bool saving);
	bool saveToFile();
	bool restoreFromFile();
	bool keyAccepted(uint16 ascii);
	void handleSaveKey(Common::KeyState kbd);

	void renderVolumeBar(uint8 id, uint8 volL, uint8 volR);
	uint16 getTextWidth(const uint8 *str);
	void renderText(const uint8 *str, uint16 x, uint16 y, uint8 mode);
	uint8 _numButtons;
	uint8 _selectedButton;
	void createButtons(const ButtonInfo *buttons, uint8 num);
	void destroyButtons();
	ControlButton *_buttons[MAX_BUTTONS];
	static const ButtonInfo _deathButtons[3], _panelButtons[7], _saveButtons[16], _volumeButtons[4];
	static const uint8 _languageStrings[8 * 20][43];
	const uint8(*_lStrings)[43];
	Common::SaveFileManager *_saveFileMan;
	ObjectMan *_objMan;
	ResMan *_resMan;
	OSystem *_system;
	Mouse *_mouse;
	Music *_music;
	Sound *_sound;
	uint8 *_font, *_redFont;
	uint8 *_screenBuf;
	Common::KeyState _keyPressed;
	void delay(uint32 msecs);
	Common::Point _mouseCoord;
	uint16 _mouseState;
	bool _mouseDown;
};

} // End of namespace Sword1

#endif //BSCONTROL_H
