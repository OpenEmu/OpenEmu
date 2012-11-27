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
 *
 */

#ifndef TOLTECS_SCRIPT_H
#define TOLTECS_SCRIPT_H

#include "common/func.h"

namespace Toltecs {

const int kMaxScriptSlots = 50;
const int kScriptStackSize = 4096 + 4;

enum VarType {
	vtByte,
	vtWord
};

typedef Common::Functor0<void> ScriptFunction;

class ScriptInterpreter {
public:
	ScriptInterpreter(ToltecsEngine *vm);
	~ScriptInterpreter();

	void loadScript(uint resIndex, uint slotIndex);
	void setMainScript(uint slotIndex);
	void runScript();

	byte *getSlotData(int slotIndex) const { return _slots[slotIndex].data; }

	VarType getGameVarType(uint variable);
	int16 getGameVar(uint variable);
	void setGameVar(uint variable, int16 value);

	void saveState(Common::WriteStream *out);
	void loadState(Common::ReadStream *in);

	void setSwitchLocalDataNear(bool newValue) { _switchLocalDataNear = newValue; }

protected:

	struct ScriptRegs {
		int16 reg0;
		int16 reg1;
		int16 reg2;
		int16 reg3;
		int16 reg4;
		int16 reg5;
		int16 reg6;
		int16 sp;
		int16 reg8;
	};

	struct ScriptSlot {
		byte *data;
		int32 size;
		uint resIndex;
	};

	ToltecsEngine *_vm;
	Common::Array<const ScriptFunction *> _scriptFuncs;
	Common::Array<const char *> _scriptFuncNames;

	byte *_stack;

	byte *_code, *_subCode;
	byte *_localData;
	bool _switchLocalDataNear, _switchLocalDataFar, _switchLocalDataToStack;
	bool _cmpBitTest;

	ScriptSlot _slots[kMaxScriptSlots];

	ScriptRegs _regs;
	int16 _savedSp;

	byte readByte();
	int16 readInt16();

	void execOpcode(byte opcode);

	void setupScriptFunctions();
	void execScriptFunction(uint16 index);

	byte arg8(int16 offset);
	int16 arg16(int16 offset);

	void pushInt16(int16 value);
	int16 popInt16();

	void localWrite8(int16 offset, byte value);
	byte localRead8(int16 offset);
	void localWrite16(int16 offset, int16 value);
	int16 localRead16(int16 offset);
	byte *localPtr(int16 offset);

	void sfNop();
	void sfGetGameVar();
	void sfSetGameVar();
	void sfUpdateScreen();
	void sfGetRandomNumber();
	void sfDrawGuiTextMulti();
	void sfUpdateVerbLine();
	void sfSetFontColor();
	void sfGetTalkTextDuration();
	void sfTalk();
	void sfFindPaletteFragment();
	void sfClearPaletteFragments();
	void sfAddPaletteFragment();
	void sfSetDeltaAnimPalette();
	void sfSetUnkPaletteEffect();
	void sfBuildColorTransTable();
	void sfSetDeltaMainPalette();
	void sfLoadScript();
	void sfRegisterFont();
	void sfLoadAddPalette();
	void sfLoadScene();
	void sfSetGuiHeight();
	void sfFindMouseInRectIndex1();
	void sfFindMouseInRectIndex2();
	void sfDrawGuiImage();
	void sfAddAnimatedSpriteNoLoop();
	void sfAddAnimatedSprite();
	void sfAddStaticSprite();
	void sfAddAnimatedSpriteScaled();
	void sfFindPath();
	void sfWalk();
	void sfScrollCameraUp();
	void sfScrollCameraDown();
	void sfScrollCameraLeft();
	void sfScrollCameraRight();
	void sfScrollCameraUpEx();
	void sfScrollCameraDownEx();
	void sfScrollCameraLeftEx();
	void sfScrollCameraRightEx();
	void sfSetCamera();
	void sfGetCameraChanged();
	void sfGetRgbModifiertAtPoint();
	void sfStartAnim();
	void sfAnimNextFrame();
	void sfGetAnimFrameNumber();
	void sfGetAnimStatus();
	void sfStartShakeScreen();
	void sfStopShakeScreen();
	void sfStartSequence();
	void sfEndSequence();
	void sfSetSequenceVolume();
	void sfPlayPositionalSound();
	void sfPlaySound2();
	void sfClearScreen();
	void sfHandleInput();
	void sfRunOptionsScreen();
	void sfPrecacheSprites();
	void sfPrecacheSounds1();
	void sfDeletePrecachedFiles();
	void sfPrecacheSounds2();
	void sfRestoreStackPtr();
	void sfSaveStackPtr();
	void sfPlayMovie();

};


} // End of namespace Toltecs

#endif /* TOLTECS_H */
