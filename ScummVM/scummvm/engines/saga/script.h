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

// Scripting module private header

#ifndef SAGA_SCRIPT_H
#define SAGA_SCRIPT_H

#include "common/endian.h"

#include "saga/font.h"

namespace Saga {

#define COMMON_BUFFER_SIZE 1024   // Why 1024?

#define SCRIPT_TBLENTRY_LEN 4

#define SCRIPT_MAX 5000

#define ITE_SCRIPT_FUNCTION_MAX 78
#define IHNM_SCRIPT_FUNCTION_MAX 105

enum AddressTypes {
	kAddressCommon     = 0,	// offset from global variables
	kAddressStatic     = 1,	// offset from global variables
	kAddressModule     = 2,	// offset from start of module
	kAddressStack      = 3,	// offset from stack
	kAddressThread     = 4	// offset from thread structure
/*	kAddressId         = 5,	// offset from const id object
	kAddressIdIndirect = 6,	// offset from stack id object
	kAddressIndex      = 7	// index from id*/
};

enum VerbTypes {
	kVerbNone,
	kVerbWalkTo,
	kVerbGive,
	kVerbUse,
	kVerbEnter,
	kVerbLookAt,
	kVerbPickUp,
	kVerbOpen,
	kVerbClose,
	kVerbTalkTo,
	kVerbWalkOnly,
	kVerbLookOnly,
	kVerbOptions
};

#define STHREAD_TIMESLICE 8

enum ThreadVarTypes {
	kThreadVarObject = 0,
	kThreadVarWithObject = 1,
	kThreadVarAction = 2,
	kThreadVarActor = 3,

	kThreadVarMax = kThreadVarActor + 1
};

enum ThreadFlags {
	kTFlagNone = 0,
	kTFlagWaiting = 1,	// wait for even denoted in waitType
	kTFlagFinished = 2,
	kTFlagAborted = 4,
	kTFlagAsleep = kTFlagWaiting | kTFlagFinished | kTFlagAborted	// Combination of all flags which can halt a thread
};

enum ThreadWaitTypes {
	kWaitTypeNone = 0,			// waiting for nothing
	kWaitTypeDelay = 1,			// waiting for a timer
	kWaitTypeSpeech = 2,		// waiting for speech to finish
	kWaitTypeDialogEnd = 3,		// waiting for my dialog to finish
	kWaitTypeDialogBegin = 4,	// waiting for other dialog to finish
	kWaitTypeWalk = 5,			// waiting to finish walking
	kWaitTypeRequest = 6,		// a request is up
	kWaitTypePause = 7,
	kWaitTypePlacard = 8,
	kWaitTypeStatusTextInput = 9,
	kWaitTypeWaitFrames = 10,   // IHNM. waiting for a frame count
	kWaitTypeWakeUp = 11		// IHNM. wait until get waken up
};

enum CycleFlags {
	kCyclePong    = 1 << 0,
	kCycleOnce    = 1 << 1,
	kCycleRandom  = 1 << 2,
	kCycleReverse = 1 << 3
};

enum WalkFlags {
	kWalkBackPedal = 1 << 0,
	kWalkAsync     = 1 << 1,
	kWalkUseAngle  = 1 << 2,
	kWalkFace      = 1 << 5
};

enum ReplyFlags {
	kReplyOnce      = 1 << 0,
	kReplySummary   = 1 << 1,
	kReplyCondition = 1 << 2
};

struct EntryPoint {
	uint16 nameOffset;
	uint16 offset;
};

typedef Common::Array<uint16> VoiceLUT;

struct ModuleData {
	bool loaded;			// is it loaded or not?
	int scriptResourceId;
	int stringsResourceId;
	int voicesResourceId;

	ByteArray moduleBase;	// all base module
	uint16 staticSize;				// size of static data
	uint staticOffset;				// offset of static data begining in _commonBuffer
	Common::Array<EntryPoint> entryPoints;

	StringsTable strings;
	VoiceLUT voiceLUT;

	void clear() {
		loaded = false;
		strings.clear();
		voiceLUT.clear();
		moduleBase.clear();
		entryPoints.clear();
	}

	ModuleData() : loaded(false), scriptResourceId(0), stringsResourceId(0), voicesResourceId(0), staticSize(0), staticOffset(0) {
	}
};

class ScriptThread {
public:
	Common::Array<int16> _stackBuf;

	uint16 _stackTopIndex;
	uint16 _frameIndex;

	uint16 _threadVars[kThreadVarMax];

	byte *_moduleBase;					//
	uint16 _moduleBaseSize;

	byte *_commonBase;					//
	byte *_staticBase;					//
	VoiceLUT *_voiceLUT;				//
	StringsTable *_strings;				//

	int _flags;							// ThreadFlags
	int _waitType;						// ThreadWaitTypes
	uint _sleepTime;
	void *_threadObj;					// which object we're handling

	int16 _returnValue;

	uint16 _instructionOffset;			// Instruction offset

	int32 _frameWait;

	enum {
		THREAD_STACK_SIZE = 256
	};

public:
	byte *baseAddress(byte addrMode) {
		switch (addrMode) {
		case kAddressCommon:
			return _commonBase;
		case kAddressStatic:
			return _staticBase;
		case kAddressModule:
			return _moduleBase;
		case kAddressStack:
			return (byte *)&_stackBuf[_frameIndex];
		case kAddressThread:
			return (byte *)_threadVars;
		default:
			return _commonBase;
		}
	}

	int16 stackTop() {
		return _stackBuf[_stackTopIndex];
	}

	uint pushedSize() {
		return THREAD_STACK_SIZE - _stackTopIndex - 2;
	}

	void push(int16 value) {
		if (_stackTopIndex <= 0) {
			error("ScriptThread::push() stack overflow");
		}
		_stackBuf[--_stackTopIndex] = value;
	}

	int16 pop() {
		if (_stackTopIndex >= THREAD_STACK_SIZE) {
			error("ScriptThread::pop() stack underflow");
		}
		return _stackBuf[_stackTopIndex++];
	}


// wait stuff
	void wait(int waitType) {
		_waitType = waitType;
		_flags |= kTFlagWaiting;
	}

	void waitWalk(void *threadObj) {
		debug(3, "waitWalk()");
		wait(kWaitTypeWalk);
		_threadObj = threadObj;
	}

	void waitDelay(int sleepTime) {
		debug(3, "waitDelay(%d)", sleepTime);
		wait(kWaitTypeDelay);
		_sleepTime = sleepTime;
	}

	void waitFrames(int frames) {
		debug(3, "waitFrames(%d)", frames);
		wait(kWaitTypeWaitFrames);
		_frameWait = frames;
	}

	ScriptThread() {
		memset(&_frameIndex, 0xFE, sizeof(_frameIndex));
		memset(_threadVars, 0xFE, sizeof(_threadVars));
		memset(&_waitType, 0xFE, sizeof(_waitType));
		memset(&_sleepTime, 0xFE, sizeof(_sleepTime));
		memset(&_threadObj, 0xFE, sizeof(_threadObj));
		memset(&_returnValue, 0xFE, sizeof(_threadObj));
		memset(&_frameWait, 0xFE, sizeof(_frameWait));

		_flags = kTFlagNone;
	}
};

typedef Common::List<ScriptThread> ScriptThreadList;

#define SCRIPTOP_PARAMS ScriptThread *thread, Common::SeekableReadStream *scriptS, bool &stopParsing, bool &breakOut
#define SCRIPTFUNC_PARAMS ScriptThread *thread, int nArgs, bool &disContinue
#define OPCODE(x) {&Script::x, #x}

class Script {
public:
	StringsTable _mainStrings;

	Script(SagaEngine *vm);
	virtual ~Script();

	void loadModule(uint scriptModuleNumber);
	void clearModules();

	void doVerb();
	void showVerb(int statusColor = -1);
	void setVerb(int verb);
	int getCurrentVerb() const { return _currentVerb; }
	void setPointerVerb();
	void whichObject(const Point& mousePoint);
	void hitObject(bool leftButton);
	void playfieldClick(const Point& mousePoint, bool leftButton);

	void setLeftButtonVerb(int verb);
	int getLeftButtonVerb() const { return _leftButtonVerb; }
	void setRightButtonVerb(int verb);
	int getRightButtonVerb() const { return _rightButtonVerb; }
	void setNonPlayfieldVerb() {
		setRightButtonVerb(getVerbType(kVerbNone));
		_pointerObject = ID_NOTHING;
		_currentObject[_firstObjectSet ? 1 : 0] = ID_NOTHING;
	}
	void setNoPendingVerb() {
		_pendingVerb = getVerbType(kVerbNone);
		_currentObject[0] = _currentObject[1] = ID_NOTHING;
		setPointerVerb();
	}
	int getVerbType(VerbTypes verbType);
	TextListEntry *getPlacardTextEntry() { return _placardTextEntry; }

	bool isNonInteractiveDemo();

protected:
	// When reading or writing data to the common buffer, we have to use a
	// well-defined byte order since it's stored in savegames. Otherwise,
	// we use native byte ordering since that data may be accessed in other
	// ways than through these functions.
	//
	// The "module" area is a special case, which possibly never ever
	// happens. But if it does, we need well-defined byte ordering.

	uint16 readUint16(byte *addr, byte addrMode) {
		switch (addrMode) {
		case kAddressCommon:
		case kAddressStatic:
		case kAddressModule:
			return READ_LE_UINT16(addr);
		default:
			return READ_UINT16(addr);
		}
	}

	void writeUint16(byte *addr, uint16 value, byte addrMode) {
		switch (addrMode) {
		case kAddressCommon:
		case kAddressStatic:
		case kAddressModule:
			WRITE_LE_UINT16(addr, value);
			break;
		default:
			WRITE_UINT16(addr, value);
			break;
		}
	}

	SagaEngine *_vm;
	ResourceContext *_scriptContext;
	ResourceContext *_dataContext;

	uint16 _modulesLUTEntryLen;
	Common::Array<ModuleData> _modules;
	TextListEntry *_placardTextEntry;

	friend class SagaEngine;
	ByteArray _commonBuffer;

	uint _staticSize;
	ScriptThreadList _threadList;
	ScriptThread *_conversingThread;

//verb
	bool _firstObjectSet;
	bool _secondObjectNeeded;
	uint16 _currentObject[2];
	int16 _currentObjectFlags[2];
	int _currentVerb;
	int _stickyVerb;
	int _leftButtonVerb;
	int _rightButtonVerb;
	int _ihnmDemoCurrentY;

public:
	uint16 _pendingObject[2];
	int _pendingVerb;
	uint16 _pointerObject;

	bool _skipSpeeches;
	bool _abortEnabled;

	VoiceLUT _globalVoiceLUT;

public:
	ScriptThread &createThread(uint16 scriptModuleNumber, uint16 scriptEntryPointNumber);
	int executeThread(ScriptThread *thread, int entrypointNumber);
	void executeThreads(uint msec);
	void completeThread();
	void abortAllThreads();

	void wakeUpActorThread(int waitType, void *threadObj);
	void wakeUpThreads(int waitType);
	void wakeUpThreadsDelayed(int waitType, int sleepTime);

	void loadVoiceLUT(VoiceLUT &voiceLUT, const ByteArray &resourceData);

protected:
	void loadModuleBase(ModuleData &module, const ByteArray &resourceData);

	// runThread returns true if we should break running of other threads
	bool runThread(ScriptThread &thread);
	void setThreadEntrypoint(ScriptThread *thread, int entrypointNumber);

public:
	void finishDialog(int strID, int replyID, int flags, int bitOffset);

protected:
	// Script opcodes ------------------------------------------------------------
	typedef void (Script::*ScriptOpType)(SCRIPTOP_PARAMS);
	struct ScriptOpDescription {
		ScriptOpType scriptOp;
		const char *scriptOpName;
	};
	const ScriptOpDescription *_scriptOpsList;

	void setupScriptOpcodeList();
	void opDummy(SCRIPTOP_PARAMS) { warning("Dummy opcode called"); }
	void opNextBlock(SCRIPTOP_PARAMS) {
		thread->_instructionOffset = (((thread->_instructionOffset) >> 10) + 1) << 10;
	}
	void opDup(SCRIPTOP_PARAMS);
	void opDrop(SCRIPTOP_PARAMS);
	void opZero(SCRIPTOP_PARAMS);
	void opOne(SCRIPTOP_PARAMS);
	void opConstInt(SCRIPTOP_PARAMS);
	void opStrLit(SCRIPTOP_PARAMS);
	void opGetFlag(SCRIPTOP_PARAMS);
	void opGetByte(SCRIPTOP_PARAMS);		// SAGA 2
	void opGetInt(SCRIPTOP_PARAMS);
	void opPutFlag(SCRIPTOP_PARAMS);
	void opPutByte(SCRIPTOP_PARAMS);		// SAGA 2
	void opPutInt(SCRIPTOP_PARAMS);
	void opPutFlagV(SCRIPTOP_PARAMS);
	void opPutByteV(SCRIPTOP_PARAMS);
	void opPutIntV(SCRIPTOP_PARAMS);
	void opCall(SCRIPTOP_PARAMS);			// SAGA 1
	void opCallNear(SCRIPTOP_PARAMS);		// SAGA 2
	void opCallFar(SCRIPTOP_PARAMS);		// SAGA 2
	void opCcall(SCRIPTOP_PARAMS);
	void opCcallV(SCRIPTOP_PARAMS);
	void opCallMember(SCRIPTOP_PARAMS);		// SAGA 2
	void opCallMemberV(SCRIPTOP_PARAMS);	// SAGA 2
	void opEnter(SCRIPTOP_PARAMS);
	void opReturn(SCRIPTOP_PARAMS);
	void opReturnV(SCRIPTOP_PARAMS);
	void opJmp(SCRIPTOP_PARAMS);
	void opJmpTrueV(SCRIPTOP_PARAMS);
	void opJmpFalseV(SCRIPTOP_PARAMS);
	void opJmpTrue(SCRIPTOP_PARAMS);
	void opJmpFalse(SCRIPTOP_PARAMS);
	void opJmpSwitch(SCRIPTOP_PARAMS);
	void opJmpRandom(SCRIPTOP_PARAMS);
	void opNegate(SCRIPTOP_PARAMS);
	void opNot(SCRIPTOP_PARAMS);
	void opCompl(SCRIPTOP_PARAMS);
	void opIncV(SCRIPTOP_PARAMS);
	void opDecV(SCRIPTOP_PARAMS);
	void opPostInc(SCRIPTOP_PARAMS);
	void opPostDec(SCRIPTOP_PARAMS);
	void opAdd(SCRIPTOP_PARAMS);
	void opSub(SCRIPTOP_PARAMS);
	void opMul(SCRIPTOP_PARAMS);
	void opDiv(SCRIPTOP_PARAMS);
	void opMod(SCRIPTOP_PARAMS);
	void opEq(SCRIPTOP_PARAMS);
	void opNe(SCRIPTOP_PARAMS);
	void opGt(SCRIPTOP_PARAMS);
	void opLt(SCRIPTOP_PARAMS);
	void opGe(SCRIPTOP_PARAMS);
	void opLe(SCRIPTOP_PARAMS);
	void opRsh(SCRIPTOP_PARAMS);
	void opLsh(SCRIPTOP_PARAMS);
	void opAnd(SCRIPTOP_PARAMS);
	void opOr(SCRIPTOP_PARAMS);
	void opXor(SCRIPTOP_PARAMS);
	void opLAnd(SCRIPTOP_PARAMS);
	void opLOr(SCRIPTOP_PARAMS);
	void opLXor(SCRIPTOP_PARAMS);
	void opSpeak(SCRIPTOP_PARAMS);
	void opDialogBegin(SCRIPTOP_PARAMS);
	void opDialogEnd(SCRIPTOP_PARAMS);
	void opReply(SCRIPTOP_PARAMS);
	void opAnimate(SCRIPTOP_PARAMS);
	void opJmpSeedRandom(SCRIPTOP_PARAMS);

	// Script functions ----------------------------------------------------------
	typedef void (Script::*ScriptFunctionType)(SCRIPTFUNC_PARAMS);

	struct ScriptFunctionDescription {
		ScriptFunctionType scriptFunction;
		const char *scriptFunctionName;
	};
	const ScriptFunctionDescription *_scriptFunctionsList;

	void setupITEScriptFuncList();
	void setupIHNMScriptFuncList();

	void sfPutString(SCRIPTFUNC_PARAMS);
	void sfWait(SCRIPTFUNC_PARAMS);
	void sfTakeObject(SCRIPTFUNC_PARAMS);
	void sfIsCarried(SCRIPTFUNC_PARAMS);
	void sfStatusBar(SCRIPTFUNC_PARAMS);
	void sfMainMode(SCRIPTFUNC_PARAMS);
	void sfScriptWalkTo(SCRIPTFUNC_PARAMS);
	void sfScriptDoAction(SCRIPTFUNC_PARAMS);
	void sfSetActorFacing(SCRIPTFUNC_PARAMS);
	void sfStartBgdAnim(SCRIPTFUNC_PARAMS);
	void sfStopBgdAnim(SCRIPTFUNC_PARAMS);
	void sfLockUser(SCRIPTFUNC_PARAMS);
	void sfPreDialog(SCRIPTFUNC_PARAMS);
	void sfKillActorThreads(SCRIPTFUNC_PARAMS);
	void sfFaceTowards(SCRIPTFUNC_PARAMS);
	void sfSetFollower(SCRIPTFUNC_PARAMS);
	void sfScriptGotoScene(SCRIPTFUNC_PARAMS);
	void sfSetObjImage(SCRIPTFUNC_PARAMS);
	void sfSetObjName(SCRIPTFUNC_PARAMS);
	void sfGetObjImage(SCRIPTFUNC_PARAMS);
	void sfGetNumber(SCRIPTFUNC_PARAMS);
	void sfScriptOpenDoor(SCRIPTFUNC_PARAMS);
	void sfScriptCloseDoor(SCRIPTFUNC_PARAMS);
	void sfSetBgdAnimSpeed(SCRIPTFUNC_PARAMS);
	void sfCycleColors(SCRIPTFUNC_PARAMS);
	void sfDoCenterActor(SCRIPTFUNC_PARAMS);
	void sfStartBgdAnimSpeed(SCRIPTFUNC_PARAMS);
	void sfScriptWalkToAsync(SCRIPTFUNC_PARAMS);
	void sfEnableZone(SCRIPTFUNC_PARAMS);
	void sfSetActorState(SCRIPTFUNC_PARAMS);
	void sfScriptMoveTo(SCRIPTFUNC_PARAMS);
	void sfSceneEq(SCRIPTFUNC_PARAMS);
	void sfDropObject(SCRIPTFUNC_PARAMS);
	void sfFinishBgdAnim(SCRIPTFUNC_PARAMS);
	void sfSwapActors(SCRIPTFUNC_PARAMS);
	void sfSimulSpeech(SCRIPTFUNC_PARAMS);
	void sfScriptWalk(SCRIPTFUNC_PARAMS);
	void sfCycleFrames(SCRIPTFUNC_PARAMS);
	void sfSetFrame(SCRIPTFUNC_PARAMS);
	void sfSetPortrait(SCRIPTFUNC_PARAMS);
	void sfSetProtagPortrait(SCRIPTFUNC_PARAMS);
	void sfChainBgdAnim(SCRIPTFUNC_PARAMS);
	void sfScriptSpecialWalk(SCRIPTFUNC_PARAMS);
	void sfPlaceActor(SCRIPTFUNC_PARAMS);
	void sfCheckUserInterrupt(SCRIPTFUNC_PARAMS);
	void sfScriptWalkRelative(SCRIPTFUNC_PARAMS);
	void sfScriptMoveRelative(SCRIPTFUNC_PARAMS);
	void sfSimulSpeech2(SCRIPTFUNC_PARAMS);
	void sfPlacard(SCRIPTFUNC_PARAMS);
	void sfPlacardOff(SCRIPTFUNC_PARAMS);
	void sfSetProtagState(SCRIPTFUNC_PARAMS);
	void sfResumeBgdAnim(SCRIPTFUNC_PARAMS);
	void sfThrowActor(SCRIPTFUNC_PARAMS);
	void sfWaitWalk(SCRIPTFUNC_PARAMS);
	void sfScriptSceneID(SCRIPTFUNC_PARAMS);
	void sfChangeActorScene(SCRIPTFUNC_PARAMS);
	void sfScriptClimb(SCRIPTFUNC_PARAMS);
	void sfSetDoorState(SCRIPTFUNC_PARAMS);
	void sfSetActorZ(SCRIPTFUNC_PARAMS);
	void sfScriptText(SCRIPTFUNC_PARAMS);
	void sfGetActorX(SCRIPTFUNC_PARAMS);
	void sfGetActorY(SCRIPTFUNC_PARAMS);
	void sfEraseDelta(SCRIPTFUNC_PARAMS);
	void sfPlayMusic(SCRIPTFUNC_PARAMS);
	void sfPickClimbOutPos(SCRIPTFUNC_PARAMS);
	void sfTossRif(SCRIPTFUNC_PARAMS);
	void sfShowControls(SCRIPTFUNC_PARAMS);
	void sfShowMap(SCRIPTFUNC_PARAMS);
	void sfPuzzleWon(SCRIPTFUNC_PARAMS);
	void sfEnableEscape(SCRIPTFUNC_PARAMS);
	void sfPlaySound(SCRIPTFUNC_PARAMS);
	void sfPlayLoopedSound(SCRIPTFUNC_PARAMS);
	void sfGetDeltaFrame(SCRIPTFUNC_PARAMS);
	void sfShowProtect(SCRIPTFUNC_PARAMS);
	void sfProtectResult(SCRIPTFUNC_PARAMS);
	void sfRand(SCRIPTFUNC_PARAMS);
	void sfFadeMusic(SCRIPTFUNC_PARAMS);
	void sfScriptStartCutAway(SCRIPTFUNC_PARAMS);
	void sfReturnFromCutAway(SCRIPTFUNC_PARAMS);
	void sfEndCutAway(SCRIPTFUNC_PARAMS);
	void sfGetMouseClicks(SCRIPTFUNC_PARAMS);
	void sfResetMouseClicks(SCRIPTFUNC_PARAMS);
	void sfWaitFrames(SCRIPTFUNC_PARAMS);
	void sfScriptFade(SCRIPTFUNC_PARAMS);
	void sfPlayVoice(SCRIPTFUNC_PARAMS);
	void sfVstopFX(SCRIPTFUNC_PARAMS);
	void sfVstopLoopedFX(SCRIPTFUNC_PARAMS);
	void sfDemoIsInteractive(SCRIPTFUNC_PARAMS);
	void sfVsetTrack(SCRIPTFUNC_PARAMS);
	void sfDebugShowData(SCRIPTFUNC_PARAMS);
	void sfNull(SCRIPTFUNC_PARAMS);
	void sfWaitFramesEsc(SCRIPTFUNC_PARAMS);
	void sfPsychicProfile(SCRIPTFUNC_PARAMS);
	void sfPsychicProfileOff(SCRIPTFUNC_PARAMS);
	void sfSetSpeechBox(SCRIPTFUNC_PARAMS);
	void sfSetChapterPoints(SCRIPTFUNC_PARAMS);
	void sfSetPortraitBgColor(SCRIPTFUNC_PARAMS);
	void sfScriptStartVideo(SCRIPTFUNC_PARAMS);
	void sfScriptReturnFromVideo(SCRIPTFUNC_PARAMS);
	void sfScriptEndVideo(SCRIPTFUNC_PARAMS);
	void sfShowIHNMDemoHelpBg(SCRIPTFUNC_PARAMS);
	void sfAddIHNMDemoHelpTextLine(SCRIPTFUNC_PARAMS);
	void sfShowIHNMDemoHelpPage(SCRIPTFUNC_PARAMS);
	void sfGetPoints(SCRIPTFUNC_PARAMS);
	void sfSetGlobalFlag(SCRIPTFUNC_PARAMS);
	void sfDemoSetInteractive(SCRIPTFUNC_PARAMS);
	void sfClearGlobalFlag(SCRIPTFUNC_PARAMS);
	void sfTestGlobalFlag(SCRIPTFUNC_PARAMS);
	void sfSetPoints(SCRIPTFUNC_PARAMS);
	void sfQueueMusic(SCRIPTFUNC_PARAMS);
	void sfDisableAbortSpeeches(SCRIPTFUNC_PARAMS);

	void sfStub(const char *name, ScriptThread *thread, int nArgs);
};

class SAGA1Script : public Script {
public:
	SAGA1Script(SagaEngine *vm);
	~SAGA1Script();
};

class SAGA2Script : public Script {
public:
	SAGA2Script(SagaEngine *vm);
	~SAGA2Script();
};

} // End of namespace Saga

#endif
