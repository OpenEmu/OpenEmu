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

#ifndef CINE_SCRIPT_H
#define CINE_SCRIPT_H

#include "common/savefile.h"
#include "common/array.h"
#include "common/list.h"
#include "common/ptr.h"

namespace Cine {

#define SCRIPT_STACK_SIZE 50
#define LOCAL_VARS_SIZE 50

/**
 * Fixed size array of script variables.
 *
 * Array size can be set in constructors, once the instance is created,
 * it cannot be changed directly.
 */

class FWScript;

typedef int (FWScript::*OpFunc)();

struct Opcode {
	OpFunc proc;
	const char *args;
};

/**
 *  Fixed size array for script variables
 */
class ScriptVars {
private:
	unsigned int _size; ///< Size of array
	int16 *_vars; ///< Variable values

public:
	// Explicit to prevent var=0 instead of var[i]=0 typos.
	explicit ScriptVars(unsigned int len = 50);
	ScriptVars(Common::SeekableReadStream &fHandle, unsigned int len = 50);
	ScriptVars(const ScriptVars &src);
	~ScriptVars();

	void reinit(unsigned int len);
	ScriptVars &operator=(const ScriptVars &src);
	int16 &operator[](unsigned int idx);
	int16 operator[](unsigned int idx) const;

	void save(Common::OutSaveFile &fHandle) const;
	void save(Common::OutSaveFile &fHandle, unsigned int len) const;
	void load(Common::SeekableReadStream &fHandle);
	void load(Common::SeekableReadStream &fHandle, unsigned int len);
	void reset();
};

class FWScriptInfo;

/**
 *  Script bytecode and initial labels, ScriptStruct replacement.
 *
 * _data is one byte longer to make sure strings in bytecode are properly
 * terminated
 */
class RawScript {
private:
	byte *_data; ///< Script bytecode
	ScriptVars _labels; ///< Initial script labels

protected:
	void computeLabels(const FWScriptInfo &info);
	int getNextLabel(const FWScriptInfo &info, int offset) const;

public:
	uint16 _size; ///< Bytecode length

	explicit RawScript(uint16 size);
	RawScript(const FWScriptInfo &info, const byte *data, uint16 size);
	RawScript(const RawScript &src);
	~RawScript();

	RawScript &operator=(const RawScript &src);

	void setData(const FWScriptInfo &info, const byte *data);
	const ScriptVars &labels() const;
	byte getByte(unsigned int pos) const;
	uint16 getWord(unsigned int pos) const;
	const char *getString(unsigned int pos) const;
	uint16 getLabel(const FWScriptInfo &info, byte index, uint16 offset) const;
};

/**
 * Object script class, RelObjectScript replacement
 *
 * Script parameters are not used, this class is required by different
 * script initialization of object scripts
 */
class RawObjectScript : public RawScript {
public:
	int16 _runCount; ///< How many times the script was used
	uint16 _param1; ///< Additional parameter not used at the moment
	uint16 _param2; ///< Additional parameter not used at the moment
	uint16 _param3; ///< Additional parameter not used at the moment

	RawObjectScript(uint16 size, uint16 p1, uint16 p2, uint16 p3);
	RawObjectScript(const FWScriptInfo &info, const byte *data, uint16 size, uint16 p1, uint16 p2, uint16 p3);

	/**
	 * Run the script one more time.
	 * @return Run count before incrementation
	 */
	int16 run() { return _runCount++; }
};

/**
 * Future Wars script, prcLinkedListStruct replacement.
 * @todo Rewrite _globalVars initialization
 */
class FWScript {
private:
	const RawScript &_script; ///< Script bytecode reference
	uint16 _pos; ///< Current position in script
	uint16 _line; ///< Current opcode index in bytecode for debugging
	uint16 _compare; ///< Last compare result
	ScriptVars _labels; ///< Current script labels
	ScriptVars _localVars; ///< Local script variables
	ScriptVars &_globalVars; ///< Global variables reference
	FWScriptInfo *_info; ///< Script info

protected:
	static const Opcode *_opcodeTable;
	static unsigned int _numOpcodes;

	int o1_modifyObjectParam();
	int o1_getObjectParam();
	int o1_addObjectParam();
	int o1_subObjectParam();
	int o1_mulObjectParam();
	int o1_divObjectParam();
	int o1_compareObjectParam();
	int o1_setupObject();
	int o1_checkCollision();
	int o1_loadVar();
	int o1_addVar();
	int o1_subVar();
	int o1_mulVar();
	int o1_divVar();
	int o1_compareVar();
	int o1_modifyObjectParam2();
	int o1_loadMask0();
	int o1_unloadMask0();
	int o1_addToBgList();
	int o1_loadMask1();
	int o1_unloadMask1();
	int o1_loadMask4();
	int o1_unloadMask4();
	int o1_addSpriteFilledToBgList();
	int o1_op1B();
	int o1_label();
	int o1_goto();
	int o1_gotoIfSup();
	int o1_gotoIfSupEqu();
	int o1_gotoIfInf();
	int o1_gotoIfInfEqu();
	int o1_gotoIfEqu();
	int o1_gotoIfDiff();
	int o1_removeLabel();
	int o1_loop();
	int o1_startGlobalScript();
	int o1_endGlobalScript();
	int o1_loadAnim();
	int o1_loadBg();
	int o1_loadCt();
	int o1_loadPart();
	int o1_closePart();
	int o1_loadNewPrcName();
	int o1_requestCheckPendingDataLoad();
	int o1_blitAndFade();
	int o1_fadeToBlack();
	int o1_transformPaletteRange();
	int o1_setDefaultMenuBgColor();
	int o1_palRotate();
	int o1_break();
	int o1_endScript();
	int o1_message();
	int o1_loadGlobalVar();
	int o1_compareGlobalVar();
	int o1_declareFunctionName();
	int o1_freePartRange();
	int o1_unloadAllMasks();
	int o1_setScreenDimensions();
	int o1_displayBackground();
	int o1_initializeZoneData();
	int o1_setZoneDataEntry();
	int o1_getZoneDataEntry();
	int o1_setPlayerCommandPosY();
	int o1_allowPlayerInput();
	int o1_disallowPlayerInput();
	int o1_changeDataDisk();
	int o1_loadMusic();
	int o1_playMusic();
	int o1_fadeOutMusic();
	int o1_stopSample();
	int o1_op71();
	int o1_op72();
	int o1_op73();
	int o1_playSample();
	int o1_playSampleSwapped();
	int o1_disableSystemMenu();
	int o1_loadMask5();
	int o1_unloadMask5();

	// pointers to member functions in C++ suck...
	int o2_loadCt();
	int o2_loadPart();
	int o2_addSeqListElement();
	int o2_removeSeq();
	int o2_playSample();
	int o2_playSampleAlt();
	int o2_op81();
	int o2_modifySeqListElement();
	int o2_isSeqRunning();
	int o2_gotoIfSupNearest();
	int o2_gotoIfSupEquNearest();
	int o2_gotoIfInfNearest();
	int o2_gotoIfInfEquNearest();
	int o2_gotoIfEquNearest();
	int o2_gotoIfDiffNearest();
	int o2_startObjectScript();
	int o2_stopObjectScript();
	int o2_op8D();
	int o2_addBackground();
	int o2_removeBackground();
	int o2_loadAbs();
	int o2_loadBg();
	int o2_wasZoneChecked();
	int o2_op9B();
	int o2_op9C();
	int o2_useBgScroll();
	int o2_setAdditionalBgVScroll();
	int o2_op9F();
	int o2_addGfxElementType20();
	int o2_removeGfxElementType20();
	int o2_addGfxElementType21();
	int o2_removeGfxElementType21();
	int o2_loadMask22();
	int o2_unloadMask22();

	byte getNextByte();
	uint16 getNextWord();
	const char *getNextString();

	void load(const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos);

	FWScript(const RawScript &script, int16 index, FWScriptInfo *info);
	FWScript(RawObjectScript &script, int16 index, FWScriptInfo *info);
	FWScript(const FWScript &src, FWScriptInfo *info);

public:
	int16 _index; ///< Index in script table

	static void setupTable();

	FWScript(const RawScript &script, int16 index);
//	FWScript(const RawObjectScript &script, int16 index);
	FWScript(const FWScript &src);
	~FWScript();

	int execute();
	void save(Common::OutSaveFile &fHandle) const;

	friend class FWScriptInfo;

	// workaround for bug in g++ which prevents protected member functions
	// of FWScript from being used in OSScript::_opcodeTable[]
	// initialization ("error: protected within this context")
	friend class OSScript;
};

/**
 * Operation Stealth script, prcLinkedListStruct replacement
 */
class OSScript : public FWScript {
private:
	static const Opcode *_opcodeTable;
	static unsigned int _numOpcodes;

protected:
	void load(const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos);

public:
	static void setupTable();

	OSScript(const RawScript &script, int16 index);
	OSScript(RawObjectScript &script, int16 index);
	OSScript(const OSScript &src);

	friend class OSScriptInfo;
};

/**
 * Future Wars script factory and info
 */
class FWScriptInfo {
protected:
	virtual OpFunc opcodeHandler(byte opcode) const;

public:
	virtual ~FWScriptInfo() {}

	virtual const char *opcodeInfo(byte opcode) const;
	virtual FWScript *create(const RawScript &script, int16 index) const;
	virtual FWScript *create(const RawObjectScript &script, int16 index) const;
	virtual FWScript *create(const RawScript &script, int16 index, const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos) const;
	virtual FWScript *create(const RawObjectScript &script, int16 index, const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos) const;

	friend class FWScript;
};

/**
 * Operation Stealth script factory and info
 */
class OSScriptInfo : public FWScriptInfo {
protected:
	virtual OpFunc opcodeHandler(byte opcode) const;

public:
	virtual ~OSScriptInfo() {}

	virtual const char *opcodeInfo(byte opcode) const;
	virtual FWScript *create(const RawScript &script, int16 index) const;
	virtual FWScript *create(const RawObjectScript &script, int16 index) const;
	virtual FWScript *create(const RawScript &script, int16 index, const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos) const;
	virtual FWScript *create(const RawObjectScript &script, int16 index, const ScriptVars &labels, const ScriptVars &local, uint16 compare, uint16 pos) const;

	friend class FWScript;
};

typedef Common::SharedPtr<FWScript> ScriptPtr;
typedef Common::SharedPtr<RawScript> RawScriptPtr;
typedef Common::SharedPtr<RawObjectScript> RawObjectScriptPtr;
typedef Common::List<ScriptPtr> ScriptList;
typedef Common::Array<RawScriptPtr> RawScriptArray;
typedef Common::Array<RawObjectScriptPtr> RawObjectScriptArray;

#define NUM_MAX_SCRIPT 50

extern FWScriptInfo *scriptInfo;

void setupOpcodes();

void decompileScript(const byte *scriptPtr, uint16 scriptSize, uint16 scriptIdx);
void dumpScript(char *dumpName);

#define OP_loadPart                     0x3F
#define OP_loadNewPrcName               0x41
#define OP_requestCheckPendingDataLoad  0x42
#define OP_endScript                    0x50

void addScriptToGlobalScripts(uint16 idx);
int16 checkCollision(int16 objIdx, int16 x, int16 y, int16 numZones, int16 zoneIdx);

void runObjectScript(int16 entryIdx);

void executeObjectScripts();
void executeGlobalScripts();

void purgeObjectScripts();
void purgeGlobalScripts();

} // End of namespace Cine

#endif
