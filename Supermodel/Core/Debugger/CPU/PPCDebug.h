/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * PPCDebug.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_PPCDEBUG_H
#define INCLUDED_PPCDEBUG_H

#include "Debugger/CPUDebug.h"
#include "CPU/Bus.h"
#include "Types.h"

#define PPCSPECIAL_LR 0
#define PPCSPECIAL_FPSCR 1

namespace Debugger
{
	/*
	 * CCPUDebug implementation for the PowerPC PPC603 emulator.
	 */
	class CPPCDebug : public CCPUDebug, public ::CBus
	{
	private:
		char m_crNames[32][5];
		char m_gprNames[32][4];
		char m_fprNames[32][4];

		::CBus *m_bus;

		UINT8 m_irqState;

	public:
		CPPCDebug(const char *name);

		virtual ~CPPCDebug();

		// CCPUDebug methods

		void AttachToCPU();

		::CBus *AttachBus(::CBus *bus);

		void DetachFromCPU();

		::CBus *DetachBus();

		void CheckException(UINT16 exCode);

		UINT32 GetResetAddr();

		bool UpdatePC(UINT32 pc);

		bool ForceException(CException *ex);

		bool ForceInterrupt(CInterrupt *in);

		UINT64 ReadMem(UINT32 addr, unsigned dataSize);

		bool WriteMem(UINT32 addr, unsigned dataSize, UINT64 data);

		int Disassemble(UINT32 addr, char *mnemonic, char *operands);
		
		EOpFlags GetOpFlags(UINT32 addr, UINT32 opcode);

		bool GetJumpAddr(UINT32 addr, UINT32 opcode, UINT32 &jumpAddr);

		bool GetJumpRetAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr);

		bool GetReturnAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr);

		bool GetHandlerAddr(CException *ex, UINT32 &handlerAddr);

		bool GetHandlerAddr(CInterrupt *in, UINT32 &handlerAddr);

		// CBus methods
		
		UINT8 Read8(UINT32 addr);

		UINT16 Read16(UINT32 addr);

		UINT32 Read32(UINT32 addr);

		UINT64 Read64(UINT32 addr);

		void Write8(UINT32 addr, UINT8 data);

		void Write16(UINT32 addr, UINT16 data);

		void Write32(UINT32 addr, UINT32 data);

		void Write64(UINT32 addr, UINT64 data);
	};
}

#endif	// INCLUDED_PPCDEBUG_H
#endif  // SUPERMODEL_DEBUGGER