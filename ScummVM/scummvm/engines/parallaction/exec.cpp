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
#include "parallaction/parallaction.h"

namespace Parallaction {

void ProgramExec::runScript(ProgramPtr script, AnimationPtr a) {
	debugC(9, kDebugExec, "runScript(Animation = %s)", a->_name);

	_ctxt._ip = script->_ip;
	_ctxt._anim = a;
	_ctxt._program = script;
	_ctxt._suspend = false;
	_ctxt._modCounter = _modCounter;

	InstructionPtr inst;
	for ( ; (a->_flags & kFlagsActing) ; ) {

		inst = script->_instructions[_ctxt._ip];
		_ctxt._inst = inst;
		++_ctxt._ip;

		debugC(9, kDebugExec, "inst [%02i] %s\n", inst->_index, _instructionNames[inst->_index - 1]);

		script->_status = kProgramRunning;

		(*_opcodes[inst->_index])(_ctxt);

		if (_ctxt._suspend)
			break;

	}
	script->_ip = _ctxt._ip;

}

void ProgramExec::runScripts(ProgramList::iterator first, ProgramList::iterator last) {
	if (g_engineFlags & kEnginePauseJobs) {
		return;
	}

	for (ProgramList::iterator it = first; it != last; ++it) {

		AnimationPtr a = (*it)->_anim;

		if (a->_flags & kFlagsCharacter)
			a->resetZ();

		if ((a->_flags & kFlagsActing) == 0)
			continue;

		runScript(*it, a);

		if (a->_flags & kFlagsCharacter)
			a->resetZ();
	}

	_modCounter++;

	return;
}

ProgramExec::ProgramExec() : _modCounter(0) {
}


void CommandExec::runList(CommandList::iterator first, CommandList::iterator last) {

	uint32 useFlags = 0;
	bool useLocalFlags;

	_suspend = false;
	_running = true;

	for ( ; first != last; ++first) {
		if (_vm->shouldQuit())
			break;

		CommandPtr cmd = *first;

		if (cmd->_valid && !cmd->_zone && !cmd->_zoneName.empty()) {
			// try binding the command to a zone
			cmd->_zone = _vm->_location.findZone(cmd->_zoneName.c_str());
			cmd->_valid = cmd->_zone != 0;
		}

		if (!cmd->_valid) {
			continue;
		}

		if (cmd->_flagsOn & kFlagsGlobal) {
			useFlags = g_globalFlags | kFlagsGlobal;
			useLocalFlags = false;
		} else {
			useFlags = _vm->getLocationFlags();
			useLocalFlags = true;
		}

		bool onMatch = (cmd->_flagsOn & useFlags) == cmd->_flagsOn;
		bool offMatch = (cmd->_flagsOff & ~useFlags) == cmd->_flagsOff;

		debugC(3, kDebugExec, "runCommands[%i] (on: %x, off: %x), (%s = %x)", cmd->_id,  cmd->_flagsOn, cmd->_flagsOff,
			useLocalFlags ? "LOCALFLAGS" : "GLOBALFLAGS", useFlags);

		if (!onMatch || !offMatch) continue;

		_ctxt._z = _execZone;
		_ctxt._cmd = cmd;

		(*_opcodes[cmd->_id])(_ctxt);

		if (_suspend) {
			createSuspendList(++first, last);
			return;
		}
	}

	_running = false;

}

void CommandExec::run(CommandList& list, ZonePtr z) {
	if (list.size() == 0) {
		debugC(3, kDebugExec, "runCommands: nothing to do");
		return;
	}

	_execZone = z;

	debugC(3, kDebugExec, "runCommands starting");
	runList(list.begin(), list.end());
	debugC(3, kDebugExec, "runCommands completed");
}

void CommandExec::createSuspendList(CommandList::iterator first, CommandList::iterator last) {
	if (first == last) {
		return;
	}

	debugC(3, kDebugExec, "CommandExec::createSuspendList()");

	_suspendedCtxt._valid = true;
	_suspendedCtxt._first = first;
	_suspendedCtxt._last = last;
	_suspendedCtxt._zone = _execZone;
}

void CommandExec::cleanSuspendedList() {
	debugC(3, kDebugExec, "CommandExec::cleanSuspended()");

	_suspendedCtxt._valid = false;
	_suspendedCtxt._first = _suspendedCtxt._last;
	_suspendedCtxt._zone.reset();
}

void CommandExec::suspend() {
	if (!_running)
		return;

	_suspend = true;
}

void CommandExec::runSuspended() {
	if (g_engineFlags & kEngineWalking) {
		return;
	}

	if (_suspendedCtxt._valid) {
		debugC(3, kDebugExec, "CommandExec::runSuspended()");

		_execZone = _suspendedCtxt._zone;
		CommandList::iterator first = _suspendedCtxt._first;
		CommandList::iterator last = _suspendedCtxt._last;
		cleanSuspendedList();
		runList(first, last);
	}
}


CommandExec::CommandExec(Parallaction *vm) : _vm(vm), _suspend(false), _running(false) {
	_suspendedCtxt._valid = false;
}

}
