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

#ifndef SKY_CONTROL_H
#define SKY_CONTROL_H


#include "common/events.h"
#include "common/scummsys.h"
#include "common/str-array.h"

class OSystem;
namespace Common {
class SaveFileManager;
}

namespace Sky {

class Disk;
class Screen;
class Logic;
class Mouse;
class Text;
class MusicBase;
class Sound;
class SkyCompact;
struct Compact;
struct DataFileHeader;
struct MegaSet;

#define MAX_SAVE_GAMES 999
#define MAX_TEXT_LEN 80
#define PAN_LINE_WIDTH 184
#define PAN_CHAR_HEIGHT 12
#define STATUS_WIDTH 146
#define MPNL_X 60  // Main Panel
#define MPNL_Y 10

#define SPNL_X 20  // Save Panel
#define SPNL_Y 20
#define SP_HEIGHT 149
#define SP_TOP_GAP 12
#define SP_BOT_GAP 27
#define CROSS_SZ_X 27
#define CROSS_SZ_Y 22

#define TEXT_FLAG_MASK (SF_ALLOW_SPEECH | SF_ALLOW_TEXT)

#define GAME_NAME_X (SPNL_X + 18)				// x coordinate of game names
#define GAME_NAME_Y (SPNL_Y + SP_TOP_GAP)		// start y coord of game names
#define MAX_ON_SCREEN ((SP_HEIGHT - SP_TOP_GAP - SP_BOT_GAP) / PAN_CHAR_HEIGHT) // no of save games on screen
#define CP_PANEL 60400 // main panel sprite

#define MAINPANEL 0
#define SAVEPANEL 1

#define NO_MASK false
#define WITH_MASK true

// resource's onClick routines
#define DO_NOTHING		0
#define REST_GAME_PANEL	1
#define SAVE_GAME_PANEL	2
#define SAVE_A_GAME		3
#define RESTORE_A_GAME	4
#define SP_CANCEL		5
#define SHIFT_DOWN_FAST	6
#define SHIFT_DOWN_SLOW	7
#define SHIFT_UP_FAST	8
#define SHIFT_UP_SLOW	9
#define SPEED_SLIDE		10
#define MUSIC_SLIDE		11
#define TOGGLE_FX		12
#define TOGGLE_MS		13
#define TOGGLE_TEXT		14
#define EXIT			15
#define RESTART			16
#define QUIT_TO_DOS		17
#define RESTORE_AUTO	18

// onClick return codes
#define CANCEL_PRESSED	100
#define NAME_TOO_SHORT	101
#define GAME_SAVED		102
#define SHIFTED			103
#define TOGGLED			104
#define RESTARTED		105
#define GAME_RESTORED	106
#define RESTORE_FAILED	107
#define NO_DISK_SPACE	108
#define SPEED_CHANGED	109
#define QUIT_PANEL		110

#define SLOW 0
#define FAST 1

#define SPEED_MULTIPLY	12

//-
#define SAVE_EXT	 1
#define SAVE_MEGA0	 2
#define SAVE_MEGA1	 4
#define SAVE_MEGA2	 8
#define SAVE_MEGA3	16
#define SAVE_GRAFX	32
#define SAVE_TURNP	64

#define SAVE_FILE_REVISION 6
#define OLD_SAVEGAME_TYPE 5

struct AllocedMem {
	uint16 *mem;
	AllocedMem *next;
};

class ConResource {
public:
	ConResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, uint16 pX, uint16 pY, uint32 pText, uint8 pOnClick, OSystem *system, uint8 *screen);
	virtual ~ConResource() {}
	void setSprite(void *pSpData) { _spriteData = (DataFileHeader *)pSpData; }
	void setText(uint32 pText) { if (pText) _text = pText + 0x7000; else _text = 0; }
	void setXY(uint16 x, uint16 y) { _x = x; _y = y; }
	bool isMouseOver(uint32 mouseX, uint32 mouseY);
	virtual void drawToScreen(bool doMask);

	DataFileHeader *_spriteData;
	uint32 _numSprites, _curSprite;
	uint16 _x, _y;
	uint32 _text;
	uint8 _onClick;
	OSystem *_system;
	uint8 *_screen;
private:
};

class TextResource : public ConResource {
public:
	TextResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, uint16 pX, uint16 pY, uint32 pText, uint8 pOnClick, OSystem *system, uint8 *screen);
	virtual ~TextResource();
	virtual void drawToScreen(bool doMask);
	void flushForRedraw();
private:
	uint16 _oldX, _oldY;
	uint8 *_oldScreen;
};

class ControlStatus {
public:
	ControlStatus(Text *skyText, OSystem *system, uint8 *scrBuf);
	~ControlStatus();
	void setToText(const char *newText);
	void setToText(uint16 textNum);
	void drawToScreen();
private:
	TextResource *_statusText;
	DataFileHeader *_textData;
	Text *_skyText;
	OSystem *_system;
	uint8 *_screenBuf;
};

class Control {
public:
	Control(Common::SaveFileManager *saveFileMan, Screen *screen, Disk *disk, Mouse *mouse, Text *text, MusicBase *music, Logic *logic, Sound *sound, SkyCompact *skyCompact, OSystem *system);
	void doControlPanel();
	void doLoadSavePanel();
	void restartGame();
	void showGameQuitMsg();
	void doAutoSave();
	uint16 quickXRestore(uint16 slot);
	bool loadSaveAllowed();

	uint16 _selectedGame;
	uint16 saveGameToFile(bool fromControlPanel, const char *filename = 0);

	void loadDescriptions(Common::StringArray &list);
	void saveDescriptions(const Common::StringArray &list);

private:
	int displayMessage(const char *altButton, const char *message, ...) GCC_PRINTF(3, 4);

	void initPanel();
	void removePanel();

	void drawMainPanel();

	/**
	 * Waits for a specified amount while still processing events.
	 *
	 * @param amount The duration in milliseconds to wait
	 */
	void delay(unsigned int amount);

	void animClick(ConResource *pButton);
	bool getYesNo(char *text);
	void buttonControl(ConResource *pButton);
	uint16 handleClick(ConResource *pButton);
	uint16 doMusicSlide();
	uint16 doSpeedSlide();
	void toggleFx(ConResource *pButton);
	uint16 toggleText();
	void toggleMusic(ConResource *pButton);
	uint16 shiftDown(uint8 speed);
	uint16 shiftUp(uint8 speed);
	void drawTextCross(uint32 flags);
	void drawCross(uint16 x, uint16 y);

	uint16 saveRestorePanel(bool allowSave);
	void setUpGameSprites(const Common::StringArray &saveGameNames, DataFileHeader **nameSprites, uint16 firstNum, uint16 selectedGame);
	void showSprites(DataFileHeader **nameSprites, bool allowSave);
	void handleKeyPress(Common::KeyState kbd, Common::String &textBuf);

	uint32 prepareSaveData(uint8 *destBuf);

	bool autoSaveExists();
	uint16 restoreGameFromFile(bool autoSave);
	void importOldMegaSet(uint8 **srcPos, MegaSet *mega);
	void importOldCompact(Compact* destCpt, uint8 **srcPos, uint16 numElems, uint16 type, char *name);
	uint16 parseSaveData(uint8 *srcBuf);

	Common::SaveFileManager *_saveFileMan;
	SkyCompact *_skyCompact;
	Screen *_skyScreen;
	Disk *_skyDisk;
	Mouse *_skyMouse;
	Text *_skyText;
	MusicBase *_skyMusic;
	Logic *_skyLogic;
	Sound *_skySound;
	OSystem *_system;
	bool _mouseClicked;
	Common::KeyState _keyPressed;
	int _mouseWheel;

	struct {
		uint8 *controlPanel;
		uint8 *button;
		uint8 *buttonDown;
		uint8 *savePanel;
		uint8 *yesNo;
		uint8 *slide;
		uint8 *slode;
		uint8 *slode2;
		uint8 *slide2;
		uint8 *musicBodge;
	} _sprites;

	uint8 *_screenBuf;
	int _lastButton;
	uint32 _curButtonText;
	uint16 _firstText;
	uint16 _savedMouse;
	uint32 _savedCharSet;
	uint16 _enteredTextWidth;

	ConResource *createResource(void *pSpData, uint32 pNSprites, uint32 pCurSprite, int16 pX, int16 pY, uint32 pText, uint8 pOnClick, uint8 panelType);

	DataFileHeader *_textSprite;
	TextResource *_text;

	ConResource *_controlPanel, *_exitButton, *_slide, *_slide2, *_slode;
	ConResource *_restorePanButton, *_savePanButton, *_dosPanButton, *_restartPanButton, *_fxPanButton, *_musicPanButton;
	ConResource *_bodge, *_yesNo;
	ConResource *_controlPanLookList[9];

	//- Save/restore panel
	ConResource *_savePanel;
	ConResource *_saveButton, *_downFastButton, *_downSlowButton;
	ConResource *_upFastButton, *_upSlowButton, *_quitButton, *_restoreButton;
	ConResource *_autoSaveButton;

	ConResource *_savePanLookList[6], *_restorePanLookList[7];

	ControlStatus *_statusBar;

	static char _quitTexts[16][35];
	static uint8 _crossImg[594];
};

} // End of namespace Sky

#endif // CONTROL_H
