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

#ifndef TOON_STATE_H
#define TOON_STATE_H

#include "common/file.h"
#include "common/str.h"
#include "toon/conversation.h"

namespace Toon {

struct Location {
	char _name[64];
	char _music[64];
	char _cutaway[64];
	bool _visited;
	int32 _numSceneAnimations;
	int32 _flags;
	int32 _numRifBoxes;
	int16 _rifBoxesFlags[256];

	void save(Common::WriteStream *stream);
	void load(Common::ReadStream *stream);
};

class State {
public:
	State(void);
	~State(void);

	Location _locations[256];
	int16 _gameGlobalData[256];
	uint8 _gameFlag[256];
	int16 _lastVisitedScene;
	int16 _currentScene;
	int16 _currentScrollValue;
	bool _currentScrollLock;
	int16 _inventory[35];
	int16 _confiscatedInventory[35];
	int32 _numInventoryItems;
	int32 _numConfiscatedInventoryItems;
	bool _inMenu;
	bool _inCloseUp;
	bool _inCutaway;
	bool _inConversation;
	bool _inInventory;
	bool _showConversationIcons;
	int16 _mouseState;
	int16 *_conversationData;
	Conversation _conversationState[60];
	int16 _currentConversationId;
	bool _firstConverstationLine;
	bool _exitConversation;
	bool _mouseHidden;
	bool _sackVisible;
	int32 _gameTimer;
	int8 _currentChapter;
	int32 _nextSpecialEnterX;
	int32 _nextSpecialEnterY;

	bool _timerEnabled[2];
	int32 _timerTimeout[2];
	int32 _timerDelay[2];

	int32 getGameFlag(int32 flagId);
	bool hasItemInInventory(int32 item);

	void load(Common::ReadStream *stream);
	void save(Common::WriteStream *stream);

	void loadConversations(Common::ReadStream *stream);
	void saveConversations(Common::WriteStream *stream);
};

} // End of namespace Toon

#endif
