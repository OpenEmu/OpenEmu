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
 * Breakpoint.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_BREAKPOINT_H
#define INCLUDED_BREAKPOINT_H

#include "AddressTable.h"
#include "Types.h"

namespace Debugger
{
	class CCPUDebug;

	/*
	 * Base class for a breakpoint.
	 */
	class CBreakpoint : public CAddressRef
	{
	protected:
		CBreakpoint(CCPUDebug *bpCPU, int bpAddr, char bpSymbol, const char *bpType);

	public:
		const char symbol;
		const char *type;

		unsigned num;

		bool active;

		bool Check(UINT32 pc, UINT32 opcode);

		virtual bool CheckBreak(UINT32 pc, UINT32 opcode) = 0;
		
		virtual void Reset();

		virtual bool GetInfo(char *str) = 0;
	};

	/*
	 * Simple breakpoint that will always halt execution when hit.
	 */
	class CSimpleBreakpoint : public CBreakpoint
	{
	public:
		CSimpleBreakpoint(CCPUDebug *bpCPU, int bpAddr);

		bool CheckBreak(UINT32 pc, UINT32 opcode);

		bool GetInfo(char *str);
	};

	/*
	 * Count breakpoint that will halt execution after it has been hit a certain number of times.
	 */
	class CCountBreakpoint : public CBreakpoint
	{
	private:
		int m_count;
		int m_counter;

	public:
		CCountBreakpoint(CCPUDebug *bpCPU, int bpAddr, int count);

		bool CheckBreak(UINT32 pc, UINT32 opcode);

		void Reset();

		bool GetInfo(char *str);
	};

	class CPrintBreakpoint : public CBreakpoint
	{
	public:
		CPrintBreakpoint(CCPUDebug *bpCPU, int bpAddr);

		bool CheckBreak(UINT32 pc, UINT32 opcode);

		void Reset();

		bool GetInfo(char *str);
	};

	//class CConditionBreakpoint : public CBrekapoint
	//{
	//	// TODO
	//}
}

#endif	// INCLUDED_BREAKPOINT_H
#endif  // SUPERMODEL_DEBUGGER