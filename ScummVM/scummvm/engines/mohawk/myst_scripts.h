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

#ifndef MYST_SCRIPTS_H
#define MYST_SCRIPTS_H

#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/util.h"

#include "mohawk/myst_state.h"

namespace Mohawk {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class MohawkEngine_Myst;
class MystResource;

enum MystScriptType {
	kMystScriptNone,
	kMystScriptNormal,
	kMystScriptInit,
	kMystScriptExit
};

struct MystScriptEntry {
	MystScriptEntry();
	~MystScriptEntry();

	MystScriptType type;
	uint16 resourceId;
	uint16 opcode;
	uint16 var;
	uint16 argc;
	uint16 *argv;
	uint16 u1;
};

typedef Common::SharedPtr<Common::Array<MystScriptEntry> > MystScript;

class MystScriptParser {
public:
	MystScriptParser(MohawkEngine_Myst *vm);
	virtual ~MystScriptParser();

	void runScript(MystScript script, MystResource *invokingResource = NULL);
	void runOpcode(uint16 op, uint16 var = 0, uint16 argc = 0, uint16 *argv = NULL);
	const Common::String getOpcodeDesc(uint16 op);
	MystScript readScript(Common::SeekableReadStream *stream, MystScriptType type);
	void setInvokingResource(MystResource *resource) { _invokingResource = resource; }

	virtual void disablePersistentScripts() = 0;
	virtual void runPersistentScripts() = 0;

	virtual uint16 getVar(uint16 var);
	virtual void toggleVar(uint16 var);
	virtual bool setVarValue(uint16 var, uint16 value);

	virtual uint16 getMap() { return 0; }
	void showMap();

	void animatedUpdate(uint16 argc, uint16 *argv, uint16 delay);

	DECLARE_OPCODE(unknown);

	// Common opcodes
	DECLARE_OPCODE(o_toggleVar);
	DECLARE_OPCODE(o_setVar);
	DECLARE_OPCODE(o_changeCardSwitch);
	DECLARE_OPCODE(o_takePage);
	DECLARE_OPCODE(o_redrawCard);
	DECLARE_OPCODE(o_goToDest);
	DECLARE_OPCODE(o_triggerMovie);
	DECLARE_OPCODE(o_toggleVarNoRedraw);
	DECLARE_OPCODE(o_drawAreaState);
	DECLARE_OPCODE(o_redrawAreaForVar);
	DECLARE_OPCODE(o_changeCardDirectional);
	DECLARE_OPCODE(o_changeCardPush);
	DECLARE_OPCODE(o_changeCardPop);
	DECLARE_OPCODE(o_enableAreas);
	DECLARE_OPCODE(o_disableAreas);
	DECLARE_OPCODE(o_directionalUpdate);
	DECLARE_OPCODE(o_toggleAreasActivation);
	DECLARE_OPCODE(o_playSound);
	DECLARE_OPCODE(o_stopSoundBackground);
	DECLARE_OPCODE(o_playSoundBlocking);
	DECLARE_OPCODE(o_copyBackBufferToScreen);
	DECLARE_OPCODE(o_copyImageToBackBuffer);
	DECLARE_OPCODE(o_changeBackgroundSound);
	DECLARE_OPCODE(o_soundPlaySwitch);
	DECLARE_OPCODE(o_copyImageToScreen);
	DECLARE_OPCODE(o_soundResumeBackground);
	DECLARE_OPCODE(o_changeCard);
	DECLARE_OPCODE(o_drawImageChangeCard);
	DECLARE_OPCODE(o_changeMainCursor);
	DECLARE_OPCODE(o_hideCursor);
	DECLARE_OPCODE(o_showCursor);
	DECLARE_OPCODE(o_delay);
	DECLARE_OPCODE(o_changeStack);
	DECLARE_OPCODE(o_changeCardPlaySoundDirectional);
	DECLARE_OPCODE(o_directionalUpdatePlaySound);
	DECLARE_OPCODE(o_saveMainCursor);
	DECLARE_OPCODE(o_restoreMainCursor);
	DECLARE_OPCODE(o_soundWaitStop);
	DECLARE_OPCODE(o_exitMap);

	// Used in multiple stacks
	DECLARE_OPCODE(o_quit);

	DECLARE_OPCODE(NOP);

protected:
	MohawkEngine_Myst *_vm;
	MystGameState::Globals &_globals;

	typedef void (MystScriptParser::*OpcodeProcMyst)(uint16 op, uint16 var, uint16 argc, uint16* argv);

	struct MystOpcode {
		MystOpcode(uint16 o, OpcodeProcMyst p, const char *d) : op(o), proc(p), desc(d) {}

		uint16 op;
		OpcodeProcMyst proc;
		const char *desc;
	};

	Common::Array<MystOpcode *> _opcodes;

	MystResource *_invokingResource;

	uint16 _savedCardId;
	uint16 _savedMapCardId;
	uint16 _savedCursorId;
	int16 _tempVar; // Generic temp var used by the scripts
	uint32 _startTime; // Generic start time used by the scripts

	static const uint8 _stackMap[];
	static const uint16 _startCard[];

	void setupCommonOpcodes();
	void varUnusedCheck(uint16 op, uint16 var);
};

} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
