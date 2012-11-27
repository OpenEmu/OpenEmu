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

#ifndef KYRA_SCRIPT_H
#define KYRA_SCRIPT_H

#include "common/stream.h"
#include "common/array.h"
#include "common/func.h"
#include "common/iff_container.h"

namespace Kyra {

struct EMCState;
typedef Common::Functor1<EMCState *, int> Opcode;

struct EMCData {
	char filename[13];

	byte *text;
	uint16 *data;
	uint16 *ordr;
	uint16 dataSize;

	const Common::Array<const Opcode *> *sysFuncs;
};

struct EMCState {
	enum {
		kStackSize = 100,
		kStackLastEntry = kStackSize - 1
	};

	const uint16 *ip;
	const EMCData *dataPtr;
	int16 retValue;
	uint16 bp;
	uint16 sp;
	int16 regs[30];           // VM registers
	int16 stack[kStackSize];  // VM stack
};

#define stackPos(x) (script->stack[script->sp+x])
#define stackPosString(x) ((const char *)&script->dataPtr->text[READ_BE_UINT16(&script->dataPtr->text[stackPos(x)<<1])])

class Resource;
class KyraEngine_v1;

class IFFParser : public Common::IFFParser {
public:
	IFFParser(Common::ReadStream &input) : Common::IFFParser(&input) {
		// It seems Westwood missunderstood the 'size' field of the FORM chunk.
		//
		// For EMC scripts (type EMC2) it's filesize instead of filesize - 8,
		// means accidently including the 8 bytes used by the chunk header for the FORM
		// chunk.
		//
		// For TIM scripts (type AVFS) it's filesize - 12 instead of filesize - 8,
		// means it will not include the size of the 'type' field in the FORM chunk,
		// instead of only not including the chunk header size.
		//
		// Both lead to some problems in our IFF parser, either reading after the end
		// of file or producing a "Chunk overread" error message. To work around this
		// we need to adjust the size field properly.

		// Fix for certain Russian fan translations:
		// Westwood's original code completely ignores the FORM chunk and its size
		// setting. After opening a TIM or EMC file they just check whether the FORM
		// chunk exists (as a kind of file type verification) and then immediately seek
		// behind the FORM chunk.
		// This means that their parser is immune to weird fan translation scripts
		// where the file size doesn't match the form chunk size. In our implemetation
		// this would produce "Chunk overread" errors.
		// Westwood also always pads all chunk sizes to 2 byte alignment after reading
		// them from the file (not with FORM though, since they completely ignore it).
		// This seems to do the trick for our FORM chunk size issue with the Russian
		// fan translations. Another method which I have tried and which seems to work
		// well would be simply setting _formChunk.size to the file size (-12 for TIM).

		_formChunk.size = (_formChunk.size + 1) & ~1;

		if (_formType == MKTAG('E','M','C','2'))
			_formChunk.size -= 8;
		else if (_formType == MKTAG('A','V','F','S'))
			_formChunk.size += 4;
	}
};

class EMCInterpreter {
public:
	EMCInterpreter(KyraEngine_v1 *vm);

	bool load(const char *filename, EMCData *data, const Common::Array<const Opcode *> *opcodes);
	void unload(EMCData *data);

	void init(EMCState *scriptState, const EMCData *data);
	bool start(EMCState *script, int function);

	bool isValid(EMCState *script);

	bool run(EMCState *script);
protected:
	KyraEngine_v1 *_vm;
	int16 _parameter;

	const char *_filename;
	EMCData *_scriptData;

	bool callback(Common::IFFChunk &chunk);

	typedef void (EMCInterpreter::*OpcodeProc)(EMCState *);
	struct OpcodeEntry {
		OpcodeProc proc;
		const char *desc;
	};

	const OpcodeEntry *_opcodes;
private:
	void op_jmp(EMCState *);
	void op_setRetValue(EMCState *);
	void op_pushRetOrPos(EMCState *);
	void op_push(EMCState *);
	void op_pushReg(EMCState *);
	void op_pushBPNeg(EMCState *);
	void op_pushBPAdd(EMCState *);
	void op_popRetOrPos(EMCState *);
	void op_popReg(EMCState *);
	void op_popBPNeg(EMCState *);
	void op_popBPAdd(EMCState *);
	void op_addSP(EMCState *);
	void op_subSP(EMCState *);
	void op_sysCall(EMCState *);
	void op_ifNotJmp(EMCState *);
	void op_negate(EMCState *);
	void op_eval(EMCState *);
	void op_setRetAndJmp(EMCState *);
};
} // End of namespace Kyra

#endif
