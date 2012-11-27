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

#ifndef SCUMM_SCRIPT_V2_H
#define SCUMM_SCRIPT_V2_H

#include "scumm/scumm_v3.h"

namespace Scumm {

/**
 * Engine for version 2 SCUMM games.
 */
class ScummEngine_v2 : public ScummEngine_v3old {
protected:
	struct V2MouseoverBox {
		Common::Rect rect;
		byte color;
		byte hicolor;
	};

	V2MouseoverBox _mouseOverBoxesV2[7];
	int8 _mouseOverBoxV2;

	Common::String _sentenceBuf;
	uint16 _inventoryOffset;

public:
	ScummEngine_v2(OSystem *syst, const DetectorResult &dr);

	virtual void resetScumm();

	void checkV2MouseOver(Common::Point pos);
	int checkV2Inventory(int x, int y);
	void redrawV2Inventory();

protected:
	virtual void setupOpcodes();

	virtual void setupScummVars();
	virtual void resetScummVars();
	virtual void decodeParseString();

	virtual void saveOrLoad(Serializer *s);

	virtual void processKeyboard(Common::KeyState lastKeyHit);

	virtual void readIndexFile();
	void readClassicIndexFile();	// V1
	void readEnhancedIndexFile();	// V2
	virtual void readGlobalObjects();
	virtual void loadCharset(int no);

	virtual void runInputScript(int clickArea, int val, int mode);
	virtual void runInventoryScript(int i);

	virtual int getVar();

	void getResultPosIndirect();
	virtual void getResultPos();
	virtual int readVar(uint var);
	virtual void writeVar(uint var, int value);

	virtual int getActiveObject();
	void ifStateCommon(byte type);
	void ifNotStateCommon(byte type);
	void setStateCommon(byte type);
	void clearStateCommon(byte type);
	void stopScriptCommon(int script);

	void resetSentence();
	void setUserState(byte state);

	virtual void handleMouseOver(bool updateInventory);
	virtual void checkExecVerbs();
	void initV2MouseOver();
	void initNESMouseOver();

	virtual void setBuiltinCursor(int index);

	void drawPreposition(int index);

	void walkActorToObject(int actor, int obj);

	/* Version 2 script opcodes */
	void o2_actorFromPos();
	void o2_actorOps();
	void o2_add();
	void o2_addIndirect();
	void o2_assignVarByte();
	void o2_assignVarWordIndirect();
	void o2_beginOverride();
	void o2_chainScript();
	void o2_clearState01();
	void o2_clearState02();
	void o2_clearState04();
	void o2_clearState08();
	void o2_cursorCommand();
	void o2_cutscene();
	void o2_delay();
	void o2_doSentence();
	void o2_drawObject();
	void o2_drawSentence();
	void o2_dummy();
	void o2_endCutscene();
	void o2_findObject();
	void o2_getActorWalkBox();
	void o2_getActorX();
	void o2_getActorY();
	void o2_getBitVar();
	void o2_getObjPreposition();
	void o2_ifClassOfIs();
	void o2_ifNotState01();
	void o2_ifNotState02();
	void o2_ifNotState04();
	void o2_ifNotState08();
	void o2_ifState01();
	void o2_ifState02();
	void o2_ifState04();
	void o2_ifState08();
	void o2_isGreater();
	void o2_isGreaterEqual();
	void o2_isLess();
	void o2_isLessEqual();
	void o2_lights();
	void o2_loadRoomWithEgo();
	void o2_panCameraTo();
	void o2_pickupObject();
	void o2_putActor();
	void o2_putActorAtObject();
	void o2_putActorInRoom();
	void o2_resourceRoutines();
	void o2_restart();
	void o2_roomOps();
	void o2_getActorElevation();
	void o2_setActorElevation();
	void o2_setBitVar();
	void o2_setCameraAt();
	void o2_setObjPreposition();
	void o2_setOwnerOf();
	void o2_setState01();
	void o2_setState02();
	void o2_setState04();
	void o2_setState08();
	void o2_startScript();
	void o2_stopScript();
	void o2_subtract();
	void o2_subIndirect();
	void o2_switchCostumeSet();
	void o2_verbOps();
	void o2_waitForActor();
	void o2_waitForMessage();
	void o2_waitForSentence();
	void o2_walkActorTo();
	void o2_walkActorToObject();

	byte VAR_SENTENCE_VERB;
	byte VAR_SENTENCE_OBJECT1;
	byte VAR_SENTENCE_OBJECT2;
	byte VAR_SENTENCE_PREPOSITION;
	byte VAR_BACKUP_VERB;

	byte VAR_CLICK_AREA;
	byte VAR_CLICK_VERB;
	byte VAR_CLICK_OBJECT;
};


} // End of namespace Scumm

#endif
