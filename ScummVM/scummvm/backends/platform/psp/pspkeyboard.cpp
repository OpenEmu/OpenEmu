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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

//#define PSP_KB_SHELL	/* Need a hack to properly load the keyboard from the PSP shell */

#ifdef PSP_KB_SHELL
#define PSP_KB_SHELL_PATH 	"ms0:/psp/game5xx/scummvm-solid/"	/* path to kbd.zip */
#endif


#include <malloc.h>
#include <pspkernel.h>

#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/pspkeyboard.h"
#include "backends/platform/psp/png_loader.h"
#include "backends/platform/psp/input.h"
#include "common/keyboard.h"
#include "common/fs.h"
#include "common/unzip.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging the stack */
//#define __PSP_DEBUG_PRINT__

#include "backends/platform/psp/trace.h"

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define K(x)	((short)(Common::KEYCODE_INVALID + (x)))
#define C(x)	((short)(Common::KEYCODE_##x))

// Layout of the keyboard: Order for the boxes is clockwise and then middle:
//			1
//		4	5	2
//			3
// and order of letters is clockwise in each box, plus 2 top letters:
//		e		f
//			a
//		d		b
//			c
// K(x) is used for ascii values. C(x) is used for keys without ascii values
short PSPKeyboard::_modeChar[MODE_COUNT][5][6] = {
	{	//standard letters
		{ K('a'),  K('b'), K('c'), K('d'), K('f'), K('g') },
		{ K('h'),  K('i'), K('l'), K('m'), K('j'), K('k') },
		{ K('o'),  K('n'), K('r'), K('s'), K('p'), K('q') },
		{ K('u'),  K('v'), K('w'), K('y'), K('x'), K('z') },
		{ K('\b'), K('t'), K(' '), K('e'), K(0),   K(0)   }
	},
	{	//capital letters
		{ K('A'),  K('B'), K('C'), K('D'), K('F'), K('G') },
		{ K('H'),  K('I'), K('L'), K('M'), K('J'), K('K') },
		{ K('O'),  K('N'), K('R'), K('S'), K('P'), K('Q') },
		{ K('U'),  K('V'), K('W'), K('Y'), K('X'), K('Z') },
		{ K('\b'), K('T'), K(' '), K('E'), K(0),   K(0)   }
	},
	{	//numbers
		{ K('1'),  K('2'), K('3'), K('4'), K(0),   K(0)   },
		{ C(F5),   C(F6),  C(F7),  C(F8),  C(F9),  C(F10) },
		{ K('5'),  K('6'), K('7'), K('8'), K(0),   K(0)   },
		{ C(F1),   C(F2),  C(F3),  C(F4),  K(0),   K(0)   },
		{ K('\b'), K('0'), K(' '), K('9'), K(0),   K(0)   }
	},
	{	//symbols
		{ K('!'),  K(')'), K('?'), K('('), K('<'), K('>') },
		{ K('+'),  K('/'), K('='), K('\\'), K('\''), K('"') },
		{ K(':'),  K(']'), K(';'), K('['), K('@'), K('#') },
		{ K('-'),  K('}'), K('_'), K('{'), K('*'), K('$') },
		{ K('\b'), K('.'), K(' '), K(','), K(0),   K(0)   }
	}
};

// Array with file names
const char *PSPKeyboard::_guiStrings[] = {
	"keys4.png", "keys_s4.png",
	"keys_c4.png", "keys_s_c4.png",
	"nums4.png", "nums_s4.png",
	"syms4.png", "syms_s4.png"
};

// Constructor
PSPKeyboard::PSPKeyboard() {
	DEBUG_ENTER_FUNC();

	_init = false;			// we're not initialized yet
	_prevButtons = 0;		// Reset previous buttons
	_dirty = false;        	// keyboard needs redrawing
	_mode = 0;              // charset selected. (0: letters, 1: uppercase 2: numbers 3: symbols)
	_oldCursor = kCenter;	// Center cursor by default
	_movedX = 20;			// Default starting location
	_movedY = 50;
	_moved = false;			// Keyboard wasn't moved recently
	_state = kInvisible;	// We start invisible
	_lastState = kInvisible;

	// Constant renderer settings
	_renderer.setAlphaBlending(true);
	_renderer.setColorTest(false);
	_renderer.setUseGlobalScaler(false);
}

// Destructor
PSPKeyboard::~PSPKeyboard() {
	DEBUG_ENTER_FUNC();

	if (!_init) {
		return;
	}

	for (int i = 0; i < guiStringsSize; i++) {
		_buffers[i].deallocate();
		_palettes[i].deallocate();
	}
	_init = false;
}

void PSPKeyboard::setVisible(bool val) {
	if (val && _state == kInvisible && _init) {	// Check also that were loaded correctly
		_lastState = _state;
		_state = kMove;
	} else if (!val && _state != kInvisible) {
		_lastState = _state;
		_state = kInvisible;
	}
	setDirty();
}

/* move the position the keyboard is currently drawn at */
void PSPKeyboard::moveTo(const int newX, const int newY) {
	DEBUG_ENTER_FUNC();

	_movedX = newX;
	_movedY = newY;
	setDirty();
}

/* move the position the keyboard is currently drawn at */
void PSPKeyboard::increaseKeyboardLocationX(int amount) {
	DEBUG_ENTER_FUNC();

	int newX = _movedX + amount;

	if (newX > PSP_SCREEN_WIDTH - 5 || newX < 0 - 140) {	// clamp
		return;
	}
	_movedX = newX;
	setDirty();
}

/* move the position the keyboard is currently drawn at */
void PSPKeyboard::increaseKeyboardLocationY(int amount) {
	DEBUG_ENTER_FUNC();

	int newY = _movedY + amount;

	if (newY > PSP_SCREEN_HEIGHT - 5 || newY < 0 - 140)	{ // clamp
		return;
	}
	_movedY = newY;
	setDirty();
}

/* draw the keyboard at the current position */
void PSPKeyboard::render() {
	DEBUG_ENTER_FUNC();

	unsigned int currentBuffer = _mode << 1;

	// Draw the background letters
	// Set renderer to current buffer & palette
	_renderer.setBuffer(&_buffers[currentBuffer]);
	_renderer.setPalette(&_palettes[currentBuffer]);
	_renderer.setOffsetOnScreen(_movedX, _movedY);
	_renderer.setOffsetInBuffer(0, 0);
	_renderer.setDrawWholeBuffer();
	_renderer.render();

	// Get X and Y coordinates for the orange block
	int x, y;
	convertCursorToXY(_oldCursor, x, y);

	const int OrangeBlockSize = 64;
	const int GrayBlockSize = 43;

	// Draw the current Highlighted Selector (orange block)
	_renderer.setBuffer(&_buffers[currentBuffer + 1]);
	_renderer.setPalette(&_palettes[currentBuffer + 1]);
	_renderer.setOffsetOnScreen(_movedX + (x * GrayBlockSize), _movedY + (y * GrayBlockSize));
	_renderer.setOffsetInBuffer(x * OrangeBlockSize, y * OrangeBlockSize);
	_renderer.setDrawSize(OrangeBlockSize, OrangeBlockSize);
	_renderer.render();
}

inline void PSPKeyboard::convertCursorToXY(CursorDirections cur, int &x, int &y) {
	switch (cur) {
	case kUp:
		x = 1;
		y = 0;
		break;
	case kRight:
		x = 2;
		y = 1;
		break;
	case kDown:
		x = 1;
		y = 2;
		break;
	case kLeft:
		x = 0;
		y = 1;
		break;
	default:
		x = 1;
		y = 1;
		break;
	}
}

/* load the keyboard into memory */
bool PSPKeyboard::load() {
	DEBUG_ENTER_FUNC();

	if (_init) {
		PSP_DEBUG_PRINT("keyboard already loaded into memory\n");
		return true;
	}

	// For the shell, we must use a hack
#ifdef PSP_KB_SHELL
	Common::FSNode node(PSP_KB_SHELL_PATH);
#else /* normal mode */
	Common::FSNode node(".");				// Look in current directory
#endif
	PSP_DEBUG_PRINT("path[%s]\n", node.getPath().c_str());

	Common::Archive *fileArchive = NULL;
	Common::Archive *zipArchive = NULL;
	Common::SeekableReadStream * file = 0;

	if (node.getChild("kbd").exists() && node.getChild("kbd").isDirectory()) {
		PSP_DEBUG_PRINT("found directory ./kbd\n");
		fileArchive = new Common::FSDirectory(node.getChild("kbd"));
	}
	if (node.getChild("kbd.zip").exists()) {
		PSP_DEBUG_PRINT("found kbd.zip\n");
		zipArchive  = Common::makeZipArchive(node.getChild("kbd.zip"));
	}

	int i;

	// Loop through all png images
	for (i = 0; i < guiStringsSize; i++) {
		PSP_DEBUG_PRINT("Opening %s.\n", _guiStrings[i]);

		// Look for the file in the kbd directory
		if (fileArchive && fileArchive->hasFile(_guiStrings[i])) {
			PSP_DEBUG_PRINT("found it in kbd directory.\n");

			file = fileArchive->createReadStreamForMember(_guiStrings[i]);
			if (!file) {
				PSP_ERROR("Can't open kbd/%s for keyboard. No keyboard will load.\n", _guiStrings[i]);
				goto ERROR;
			}
		}
		// We didn't find it. Look for it in the zip file
		else if (zipArchive && zipArchive->hasFile(_guiStrings[i])) {
			PSP_DEBUG_PRINT("found it in kbd.zip.\n");

			file = zipArchive->createReadStreamForMember(_guiStrings[i]);
			if (!file) {
				PSP_ERROR("Can't open %s in kbd.zip for keyboard. No keyboard will load.\n", _guiStrings[i]);
				goto ERROR;
			}
		} else {	// Couldn't find the file
			PSP_ERROR("Can't find %s for keyboard. No keyboard will load.\n", _guiStrings[i]);
			goto ERROR;
		}

		PngLoader image(*file, _buffers[i], _palettes[i]);

		if (image.allocate() != PngLoader::OK) {
			PSP_ERROR("Failed to allocate memory for keyboard image %s\n", _guiStrings[i]);
			goto ERROR;
		}
		if (!image.load()) {
			PSP_ERROR("Failed to load image from file %s\n", _guiStrings[i]);
			goto ERROR;
		}

		delete file;
	} /* for loop */

	_init = true;

	delete fileArchive;
	delete zipArchive;

	return true;

ERROR:

	delete file;
	delete fileArchive;
	delete zipArchive;

	for (int j = 0; j < i; j++) {
		_buffers[j].deallocate();
		_palettes[j].deallocate();
	}
	_init = false;

	return false;
}

// Defines for working with PSP buttons
#define CHANGED(x)	 (_buttonsChanged & (x))
#define PRESSED(x)   ((_buttonsChanged & (x)) && (pad.Buttons & (x)))
#define UNPRESSED(x) ((_buttonsChanged & (x)) && !(pad.Buttons & (x)))
#define DOWN(x)		 (pad.Buttons & (x))
#define UP(x)		 (!(pad.Buttons & (x)))
#define PSP_DPAD	 (PSP_CTRL_DOWN|PSP_CTRL_UP|PSP_CTRL_LEFT|PSP_CTRL_RIGHT)
#define PSP_4BUTTONS (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_TRIANGLE | PSP_CTRL_SQUARE)

/*
 *  Attempts to read a character from the controller
 *  Uses the state machine.
 *  returns whether we have an event
 */
bool PSPKeyboard::processInput(Common::Event &event, PspEvent &pspEvent, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();

	bool haveEvent = false;		// Whether we have an event for the event manager to process
	bool havePspEvent = false;
	event.kbd.flags = 0;

	_buttonsChanged = _prevButtons ^ pad.Buttons;

	if (!_init)					// In case we never had init
		return false;
	if (_state == kInvisible)	// Return if we're invisible
		return false;
	if (_state != kMove && PRESSED(PSP_CTRL_SELECT)) {
		_lastState = _state;
		_state = kMove;			// Check for move or visible state
	} else if (CHANGED(PSP_CTRL_START)) {		// Handle start button: enter, make KB invisible
		event.kbd.ascii = '\r';
		event.kbd.keycode = Common::KEYCODE_RETURN;
		event.type = DOWN(PSP_CTRL_START) ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
		haveEvent = true;
		_dirty = true;
		if (UP(PSP_CTRL_START))
			havePspEvent = true;
	}
	// Check for being in state of moving the keyboard onscreen or pressing select
	else if (_state == kMove)
		havePspEvent = handleMoveState(pad);
	else if (_state == kDefault)
		haveEvent = handleDefaultState(event, pad);
	else if (_state == kCornersSelected)
		haveEvent = handleCornersSelectedState(event, pad);
	else if (_state == kRTriggerDown)
		handleRTriggerDownState(pad);	// Deal with trigger states
	else if (_state == kLTriggerDown)
		handleLTriggerDownState(pad);	// Deal with trigger states

	if (havePspEvent) {
		pspEvent.type = PSP_EVENT_SHOW_VIRTUAL_KB;	// tell the input handler we're off
		pspEvent.data = false;
	}
	_prevButtons = pad.Buttons;

	return haveEvent;
}

bool PSPKeyboard::handleMoveState(SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();
	if (UP(PSP_CTRL_SELECT)) {
		// Toggle between visible and invisible
		_state = (_lastState == kInvisible) ? kDefault : kInvisible;
		_dirty = true;

		if (_moved) {					// We moved the keyboard. Keep the keyboard onscreen anyway
			_state = kDefault;
			_moved = false;				// reset moved flag
		}
		if (_state == kInvisible) {
			return true;				// we become invisible
		}
	} else if (DOWN(PSP_DPAD)) {		// How we move the KB onscreen
		_moved = true;
		_dirty = true;

		if (DOWN(PSP_CTRL_DOWN))
			increaseKeyboardLocationY(5);
		else if (DOWN(PSP_CTRL_UP))
			increaseKeyboardLocationY(-5);
		else if (DOWN(PSP_CTRL_LEFT))
			increaseKeyboardLocationX(-5);
		else  /* DOWN(PSP_CTRL_RIGHT) */
			increaseKeyboardLocationX(5);
	}
	return false;
}

bool PSPKeyboard::handleDefaultState(Common::Event &event, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();
	bool haveEvent = false;

	if (PRESSED(PSP_CTRL_LTRIGGER)) 			// Don't say we used up the input
		_state = kLTriggerDown;
	else if (PRESSED(PSP_CTRL_RTRIGGER)) 	// Don't say we used up the input
		_state = kRTriggerDown;
	else if (CHANGED(PSP_4BUTTONS))			// We only care about the 4 buttons
		haveEvent = getInputChoice(event, pad);
	else if (!DOWN(PSP_4BUTTONS))				// Must be up to move cursor
		getCursorMovement(pad);

	return haveEvent;
}

bool PSPKeyboard::handleCornersSelectedState(Common::Event &event, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();
	// We care about 4 buttons + triggers (for letter selection)
	bool haveEvent = false;

	if (CHANGED(PSP_4BUTTONS | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER))
		haveEvent = getInputChoice(event, pad);
	if (!DOWN(PSP_4BUTTONS | PSP_CTRL_RTRIGGER | PSP_CTRL_LTRIGGER)) // Must be up to move cursor
		getCursorMovement(pad);

	return haveEvent;
}

bool PSPKeyboard::getInputChoice(Common::Event &event, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();
	int innerChoice;
	bool haveEvent = false;

	if (UNPRESSED(PSP_CTRL_TRIANGLE)) {
		innerChoice = 0;
		event.type = Common::EVENT_KEYUP;			// We give priority to key_up
	} else if (UNPRESSED(PSP_CTRL_CIRCLE)) {
		innerChoice = 1;
		event.type = Common::EVENT_KEYUP;			// We give priority to key_up
	} else if (UNPRESSED(PSP_CTRL_CROSS)) {
		innerChoice = 2;
		event.type = Common::EVENT_KEYUP;			// We give priority to key_up
	} else if (UNPRESSED(PSP_CTRL_SQUARE)) {
		innerChoice = 3;
		event.type = Common::EVENT_KEYUP;			// We give priority to key_up
	} else if (UNPRESSED(PSP_CTRL_LTRIGGER) && _state == kCornersSelected) {
		innerChoice = 4;
		event.type = Common::EVENT_KEYUP;			// We give priority to key_up
	} else if (UNPRESSED(PSP_CTRL_RTRIGGER) && _state == kCornersSelected) {
		innerChoice = 5;
		event.type = Common::EVENT_KEYUP;			// We give priority to key_up
	} else if (PRESSED(PSP_CTRL_TRIANGLE)) {
		innerChoice = 0;
		event.type = Common::EVENT_KEYDOWN;
	} else if (PRESSED(PSP_CTRL_CIRCLE)) {
		innerChoice = 1;
		event.type = Common::EVENT_KEYDOWN;
	} else if (PRESSED(PSP_CTRL_CROSS)) {
		innerChoice = 2;
		event.type = Common::EVENT_KEYDOWN;
	} else if (PRESSED(PSP_CTRL_SQUARE)) {
		innerChoice = 3;
		event.type = Common::EVENT_KEYDOWN;
	} else if (PRESSED(PSP_CTRL_LTRIGGER) && _state == kCornersSelected) {
		innerChoice = 4;
		event.type = Common::EVENT_KEYDOWN;			// We give priority to key_up
	} else { /* (PRESSED(PSP_CTRL_RTRIGGER)) && _state == kCornersSelected */
		innerChoice = 5;
		event.type = Common::EVENT_KEYDOWN;			// We give priority to key_up
	}

#define IS_UPPERCASE(x)	((x) >= (unsigned short)'A' && (x) <= (unsigned short)'Z')
#define TO_LOWER(x)		((x) += 'a'-'A')

	//Now grab the value out of the array
	short choice = _modeChar[_mode][_oldCursor][innerChoice];

	event.kbd.ascii = choice <= 255 ? choice : 0;

	// Handle upper-case which is missing in Common::KeyCode
	if (IS_UPPERCASE(choice)) {
		event.kbd.keycode = (Common::KeyCode) TO_LOWER(choice);
		event.kbd.flags = Common::KBD_SHIFT;
	} else
		event.kbd.keycode = (Common::KeyCode) choice;

	haveEvent = (choice != Common::KEYCODE_INVALID) ? true : false;	// We have an event/don't if it's invalid

	return haveEvent;
}

void PSPKeyboard::getCursorMovement(SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();
	CursorDirections cursor;

	// Find where the cursor is pointing
	cursor = kCenter;
	_state = kDefault;

	if (DOWN(PSP_DPAD)) {
		_state = kCornersSelected;

		if (DOWN(PSP_CTRL_UP))
			cursor = kUp;
		else if (DOWN(PSP_CTRL_RIGHT))
			cursor = kRight;
		else if (DOWN(PSP_CTRL_DOWN))
			cursor = kDown;
		else if (DOWN(PSP_CTRL_LEFT))
			cursor = kLeft;
	}

	if (cursor != _oldCursor) { //If we've moved, update dirty and return
		_dirty = true;
		_oldCursor = cursor;
	}
}

void PSPKeyboard::handleLTriggerDownState(SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();
	if (UNPRESSED(PSP_CTRL_LTRIGGER)) {
		_dirty = true;

		if (_mode < 2)
			_mode = 2;
		else
			_mode = (_mode == 2) ? 3 : 2;

		_state = kDefault;
	}
}

void PSPKeyboard::handleRTriggerDownState(SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();
	if (UNPRESSED(PSP_CTRL_RTRIGGER)) {
		_dirty = true;

		if (_mode > 1)
			_mode = 0;
		else
			_mode = (_mode == 0) ? 1 : 0;

		_state = kDefault;
	}
}
