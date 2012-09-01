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
 * CPUDebug.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "CPUDebug.h"
#include "CodeAnalyser.h"
#include "Label.h"

#include <cctype>
#include <string>

namespace Debugger
{
	CCPUDebug::CCPUDebug(const char *cpuType, const char *cpuName,
		UINT8 cpuMinInstrLen, UINT8 cpuMaxInstrLen, bool cpuBigEndian, UINT8 cpuMemBusWidth, UINT8 cpuMaxMnemLen) : 
		type(cpuType), name(cpuName), minInstrLen(cpuMinInstrLen), maxInstrLen(cpuMaxInstrLen), 
		bigEndian(cpuBigEndian), memBusWidth(cpuMemBusWidth), maxMnemLen(cpuMaxMnemLen),
		enabled(true), addrFmt(HexDollar), portFmt(Decimal), dataFmt(HexDollar), debugger(NULL), 
		numExCodes(0), numIntCodes(0), numPorts(0), memSize(0), active(false), instrCount(0), totalCycles(0), cyclesPerPoll(0), pc(0), opcode(0),
		m_break(false), m_breakUser(false), m_halted(false), m_step(false), m_steppingOver(false), m_steppingOut(false), 
		m_count(0), m_until(false), m_untilAddr(0),
		m_mappedIOTable(NULL), m_memWatchTable(NULL), m_bpTable(NULL), m_numRegMons(0), m_regMonArray(NULL),
		m_analyser(NULL), m_stateUpdated(false), m_exRaised(NULL), m_exTrapped(NULL), m_intRaised(NULL), m_intTrapped(NULL), m_bpReached(NULL), 
		m_memWatchTriggered(NULL), m_ioWatchTriggered(NULL), m_regMonTriggered(NULL), m_prevTotalCycles(0)
	{ 
		memset(m_exArray, NULL, sizeof(m_exArray));
		memset(m_intArray, NULL, sizeof(m_intArray));
		memset(m_portArray, NULL, sizeof(m_portArray));

#ifdef DEBUGGER_HASTHREAD
		m_breakWait = false;
		m_mutex = CThread::CreateMutex();
		m_condVar = CThread::CreateCondVar();
#endif // DEBUGGER_HASTHREAD
	}

	CCPUDebug::~CCPUDebug()
	{
		// Delete registers
		for (vector<CRegister*>::iterator it = regs.begin(); it != regs.end(); it++)
			delete *it;
		regs.clear();

		// Delete exceptions
		for (vector<CException*>::iterator it = exceps.begin(); it != exceps.end(); it++)
			delete *it;
		exceps.clear();
		memset(m_exArray, NULL, sizeof(m_exArray));

		// Delete interrupts
		for (vector<CInterrupt*>::iterator it = inters.begin(); it != inters.end(); it++)
			delete *it;
		inters.clear();
		memset(m_intArray, NULL, sizeof(m_intArray));
		
		// Delete IOs
		for (vector<CIO*>::iterator it = ios.begin(); it != ios.end(); it++)
			delete *it;
		ios.clear();
		memset(m_portArray, NULL, sizeof(m_portArray));
		delete m_mappedIOTable;
		
		// Delete regions
		for (vector<CRegion*>::iterator it = regions.begin(); it != regions.end(); it++)
			delete *it;
		regions.clear();

		// Delete labels, comments, memory watches, I/O watches, breakpoints and register monitors
		RemoveAllLabels();
		RemoveAllComments();
		RemoveAllMemWatches();
		RemoveAllIOWatches();
		RemoveAllBreakpoints();
		RemoveAllRegMonitors();

		// Delete analyser, if any
		if (m_analyser != NULL)
			delete m_analyser;
	}

	void CCPUDebug::AttachToDebugger(CDebugger *theDebugger)
	{
		debugger = theDebugger;
	}

	void CCPUDebug::DetachFromDebugger(CDebugger *theDebugger)
	{
		if (debugger == theDebugger)
			debugger = NULL;
	}

	CPCRegister *CCPUDebug::AddPCRegister(const char *name, const char *group)
	{
		CPCRegister *reg = new CPCRegister(this, name, group);
		regs.push_back(reg);
		return reg;
	}

	CAddrRegister *CCPUDebug::AddAddrRegister(const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc)
	{
		CAddrRegister *reg = new CAddrRegister(this, name, group, id, getFunc, setFunc);
		regs.push_back(reg);
		return reg;
	}

	CIntRegister *CCPUDebug::AddInt8Register(const char *name, const char *group, unsigned id, GetInt8FPtr getFunc, SetInt8FPtr setFunc)
	{
		CIntRegister *reg = new CIntRegister(this, name, group, id, getFunc, setFunc);
		regs.push_back(reg);
		return reg;
	}

	CIntRegister *CCPUDebug::AddInt16Register(const char *name, const char *group, unsigned id, GetInt16FPtr getFunc, SetInt16FPtr setFunc)
	{
		CIntRegister *reg = new CIntRegister(this, name, group, id, getFunc, setFunc);
		regs.push_back(reg);
		return reg;
	}

	CIntRegister *CCPUDebug::AddInt32Register(const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc)
	{
		CIntRegister *reg = new CIntRegister(this, name, group, id, getFunc, setFunc);
		regs.push_back(reg);
		return reg;
	}

	CIntRegister *CCPUDebug::AddInt64Register(const char *name, const char *group, unsigned id, GetInt64FPtr getFunc, SetInt64FPtr setFunc)
	{
		CIntRegister *reg = new CIntRegister(this, name, group, id, getFunc, setFunc);
		regs.push_back(reg);
		return reg;
	}

	CStatusRegister *CCPUDebug::AddStatus8Register(const char *name, const char *group, unsigned id, const char *bitStr, GetInt8FPtr getFunc, SetInt8FPtr setFunc)
	{
		CStatusRegister *reg = new CStatusRegister(this, name, group, id, getFunc, setFunc);
		reg->SetBits(bitStr);
		regs.push_back(reg);
		return reg;
	}

	CStatusRegister *CCPUDebug::AddStatus16Register(const char *name, const char *group, unsigned id, const char *bitStr, GetInt16FPtr getFunc, SetInt16FPtr setFunc)
	{
		CStatusRegister *reg = new CStatusRegister(this, name, group, id, getFunc, setFunc);
		reg->SetBits(bitStr);
		regs.push_back(reg);
		return reg;
	}

	CStatusRegister *CCPUDebug::AddStatus32Register(const char *name, const char *group, unsigned id, const char *bitStr, GetInt32FPtr getFunc, SetInt32FPtr setFunc)
	{
		CStatusRegister *reg = new CStatusRegister(this, name, group, id, getFunc, setFunc);
		reg->SetBits(bitStr);
		regs.push_back(reg);
		return reg;
	}

	CFPointRegister *CCPUDebug::AddFPointRegister(const char *name, const char *group, unsigned id, GetFPointFPtr getFunc, SetFPointFPtr setFunc)
	{
		CFPointRegister *reg = new CFPointRegister(this, name, group, id, getFunc, setFunc);
		regs.push_back(reg);
		return reg;
	}

	CException *CCPUDebug::AddException(const char *id, UINT16 code, const char *name)
	{
		if (code >= MAX_EXCEPTIONS)
			return NULL;
		CException *ex = new CException(this, id, code, name);
		exceps.push_back(ex);
		m_exArray[code] = ex;
		numExCodes = max<UINT16>(numExCodes, code + 1);
		return ex;
	}

	CInterrupt *CCPUDebug::AddInterrupt(const char *id, UINT16 code, const char *name)
	{
		if (code >= MAX_INTERRUPTS)
			return NULL;
		CInterrupt *in = new CInterrupt(this, id, code, name);
		inters.push_back(in);
		m_intArray[code] = in;
		numIntCodes = max<UINT16>(numIntCodes, code + 1);
		return in;
	}

	CRegion *CCPUDebug::AddRegion(UINT32 start, UINT32 end, bool isCode, bool isReadOnly, const char *name)
	{
		if (debugger != NULL)
			return NULL;
		CRegion *region = new CRegion(this, start, end, isCode, isReadOnly, name);
		if (regions.size() > 0)
		{
			region->prevRegion = regions[regions.size() - 1];
			region->prevRegion->nextRegion = region;
		}
		regions.push_back(region);
		memSize = max<UINT32>(memSize, end);
		return region;
	}

	CPortIO *CCPUDebug::AddPortIO(UINT16 portNum, unsigned dataSize, const char *name, const char *group)
	{
		if (debugger != NULL || portNum >= MAX_IOPORTS)
			return NULL;
		CPortIO *port = new CPortIO(this, name, group, dataSize, portNum);
		ios.push_back(port);
		m_portArray[portNum] = port;
		numPorts = max<UINT16>(numPorts, portNum + 1);
		return port;
	}

	CMappedIO *CCPUDebug::AddMappedIO(UINT32 addr, unsigned dataSize, const char *name, const char *group)
	{
		if (debugger != NULL)
			return NULL;
		if (m_mappedIOTable == NULL)
			m_mappedIOTable = new CAddressTable();
		CMappedIO *mapped = new CMappedIO(this, name, group, dataSize, addr);
		ios.push_back(mapped);
		m_mappedIOTable->Add(mapped);
		return mapped;
	}

	bool CCPUDebug::ParseData(const char *str, unsigned dataSize, UINT64 *data, bool registers)
	{
		// Check registers, if required
		if (registers)
		{
			CRegister *reg = GetRegister(str);
			if (reg != NULL)
			{
				*data = reg->GetValueAsInt();
				return *data == CDebugger::MaskData(dataSize, *data);
			}
		}

		return debugger->ParseData(str, dataFmt, dataSize, data) && *data == CDebugger::MaskData(dataSize, *data);	
	}

	void CCPUDebug::FormatData(char *str, unsigned dataSize, UINT64 data)
	{
		debugger->FormatData(str, dataFmt, dataSize, data); 
	}

	bool CCPUDebug::ParseAddress(const char *str, UINT32 *addr, bool customLabels, bool autoLabels, bool registers)
	{
		// Check custom labels, if required
		if (customLabels)
		{
			CLabel *label = GetLabel(str);
			if (label != NULL)
			{
				*addr = label->addr;
				return GetRegion(*addr) != NULL;
			}
		}

		// Check auto-labels, if required
		if (autoLabels && m_analyser != NULL)
		{
			CAutoLabel *autoLabel = m_analyser->analysis->GetAutoLabel(str);
			if (autoLabel != NULL)
			{
				*addr = autoLabel->addr;
				return GetRegion(*addr) != NULL;
			}
		}

		// Check registers, if required
		if (registers)
		{
			CRegister *reg = GetRegister(str);
			if (reg != NULL)
			{
				*addr = reg->GetValueAsInt();
				return GetRegion(*addr) != NULL;
			}
		}

		// Parse address
		unsigned dataSize = (unsigned)(memBusWidth / 8);
		UINT64 vlAddr;
		if (!debugger->ParseData(str, addrFmt, dataSize, &vlAddr) || vlAddr > 0xFFFFFFFFULL)
			return false;
		*addr = (UINT32)vlAddr;
		return GetRegion(*addr) != NULL;
	}

	void CCPUDebug::FormatAddress(char *str, UINT32 addr, bool customLabels, ELabelFlags autoLabelFlags)
	{
		// Check custom labels, if required
		if (customLabels)
		{
			CLabel *label = GetLabel(addr);
			if (label != NULL)
			{
				strcpy(str, label->name);
				return;
			}
		}

		// Check auto-labels, if required
		if (autoLabelFlags != LFNone && m_analyser != NULL)
		{	
			CAutoLabel *autoLabel = m_analyser->analysis->GetAutoLabel(addr);
			if (autoLabel != NULL && autoLabel->GetLabel(str, autoLabelFlags))
			{
				autoLabel->GetLabel(str, autoLabelFlags);
				return;
			}
		}

		// Format address
		debugger->FormatData(str, addrFmt, (unsigned)(memBusWidth / 8), (UINT64)addr);
	}

	void CCPUDebug::FormatJumpAddress(char *str, UINT32 jumpAddr, EOpFlags opFlags)
	{
		if      (opFlags & JumpSub)    FormatAddress(str, jumpAddr, true, LFSubroutine);
		else if (opFlags & JumpLoop)   FormatAddress(str, jumpAddr, true, LFLoopPoint);
		else if (opFlags & JumpSimple) FormatAddress(str, jumpAddr, true, LFJumpTarget);
		else                           FormatAddress(str, jumpAddr, true, LFNone);                            
	}

	bool CCPUDebug::ParsePortNum(const char *str, UINT16 *portNum, bool customLabels, bool registers)
	{
		// Check custom labels, if required
		if (customLabels)
		{
			for (UINT16 loopNum = 0; loopNum < numPorts; loopNum++)
			{
				CPortIO *port = m_portArray[loopNum];
				if (port == NULL || port->label == NULL)
					continue;
				if (stricmp(port->label, str) == 0)
				{
					*portNum = loopNum;
					return true;
				}
			}
		}

		// Check registers, if required
		if (registers)
		{
			CRegister *reg = GetRegister(str);
			if (reg != NULL)
			{
				*portNum = reg->GetValueAsInt();
				return *portNum < numPorts;
			}
		}

		// Parse port num
		unsigned dataSize = CDebugger::GetDataSize(numPorts);
		UINT64 vlPortNum;
		if (!debugger->ParseData(str, portFmt, dataSize, &vlPortNum))
			return false;
		*portNum = (UINT16)vlPortNum;
		return *portNum < numPorts && GetPortIO(*portNum) != NULL;
	}

	void CCPUDebug::FormatPortNum(char *str, UINT16 portNum, bool customLabels)
	{
		if (portNum >= numPorts)
		{
			str[0] = '\0';
			return;
		}

		// Check custom labels, if reqired
		if (customLabels)
		{
			CPortIO *port = m_portArray[portNum];
			if (port != NULL && port->label != NULL)
			{
				strcpy(str, port->label);
				return;
			}
		}

		// Format port num
		unsigned dataSize = CDebugger::GetDataSize(numPorts);
		debugger->FormatData(str, portFmt, dataSize, (UINT64)portNum);
	}

	void CCPUDebug::ForceBreak(bool user)
	{
		m_breakUser |= user;
		m_break = true;
	}

	void CCPUDebug::ClearBreak()
	{
#ifdef DEBUGGER_HASTHREAD
		m_breakWait = false;
#endif // DEBUGGER_HASTHREAD
		m_breakUser = false;
		m_break = false;
	}

	void CCPUDebug::SetContinue()
	{
		m_step = false;
		m_count = 0;
		m_until = false;
	}

	void CCPUDebug::SetStepMode(EStepMode stepMode)
	{
		m_step = true;
		m_stepMode = stepMode;
		m_steppingOver = false;
		m_steppingOut = false;
		m_count = 0;
		m_until = false;
	}

	void CCPUDebug::SetCount(int count)
	{
		m_step = false;
		m_count = count;
		m_until = false;
	}

	void CCPUDebug::SetUntil(UINT32 untilAddr)
	{
		m_step = false;
		m_count = 0;
		m_until = true;
		m_untilAddr = untilAddr;
	}

	bool CCPUDebug::SetPC(UINT32 newPC)
	{
		if (!UpdatePC(newPC))
			return false;
		pc = newPC;
		m_stateUpdated = true;
		return true;
	}

	bool CCPUDebug::GenerateException(CException *ex)
	{
		if (!ForceException(ex))
			return false;
		m_stateUpdated = true;
		return true;
	}

	bool CCPUDebug::GenerateInterrupt(CInterrupt *in)
	{
		if (!ForceInterrupt(in))
			return false;
		m_stateUpdated = true;
		return true;
	}

	CRegister *CCPUDebug::GetRegister(const char *name)
	{
		for (vector<CRegister*>::iterator it = regs.begin(); it != regs.end(); it++)
		{
			if (stricmp((*it)->name, name) == 0)
				return *it;
		}
		return NULL;
	}

	CException *CCPUDebug::GetException(const char *id)
	{
		for (vector<CException*>::iterator it = exceps.begin(); it != exceps.end(); it++)
		{
			if (stricmp((*it)->id, id) == 0)
				return *it;
		}
		return NULL;
	}

	CException *CCPUDebug::GetException(UINT16 code)
	{
		if (code >= numExCodes)
			return NULL;
		return m_exArray[code];
	}

	CInterrupt *CCPUDebug::GetInterrupt(const char *id)
	{
		for (vector<CInterrupt*>::iterator it = inters.begin(); it != inters.end(); it++)
		{
			if (stricmp((*it)->id, id) == 0)
				return *it;
		}
		return NULL;
	}

	CInterrupt *CCPUDebug::GetInterrupt(UINT16 code)
	{
		if (code >= numIntCodes)
			return NULL;
		return m_intArray[code];
	}

	CPortIO *CCPUDebug::GetPortIO(UINT16 portNum)
	{
		if (portNum >= numPorts)
			return NULL;
		return m_portArray[portNum];
	}

	CMappedIO *CCPUDebug::GetMappedIO(UINT32 addr)
	{
		if (m_mappedIOTable == NULL)
			return NULL;
		return (CMappedIO*)m_mappedIOTable->Get(addr);
	}

	CRegion *CCPUDebug::GetRegion(UINT32 addr)
	{
		for (vector<CRegion*>::iterator it = regions.begin(); it != regions.end(); it++)
		{
			if ((*it)->CheckAddr(addr))
				return *it;
		}
		return NULL;
	}

	CLabel *CCPUDebug::GetLabel(UINT32 addr)
	{
		for (vector<CLabel*>::iterator it = labels.begin(); it != labels.end(); it++)
		{
			if ((*it)->CheckAddr(addr))
				return *it;
		}
		return NULL;
	}

	CLabel *CCPUDebug::GetLabel(const char *name)
	{
		// TODO - use binary search
		for (vector<CLabel*>::iterator it = labels.begin(); it != labels.end(); it++)
		{
			if (stricmp((*it)->name, name) == 0)
				return *it;
		}
		return NULL;
	}

	CLabel *CCPUDebug::AddLabel(UINT32 addr, const char *name)
	{
		RemoveLabel(addr);
		RemoveLabel(name);
		CLabel *label = new CLabel(this, addr, name);
		// TODO - keep labels in order of increasing address so can use binary search
		labels.push_back(label);
		return label;
	}

	bool CCPUDebug::RemoveLabel(const char *name)
	{
		CLabel *label = GetLabel(name);
		if (label == NULL)
			return false;
		return RemoveLabel(label);
	}

	bool CCPUDebug::RemoveLabel(UINT32 addr)
	{
		CLabel *label = GetLabel(addr);
		if (label == NULL)
			return false;
		RemoveLabel(label);
		return true;
	}

	bool CCPUDebug::RemoveLabel(CLabel *label)
	{
		vector<CLabel*>::iterator it = find(labels.begin(), labels.end(), label);
		if (it == labels.end())
			return false;
		labels.erase(it);
		delete label;
		return true;
	}

	bool CCPUDebug::RemoveAllLabels()
	{
		if (labels.size() == 0)
			return false;
		for (vector<CLabel*>::iterator it = labels.begin(); it != labels.end(); it++)
			delete *it;
		labels.clear();
		return true;
	}

	CComment *CCPUDebug::GetComment(UINT32 addr)
	{
		// TODO - use binary search
		for (vector<CComment*>::iterator it = comments.begin(); it != comments.end(); it++)
		{
			if ((*it)->CheckAddr(addr))
				return *it;
		}
		return NULL;
	}

	CComment *CCPUDebug::AddComment(UINT32 addr, const char *commentText)
	{
		RemoveComment(addr);
		CComment *comment = new CComment(this, addr, commentText);
		// TODO - keep comments in order of increasing address so can use binary search
		comments.push_back(comment);
		return comment;
	}

	bool CCPUDebug::RemoveComment(UINT32 addr)
	{
		CComment *comment = GetComment(addr);
		if (comment == NULL)
			return false;
		RemoveComment(comment);
		return true;
	}

	bool CCPUDebug::RemoveComment(CComment *comment)
	{
		vector<CComment*>::iterator it = find(comments.begin(), comments.end(), comment);
		if (it == comments.end())
			return false;
		comments.erase(it);
		delete comment;
		return true;
	}

	bool CCPUDebug::RemoveAllComments()
	{
		if (comments.size() == 0)
			return false;
		for (vector<CComment*>::iterator it = comments.begin(); it != comments.end(); it++)
			delete *it;
		comments.clear();
		return true;
	}

	CSimpleWatch *CCPUDebug::AddSimpleMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite)
	{
		CSimpleWatch *watch = NULL;
		// First check for mapped I/O at given addr
		if (m_mappedIOTable != NULL)
		{
			CMappedIO *mapped = (CMappedIO*)m_mappedIOTable->Get(addr, size);
			// If found, create watch for I/O
			if (mapped != NULL)
				watch = new CSimpleWatch(this, mapped, trigRead, trigWrite);
		}
		// Otherwise, create watch for memory addr
		if (watch == NULL)
			watch = new CSimpleWatch(this, addr, size, trigRead, trigWrite);
		AddWatch(watch);
		return watch;
	}

	CCountWatch *CCPUDebug::AddCountMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite, unsigned count)
	{
		CCountWatch *watch = NULL;
		// First check for mapped I/O at given addr
		if (m_mappedIOTable != NULL)
		{
			CMappedIO *mapped = (CMappedIO*)m_mappedIOTable->Get(addr, size);
			// If found, create watch for I/O
			if (mapped != NULL)
				watch = new CCountWatch(this, mapped, trigRead, trigWrite, count);
		}
		// Otherwise, create watch for memory addr
		if (watch == NULL)
			watch = new CCountWatch(this, addr, size, trigRead, trigWrite, count);
		AddWatch(watch);
		return watch;
	}

	CMatchWatch *CCPUDebug::AddMatchMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite, vector<UINT64> &dataSeq)
	{
		CMatchWatch *watch = NULL;
		// First check for mapped I/O at given addr
		if (m_mappedIOTable != NULL)
		{
			CMappedIO *mapped = (CMappedIO*)m_mappedIOTable->Get(addr, size);
			// If found, create watch for I/O
			if (mapped != NULL)
				watch = new CMatchWatch(this, mapped, trigRead, trigWrite, dataSeq);
		}
		// Otherwise, create watch for memory addr
		if (watch == NULL)
			watch = new CMatchWatch(this, addr, size, trigRead, trigWrite, dataSeq);
		AddWatch(watch);
		return watch;
	}

	CPrintWatch *CCPUDebug::AddPrintMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite)
	{
		CPrintWatch *watch = NULL;
		// First check for mapped I/O at given addr
		if (m_mappedIOTable != NULL)
		{
			CMappedIO *mapped = (CMappedIO*)m_mappedIOTable->Get(addr, size);
			// If found, create watch for I/O
			if (mapped != NULL)
				watch = new CPrintWatch(this, mapped, trigRead, trigWrite);
		}
		// Otherwise, create watch for memory addr
		if (watch == NULL)
			watch = new CPrintWatch(this, addr, size, trigRead, trigWrite);
		AddWatch(watch);
		return watch;
	}

	CCaptureWatch *CCPUDebug::AddCaptureMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite, unsigned maxLen)
	{
		CCaptureWatch *watch = NULL;
		// First check for mapped I/O at given addr
		if (m_mappedIOTable != NULL)
		{
			CMappedIO *mapped = (CMappedIO*)m_mappedIOTable->Get(addr, size);
			// If found, create watch for I/O
			if (mapped != NULL)
				watch = new CCaptureWatch(this, mapped, trigRead, trigWrite, maxLen);
		}
		// Otherwise, create watch for memory addr
		if (watch == NULL)
			watch = new CCaptureWatch(this, addr, size, trigRead, trigWrite, maxLen);
		AddWatch(watch);
		return watch;
	}

	bool CCPUDebug::RemoveAllMemWatches()
	{
		if (memWatches.size() == 0)
			return false;
		for (vector<CWatch*>::iterator it = memWatches.begin(); it != memWatches.end(); it++)
			delete *it;
		memWatches.clear();
		delete m_memWatchTable;
		m_memWatchTable = NULL;
		return true;
	}

	bool CCPUDebug::RemoveAllIOWatches()
	{
		if (ioWatches.size() == 0)
			return false;
		for (vector<CWatch*>::iterator it = ioWatches.begin(); it != ioWatches.end(); it++)
		{
			(*it)->io->watch = NULL;
			delete *it;
		}
		ioWatches.clear();
		return true;
	}

	bool CCPUDebug::RemoveMemWatch(UINT32 addr, unsigned size)
	{
		CWatch *watch;
		bool removed = false;
		while ((watch = GetMemWatch(addr, size)) != NULL)
			removed |= RemoveWatch(watch);
		return removed;
	}

	CWatch *CCPUDebug::GetMemWatch(UINT32 addr, unsigned size)
	{
		// First check for mapped I/O at given addr and size
		if (m_mappedIOTable != NULL)
		{
			CMappedIO *mapped = (CMappedIO*)m_mappedIOTable->Get(addr, size);
			// If found, check watch on I/O
			if (mapped != NULL)
				return mapped->watch;
		}
		// If none, check memory table for watch
		if (m_memWatchTable == NULL)
			return NULL;
		return (CWatch*)m_memWatchTable->Get(addr, size);
	}

	void CCPUDebug::AddWatch(CWatch *watch)
	{
		CIO *io = watch->io;
		if (io != NULL)
		{
			if (io->watch != NULL)
				RemoveWatch(io->watch);
			ioWatches.push_back(watch);
			UpdateIOWatchNums();
			io->watch = watch;
		}
		else
		{
			RemoveMemWatch(watch->addr, watch->size);
			memWatches.push_back(watch);
			UpdateMemWatchNums();
			if (m_memWatchTable == NULL)
				m_memWatchTable = new CAddressTable();
			m_memWatchTable->Add(watch);
		}
	}

	bool CCPUDebug::RemoveWatch(CWatch *watch)
	{
		CIO *io = watch->io;
		if (io != NULL)
		{
			io->watch = NULL;
			vector<CWatch*>::iterator it = find(ioWatches.begin(), ioWatches.end(), watch);
			if (it == ioWatches.end())
				return false;
			ioWatches.erase(it);
			UpdateIOWatchNums();
		}
		else
		{
			vector<CWatch*>::iterator it = find(memWatches.begin(), memWatches.end(), watch);
			if (it == memWatches.end())
				return false;
			memWatches.erase(it);
			UpdateMemWatchNums();
			m_memWatchTable->Remove(watch);
			if (m_memWatchTable->IsEmpty())
			{
				delete m_memWatchTable;
				m_memWatchTable = NULL;
			}
		}
		delete watch;
		return true;
	}

	void CCPUDebug::UpdateIOWatchNums()
	{
		unsigned num = 0;
		for (vector<CWatch*>::iterator it = ioWatches.begin(); it != ioWatches.end(); it++)
			(*it)->num = num++;
	}

	void CCPUDebug::UpdateMemWatchNums()
	{
		unsigned num = 0;
		for (vector<CWatch*>::iterator it = memWatches.begin(); it != memWatches.end(); it++)
			(*it)->num = num++;
	}

	CSimpleBreakpoint *CCPUDebug::AddSimpleBreakpoint(UINT32 addr)
	{
		CSimpleBreakpoint *bp = new CSimpleBreakpoint(this, addr);
		AddBreakpoint(bp);
		return bp;
	}

	CCountBreakpoint *CCPUDebug::AddCountBreakpoint(UINT32 addr, unsigned count)
	{
		CCountBreakpoint *bp = new CCountBreakpoint(this, addr, count);
		AddBreakpoint(bp);
		return bp;
	}

	CPrintBreakpoint *CCPUDebug::AddPrintBreakpoint(UINT32 addr)
	{
		CPrintBreakpoint *bp = new CPrintBreakpoint(this, addr);
		AddBreakpoint(bp);
		return bp;
	}

	CBreakpoint *CCPUDebug::GetBreakpoint(UINT32 addr)
	{
		if (m_bpTable == NULL)
			return NULL;
		return (CBreakpoint*)m_bpTable->Get(addr);
	}

	void CCPUDebug::AddBreakpoint(CBreakpoint *bp)
	{
		RemoveBreakpoint(bp->addr);
		bps.push_back(bp);
		UpdateBreakpointNums();
		if (m_bpTable == NULL)
			m_bpTable = new CAddressTable();
		m_bpTable->Add(bp);
	}

	bool CCPUDebug::RemoveBreakpoint(UINT32 addr)
	{
		CBreakpoint *bp = GetBreakpoint(addr);
		if (bp == NULL)
			return false;
		return RemoveBreakpoint(bp);
	}

	bool CCPUDebug::RemoveBreakpoint(CBreakpoint *bp)
	{
		vector<CBreakpoint*>::iterator it = find(bps.begin(), bps.end(), bp);
		if (it == bps.end())
			return false;
		bps.erase(it);
		UpdateBreakpointNums();
		m_bpTable->Remove(bp);
		if (m_bpTable->IsEmpty())
		{
			delete m_bpTable;
			m_bpTable = NULL;
		}
		delete bp;
		return true;
	}

	bool CCPUDebug::RemoveAllBreakpoints()
	{
		if (bps.size() == 0)
			return false;
		for (vector<CBreakpoint*>::iterator it = bps.begin(); it != bps.end(); it++)
			delete *it;
		bps.clear();
		delete m_bpTable;
		m_bpTable = NULL;
		return true;
	}

	void CCPUDebug::UpdateBreakpointNums()
	{
		unsigned num = 0;
		for (vector<CBreakpoint*>::iterator it = bps.begin(); it != bps.end(); it++)
			(*it)->num = num++;
	}

	CRegMonitor *CCPUDebug::AddRegMonitor(const char *regName)
	{
		RemoveRegMonitor(regName);

		CRegister *reg = GetRegister(regName);
		if (reg == NULL)
			return NULL;
		CRegMonitor *regMon = reg->CreateMonitor();
		AddRegMonitor(regMon);
		return regMon;
	}

	CRegMonitor *CCPUDebug::GetRegMonitor(const char *regName)
	{
		for (vector<CRegMonitor*>::iterator it = regMons.begin(); it != regMons.end(); it++)
		{
			if (stricmp((*it)->reg->name, regName) == 0)
				return *it;
		}
		return NULL;
	}

	void CCPUDebug::AddRegMonitor(CRegMonitor *regMon)
	{
		regMons.push_back(regMon);
		UpdateRegMonArray();
	}

	bool CCPUDebug::RemoveRegMonitor(const char *regName)
	{
		CRegMonitor *regMon = GetRegMonitor(regName);
		if (regMon == NULL)
			return false;
		return RemoveRegMonitor(regMon);
	}

	bool CCPUDebug::RemoveRegMonitor(CRegMonitor *regMon)
	{
		vector<CRegMonitor*>::iterator it = find(regMons.begin(), regMons.end(), regMon);
		if (it == regMons.end())
			return false;
		regMons.erase(it);
		delete regMon;
		UpdateRegMonArray();
		return true;
	}

	bool CCPUDebug::RemoveAllRegMonitors()
	{
		if (regMons.size() == 0)
			return false;
		for (vector<CRegMonitor*>::iterator it = regMons.begin(); it != regMons.end(); it++)
			delete *it;
		regMons.clear();
		UpdateRegMonArray();
		return true;
	}

	void CCPUDebug::UpdateRegMonArray()
	{
		if (m_regMonArray != NULL)
			delete m_regMonArray;
		m_numRegMons = regMons.size();
		if (m_numRegMons > 0)
		{
			m_regMonArray = new CRegMonitor*[m_numRegMons];
			copy(regMons.begin(), regMons.end(), m_regMonArray);		
		}
		else
			m_regMonArray = NULL;
	}

	CCodeAnalyser *CCPUDebug::GetCodeAnalyser()
	{
		if (m_analyser == NULL)
			m_analyser = new CCodeAnalyser(this);
		return m_analyser;
	}

	bool CCPUDebug::FindInMem(UINT32 start, UINT32 end, const char *str, bool matchCase, UINT32 &findAddr)
	{
		size_t len = strlen(str);
		const char *matchEnd = str + len - 1;
		const char *matchPos;
		if (start <= end)
		{
			matchPos = str;
			for (UINT32 addr = start; addr <= end + len - 1; addr++)
			{
				char c = (char)ReadMem(addr, 1);
				if (matchCase && c == *matchPos || !matchCase && toupper(c) == toupper(*matchPos))
				{
					if (matchPos == matchEnd)
					{
						findAddr = addr - len + 1;
						return true;
					}
					else
						matchPos++;
				}
				else
					matchPos = str;
				if (addr == 0xFFFFFFFFU)
					break;
			}
		}
		else
		{
			matchPos = matchEnd;
			for (UINT32 addr = start + len - 1; addr >= end; addr--)
			{
				char c = (char)ReadMem(addr, 1);
				if (matchCase && c == *matchPos || !matchCase && toupper(c) == toupper(*matchPos))
				{
					if (matchPos == str)
					{
						findAddr = addr;
						return true;
					}
					else
						matchPos--;
				}
				else
					matchPos = matchEnd;
				if (addr == 0)
					break;
			}
		}
		return false;
	}

	bool CCPUDebug::FindInMem(UINT32 start, UINT32 end, UINT8 *bytes, unsigned length, UINT32 &findAddr)
	{
		UINT32 matchEnd = length - 1;
		UINT32 matchPos;
		if (start <= end)
		{
			matchPos = 0;
			for (UINT32 addr = start; addr <= end; addr++)
			{
				UINT8 b = (UINT8)ReadMem(addr, 1);
				if (b == bytes[matchPos])
				{
					if (matchPos == matchEnd)
					{
						findAddr = addr - matchPos;
						return true;
					}
					else
						matchPos++;
				}
				else
					matchPos = 0;
				if (addr == 0xFFFFFFFFU)
					break;
			}
		}
		else
		{
			matchPos = matchEnd;
			for (UINT32 addr = end; addr >= start; addr--)
			{
				UINT8 b = (UINT8)ReadMem(addr, 1);
				if (b == bytes[matchPos])
				{
					if (matchPos == 0)
					{
						findAddr = addr;
						return true;
					}
					else
						matchPos++;
				}
				else
					matchPos = matchEnd;
				if (addr == 0)
					break;
			}
		}
		return false;
	}

#ifdef DEBUGGER_HASBLOCKFILE
	bool CCPUDebug::LoadState(CBlockFile *state)
	{
		char blockStr[255];
		UINT32 addr;
		UINT32 addrEnd;
		UINT32 strLen;
		char str[4096];

		// Load labels
		sprintf(blockStr, "%s.labels", name);
		if (state->FindBlock(blockStr) == OKAY)
		{
			labels.clear();
			UINT32 numLabels;
			state->Read(&numLabels, sizeof(numLabels));
			for (UINT32 i = 0; i < numLabels; i++)
			{	
				state->Read(&addr, sizeof(addr));
				state->Read(&addrEnd, sizeof(addrEnd));
				state->Read(&strLen, sizeof(strLen));
				state->Read(str, strLen * sizeof(char));
				str[strLen] = '\0';
				CLabel *label = new CLabel(this, addr, addrEnd, str);
				labels.push_back(label);
			}
		}

		// Load comments
		sprintf(blockStr, "%s.comments", name);
		if (state->FindBlock(blockStr) == OKAY)
		{
			comments.clear();
			UINT32 numComments;
			state->Read(&numComments, sizeof(numComments));
			for (UINT32 i = 0; i < numComments; i++)
			{	
				state->Read(&addr, sizeof(addr));
				state->Read(&strLen, sizeof(strLen));
				state->Read(str, strLen * sizeof(char));
				str[strLen] = '\0';
				CComment *comment = new CComment(this, addr, str);
				comments.push_back(comment);
			}
		}

		// Load analyser state
		CCodeAnalyser *analyser = GetCodeAnalyser();
		if (!analyser->LoadState(state))
			return false;

		// TODO - load breakpoints, watches, exception/interrupt traps and register monitors

		return true;
	}

	bool CCPUDebug::SaveState(CBlockFile *state)
	{
		char blockStr[255];
		UINT32 addr;
		UINT32 addrEnd;
		UINT32 strLen;
		const char *str;

		// Save labels
		sprintf(blockStr, "%s.labels", name);
		state->NewBlock(blockStr, __FILE__);
		UINT32 numLabels = (UINT32)labels.size();
		state->Write(&numLabels, sizeof(numLabels));
		for (vector<CLabel*>::iterator it = labels.begin(); it != labels.end(); it++)
		{
			addr = (*it)->addr;
			addrEnd = (*it)->addrEnd;
			str = (*it)->name;
			strLen = min<UINT32>(4095, (UINT32)strlen(str));
			state->Write(&addr, sizeof(addr));
			state->Write(&addrEnd, sizeof(addrEnd));
			state->Write(&strLen, sizeof(strLen));
			state->Write(str, strLen * sizeof(char));
		}

		// Save comments
		sprintf(blockStr, "%s.comments", name);
		state->NewBlock(blockStr, __FILE__);
		UINT32 numComments = (UINT32)comments.size();
		state->Write(&numComments, sizeof(numComments));
		for (vector<CComment*>::iterator it = comments.begin(); it != comments.end(); it++)
		{
			addr = (*it)->addr;
			addrEnd = (*it)->addrEnd;
			str = (*it)->text;
			strLen = min<UINT32>(4095, (UINT32)strlen(str));
			state->Write(&addr, sizeof(addr));
			state->Write(&strLen, sizeof(strLen));
			state->Write(str, strLen * sizeof(char));
		}

		// Save analyser state, if available
		if (m_analyser != NULL && !m_analyser->SaveState(state))
			return false;

		// TODO - save breakpoints, watches, exception/interrupt traps and register monitors

		return true;
	}
#endif // DEBUGGER_HASBLOCKFILE

	void CCPUDebug::CPUException(UINT16 exCode)
	{
		if (exCode >= numExCodes)
			return;
		CException *ex = m_exArray[exCode];
		if (ex == NULL)
			return;

		m_exRaised = ex;
		ex->count++;
		if (!ex->trap)
			return;
			
		m_exTrapped = ex;
		debugger->ExceptionTrapped(ex);
		m_break = true;	
	}

	void CCPUDebug::CPUInterrupt(UINT16 intCode)
	{
		if (intCode >= numIntCodes)
			return;
		CInterrupt *in = m_intArray[intCode];
		if (in == NULL)
			return;

		m_intRaised = in;
		in->count++;
		if (!in->trap)
			return;
			
		m_intTrapped = in;
		debugger->InterruptTrapped(in);
		m_break = true;	
	}

	void CCPUDebug::CPUActive()
	{
#ifdef DEBUGGER_HASTHREAD
		m_mutex->Lock();

		active = true;
		m_condVar->Signal();

		m_mutex->Unlock();
#else
		active = true;
#endif // DEBUGGER_HASTHREAD
	}

	void CCPUDebug::CPUInactive()
	{
#ifdef DEBUGGER_HASTHREAD
		m_mutex->Lock();

		active = false;
		m_condVar->Signal();

		m_mutex->Unlock();
#else
		active = false;
#endif // DEBUGGER_HASTHREAD
	}

	void CCPUDebug::WaitCommand(EHaltReason reason)
	{
#ifdef DEBUGGER_HASTHREAD
		m_mutex->Lock();

		m_halted = true;
		m_condVar->Signal();

		if (debugger->MakePrimary(this))
		{
			if (reason != HaltNone)
				debugger->ExecutionHalted(this, reason);
			debugger->WaitCommand(this);

			if (!m_stateUpdated)
				debugger->ReleasePrimary();
		}
		else
		{
			if (reason != HaltNone)
				debugger->ExecutionHalted(this, reason);
			while (m_breakWait)
				m_condVar->Wait(m_mutex);
		}

		m_halted = false;
		m_condVar->Signal();

		m_mutex->Unlock();
#else
		if (reason != HaltNone)
			debugger->ExecutionHalted(this, reason);
		debugger->WaitCommand(this);

		debugger->ClearBreak();
#endif // DEBUGGER_HASTHREAD
	}

#ifdef DEBUGGER_HASTHREAD
	void CCPUDebug::ForceWait()
	{
		m_mutex->Lock();

		m_breakWait = true;
		m_break = true;
		m_condVar->Signal();

		m_mutex->Unlock();
	}

	void CCPUDebug::WaitForHalt()
	{
		m_mutex->Lock();
			
		// Wait for CPU to become inactive or halt
		while (active && !m_halted)
			m_condVar->Wait(m_mutex);
		
		m_mutex->Unlock();
	}

	void CCPUDebug::ClearWait()
	{
		m_mutex->Lock();
		
		ClearBreak();
		m_condVar->Signal();

		m_mutex->Unlock();
	}
#endif // DEBUGGER_HASTHREAD

	void CCPUDebug::DebuggerReset()
	{
		instrCount = 0;
		
		// TODO - reset breakpoints, watches, I/O ports, exceptions, interrupts and reg monitors
		
		// Reset code analyser
		if (m_analyser != NULL)
			m_analyser->Reset();
	}

	void CCPUDebug::DebuggerPolled()
    {
        cyclesPerPoll = totalCycles - m_prevTotalCycles;
        m_prevTotalCycles = totalCycles;
	}

	UINT64 CCPUDebug::ReadMem(UINT32 addr, unsigned dataSize)
	{
		// TODO - currently assumes big-endian - should act according to this.bigEndian
		UINT64 dataH, dataL;
		switch (dataSize)
		{
			case 1:  
				// Sub-class should at the very least override and implement this case
				return 0;
			case 2:  
				dataH = ReadMem(addr, 1);
				dataL = ReadMem(addr + 1, 1);
				return (dataH<<8) | dataL;
			case 4:
				dataH = ReadMem(addr, 2);
				dataL = ReadMem(addr + 2, 2);
				return (dataH<<16) | dataL;
			case 8:
				dataH = ReadMem(addr, 4);
				dataL = ReadMem(addr + 4, 4);
				return (dataH<<32) | dataL;
			default: return 0;
		}
	}

	bool CCPUDebug::WriteMem(UINT32 addr, unsigned dataSize, UINT64 data)
	{
		// TODO - currently assumes big-endian - should act according to this.bigEndian
		UINT64 dataH, dataL;
		switch (dataSize)
		{
			case 1:  
				// Sub-class should at the very least override and implement this case
				return false;
			case 2:
				dataH = (data>>8)&0xFF;
				dataL = data&0xFF;
				return WriteMem(addr, 1, dataH) && WriteMem(addr + 1, 1, dataL);
			case 4:
				dataH = (data>>16)&0xFFFF;
				dataL = data&0xFFFF;
				return WriteMem(addr, 2, dataH) && WriteMem(addr + 2, 2, dataL);
			case 8:
				dataH = data>>32;
				dataL = data&0xFFFFFFFF;
				return WriteMem(addr, 4, dataH) && WriteMem(addr + 4, 4, dataL);
			default: return false;
		}
	}

	UINT64 CCPUDebug::ReadPort(UINT16 portNum) 
	{
		return 0; 
	}

	bool CCPUDebug::WritePort(UINT16 portNum, UINT64 data) 
	{
		return false; 
	}

	int CCPUDebug::GetOpLength(UINT32 addr)
	{
		// Default is to use disassemble to work out instruction length
		char mnemonic[255];
		char operands[255];
		return Disassemble(addr, mnemonic, operands);
	}

	UINT32 CCPUDebug::GetOpcode(UINT32 addr)
	{
		return (UINT32)ReadMem(addr, min<int>(4, minInstrLen));
	}
}

#endif  // SUPERMODEL_DEBUGGER
