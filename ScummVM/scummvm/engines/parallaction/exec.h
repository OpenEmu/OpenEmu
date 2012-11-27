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


#ifndef PARALLACTION_EXEC_H
#define PARALLACTION_EXEC_H

#include "common/util.h"
#include "parallaction/objects.h"


namespace Parallaction {

class Parallaction_ns;
class Parallaction_br;

/* NOTE: CommandExec and ProgramExec perform similar tasks on different data.
   CommandExec executes commands found in location scripts, while ProgramExec
   runs animation programs.

   The main difference is how suspension is handled. CommandExec is coded with
   the assumption that there may be at most one suspended list of commands at any
   moment, and thus stores the suspended context itself. It also offers a
   runSuspended() routine that resumes execution on request.
   ProgramExec instead stores the suspension information in the programs themselves.
   Programs are in fact meant to be run (almost) regularly on each frame .
 */

struct CommandContext {
	CommandPtr _cmd;
	ZonePtr	_z;

	// TODO: add a way to invoke CommandExec::suspend() from the context. With that
	// in place, opcodes dependency on CommandExec would be zero, and they could
	// be moved into a Game object, together with the non-infrastructural code now
	// in Parallaction_XX
};
typedef Common::Functor1<CommandContext&, void> CommandOpcode;
typedef Common::Array<const CommandOpcode *>	CommandOpcodeSet;
#define DECLARE_UNQUALIFIED_COMMAND_OPCODE(op) void cmdOp_##op(CommandContext &)

struct ProgramContext {
	AnimationPtr	_anim;
	ProgramPtr		_program;
	InstructionPtr _inst;
	uint32 		_ip;
	uint16		_modCounter;
	bool		_suspend;
};
typedef Common::Functor1<ProgramContext&, void> ProgramOpcode;
typedef Common::Array<const ProgramOpcode *>	ProgramOpcodeSet;
#define DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(op) void instOp_##op(ProgramContext &)


template<class OpcodeSet>
class Exec {
protected:
	OpcodeSet _opcodes;
	typedef typename OpcodeSet::iterator OpIt;
public:
	virtual ~Exec() {
		for (OpIt i = _opcodes.begin(); i != _opcodes.end(); ++i)
			delete *i;
		_opcodes.clear();
	}
};

class CommandExec : public Exec<CommandOpcodeSet> {
protected:
	Parallaction *_vm;

	CommandContext _ctxt;
	ZonePtr	_execZone;
	bool _running;
	bool _suspend;

	struct SuspendedContext {
		bool _valid;
		CommandList::iterator _first;
		CommandList::iterator _last;
		ZonePtr	_zone;
	} _suspendedCtxt;

	void runList(CommandList::iterator first, CommandList::iterator last);
	void createSuspendList(CommandList::iterator first, CommandList::iterator last);
	void cleanSuspendedList();
public:
	CommandExec(Parallaction *vm);

	void run(CommandList &list, ZonePtr z = ZonePtr());
	void runSuspended();
	void suspend();
};

class CommandExec_ns : public CommandExec {
protected:
	Parallaction_ns	*_vm;

	DECLARE_UNQUALIFIED_COMMAND_OPCODE(invalid);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(set);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(clear);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(start);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(speak);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(get);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(location);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(open);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(close);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(on);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(off);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(call);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(toggle);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(drop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(quit);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(move);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(stop);
public:
	CommandExec_ns(Parallaction_ns* vm);
};

class CommandExec_br : public CommandExec {
protected:
	Parallaction_br	*_vm;

	DECLARE_UNQUALIFIED_COMMAND_OPCODE(invalid);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(set);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(clear);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(speak);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(get);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(toggle);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(quit);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(location);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(open);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(close);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(on);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(off);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(call);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(drop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(move);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(start);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(stop);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(character);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(followme);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(onmouse);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(offmouse);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(add);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(leave);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(inc);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(dec);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ifeq);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(iflt);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ifgt);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(let);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(music);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(fix);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(unfix);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(zeta);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(scroll);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(swap);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(give);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(text);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(part);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(testsfx);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(ret);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(onsave);
	DECLARE_UNQUALIFIED_COMMAND_OPCODE(offsave);
public:
	CommandExec_br(Parallaction_br* vm);
};




class ProgramExec : public Exec<ProgramOpcodeSet> {
protected:
	ProgramContext _ctxt;
	uint16	_modCounter;
	const char **_instructionNames;

	void runScript(ProgramPtr script, AnimationPtr a);
public:
	void runScripts(ProgramList::iterator first, ProgramList::iterator last);
	ProgramExec();
};

class ProgramExec_ns : public ProgramExec {
protected:
	Parallaction_ns *_vm;

	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(invalid);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(on);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(off);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(loop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endloop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(show);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(call);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(inc);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(set);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(put);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(wait);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(start);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(sound);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(move);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endscript);
public:
	ProgramExec_ns(Parallaction_ns *vm);
};

class ProgramExec_br : public ProgramExec {
protected:
	Parallaction_br *_vm;

	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(invalid);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(loop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endloop);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(show);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(call);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endscript);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(on);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(off);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(inc);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(dec);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(set);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(put);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(wait);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(start);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(process);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(move);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(color);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(mask);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(print);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(text);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(mul);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(div);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(ifeq);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(iflt);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(ifgt);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(endif);
	DECLARE_UNQUALIFIED_INSTRUCTION_OPCODE(stop);
public:
	ProgramExec_br(Parallaction_br *vm);
};

} // namespace Parallaction

#endif
