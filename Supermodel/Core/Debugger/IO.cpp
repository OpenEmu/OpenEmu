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
 * IO.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "CPUDebug.h"
#include "IO.h"
#include "Watch.h"

namespace Debugger
{
	CIO::CIO(CCPUDebug *ioCPU, const char *ioName, const char *ioGroup, unsigned ioDataSize) : 
		cpu(ioCPU), name(ioName), group(ioGroup), dataSize(ioDataSize), 
		watch(NULL), inCount(0), outCount(0), lastIn(0), lastOut(0), last(&lastIn)
	{
		//
	}

	bool CIO::CheckInput(UINT64 data)
	{
		inCount++;
		RecordInput(data);

		// Check watch, if any
		return watch != NULL && watch->CheckInput(this, data);
	}

	bool CIO::CheckOutput(UINT64 data)
	{
		outCount++;
		RecordOutput(data);

		// Check watch, if any
		return watch != NULL && watch->CheckOutput(this, data);
	}

	CSimpleWatch *CIO::AddSimpleWatch(bool trigInput, bool trigOutput)
	{
		CSimpleWatch *sWatch = new CSimpleWatch(cpu, this, trigInput, trigOutput);
		cpu->AddWatch(sWatch);
		return sWatch;
	}

	CCountWatch *CIO::AddCountWatch(bool trigInput, bool trigOutput, unsigned count)
	{
		CCountWatch *cWatch = new CCountWatch(cpu, this, trigInput, trigOutput, count);
		cpu->AddWatch(cWatch);
		return cWatch;
	}

	CMatchWatch *CIO::AddMatchWatch(bool trigInput, bool trigOutput, vector<UINT64> &dataSeq)
	{
		CMatchWatch *mWatch = new CMatchWatch(cpu, this, trigInput, trigOutput, dataSeq);
		cpu->AddWatch(mWatch);
		return mWatch;
	}

	CPrintWatch *CIO::AddPrintWatch(bool trigInput, bool trigOutput)
	{
		CPrintWatch *pWatch = new CPrintWatch(cpu, this, trigInput, trigOutput);
		cpu->AddWatch(pWatch);
		return pWatch;
	}

	CCaptureWatch *CIO::AddCaptureWatch(bool trigInput, bool trigOutput, unsigned maxLen)
	{
		CCaptureWatch *cWatch = new CCaptureWatch(cpu, this, trigInput, trigOutput, maxLen);
		cpu->AddWatch(cWatch);
		return cWatch;
	}

	bool CIO::RemoveWatch()
	{
		return watch != NULL && cpu->RemoveWatch(watch);
	}

	CPortIO::CPortIO(CCPUDebug *ioCPU, const char *ioName, const char *ioGroup, unsigned ioDataSize, UINT16 ioPortNum) : 
		CIO(ioCPU, ioName, ioGroup, ioDataSize), portNum(ioPortNum), label(NULL)
	{
		//
	}
		
	void CPortIO::AddLabel(const char *pLabel)
	{
		strncpy(labelStr, pLabel, MAX_LABEL_LENGTH);
		labelStr[MAX_LABEL_LENGTH] = '\0';
		label = labelStr;
	}

	void CPortIO::RemoveLabel()
	{
		label = NULL;
	}

	void CPortIO::GetLocation(char *str)
	{
		if (name != NULL)
			strcpy(str, name);
		else
			sprintf(str, "port %u", portNum);
	}

	UINT64 CPortIO::Read()
	{
		UINT64 data = cpu->ReadPort(portNum);
		RecordInput(data);
		return data;
	}

	bool CPortIO::Write(UINT64 data)
	{
		if (!cpu->WritePort(portNum, data))
			return false;
		RecordOutput(data);
		return true;
	}

	CMappedIO::CMappedIO(CCPUDebug *ioCPU, const char *ioName, const char *ioGroup, unsigned ioDataSize, UINT32 ioAddr) : 
		CIO(ioCPU, ioName, ioGroup, ioDataSize), CAddressRef(ioCPU, ioAddr, ioDataSize)
	{
		//
	}

	bool CMappedIO::CheckInput(UINT32 cAddr, unsigned cDataSize, UINT64 data)
	{
		// Take care with input that was not aligned with mapped I/O address and size
		UINT64 sData = CDebugger::GetSlottedData(cAddr, cDataSize, data, addr, dataSize);
		return CIO::CheckInput(sData);
	}

	bool CMappedIO::CheckOutput(UINT32 cAddr, unsigned cDataSize, UINT64 data)
	{
		// Take care with output that was not aligned with mapped I/O address and size
		UINT64 sData = CDebugger::GetSlottedData(cAddr, cDataSize, data, addr, dataSize);
		return CIO::CheckOutput(sData);
	}

	void CMappedIO::AddLabel(const char *label)
	{
		CIO::cpu->AddLabel(addr, label);
	}

	void CMappedIO::RemoveLabel()
	{
		CIO::cpu->RemoveLabel(addr);
	}

	void CMappedIO::GetLocation(char *str)
	{
		CIO::cpu->FormatAddress(str, addr);
	}

	UINT64 CMappedIO::Read()
	{
		UINT64 data = CIO::cpu->ReadMem(addr, dataSize);
		RecordInput(data);
		return data;
	}

	bool CMappedIO::Write(UINT64 data)
	{
		if (!CIO::cpu->WriteMem(addr, dataSize, data))
			return false;
		RecordOutput(data);
		return true;
	}
}

#endif  // SUPERMODEL_DEBUGGER