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

// Script debugger functionality. Absolutely not threadsafe.

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/debug.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/script.h"

namespace Sci {

//#define VM_DEBUG_SEND

// This table is only used for debugging. Don't include it for devices
// with not enough available memory (e.g. phones), where REDUCE_MEMORY_USAGE
// is defined
#ifndef REDUCE_MEMORY_USAGE
const char *opcodeNames[] = {
	   "bnot",       "add",      "sub",      "mul",      "div",
		"mod",       "shr",      "shl",      "xor",      "and",
		 "or",       "neg",      "not",      "eq?",      "ne?",
		"gt?",       "ge?",      "lt?",      "le?",     "ugt?",
	   "uge?",      "ult?",     "ule?",       "bt",      "bnt",
		"jmp",       "ldi",     "push",    "pushi",     "toss",
		"dup",      "link",     "call",    "callk",    "callb",
	  "calle",       "ret",     "send",    "dummy",    "dummy",
	  "class",     "dummy",     "self",    "super",    "&rest",
		"lea",    "selfID",    "dummy",    "pprev",     "pToa",
	   "aTop",      "pTos",     "sTop",    "ipToa",    "dpToa",
	  "ipTos",     "dpTos",    "lofsa",    "lofss",    "push0",
	  "push1",     "push2", "pushSelf",     "line",      "lag",
		"lal",       "lat",      "lap",      "lsg",      "lsl",
		"lst",       "lsp",     "lagi",     "lali",     "lati",
	   "lapi",      "lsgi",     "lsli",     "lsti",     "lspi",
		"sag",       "sal",      "sat",      "sap",      "ssg",
		"ssl",       "sst",      "ssp",     "sagi",     "sali",
	   "sati",      "sapi",     "ssgi",     "ssli",     "ssti",
	   "sspi",       "+ag",      "+al",      "+at",      "+ap",
		"+sg",       "+sl",      "+st",      "+sp",     "+agi",
	   "+ali",      "+ati",     "+api",     "+sgi",     "+sli",
	   "+sti",      "+spi",      "-ag",      "-al",      "-at",
		"-ap",       "-sg",      "-sl",      "-st",      "-sp",
	   "-agi",      "-ali",     "-ati",     "-api",     "-sgi",
	   "-sli",      "-sti",     "-spi"
};
#endif	// REDUCE_MEMORY_USAGE

// Disassembles one command from the heap, returns address of next command or 0 if a ret was encountered.
reg_t disassemble(EngineState *s, reg32_t pos, bool printBWTag, bool printBytecode) {
	SegmentObj *mobj = s->_segMan->getSegment(pos.getSegment(), SEG_TYPE_SCRIPT);
	Script *script_entity = NULL;
	const byte *scr;
	uint32 scr_size;
	reg_t retval = make_reg(pos.getSegment(), pos.getOffset() + 1);
	uint16 param_value = 0xffff; // Suppress GCC warning by setting default value, chose value as invalid to getKernelName etc.
	uint i = 0;
	Kernel *kernel = g_sci->getKernel();

	if (!mobj) {
		warning("Disassembly failed: Segment %04x non-existant or not a script", pos.getSegment());
		return retval;
	} else
		script_entity = (Script *)mobj;

	scr = script_entity->getBuf();
	scr_size = script_entity->getBufSize();

	if (pos.getOffset() >= scr_size) {
		warning("Trying to disassemble beyond end of script");
		return NULL_REG;
	}

	int16 opparams[4];
	byte opsize;
	uint bytecount = readPMachineInstruction(scr + pos.getOffset(), opsize, opparams);
	const byte opcode = opsize >> 1;

	debugN("%04x:%04x: ", PRINT_REG(pos));

	if (opcode == op_pushSelf) { // 0x3e (62)
		if ((opsize & 1) && g_sci->getGameId() != GID_FANMADE) {
			// Debug opcode op_file
			debugN("file \"%s\"\n", scr + pos.getOffset() + 1);	// +1: op_pushSelf size
			retval.incOffset(bytecount - 1);
			return retval;
		}
	}

	opsize &= 1; // byte if true, word if false

	if (printBytecode) {
		if (pos.getOffset() + bytecount > scr_size) {
			warning("Operation arguments extend beyond end of script");
			return retval;
		}

		for (i = 0; i < bytecount; i++)
			debugN("%02x ", scr[pos.getOffset() + i]);

		for (i = bytecount; i < 5; i++)
			debugN("   ");
	}

	if (printBWTag)
		debugN("[%c] ", opsize ? 'B' : 'W');

#ifndef REDUCE_MEMORY_USAGE
	debugN("%s", opcodeNames[opcode]);
#endif

	i = 0;
	while (g_sci->_opcode_formats[opcode][i]) {
		switch (g_sci->_opcode_formats[opcode][i++]) {
		case Script_Invalid:
			warning("-Invalid operation-");
			break;

		case Script_SByte:
		case Script_Byte:
			param_value = scr[retval.getOffset()];
			debugN(" %02x", scr[retval.getOffset()]);
			retval.incOffset(1);
			break;

		case Script_Word:
		case Script_SWord:
			param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.getOffset()]);
			debugN(" %04x", READ_SCI11ENDIAN_UINT16(&scr[retval.getOffset()]));
			retval.incOffset(2);
			break;

		case Script_SVariable:
		case Script_Variable:
		case Script_Property:
		case Script_Global:
		case Script_Local:
		case Script_Temp:
		case Script_Param:
			if (opsize) {
				param_value = scr[retval.getOffset()];
				retval.incOffset(1);
			} else {
				param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.getOffset()]);
				retval.incOffset(2);
			}

			if (opcode == op_callk)
				debugN(" %s[%x]", (param_value < kernel->_kernelFuncs.size()) ?
							((param_value < kernel->getKernelNamesSize()) ? kernel->getKernelName(param_value).c_str() : "[Unknown(postulated)]")
							: "<invalid>", param_value);
			else
				debugN(opsize ? " %02x" : " %04x", param_value);

			break;

		case Script_Offset:
			if (opsize) {
				param_value = scr[retval.getOffset()];
				retval.incOffset(1);
			} else {
				param_value = READ_SCI11ENDIAN_UINT16(&scr[retval.getOffset()]);
				retval.incOffset(2);
			}
			debugN(opsize ? " %02x" : " %04x", param_value);
			break;

		case Script_SRelative:
			if (opsize) {
				int8 offset = (int8)scr[retval.getOffset()];
				retval.incOffset(1);
				debugN(" %02x  [%04x]", 0xff & offset, 0xffff & (retval.getOffset() + offset));
			}
			else {
				int16 offset = (int16)READ_SCI11ENDIAN_UINT16(&scr[retval.getOffset()]);
				retval.incOffset(2);
				debugN(" %04x  [%04x]", 0xffff & offset, 0xffff & (retval.getOffset() + offset));
			}
			break;

		case Script_End:
			retval = NULL_REG;
			break;

		default:
			error("Internal assertion failed in disassemble()");

		}
	}

	if (pos == s->xs->addr.pc) { // Extra information if debugging the current opcode
		if ((opcode == op_pTos) || (opcode == op_sTop) || (opcode == op_pToa) || (opcode == op_aTop) ||
		        (opcode == op_dpToa) || (opcode == op_ipToa) || (opcode == op_dpTos) || (opcode == op_ipTos)) {
			const Object *obj = s->_segMan->getObject(s->xs->objp);
			if (!obj) {
				warning("Attempted to reference on non-object at %04x:%04x", PRINT_REG(s->xs->objp));
			} else {
				if (getSciVersion() == SCI_VERSION_3)
					debugN("\t(%s)", g_sci->getKernel()->getSelectorName(param_value).c_str());
				else
					debugN("\t(%s)", g_sci->getKernel()->getSelectorName(obj->propertyOffsetToId(s->_segMan, param_value)).c_str());
			}
		}
	}

	debugN("\n");

	if (pos == s->xs->addr.pc) { // Extra information if debugging the current opcode
		if (opcode == op_callk) {
			int stackframe = (scr[pos.getOffset() + 2] >> 1) + (s->r_rest);
			int argc = ((s->xs->sp)[- stackframe - 1]).getOffset();
			bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

			if (!oldScriptHeader)
				argc += (s->r_rest);

			debugN(" Kernel params: (");

			for (int j = 0; j < argc; j++) {
				debugN("%04x:%04x", PRINT_REG((s->xs->sp)[j - stackframe]));
				if (j + 1 < argc)
					debugN(", ");
			}
			debugN(")\n");
		} else if ((opcode == op_send) || (opcode == op_self)) {
			int restmod = s->r_rest;
			int stackframe = (scr[pos.getOffset() + 1] >> 1) + restmod;
			reg_t *sb = s->xs->sp;
			uint16 selector;
			reg_t fun_ref;

			while (stackframe > 0) {
				int argc = sb[- stackframe + 1].getOffset();
				const char *name = NULL;
				reg_t called_obj_addr = s->xs->objp;

				if (opcode == op_send)
					called_obj_addr = s->r_acc;
				else if (opcode == op_self)
					called_obj_addr = s->xs->objp;

				selector = sb[- stackframe].getOffset();

				name = s->_segMan->getObjectName(called_obj_addr);

				if (!name)
					name = "<invalid>";

				debugN("  %s::%s[", name, g_sci->getKernel()->getSelectorName(selector).c_str());

				switch (lookupSelector(s->_segMan, called_obj_addr, selector, 0, &fun_ref)) {
				case kSelectorMethod:
					debugN("FUNCT");
					argc += restmod;
					restmod = 0;
					break;
				case kSelectorVariable:
					debugN("VAR");
					break;
				case kSelectorNone:
					debugN("INVALID");
					break;
				}

				debugN("](");

				while (argc--) {
					debugN("%04x:%04x", PRINT_REG(sb[- stackframe + 2]));
					if (argc)
						debugN(", ");
					stackframe--;
				}

				debugN(")\n");
				stackframe -= 2;
			} // while (stackframe > 0)
		} // Send-like opcodes
	} // (heappos == *p_pc)

	return retval;
}

bool isJumpOpcode(EngineState *s, reg_t pos, reg_t& jumpTarget) {
	SegmentObj *mobj = s->_segMan->getSegment(pos.getSegment(), SEG_TYPE_SCRIPT);
	if (!mobj)
		return false;
	Script *script_entity = (Script *)mobj;

	const byte *scr = script_entity->getBuf();
	uint scr_size = script_entity->getScriptSize();

	if (pos.getOffset() >= scr_size)
		return false;

	int16 opparams[4];
	byte opsize;
	int bytecount = readPMachineInstruction(scr + pos.getOffset(), opsize, opparams);
	const byte opcode = opsize >> 1;

	switch (opcode) {
	case op_bt:
	case op_bnt:
	case op_jmp:
		{
		reg_t jmpTarget = pos + bytecount + opparams[0];
		// QFG2 has invalid jumps outside the script buffer in script 260
		if (jmpTarget.getOffset() >= scr_size)
			return false;
		jumpTarget = jmpTarget;
		}
		return true;
	default:
		return false;
	}
}


void SciEngine::scriptDebug() {
	EngineState *s = _gamestate;
	if (_debugState.seeking && !_debugState.breakpointWasHit) { // Are we looking for something special?
		if (_debugState.seeking == kDebugSeekStepOver) {
			// are we above seek-level? resume then
			if (_debugState.seekLevel < (int)s->_executionStack.size())
				return;
			_debugState.seeking = kDebugSeekNothing;
		}

		if (_debugState.seeking != kDebugSeekNothing) {
			const reg32_t pc = s->xs->addr.pc;
			SegmentObj *mobj = s->_segMan->getSegment(pc.getSegment(), SEG_TYPE_SCRIPT);

			if (mobj) {
				Script *scr = (Script *)mobj;
				const byte *code_buf = scr->getBuf();
				uint16 code_buf_size = scr->getBufSize();	// TODO: change to a 32-bit integer for large SCI3 scripts
				int opcode = pc.getOffset() >= code_buf_size ? 0 : code_buf[pc.getOffset()];
				int op = opcode >> 1;
				uint16 paramb1 = pc.getOffset() + 1 >= code_buf_size ? 0 : code_buf[pc.getOffset() + 1];
				uint16 paramf1 = (opcode & 1) ? paramb1 : (pc.getOffset() + 2 >= code_buf_size ? 0 : (int16)READ_SCI11ENDIAN_UINT16(code_buf + pc.getOffset() + 1));

				switch (_debugState.seeking) {
				case kDebugSeekSpecialCallk:
					if (paramb1 != _debugState.seekSpecial)
						return;

				case kDebugSeekCallk:
					if (op != op_callk)
						return;
					break;

				case kDebugSeekLevelRet:
					if ((op != op_ret) || (_debugState.seekLevel < (int)s->_executionStack.size()-1))
						return;
					break;

				case kDebugSeekGlobal:
					if (op < op_sag)
						return;
					if ((op & 0x3) > 1)
						return; // param or temp
					if ((op & 0x3) && s->_executionStack.back().local_segment > 0)
						return; // locals and not running in script.000
					if (paramf1 != _debugState.seekSpecial)
						return; // CORRECT global?
					break;

				default:
					break;
				}

				_debugState.seeking = kDebugSeekNothing;
			}
		}
		// OK, found whatever we were looking for
	}

	debugN("Step #%d\n", s->scriptStepCounter);
	disassemble(s, s->xs->addr.pc, false, true);

	if (_debugState.runningStep) {
		_debugState.runningStep--;
		return;
	}

	_debugState.debugging = false;

	_console->attach();
}

void Kernel::dumpScriptObject(char *data, int seeker, int objsize) {
	int selectors, overloads, selectorsize;
	int species = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 8 + seeker);
	int superclass = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 10 + seeker);
	int namepos = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 14 + seeker);
	int i = 0;

	debugN("Object\n");

	Common::hexdump((unsigned char *) data + seeker, objsize - 4, 16, seeker);
	//-4 because the size includes the two-word header

	debugN("Name: %s\n", namepos ? ((char *)(data + namepos)) : "<unknown>");
	debugN("Superclass: %x\n", superclass);
	debugN("Species: %x\n", species);
	debugN("-info-:%x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 12 + seeker) & 0xffff);

	debugN("Function area offset: %x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + seeker + 4));
	debugN("Selectors [%x]:\n", selectors = (selectorsize = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + seeker + 6)));

	seeker += 8;

	while (selectors--) {
		debugN("  [#%03x] = 0x%x\n", i++, (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker) & 0xffff);
		seeker += 2;
	}

	debugN("Overridden functions: %x\n", selectors = overloads = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker));

	seeker += 2;

	if (overloads < 100)
		while (overloads--) {
			int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + (seeker));

			debugN("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)_selectorNames.size()) ? _selectorNames[selector].c_str() : "<?>");
			debugN("%04x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + selectors*2 + 2) & 0xffff);

			seeker += 2;
		}
}

void Kernel::dumpScriptClass(char *data, int seeker, int objsize) {
	int selectors, overloads, selectorsize;
	int species = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 8 + seeker);
	int superclass = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 10 + seeker);
	int namepos = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + 14 + seeker);

	debugN("Class\n");

	Common::hexdump((unsigned char *) data + seeker, objsize - 4, 16, seeker);

	debugN("Name: %s\n", namepos ? ((char *)data + namepos) : "<unknown>");
	debugN("Superclass: %x\n", superclass);
	debugN("Species: %x\n", species);
	debugN("-info-:%x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + 12 + seeker) & 0xffff);

	debugN("Function area offset: %x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + 4));
	debugN("Selectors [%x]:\n", selectors = (selectorsize = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + 6)));

	seeker += 8;
	selectorsize <<= 1;

	while (selectors--) {
		int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *) data + (seeker) + selectorsize);

		debugN("  [%03x] %s = 0x%x\n", 0xffff & selector, (selector >= 0 && selector < (int)_selectorNames.size()) ? _selectorNames[selector].c_str() : "<?>",
		          (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker) & 0xffff);

		seeker += 2;
	}

	seeker += selectorsize;

	debugN("Overloaded functions: %x\n", selectors = overloads = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker));

	seeker += 2;

	while (overloads--) {
		int selector = (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + (seeker));
		debugN("selector=%d; selectorNames.size() =%d\n", selector, _selectorNames.size());
		debugN("  [%03x] %s: @", selector & 0xffff, (selector >= 0 && selector < (int)_selectorNames.size()) ?
		          _selectorNames[selector].c_str() : "<?>");
		debugN("%04x\n", (int16)READ_SCI11ENDIAN_UINT16((unsigned char *)data + seeker + selectors * 2 + 2) & 0xffff);

		seeker += 2;
	}
}

void Kernel::dissectScript(int scriptNumber, Vocabulary *vocab) {
	int objectctr[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned int _seeker = 0;
	Resource *script = _resMan->findResource(ResourceId(kResourceTypeScript, scriptNumber), 0);

	if (!script) {
		warning("dissectScript(): Script not found!\n");
		return;
	}

	while (_seeker < script->size) {
		int objType = (int16)READ_SCI11ENDIAN_UINT16(script->data + _seeker);
		int objsize;
		unsigned int seeker = _seeker + 4;

		if (!objType) {
			debugN("End of script object (#0) encountered.\n");
			debugN("Classes: %i, Objects: %i, Export: %i,\n Var: %i (all base 10)\n",
			          objectctr[6], objectctr[1], objectctr[7], objectctr[10]);
			return;
		}

		debugN("\n");

		objsize = (int16)READ_SCI11ENDIAN_UINT16(script->data + _seeker + 2);

		debugN("Obj type #%x, size 0x%x: ", objType, objsize);

		_seeker += objsize;

		if (objType >= 0 && objType < ARRAYSIZE(objectctr))
			objectctr[objType]++;

		switch (objType) {
		case SCI_OBJ_OBJECT:
			dumpScriptObject((char *)script->data, seeker, objsize);
			break;

		case SCI_OBJ_CODE:
			debugN("Code\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		case SCI_OBJ_SYNONYMS:
			debugN("Synonyms\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		case SCI_OBJ_SAID:
			debugN("Said\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);

			debugN("%04x: ", seeker);
			vocab->debugDecipherSaidBlock(script->data + seeker);
			debugN("\n");
			break;

		case SCI_OBJ_STRINGS:
			debugN("Strings\n");
			while (script->data [seeker]) {
				debugN("%04x: %s\n", seeker, script->data + seeker);
				seeker += strlen((char *)script->data + seeker) + 1;
			}
			seeker++; // the ending zero byte
			break;

		case SCI_OBJ_CLASS:
			dumpScriptClass((char *)script->data, seeker, objsize);
			break;

		case SCI_OBJ_EXPORTS:
			debugN("Exports\n");
			Common::hexdump((unsigned char *)script->data + seeker, objsize - 4, 16, seeker);
			break;

		case SCI_OBJ_POINTERS:
			debugN("Pointers\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		case 9:
			debugN("<unknown>\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		case SCI_OBJ_LOCALVARS:
			debugN("Local vars\n");
			Common::hexdump(script->data + seeker, objsize - 4, 16, seeker);
			break;

		default:
			debugN("Unsupported!\n");
			return;
		}

	}

	debugN("Script ends without terminator\n");
}

bool SciEngine::checkSelectorBreakpoint(BreakpointType breakpointType, reg_t send_obj, int selector) {
	Common::String methodName = _gamestate->_segMan->getObjectName(send_obj);
	methodName += ("::" + getKernel()->getSelectorName(selector));

	Common::List<Breakpoint>::const_iterator bpIter;
	for (bpIter = _debugState._breakpoints.begin(); bpIter != _debugState._breakpoints.end(); ++bpIter) {
		if ((*bpIter).type == breakpointType && (*bpIter).name == methodName) {
			_console->DebugPrintf("Break on %s (in [%04x:%04x])\n", methodName.c_str(), PRINT_REG(send_obj));
			_debugState.debugging = true;
			_debugState.breakpointWasHit = true;
			return true;
		}
	}
	return false;
}

bool SciEngine::checkExportBreakpoint(uint16 script, uint16 pubfunct) {
	if (_debugState._activeBreakpointTypes & BREAK_EXPORT) {
		uint32 bpaddress = (script << 16 | pubfunct);

		Common::List<Breakpoint>::const_iterator bp;
		for (bp = _debugState._breakpoints.begin(); bp != _debugState._breakpoints.end(); ++bp) {
			if (bp->type == BREAK_EXPORT && bp->address == bpaddress) {
				_console->DebugPrintf("Break on script %d, export %d\n", script, pubfunct);
				_debugState.debugging = true;
				_debugState.breakpointWasHit = true;
				return true;
			}
		}
	}

	return false;
}

void debugSelectorCall(reg_t send_obj, Selector selector, int argc, StackPtr argp, ObjVarRef &varp, reg_t funcp, SegManager *segMan, SelectorType selectorType) {
	int activeBreakpointTypes = g_sci->_debugState._activeBreakpointTypes;
	const char *objectName = segMan->getObjectName(send_obj);
	const char *selectorName = g_sci->getKernel()->getSelectorName(selector).c_str();
	Console *con = g_sci->getSciDebugger();

#ifdef VM_DEBUG_SEND
		debugN("Send to %04x:%04x (%s), selector %04x (%s):", PRINT_REG(send_obj),
			segMan->getObjectName(send_obj), selector,
			g_sci->getKernel()->getSelectorName(selector).c_str());
#endif // VM_DEBUG_SEND

	switch (selectorType) {
	case kSelectorNone:
		debugN("\n");
		break;
	case kSelectorVariable:
#ifdef VM_DEBUG_SEND
		if (argc)
			debugN("Varselector: Write %04x:%04x\n", PRINT_REG(argp[1]));
		else
			debugN("Varselector: Read\n");
#endif // VM_DEBUG_SEND

		// argc == 0: read selector
		// argc == 1: write selector
		// argc can be bigger than 1 in some cases, because of a script bug.
		// Usually, these aren't fatal.
		if ((activeBreakpointTypes & BREAK_SELECTORREAD) ||
			(activeBreakpointTypes & BREAK_SELECTORWRITE) ||
			argc > 1) {

			reg_t selectorValue = *varp.getPointer(segMan);
			if (!argc && (activeBreakpointTypes & BREAK_SELECTORREAD)) {
				if (g_sci->checkSelectorBreakpoint(BREAK_SELECTORREAD, send_obj, selector))
					con->DebugPrintf("Read from selector (%s:%s): %04x:%04x\n",
							objectName, selectorName,
							PRINT_REG(selectorValue));
			} else if (argc && (activeBreakpointTypes & BREAK_SELECTORWRITE)) {
				if (g_sci->checkSelectorBreakpoint(BREAK_SELECTORWRITE, send_obj, selector))
					con->DebugPrintf("Write to selector (%s:%s): change %04x:%04x to %04x:%04x\n",
							objectName, selectorName,
							PRINT_REG(selectorValue), PRINT_REG(argp[1]));
			}

			if (argc > 1)
				debug(kDebugLevelScripts, "Write to selector (%s:%s): change %04x:%04x to %04x:%04x, argc == %d\n",
							objectName, selectorName,
							PRINT_REG(selectorValue), PRINT_REG(argp[1]), argc);
		}
		break;
	case kSelectorMethod:
#ifndef VM_DEBUG_SEND
			if (activeBreakpointTypes & BREAK_SELECTOREXEC) {
				if (g_sci->checkSelectorBreakpoint(BREAK_SELECTOREXEC, send_obj, selector)) {
#else
			if (true) {
				if (true) {
#endif
					con->DebugPrintf("%s::%s(", objectName, selectorName);
					for (int i = 0; i < argc; i++) {
						con->DebugPrintf("%04x:%04x", PRINT_REG(argp[i+1]));
						if (i + 1 < argc)
							con->DebugPrintf(", ");
					}
					con->DebugPrintf(") at %04x:%04x\n", PRINT_REG(funcp));
				}
			}
		break;
	}	// switch
}

void logKernelCall(const KernelFunction *kernelCall, const KernelSubFunction *kernelSubCall, EngineState *s, int argc, reg_t *argv, reg_t result) {
	Kernel *kernel = g_sci->getKernel();
	if (!kernelSubCall) {
		debugN("k%s: ", kernelCall->name);
	} else {
		int callNameLen = strlen(kernelCall->name);
		if (strncmp(kernelCall->name, kernelSubCall->name, callNameLen) == 0) {
			const char *subCallName = kernelSubCall->name + callNameLen;
			debugN("k%s(%s): ", kernelCall->name, subCallName);
		} else {
			debugN("k%s(%s): ", kernelCall->name, kernelSubCall->name);
		}
	}
	for (int parmNr = 0; parmNr < argc; parmNr++) {
		if (parmNr)
			debugN(", ");
		uint16 regType = kernel->findRegType(argv[parmNr]);
		if (regType & SIG_TYPE_NULL)
			debugN("0");
		else if (regType & SIG_TYPE_UNINITIALIZED)
			debugN("UNINIT");
		else if (regType & SIG_IS_INVALID)
			debugN("INVALID");
		else if (regType & SIG_TYPE_INTEGER)
			debugN("%d", argv[parmNr].getOffset());
		else {
			debugN("%04x:%04x", PRINT_REG(argv[parmNr]));
			switch (regType) {
			case SIG_TYPE_OBJECT:
				debugN(" (%s)", s->_segMan->getObjectName(argv[parmNr]));
				break;
			case SIG_TYPE_REFERENCE:
			{
				SegmentObj *mobj = s->_segMan->getSegmentObj(argv[parmNr].getSegment());
				switch (mobj->getType()) {
				case SEG_TYPE_HUNK:
				{
					HunkTable *ht = (HunkTable *)mobj;
					int index = argv[parmNr].getOffset();
					if (ht->isValidEntry(index)) {
						// NOTE: This ", deleted" isn't as useful as it could
						// be because it prints the status _after_ the kernel
						// call.
						debugN(" ('%s' hunk%s)", ht->_table[index].type, ht->_table[index].mem ? "" : ", deleted");
					} else
						debugN(" (INVALID hunk ref)");
					break;
				}
				default:
					// TODO: Any other segment types which could
					// use special handling?

					if (kernelCall->function == kSaid) {
						SegmentRef saidSpec = s->_segMan->dereference(argv[parmNr]);
						if (saidSpec.isRaw) {
							debugN(" ('");
							g_sci->getVocabulary()->debugDecipherSaidBlock(saidSpec.raw);
							debugN("')");
						} else {
							debugN(" (non-raw said-spec)");
						}
					} else {
						debugN(" ('%s')", s->_segMan->getString(argv[parmNr]).c_str());
					}
					break;
				}
			}
			default:
				break;
			}
		}
	}
	if (result.isPointer())
		debugN(" = %04x:%04x\n", PRINT_REG(result));
	else
		debugN(" = %d\n", result.getOffset());
}

} // End of namespace Sci
