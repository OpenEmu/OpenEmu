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
 * Breakpoint.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "CPUDebug.h"
#include "Breakpoint.h"

#include <cstdio>

namespace Debugger
{
	CBreakpoint::CBreakpoint(CCPUDebug *bpCPU, int bpAddr, char bpSymbol, const char *bpType) : 
		CAddressRef(bpCPU, bpAddr), symbol(bpSymbol), type(bpType), active(true)
	{
		//
	}

	bool CBreakpoint::Check(UINT32 pc, UINT32 opcode)
	{
		return CheckAddr(pc) && active && CheckBreak(pc, opcode);
	}

	void CBreakpoint::Reset()
	{
		//
	}

	CSimpleBreakpoint::CSimpleBreakpoint(CCPUDebug *bpCPU, int bpAddr) : CBreakpoint(bpCPU, bpAddr, '*', "simple")
	{
		//
	}

	bool CSimpleBreakpoint::CheckBreak(UINT32 pc, UINT32 opcode) 
	{
		return true;
	}

	bool CSimpleBreakpoint::GetInfo(char *str)
	{
		return false;
	}

	CCountBreakpoint::CCountBreakpoint(CCPUDebug *bpCPU, int bpAddr, int count) : CBreakpoint(bpCPU, bpAddr, '+', "count"), 
		m_count(count), m_counter(0)
	{
		//
	}

	bool CCountBreakpoint::CheckBreak(UINT32 pc, UINT32 opcode) 
	{
		return ++m_counter == m_count;
	}

	void CCountBreakpoint::Reset()
	{
		m_counter = 0;
	}

	bool CCountBreakpoint::GetInfo(char *str)
	{
		sprintf(str, "%d / %d", m_counter, m_count); 
		return true;
	}

	CPrintBreakpoint::CPrintBreakpoint(CCPUDebug *bpCPU, int bpAddr) : CBreakpoint(bpCPU, bpAddr, 'P', "print")
	{
		//
	}

	bool CPrintBreakpoint::CheckBreak(UINT32 pc, UINT32 opcode) 
	{
		char addrStr[50];
		cpu->FormatAddress(addrStr, addr, true);
		cpu->debugger->PrintEvent(cpu, "Breakpoint #%u hit at %s.\n", num, addrStr);
		return false;
	}

	void CPrintBreakpoint::Reset()
	{
		//
	}

	bool CPrintBreakpoint::GetInfo(char *str)
	{
		return false;
	}
}

#endif  // SUPERMODEL_DEBUGGER
