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
#include "parallaction/sound.h"

#include "common/textconsole.h"

namespace Parallaction {

#define INST_ON							1
#define INST_OFF						2
#define INST_X							3
#define INST_Y							4
#define INST_Z							5
#define INST_F							6
#define INST_LOOP						7
#define INST_ENDLOOP					8
#define INST_SHOW						9
#define INST_INC						10
#define INST_DEC						11
#define INST_SET						12
#define INST_PUT						13
#define INST_CALL						14
#define INST_WAIT						15
#define INST_START						16
#define INST_SOUND						17
#define INST_MOVE						18
#define INST_ENDSCRIPT					19

#define SetOpcodeTable(x) table = &x;



typedef Common::Functor1Mem<CommandContext&, void, CommandExec_ns> OpcodeV1;
#define COMMAND_OPCODE(op) table->push_back(new OpcodeV1(this, &CommandExec_ns::cmdOp_##op))
#define DECLARE_COMMAND_OPCODE(op) void CommandExec_ns::cmdOp_##op(CommandContext& ctxt)

typedef Common::Functor1Mem<ProgramContext&, void, ProgramExec_ns> OpcodeV2;
#define INSTRUCTION_OPCODE(op) table->push_back(new OpcodeV2(this, &ProgramExec_ns::instOp_##op))
#define DECLARE_INSTRUCTION_OPCODE(op) void ProgramExec_ns::instOp_##op(ProgramContext& ctxt)

extern const char *_instructionNamesRes_ns[];


DECLARE_INSTRUCTION_OPCODE(on) {
	InstructionPtr inst = ctxt._inst;

	inst->_a->_flags |= kFlagsActive;
	inst->_a->_flags &= ~kFlagsRemove;
}


DECLARE_INSTRUCTION_OPCODE(off) {
	ctxt._inst->_a->_flags |= kFlagsRemove;
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

DECLARE_INSTRUCTION_OPCODE(inc) {
	InstructionPtr inst = ctxt._inst;
	int16 _si = inst->_opB.getValue();

	if (inst->_flags & kInstMod) {	// mod
		int16 _bx = (_si > 0 ? _si : -_si);
		if (ctxt._modCounter % _bx != 0) return;

		_si = (_si > 0 ?  1 : -1);
	}

	int16 lvalue = inst->_opA.getValue();

	if (inst->_index == INST_INC) {
		lvalue += _si;
	} else {
		lvalue -= _si;
	}

	inst->_opA.setValue(lvalue);

}


DECLARE_INSTRUCTION_OPCODE(set) {
	ctxt._inst->_opA.setValue(ctxt._inst->_opB.getValue());
}


DECLARE_INSTRUCTION_OPCODE(put) {
	InstructionPtr inst = ctxt._inst;
	Common::Rect r;
	inst->_a->getFrameRect(r);

	Graphics::Surface v18;
	v18.w = r.width();
	v18.h = r.height();
	v18.pixels = inst->_a->getFrameData();

	int16 x = inst->_opA.getValue();
	int16 y = inst->_opB.getValue();
	bool mask = (inst->_flags & kInstMaskedPut) == kInstMaskedPut;

	_vm->_gfx->patchBackground(v18, x, y, mask);
}

DECLARE_INSTRUCTION_OPCODE(show) {
	ctxt._suspend = true;
}

DECLARE_INSTRUCTION_OPCODE(invalid) {
	error("Can't execute invalid opcode %i", ctxt._inst->_index);
}

DECLARE_INSTRUCTION_OPCODE(call) {
	_vm->callFunction(ctxt._inst->_immediate, 0);
}


DECLARE_INSTRUCTION_OPCODE(wait) {
	if (g_engineFlags & kEngineWalking) {
		ctxt._ip--;
		ctxt._suspend = true;
	}
}


DECLARE_INSTRUCTION_OPCODE(start) {
	ctxt._inst->_a->_flags |= (kFlagsActing | kFlagsActive);
}


DECLARE_INSTRUCTION_OPCODE(sound) {
	_vm->_activeZone = ctxt._inst->_z;
}


DECLARE_INSTRUCTION_OPCODE(move) {
	InstructionPtr inst = ctxt._inst;

	int16 x = inst->_opA.getValue();
	int16 y = inst->_opB.getValue();

	_vm->scheduleWalk(x, y, false);
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


DECLARE_COMMAND_OPCODE(clear) {
	if (ctxt._cmd->_flags & kFlagsGlobal) {
		ctxt._cmd->_flags &= ~kFlagsGlobal;
		g_globalFlags &= ~ctxt._cmd->_flags;
	} else {
		_vm->clearLocationFlags(ctxt._cmd->_flags);
	}
}


DECLARE_COMMAND_OPCODE(start) {
	ctxt._cmd->_zone->_flags |= kFlagsActing;
}


DECLARE_COMMAND_OPCODE(speak) {
	if (ACTIONTYPE(ctxt._cmd->_zone) == kZoneSpeak) {
		_vm->enterDialogueMode(ctxt._cmd->_zone);
	} else {
		_vm->_activeZone = ctxt._cmd->_zone;
	}
}


DECLARE_COMMAND_OPCODE(get) {
	ctxt._cmd->_zone->_flags &= ~kFlagsFixed;
	_vm->runZone(ctxt._cmd->_zone);
}


DECLARE_COMMAND_OPCODE(location) {
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


DECLARE_COMMAND_OPCODE(toggle) {
	if (ctxt._cmd->_flags & kFlagsGlobal) {
		ctxt._cmd->_flags &= ~kFlagsGlobal;
		g_globalFlags ^= ctxt._cmd->_flags;
	} else {
		_vm->toggleLocationFlags(ctxt._cmd->_flags);
	}
}


DECLARE_COMMAND_OPCODE(drop){
	_vm->dropItem( ctxt._cmd->_object );
}


DECLARE_COMMAND_OPCODE(quit) {
	_vm->quitGame();
}


DECLARE_COMMAND_OPCODE(move) {
	_vm->scheduleWalk(ctxt._cmd->_move.x, ctxt._cmd->_move.y, false);
}


DECLARE_COMMAND_OPCODE(stop) {
	ctxt._cmd->_zone->_flags &= ~kFlagsActing;
}

CommandExec_ns::CommandExec_ns(Parallaction_ns* vm) : CommandExec(vm), _vm(vm) {
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
}

ProgramExec_ns::ProgramExec_ns(Parallaction_ns *vm) : _vm(vm) {
	_instructionNames = _instructionNamesRes_ns;

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
	INSTRUCTION_OPCODE(show);
	INSTRUCTION_OPCODE(inc);
	INSTRUCTION_OPCODE(inc);		// dec
	INSTRUCTION_OPCODE(set);
	INSTRUCTION_OPCODE(put);
	INSTRUCTION_OPCODE(call);
	INSTRUCTION_OPCODE(wait);
	INSTRUCTION_OPCODE(start);
	INSTRUCTION_OPCODE(sound);
	INSTRUCTION_OPCODE(move);
	INSTRUCTION_OPCODE(endscript);
}

}	// namespace Parallaction
