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

#ifndef SCUMM_SCRIPT_V6_H
#define SCUMM_SCRIPT_V6_H

#include "scumm/scumm.h"

namespace Scumm {

class ScummEngine_v6 : public ScummEngine {
protected:
	enum ArrayType {
		kBitArray = 1,
		kNibbleArray = 2,
		kByteArray = 3,
		kStringArray = 4,
		kIntArray = 5,
		kDwordArray = 6
	};

#include "common/pack-start.h"	// START STRUCT PACKING

	struct ArrayHeader {
		int16 dim1;
		int16 type;
		int16 dim2;
		byte data[1];
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

	struct TextObject {
		int16 xpos, ypos;
		byte color;
		byte charset;
		byte text[256];
	};

	/** BlastObjects to draw */
	struct BlastObject {
		uint16 number;
		Common::Rect rect;
		uint16 scaleX, scaleY;
		uint16 image;
		uint16 mode;
	};

	int _blastObjectQueuePos;
	BlastObject _blastObjectQueue[200];

	struct BlastText : TextObject {
		Common::Rect rect;
		bool center;
	};

	int _blastTextQueuePos;
	BlastText _blastTextQueue[50];

	// Akos Class
	struct {
		int16 cmd;
		int16 actor;
		int16 param1;
		int16 param2;
	} _akosQueue[32];
	int16 _akosQueuePos;

	byte _curActor;
	int _curVerb;
	int _curVerbSlot;

	bool _forcedWaitForMessage;
	bool _skipVideo;

public:
	ScummEngine_v6(OSystem *syst, const DetectorResult &dr);

	virtual void resetScumm();

protected:
	virtual void setupOpcodes();

	virtual void scummLoop_handleActors();
	virtual void processKeyboard(Common::KeyState lastKeyHit);

	virtual void setupScummVars();
	virtual void decodeParseString(int a, int b);
	virtual void readArrayFromIndexFile();

	virtual byte *getStringAddress(ResId idx);
	virtual void readMAXS(int blockSize);

	virtual void palManipulateInit(int resID, int start, int end, int time);
	virtual void drawDirtyScreenParts();

	int getStackList(int *args, uint maxnum);
	int popRoomAndObj(int *room);

	ArrayHeader *getArray(int array);
	byte *defineArray(int array, int type, int dim2, int dim1);
	int findFreeArrayId();
	void nukeArray(int array);
	virtual int readArray(int array, int index, int base);
	virtual void writeArray(int array, int index, int base, int value);
	void shuffleArray(int num, int minIdx, int maxIdx);

	virtual void setDefaultCursor();
	void setCursorTransparency(int a);
	void setCursorHotspot(int x, int y);

	virtual void setCursorFromImg(uint img, uint room, uint imgindex);
	void useIm01Cursor(const byte *im, int w, int h);
	void useBompCursor(const byte *im, int w, int h);
	void grabCursor(int x, int y, int w, int h);

	void enqueueText(const byte *text, int x, int y, byte color, byte charset, bool center);
	void drawBlastTexts();
	void removeBlastTexts();

	void enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
	                   int objectHeight, int scaleX, int scaleY, int image, int mode);
	void drawBlastObjects();
	void drawBlastObject(BlastObject *eo);
	void removeBlastObjects();
	void removeBlastObject(BlastObject *eo);

	virtual void clearDrawQueues();

public:
	bool akos_increaseAnims(const byte *akos, Actor *a);
	bool akos_increaseAnim(Actor *a, int i, const byte *aksq, const uint16 *akfo, int numakfo);
protected:
	void akos_queCommand(byte cmd, Actor *a, int param_1, int param_2);
	virtual void akos_processQueue();

	virtual void processActors();

	int getSpecialBox(int x, int y);

	int getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f);

	/* Version 6 script opcodes */
	void o6_setBlastObjectWindow();
	void o6_pushByte();
	void o6_pushWord();
	void o6_pushByteVar();
	void o6_pushWordVar();
	void o6_byteArrayRead();
	void o6_wordArrayRead();
	void o6_byteArrayIndexedRead();
	void o6_wordArrayIndexedRead();
	void o6_dup();
	void o6_pop();
	void o6_not();
	void o6_eq();
	void o6_neq();
	void o6_gt();
	void o6_lt();
	void o6_le();
	void o6_ge();
	void o6_add();
	void o6_sub();
	void o6_mul();
	void o6_div();
	void o6_land();
	void o6_lor();
	void o6_writeByteVar();
	void o6_writeWordVar();
	void o6_byteArrayWrite();
	void o6_wordArrayWrite();
	void o6_byteArrayIndexedWrite();
	void o6_wordArrayIndexedWrite();
	void o6_byteVarInc();
	void o6_wordVarInc();
	void o6_byteArrayInc();
	void o6_wordArrayInc();
	void o6_byteVarDec();
	void o6_wordVarDec();
	void o6_byteArrayDec();
	void o6_wordArrayDec();
	void o6_if();
	void o6_ifNot();
	void o6_jump();
	void o6_startScript();
	void o6_startScriptQuick();
	void o6_startObject();
	void o6_drawObject();
	void o6_drawObjectAt();
	void o6_stopObjectCode();
	void o6_endCutscene();
	void o6_cutscene();
	void o6_stopMusic();
	void o6_freezeUnfreeze();
	void o6_cursorCommand();
	void o6_breakHere();
	void o6_ifClassOfIs();
	void o6_setClass();
	void o6_getState();
	void o6_setState();
	void o6_setOwner();
	void o6_getOwner();
	void o6_startSound();
	void o6_stopSound();
	void o6_startMusic();
	void o6_stopObjectScript();
	void o6_panCameraTo();
	void o6_actorFollowCamera();
	void o6_setCameraAt();
	void o6_loadRoom();
	void o6_stopScript();
	void o6_walkActorToObj();
	void o6_walkActorTo();
	void o6_putActorAtXY();
	void o6_putActorAtObject();
	void o6_faceActor();
	void o6_animateActor();
	void o6_doSentence();
	void o6_pickupObject();
	void o6_loadRoomWithEgo();
	void o6_getRandomNumber();
	void o6_getRandomNumberRange();
	void o6_getActorMoving();
	void o6_isScriptRunning();
	void o6_getActorRoom();
	void o6_getObjectX();
	void o6_getObjectY();
	void o6_getObjectOldDir();
	void o6_getObjectNewDir();
	void o6_getActorWalkBox();
	void o6_getActorCostume();
	void o6_findInventory();
	void o6_getInventoryCount();
	void o6_getVerbFromXY();
	void o6_beginOverride();
	void o6_endOverride();
	void o6_setObjectName();
	void o6_isSoundRunning();
	void o6_setBoxFlags();
	void o6_createBoxMatrix();
	void o6_resourceRoutines();
	void o6_roomOps();
	void o6_actorOps();
	void o6_verbOps();
	void o6_getActorFromXY();
	void o6_findObject();
	void o6_pseudoRoom();
	void o6_getActorElevation();
	void o6_getVerbEntrypoint();
	void o6_arrayOps();
	void o6_saveRestoreVerbs();
	void o6_drawBox();
	void o6_getActorWidth();
	void o6_wait();
	void o6_getActorScaleX();
	void o6_getActorAnimCounter();
	void o6_soundKludge();
	void o6_isAnyOf();
	void o6_systemOps();
	void o6_isActorInBox();
	void o6_delay();
	void o6_delaySeconds();
	void o6_delayMinutes();
	void o6_stopSentence();
	void o6_printLine();
	void o6_printText();
	void o6_printDebug();
	void o6_printSystem();
	void o6_printActor();
	void o6_printEgo();
	void o6_talkActor();
	void o6_talkEgo();
	void o6_dimArray();
	void o6_dummy();
	void o6_startObjectQuick();
	void o6_startScriptQuick2();
	void o6_dim2dimArray();
	void o6_abs();
	void o6_distObjectObject();
	void o6_distObjectPt();
	void o6_distPtPt();
	virtual void o6_kernelSetFunctions();
	void o6_delayFrames();
	void o6_pickOneOf();
	void o6_pickOneOfDefault();
	void o6_jumpToScript();
	void o6_isRoomScriptRunning();
	void o6_kernelGetFunctions();
	void o6_getAnimateVariable();
	void o6_drawBlastObject();
	void o6_getActorLayer();
	void o6_stampObject();
	void o6_bor();
	void o6_band();
	void o6_stopTalking();
	void o6_findAllObjects();
	void o6_pickVarRandom();
	void o6_getDateTime();
	void o6_getPixel();
	void o6_setBoxSet();
	void o6_shuffle();

	byte VAR_VIDEONAME;
	byte VAR_RANDOM_NR;
	byte VAR_STRING2DRAW;

	byte VAR_TIMEDATE_YEAR;
	byte VAR_TIMEDATE_MONTH;
	byte VAR_TIMEDATE_DAY;
	byte VAR_TIMEDATE_HOUR;
	byte VAR_TIMEDATE_MINUTE;
	byte VAR_TIMEDATE_SECOND;
};


} // End of namespace Scumm

#endif
