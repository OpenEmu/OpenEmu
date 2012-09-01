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
 * AddressTable.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_ADDRESSTABLE_H
#define INCLUDED_ADDRESSTABLE_H

#include "Types.h"

#include <vector>
#include <algorithm>
using namespace std;

#define TABLE_WIDTH 16
#define TABLE_SIZE (1 << TABLE_WIDTH)
#define TABLE_MASK (TABLE_SIZE - 1)
#define INDEX_SHIFT TABLE_WIDTH
#define NUM_TABLES (0x100000000ULL / TABLE_SIZE)

namespace Debugger
{
	class CCPUDebug;

	/*
	 * Base class for objects that reference an address, such as a label or a comment.
	 */
	class CAddressRef
	{
	protected:
		CAddressRef(CCPUDebug *refCPU, UINT32 refAddr, UINT32 refSize = 1);

	public:
		CCPUDebug *cpu;
		const UINT32 addr;
		const UINT32 size;
		const UINT32 addrEnd;

		bool CheckAddr(UINT32 loc);

		// TODO - implement operators
	};

	/*
	 * Class that holds a table of address references.  It does so in an a memory-expensive manner but with the result that it 
	 * performs better (hopefully!) as lookups are constant-time.
	 */
	class CAddressTable
	{
	private:
		int m_count;
		CAddressRef **m_tables[NUM_TABLES];

		CAddressRef **GetTableNoCreate(UINT32 addr, int &tableIndex, unsigned &indexInTable);

		CAddressRef **GetTableCreate(UINT32 addr, int &tableIndex, unsigned &indexInTable);
		
		void CheckAndReleaseTable(int tableIndex);

	public:
		CAddressTable();

		~CAddressTable();

		bool IsEmpty();

		void Clear();

		CAddressRef *Get(UINT32 addr);

		CAddressRef *Get(UINT32 addr, UINT32 size);

		void Add(CAddressRef *value);

		bool Remove(CAddressRef *value);
	};

	//
	// Inlined methods
	//

	inline bool CAddressRef::CheckAddr(UINT32 loc)
	{
		return addr <= loc && loc <= addrEnd;
	}

	inline CAddressRef **CAddressTable::GetTableNoCreate(UINT32 addr, int &tableIndex, unsigned &indexInTable)
	{
		tableIndex = addr >> INDEX_SHIFT;
		if (m_tables[tableIndex] == NULL)
			return NULL;
		indexInTable = addr & TABLE_MASK;
		return m_tables[tableIndex];
	}

	inline CAddressRef *CAddressTable::Get(UINT32 addr)
	{
		int tableIndex;
		unsigned indexInTable;
		CAddressRef **table = GetTableNoCreate(addr, tableIndex, indexInTable);
		if (table == NULL)
			return NULL;
		return table[indexInTable];
	}

	inline CAddressRef *CAddressTable::Get(UINT32 addr, UINT32 size)
	{
		int tableIndex;
		unsigned indexInTable;
		CAddressRef **table = GetTableNoCreate(addr, tableIndex, indexInTable);
		for (UINT32 i = 0; i < size; i++)
		{
			if (table != NULL)
			{
				CAddressRef *ref = table[indexInTable];
				if (ref != NULL)
					return ref;
				addr++;
				if (indexInTable >= TABLE_SIZE - 1)
					table = GetTableNoCreate(addr, tableIndex, indexInTable);
				else
					indexInTable++;
			}
			else
			{
				addr++;
				table = GetTableNoCreate(addr, tableIndex, indexInTable);
			}
		}
		return NULL;
	}
}

#endif	// INCLUDED_ADDRESSTABLE_H
#endif  // SUPERMODEL_DEBUGGER
