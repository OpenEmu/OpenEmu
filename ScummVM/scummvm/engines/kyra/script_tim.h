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

#ifndef KYRA_SCRIPT_TIM_H
#define KYRA_SCRIPT_TIM_H

#include "kyra/kyra_v1.h"

#include "common/array.h"
#include "common/func.h"
#include "common/str.h"

namespace Kyra {

class WSAMovie_v2;
class Screen_v2;
class Movie;
class LoLEngine;

class TimAnimator {
public:
	struct AnimPart {
		uint16 firstFrame;
		uint16 lastFrame;
		uint16 cycles;
		int16 nextPart;
		int16 partDelay;
		uint16 field_A;
		int16 sfxIndex;
		uint16 sfxFrame;
	};

	struct Animation {
		Movie *wsa;
		int16 x, y;
		uint32 nextFrame;
		uint8 enable;
		uint8 field_D;
		uint8 frameDelay;
		int8 curPart;
		uint8 curFrame;
		uint8 cyclesCompleted;
		uint16 wsaCopyParams;
		int8 lastPart;
		AnimPart *parts;
	};

#ifdef ENABLE_LOL
	TimAnimator(LoLEngine *engine, Screen_v2 *screen_v2, OSystem *system, bool useParts);
#else
	TimAnimator(KyraEngine_v1 *engine, Screen_v2 *screen_v2, OSystem *system, bool useParts);
#endif
	~TimAnimator();

	void init(int animIndex, Movie *wsa, int x, int y, int wsaCopyParams, int frameDelay);
	void reset(int animIndex, bool clearStruct);

	void displayFrame(int animIndex, int page, int frame, int flags = -1);

	const Movie *getWsaCPtr(int animIndex) { return (animIndex >= 0 && animIndex < 6) ? _animations[animIndex].wsa : 0; }
	int getAnimX(int animIndex) { return (animIndex >= 0 && animIndex < 6) ? _animations[animIndex].x : 0; }
	int getAnimY(int animIndex) { return (animIndex >= 0 && animIndex < 6) ? _animations[animIndex].y : 0; }

#ifdef ENABLE_LOL
	void setupPart(int animIndex, int part, int firstFrame, int lastFrame, int cycles, int nextPart, int partDelay, int f, int sfxIndex, int sfxFrame);
	void start(int animIndex, int part);
	void stop(int animIndex);
	void update(int animIndex);
	void playPart(int animIndex, int firstFrame, int lastFrame, int delay);
	int resetLastPart(int animIndex);
#endif

private:
#ifdef ENABLE_LOL
	LoLEngine *_vm;
#else
	KyraEngine_v1 *_vm;
#endif
	Screen_v2 *_screen;
	OSystem *_system;

	Animation *_animations;

	const bool _useParts;
};

struct TIM;
typedef Common::Functor2<const TIM *, const uint16 *, int> TIMOpcode;

struct TIM {
	char filename[13];

	uint16 clickedButton;
	int16 dlgFunc;

	int16 procFunc;
	uint16 procParam;

	enum {
		kCountFuncs = 10
	};

	struct Function {
		uint16 *ip;

		uint32 lastTime;
		uint32 nextTime;

		uint16 *loopIp;

		uint16 *avtl;
	} func[kCountFuncs];

	enum {
		kWSASlots = 6,
		kAnimParts = 10
	};

	struct WSASlot {
		int anim;

		int16 x, y;
		uint16 wsaFlags;
		uint16 offscreen;
	} wsa[kWSASlots];

	uint16 *avtl;
	uint8 *text;

	const Common::Array<const TIMOpcode *> *opcodes;

	// TODO: Get rid of this ugly HACK to allow the
	// Lands of Lore outro to be working properly.
	bool isLoLOutro;
	uint8 lolCharacter;
};

class TIMInterpreter {
public:
	TIMInterpreter(KyraEngine_v1 *engine, Screen_v2 *screen_v2, OSystem *system);
	virtual ~TIMInterpreter();

	TIM *load(const char *filename, const Common::Array<const TIMOpcode *> *opcodes);
	void unload(TIM *&tim) const;

	bool callback(Common::IFFChunk &chunk);

	virtual int initAnimStruct(int index, const char *filename, int x, int y, int, int offscreenBuffer, uint16 wsaFlags);
	virtual int freeAnimStruct(int index);
	TimAnimator *animator() { return _animator; }

	void setLangData(const char *filename);
	void clearLangData() { delete[] _langData; _langData = 0; }

	const char *getCTableEntry(uint idx) const;

	void resetFinishedFlag() { _finished = false; }
	bool finished() const { return _finished; }

	int exec(TIM *tim, bool loop);
	void stopCurFunc() { if (_currentTim) cmd_stopCurFunc(0); }
	void stopAllFuncs(TIM *tim);

	void refreshTimersAfterPause(uint32 elapsedTime);

	void displayText(uint16 textId, int16 flags);
	void displayText(uint16 textId, int16 flags, uint8 color);
	void setupTextPalette(uint index, int fadePalette);

	virtual void resetDialogueState(TIM *tim) {}

	int _drawPage2;

	int _palDelayInc, _palDiff, _palDelayAcc;
	int _abortFlag;

protected:
	KyraEngine_v1 *_vm;
	Screen_v2 *_screen;
	OSystem *_system;

	TIM *_currentTim;
	int _currentFunc;

	TimAnimator *_animator;

	bool _finished;

	// used when loading
	int _avtlChunkSize;
	const char *_filename;
	TIM *_tim;

	Common::String _vocFiles[120];

	virtual void update() {}
	virtual void checkSpeechProgress() {}

	char _audioFilename[32];

	uint8 *_langData;
	char *getTableEntry(uint idx);
	bool _textDisplayed;
	uint8 *_textAreaBuffer;

	virtual int execCommand(int cmd, const uint16 *param);

	typedef int (TIMInterpreter::*CommandProc)(const uint16 *);
	struct CommandEntry {
		CommandProc proc;
		const char *desc;
	};

	const CommandEntry *_commands;
	int _commandsSize;

	int cmd_initFunc0(const uint16 *param);
	int cmd_stopCurFunc(const uint16 *param);
	int cmd_initWSA(const uint16 *param);
	int cmd_uninitWSA(const uint16 *param);
	int cmd_initFunc(const uint16 *param);
	int cmd_stopFunc(const uint16 *param);
	int cmd_wsaDisplayFrame(const uint16 *param);
	int cmd_displayText(const uint16 *param);
	int cmd_loadVocFile(const uint16 *param);
	int cmd_unloadVocFile(const uint16 *param);
	int cmd_playVocFile(const uint16 *param);
	int cmd_loadSoundFile(const uint16 *param);
	int cmd_playMusicTrack(const uint16 *param);
	virtual int cmd_setLoopIp(const uint16 *param);
	virtual int cmd_continueLoop(const uint16 *param);
	int cmd_resetLoopIp(const uint16 *param);
	int cmd_resetAllRuntimes(const uint16 *param);
	int cmd_execOpcode(const uint16 *param);
	int cmd_initFuncNow(const uint16 *param);
	int cmd_stopFuncNow(const uint16 *param);
#define cmd_return(n, v) \
	int cmd_return_##n(const uint16 *){ return v; }
	cmd_return( 1,  1)
	cmd_return(n1, -1)
#undef cmd_return
};

#ifdef ENABLE_LOL
class LoLEngine;
class Screen_LoL;
class TIMInterpreter_LoL : public TIMInterpreter {
public:
	TIMInterpreter_LoL(LoLEngine *engine, Screen_v2 *screen_v2, OSystem *system);

	int initAnimStruct(int index, const char *filename, int x, int y, int frameDelay, int, uint16 wsaCopyParams);
	int freeAnimStruct(int index);

	void resetDialogueState(TIM *tim);

private:
	void update();
	void checkSpeechProgress();

	char *getTableString(int id);
	void advanceToOpcode(int opcode);

	LoLEngine *_vm;
	Screen_LoL *_screen;

	virtual int execCommand(int cmd, const uint16 *param);

	typedef int (TIMInterpreter_LoL::*CommandProc)(const uint16 *);
	struct CommandEntry {
		CommandProc proc;
		const char *desc;
	};

	const CommandEntry *_commands;
	int _commandsSize;

	int cmd_stopAllFuncs(const uint16 *param);
	int cmd_setLoopIp(const uint16 *param);
	int cmd_continueLoop(const uint16 *param);
	int cmd_processDialogue(const uint16 *param);
	int cmd_dialogueBox(const uint16 *param);
};
#endif // ENABLE_LOL

} // End of namespace Kyra

#endif
