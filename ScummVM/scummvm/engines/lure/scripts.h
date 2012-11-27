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

#ifndef LURE_SCRIPTS_H
#define LURE_SCRIPTS_H

#include "lure/luredefs.h"
#include "lure/memory.h"
#include "lure/hotspots.h"

namespace Lure {

// Opcode list
#define S_OPCODE_ABORT 0
#define S_OPCODE_ADD 1
#define S_OPCODE_SUBTRACT 2
#define S_OPCODE_MULTIPLY 3
#define S_OPCODE_DIVIDE 4
#define S_OPCODE_EQUALS 5
#define S_OPCODE_NOT_EQUALS 6
#define S_OPCODE_LT 7
#define S_OPCODE_GT 8
#define S_OPCODE_LTE 9
#define S_OPCODE_GTE 10
#define S_OPCODE_AND 11
#define S_OPCODE_OR 12
#define S_OPCODE_LOGICAL_AND 13
#define S_OPCODE_LOGICAL_OR 14
#define S_OPCODE_GET_FIELD 15
#define S_OPCODE_SET_FIELD 16
#define S_OPCODE_PUSH 17
#define S_OPCODE_SUBROUTINE 18
#define S_OPCODE_EXEC 19
#define S_OPCODE_END 20
#define S_OPCODE_COND_JUMP 21
#define S_OPCODE_JUMP 22
#define S_OPCODE_ABORT2 23
#define S_OPCODE_ABORT3 24
#define S_OPCODE_RANDOM 25

#define S2_OPCODE_FRAME_CTR -1
#define S2_OPCODE_POSITION -2
#define S2_OPCODE_CHANGE_POS -3
#define S2_OPCODE_UNLOAD -4
#define S2_OPCODE_DIMENSIONS -5
#define S2_OPCODE_JUMP -6
#define S2_OPCODE_ANIMATION -7
#define S2_OPCODE_PLAY_SOUND -8
#define S2_OPCODE_STOP_SOUND -9
#define S2_OPCODE_ACTIONS -10



class Script {
public:
	static uint16 execute(uint16 startOffset);

	static void executeMethod(int methodIndex, uint16 v1, uint16 v2, uint16 v3);
	static void activateHotspot(uint16 hotspotId, uint16 v2, uint16 v3);
	static void setHotspotScript(uint16 hotspotId, uint16 scriptIndex, uint16 v3);
	static void addSound2(uint16 soundIndex, uint16 v2, uint16 v3);
	static void setHotspotFlagMask(uint16 maskVal, uint16 v2, uint16 v3);
	static void clearSequenceDelayList(uint16 v1, uint16 scriptIndex, uint16 v3);
	static void deactivateHotspotSet(uint16 listIndex, uint16 v2, uint16 v3);
	static void deactivateHotspot(uint16 hotspotId, uint16 v2, uint16 v3);
	static void resetPathfinder(uint16 v1, uint16 v2, uint16 v3);
	static void addDelayedSequence(uint16 seqOffset, uint16 delay, uint16 canClear);
	static void killSound(uint16 soundNumber, uint16 v2, uint16 v3);
	static void characterInRoom(uint16 characterId, uint16 roomNumber, uint16 v3);
	static void setDesc(uint16 hotspotId, uint16 descId, uint16 v3);
	static void setHotspotName(uint16 hotspotId, uint16 nameId, uint16 v3);
	static void addSound(uint16 soundIndex, uint16 v2, uint16 v3);
	static void endgameSequence(uint16 v1, uint16 v2, uint16 v3);
	static void setupPigFight(uint16 v1, uint16 v2, uint16 v3);
	static void displayDialog(uint16 stringId, uint16 v2, uint16 v3);
	static void setupSkorlFight(uint16 v1, uint16 v2, uint16 v3);
	static void remoteRoomViewSetup(uint16 v1, uint16 v2, uint16 v3);
	static void startSpeakingToNoone(uint16 characterId, uint16 stringId, uint16 v3);
	static void stopSound(uint16 soundIndex, uint16 v2, uint16 v3);
	static void getDoorBlocked(uint16 hotspotId, uint16 v2, uint16 v3);
	static void isSkorlInCell(uint16 v1, uint16 v2, uint16 v3);
	static void ratpouchPushBricks(uint16 v1, uint16 v2, uint16 v3);
	static void characterChangeRoom(uint16 y, uint16 x, uint16 roomNumber);
	static void pauseRatpouch(uint16 v1, uint16 v2, uint16 v3);
	static void setBlockingHotspotScript(uint16 charId, uint16 scriptIndex, uint16 v3);
	static void decrInventoryItems(uint16 v1, uint16 v2, uint16 v3);
	static void setTalking(uint16 characterId, uint16 destHotspot, uint16 messageId);
	static void setActionCtr(uint16 hotspotId, uint16 value, uint16 v3);
	static void startSpeaking(uint16 characterId, uint16 destHotspot, uint16 messageId);
	static void disableHotspot(uint16 hotspotId, uint16 v2, uint16 v3);
	static void cutSack(uint16 hotspotId, uint16 v2, uint16 v3);
	static void increaseNumGroats(uint16 characterId, uint16 numGroats, uint16 v3);
	static void enableHotspot(uint16 hotspotId, uint16 v2, uint16 v3);
	static void displayMessage2(uint16 messageId, uint16 hotspotId, uint16 v3);
	static void startOilBurner(uint16 v1, uint16 v2, uint16 v3);
	static void transformPlayer(uint16 v1, uint16 v2, uint16 v3);
	static void townHallClose(uint16 v1, uint16 v2, uint16 v3);
	static void checkRoomNumber(uint16 hotspotId, uint16 roomNumber, uint16 v3);
	static void makeGoewinFollow(uint16 v1, uint16 v2, uint16 v3);
	static void doorClose(uint16 hotspotId, uint16 v2, uint16 v3);
	static void fixGoewin(uint16 v1, uint16 v2, uint16 v3);
	static void displayMessage(uint16 messageId, uint16 characterId, uint16 destCharacterId);
	static void doorOpen(uint16 hotspotId, uint16 v2, uint16 v3);
	static void npcWait(uint16 hotspotId, uint16 v2, uint16 v3);
	static void checkWakeBrenda(uint16 v1, uint16 v2, uint16 v3);
	static void setNewSupportData(uint16 index, uint16 hotspotId, uint16 v3);
	static void setSupportData(uint16 hotspotId, uint16 index, uint16 v3);
	static void givePlayerItem(uint16 hotspotId, uint16 v2, uint16 v3);
	static void decreaseNumGroats(uint16 characterId, uint16 numGroats, uint16 v3);
	static void makeGoewinWork(uint16 v1, uint16 v2, uint16 v3);
	static void moveCharacterToPlayer(uint16 characterId, uint16 v2, uint16 v3);
	static void setVillageSkorlTickProc(uint16 v1, uint16 v2, uint16 v3);
	static void freeGoewin(uint16 v1, uint16 v2, uint16 v3);
	static void barmanServe(uint16 v1, uint16 v2, uint16 v3);
	static void getNumGroats(uint16 v1, uint16 v2, uint16 v3);
	static void checkHasBook(uint16 v1, uint16 v2, uint16 v3);
	static void enableGargoylesTalk(uint16 v1, uint16 v2, uint16 v3);
	static void killPlayer(uint16 v1, uint16 v2, uint16 v3);
	static void normalGoewin(uint16 v1, uint16 v2, uint16 v3);
	static void animationLoad(uint16 hotspotId, uint16 v2, uint16 v3);
	static void addActions(uint16 hotspotId, uint16 actions, uint16 v3);
	static void randomToGeneral(uint16 maxVal, uint16 minVal, uint16 v3);
	static void checkCellDoor(uint16 v1, uint16 v2, uint16 v3);
	static void checkSound(uint16 soundNumber, uint16 v2, uint16 v3);
};

class HotspotScript {
private:
	static int16 nextVal(MemoryBlock *data, uint16 &offset);
public:
	static bool execute(Hotspot *h);
};

} // End of namespace Lure

#endif
