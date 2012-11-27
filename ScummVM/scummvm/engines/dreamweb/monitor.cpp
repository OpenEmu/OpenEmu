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

#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"

namespace DreamWeb {

struct MonitorKeyEntry {
	uint8 keyAssigned;
	char  username[12];
	char  password[12];
};

// New monitor key list
static MonitorKeyEntry monitorKeyEntries[4] = {
	{ 1, "PUBLIC",  "PUBLIC"      },
	{ 0, "RYAN",    "BLACKDRAGON" },
	{ 0, "LOUIS",   "HENDRIX"     },
	{ 0, "BECKETT", "SEPTIMUS"    }
};

void DreamWebEngine::useMon() {
	_vars._lastTrigger = 0;
	_currentFile[0] = 34;
	memset(_currentFile+1, ' ', 12);
	_currentFile[13] = 0;

	monitorKeyEntries[0].keyAssigned = 1;
	monitorKeyEntries[1].keyAssigned = 0;
	monitorKeyEntries[2].keyAssigned = 0;
	monitorKeyEntries[3].keyAssigned = 0;

	createPanel();
	showPanel();
	showIcon();
	drawFloor();
	getRidOfAll();
	loadGraphicsFile(_monitorGraphics, "G03"); // mon. graphic name
	loadPersonal();
	loadNews();
	loadCart();
	loadGraphicsFile(_monitorCharset, "C01"); // character set 2
	printOuterMon();
	initialMonCols();
	printLogo();
	workToScreen();
	turnOnPower();
	fadeUpYellows();
	fadeUpMonFirst();
	_monAdX = 76;
	_monAdY = 141;
	monMessage(1);
	hangOnCurs(120);
	monMessage(2);
	randomAccess(60);
	monMessage(3);
	hangOnCurs(100);
	printLogo();
	scrollMonitor();
	_bufferIn = 0;
	_bufferOut = 0;
	bool stop = false;
	do {
		uint16 oldMonadx = _monAdX;
		uint16 oldMonady = _monAdY;
		input();
		_monAdX = oldMonadx;
		_monAdY = oldMonady;
		stop = execCommand();
		if (_quitRequested) //TODO : Check why it crashes when put before the execcommand
			break;
	} while (!stop);
	_monitorGraphics.clear();
	_monitorCharset.clear();

	_textFile1.clear();
	_textFile2.clear();
	_textFile3.clear();

	_getBack = 1;
	_sound->playChannel1(26);
	_manIsOffScreen = 0;
	restoreAll();
	redrawMainScrn();
	workToScreenM();
}

bool DreamWebEngine::execCommand() {
	static const char *comlist[] = {
		"EXIT",
		"HELP",
		"LIST",
		"READ",
		"LOGON",
		"KEYS"
	};

	if (_inputLine[0] == 0) {
		// No input
		scrollMonitor();
		return false;
	}

	int cmd;
	bool done = false;
	// Loop over all commands in the list and see if we get a match
	for (cmd = 0; cmd < ARRAYSIZE(comlist); ++cmd) {
		const char *cmdStr = comlist[cmd];
		const char *inputStr = _inputLine;
		// Compare the command, char by char, to see if we get a match.
		// We only care about the prefix matching, though.
		char inputChar, cmdChar;
		do {
			inputChar = *inputStr; inputStr += 2;
			cmdChar = *cmdStr++;
			if (cmdChar == 0) {
				done = true;
				break;
			}
		} while (inputChar == cmdChar);

		if (done)
			break;
	}

	// Execute the selected command
	switch (cmd) {
	case 0:
		return true;
	case 1:
		monMessage(6);
		// An extra addition in ScummVM: available commands.
		// Since the reference to the game manual is a form of copy protection,
		// this extra text is wrapped around the common copy protection check,
		// to keep it faithful to the original, if requested.
		if (!_copyProtection) {
			monPrint("VALID COMMANDS ARE EXIT, HELP, LIST, READ, LOGON, KEYS");
		}
		break;
	case 2:
		dirCom();
		break;
	case 3:
		read();
		break;
	case 4:
		signOn();
		break;
	case 5:
		showKeys();
		break;
	default:
		netError();
		break;
	}
	return false;
}



void DreamWebEngine::monitorLogo() {
	if (_logoNum != _oldLogoNum) {
		_oldLogoNum = _logoNum;
		//fadeDownMon(); // FIXME: Commented out in ASM
		printLogo();
		printUnderMonitor();
		workToScreen();
		printLogo();
		//fadeUpMon(); // FIXME: Commented out in ASM
		printLogo();
		_sound->playChannel1(26);
		randomAccess(20);
	} else {
		printLogo();
	}
}

void DreamWebEngine::printLogo() {
	showFrame(_monitorGraphics, 56, 32, 0, 0);
	showCurrentFile();
}

void DreamWebEngine::input() {
	memset(_inputLine, 0, sizeof(_inputLine));
	_curPos = 0;
	printChar(_monitorCharset, _monAdX, _monAdY, '>', 0, NULL, NULL);
	multiDump(_monAdX, _monAdY, 6, 8);
	_monAdX += 6;
	_cursLocX = _monAdX;
	_cursLocY = _monAdY;
	while (true) {
		printCurs();
		waitForVSync();
		delCurs();
		readKey();
		if (_quitRequested)
			return;
		uint8 currentKey = _currentKey;
		if (currentKey == 0)
			continue;
		if (currentKey == 13)
			return;
		if (currentKey == 8) {
			if (_curPos > 0)
				delChar();
			continue;
		}
		if (_curPos == 28)
			continue;
		if ((currentKey == 32) && (_curPos == 0))
			continue;
		currentKey = makeCaps(currentKey);
		_inputLine[_curPos * 2 + 0] = currentKey;
		if (currentKey > 'Z')
			continue;
		multiGet(_mapStore + _curPos * 256, _monAdX, _monAdY, 8, 8);
		uint8 charWidth;
		printChar(_monitorCharset, _monAdX, _monAdY, currentKey, 0, &charWidth, NULL);
		_inputLine[_curPos * 2 + 1] = charWidth;
		_monAdX += charWidth;
		++_curPos;
		_cursLocX += charWidth;
	}
}

byte DreamWebEngine::makeCaps(byte c) {
	// TODO: Replace calls to this by toupper() ?
	if (c >= 'a')
		c -= 'a' - 'A'; // = 32
	return c;
}

void DreamWebEngine::delChar() {
	--_curPos;
	_inputLine[_curPos * 2] = 0;
	uint8 width = _inputLine[_curPos * 2 + 1];
	_monAdX -= width;
	_cursLocX -= width;
	uint16 offset = _curPos;
	offset = ((offset & 0x00ff) << 8) | ((offset & 0xff00) >> 8);
	multiPut(_mapStore + offset, _monAdX, _monAdY, 8, 8);
	multiDump(_monAdX, _monAdY, 8, 8);
}

void DreamWebEngine::printCurs() {
	uint16 x = _cursLocX;
	uint16 y = _cursLocY;
	uint16 height;
	if (_foreignRelease) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiGet(_textUnder, x, y, 6, height);
	++_mainTimer;
	if ((_mainTimer & 16) == 0)
		showFrame(_monitorCharset, x, y, '/' - 32, 0);
	multiDump(x - 6, y, 12, height);
}

void DreamWebEngine::delCurs() {
	uint16 x = _cursLocX;
	uint16 y = _cursLocY;
	uint16 width = 6;
	uint16 height;
	if (_foreignRelease) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiPut(_textUnder, x, y, width, height);
	multiDump(x, y, width, height);
}

void DreamWebEngine::scrollMonitor() {
	printLogo();
	printUnderMonitor();
	workToScreen();
	_sound->playChannel1(25);
}

void DreamWebEngine::showCurrentFile() {
	uint16 x;
	// Monitor Frame position differs between Floppy and CD version
	if (isCD())
		x = 178;
	else
		x = 199;
	const char *currentFile = _currentFile + 1;
	while (*currentFile) {
		char c = *currentFile++;
		c = modifyChar(c);
		printChar(_monitorCharset, &x, 37, c, 0, NULL, NULL);
	}
}

void DreamWebEngine::accessLightOn() {
	showFrame(_monitorGraphics, 74, 182, 8, 0);
	multiDump(74, 182, 12, 8);
}

void DreamWebEngine::accessLightOff() {
	showFrame(_monitorGraphics, 74, 182, 7, 0);
	multiDump(74, 182, 12, 8);
}

void DreamWebEngine::randomAccess(uint16 count) {
	for (uint16 i = 0; i < count; ++i) {
		waitForVSync();
		waitForVSync();
		uint16 v = _rnd.getRandomNumber(15);
		if (v < 10)
			accessLightOff();
		else
			accessLightOn();
	}
	accessLightOff();
}

void DreamWebEngine::monMessage(uint8 index) {
	assert(index > 0);
	const char *string = _textFile1._text;
	for (uint8 i = 0; i < index; ++i) {
		while (*string++ != '+') {
		}
	}
	monPrint(string);
}

void DreamWebEngine::netError() {
	monMessage(5);
	scrollMonitor();
}

void DreamWebEngine::powerLightOn() {
	showFrame(_monitorGraphics, 257+4, 182, 6, 0);
	multiDump(257+4, 182, 12, 8);
}

void DreamWebEngine::powerLightOff() {
	showFrame(_monitorGraphics, 257+4, 182, 5, 0);
	multiDump(257+4, 182, 12, 8);
}

void DreamWebEngine::lockLightOn() {
	showFrame(_monitorGraphics, 56, 182, 10, 0);
	multiDump(58, 182, 12, 8);
}

void DreamWebEngine::lockLightOff() {
	showFrame(_monitorGraphics, 56, 182, 9, 0);
	multiDump(58, 182, 12, 8);
}

void DreamWebEngine::turnOnPower() {
	for (size_t i = 0; i < 3; ++i) {
		powerLightOn();
		hangOn(30);
		powerLightOff();
		hangOn(30);
	}
	powerLightOn();
}

void DreamWebEngine::printOuterMon() {
	showFrame(_monitorGraphics, 40, 32, 1, 0);
	showFrame(_monitorGraphics, 264, 32, 2, 0);
	showFrame(_monitorGraphics, 40, 12, 3, 0);
	showFrame(_monitorGraphics, 40, 164, 4, 0);
}

void DreamWebEngine::loadPersonal() {
	if (_vars._location == 0 || _vars._location == 42)
		loadTextFile(_textFile1, "T01"); // monitor file 1
	else
		loadTextFile(_textFile1, "T02"); // monitor file 2
}

void DreamWebEngine::loadNews() {
	// textfile2 holds information accessible by anyone
	if (_vars._newsItem == 0)
		loadTextFile(_textFile2, "T10"); // monitor file 10
	else if (_vars._newsItem == 1)
		loadTextFile(_textFile2, "T11"); // monitor file 11
	else if (_vars._newsItem == 2)
		loadTextFile(_textFile2, "T12"); // monitor file 12
	else
		loadTextFile(_textFile2, "T13"); // monitor file 13
}

void DreamWebEngine::loadCart() {
	byte cartridgeId = 0;
	uint16 objectIndex = findSetObject("INTF");
	uint16 cartridgeIndex = checkInside(objectIndex, 1);
	if (cartridgeIndex != kNumexobjects)
		cartridgeId = getExAd(cartridgeIndex)->objId[3] + 1;

	if (cartridgeId == 0)
		loadTextFile(_textFile3, "T20"); // monitor file 20
	else if (cartridgeId == 1)
		loadTextFile(_textFile3, "T21"); // monitor file 21
	else if (cartridgeId == 2)
		loadTextFile(_textFile3, "T22"); // monitor file 22
	else if (cartridgeId == 3)
		loadTextFile(_textFile3, "T23"); // monitor file 23
	else
		loadTextFile(_textFile3, "T24"); // monitor file 24
}

void DreamWebEngine::showKeys() {
	randomAccess(10);
	scrollMonitor();
	monMessage(18);

	for (int i = 0; i < 4; i++) {
		if (monitorKeyEntries[i].keyAssigned)
			monPrint(monitorKeyEntries[i].username);
	}

	scrollMonitor();
}

const char *DreamWebEngine::getKeyAndLogo(const char *foundString) {
	byte newLogo = foundString[1] - 48;
	byte keyNum = foundString[3] - 48;

	if (monitorKeyEntries[keyNum].keyAssigned == 1) {
		// Key OK
		_logoNum = newLogo;
		return foundString + 4;
	} else {
		monMessage(12);	// "Access denied, key required -"
		monPrint(monitorKeyEntries[keyNum].username);
		scrollMonitor();
		return 0;
	}
}

const char *DreamWebEngine::searchForString(const char *topic, const char *text) {
	char delim = *topic;

	while (true) {
		const char *s = topic;
		int delimCount = 0;

		char c;
		do {
			c = makeCaps(*text++);

			if (c == '*' || (delim == '=' && c == 34))
				return 0;

			if (c == delim) {
				delimCount++;
				if (delimCount == 2)
					return text;
			}

		} while (c == *s++);
	}
}

void DreamWebEngine::dirCom() {
	randomAccess(30);

	const char *dirname = parser();
	if (dirname[1]) {
		dirFile(dirname);
		return;
	}

	_logoNum = 0;
	memcpy(_currentFile+1, "ROOT        ", 12);
	monitorLogo();
	scrollMonitor();
	monMessage(9);
	searchForFiles(_textFile1._text);
	searchForFiles(_textFile2._text);
	searchForFiles(_textFile3._text);
	scrollMonitor();
}

void DreamWebEngine::dirFile(const char *dirName) {
	char topic[14];

	memcpy(topic, dirName, 14);
	topic[0] = 34;

	const char *text = _textFile1._text;
	const char *found = searchForString(topic, text);
	if (!found) {
		text = _textFile2._text;
		found = searchForString(topic, text);
		if (!found) {
			text = _textFile3._text;
			found = searchForString(topic, text);
		}
	}

	if (found) {
		found = getKeyAndLogo(found);
		if (!found)
			return; // not logged in
	} else {
		monMessage(7);
		return;
	}

	// "keyok2"
	memcpy(_currentFile+1, dirName+1, 12);
	monitorLogo();
	scrollMonitor();
	monMessage(10);

	while (true) {
		byte curChar = *found++;
		if (curChar == 34 || curChar == '*') {
			// "endofdir2"
			scrollMonitor();
			return;
		}

		if (curChar == '=')
			found = monPrint(found);
	}
}

void DreamWebEngine::read() {
	randomAccess(40);
	const char *name = parser();
	if (name[1] == 0) {
		netError();
		return;
	}

	const char *topic = _currentFile;

	const char *text = _textFile1._text;
	const char *found = searchForString(topic, text);
	if (!found) {
		text = _textFile2._text;
		found = searchForString(topic, text);
		if (!found) {
			text = _textFile3._text;
			found = searchForString(topic, text);
		}
	}

	if (found) {
		if (!getKeyAndLogo(found))
			return;
	} else {
		monMessage(7);
		return;
	}

	// "keyok1"
	found = searchForString(name, found);
	if (!found) {
		_logoNum = _oldLogoNum;
		monMessage(11);
		return;
	}

	// "findtopictext"
	monitorLogo();
	scrollMonitor();

	found++;

	while (true) {
		found = monPrint(found);
		if (found[0] == 34 || found[0] == '=' || found[0] == '*') {
			// "endoftopic"
			scrollMonitor();
			return;
		}

		processTrigger();
		randomAccess(24);
	}
}

void DreamWebEngine::signOn() {
	const char *name = parser();

	int8 foundIndex = -1;
	Common::String inputLine = name + 1;
	inputLine.trim();

	for (byte i = 0; i < 4; i++) {
		if (inputLine.equalsIgnoreCase(monitorKeyEntries[i].username)) {
			// Check if the key has already been assigned
			if (monitorKeyEntries[i].keyAssigned) {
				monMessage(17);
				return;
			} else {
				foundIndex = i;
				break;
			}
		}
	}

	if (foundIndex == -1) {
		monMessage(13);
		return;
	}

	monMessage(15);

	uint16 prevX = _monAdX;
	uint16 prevY = _monAdY;
	input();	// password input
	_monAdX = prevX;
	_monAdY = prevY;

	// The entered line has zeroes in-between each character
	uint32 len = strlen(monitorKeyEntries[foundIndex].password);
	bool found = true;

	for (uint32 i = 0; i < len; i++) {
		if (monitorKeyEntries[foundIndex].password[i] != _inputLine[i * 2]) {
			found = false;
			break;
		}
	}

	if (!found) {
		scrollMonitor();
		monMessage(16);
	} else {
		monMessage(14);
		monPrint(monitorKeyEntries[foundIndex].username);
		scrollMonitor();
		monitorKeyEntries[foundIndex].keyAssigned = 1;
	}
}

void DreamWebEngine::searchForFiles(const char *filesString) {
	byte curChar;

	while (true) {
		curChar = filesString[0];
		filesString++;
		if (curChar == '*')
			return; // "endofdir"
		if (curChar == 34)
			filesString = monPrint(filesString);
	}
}

const char *DreamWebEngine::parser() {
	char *output = _operand1;

	memset(output, 0, sizeof(_operand1));

	*output++ = '=';

	const char *in = _inputLine;

	uint8 c;

	// skip command
	do {
		c = *in++;
		in++;

		if (!c)
			return output;
	} while (c != 32);

	// skip spaces between command and operand
	do {
		c = *in++;
		in++;
	} while (c == 32);

	// copy first operand
	do {
		*output++ = c;
		c = *in++;
		in++;
		if (!c)
			return _operand1;
	} while (c != 32);

	return _operand1;
}

} // End of namespace DreamWeb
