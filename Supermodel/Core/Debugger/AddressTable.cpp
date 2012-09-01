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
 * AddressTable.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "AddressTable.h"

#include <cstring>
#include <string>

namespace Debugger
{
	CAddressRef::CAddressRef(CCPUDebug *refCPU, UINT32 refAddr, UINT32 refSize) : 
		cpu(refCPU), addr(refAddr), size(refSize), addrEnd(refAddr + refSize - 1)
	{
		//
	}
		
	CAddressTable::CAddressTable() : m_count(0)
	{
		memset(m_tables, NULL, sizeof(m_tables));
	}

	CAddressTable::~CAddressTable()
	{
		Clear();
	}

	CAddressRef **CAddressTable::GetTableCreate(UINT32 addr, int &tableIndex, unsigned &indexInTable)
	{
		// Convert address to table index
		tableIndex = addr >> INDEX_SHIFT;
		// See if have a table at index
		if (m_tables[tableIndex] == NULL)
		{
			// If not, create one now and increase count
			m_tables[tableIndex] = new CAddressRef*[TABLE_SIZE];
			memset(m_tables[tableIndex], NULL, sizeof(CAddressRef*) * TABLE_SIZE);
			m_count++;
		}
		// Calculate index within table
		indexInTable = addr & TABLE_MASK;
		return m_tables[tableIndex];
	}
		
	void CAddressTable::CheckAndReleaseTable(int tableIndex)
	{
		// See if have table at give index
		CAddressRef **table = m_tables[tableIndex];
		if (table == NULL)
			return;
		// If so, check if it is empty
		for (int i = 0; i < TABLE_SIZE; i++)
		{
			if (table[i] != NULL)
				return;
		}
		// If so, delete it and decrease count
		delete[] table;
		m_tables[tableIndex] = NULL;
		m_count--;
	}

	bool CAddressTable::IsEmpty()
	{
		return m_count == 0;
	} 

	void CAddressTable::Clear()
	{
		if (m_count == 0)
			return;
		// Delete all tables and reset count
		for (int i = 0; i < NUM_TABLES; i++)
		{
			if (m_tables[i] != NULL)
			{
				delete[] m_tables[i];
				m_tables[i] = NULL;
			}
		}
		m_count = 0;
	}

	void CAddressTable::Add(CAddressRef *value)
	{
		int tableIndex;
		unsigned indexInTable;
		CAddressRef **table = GetTableCreate(value->addr, tableIndex, indexInTable);
		for (UINT32 i = 0; i < value->size; i++)
		{
			table[indexInTable] = value;
			if (indexInTable >= TABLE_SIZE - 1)
				table = GetTableCreate(value->addr + i, tableIndex, indexInTable);
			else
				indexInTable++;
		}
	}

	bool CAddressTable::Remove(CAddressRef *value)
	{
		int tableIndex;
		unsigned indexInTable;
		CAddressRef **table = GetTableNoCreate(value->addr, tableIndex, indexInTable);
		bool removed = false;
		for (UINT32 i = 0; i < value->size; i++)
		{
			if (table != NULL)
			{
				removed |= table[indexInTable] != NULL;
				table[indexInTable] = NULL;
			}
			if (indexInTable >= TABLE_SIZE - 1)
			{
				CheckAndReleaseTable(tableIndex);
				table = GetTableNoCreate(value->addr + i, tableIndex, indexInTable);
			}
			else
				indexInTable++;
		}
		CheckAndReleaseTable(tableIndex);
		return removed;
	}
}

#endif  // SUPERMODEL_DEBUGGER
