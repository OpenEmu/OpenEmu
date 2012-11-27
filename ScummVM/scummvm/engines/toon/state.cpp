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

#include "common/debug.h"

#include "toon/state.h"
#include "toon/toon.h"

namespace Toon {

void Location::save(Common::WriteStream *stream) {
	stream->write(_cutaway, 64);
	stream->write(_music, 64);
	stream->write(_name, 64);
	stream->writeSint16BE(_numRifBoxes);
	stream->writeSint16BE(_numSceneAnimations);
	stream->writeSByte(_visited);

	for (int32 i = 0; i < _numRifBoxes * 2; i++) {
		stream->writeSint16BE(_rifBoxesFlags[i]);
	}
}
void Location::load(Common::ReadStream *stream) {
	stream->read(_cutaway, 64);
	stream->read(_music, 64);
	stream->read(_name, 64);
	_numRifBoxes = stream->readSint16BE();
	_numSceneAnimations = stream->readSint16BE();
	_visited = stream->readSByte();

	for (int32 i = 0; i < _numRifBoxes * 2; i++) {
		_rifBoxesFlags[i] = stream->readSint16BE();
	}
}

State::State(void) {
	for (int32 i = 0; i < 256; i++) {
		_locations[i]._visited = false;
		_locations[i]._numSceneAnimations = 0;
		_locations[i]._numRifBoxes = 0;
	}

	memset(_gameFlag, 0, sizeof(_gameFlag));
	memset(_gameGlobalData, -1, sizeof(_gameGlobalData));

	for (int32 i = 0; i < 2; i++) {
		_timerEnabled[i] = false;
		_timerTimeout[i] = 0;
		_timerDelay[i] = -1;
	}

	_lastVisitedScene = -1;
	_currentScene = -1;

	_currentScrollLock = false;
	_currentScrollValue = 0;

	_gameTimer = 0;
	_currentChapter = 1;

	_showConversationIcons = false;

	_inMenu = false;
	_inCloseUp = false;
	_inConversation = false;

	_mouseState = -1;
	_mouseHidden = false;

	_firstConverstationLine = false;

	_sackVisible = false; // to change
	_inCutaway = false;

	_inInventory = false;
	_numInventoryItems = 0; //To chhange
	_numConfiscatedInventoryItems = 0;

	_nextSpecialEnterX = -1;
	_nextSpecialEnterY = -1;

#if 0
	for (int i = 0; i < 30; i++) {
		_inventory[i] = 90 + i;
		if (_inventory[i] == 41)
			_inventory[i] = 42;
	}

	_inventory[0] = 53;
	_inventory[1] = 22;
	_inventory[2] = 93;
	_inventory[3] = 49;
	_inventory[4] = 47;
	_inventory[5] = 14;
	_numInventoryItems = 6; //To change
#endif

	memset(_conversationState, 0, sizeof(Conversation) * 60);
}

State::~State(void) {
}

int32 State::getGameFlag(int32 flagId) {
	return (_gameFlag[flagId >> 3] & (1 << (flagId & 7))) != 0;
}

bool State::hasItemInInventory(int32 item) {
	debugC(1, kDebugState, "hasItemInInventory(%d)", item);

	for (int32 i = 0; i < _numInventoryItems; i++) {
		if (_inventory[i] == item)
			return true;
	}
	return false;
}

void State::save(Common::WriteStream *stream) {

	for (int32 i = 0; i < 256; i++) {
		_locations[i].save(stream);
	}

	for (int32 i = 0; i < 256; i++) {
		stream->writeSint16BE(_gameGlobalData[i]);
	}

	for (int32 i = 0; i < 256; i++) {
		stream->writeSint16BE(_gameFlag[i]);
	}

	stream->writeSint16BE(_lastVisitedScene);
	stream->writeSint16BE(_currentScene);
	stream->writeSint16BE(_currentScrollValue);
	stream->writeSByte(_currentScrollLock);

	for (int32 i = 0; i < 35; i++) {
		stream->writeSint16BE(_inventory[i]);
	}

	for (int32 i = 0; i < 35; i++) {
		stream->writeSint16BE(_confiscatedInventory[i]);
	}

	stream->writeSint32BE(_numInventoryItems);
	stream->writeSint32BE(_numConfiscatedInventoryItems);

	stream->writeSByte(_inCloseUp);
	stream->writeSByte(_inCutaway);
	stream->writeSByte(_inConversation);
	stream->writeSByte(_inInventory);
	stream->writeSByte(_showConversationIcons);

	stream->writeSint16BE(_mouseState);

	stream->writeSint16BE(_currentConversationId);
	stream->writeSByte(_firstConverstationLine);
	stream->writeSByte(_exitConversation);
	stream->writeSByte(_mouseHidden);
	stream->writeSByte(_sackVisible);
	stream->writeSint32BE(_gameTimer);
	stream->writeSByte(_currentChapter);

	stream->writeByte(_timerEnabled[0]);
	stream->writeByte(_timerEnabled[1]);

	stream->writeSint32BE(_timerTimeout[0]);
	stream->writeSint32BE(_timerTimeout[1]);

	stream->writeSint32BE(_timerDelay[0]);
	stream->writeSint32BE(_timerDelay[1]);
}

void State::load(Common::ReadStream *stream) {
	for (int32 i = 0; i < 256; i++) {
		_locations[i].load(stream);
	}

	for (int32 i = 0; i < 256; i++) {
		_gameGlobalData[i] = stream->readSint16BE();
	}

	for (int32 i = 0; i < 256; i++) {
		_gameFlag[i] = stream->readSint16BE();
	}

	_lastVisitedScene = stream->readSint16BE();
	_currentScene = stream->readSint16BE();
	_currentScrollValue = stream->readSint16BE();
	_currentScrollLock = stream->readSByte();

	for (int32 i = 0; i < 35; i++) {
		_inventory[i] = stream->readSint16BE();
	}

	for (int32 i = 0; i < 35; i++) {
		_confiscatedInventory[i] = stream->readSint16BE();
	}

	_numInventoryItems = stream->readSint32BE();
	_numConfiscatedInventoryItems = stream->readSint32BE();

	_inCloseUp = stream->readSByte();
	_inCutaway = stream->readSByte();
	_inConversation = stream->readSByte();
	_inInventory = stream->readSByte();
	_showConversationIcons = stream->readSByte();

	_mouseState = stream->readSint16BE();

	_currentConversationId = stream->readSint16BE();
	_firstConverstationLine = stream->readSByte();
	_exitConversation = stream->readSByte();
	_mouseHidden = stream->readSByte();
	_sackVisible = stream->readSByte();
	_gameTimer = stream->readSint32BE();
	_currentChapter = stream->readSByte();

	_timerEnabled[0] = stream->readByte();
	_timerEnabled[1] = stream->readByte();

	_timerTimeout[0] = stream->readSint32BE();
	_timerTimeout[1] = stream->readSint32BE();

	_timerDelay[0] = stream->readSint32BE();
	_timerDelay[1] = stream->readSint32BE();
}

void State::loadConversations(Common::ReadStream *stream) {
	for (int32 i = 0; i < 60; i++) {
		_conversationState[i].load(stream, _conversationData);
	}
}

void State::saveConversations(Common::WriteStream *stream) {
	for (int32 i = 0; i < 60; i++) {
		_conversationState[i].save(stream, _conversationData);
	}
}

} // End of namespace Toon
