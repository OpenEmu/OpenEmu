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
 * Musashi68KDebug.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "Musashi68KDebug.h"

#include <cctype>
#include <string>

//namespace Debugger
//{
	UINT32 CMusashi68KDebug::GetReg(CCPUDebug *cpu, unsigned id)
	{
		CMusashi68KDebug *m68K = (CMusashi68KDebug*)cpu;
		return M68KGetRegister(m68K->m_ctx, id);
	}

	bool CMusashi68KDebug::SetReg(CCPUDebug *cpu, unsigned id, UINT32 data)
	{
		CMusashi68KDebug *m68K = (CMusashi68KDebug*)cpu;
		m68K->SetM68KContext();
		bool okay = M68KSetRegister(m68K->m_ctx, id, data);
		m68K->RestoreM68KContext();
		return okay;
	}

	static const char *srGroup = "Special Registers";
	static const char *drGroup = "Data Registers";
	static const char *arGroup = "Address Regsters";

	CMusashi68KDebug::CMusashi68KDebug(const char *name, M68KCtx *ctx) : C68KDebug(name), m_ctx(ctx), m_resetAddr(0)
	{
		// Special registers
		AddPCRegister      ("PC", srGroup);
		AddAddrRegister    ("SP", srGroup, DBG68K_REG_SP, GetReg, SetReg);
		AddStatus32Register("SR", srGroup, DBG68K_REG_SR, "TtSM.210...XNZVC", GetReg, SetReg);

		// Data registers
		AddInt32Register("D0", drGroup, DBG68K_REG_D0, GetReg, SetReg);
		AddInt32Register("D1", drGroup, DBG68K_REG_D1, GetReg, SetReg);
		AddInt32Register("D2", drGroup, DBG68K_REG_D2, GetReg, SetReg);
		AddInt32Register("D3", drGroup, DBG68K_REG_D3, GetReg, SetReg);
		AddInt32Register("D4", drGroup, DBG68K_REG_D4, GetReg, SetReg);
		AddInt32Register("D5", drGroup, DBG68K_REG_D5, GetReg, SetReg);
		AddInt32Register("D6", drGroup, DBG68K_REG_D6, GetReg, SetReg);
		AddInt32Register("D7", drGroup, DBG68K_REG_D7, GetReg, SetReg);

		// Address registers
		AddInt32Register("A0", arGroup, DBG68K_REG_A0, GetReg, SetReg);
		AddInt32Register("A1", arGroup, DBG68K_REG_A1, GetReg, SetReg);
		AddInt32Register("A2", arGroup, DBG68K_REG_A2, GetReg, SetReg);
		AddInt32Register("A3", arGroup, DBG68K_REG_A3, GetReg, SetReg);
		AddInt32Register("A4", arGroup, DBG68K_REG_A4, GetReg, SetReg);
		AddInt32Register("A5", arGroup, DBG68K_REG_A5, GetReg, SetReg);
		AddInt32Register("A6", arGroup, DBG68K_REG_A6, GetReg, SetReg);
		AddInt32Register("A7", arGroup, DBG68K_REG_A7, GetReg, SetReg);
	}

	CMusashi68KDebug::~CMusashi68KDebug()
	{
		DetachFromCPU();	
	}

	UINT32 CMusashi68KDebug::GetSP()
	{
		return M68KGetRegister(m_ctx, DBG68K_REG_SP);
	}

	void CMusashi68KDebug::AttachToCPU()
	{
		if (m_ctx->Debug != NULL)
			DetachFromCPU();
		m_ctx->Debug = this;
		m_bus = m_ctx->Bus;
		m_ctx->Bus = this;

		// Reset address is held at 0x000004
		m_resetAddr = m_bus->Read32(0x000004);
	}

	void CMusashi68KDebug::DetachFromCPU()
	{
		if (m_ctx->Debug == NULL)
			return;
		m_ctx->Bus = m_bus;
		m_bus = NULL;
		m_ctx->Debug = NULL;
	}

	UINT32 CMusashi68KDebug::GetResetAddr()
	{
		return m_resetAddr;
	}

	bool CMusashi68KDebug::UpdatePC(UINT32 newPC)
	{ 
		// TODO
		return false;
	}

	bool CMusashi68KDebug::ForceException(CException *ex)
	{
		// TODO
		return false;
	}

	bool CMusashi68KDebug::ForceInterrupt(CInterrupt *in)
	{
		if (in->code > 6)
			return false;
		M68KSetIRQ(in->code + 1);
		return true;
	}

	UINT64 CMusashi68KDebug::ReadMem(UINT32 addr, unsigned dataSize)
	{
		switch (dataSize)
		{
			case 1:  return (UINT64)m_bus->Read8(addr);
			case 2:  return (UINT64)m_bus->Read16(addr);
			case 4:  return (UINT64)m_bus->Read32(addr);
			default: return CCPUDebug::ReadMem(addr, dataSize);
		}
	}

	bool CMusashi68KDebug::WriteMem(UINT32 addr, unsigned dataSize, UINT64 data)
	{
		switch (dataSize)
		{
			case 1:  m_bus->Write8(addr, (UINT8)data); return true;
			case 2:  m_bus->Write16(addr, (UINT16)data); return true;
			case 4:  m_bus->Write32(addr, (UINT32)data); return true;
			case 8:  return CCPUDebug::WriteMem(addr, dataSize, data);
			default: return false;
		}
	}

	// CBus methods

	UINT8 CMusashi68KDebug::Read8(UINT32 addr)
	{
		UINT8 data = m_bus->Read8(addr);
		CheckRead8(addr, data);
		return data;
	}

	void CMusashi68KDebug::Write8(UINT32 addr, UINT8 data)
	{
		m_bus->Write8(addr, data);
		CheckWrite8(addr, data);
	}

	UINT16 CMusashi68KDebug::Read16(UINT32 addr)
	{
		UINT16 data = m_bus->Read16(addr);
		CheckRead16(addr, data);
		return data;
	}

	void CMusashi68KDebug::Write16(UINT32 addr, UINT16 data)
	{
		m_bus->Write16(addr, data);
		CheckWrite16(addr, data);
	}

	UINT32 CMusashi68KDebug::Read32(UINT32 addr)
	{
		UINT32 data = m_bus->Read32(addr);
		CheckRead32(addr, data);
		return data;
	}

	void CMusashi68KDebug::Write32(UINT32 addr, UINT32 data)
	{
		m_bus->Write32(addr, data);
		CheckWrite32(addr, data);
	}
//}

#endif  // SUPERMODEL_DEBUGGER