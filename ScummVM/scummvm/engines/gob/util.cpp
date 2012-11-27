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

#include "common/stream.h"

#include "graphics/palette.h"

#include "gob/gob.h"
#include "gob/util.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

Util::Util(GobEngine *vm) : _vm(vm) {
	_mouseButtons   = kMouseButtonsNone,
	_keyBufferHead  = 0;
	_keyBufferTail  = 0;
	_fastMode       = 0;
	_frameRate      = 12;
	_frameWaitTime  = 0;
	_startFrameTime = 0;

	_keyState = 0;
}

uint32 Util::getTimeKey() {
	return g_system->getMillis() * _vm->_global->_speedFactor;
}

int16 Util::getRandom(int16 max) {
	if (max == 0)
		return 0;

	return _vm->_rnd.getRandomNumber(max - 1);
}

void Util::beep(int16 freq) {
	if (_vm->_global->_soundFlags == 0)
		return;

	_vm->_sound->speakerOn(freq, 50);
}

void Util::notifyPaused(uint32 duration) {
	_startFrameTime += duration;
}

void Util::delay(uint16 msecs) {
	g_system->delayMillis(msecs / _vm->_global->_speedFactor);
}

void Util::longDelay(uint16 msecs) {
	uint32 time = g_system->getMillis() * _vm->_global->_speedFactor + msecs;
	do {
		_vm->_video->waitRetrace();
		processInput();
		delay(15);
	} while (!_vm->shouldQuit() &&
	         ((g_system->getMillis() * _vm->_global->_speedFactor) < time));
}

void Util::initInput() {
	_mouseButtons  = kMouseButtonsNone;
	_keyBufferHead = _keyBufferTail = 0;
}

void Util::processInput(bool scroll) {
	Common::Event event;
	Common::EventManager *eventMan = g_system->getEventManager();
	int16 x = 0, y = 0;
	bool hasMove = false;

	_vm->_vidPlayer->updateLive();

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			hasMove = true;
			x = event.mouse.x;
			y = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseButtons = (MouseButtons) (((uint32) _mouseButtons) | ((uint32) kMouseButtonsLeft));
			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons = (MouseButtons) (((uint32) _mouseButtons) | ((uint32) kMouseButtonsRight));
			break;
		case Common::EVENT_LBUTTONUP:
			_mouseButtons = (MouseButtons) (((uint32) _mouseButtons) & ~((uint32) kMouseButtonsLeft));
			break;
		case Common::EVENT_RBUTTONUP:
			_mouseButtons = (MouseButtons) (((uint32) _mouseButtons) & ~((uint32) kMouseButtonsRight));
			break;
		case Common::EVENT_KEYDOWN:
			keyDown(event);

			if (event.kbd.hasFlags(Common::KBD_CTRL)) {
				if (event.kbd.keycode == Common::KEYCODE_f)
					_fastMode ^= 1;
				else if (event.kbd.keycode == Common::KEYCODE_g)
					_fastMode ^= 2;
				else if (event.kbd.keycode == Common::KEYCODE_p)
					_vm->pauseGame();
				else if (event.kbd.keycode == Common::KEYCODE_d) {
					_vm->getDebugger()->attach();
					_vm->getDebugger()->onFrame();
				}
				break;
			}
			addKeyToBuffer(event.kbd);
			break;
		case Common::EVENT_KEYUP:
			keyUp(event);
			break;
		default:
			break;
		}
	}

	_vm->_global->_speedFactor = MIN(_fastMode + 1, 3);
	if (hasMove && scroll) {
		x = CLIP(x, _vm->_global->_mouseMinX, _vm->_global->_mouseMaxX);
		y = CLIP(y, _vm->_global->_mouseMinY, _vm->_global->_mouseMaxY);

		x -= _vm->_video->_screenDeltaX;
		y -= _vm->_video->_screenDeltaY;

		_vm->_util->setMousePos(x, y);
		_vm->_game->wantScroll(x, y);

		// WORKAROUND:
		// Force a check of the mouse in order to fix the sofa bug. This apply only for Gob3, and only
		// in the impacted TOT file so that the second screen animation is not broken.
		if ((_vm->getGameType() == kGameTypeGob3) && _vm->isCurrentTot("EMAP1008.TOT"))
			_vm->_game->evaluateScroll();
	}
}

void Util::clearKeyBuf() {
	processInput();
	_keyBufferHead = _keyBufferTail = 0;
}

bool Util::keyBufferEmpty() {
	return (_keyBufferHead == _keyBufferTail);
}

void Util::addKeyToBuffer(const Common::KeyState &key) {
	if ((_keyBufferHead + 1) % KEYBUFSIZE == _keyBufferTail) {
		warning("key buffer overflow");
		return;
	}

	_keyBuffer[_keyBufferHead] = key;
	_keyBufferHead = (_keyBufferHead + 1) % KEYBUFSIZE;
}

bool Util::getKeyFromBuffer(Common::KeyState &key) {
	if (_keyBufferHead == _keyBufferTail) return false;

	key = _keyBuffer[_keyBufferTail];
	_keyBufferTail = (_keyBufferTail + 1) % KEYBUFSIZE;

	return true;
}

static const uint16 kLatin1ToCP850[] = {
	0xFF, 0xAD, 0xBD, 0x9C, 0xCF, 0xBE, 0xDD, 0xF5, 0xF9, 0xB8, 0xA6, 0xAE, 0xAA, 0xF0, 0xA9, 0xEE,
	0xF8, 0xF1, 0xFD, 0xFC, 0xEF, 0xE6, 0xF4, 0xFA, 0xF7, 0xFB, 0xA7, 0xAF, 0xAC, 0xAB, 0xF3, 0xA8,
	0xB7, 0xB5, 0xB6, 0xC7, 0x8E, 0x8F, 0x92, 0x80, 0xD4, 0x90, 0xD2, 0xD3, 0xDE, 0xD6, 0xD7, 0xD8,
	0xD1, 0xA5, 0xE3, 0xE0, 0xE2, 0xE5, 0x99, 0x9E, 0x9D, 0xEB, 0xE9, 0xEA, 0x9A, 0xED, 0xE8, 0xE1,
	0x85, 0xA0, 0x83, 0xC6, 0x84, 0x86, 0x91, 0x87, 0x8A, 0x82, 0x88, 0x89, 0x8D, 0xA1, 0x8C, 0x8B,
	0xD0, 0xA4, 0x95, 0xA2, 0x93, 0xE4, 0x94, 0xF6, 0x9B, 0x97, 0xA3, 0x96, 0x81, 0xEC, 0xE7, 0x98
};

int16 Util::toCP850(uint16 latin1) {
	if ((latin1 < 0xA0) || ((latin1 - 0xA0) >= ARRAYSIZE(kLatin1ToCP850)))
		return 0;

	return kLatin1ToCP850[latin1 - 0xA0];
}

int16 Util::translateKey(const Common::KeyState &key) {
	static struct keyS {
		int16 from;
		int16 to;
	} keys[] = {
		{Common::KEYCODE_BACKSPACE, kKeyBackspace},
		{Common::KEYCODE_SPACE,     kKeySpace    },
		{Common::KEYCODE_RETURN,    kKeyReturn   },
		{Common::KEYCODE_ESCAPE,    kKeyEscape   },
		{Common::KEYCODE_DELETE,    kKeyDelete   },
		{Common::KEYCODE_UP,        kKeyUp       },
		{Common::KEYCODE_DOWN,      kKeyDown     },
		{Common::KEYCODE_RIGHT,     kKeyRight    },
		{Common::KEYCODE_LEFT,      kKeyLeft     },
		{Common::KEYCODE_F1,        kKeyF1       },
		{Common::KEYCODE_F2,        kKeyF2       },
		{Common::KEYCODE_F3,        kKeyF3       },
		{Common::KEYCODE_F4,        kKeyF4       },
		{Common::KEYCODE_F5,        kKeyEscape   },
		{Common::KEYCODE_F6,        kKeyF6       },
		{Common::KEYCODE_F7,        kKeyF7       },
		{Common::KEYCODE_F8,        kKeyF8       },
		{Common::KEYCODE_F9,        kKeyF9       },
		{Common::KEYCODE_F10,       kKeyF10      }
	};

	// Translate special keys
	for (int i = 0; i < ARRAYSIZE(keys); i++)
		if (key.keycode == keys[i].from)
			return keys[i].to;

	// Return the ascii value, for text input
	if ((key.ascii >= 32) && (key.ascii <= 127))
		return key.ascii;

	// Translate international characters into CP850 characters
	if ((key.ascii >= 160) && (key.ascii <= 255))
		return toCP850(key.ascii);

	return 0;
}

static const uint8 kLowerToUpper[][2] = {
	{0x81, 0x9A},
	{0x82, 0x90},
	{0x83, 0xB6},
	{0x84, 0x8E},
	{0x85, 0xB7},
	{0x86, 0x8F},
	{0x87, 0x80},
	{0x88, 0xD2},
	{0x89, 0xD3},
	{0x8A, 0xD4},
	{0x8B, 0xD8},
	{0x8C, 0xD7},
	{0x8D, 0xDE},
	{0x91, 0x92},
	{0x93, 0xE2},
	{0x94, 0x99},
	{0x95, 0xE3},
	{0x96, 0xEA},
	{0x97, 0xEB},
	{0x95, 0xE3},
	{0x96, 0xEA},
	{0x97, 0xEB},
	{0x9B, 0x9D},
	{0xA0, 0xB5},
	{0xA1, 0xD6},
	{0xA2, 0xE0},
	{0xA3, 0xE9},
	{0xA4, 0xA5},
	{0xC6, 0xC7},
	{0xD0, 0xD1},
	{0xE4, 0xE5},
	{0xE7, 0xE8},
	{0xEC, 0xED}
};

char Util::toCP850Lower(char cp850) {
	const uint8 cp = (unsigned char)cp850;
	if (cp <= 32)
		return cp850;

	if (cp <= 127)
		return tolower(cp850);

	for (uint i = 0; i < ARRAYSIZE(kLowerToUpper); i++)
		if (cp == kLowerToUpper[i][1])
			return (char)kLowerToUpper[i][0];

	return cp850;
}

char Util::toCP850Upper(char cp850) {
	const uint8 cp = (unsigned char)cp850;
	if (cp <= 32)
		return cp850;

	if (cp <= 127)
		return toupper(cp850);

	for (uint i = 0; i < ARRAYSIZE(kLowerToUpper); i++)
		if (cp == kLowerToUpper[i][0])
			return (char)kLowerToUpper[i][1];

	return cp850;
}

int16 Util::getKey() {
	Common::KeyState key;

	while (!getKeyFromBuffer(key)) {
		processInput();

		if (keyBufferEmpty())
			g_system->delayMillis(10 / _vm->_global->_speedFactor);
	}
	return translateKey(key);
}

int16 Util::checkKey() {
	Common::KeyState key;

	getKeyFromBuffer(key);

	return translateKey(key);
}

bool Util::checkKey(int16 &key) {
	Common::KeyState keyS;

	if (!getKeyFromBuffer(keyS))
		return false;

	key = translateKey(keyS);

	return true;
}

bool Util::keyPressed() {
	int16 key = checkKey();
	if (key)
		return true;

	int16 x, y;
	MouseButtons buttons;

	getMouseState(&x, &y, &buttons);
	return buttons != kMouseButtonsNone;
}

void Util::getMouseState(int16 *pX, int16 *pY, MouseButtons *pButtons) {
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	*pX = mouse.x + _vm->_video->_scrollOffsetX - _vm->_video->_screenDeltaX;
	*pY = mouse.y + _vm->_video->_scrollOffsetY - _vm->_video->_screenDeltaY;

	if (pButtons != 0)
		*pButtons = _mouseButtons;
}

void Util::setMousePos(int16 x, int16 y) {
	x = CLIP<int>(x + _vm->_video->_screenDeltaX, 0, _vm->_width - 1);
	y = CLIP<int>(y + _vm->_video->_screenDeltaY, 0, _vm->_height - 1);
	g_system->warpMouse(x, y);
}

void Util::waitMouseUp() {
	do {
		processInput();
		if (_mouseButtons != kMouseButtonsNone)
			delay(10);
	} while (_mouseButtons != kMouseButtonsNone);
}

void Util::waitMouseDown() {
	int16 x;
	int16 y;
	MouseButtons buttons;

	do {
		processInput();
		getMouseState(&x, &y, &buttons);
		if (buttons == 0)
			delay(10);
	} while (buttons == 0);
}

void Util::waitMouseRelease(char drawMouse) {
	MouseButtons buttons;
	int16 mouseX;
	int16 mouseY;

	_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, drawMouse);
	while (buttons != 0) {
		if (drawMouse != 0)
			_vm->_draw->animateCursor(2);
		delay(10);
		_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, drawMouse);
	}
}

void Util::forceMouseUp(bool onlyWhenSynced) {
	if (onlyWhenSynced && (_vm->_game->_mouseButtons != _mouseButtons))
		return;

	_vm->_game->_mouseButtons = kMouseButtonsNone;
	_mouseButtons             = kMouseButtonsNone;
}

void Util::clearPalette() {
	int16 i;
	byte colors[768];

	_vm->validateVideoMode(_vm->_global->_videoMode);

	if (_vm->_global->_setAllPalette) {
		if (_vm->getPixelFormat().bytesPerPixel == 1) {
			memset(colors, 0, sizeof(colors));
			g_system->getPaletteManager()->setPalette(colors, 0, 256);
		}

		return;
	}

	for (i = 0; i < 16; i++)
		_vm->_video->setPalElem(i, 0, 0, 0, 0, _vm->_global->_videoMode);
}

int16 Util::getFrameRate() {
	return _frameRate;
}

void Util::setFrameRate(int16 rate) {
	if (rate == 0)
		rate = 1;

	_frameRate = rate;
	_frameWaitTime = 1000 / rate;
	_startFrameTime = getTimeKey();
}

void Util::notifyNewAnim() {
	_startFrameTime = getTimeKey();
}

void Util::waitEndFrame(bool handleInput) {
	int32 time;

	time = getTimeKey() - _startFrameTime;
	if ((time > 1000) || (time < 0)) {
		_vm->_video->retrace();
		_startFrameTime = getTimeKey();
		return;
	}

	int32 toWait = 0;
	do {
		if (toWait > 0)
			delay(MIN<int>(toWait, 10));

		if (handleInput)
			processInput();

		_vm->_video->retrace();

		time   = getTimeKey() - _startFrameTime;
		toWait = _frameWaitTime - time;
	} while (toWait > 0);

	_startFrameTime = getTimeKey();
}

void Util::setScrollOffset(int16 x, int16 y) {
	processInput();

	if (x >= 0)
		_vm->_video->_scrollOffsetX = x;
	else
		_vm->_video->_scrollOffsetX = _vm->_draw->_scrollOffsetX;

	if (y >= 0)
		_vm->_video->_scrollOffsetY = y;
	else
		_vm->_video->_scrollOffsetY = _vm->_draw->_scrollOffsetY;

	_vm->_video->waitRetrace();
}

void Util::insertStr(const char *str1, char *str2, int16 pos) {
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	int from = MIN((int) pos, len2);

	for (int i = len2; i >= from; i--)
		str2[len1 + i] = str2[i];
	for (int i = 0; i < len1; i++)
		str2[i + from] = str1[i];
}

void Util::cutFromStr(char *str, int16 from, int16 cutlen) {
	int len = strlen(str);

	if (from >= len)
		return;
	if ((from + cutlen) > len) {
		str[from] = 0;
		return;
	}

	int i = from;
	do {
		str[i] = str[i + cutlen];
		i++;
	} while (str[i] != 0);
}

void Util::replaceChar(char *str, char c1, char c2) {
	while ((str = strchr(str, c1)))
		*str = c2;
}

static const char trStr1[] =
	"       '   + - :0123456789: <=>  abcdefghijklmnopqrstuvwxyz      "
	"abcdefghijklmnopqrstuvwxyz     ";
static const char trStr2[] =
	" ueaaaaceeeiii     ooouu        aioun"
	"                                                           ";
static const char trStr3[] = "                                ";

void Util::cleanupStr(char *str) {
	char *start, *end;
	char buf[300];

	strcpy(buf, trStr1);
	strcat(buf, trStr2);
	strcat(buf, trStr3);

	// Translating "wrong" characters
	for (size_t i = 0; i < strlen(str); i++)
		str[i] = buf[MIN<int>(str[i] - 32, 32)];

	// Trim spaces left
	while (str[0] == ' ')
		cutFromStr(str, 0, 1);

	// Trim spaces right
	while ((*str != '\0') && (str[strlen(str) - 1] == ' '))
		cutFromStr(str, strlen(str) - 1, 1);

	// Merge double spaces
	start = strchr(str, ' ');
	while (start) {
		if (start[1] == ' ') {
			cutFromStr(str, start - str, 1);
			continue;
		}

		end = strchr(start + 1, ' ');
		start = end ? end + 1 : 0;
	}
}

void Util::listInsertFront(List *list, void *data) {
	ListNode *node;

	node = new ListNode;
	if (list->pHead) {
		node->pData = data;
		node->pNext = list->pHead;
		node->pPrev = 0;
		list->pHead->pPrev = node;
		list->pHead = node;
	} else {
		list->pHead = node;
		list->pTail = node;
		node->pData = data;
		node->pNext = 0;
		node->pPrev = 0;
	}
}

void Util::listInsertBack(List *list, void *data) {
	ListNode *node;

	if (list->pHead != 0) {
		if (list->pTail == 0) {
			list->pTail = list->pHead;
			warning("Util::listInsertBack(): Broken list");
		}

		node = new ListNode;
		node->pData = data;
		node->pPrev = list->pTail;
		node->pNext = 0;
		list->pTail->pNext = node;
		list->pTail = node;
	} else
		listInsertFront(list, data);
}

void Util::listDropFront(List *list) {
	if (list->pHead->pNext == 0) {
		delete list->pHead;
		list->pHead = 0;
		list->pTail = 0;
	} else {
		list->pHead = list->pHead->pNext;
		delete list->pHead->pPrev;
		list->pHead->pPrev = 0;
	}
}

void Util::deleteList(List *list) {
	while (list->pHead)
		listDropFront(list);

	delete list;
}

char *Util::setExtension(char *str, const char *ext) {
	assert(str && ext);

	if (str[0] == '\0')
		return str;

	char *dot = strrchr(str, '.');
	if (dot)
		*dot = '\0';

	strcat(str, ext);
	return str;
}

Common::String Util::setExtension(const Common::String &str, const Common::String &ext) {
	if (str.empty())
		return str;

	const char *dot = strrchr(str.c_str(), '.');
	if (dot)
		return Common::String(str.c_str(), dot - str.c_str()) + ext;

	return str + ext;
}

Common::String Util::readString(Common::SeekableReadStream &stream, int n) {
	Common::String str;

	char c;
	while (n-- > 0) {
		if ((c = stream.readByte()) == '\0')
			break;

		str += c;
	}

	if (n > 0)
		stream.skip(n);

	return str;
}

/* NOT IMPLEMENTED */
void Util::checkJoystick() {
	_vm->_global->_useJoystick = 0;
}

uint32 Util::getKeyState() const {
	return _keyState;
}

void Util::keyDown(const Common::Event &event) {
	if      (event.kbd.keycode == Common::KEYCODE_UP)
		_keyState |= 0x0001;
	else if (event.kbd.keycode == Common::KEYCODE_DOWN)
		_keyState |= 0x0002;
	else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
		_keyState |= 0x0004;
	else if (event.kbd.keycode == Common::KEYCODE_LEFT)
		_keyState |= 0x0008;
	else if (event.kbd.keycode == Common::KEYCODE_SPACE)
		_keyState |= 0x0020;
	else if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
		_keyState |= 0x0040;
}

void Util::keyUp(const Common::Event &event) {
	if      (event.kbd.keycode == Common::KEYCODE_UP)
		_keyState &= ~0x0001;
	else if (event.kbd.keycode == Common::KEYCODE_DOWN)
		_keyState &= ~0x0002;
	else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
		_keyState &= ~0x0004;
	else if (event.kbd.keycode == Common::KEYCODE_LEFT)
		_keyState &= ~0x0008;
	else if (event.kbd.keycode == Common::KEYCODE_SPACE)
		_keyState &= ~0x0020;
	else if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
		_keyState &= ~0x0040;
}

} // End of namespace Gob
