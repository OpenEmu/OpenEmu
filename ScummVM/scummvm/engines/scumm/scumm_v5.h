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

#ifndef SCUMM_SCRIPT_V5_H
#define SCUMM_SCRIPT_V5_H

#include "scumm/scumm.h"

namespace Scumm {

class ScummEngine_v5 : public ScummEngine {
protected:
	uint16 _cursorImages[4][17];
	byte _cursorHotspots[2 * 4];

	struct {
		int x, y, w, h;
		byte *buffer;
		uint16 xStrips, yStrips;
		bool isDrawn;
	} _flashlight;

	char _saveLoadVarsFilename[256];

	enum {
		PARAM_1 = 0x80,
		PARAM_2 = 0x40,
		PARAM_3 = 0x20
	};

	int _resultVarNumber;

public:
	ScummEngine_v5(OSystem *syst, const DetectorResult &dr);

	void clearFlashlight();

	virtual void resetCursors();

protected:
	virtual void setupOpcodes();

	virtual void scummLoop_handleActors();
	virtual void scummLoop_handleSaveLoad();

	virtual void setupScummVars();
	virtual void resetScummVars();
	virtual void decodeParseString();

	virtual void saveOrLoad(Serializer *s);

	virtual void readMAXS(int blockSize);

	int getWordVararg(int *ptr);

	virtual int getVar();
	virtual int getVarOrDirectByte(byte mask);
	virtual int getVarOrDirectWord(byte mask);

	virtual void getResultPos();
	void setResult(int result);

	virtual void animateCursor();

	virtual void setBuiltinCursor(int index);
	void redefineBuiltinCursorFromChar(int index, int chr);
	void redefineBuiltinCursorHotspot(int index, int x, int y);

	void drawFlashlight();

	void walkActorToActor(int actor, int toActor, int dist);

	/**
	 * Fetch the next script word, then if cond is *false*, perform a relative jump.
	 * So this corresponds to a "jne" jump instruction.
	 */
	void jumpRelative(bool cond);

	/* Version 5 script opcodes */
	void o5_actorFollowCamera();
	void o5_actorFromPos();
	void o5_actorOps();
	void o5_add();
	void o5_and();
	void o5_animateActor();
	void o5_breakHere();
	void o5_chainScript();
	void o5_cursorCommand();
	void o5_cutscene();
	void o5_debug();
	void o5_decrement();
	void o5_delay();
	void o5_delayVariable();
	void o5_divide();
	void o5_doSentence();
	void o5_drawBox();
	void o5_drawObject();
	void o5_dummy();
	void o5_endCutscene();
	void o5_equalZero();
	void o5_expression();
	void o5_faceActor();
	void o5_findInventory();
	void o5_findObject();
	void o5_freezeScripts();
	void o5_getActorCostume();
	void o5_getActorElevation();
	void o5_getActorFacing();
	void o5_getActorMoving();
	void o5_getActorRoom();
	void o5_getActorScale();
	void o5_getActorWalkBox();
	void o5_getActorWidth();
	void o5_getActorX();
	void o5_getActorY();
	void o5_getAnimCounter();
	void o5_getClosestObjActor();
	void o5_getDist();
	void o5_getInventoryCount();
	void o5_getObjectOwner();
	void o5_getObjectState();
	void o5_getRandomNr();
	void o5_getStringWidth();
	void o5_getVerbEntrypoint();
	void o5_ifClassOfIs();
	void o5_increment();
	void o5_isActorInBox();
	void o5_isEqual();
	void o5_isGreater();
	void o5_isGreaterEqual();
	void o5_isLess();
	void o5_isLessEqual();
	void o5_isNotEqual();
	void o5_isScriptRunning();
	void o5_isSoundRunning();
	void o5_jumpRelative();
	void o5_lights();
	void o5_loadRoom();
	void o5_loadRoomWithEgo();
	void o5_matrixOps();
	void o5_move();
	void o5_multiply();
	void o5_notEqualZero();
	void o5_or();
	void o5_beginOverride();
	void o5_panCameraTo();
	void o5_pickupObject();
	void o5_print();
	void o5_printEgo();
	void o5_pseudoRoom();
	void o5_putActor();
	void o5_putActorAtObject();
	void o5_putActorInRoom();
	void o5_systemOps();
	void o5_resourceRoutines();
	void o5_roomOps();
	void o5_saveRestoreVerbs();
	void o5_setCameraAt();
	void o5_setClass();
	void o5_setObjectName();
	void o5_setOwnerOf();
	void o5_setState();
	void o5_setVarRange();
	void o5_soundKludge();
	void o5_startMusic();
	void o5_startObject();
	void o5_startScript();
	void o5_startSound();
	void o5_stopMusic();
	void o5_stopObjectCode();
	void o5_stopObjectScript();
	void o5_stopScript();
	void o5_stopSound();
	void o5_stringOps();
	void o5_subtract();
	void o5_verbOps();
	void o5_wait();
	void o5_walkActorTo();
	void o5_walkActorToActor();
	void o5_walkActorToObject();
};


} // End of namespace Scumm

#endif
