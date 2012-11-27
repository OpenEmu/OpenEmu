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

#include "common/system.h"
#include "common/textconsole.h"

#include "parallaction/gui.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/saveload.h"

namespace Parallaction {

class SplashInputState_BR : public MenuInputState {
protected:
	Common::String _slideName;
	uint32 _timeOut;
	Common::String _nextState;
	uint32	_startTime;
	Palette blackPal;
	Palette pal;

	Parallaction *_vm;
	int _fadeSteps;

public:
	SplashInputState_BR(Parallaction *vm, const Common::String &name, MenuInputHelper *helper) : MenuInputState(name, helper), _vm(vm)  {
	}

	virtual MenuInputState* run() {
		if (_fadeSteps > 0) {
			pal.fadeTo(blackPal, 1);
			_vm->_gfx->setPalette(pal);
			_fadeSteps--;
			return this;
		}

		if (_fadeSteps == 0) {
			return _helper->getState(_nextState);
		}

		uint32 curTime = _vm->_system->getMillis();
		if (curTime - _startTime > _timeOut) {
			_fadeSteps = 64;
			pal.clone(_vm->_gfx->_backgroundInfo->palette);
		}
		return this;
	}

	virtual void enter() {
		_vm->_gfx->clearScreen();
		_vm->showSlide(_slideName.c_str(), CENTER_LABEL_HORIZONTAL, CENTER_LABEL_VERTICAL);
		_vm->_input->setMouseState(MOUSE_DISABLED);

		_startTime = _vm->_system->getMillis();
		_fadeSteps = -1;
	}
};

class SplashInputState0_BR : public SplashInputState_BR {

public:
	SplashInputState0_BR(Parallaction_br *vm, MenuInputHelper *helper) : SplashInputState_BR(vm, "intro0", helper)  {
		_slideName = "dyna";
		_timeOut = 600;
		_nextState = "intro1";
	}
};

class SplashInputState1_BR : public SplashInputState_BR {

public:
	SplashInputState1_BR(Parallaction_br *vm, MenuInputHelper *helper) : SplashInputState_BR(vm, "intro1", helper) {
		_slideName = "core";
		_timeOut = 600;
		_nextState = "mainmenu";
	}
};


struct LocationPart {
	int part;
	const char *location;
};

class MainMenuInputState_BR : public MenuInputState {
	Parallaction_br *_vm;

	#define MENUITEMS_X			250
	#define MENUITEMS_Y			200

	#define MENUITEM_WIDTH		200
	#define MENUITEM_HEIGHT		20

	Frames* renderMenuItem(const char *text) {
		// this builds a surface containing two copies of the text.
		// one is in normal color, the other is inverted.
		// the two 'frames' are used to display selected/unselected menu items

		byte *data = new byte[MENUITEM_WIDTH * MENUITEM_HEIGHT * 2];
		memset(data, 0, MENUITEM_WIDTH * MENUITEM_HEIGHT * 2);

		// build first frame to be displayed when item is not selected
		if (_vm->getPlatform() == Common::kPlatformPC) {
			_vm->_menuFont->setColor(0);
		} else {
			_vm->_menuFont->setColor(23);
		}
		byte *dst = data + 5 + 2 * MENUITEM_WIDTH;
		_vm->_menuFont->drawString(dst, MENUITEM_WIDTH, text);

		// build second frame to be displayed when item is selected
		dst = dst + MENUITEM_WIDTH * MENUITEM_HEIGHT;
		_vm->_menuFont->drawString(dst, MENUITEM_WIDTH, text);

		dst = data + MENUITEM_WIDTH * MENUITEM_HEIGHT;
		for (int i = 0; i < MENUITEM_WIDTH * MENUITEM_HEIGHT; i++) {
			*dst++ ^= 0xD;
		}

		// wrap the surface into the suitable Frames adapter
		return new Cnv(2, MENUITEM_WIDTH, MENUITEM_HEIGHT, data, true);
	}

	enum MenuOptions {
		kMenuPart0 = 0,
		kMenuPart1 = 1,
		kMenuPart2 = 2,
		kMenuPart3 = 3,
		kMenuPart4 = 4,
		kMenuLoadGame = 5,
		kMenuQuit = 6
	};

	#define NUM_MENULINES	7
	GfxObj *_lines[NUM_MENULINES];

	static const char *_menuStringsAmiga[NUM_MENULINES];
	static const char *_menuStringsPC[NUM_MENULINES];
	static const MenuOptions _optionsAmiga[NUM_MENULINES];
	static const MenuOptions _optionsPC[NUM_MENULINES];

	const char **_menuStrings;
	const MenuOptions *_options;

	static LocationPart _firstLocation[];

	int _availItems;
	int _selection;

	void cleanup() {
		_vm->_gfx->freeDialogueObjects();

		for (int i = 0; i < _availItems; i++) {
			delete _lines[i];
			_lines[i] = 0;
		}
	}

	void redrawMenu() {
		Common::Point p;
		_vm->_input->getCursorPos(p);

		if ((p.x > MENUITEMS_X) && (p.x < (MENUITEMS_X+MENUITEM_WIDTH)) && (p.y > MENUITEMS_Y)) {
			_selection = (p.y - MENUITEMS_Y) / MENUITEM_HEIGHT;

			if (!(_selection < _availItems))
				_selection = -1;
		} else
			_selection = -1;

		for (int i = 0; i < _availItems; i++) {
			_vm->_gfx->setItemFrame(i, _selection == i ? 1 : 0);
		}
	}

public:
	MainMenuInputState_BR(Parallaction_br *vm, MenuInputHelper *helper) : MenuInputState("mainmenu", helper), _vm(vm)  {
	    memset(_lines, 0, sizeof(_lines));
	}

	~MainMenuInputState_BR() {
		cleanup();
	}

	virtual MenuInputState* run() {
		int event = _vm->_input->getLastButtonEvent();
		if (!((event == kMouseLeftUp) && _selection >= 0)) {
			redrawMenu();
			return this;
		}

		int selection = _options[_selection];
		switch (selection) {
		case kMenuQuit: {
			_vm->quitGame();
			break;
		}

		case kMenuLoadGame:
			warning("loadgame not yet implemented");
			if (!_vm->_saveLoad->loadGame()) {
				return this;
			}
			break;

		default:
			_vm->_nextPart = _firstLocation[selection].part;
			_vm->scheduleLocationSwitch(_firstLocation[selection].location);

		}

		_vm->_system->showMouse(false);
		cleanup();

		return 0;
	}

	virtual void enter() {
		_vm->_gfx->clearScreen();
		int x = 0, y = 0, i = 0;
		if (_vm->getPlatform() == Common::kPlatformPC) {
			x = 20;
			y = 50;
		}
		_vm->showSlide("tbra", x, y);

		_availItems = 4;

		bool complete[3];
		_vm->_saveLoad->getGamePartProgress(complete, 3);
		for (i = 0; i < 3 && complete[i]; i++, _availItems++)
			;

		if (_vm->getPlatform() == Common::kPlatformAmiga) {
			_menuStrings = _menuStringsAmiga;
			_options = _optionsAmiga;
		} else {
			_menuStrings = _menuStringsPC;
			_options = _optionsPC;
		}

		for (i = 0; i < _availItems; i++) {
			_lines[i] = new GfxObj(0, renderMenuItem(_menuStrings[i]), "MenuItem");
			_vm->_gfx->setItem(_lines[i], MENUITEMS_X, MENUITEMS_Y + MENUITEM_HEIGHT * i, 0xFF);
		}
		_selection = -1;
		_vm->_input->setArrowCursor();
		_vm->_input->setMouseState(MOUSE_ENABLED_SHOW);
	}

};

LocationPart MainMenuInputState_BR::_firstLocation[] = {
	{ 0, "intro" },
	{ 1, "museo" },
	{ 2, "start" },
	{ 3, "bolscoi" },
	{ 4, "treno" }
};


const char *MainMenuInputState_BR::_menuStringsAmiga[NUM_MENULINES] = {
	"See the introduction",
	"Load a Saved Game",
	"Exit to WorkBench",
	"Start a new game",
	"Start PART 2",
	"Start PART 3",
	"Start PART 4"
};

const MainMenuInputState_BR::MenuOptions MainMenuInputState_BR::_optionsAmiga[NUM_MENULINES] = {
	kMenuPart0,
	kMenuLoadGame,
	kMenuQuit,
	kMenuPart1,
	kMenuPart2,
	kMenuPart3,
	kMenuPart4
};

const char *MainMenuInputState_BR::_menuStringsPC[NUM_MENULINES] = {
	"SEE INTRO",
	"NEW GAME",
	"SAVED GAME",
	"EXIT TO DOS",
	"PART 2",
	"PART 3",
	"PART 4"
};

const MainMenuInputState_BR::MenuOptions MainMenuInputState_BR::_optionsPC[NUM_MENULINES] = {
	kMenuPart0,
	kMenuPart1,
	kMenuLoadGame,
	kMenuQuit,
	kMenuPart2,
	kMenuPart3,
	kMenuPart4
};

void Parallaction_br::startGui(bool showSplash) {
	_menuHelper = new MenuInputHelper;

	new MainMenuInputState_BR(this, _menuHelper);

	if (showSplash) {
		new SplashInputState0_BR(this, _menuHelper);
		new SplashInputState1_BR(this, _menuHelper);
		_menuHelper->setState("intro0");
	} else {
		_menuHelper->setState("mainmenu");
	}

	_input->_inputMode = Input::kInputModeMenu;
}


class IngameMenuInputState_BR : public MenuInputState {
	Parallaction_br *_vm;
	GfxObj *_menuObj, *_mscMenuObj, *_sfxMenuObj;
	int _menuObjId, _mscMenuObjId, _sfxMenuObjId;

	Common::Rect _menuRect;
	int _cellW, _cellH;

	int _sfxStatus, _mscStatus;

	int frameFromStatus(int status) const {
		int frame;
		if (status == 0) {
			frame = 1;
		} else
		if (status == 1) {
			frame = 0;
		} else {
			frame = 2;
		}

		return frame;
	}

public:
	IngameMenuInputState_BR(Parallaction_br *vm, MenuInputHelper *helper) : MenuInputState("ingamemenu", helper), _vm(vm) {
		Frames *menuFrames = _vm->_disk->loadFrames("request.win");
		assert(menuFrames);
		_menuObj = new GfxObj(kGfxObjTypeMenu, menuFrames, "ingamemenu");

		Frames *mscFrames = _vm->_disk->loadFrames("onoff.win");
		assert(mscFrames);
		_mscMenuObj = new GfxObj(kGfxObjTypeMenu, mscFrames, "msc");

		Frames *sfxFrames = _vm->_disk->loadFrames("sfx.win");
		assert(sfxFrames);
		_sfxMenuObj = new GfxObj(kGfxObjTypeMenu, sfxFrames, "sfx");

		_menuObj->getRect(0, _menuRect);
		_cellW = _menuRect.width() / 3;
		_cellH = _menuRect.height() / 2;
	}

	~IngameMenuInputState_BR() {
		delete _menuObj;
		delete _mscMenuObj;
		delete _sfxMenuObj;
	}

	MenuInputState *run() {
		if (_vm->_input->getLastButtonEvent() != kMouseLeftUp) {
			return this;
		}

		int cell = -1;

		Common::Point p;
		_vm->_input->getCursorPos(p);
		if (_menuRect.contains(p)) {
			cell = (p.x - _menuRect.left) / _cellW + 3 * ((p.y - _menuRect.top) / _cellH);
		}

		bool close = false;

		switch (cell) {
		case 4:	// resume
		case -1: // invalid cell
			close = true;
			break;

		case 0:	// toggle music
			if (_mscStatus != -1) {
				_vm->enableMusic(!_mscStatus);
				_mscStatus = _vm->getMusicStatus();
				_vm->_gfx->setItemFrame(_mscMenuObjId, frameFromStatus(_mscStatus));
			}
			break;

		case 1:	// toggle sfx
			if (_sfxStatus != -1) {
				_vm->enableSfx(!_sfxStatus);
				_sfxStatus = _vm->getSfxStatus();
				_vm->_gfx->setItemFrame(_sfxMenuObjId, frameFromStatus(_sfxStatus));
			}
			break;

		case 2:	// save
			warning("Saving is not supported yet");
			_vm->_saveLoad->saveGame();
			break;

		case 3:	// load
			warning("Loading is not supported yet");
			close = _vm->_saveLoad->loadGame();
			break;

		case 5:	// quit
			return _helper->getState("quitdialog");
		}

		if (close) {
			_vm->_gfx->freeDialogueObjects();
			return 0;
		}

		_vm->_input->setArrowCursor();
		return this;
	}

	void enter() {
		// TODO: find the right position of the menu object
		_menuObjId = _vm->_gfx->setItem(_menuObj, 0, 0, 0);
		_vm->_gfx->setItemFrame(_menuObjId, 0);

		_mscMenuObjId = _vm->_gfx->setItem(_mscMenuObj, 0, 0, 0);
		_mscStatus = _vm->getMusicStatus();
		_vm->_gfx->setItemFrame(_mscMenuObjId, frameFromStatus(_mscStatus));

		_sfxMenuObjId = _vm->_gfx->setItem(_sfxMenuObj, 0, 0, 0);
		_sfxStatus = _vm->getSfxStatus();
		_vm->_gfx->setItemFrame(_sfxMenuObjId, frameFromStatus(_sfxStatus));
	}
};

class QuitDialogInputState_BR : public MenuInputState {
	Parallaction_br *_vm;
	Font *_font;
	int _x, _y;
	GfxObj *_obj;

public:
	QuitDialogInputState_BR(Parallaction_br *vm, MenuInputHelper *helper) : MenuInputState("quitdialog", helper), _vm(vm) {
		_font = _vm->_dialogueFont;

		const char *question = "Do you really want to quit ?";
		const char *option = "Yes No";

		int questionW = _font->getStringWidth(question);
		int optionW = _font->getStringWidth(option);
		int w = MAX(questionW, optionW) + 30;

		_x = (640 - w) / 2;
		_y = 90;

		Graphics::Surface *surf = new Graphics::Surface;
		surf->create(w, 110, Graphics::PixelFormat::createFormatCLUT8());
		surf->fillRect(Common::Rect(0, 0, w, 110), 12);
		surf->fillRect(Common::Rect(10, 10, w-10, 100), 15);

		_font->setColor(0);
		int x = (w - questionW)/2;
		int y = 13;
		_font->drawString((byte *)surf->getBasePtr(x, y), surf->pitch, question);
		x = (w - optionW)/2;
		y = 13 + _font->height()*2;
		_font->drawString((byte *)surf->getBasePtr(x,y), surf->pitch, option);

		_obj = new GfxObj(kGfxObjTypeMenu, new SurfaceToFrames(surf), "quitdialog");
		assert(_obj);
	}

	~QuitDialogInputState_BR() {
		delete _obj;
	}

	MenuInputState *run() {
		uint16 key;
		bool e = _vm->_input->getLastKeyDown(key);
		if (!e) {
			return this;
		}

		if (key == 'y' || key == 'Y') {
			_vm->quitGame();
			return 0;
		} else
		if (key == 'n' || key == 'N') {
			// NOTE: when the quit dialog is hidden, the in-game menu is
			// deleted for a frame, and then redrawn. This is because the
			// current implementation of graphic 'items' doesn't allow
			// deletion of a single 'item'.
			_vm->_gfx->freeDialogueObjects();
			return _helper->getState("ingamemenu");
		}

		return this;
	}


	void enter() {
	//	setPaletteEntry(1, 0, 0, 0);	// text color
	//	setPaletteEntry(15, 255, 255, 255);	// background color
		int id = _vm->_gfx->setItem(_obj, _x, _y, 0);
		_vm->_gfx->setItemFrame(id, 0);
	}
};


void Parallaction_br::startIngameMenu() {
	_menuHelper = new MenuInputHelper;

	new IngameMenuInputState_BR(this, _menuHelper);
	new QuitDialogInputState_BR(this, _menuHelper);

	_menuHelper->setState("ingamemenu");

	_input->_inputMode = Input::kInputModeMenu;
}



} // namespace Parallaction
