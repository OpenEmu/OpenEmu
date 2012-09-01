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
 * Debugger.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "Debugger.h"
#include "CPUDebug.h"

#include <cctype>
#include <string>

namespace Debugger
{
	unsigned CDebugger::GetDataSize(UINT64 data)
	{
		if      (data <= 0xFFULL)             return 1;
		else if (data <= 0xFFFFULL)           return 2;
		else if (data <= 0xFFFFFFULL)         return 3;
		else if (data <= 0xFFFFFFFFULL)       return 4;
		else if (data <= 0xFFFFFFFFFFULL)     return 5;
		else if (data <= 0xFFFFFFFFFFFFULL)   return 6;
		else if (data <= 0xFFFFFFFFFFFFFFULL) return 7;
		else								  return 8;
	}

	const char *CDebugger::GetSizeString(unsigned dataSize)
	{
		switch (dataSize)
		{
			case 1:  return "8-bit";
			case 2:  return "16-bit";
			case 3:  return "24-bit";
			case 4:  return "32-bit";
			case 5:  return "40-bit";
			case 6:  return "48-bit";
			case 7:  return "56-bit";
			case 8:  return "64-bit";
			default: return "";
		}
	}

	UINT64 CDebugger::MaskData(unsigned dataSize, UINT64 data)
	{
		switch (dataSize)
		{
			case 0:  return 0;
			case 1:  return data&0xFFULL;
			case 2:  return data&0xFFFFULL;
			case 3:  return data&0xFFFFFFULL;
			case 4:  return data&0xFFFFFFFFULL;
			case 5:  return data&0xFFFFFFFFFFULL;
			case 6:  return data&0xFFFFFFFFFFFFULL;
			case 7:  return data&0xFFFFFFFFFFFFFFULL;
			default: return data;
		}
	}

	UINT64 CDebugger::GetSlottedData(UINT32 addr, unsigned dataSize, UINT64 data, UINT32 slotAddr, unsigned slotDataSize)
	{
		if (addr == slotAddr && dataSize == slotDataSize)
		{
			// Data perfectly aligned with slot
			return MaskData(slotDataSize, data);
		}
		unsigned overlap, shift;
		if (addr >= slotAddr)
		{
			// Data starts after slot beginning
			overlap = slotAddr + slotDataSize - addr;
			if (overlap < dataSize)
			{
				// Data ends after slot end
				shift = dataSize - overlap;
				return MaskData(overlap, data>>(8 * shift));
			}
			else
			{
				// Data falls completely inside slot
				shift = overlap - dataSize;
				return MaskData(dataSize, data)<<(8 * shift);
			}
		}
		else
		{
			// Data starts before slot beginning
			overlap = addr + dataSize - slotAddr;
			if (overlap < slotDataSize)
			{
				// Data ends before slot end
				shift = slotDataSize - overlap;
				return MaskData(overlap, data)<<(8 * shift);
			}
			else
			{
				// Slot lies completely inside data
				shift = overlap - slotDataSize;
				return MaskData(slotDataSize, data>>(8 * shift));
			}
		}
	}

	bool CDebugger::ParseInt(const char *str, int *val)
	{
		return sscanf(str, "%d", val) == 1;
	}

	CDebugger::CDebugger() : m_exit(false), m_pause(false), m_break(false), m_breakUser(false),
		frameCount(0), logDebug(true), logInfo(true), logError(true)
	{ 
#ifdef DEBUGGER_HASTHREAD
		m_mutex = CThread::CreateMutex();
		m_primaryCPU = NULL;
#endif // DEBUGGER_HASTHREAD
	}

	CDebugger::~CDebugger()
	{	
		DeleteCPUs();
	}

	void CDebugger::DeleteCPUs()
	{
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			delete *it;
		cpus.clear();
	}

	void CDebugger::AddCPU(CCPUDebug *cpu)
	{
		cpu->AttachToDebugger(this);
		cpus.push_back(cpu);
		if (m_break)
			cpu->ForceBreak(m_breakUser);
	}

	void CDebugger::RemoveCPU(CCPUDebug *cpu)
	{
		cpu->DetachFromDebugger(this);
		vector<CCPUDebug*>::iterator it = find(cpus.begin(), cpus.end(), cpu);
		if (it != cpus.end())
			cpus.erase(it);
	}

	CCPUDebug *CDebugger::GetCPU(const char *name)
	{
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
		{
			if (stricmp(name, (*it)->name) == 0)
				return *it;
		}
		return NULL;
	}

	void CDebugger::SetExit()
	{
		m_exit = true;
		
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
		{
			//(*it)->RemoveAllExceptionTraps();
			(*it)->RemoveAllMemWatches();
			(*it)->RemoveAllIOWatches();
			(*it)->RemoveAllBreakpoints();
			(*it)->RemoveAllRegMonitors();
			(*it)->SetContinue();
		}
	}

	void CDebugger::SetPause(bool pause)
	{
		m_pause = pause;
	}

	void CDebugger::ForceBreak(bool user)
	{
		m_break = true;
		m_breakUser = user;
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			(*it)->ForceBreak(user);
	}

	void CDebugger::ClearBreak()
	{
		m_break = false;
		m_breakUser = false;
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			(*it)->ClearBreak();
	}

#ifdef DEBUGGER_HASTHREAD
	bool CDebugger::MakePrimary(CCPUDebug *cpu)
	{
		m_mutex->Lock();

		bool isPrimary = m_primaryCPU == NULL || m_primaryCPU == cpu;
		if (isPrimary)
			m_primaryCPU = cpu;
		
		m_mutex->Unlock();
		
		if (isPrimary)
		{
			for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			{
				if ((*it) != m_primaryCPU)
					(*it)->ForceWait();
			}
			for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			{
				if ((*it) != m_primaryCPU)
					(*it)->WaitForHalt();
			}
		}

		return isPrimary;
	}

	void CDebugger::ReleasePrimary()
	{
		m_mutex->Lock();

		m_primaryCPU = NULL;

		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
		{
			if ((*it) == m_primaryCPU)
				(*it)->ClearBreak();
			else 
				(*it)->ClearWait();
		}

		m_mutex->Unlock();
	}
#endif // DEBUGGER_HASTHREAD

	void CDebugger::Attach()
	{
		AddCPUs();

		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			(*it)->AttachToCPU();
	}

	void CDebugger::Detach()
	{
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			(*it)->DetachFromCPU();

		DeleteCPUs();
	}

	void CDebugger::Reset()
	{
		frameCount = 0;
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			(*it)->DebuggerReset();
	}

	void CDebugger::Poll()
	{
		frameCount++;
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
			(*it)->DebuggerPolled();
	}

	void CDebugger::PrintEvent(CCPUDebug *cpu, const char *fmt, ...)
	{
		va_list vl;
		va_start(vl, fmt);
		Log(cpu, NULL, fmt, vl);
		va_end(vl);
	}

#ifdef DEBUGGER_HASLOGGER
	void CDebugger::DebugLog(const char *fmt, va_list vl)
	{
		if (logDebug)
			Log(NULL, "Debug", fmt, vl);
	}

	void CDebugger::InfoLog(const char *fmt, va_list vl)
	{
		if (logInfo)
			Log(NULL, "Info", fmt, vl);
	}

	void CDebugger::ErrorLog(const char *fmt, va_list vl)
	{
		if (logError)
			Log(NULL, "Error", fmt, vl);
	}
#endif // DEBUGGER_HASLOGGER

	bool CDebugger::ParseData(const char *str, EFormat format, unsigned dataSize, UINT64 *data)
	{
		unsigned i;
		char upperStr[255];
		const char *p;
		switch (format)
		{
			case Hex:
			case Hex0x:
			case HexDollar:
			case HexPostH:
				for (i = 0; str[i] && i < 254; i++)
					upperStr[i] = toupper(str[i]);
				upperStr[i] = '\0';
				if (sscanf(upperStr, "%llX", data) != 1 && 
					sscanf(upperStr, "0x%llX", data) != 1 &&
					sscanf(upperStr, "$%llX", data) != 1 &&
					sscanf(upperStr, "%llXH", data) != 1)
					return false;
				break;
			case Decimal:
				if (sscanf(str, "%llu", data) != 1)
					return false;
				break;
			case Binary:
				p = str;
				while (*p != '\0' && *p != '0' && *p != '1')
					p++;
				if (*p == '\0')
					return false;
				*data = 0;
				for (i = 0; i < dataSize * 8; i++)
				{
					if (*p == '1')
						*data |= 1;
					else if (*p != '0')
						break;
					*data <<= 1;
					p++;
				}
				break;
			case ASCII:
				// TODO
				return false;
			default:
				return false;
		}
		switch (dataSize)
		{
			case 0:  return 0;
			case 1:  return *data <= (UINT64)0xFF;
			case 2:  return *data <= (UINT64)0xFFFF;
			case 3:  return *data <= (UINT64)0xFFFFFF;
			case 4:  return *data <= (UINT64)0xFFFFFFFF;
			case 5:  return *data <= (UINT64)0xFFFFFFFFFF;
			case 6:  return *data <= (UINT64)0xFFFFFFFFFFFF;
			case 7:  return *data <= (UINT64)0xFFFFFFFFFFFFFF;
			default: return true;
		}
	}

	void CDebugger::FormatData(char *str, EFormat format, unsigned dataSize, INT64 data)
	{
		if (data >= 0)
			FormatData(str, format, dataSize, (UINT64)data);
		else
		{
			*str++ = '-';
			FormatData(str, format, dataSize, (UINT64)(data >= 0 ? data : -data));  // No portable abs64 available
		}
	}

	void CDebugger::FormatData(char *str, EFormat format, unsigned dataSize, UINT64 data)
	{
		data = MaskData(dataSize, data);
		unsigned i;
		switch (format)
		{
			case Hex:       sprintf(str, "%0*llX", (int)dataSize * 2, data); break;
			case Hex0x:     sprintf(str, "0x%0*llX", (int)dataSize * 2, data); break;
			case HexDollar: sprintf(str, "$%0*llX", (int)dataSize * 2, data); break;
			case HexPostH: 	sprintf(str, "%0*llXh", (int)dataSize * 2, data); break;
			case Decimal:  	sprintf(str, "%llu", data); break;
			case Binary:
				str += dataSize * 8;
				*str-- = '\0';
				for (i = 0; i < dataSize * 8; i++)
				{
					*str-- = ((data&1) ? '1' : '0');
					data >>= 1;
				}
				break;
			case ASCII:
				for (i = 0; i < dataSize; i++)
				{
					*str++ = (char)(data & 0xFF);
					data >>= 8;
				}
				*str++ = '\0';
				break;
			default: 
				str[0] = '\0';
				break;
		}
	}

	bool CDebugger::HasState()
	{
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
		{
			if ((*it)->labels.size() + (*it)->comments.size() > 0)
				return true;
		}
		return false;
	}

	bool CDebugger::LoadState(const char *fileName)
	{
#ifdef DEBUGGER_HASBLOCKFILE
		// Open file and find header 
		CBlockFile state;
		if (state.Load(fileName) != OKAY)
			return false;
		if (state.FindBlock("Debugger State") != OKAY)
		{
			state.Close();
			return false;
		}

		// Check version in header matches
		unsigned version;
		state.Read(&version, sizeof(version));
		if (version != DEBUGGER_STATEFILE_VERSION)
		{
			state.Close();
			return false;
		}

		// Load debugger state
		if (!LoadState(&state))
		{
			state.Close();
			return false;
		}

		// Load state for each CPU
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
		{
			if (!(*it)->LoadState(&state))
			{
				state.Close();
				return false;
			}
		}

		state.Close();
		return true;
#else
		return false;
#endif // DEBUGGER_HASBLOCKFILE
	}

	bool CDebugger::SaveState(const char *fileName)
	{
#ifdef DEBUGGER_HASBLOCKFILE
		// Create file with header
		CBlockFile state;
		if (state.Create(fileName, "Debugger State", __FILE__) != OKAY)
			return false;

		// Write out version in header
		unsigned version = DEBUGGER_STATEFILE_VERSION;
		state.Write(&version, sizeof(version));

		// Save debugger state
		if (!SaveState(&state))
		{
			state.Close();
			return false;
		}

		// Save state for each CPU
		for (vector<CCPUDebug*>::iterator it = cpus.begin(); it != cpus.end(); it++)
		{
			if (!(*it)->SaveState(&state))
				return false;
		}

		state.Close();
		return true;
#else
		return false;
#endif // DEBUGGER_HASBLOCKFILE
	}

#ifdef DEBUGGER_HASBLOCKFILE
	bool CDebugger::LoadState(CBlockFile *state)
	{
		return true;
	}

	bool CDebugger::SaveState(CBlockFile *state)
	{
		return true;
	}
#endif
}

#endif  // SUPERMODEL_DEBUGGER
