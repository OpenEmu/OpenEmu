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

#ifndef SCUMM_SCRIPT_V0_H
#define SCUMM_SCRIPT_V0_H

#include "scumm/scumm_v2.h"

namespace Scumm {

/**
 * Engine for Apple II and Commodore 64 versions of Maniac Mansion
 */
class ScummEngine_v0 : public ScummEngine_v2 {
protected:
	enum CurrentMode {
		kModeCutscene = 0,   // cutscene active
		kModeKeypad = 1,     // kid selection / dial pad / save-load dialog
		kModeNoNewKid = 2,   // verb "new kid" disabled (e.g. when entering lab)
		kModeNormal = 3      // normal playing mode
	};

	enum WalkToObjectState {
		kWalkToObjectStateDone = 0,
		kWalkToObjectStateWalk = 1,
		kWalkToObjectStateTurn = 2
	};

protected:
	byte _currentMode;
	byte _currentLights;

	int _activeVerb;		// selected verb
	int _activeObject;		// 1st selected object (see OBJECT_V0())
	int _activeObject2;		// 2nd selected object or actor (see OBJECT_V0())

	int _cmdVerb;			// script verb
	int _cmdObject;			// 1st script object (see OBJECT_V0())
	int _cmdObject2;		// 2nd script object or actor (see OBJECT_V0())
	int _sentenceNestedCount;

	int _walkToObject;
	int _walkToObjectState;
	bool _redrawSentenceLine;

public:
	ScummEngine_v0(OSystem *syst, const DetectorResult &dr);

	virtual void resetScumm();

protected:
	virtual void resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);

	virtual void setupOpcodes();

	virtual void setupScummVars();
	virtual void resetScummVars();
	virtual void scummLoop(int delta);
	virtual void decodeParseString();

	virtual void processInput();

	virtual void saveOrLoad(Serializer *s);

	virtual bool objIsActor(int obj);
	virtual int objToActor(int obj);
	virtual int actorToObj(int actor);

	// V0 MM Verb commands
	int getVerbPrepId();
	int activeVerbPrep();
	void walkToActorOrObject(int object);
	void verbExec();

	virtual void runSentenceScript();
	virtual void checkAndRunSentenceScript();
	bool checkPendingWalkAction();
	bool checkSentenceComplete();
	virtual void checkExecVerbs();
	virtual void handleMouseOver(bool updateInventory);
	int verbPrepIdType(int verbid);
	void resetVerbs();

	void clearSentenceLine();
	void flushSentenceLine();
	void drawSentenceObject(int object);
	void drawSentenceLine();

	void setMode(byte mode);

	void switchActor(int slot);

	virtual int getVarOrDirectWord(byte mask);
	virtual uint fetchScriptWord();

	virtual int getActiveObject();

	void resetSentence();

	virtual bool areBoxesNeighbors(int box1nr, int box2nr);

	bool ifEqualActiveObject2Common(bool checkType);

	virtual int getCurrentLights() const;

	/* Version 0 script opcodes */
	void o_stopCurrentScript();
	void o_walkActorToObject();
	void o_loadSound();
	void o_getActorMoving();
	void o_animateActor();
	void o_putActorAtObject();
	void o_pickupObject();
	void o_setObjectName();
	void o_lockSound();
	void o_lockCostume();
	void o_loadCostume();
	void o_loadRoom();
	void o_loadRoomWithEgo();
	void o_lockScript();
	void o_loadScript();
	void o_lockRoom();
	void o_setMode();
	void o_lights();
	void o_unlockCostume();
	void o_unlockScript();
	void o_decrement();
	void o_nop();
	void o_getObjectOwner();
	void o_getActorBitVar();
	void o_setActorBitVar();
	void o_getBitVar();
	void o_setBitVar();
	void o_doSentence();
	void o_ifEqualActiveObject2();
	void o_ifNotEqualActiveObject2();
	void o_getClosestActor();
	void o_printEgo();
	void o_print();
	void o_unlockRoom();
	void o_unlockSound();
	void o_cutscene();
	void o_endCutscene();
	void o_setOwnerOf();

	byte VAR_ACTIVE_OBJECT2;
	byte VAR_IS_SOUND_RUNNING;
	byte VAR_ACTIVE_VERB;
};


} // End of namespace Scumm

#endif
