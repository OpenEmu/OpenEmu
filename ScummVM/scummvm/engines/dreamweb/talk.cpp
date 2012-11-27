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

void DreamWebEngine::talk() {
	_talkPos = 0;
	_inMapArea = 0;
	_character = _command;
	createPanel();
	showPanel();
	showMan();
	showExit();
	underTextLine();
	convIcons();
	startTalk();
	_commandType = 255;
	readMouse();
	showPointer();
	workToScreen();

	RectWithCallback talkList[] = {
		{ 273,320,157,198,&DreamWebEngine::getBack1 },
		{ 240,290,2,44,&DreamWebEngine::moreTalk },
		{ 0,320,0,200,&DreamWebEngine::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	do {
		delPointer();
		readMouse();
		animPointer();
		showPointer();
		waitForVSync();
		dumpPointer();
		dumpTextLine();
		_getBack = 0;
		checkCoords(talkList);
		if (_quitRequested)
			break;
	} while (!_getBack);

	if (_talkPos >= 4)
		_personData->b7 |= 128;

	redrawMainScrn();
	workToScreenM();
	if (_speechLoaded) {
		_sound->cancelCh1();
		_sound->volumeChange(0, -1);
	}
}

void DreamWebEngine::convIcons() {
	uint8 index = _character & 127;
	uint16 frame = getPersFrame(index);
	const GraphicsFile *base = findSource(frame);
	showFrame(*base, 234, 2, frame, 0);
}

uint16 DreamWebEngine::getPersFrame(uint8 index) {
	return READ_LE_UINT16(&_personFramesLE[index]);
}

void DreamWebEngine::startTalk() {
	_talkMode = 0;

	const uint8 *str = getPersonText(_character & 0x7F, 0);
	uint16 y;

	_charShift = 91+91;
	y = 64;
	printDirect(&str, 66, &y, 241, true);

	_charShift = 0;
	y = 80;
	printDirect(&str, 66, &y, 241, true);

	if (hasSpeech()) {
		_speechLoaded = _sound->loadSpeech('R', _realLocation, 'C', 64*(_character & 0x7F));
		if (_speechLoaded) {
			_sound->volumeChange(6, 1);
			_sound->playChannel1(62);
		}
	}
}

const uint8 *DreamWebEngine::getPersonText(uint8 index, uint8 talkPos) {
	return (const uint8 *)_personText.getString(index*64 + talkPos);
}

void DreamWebEngine::moreTalk() {
	if (_talkMode != 0) {
		redes();
		return;
	}

	commandOnlyCond(49, 215);

	if (_mouseButton == _oldButton)
		return;	// nomore

	if (!(_mouseButton & 1))
		return;

	_talkMode = 2;
	_talkPos = 4;

	if (_character >= 100)
		_talkPos = 48; // second part
	doSomeTalk();
}

void DreamWebEngine::doSomeTalk() {
	// FIXME: This is for the CD version only

	while (true) {
		const uint8 *str = getPersonText(_character & 0x7F, _talkPos);

		if (*str == 0) {
			// endheartalk
			_pointerMode = 0;
			return;
		}

		createPanel();
		showPanel();
		showMan();
		showExit();
		convIcons();

		printDirect(str, 164, 64, 144, false);

		_speechLoaded = _sound->loadSpeech('R', _realLocation, 'C', (64 * (_character & 0x7F)) + _talkPos);
		if (_speechLoaded)
			_sound->playChannel1(62);

		_pointerMode = 3;
		workToScreenM();
		if (hangOnPQ())
			return;

		_talkPos++;

		str = getPersonText(_character & 0x7F, _talkPos);
		if (*str == 0) {
			// endheartalk
			_pointerMode = 0;
			return;
		}

		if (*str != ':' && *str != 32) {
			createPanel();
			showPanel();
			showMan();
			showExit();
			convIcons();
			printDirect(str, 48, 128, 144, false);

			_speechLoaded = _sound->loadSpeech('R', _realLocation, 'C', (64 * (_character & 0x7F)) + _talkPos);
			if (_speechLoaded)
				_sound->playChannel1(62);

			_pointerMode = 3;
			workToScreenM();
			if (hangOnPQ())
				return;
		}

		_talkPos++;
	}
}

bool DreamWebEngine::hangOnPQ() {
	_getBack = 0;

	RectWithCallback quitList[] = {
		{ 273,320,157,198,&DreamWebEngine::getBack1 },
		{ 0,320,0,200,&DreamWebEngine::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	uint16 speechFlag = 0;

	do {
		delPointer();
		readMouse();
		animPointer();
		showPointer();
		waitForVSync();
		dumpPointer();
		dumpTextLine();
		checkCoords(quitList);

		if (_getBack == 1 || _quitRequested) {
			// Quit conversation
			delPointer();
			_pointerMode = 0;
			_sound->cancelCh1();
			return true;
		}

		if (_speechLoaded && !_sound->isChannel1Playing()) {
			speechFlag++;
			if (speechFlag == 40)
				break;
		}
	} while (!_mouseButton || _oldButton);

	delPointer();
	_pointerMode = 0;
	return false;
}

void DreamWebEngine::redes() {
	if (_sound->isChannel1Playing() || _talkMode != 2) {
		blank();
		return;
	}

	commandOnlyCond(50, 217);

	if (!(_mouseButton & 1))
		return;

	delPointer();
	createPanel();
	showPanel();
	showMan();
	showExit();
	convIcons();
	startTalk();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

} // End of namespace DreamWeb
