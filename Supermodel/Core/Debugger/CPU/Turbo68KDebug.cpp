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
 * Turbo68KDebug.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "Turbo68KDebug.h"

#include <cctype>
#include <string>

namespace Debugger
{
	UINT32 GetSpecialReg(CCPUDebug *cpu, unsigned id)
	{
		switch (id)
		{
			case TBO68K_REG_SP: return (UINT32)turbo68kcontext_68000.a[7];
			case TBO68K_REG_SR: return (UINT32)turbo68kcontext_68000.sr;
			default:            return 0;
		}
	}

	bool SetSpecialReg(CCPUDebug *cpu, unsigned id, UINT32 data)
	{
		switch (id)
		{
			case TBO68K_REG_SP: turbo68kcontext_68000.a[7] = data; return true;
			case TBO68K_REG_SR: turbo68kcontext_68000.sr = data; return true;
			default:            return false;
		}
	}

	UINT32 GetDataReg(CCPUDebug *cpu, unsigned id) 
	{
		return (UINT32)turbo68kcontext_68000.d[id];
	}

	bool SetDataReg(CCPUDebug *cpu, unsigned id, UINT32 data) 
	{
		turbo68kcontext_68000.d[id] = data;
		return true;
	}

	UINT32 GetAddressReg(CCPUDebug *cpu, unsigned id)
	{
		return (UINT32)turbo68kcontext_68000.a[id];
	}

	bool SetAddressReg(CCPUDebug *cpu, unsigned id, UINT32 data)
	{
		turbo68kcontext_68000.a[id] = data;
		return true;
	}

	static const char *srGroup = "Special Registers";
	static const char *drGroup = "Data Registers";
	static const char *arGroup = "Address Regsters";

	CTurbo68KDebug::CTurbo68KDebug(const char *name) : C68KDebug(name), m_resetAddr(0)
	{
		// Special registers
		AddPCRegister      ("PC", srGroup);
		AddAddrRegister    ("SP", srGroup, TBO68K_REG_SP, GetSpecialReg, SetSpecialReg);
		AddStatus32Register("SR", srGroup, TBO68K_REG_SR, "TtSM.210...XNZVC", GetSpecialReg, SetSpecialReg);

		// Data registers
		for (unsigned id = 0; id < 8; id++)
		{
			sprintf(m_drNames[id], "D%u", id);
			AddInt32Register(m_drNames[id], drGroup, id, GetDataReg, SetDataReg);
		}

		// Address registers
		for (unsigned id = 0; id < 8; id++)
		{
			sprintf(m_arNames[id], "A%u", id);
			AddInt32Register(m_arNames[id], arGroup, id, GetAddressReg, SetAddressReg);
		}
	}

	CTurbo68KDebug::~CTurbo68KDebug()
	{
		DetachFromCPU();	
	}

	bool __cdecl DebugHandler(TURBO68K_INT32 pc, TURBO68K_INT32 opcode)
	{
		// Return true to let Turbo68K know if PC was changed by user
		return debug->CPUExecute((UINT32)pc, (UINT32)opcode, 1); // TODO - lastCycles
	}

	void __cdecl InterruptHandler(TURBO68K_UINT32 intVec)
	{
		// TODO - is following correct handling of interrupts?  - need to check readme file
		if (intVec > 47)
			return;
		else if (intVec >= 25 || intVec < 32)
		{
			debug->CPUException(25); 
			debug->CPUInterrupt((UINT16)intVec - 25);
		}
		else 
			debug->CPUException((UINT16)intVec);
		
		if (origIntAckPtr != NULL)
			origIntAckPtr(intVec);
	}

	void CTurbo68KDebug::AttachToCPU()
	{
		if (debug != NULL)
			DetachFromCPU();

		debug = this;

		origDebugPtr = (DebugPtr)turbo68kcontext_68000.Debug;
		origIntAckPtr = (IntAckPtr)turbo68kcontext_68000.InterruptAcknowledge;

		origRead8Regions = (TURBO68K_DATAREGION*)Turbo68KGetReadByte(TURBO68K_NULL);
		origRead16Regions = (TURBO68K_DATAREGION*)Turbo68KGetReadWord(TURBO68K_NULL);
		origRead32Regions = (TURBO68K_DATAREGION*)Turbo68KGetReadLong(TURBO68K_NULL);
		origWrite8Regions = (TURBO68K_DATAREGION*)Turbo68KGetWriteByte(TURBO68K_NULL);
		origWrite16Regions = (TURBO68K_DATAREGION*)Turbo68KGetWriteWord(TURBO68K_NULL);
		origWrite32Regions = (TURBO68K_DATAREGION*)Turbo68KGetWriteLong(TURBO68K_NULL);

		turbo68kcontext_68000.Debug = DebugHandler;
		turbo68kcontext_68000.InterruptAcknowledge = InterruptHandler;

		debugRead8Regions[0].handler = ReadByteDebug;
		debugRead16Regions[0].handler = ReadWordDebug;
		debugRead32Regions[0].handler = ReadLongDebug;
		debugWrite8Regions[0].handler = WriteByteDebug;
		debugWrite16Regions[0].handler = WriteWordDebug;
		debugWrite32Regions[0].handler = WriteLongDebug;

		Turbo68KSetReadByte(debugRead8Regions, TURBO68K_NULL);
		Turbo68KSetReadWord(debugRead16Regions, TURBO68K_NULL);
		Turbo68KSetReadLong(debugRead32Regions, TURBO68K_NULL);
		Turbo68KSetWriteByte(debugWrite8Regions, TURBO68K_NULL);
		Turbo68KSetWriteWord(debugWrite16Regions, TURBO68K_NULL);
		Turbo68KSetWriteLong(debugWrite32Regions, TURBO68K_NULL);

		// Reset address is held at 0x000004
		m_resetAddr = ReadLongDirect(0x000004);
	}

	void CTurbo68KDebug::DetachFromCPU()
	{
		if (debug == NULL)
			return;

		turbo68kcontext_68000.Debug = origDebugPtr;
		turbo68kcontext_68000.InterruptAcknowledge = origIntAckPtr;

		Turbo68KSetReadByte(origRead8Regions, TURBO68K_NULL);
		Turbo68KSetReadWord(origRead16Regions, TURBO68K_NULL);
		Turbo68KSetReadLong(origRead32Regions, TURBO68K_NULL);
		Turbo68KSetWriteByte(origWrite8Regions, TURBO68K_NULL);
		Turbo68KSetWriteWord(origWrite16Regions, TURBO68K_NULL);
		Turbo68KSetWriteLong(origWrite32Regions, TURBO68K_NULL);

		debug = NULL;
	}

	UINT32 CTurbo68KDebug::GetResetAddr()
	{
		return m_resetAddr;
	}

	UINT32 CTurbo68KDebug::GetSP()
	{
		return (UINT32)turbo68kcontext_68000.a[7];
	}

	bool CTurbo68KDebug::UpdatePC(UINT32 newPC)
	{ 
		turbo68kcontext_68000.pc = newPC;
		return true;
	}

	bool CTurbo68KDebug::ForceException(CException *ex)
	{
		//switch (ex->code)
		//{
		//	 TODO
		//}
		return false;
	}

	bool CTurbo68KDebug::ForceInterrupt(CInterrupt *in)
	{
		if (in->code > 6)
			return false;
		//Turbo68KInterrupt(in->code, TURBO64K_AUTOVECTOR);
		//Turbo68KProcessInterrupts(); TODO - call this?
		return false; // TODO
	}

	UINT64 CTurbo68KDebug::ReadMem(UINT32 addr, unsigned dataSize)
	{
		switch (dataSize)
		{
			case 1:  return ReadByteDirect(addr);
			case 2:  return ReadWordDirect(addr);
			case 4:  return ReadLongDirect(addr);
			default: return CCPUDebug::ReadMem(addr, dataSize);	
		}
	}

	bool CTurbo68KDebug::WriteMem(UINT32 addr, unsigned dataSize, UINT64 data)
	{
		switch (dataSize)
		{
			case 1:  WriteByteDirect(addr, (TURBO68K_UINT8)data); return true;
			case 2:  WriteWordDirect(addr, (TURBO68K_UINT16)data); return true;
			case 4:  WriteLongDirect(addr, (TURBO68K_UINT32)data); return true;
			default: return CCPUDebug::WriteMem(addr, dataSize, data);
		}
	}

	TURBO68K_UINT8 __cdecl ReadByteDebug(TURBO68K_UINT32 addr)
	{
		TURBO68K_UINT8 data = ReadByteDirect(addr);
		debug->CheckRead8((UINT32)addr, (UINT8)data);
		return data;
	}

	TURBO68K_UINT16 __cdecl ReadWordDebug(TURBO68K_UINT32 addr)
	{
		TURBO68K_UINT16 data = ReadWordDirect(addr);
		debug->CheckRead16((UINT32)addr, (UINT16)data);
		return data;
	}

	TURBO68K_UINT32 __cdecl ReadLongDebug(TURBO68K_UINT32 addr)
	{
		TURBO68K_UINT32 data = ReadLongDirect(addr);
		debug->CheckRead32((UINT32)addr, (UINT32)data);
		return data;
	}

	void __cdecl WriteByteDebug(TURBO68K_UINT32 addr, TURBO68K_UINT8 data)
	{
		WriteByteDirect(addr, data);
		debug->CheckWrite8((UINT32)addr, (UINT8)data);
	}

	void __cdecl WriteWordDebug(TURBO68K_UINT32 addr, TURBO68K_UINT16 data)
	{
		WriteWordDirect(addr, data);
		debug->CheckWrite16((UINT32)addr, (UINT16)data);
	}

	void __cdecl WriteLongDebug(TURBO68K_UINT32 addr, TURBO68K_UINT32 data)
	{
		WriteLongDirect(addr, data);
		debug->CheckWrite32((UINT32)addr, (UINT32)data);
	}
}

#endif  // SUPERMODEL_DEBUGGER