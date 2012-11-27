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

#ifndef DRACI_SCRIPT_H
#define DRACI_SCRIPT_H

#include "common/array.h"
#include "common/stream.h"

namespace Common {
class ReadStream;
}

namespace Draci {

enum {
	/** The maximum number of parameters for a GPL command */
	kMaxParams = 3,

	kNumCommands = 55
};

class DraciEngine;
class Script;

typedef void (Script::*GPLHandler)(const Common::Array<int> &);
typedef int  (Script::*GPLOperatorHandler)(int, int) const;
typedef int  (Script::*GPLFunctionHandler)(int) const;

/**
 * Represents a single command in the GPL scripting language bytecode.
 * Each command is represented in the bytecode by a command number and a
 * subnumber.
 */
enum GPL2ParameterType {
	kGPL2Num = 1,
	kGPL2Str = 2,
	kGPL2Ident = 3,
	kGPL2Math = 4
};

struct GPL2Command {
	byte _number;
	byte _subNumber;
	const char *_name;
	uint16 _numParams;
	GPL2ParameterType _paramTypes[kMaxParams];
	GPLHandler _handler;
};

struct GPL2Operator {
	GPLOperatorHandler _handler;
	const char *_name;
};

struct GPL2Function {
	GPLFunctionHandler _handler;
	const char *_name;
};

/**
 *  A convenience data type that holds both the actual bytecode and the
 *  length of the bytecode. Passed to Script::run().
 */
struct GPL2Program {
	GPL2Program() : _bytecode(NULL), _length(0) {}

	byte *_bytecode;
	uint16 _length;
};

class Animation;
class GameObject;

class Script {

public:
	Script(DraciEngine *vm) : _vm(vm), _jump(0), _endProgram(false) { setupCommandList(); }

	void run(const GPL2Program &program, uint16 offset);
	void runWrapper(const GPL2Program &program, uint16 offset, bool disableCursor, bool releaseAnims);
	bool testExpression(const GPL2Program &program, uint16 offset) const;
	void endCurrentProgram(bool value) { _endProgram = value; }
	bool shouldEndProgram() const { return _endProgram; }

private:
	int _jump;
	bool _endProgram;

	/** List of all GPL commands. Initialized in the constructor. */
	const GPL2Command *_commandList;
	const GPL2Operator *_operatorList;
	const GPL2Function *_functionList;

	void c_If(const Common::Array<int> &params);
	void c_Goto(const Common::Array<int> &params);
	void c_Let(const Common::Array<int> &params);
	void load(const Common::Array<int> &params);
	void start(const Common::Array<int> &params);
	void loadMusic(const Common::Array<int> &params);
	void startMusic(const Common::Array<int> &params);
	void stopMusic(const Common::Array<int> &params);
	void mark(const Common::Array<int> &params);
	void release(const Common::Array<int> &params);
	void icoStat(const Common::Array<int> &params);
	void objStat(const Common::Array<int> &params);
	void objStatOn(const Common::Array<int> &params);
	void execInit(const Common::Array<int> &params);
	void execLook(const Common::Array<int> &params);
	void execUse(const Common::Array<int> &params);
	void stayOn(const Common::Array<int> &params);
	void walkOn(const Common::Array<int> &params);
	void walkOnPlay(const Common::Array<int> &params);
	void play(const Common::Array<int> &params);
	void startPlay(const Common::Array<int> &params);
	void justTalk(const Common::Array<int> &params);
	void justStay(const Common::Array<int> &params);
	void newRoom(const Common::Array<int> &params);
	void talk(const Common::Array<int> &params);
	void loadMap(const Common::Array<int> &params);
	void roomMap(const Common::Array<int> &params);
	void disableQuickHero(const Common::Array<int> &params);
	void enableQuickHero(const Common::Array<int> &params);
	void disableSpeedText(const Common::Array<int> &params);
	void enableSpeedText(const Common::Array<int> &params);
	void dialogue(const Common::Array<int> &params);
	void exitDialogue(const Common::Array<int> &params);
	void resetDialogue(const Common::Array<int> &params);
	void resetDialogueFrom(const Common::Array<int> &params);
	void resetBlock(const Common::Array<int> &params);
	void setPalette(const Common::Array<int> &params);
	void blackPalette(const Common::Array<int> &params);
	void fadePalette(const Common::Array<int> &params);
	void fadePalettePlay(const Common::Array<int> &params);
	void loadPalette(const Common::Array<int> &params);
	void quitGame(const Common::Array<int> &params);
	void pushNewRoom(const Common::Array<int> &params);
	void popNewRoom(const Common::Array<int> &params);

	int operAnd(int op1, int op2) const;
	int operOr(int op1, int op2) const;
	int operXor(int op1, int op2) const;
	int operSub(int op1, int op2) const;
	int operAdd(int op1, int op2) const;
	int operDiv(int op1, int op2) const;
	int operMul(int op1, int op2) const;
	int operEqual(int op1, int op2) const;
	int operNotEqual(int op1, int op2) const;
	int operGreater(int op1, int op2) const;
	int operLess(int op1, int op2) const;
	int operGreaterOrEqual(int op1, int op2) const;
	int operLessOrEqual(int op1, int op2) const;
	int operMod(int op1, int op2) const;

	int funcRandom(int n) const;
	int funcNot(int n) const;
	int funcIsIcoOn(int iconID) const;
	int funcIcoStat(int iconID) const;
	int funcActIco(int iconID) const;
	int funcIsIcoAct(int iconID) const;
	int funcIsObjOn(int objID) const;
	int funcIsObjOff(int objID) const;
	int funcIsObjAway(int objID) const;
	int funcActPhase(int objID) const;
	int funcObjStat(int objID) const;
	int funcLastBlock(int blockID) const;
	int funcAtBegin(int yesno) const;
	int funcBlockVar(int blockID) const;
	int funcHasBeen(int blockID) const;
	int funcMaxLine(int lines) const;

	void setupCommandList();
	const GPL2Command *findCommand(byte num, byte subnum) const;
	int handleMathExpression(Common::ReadStream *reader) const;

	DraciEngine *_vm;
};

} // End of namespace Draci

#endif // DRACI_SCRIPT_H
