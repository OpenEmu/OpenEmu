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
 * Watch.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "CPUDebug.h"
#include "Debugger.h"
#include "IO.h"
#include "Watch.h"

#include <string>

namespace Debugger
{
	UINT32 CWatch::GetIOAddress(CIO *io)
	{
		CMappedIO *mappedIO = dynamic_cast<CMappedIO*>(io);
		return (mappedIO != NULL ? mappedIO->addr : 0);
	}

	CWatch::CWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, char wSymbol, const char *wType, bool wTrigRead, bool wTrigWrite) : 
		CAddressRef(wCPU, wAddr, wSize), io(NULL), symbol(wSymbol), type(wType), trigRead(wTrigRead), trigWrite(wTrigWrite), active(true),
		readCount(0), writeCount(0)
	{
		//
	}
		
	CWatch::CWatch(CCPUDebug *wCPU, CIO *wIO, char wSymbol, const char *wType, bool wTrigInput, bool mTrigOutput) : 
		CAddressRef(wCPU, GetIOAddress(wIO), wIO->dataSize), io(wIO), symbol(wSymbol), type(wType), trigRead(wTrigInput), trigWrite(mTrigOutput), active(true), 
		readCount(0), writeCount(0)
	{
		//
	}

	bool CWatch::HasValue()
	{
		if (io != NULL)
			return io->inCount + io->outCount > 0;
		else
			return true;
	}

	UINT64 CWatch::GetValue()
	{
		if (io != NULL)
			return *io->last;
		else
			return cpu->ReadMem(addr, size);
	}

	bool CWatch::CheckRead(UINT32 cAddr, unsigned cDataSize, UINT64 data)
	{
		if (!active || !trigRead)
			return false;
		// Take care with read that was not aligned with watch address and size
		UINT64 sData = CDebugger::GetSlottedData(cAddr, cDataSize, data, addr, size);
		return CheckBreak(true, sData);
	}

	bool CWatch::CheckWrite(UINT32 cAddr, unsigned cDataSize, UINT64 data)
	{
		if (!active || !trigWrite)
			return false;
		// Take care with write that was not aligned with watch address and size
		UINT64 sData = CDebugger::GetSlottedData(cAddr, cDataSize, data, addr, size);
		return CheckBreak(false, sData);
	}

	bool CWatch::CheckInput(CIO *cIO, UINT64 data)
	{
		// No need to align data as already aligned by CIO
		return active && trigRead && CheckBreak(true, data);
	}

	bool CWatch::CheckOutput(CIO *cIO, UINT64 data)
	{
		// No need to align data as already aligned by CIO
		return active && trigWrite && CheckBreak(false, data);
	}

	void CWatch::Reset()
	{
		//
	}

	bool CWatch::GetInfo(char *str)
	{
		return false;
	}

	CSimpleWatch::CSimpleWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite) : 
		CWatch(wCPU, wAddr, wSize, '*', "simple", wTrigRead, wTrigWrite)
	{
		//
	}

	CSimpleWatch::CSimpleWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool mTrigOutput) : 
		CWatch(wCPU, wIO, '*', "simple", wTrigInput, mTrigOutput)
	{
		//
	}


	bool CSimpleWatch::CheckBreak(bool isRead, UINT64 data)
	{
		return true;
	}

	CCountWatch::CCountWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite, unsigned count) : 
		CWatch(wCPU, wAddr, wSize, '+', "count", wTrigRead, wTrigWrite), m_count(count), m_counter(0)
	{
		//
	}

	CCountWatch::CCountWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool mTrigOutput, unsigned count) : 
		CWatch(wCPU, wIO, '+', "count", wTrigInput, mTrigOutput), m_count(count), m_counter(0)
	{
		//
	}

	bool CCountWatch::CheckBreak(bool isRead, UINT64 data)
	{
		return ++m_counter == m_count;
	}

	void CCountWatch::Reset()
	{
		m_counter = 0;
	}

	bool CCountWatch::GetInfo(char *str)
	{	
		sprintf(str, "%d / %d", m_counter, m_count); 
		return true;
	}
		
	CMatchWatch::CMatchWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite, vector<UINT64> &dataSeq) : 
		CWatch(wCPU, wAddr, wSize, 'M', "match", wTrigRead, wTrigWrite), m_dataLen(dataSeq.size()), m_counter(0)
	{
		m_dataArray = new UINT64[m_dataLen];
		copy(dataSeq.begin(), dataSeq.end(), m_dataArray);
	}

	CMatchWatch::CMatchWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool mTrigOutput, vector<UINT64> &dataSeq) : 
		CWatch(wCPU, wIO, 'M', "match", wTrigInput, mTrigOutput), m_dataLen(dataSeq.size()), m_counter(0)
	{
		m_dataArray = new UINT64[m_dataLen];
		copy(dataSeq.begin(), dataSeq.end(), m_dataArray);
	}

	CMatchWatch::~CMatchWatch()
	{
		delete m_dataArray;
	}

	bool CMatchWatch::CheckBreak(bool isRead, UINT64 data)
	{
		if (m_dataArray[m_counter] == data)
			m_counter++;
		else
			m_counter = 0;
		return m_counter == m_dataLen;
	}

	void CMatchWatch::Reset()
	{
		m_counter = 0;
	}

	bool CMatchWatch::GetInfo(char *str)
	{	
		sprintf(str, "%d / %d matched", m_counter, m_dataLen); 
		return true;
	}

	CCaptureWatch::CCaptureWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite, unsigned wMaxLen) : 
		CWatch(wCPU, wAddr, wSize, 'C', "capture", wTrigRead, wTrigWrite), m_maxLen(min<unsigned>(255, wMaxLen)), m_len(0), m_index(0)
	{
		//
	}

	CCaptureWatch::CCaptureWatch(CCPUDebug *wCPU, CIO *wIO, bool wTrigInput, bool mTrigOutput, unsigned wMaxLen) : 
		CWatch(wCPU, wIO, 'C', "capture", wTrigInput, mTrigOutput), m_maxLen(min<unsigned>(255, wMaxLen)), m_len(0), m_index(0)
	{
		//
	}

	bool CCaptureWatch::CheckBreak(bool isRead, UINT64 data)
	{
		if (m_len < m_maxLen)
		{
			m_dataVals[m_len] = data;
			m_dataRW[m_len] = isRead;
			m_len++;
			m_index = 0;
		}
		else
		{
			m_dataVals[m_index] = data;
			m_dataRW[m_index] = isRead;
			m_index++;
			if (m_index >= m_maxLen)
				m_index = 0;
		}
		return false;
	}

	void CCaptureWatch::Reset()
	{
		m_len = 0;
		m_index = 0;
	}

	bool CCaptureWatch::GetInfo(char *str)
	{	
		char dataStr[25];
		unsigned ind, len;
		str[0] = '\0';
		if (m_len == 0)
			return true;
		else if (m_len < m_maxLen)
		{
			ind = m_len - 1;
			len = m_len;
		}
		else
		{
			if (m_index == 0)
				ind = m_maxLen - 1;
			else
				ind = m_index - 1;
			len = m_maxLen;
		}
		for (unsigned i = 0; i < len; i++)
		{
			UINT64 data = m_dataVals[ind];
			bool isRead = m_dataRW[ind];
			cpu->FormatData(dataStr, size, data);
			if (i > 0)
				strcat(str, " ");
			strcat(str, (isRead ? "<" : ">"));
			strcat(str, dataStr);
			if (ind == 0)
				ind = m_maxLen - 1;
			else
				ind--;
		}
		return true;
	}

	CPrintWatch::CPrintWatch(CCPUDebug *wCPU, UINT32 wAddr, unsigned wSize, bool wTrigRead, bool wTrigWrite) : 
		CWatch(wCPU, wAddr, wSize, 'P', "print", wTrigRead, wTrigWrite)
	{
		//
	}

	CPrintWatch::CPrintWatch(CCPUDebug *wCPU, CIO *io, bool wTrigInput, bool mTrigOutput) : 
		CWatch(wCPU, io, 'P', "print", wTrigInput, mTrigOutput)
	{
		//
	}

	bool CPrintWatch::CheckBreak(bool isRead, UINT64 data)
	{
		const char *sizeStr = CDebugger::GetSizeString(size);
		const char *rwStr = (isRead ? "Read" : "Wrote");
		const char *tfStr = (isRead ? "from" : "to");
		char dataStr[50];
		char addrStr[50];
		char locStr[50];
		cpu->FormatData(dataStr, size, data);
		cpu->FormatAddress(addrStr, addr, true);
		if (io != NULL)
		{
			io->GetLocation(locStr);
			cpu->debugger->PrintEvent(cpu, "%s %s data (%s) %s I/O %s.\n", rwStr, sizeStr, dataStr, tfStr, locStr);
		}
		else
			cpu->debugger->PrintEvent(cpu, "%s %s data (%s) %s memory %s.\n", rwStr, sizeStr, dataStr, tfStr, addrStr);
		return false;
	}
}

#endif  // SUPERMODEL_DEBUGGER
