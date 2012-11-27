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

#include "parallaction/exec.h"
#include "parallaction/input.h"
#include "parallaction/parallaction.h"

#include "common/textconsole.h"

namespace Parallaction {

#define INST_ON			1
#define INST_OFF		2
#define INST_X			3
#define INST_Y			4
#define INST_Z			5
#define INST_F			6
#define INST_LOOP		7
#define INST_ENDLOOP	8
#define INST_SHOW		9
#define INST_INC		10
#define INST_DEC		11
#define INST_SET		12
#define INST_PUT		13
#define INST_CALL		14
#define INST_WAIT		15
#define INST_START		16
#define INST_PROCESS	17
#define INST_MOVE		18
#define INST_COLOR		19
#define INST_SOUND		20
#define INST_MASK		21
#define INST_PRINT		22
#define INST_TEXT		23
#define INST_MUL		24
#define INST_DIV		25
#define INST_IFEQ		26
#define INST_IFLT		27
#define INST_IFGT		28
#define INST_ENDIF		29
#define INST_STOP		30
#define INST_ENDSCRIPT	31

#define SetOpcodeTable(x) table = &x;

typedef Common::Functor1Mem<CommandContext&, void, CommandExec_br> OpcodeV1;
#define COMMAND_OPCODE(op) table->push_back(new OpcodeV1(this, &CommandExec_br::cmdOp_##op))
#define DECLARE_COMMAND_OPCODE(op) void CommandExec_br::cmdOp_##op(CommandContext &ctxt)

typedef Common::Functor1Mem<ProgramContext&, void, ProgramExec_br> OpcodeV2;
#define INSTRUCTION_OPCODE(op) table->push_back(new OpcodeV2(this, &ProgramExec_br::instOp_##op))
#define DECLARE_INSTRUCTION_OPCODE(op) void ProgramExec_br::instOp_##op(ProgramContext &ctxt)

extern const char *_instructionNamesRes_br[];

void Parallaction_br::setupSubtitles(char *s, char *s2, int y) {
	debugC(5, kDebugExec, "setupSubtitles(%s, %s, %i)", s, s2, y);

	clearSubtitles();

	if (!scumm_stricmp("clear", s)) {
		return;
	}

	if (y != -1) {
		_subtitleY = y;
	}

	// FIXME: render subtitles using the right color (10 instead of 0).
	// The original game features a nasty hack, having the font rendering routine
	// replacing color 12 of font RUSSIA with 10 when preparing subtitles.
	uint8 color = (getPlatform() == Common::kPlatformAmiga) ? 11 : 0;
	_subtitle[0] = _gfx->createLabel(_labelFont, s, color);
	_gfx->showLabel(_subtitle[0], CENTER_LABEL_HORIZONTAL, _subtitleY);
	if (s2) {
		_subtitle[1] = _gfx->createLabel(_labelFont, s2, color);
		_gfx->showLabel(_subtitle[1], CENTER_LABEL_HORIZONTAL, _subtitleY + 5 + _labelFont->height());
	} else {
		_subtitle[1] = 0;
	}
#if 0	// disabled because no references to lip sync has been found in the scripts
	_subtitleLipSync = 0;
#endif
}

void Parallaction_br::clearSubtitles() {
	if (_subtitle[0]) {
		_gfx->hideLabel(_subtitle[0]);
	}
	delete _subtitle[0];
	_subtitle[0] = 0;

	if (_subtitle[1]) {
		_gfx->hideLabel(_subtitle[1]);
	}
	delete _subtitle[1];
	_subtitle[1] = 0;
}


DECLARE_COMMAND_OPCODE(location) {
	_vm->_location._startPosition = ctxt._cmd->_startPos;
	_vm->_location._startFrame = 0;
	_vm->_location._followerStartPosition = ctxt._cmd->_startPos2;
	_vm->_location._followerStartFrame = 0;

	_vm->scheduleLocationSwitch(ctxt._cmd->_string);
}


DECLARE_COMMAND_OPCODE(open) {
	_vm->updateDoor(ctxt._cmd->_zone, false);
}


DECLARE_COMMAND_OPCODE(close) {
	_vm->updateDoor(ctxt._cmd->_zone, true);
}


DECLARE_COMMAND_OPCODE(on) {
	_vm->showZone(ctxt._cmd->_zone, true);
}


DECLARE_COMMAND_OPCODE(off) {
	_vm->showZone(ctxt._cmd->_zone, false);
}


DECLARE_COMMAND_OPCODE(call) {
	_vm->callFunction(ctxt._cmd->_callable, &ctxt._z);
}


DECLARE_COMMAND_OPCODE(drop) {
	_vm->dropItem(ctxt._cmd->_object);
}


DECLARE_COMMAND_OPCODE(move) {
	_vm->scheduleWalk(ctxt._cmd->_move.x, ctxt._cmd->_move.y, false);
	suspend();
}

DECLARE_COMMAND_OPCODE(start) {
	ctxt._cmd->_zone->_flags |= kFlagsActing;
}

DECLARE_COMMAND_OPCODE(stop) {
	ctxt._cmd->_zone->_flags &= ~kFlagsActing;
}


DECLARE_COMMAND_OPCODE(character) {
	debugC(9, kDebugExec, "Parallaction_br::cmdOp_character(%s)", ctxt._cmd->_string);
	_vm->changeCharacter(ctxt._cmd->_string);
}


DECLARE_COMMAND_OPCODE(followme) {
	Common::String s(ctxt._cmd->_string);
	if (!s.compareToIgnoreCase("NULL")) {
		s.clear();
	}
	_vm->setFollower(s);
}


DECLARE_COMMAND_OPCODE(onmouse) {
	_vm->_input->setMouseState(MOUSE_ENABLED_SHOW);
}


DECLARE_COMMAND_OPCODE(offmouse) {
	_vm->_input->setMouseState(MOUSE_DISABLED);
}


DECLARE_COMMAND_OPCODE(add) {
	_vm->addInventoryItem(ctxt._cmd->_object);
}


DECLARE_COMMAND_OPCODE(leave) {
	ZonePtr z = ctxt._cmd->_zone;
	_vm->dropItem(z->u._getIcon);
	_vm->showZone(z, true);
}


DECLARE_COMMAND_OPCODE(inc) {
	int v = _vm->getCounterValue(ctxt._cmd->_counterName);
	_vm->setCounterValue(ctxt._cmd->_counterName, v + ctxt._cmd->_counterValue);
}


DECLARE_COMMAND_OPCODE(dec) {
	int v = _vm->getCounterValue(ctxt._cmd->_counterName);
	_vm->setCounterValue(ctxt._cmd->_counterName, v - ctxt._cmd->_counterValue);
}

// these definitions must match those in parser_br.cpp
#define CMD_TEST		25
#define CMD_TEST_GT		26
#define CMD_TEST_LT		27

DECLARE_COMMAND_OPCODE(ifeq) {
	_vm->testCounterCondition(ctxt._cmd->_counterName, CMD_TEST, ctxt._cmd->_counterValue);
}

DECLARE_COMMAND_OPCODE(iflt) {
	_vm->testCounterCondition(ctxt._cmd->_counterName, CMD_TEST_LT, ctxt._cmd->_counterValue);
}

DECLARE_COMMAND_OPCODE(ifgt) {
	_vm->testCounterCondition(ctxt._cmd->_counterName, CMD_TEST_GT, ctxt._cmd->_counterValue);
}


DECLARE_COMMAND_OPCODE(let) {
	_vm->setCounterValue(ctxt._cmd->_counterName, ctxt._cmd->_counterValue);
}


DECLARE_COMMAND_OPCODE(music) {
	warning("Parallaction_br::cmdOp_music not yet implemented");
}


DECLARE_COMMAND_OPCODE(fix) {
	ctxt._cmd->_zone->_flags |= kFlagsFixed;
}


DECLARE_COMMAND_OPCODE(unfix) {
	ctxt._cmd->_zone->_flags &= ~kFlagsFixed;
}


DECLARE_COMMAND_OPCODE(zeta) {
	_vm->_location._zeta0 = ctxt._cmd->_zeta0;
	_vm->_location._zeta1 = ctxt._cmd->_zeta1;
	_vm->_location._zeta2 = ctxt._cmd->_zeta2;
}


DECLARE_COMMAND_OPCODE(scroll) {
	Common::Point p;
	_vm->_gfx->getScrollPos(p);
	_vm->_gfx->initiateScroll(ctxt._cmd->_counterValue - p.x, 0);
}


DECLARE_COMMAND_OPCODE(swap) {
	warning("Parallaction_br::cmdOp_swap not yet implemented");
}


DECLARE_COMMAND_OPCODE(give) {
	warning("Parallaction_br::cmdOp_give not yet implemented");

	/* NOTE: the following code is disabled until I deal with _inventory and
	 * _charInventories not being public
	 */
/*  int item = ctxt._cmd->_object;
	int recipient = ctxt._cmd->_characterId;
	_vm->_charInventories[recipient]->addItem(item);
	_vm->_inventory->removeItem(item);
*/
}


DECLARE_COMMAND_OPCODE(text) {
	_vm->setupSubtitles(ctxt._cmd->_string, ctxt._cmd->_string2, ctxt._cmd->_zeta0);
}


DECLARE_COMMAND_OPCODE(part) {
	_vm->_nextPart = ctxt._cmd->_counterValue;
}


DECLARE_COMMAND_OPCODE(testsfx) {
	warning("Parallaction_br::cmdOp_testsfx not completely implemented");
	_vm->clearLocationFlags(kFlagsTestTrue);	// should test if sfx are enabled
}


DECLARE_COMMAND_OPCODE(ret) {
	g_engineFlags |= kEngineReturn;
}


DECLARE_COMMAND_OPCODE(onsave) {
	warning("Parallaction_br::cmdOp_onsave not yet implemented");
}


DECLARE_COMMAND_OPCODE(offsave) {
	warning("Parallaction_br::cmdOp_offsave not yet implemented");
}

DECLARE_INSTRUCTION_OPCODE(invalid) {
	error("Can't execute invalid opcode %i", ctxt._inst->_index);
}

DECLARE_COMMAND_OPCODE(clear) {
	if (ctxt._cmd->_flags & kFlagsGlobal) {
		ctxt._cmd->_flags &= ~kFlagsGlobal;
		g_globalFlags &= ~ctxt._cmd->_flags;
	} else {
		_vm->clearLocationFlags(ctxt._cmd->_flags);
	}
}

DECLARE_COMMAND_OPCODE(speak) {
	// WORKAROUND: this avoids crashing when the zone is not parsed, like in the case
	// of script bug in ticket #2718449.
	if (!ctxt._cmd->_zone) {
		return;
	}

	if (ACTIONTYPE(ctxt._cmd->_zone) == kZoneSpeak && ctxt._cmd->_zone->u._speakDialogue) {
		_vm->enterDialogueMode(ctxt._cmd->_zone);
	} else {
		_vm->_activeZone = ctxt._cmd->_zone;
	}
}


DECLARE_COMMAND_OPCODE(get) {
	ctxt._cmd->_zone->_flags &= ~kFlagsFixed;
	_vm->runZone(ctxt._cmd->_zone);
}

DECLARE_COMMAND_OPCODE(toggle) {
	if (ctxt._cmd->_flags & kFlagsGlobal) {
		ctxt._cmd->_flags &= ~kFlagsGlobal;
		g_globalFlags ^= ctxt._cmd->_flags;
	} else {
		_vm->toggleLocationFlags(ctxt._cmd->_flags);
	}
}

DECLARE_COMMAND_OPCODE(quit) {
	_vm->quitGame();
}

DECLARE_COMMAND_OPCODE(invalid) {
	error("Can't execute invalid command '%i'", ctxt._cmd->_id);
}

DECLARE_COMMAND_OPCODE(set) {
	if (ctxt._cmd->_flags & kFlagsGlobal) {
		ctxt._cmd->_flags &= ~kFlagsGlobal;
		g_globalFlags |= ctxt._cmd->_flags;
	} else {
		_vm->setLocationFlags(ctxt._cmd->_flags);
	}
}



DECLARE_INSTRUCTION_OPCODE(on) {
	_vm->showZone(ctxt._inst->_z, true);
}


DECLARE_INSTRUCTION_OPCODE(off) {
	_vm->showZone(ctxt._inst->_z, false);
}


DECLARE_INSTRUCTION_OPCODE(set) {
	ctxt._inst->_opA.setValue(ctxt._inst->_opB.getValue());
}



DECLARE_INSTRUCTION_OPCODE(inc) {
	InstructionPtr inst = ctxt._inst;

	int16 rvalue = inst->_opB.getValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (rvalue > 0 ? rvalue : -rvalue);
		if (ctxt._modCounter % _bx != 0) return;

		rvalue = (rvalue > 0 ?  1 : -1);
	}

	int16 lvalue = inst->_opA.getValue();

	switch (inst->_index) {
	case INST_INC:
		lvalue += rvalue;
		break;

	case INST_DEC:
		lvalue -= rvalue;
		break;

	case INST_MUL:
		lvalue *= rvalue;
		break;

	case INST_DIV:
		lvalue /= rvalue;
		break;

	default:
		error("This should never happen. Report immediately");
	}

	inst->_opA.setValue(lvalue);

}


DECLARE_INSTRUCTION_OPCODE(put) {
	// NOTE: there is not a single occurrence of PUT in the scripts
	warning("PUT instruction is not implemented");
}



DECLARE_INSTRUCTION_OPCODE(wait) {
	// NOTE: there is not a single occurrence of WAIT in the scripts
	warning("WAIT instruction is not implemented");
}


DECLARE_INSTRUCTION_OPCODE(start) {
	ctxt._inst->_z->_flags |= kFlagsActing;
}


DECLARE_INSTRUCTION_OPCODE(process) {
	_vm->_activeZone2 = ctxt._inst->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	// NOTE: I couldn't find evidence of scripts containing this instruction being used
	InstructionPtr inst = ctxt._inst;
	_vm->scheduleWalk(inst->_opA.getValue(), inst->_opB.getValue(), false);
	ctxt._suspend = true;
}


DECLARE_INSTRUCTION_OPCODE(color) {
	InstructionPtr inst = ctxt._inst;
	_vm->_gfx->_palette.setEntry(inst->_opB.getValue(), inst->_colors[0], inst->_colors[1], inst->_colors[2]);
}


DECLARE_INSTRUCTION_OPCODE(mask) {
#if 0
	Instruction *inst = *ctxt._inst;
	_gfx->_bgLayers[0] = inst->_opA.getRValue();
	_gfx->_bgLayers[1] = inst->_opB.getRValue();
	_gfx->_bgLayers[2] = inst->_opC.getRValue();
#endif
}


DECLARE_INSTRUCTION_OPCODE(print) {
	// NOTE: there is not a single occurrence of PRINT in the scripts
	// I suppose it was used for debugging
	warning("PRINT instruction is not implemented");
}

DECLARE_INSTRUCTION_OPCODE(text) {
	InstructionPtr inst = ctxt._inst;
	_vm->setupSubtitles(inst->_text, inst->_text2, inst->_y);
}


DECLARE_INSTRUCTION_OPCODE(ifeq) {
	InstructionPtr inst = ctxt._inst;
	bool cond = inst->_opA.getValue() == inst->_opB.getValue();
	if (!cond) {
		ctxt._ip = inst->_endif;
	}
}


DECLARE_INSTRUCTION_OPCODE(iflt) {
	InstructionPtr inst = ctxt._inst;
	bool cond = inst->_opA.getValue() < inst->_opB.getValue();
	if (!cond) {
		ctxt._ip = inst->_endif;
	}
}


DECLARE_INSTRUCTION_OPCODE(ifgt) {
	InstructionPtr inst = ctxt._inst;
	bool cond = inst->_opA.getValue() > inst->_opB.getValue();
	if (!cond) {
		ctxt._ip = inst->_endif;
	}
}


DECLARE_INSTRUCTION_OPCODE(endif) {
	// nothing to do here
}


DECLARE_INSTRUCTION_OPCODE(stop) {
	ZonePtr z = ctxt._inst->_z;

	// Prevent execution if zone is missing. The known case is "PART2/insegui.scr", which has
	// "STOP insegui", which doesn't exist (see ticket #3021744 for the gory details)
	if (!z) return;

	if (ACTIONTYPE(z) == kZoneHear) {
		warning("Parallaction_br::instOp_stop not yet implemented for HEAR zones");
		// TODO: stop music or sound effects generated by a zone.
	} else {
		z->_flags &= ~kFlagsActing;
	}
}

DECLARE_INSTRUCTION_OPCODE(loop) {
	InstructionPtr inst = ctxt._inst;

	ctxt._program->_loopCounter = inst->_opB.getValue();
	ctxt._program->_loopStart = ctxt._ip;
}


DECLARE_INSTRUCTION_OPCODE(endloop) {
	if (--ctxt._program->_loopCounter > 0) {
		ctxt._ip = ctxt._program->_loopStart;
	}
}

DECLARE_INSTRUCTION_OPCODE(show) {
	ctxt._suspend = true;
}

DECLARE_INSTRUCTION_OPCODE(call) {
	_vm->callFunction(ctxt._inst->_immediate, 0);
}


DECLARE_INSTRUCTION_OPCODE(endscript) {
	if ((ctxt._anim->_flags & kFlagsLooping) == 0) {
		ctxt._anim->_flags &= ~kFlagsActing;
		_vm->_cmdExec->run(ctxt._anim->_commands, ctxt._anim);
		ctxt._program->_status = kProgramDone;
	}

	ctxt._ip = 0;
	ctxt._suspend = true;
}


CommandExec_br::CommandExec_br(Parallaction_br* vm) : CommandExec(vm), _vm(vm) {
	CommandOpcodeSet *table = 0;

	SetOpcodeTable(_opcodes);
	COMMAND_OPCODE(invalid);
	COMMAND_OPCODE(set);
	COMMAND_OPCODE(clear);
	COMMAND_OPCODE(start);
	COMMAND_OPCODE(speak);
	COMMAND_OPCODE(get);
	COMMAND_OPCODE(location);
	COMMAND_OPCODE(open);
	COMMAND_OPCODE(close);
	COMMAND_OPCODE(on);
	COMMAND_OPCODE(off);
	COMMAND_OPCODE(call);
	COMMAND_OPCODE(toggle);
	COMMAND_OPCODE(drop);
	COMMAND_OPCODE(quit);
	COMMAND_OPCODE(move);
	COMMAND_OPCODE(stop);
	COMMAND_OPCODE(character);
	COMMAND_OPCODE(followme);
	COMMAND_OPCODE(onmouse);
	COMMAND_OPCODE(offmouse);
	COMMAND_OPCODE(add);
	COMMAND_OPCODE(leave);
	COMMAND_OPCODE(inc);
	COMMAND_OPCODE(dec);
	COMMAND_OPCODE(ifeq);
	COMMAND_OPCODE(iflt);
	COMMAND_OPCODE(ifgt);
	COMMAND_OPCODE(let);
	COMMAND_OPCODE(music);
	COMMAND_OPCODE(fix);
	COMMAND_OPCODE(unfix);
	COMMAND_OPCODE(zeta);
	COMMAND_OPCODE(scroll);
	COMMAND_OPCODE(swap);
	COMMAND_OPCODE(give);
	COMMAND_OPCODE(text);
	COMMAND_OPCODE(part);
	COMMAND_OPCODE(testsfx);
	COMMAND_OPCODE(ret);
	COMMAND_OPCODE(onsave);
	COMMAND_OPCODE(offsave);
}


ProgramExec_br::ProgramExec_br(Parallaction_br *vm) : _vm(vm) {
	_instructionNames = _instructionNamesRes_br;

	ProgramOpcodeSet *table = 0;

	SetOpcodeTable(_opcodes);
	INSTRUCTION_OPCODE(invalid);
	INSTRUCTION_OPCODE(on);
	INSTRUCTION_OPCODE(off);
	INSTRUCTION_OPCODE(set);		// x
	INSTRUCTION_OPCODE(set);		// y
	INSTRUCTION_OPCODE(set);		// z
	INSTRUCTION_OPCODE(set);		// f
	INSTRUCTION_OPCODE(loop);
	INSTRUCTION_OPCODE(endloop);
	INSTRUCTION_OPCODE(show);		// show
	INSTRUCTION_OPCODE(inc);
	INSTRUCTION_OPCODE(inc);		// dec
	INSTRUCTION_OPCODE(set);
	INSTRUCTION_OPCODE(put);
	INSTRUCTION_OPCODE(call);
	INSTRUCTION_OPCODE(wait);
	INSTRUCTION_OPCODE(start);
	INSTRUCTION_OPCODE(process);
	INSTRUCTION_OPCODE(move);
	INSTRUCTION_OPCODE(color);
	INSTRUCTION_OPCODE(process);	// sound
	INSTRUCTION_OPCODE(mask);
	INSTRUCTION_OPCODE(print);
	INSTRUCTION_OPCODE(text);
	INSTRUCTION_OPCODE(inc);		// mul
	INSTRUCTION_OPCODE(inc);		// div
	INSTRUCTION_OPCODE(ifeq);
	INSTRUCTION_OPCODE(iflt);
	INSTRUCTION_OPCODE(ifgt);
	INSTRUCTION_OPCODE(endif);
	INSTRUCTION_OPCODE(stop);
	INSTRUCTION_OPCODE(endscript);
}



} // namespace Parallaction
