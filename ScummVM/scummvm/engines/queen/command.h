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

#ifndef QUEEN_COMMAND_H
#define QUEEN_COMMAND_H

#include "common/util.h"
#include "queen/structs.h"

namespace Queen {

class QueenEngine;

class CmdText {
public:

	static CmdText *makeCmdTextInstance(uint8 y, QueenEngine *vm);

	CmdText(uint8 y, QueenEngine *vm);
	virtual ~CmdText() {}

	//! reset the command sentence
	void clear();

	//! display the command sentence using the specified color
	void display(InkColor color, const char *command = 0, bool outlined = false);

	//! display a temporary command sentence using the specified parameters
	void displayTemp(InkColor color, Verb v);

	//! display a temporary command sentence using the specified parameters
	virtual void displayTemp(InkColor color, const char *name, bool outlined);

	//! set the verb for the command sentence
	void setVerb(Verb v);

	//! set the link word (between verb and object) for the command sentence
	virtual void addLinkWord(Verb v);

	//! add an object name to the command sentence
	virtual void addObject(const char *objName);

	//! returns true if the command sentence is empty
	bool isEmpty() const { return _command[0] == 0; }

	enum {
		MAX_COMMAND_LEN = 256,
		COMMAND_Y_POS   = 151
	};

protected:

	//! buffer containing the current command sentence
	char _command[MAX_COMMAND_LEN];

	uint8 _y;

	QueenEngine *_vm;
};

struct CmdState {

	void init();

	Verb oldVerb, verb;
	Verb action;
	int16 oldNoun, noun;
	int commandLevel;
	int16 subject[2];

	Verb selAction;
	int16 selNoun;
};

class Command {
public:

	Command(QueenEngine *vm);
	~Command();

	//! initialize command construction
	void clear(bool clearTexts);

	//! execute last constructed command
	void executeCurrentAction();

	//! get player input and construct command from it
	void updatePlayer();

	//! read all command arrays from stream
	void readCommandsFrom(byte *&ptr);

	enum {
		MAX_MATCHING_CMDS = 50
	};

private:

	//! get a reference to the ObjectData for the specified room object
	ObjectData *findObjectData(uint16 objRoomNum) const;

	//! get a reference to the ItemData for the specified inventory object
	ItemData *findItemData(Verb invNum) const;

	//! execute the current command
	int16 executeCommand(uint16 comId, int16 condResult);

	//! move Joe to the specified position, handling new room switching
	int16 makeJoeWalkTo(int16 x, int16 y, int16 objNum, Verb v, bool mustWalk);

	//! update command state with current selected action
	void grabCurrentSelection();

	//! update command state with current selected object
	void grabSelectedObject(int16 objNum, uint16 objState, uint16 objName);

	//! update command state with current selected inventory object
	void grabSelectedItem();

	//! update command state with current selected room object
	void grabSelectedNoun();

	//! update command state with current selected verb
	void grabSelectedVerb();

	//! if the description is a cutaway file, execute it
	bool executeIfCutaway(const char *description);

	//! if the description is a dialog file, execute it
	bool executeIfDialog(const char *description);

	//! handle a wrong/invalid user action
	bool handleWrongAction();

	//! make Joe speak something for a wrong/invalid action
	void sayInvalidAction(Verb action, int16 subj1, int16 subj2);

	//! update an object state
	void changeObjectState(Verb action, int16 obj, int16 song, bool cutDone);

	//! reset current action
	void cleanupCurrentAction();

	//! OPEN_CLOSE_OTHER(OBJECT_DATA[S][4])
	void openOrCloseAssociatedObject(Verb action, int16 obj);

	//! update gamestates - P1_SET_CONDITIONS
	int16 setConditions(uint16 command, bool lastCmd);

	//! turn on/off areas - P2_SET_AREAS
	void setAreas(uint16 command);

	//! hide/show objects, redisplay if in the same room as Joe - P3_SET_OBJECTS
	void setObjects(uint16 command);

	//! inserts/deletes items (inventory) - P4_SET_ITEMS
	void setItems(uint16 command);

	//! update description for object and returns description number to use
	uint16 nextObjectDescription(ObjectDescription *objDesc, uint16 firstDesc);

	//! speak description of selected object
	void lookAtSelectedObject();

	//! get the current object under the cursor
	void lookForCurrentObject(int16 cx, int16 cy);

	//! get the current icon panel under the cursor (inventory item or verb)
	void lookForCurrentIcon(int16 cx, int16 cy);

	//! returns true if the verb is an action verb
	bool isVerbAction(Verb v) const { return (v >= VERB_PANEL_COMMAND_FIRST && v <= VERB_PANEL_COMMAND_LAST) || (v == VERB_WALK_TO); }

	//! return true if the verb is an inventory item
	bool isVerbInv(Verb v) const { return v >= VERB_INV_FIRST && v <= VERB_INV_LAST; }

	//! returns true if the specified verb is an inventory scroll
	bool isVerbInvScroll(Verb v) const { return v == VERB_SCROLL_UP || v == VERB_SCROLL_DOWN; }

	//! commands list for each possible action
	CmdListData *_cmdList;
	uint16 _numCmdList;

	//! commands list for areas
	CmdArea *_cmdArea;
	uint16 _numCmdArea;

	//! commands list for objects
	CmdObject *_cmdObject;
	uint16 _numCmdObject;

	//! commands list for inventory
	CmdInventory *_cmdInventory;
	uint16 _numCmdInventory;

	//! commands list for gamestate
	CmdGameState *_cmdGameState;
	uint16 _numCmdGameState;

	//! textual form of the command (displayed between room and panel areas)
	CmdText *_cmdText;

	//! flag indicating that the current command is fully constructed
	bool _parse;

	//! state of current constructed command
	CmdState _state;

	//! last user selection
	int _mouseKey, _selPosX, _selPosY;

	QueenEngine *_vm;
};

} // End of namespace Queen

#endif
