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
 * CPUDebug.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_CPUDEBUG_H
#define INCLUDED_CPUDEBUG_H

#include <stdio.h>
#include <vector>
#include <algorithm>
using namespace std;

#include "Types.h"

#include "CodeAnalyser.h"
#include "AddressTable.h"
#include "Breakpoint.h"
#include "Debugger.h"
#include "Exception.h"
#include "Interrupt.h"
#include "IO.h"
#include "Register.h"
#include "Watch.h"

#ifdef DEBUGGER_HASBLOCKFILE
#include "BlockFile.h"
#endif // DEBUGGER_HASBLOCKFILE

#ifdef DEBUGGER_HASTHREAD
#include "OSD/Thread.h"
#endif // DEBUGGER_HASTHREAD

#define MAX_EXCEPTIONS 255
#define MAX_INTERRUPTS 255
#define MAX_IOPORTS 255

namespace Debugger
{
	class CRegion;
	class CLabel;
	class CComment;

	enum EStepMode
	{
		StepInto = 0,
		StepOver,
		StepOut
	};

	enum EOpFlags
	{
		NormalOp    = 0,
		JumpSimple  = 1,
		JumpLoop    = 2,
		JumpSub     = 4,
		JumpEx      = 8,
		ReturnSub   = 16,
		ReturnEx    = 32,
		HaltExec    = 64,
		Relative    = 128,
		Conditional = 256,
		NotFixed    = 512
	};

	/*
	 * Base class that facilitates the debugging of a particular emulated CPU.
	 * It holds the debugging hooks that the CPU should call to implement debugging.
	 * Also contains CPU-specific information and provides access to CPU-specific objects such as CRegister, CException etc.
	 */
	class CCPUDebug
	{
	friend class CDebugger;

	private:
		bool m_break;
#ifdef DEBUGGER_HASTHREAD
		bool m_breakWait;
#endif // DEBUGGER_HASTHREAD
		bool m_breakUser;
		bool m_halted;
		bool m_step;
		EStepMode m_stepMode;
		bool m_stepBreak;
		bool m_steppingOver;
		UINT32 m_stepOverAddr;
		bool m_steppingOut;
		UINT32 m_stepOutAddr;
		int m_count;
		bool m_until;
		UINT32 m_untilAddr;

#ifdef DEBUGGER_HASTHREAD
		CMutex *m_mutex;
		CCondVar *m_condVar;
#endif // DEBUGGER_HASTHREAD

		CException *m_exArray[MAX_EXCEPTIONS];
		CInterrupt *m_intArray[MAX_INTERRUPTS];
		CPortIO *m_portArray[MAX_IOPORTS];

		CAddressTable *m_mappedIOTable;
		CAddressTable *m_memWatchTable;
		CAddressTable *m_bpTable;

		int m_numRegMons;
		CRegMonitor **m_regMonArray;

		bool ShiftAddress(UINT32 &addr, unsigned &dataSize, UINT64 &data, CAddressRef *ref);

		void CheckRead(UINT32 addr, unsigned dataSize, UINT64 data);

		void CheckWrite(UINT32 addr, unsigned dataSize, UINT64 data);
		
		void MemWatchTriggered(CWatch *watch, UINT32 addr, unsigned dataSize, UINT64 data, bool isRead);

		void IOWatchTriggered(CWatch *watch, CIO *io, UINT64 data, bool isRead);

		void AttachToDebugger(CDebugger *theDebugger);

		void DetachFromDebugger(CDebugger *theDebugger);
		
	protected:
		CCodeAnalyser *m_analyser;

		bool m_stateUpdated;
		CException *m_exRaised;
		CException *m_exTrapped;
		CInterrupt *m_intRaised;
		CInterrupt *m_intTrapped;
		CBreakpoint *m_bpReached;
		CWatch *m_memWatchTriggered;
		CWatch *m_ioWatchTriggered;
		CRegMonitor *m_regMonTriggered;

		UINT64 m_prevTotalCycles;

		CCPUDebug(const char *cpuType, const char *cpuName, 
			UINT8 cpuMinInstrLen, UINT8 cpuMaxInstrLen, bool cpuBigEndian, UINT8 cpuMemBusWidth, UINT8 cpuMaxMnemLen);

		//
		// Protected virtual methods for sub-class to implement
		//

		virtual bool UpdatePC(UINT32 pc) = 0;
		
		virtual bool ForceException(CException *ex) = 0;

		virtual bool ForceInterrupt(CInterrupt *in) = 0;

	public:
		const char *type;
		const char *name;
		
		const UINT8 minInstrLen;
		const UINT8 maxInstrLen;
		const bool bigEndian;
		const UINT8 memBusWidth;
		const UINT8 maxMnemLen;
		
		bool enabled;
		EFormat addrFmt;
		EFormat portFmt;
		EFormat dataFmt;

		CDebugger *debugger;

		UINT16 numExCodes;
		UINT16 numIntCodes;
		UINT16 numPorts;
		UINT32 memSize;

		bool active;
		UINT64 instrCount;
		UINT64 totalCycles;
		UINT64 cyclesPerPoll;
		UINT32 pc;
		UINT32 opcode;

		vector<CRegister*> regs;
		vector<CException*> exceps;
		vector<CInterrupt*> inters;
		vector<CIO*> ios;
		// TODO - should use map<UINT32,T*> for T=CRegion,CLabel&CComment so that look-ups via address are faster
		vector<CRegion*> regions;
		vector<CLabel*> labels;
		vector<CComment*> comments;
		vector<CWatch*> memWatches;
		vector<CWatch*> ioWatches;
		vector<CBreakpoint*> bps;
		vector<CRegMonitor*> regMons;

		virtual ~CCPUDebug();

		//
		// Methods to define CPU registers (must be called before attached to debugger)

		CPCRegister *AddPCRegister(const char *name, const char *group);

		CAddrRegister *AddAddrRegister(const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc = NULL);

		CIntRegister *AddInt8Register(const char *name, const char *group, unsigned id, GetInt8FPtr getFunc, SetInt8FPtr setFunc = NULL);

		CIntRegister *AddInt16Register(const char *name, const char *group, unsigned id, GetInt16FPtr getFunc, SetInt16FPtr setFunc = NULL);

		CIntRegister *AddInt32Register(const char *name, const char *group, unsigned id, GetInt32FPtr getFunc, SetInt32FPtr setFunc = NULL);

		CIntRegister *AddInt64Register(const char *name, const char *group, unsigned id, GetInt64FPtr getFunc, SetInt64FPtr setFunc = NULL);

		CStatusRegister *AddStatus8Register(const char *name, const char *group, unsigned id, const char *bitStr, GetInt8FPtr getFunc, SetInt8FPtr setFunc = NULL);
		
		CStatusRegister *AddStatus16Register(const char *name, const char *group, unsigned id, const char *bitStr, GetInt16FPtr getFunc, SetInt16FPtr setFunc = NULL);

		CStatusRegister *AddStatus32Register(const char *name, const char *group, unsigned id, const char *bitStr, GetInt32FPtr getFunc, SetInt32FPtr setFunc = NULL);

		CFPointRegister *AddFPointRegister(const char *name, const char *group, unsigned id, GetFPointFPtr getFunc, SetFPointFPtr setFunc = NULL);

		//
		// Methods to add exceptions and interrupts (must be called before attached to debugger)
		//

		CException *AddException(const char *id, UINT16 code, const char *name);

		CInterrupt *AddInterrupt(const char *id, UINT16 code, const char *name);

		//
		// Methods to define memory layout (must be called before attached to debugger)
		//

		CRegion *AddRegion(UINT32 start, UINT32 end, bool isCode, bool isReadOnly, const char *name);

		CPortIO *AddPortIO(UINT16 portNum, unsigned dataSize, const char *name, const char *group);

		CMappedIO *AddMappedIO(UINT32 addr, unsigned dataSize, const char *name, const char *group);

		//
		// Parsing and formatting methods
		//

		bool ParseData(const char *str, unsigned dataSize, UINT64 *data, bool registers = true);

		void FormatData(char *str,  unsigned dataSize, UINT64 data);

		bool ParseAddress(const char *str, UINT32 *addr, bool customLabels = true, bool autoLabels = true, bool registers = true);

		void FormatAddress(char *str, UINT32 addr, bool customLabels = false, ELabelFlags autoLabelFlags = LFNone);

		void FormatJumpAddress(char *str, UINT32 jumpAddr, EOpFlags opFlags);

		bool ParsePortNum(const char *str, UINT16 *portNum, bool customLabels = true, bool registers = true);

		void FormatPortNum(char *str, UINT16 portNum, bool customLabels = false);

		//
		// Checking methods that hook into CPU emulation code
		//

		/*
		 * Should be called after every 8-bit read.
		 */
		void CheckRead8(UINT32 addr, UINT8 data);

		/*
		 * Should be called after every 16-bit read.
		 */
		void CheckRead16(UINT32 addr, UINT16 data);
		
		/*
		 * Should be called after every 32-bit read.
		 */
		void CheckRead32(UINT32 addr, UINT32 data);

		/*
		 * Should be called after every 64-bit read.
		 */
		void CheckRead64(UINT32 addr, UINT64 data);

		/*
		 * Should be called after every 8-bit write.
		 */
		void CheckWrite8(UINT32 addr, UINT8 data);

		/*
		 * Should be called after every 16-bit write.
		 */
		void CheckWrite16(UINT32 addr, UINT16 data);
		
		/*
		 * Should be called after every 32-bit write.
		 */
		void CheckWrite32(UINT32 addr, UINT32 data);

		/*
		 * Should be called after every 64-bit write.
		 */
		void CheckWrite64(UINT32 addr, UINT64 data);

		/*
		 * Should be called after every read from an I/O port.
		 */
		void CheckPortInput(UINT16 portNum, UINT64 data);
		
		/*
		 * Should be called after every write to an I/O port.
		 */
		void CheckPortOutput(UINT16 portNum, UINT64 data);

		/*
		 * Should be called by CPU when it becomes active and starts executing an instruction loop.
		 * This call is needed so that the debugger can handle multi-threading of CPUs.
		 */
		void CPUActive();

		/*
		 * Should be called by CPU when it becomes inactive after having finished executing an instruction loop.
		 * This call is needed so that the debugger can handle multi-threading of CPUs.
		 */
		void CPUInactive();

		/*
		 * Should be called by CPU before every instruction.
		 * If returns true, then PC may have been changed by user and/or an exception/interrupt may have forced, so CPU core should 
		 * check for this and handle appropriately.  Otherwise, it can continue to execute as normal.
		 */
		bool CPUExecute(UINT32 newPC, UINT32 newOpcode, UINT32 lastCycles);

		/*
		 * Should be called by CPU whenever a CPU exception is raised (and before the exception handler is executed).
		 */
		virtual void CPUException(UINT16 exCode);

		/*
		 * Should be called by CPU whenever a CPU interrupt is raised (and before the interrupt handler is executed).
		 */
		virtual void CPUInterrupt(UINT16 intCode);

		void WaitCommand(EHaltReason reason);

#ifdef DEBUGGER_HASTHREAD
		void ForceWait();

		void WaitForHalt();

		void ClearWait();
#endif // DEBUGGER_HASTHREAD

		//
		// Execution control
		//

		void ForceBreak(bool user);

		void ClearBreak();

		void SetContinue();

		void SetStepMode(EStepMode stepMode);

		void SetCount(int count);

		void SetUntil(UINT32 untilAddr);

		bool SetPC(UINT32 newPC);

		bool GenerateException(CException *ex);

		bool GenerateInterrupt(CInterrupt *in);

		//
		// Register access
		//

		CRegister *GetRegister(const char *name);

		//
		// Exception access
		//

		CException *GetException(const char *id);

		CException *GetException(UINT16 code);

		//
		// Interrupt access
		//

		CInterrupt *GetInterrupt(const char *id);

		CInterrupt *GetInterrupt(UINT16 code);

		//
		// I/O access
		//

		CPortIO *GetPortIO(UINT16 portNum);

		CMappedIO *GetMappedIO(UINT32 addr);

		//
		// Region access
		//

		CRegion *GetRegion(UINT32 addr);

		//
		// Label handling
		//

		CLabel *AddLabel(UINT32 addr, const char *name);

		CLabel *GetLabel(UINT32 addr);

		CLabel *GetLabel(const char *name);

		bool RemoveLabel(const char *name);

		bool RemoveLabel(UINT32 addr);

		bool RemoveLabel(CLabel *label);

		bool RemoveAllLabels();

		//
		// Comment handling
		//

		CComment *AddComment(UINT32 addr, const char *commentText);

		CComment *GetComment(UINT32 addr);

		bool RemoveComment(UINT32 addr);

		bool RemoveComment(CComment *comment);

		bool RemoveAllComments();

		//
		// Watch handling
		//

		CSimpleWatch *AddSimpleMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite);

		CCountWatch *AddCountMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite, unsigned count);

		CMatchWatch *AddMatchMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite, vector<UINT64> &dataSeq);

		CPrintWatch *AddPrintMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite);

		CCaptureWatch *AddCaptureMemWatch(UINT32 addr, UINT32 size, bool trigRead, bool trigWrite, unsigned maxLen);

		bool RemoveAllMemWatches();

		bool RemoveAllIOWatches();

		bool RemoveMemWatch(UINT32 addr, unsigned size);

		CWatch *GetMemWatch(UINT32 addr, unsigned size);

		void AddWatch(CWatch *watch);

		bool RemoveWatch(CWatch *watch);

		void UpdateIOWatchNums();
	
		void UpdateMemWatchNums();

		//
		// Breakpoint handling
		//

		CSimpleBreakpoint *AddSimpleBreakpoint(UINT32 addr);

		CCountBreakpoint *AddCountBreakpoint(UINT32 addr, unsigned count);

		CPrintBreakpoint *AddPrintBreakpoint(UINT32 addr);

		void AddBreakpoint(CBreakpoint *bp);

		CBreakpoint *GetBreakpoint(UINT32 addr);

		bool RemoveBreakpoint(UINT32 addr);

		bool RemoveBreakpoint(CBreakpoint *bp);

		bool RemoveAllBreakpoints();

		void UpdateBreakpointNums();

		//
		// Register monitor handling
		//

		CRegMonitor *AddRegMonitor(const char *regName);

		CRegMonitor *GetRegMonitor(const char *regName);

		void AddRegMonitor(CRegMonitor *regMon);

		bool RemoveRegMonitor(const char *regName);

		bool RemoveRegMonitor(CRegMonitor *regMon);

		bool RemoveAllRegMonitors();

		void UpdateRegMonArray();

		//
		// Code analyser
		//

		CCodeAnalyser *GetCodeAnalyser();

		//
		// Memory searching
		//

		bool FindInMem(UINT32 start, UINT32 end, const char *str, bool matchCase, UINT32 &findAddr);

		bool FindInMem(UINT32 start, UINT32 end, UINT8 *bytes, unsigned length, UINT32 &findAddr); 

		//
		// Debugger state loading/saving
		//

#ifdef DEBUGGER_HASBLOCKFILE
		bool LoadState(CBlockFile *state);

		bool SaveState(CBlockFile *state);
#endif // DEBUGGER_HASBLOCKFILE

		//
		// Public virtual methods for sub-class to implement
		//

		virtual void AttachToCPU() = 0;

		virtual void DetachFromCPU() = 0;

		virtual void DebuggerReset();

		virtual void DebuggerPolled();

		virtual UINT32 GetResetAddr() = 0;

		virtual UINT64 ReadMem(UINT32 addr, unsigned dataSize);

		virtual bool WriteMem(UINT32 addr, unsigned dataSize, UINT64 data);

		virtual UINT64 ReadPort(UINT16 portNum);

		virtual bool WritePort(UINT16 portNum, UINT64 data);

		virtual int Disassemble(UINT32 addr, char *mnemonic, char *operands) = 0;

		virtual int GetOpLength(UINT32 addr);

		// Returns head (no more than 32-bits or min instr length) of full opcode for use with following methods
		virtual UINT32 GetOpcode(UINT32 addr);

		virtual EOpFlags GetOpFlags(UINT32 addr, UINT32 opcode) = 0;

		virtual bool GetJumpAddr(UINT32 addr, UINT32 opcode, UINT32 &jumpAddr) = 0;
		
		virtual bool GetJumpRetAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr) = 0;

		virtual bool GetReturnAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr) = 0;

		virtual bool GetHandlerAddr(CException *ex, UINT32 &handlerAddr) = 0;

		virtual bool GetHandlerAddr(CInterrupt *in, UINT32 &handlerAddr) = 0;
	};

	//
	// Inlined methods
	//

	inline bool CCPUDebug::ShiftAddress(UINT32 &addr, unsigned &dataSize, UINT64 &data, CAddressRef *ref)
	{
		UINT32 refAddr = ref->addr;
		unsigned refSize = ref->size;
		unsigned offset;
		if (refAddr >= addr)
		{
			offset = refAddr - addr;
			// dataSize will always be >= offset
			dataSize -= offset;
			if (dataSize <= refSize)
				return false;
			addr = refAddr + refSize;
			dataSize -= refSize;
			//data <<= (8 * (offset + refSize));
			return true;
		}
		else
		{
			offset = refAddr + refSize - addr;
			if (dataSize <= offset)
				return false;
			addr += offset;
			dataSize -= offset;
			//data <<= (8 * offset); 
			return true;
		}
	}

	inline void CCPUDebug::CheckRead(UINT32 addr, unsigned dataSize, UINT64 data)
	{
		// TODO - currently assumes big-endian - should act according to this.bigEndian
		
		// For reads larger than 1 byte, care is taken with mapped I/O or watches that overlap within the read region
		while (dataSize > 0)
		{
			// Check if address is mapped I/O
			CMappedIO *mappedIO = NULL;
			if (m_mappedIOTable != NULL)
			{
				mappedIO = (CMappedIO*)m_mappedIOTable->Get(addr, dataSize);
				if (mappedIO != NULL)
				{
					// Check with I/O if its watch is triggered
					if (mappedIO->CheckInput(addr, dataSize, data))
						IOWatchTriggered(mappedIO->watch, mappedIO, data, true);

					// See if still have remaining data to check
					if (!ShiftAddress(addr, dataSize, data, mappedIO))
						return;
				}
			}

			// Check if address has a watch
			CWatch *watch = NULL;
			if (m_memWatchTable != NULL)
			{
				watch = (CWatch*)m_memWatchTable->Get(addr, dataSize);
				if (watch != NULL)
				{
					// Check if watch is triggered
					if (watch->CheckRead(addr, dataSize, data))
						MemWatchTriggered(watch, addr, dataSize, data, true);

					// See if still have remaining data to check
					if (!ShiftAddress(addr, dataSize, data, watch))
						return;
				}
				else if (mappedIO == NULL)
					return;
			}
			else if (mappedIO == NULL)
				return;
		}
	}

	inline void CCPUDebug::CheckWrite(UINT32 addr, unsigned dataSize, UINT64 data)
	{
		// TODO - currently assumes big-endian - should act according to this.bigEndian

		// For writes larger than 1 byte, care is taken with mapped I/O or watches that overlap within the written region
		while (dataSize > 0)
		{
			// Check if writing to mapped I/O address
			CMappedIO *mappedIO = NULL;
			if (m_mappedIOTable != NULL)
			{
				mappedIO = (CMappedIO*)m_mappedIOTable->Get(addr, dataSize);
				if (mappedIO != NULL)
				{
					// Check with I/O if its watch is triggered
					if (mappedIO->CheckOutput(addr, dataSize, data))
						IOWatchTriggered(mappedIO->watch, mappedIO, data, false);

					// See if still have remaining data to check
					if (!ShiftAddress(addr, dataSize, data, mappedIO))
						return;
				}
			}

			// Check if address has a watch
			CWatch *watch = NULL;
			if (m_memWatchTable != NULL)
			{
				watch = (CWatch*)m_memWatchTable->Get(addr, dataSize);
				if (watch != NULL)
				{
					// Check if watch is triggered
					if (watch->CheckWrite(addr, dataSize, data))
						MemWatchTriggered(watch, addr, dataSize, data, false);

					// See if still have remaining data to check
					if (!ShiftAddress(addr, dataSize, data, watch))
						return;			
				}
				else if (mappedIO == NULL)
					return;
			}
			else if (mappedIO == NULL)
				return;
		}
	}

	inline void CCPUDebug::MemWatchTriggered(CWatch *watch, UINT32 addr, unsigned dataSize, UINT64 data, bool read)
	{
		m_memWatchTriggered = watch;
		debugger->MemWatchTriggered(watch, addr, dataSize, data, read);
		watch->Reset();
		m_break = true;
	}

	inline void CCPUDebug::IOWatchTriggered(CWatch* watch, CIO *io, UINT64 data, bool read)
	{
		m_ioWatchTriggered = watch;
		debugger->IOWatchTriggered(watch, io, data, read);
		watch->Reset();
		m_break = true;
	}

	inline void CCPUDebug::CheckRead8(UINT32 addr, UINT8 data)
	{
		// Check if reading from mapped I/O address
		unsigned dataSize = 1;
		if (m_mappedIOTable != NULL)
		{
			CMappedIO *mappedIO = (CMappedIO*)m_mappedIOTable->Get(addr);
			if (mappedIO != NULL)
			{
				if (mappedIO->CheckInput(addr, dataSize, data))
					IOWatchTriggered(mappedIO->watch, mappedIO, data, true);
				return;
			}
		}
		// Check if address has a watch
		if (m_memWatchTable == NULL)
			return;
		CWatch *watch = (CWatch*)m_memWatchTable->Get(addr);
		if (watch != NULL && watch->CheckRead(addr, dataSize, data))
			MemWatchTriggered(watch, addr, dataSize, data, true);
	}

	inline void CCPUDebug::CheckRead16(UINT32 addr, UINT16 data)
	{
		CheckRead(addr, 2, data);
	}

	inline void CCPUDebug::CheckRead32(UINT32 addr, UINT32 data)
	{
		CheckRead(addr, 4, data);
	}

	inline void CCPUDebug::CheckRead64(UINT32 addr, UINT64 data)
	{
		CheckRead(addr, 8, data);
	}

	inline void CCPUDebug::CheckWrite8(UINT32 addr, UINT8 data)
	{
		// Check if writing to mapped I/O address
		unsigned dataSize = 1;
		if (m_mappedIOTable != NULL)
		{
			CMappedIO *mappedIO = (CMappedIO*)m_mappedIOTable->Get(addr);
			if (mappedIO != NULL)
			{
				if (mappedIO->CheckOutput(addr, dataSize, data))
					IOWatchTriggered(mappedIO->watch, mappedIO, data, false);
				return;
			}
		}
		// Check if address has a watch
		if (m_memWatchTable == NULL)
			return;
		CWatch *watch = (CWatch*)m_memWatchTable->Get(addr);
		if (watch != NULL && watch->CheckWrite(addr, dataSize, data))
			MemWatchTriggered(watch, addr, dataSize, data, false);
	}

	inline void CCPUDebug::CheckWrite16(UINT32 addr, UINT16 data)
	{
		CheckWrite(addr, 2, data);
	}

	inline void CCPUDebug::CheckWrite32(UINT32 addr, UINT32 data)
	{
		CheckWrite(addr, 4, data);
	}

	inline void CCPUDebug::CheckWrite64(UINT32 addr, UINT64 data)
	{
		CheckWrite(addr, 8, data);
	}

	inline void CCPUDebug::CheckPortInput(UINT16 portNum, UINT64 data)
	{
		if (portNum >= numPorts)
			return;
		CPortIO *port = m_portArray[portNum];
		if (port != NULL && port->CheckInput(data))
			IOWatchTriggered(port->watch, port, data, true);
	}

	inline void CCPUDebug::CheckPortOutput(UINT16 portNum, UINT64 data)
	{
		if (portNum >= numPorts)
			return;
		CPortIO *port = m_portArray[portNum];
		if (port != NULL && port->CheckOutput(data))
			IOWatchTriggered(port->watch, port, data, false);
	}	

	inline bool CCPUDebug::CPUExecute(UINT32 newPC, UINT32 newOpcode, UINT32 lastCycles)
	{
		// Check if debugging is enabled for this CPU
		if (!enabled)
		{
			// If not, update instruction count, total cycles counts, pc and opcode but don't allow any execution control
			instrCount++;
			totalCycles += lastCycles;
			pc = newPC;
			opcode = newOpcode;		
			return false;
		}

		// Check not just updating state
		bool stepBreak, countBreak, untilBreak;
		if (!m_stateUpdated)
		{
			// If so, then first check to see if any registers have changed from previous instruction before updating pc and opcode
			if (m_numRegMons > 0)
			{
				for (int i = 0; i < m_numRegMons; i++)
				{
					CRegMonitor *regMon = m_regMonArray[i];
					if (regMon->HasChanged())
					{
						m_regMonTriggered = regMon;
						debugger->MonitorTriggered(regMon);
						regMon->Reset();
						m_break = true;
					}
				}
			}

			// Now update instruction count, total cycles count, pc and opcode
			instrCount++;
			totalCycles += lastCycles;
			pc = newPC;
			opcode = newOpcode;		

			// Next check for breakpoints at new pc and opcode
			if (m_bpTable != NULL)
			{
				CBreakpoint *bp = (CBreakpoint*)m_bpTable->Get(pc);
				if (bp != NULL && bp->Check(pc, opcode))
				{
					m_bpReached = bp;
					debugger->BreakpointReached(bp);
					m_break = true;
				}
			}
			
			// Check if currently implementing a step mode
			if (m_step)
			{
				if (instrCount == 1)
					stepBreak = true;
				else
				{
					switch (m_stepMode)
					{
						case StepInto: 
							// Step-into always breaks
							stepBreak = true; 
							break;
						case StepOver:
							// Step-over breaks if was not stepping over a jump or was stepping over and has returned from jump
							stepBreak = !m_steppingOver || pc == m_stepOverAddr;
							break;
						case StepOut:
							// Step-out steps over any jumps and breaks when it reaches a return or an exception/interrupt is raised
							if (m_exRaised != NULL || m_intRaised != NULL)
								stepBreak = true;
							else if (m_steppingOver)
							{
								if (pc == m_stepOverAddr)
									m_steppingOver = false;
								stepBreak = false;
							}
							else
								stepBreak = m_steppingOut && pc == m_stepOutAddr;
							break;
						default:
							stepBreak = false;
							break;
					}
				}
			}
			else
				stepBreak = false;
			
			// Lastly, check if counting instructions or if running until a given address
			countBreak = m_count > 0 && --m_count == 0;
			untilBreak = m_until && pc == m_untilAddr;
		}
		else
		{
			// Just updating state so update pc and opcode, but do not update instruction count
			pc = newPC;
			opcode = newOpcode;		

			stepBreak  = false;
			countBreak = false;
			untilBreak = false;
		}

		// If a break has been forced, state was updated, stepping through code or have run a set number of instructions or to a 
		// given address then break now and wait for command from user
		if (m_break || m_stateUpdated || stepBreak || countBreak || untilBreak)
		{
			// See if execution halt was caused by user manually breaking execution in some manner
			EHaltReason reason = HaltNone;
			if (m_stateUpdated) reason = (EHaltReason)(reason | HaltState);
			if (m_breakUser)    reason = (EHaltReason)(reason | HaltUser);
			if (stepBreak)      reason = (EHaltReason)(reason | HaltStep);
			if (countBreak)     reason = (EHaltReason)(reason | HaltCount);
			if (untilBreak)     reason = (EHaltReason)(reason | HaltUntil);
			
			// Keep hold of breakpoint, if any, and its address so that can reset it later
			UINT32 bpAddr = 0;
			CBreakpoint *bpToReset = NULL;
			if (m_bpReached != NULL)
			{
				bpAddr = m_bpReached->addr;
				bpToReset = m_bpReached;
			}

			// Reset all control flags
			m_stateUpdated = false;
			m_step = false;
			m_steppingOver = false;
			m_steppingOut = false;
			m_count = 0;
			m_until = false;
			m_exRaised = NULL;
			m_exTrapped = NULL;
			m_intRaised = NULL;
			m_intTrapped = NULL;
			m_bpReached = NULL;
			m_memWatchTriggered = NULL;
			m_ioWatchTriggered = NULL;
			m_regMonTriggered = NULL;
		
			// Wait for instruction from user
			WaitCommand(reason);
			
			// Reset breakpoint, if any
			if (bpToReset != NULL && m_bpTable != NULL)
			{
				// Check breakpoint to reset was not deleted by user
				CBreakpoint *bp = (CBreakpoint*)m_bpTable->Get(bpAddr);
				if (bp == bpToReset)
					bp->Reset();
			}

			// If stepping over, get step over address if available
			if (m_step && m_stepMode == StepOver)
				m_steppingOver = GetJumpRetAddr(pc, opcode, m_stepOverAddr);
		}

		// If stepping out, then make sure step over any jumps and if encounter a return instruction then break at return address
		if (m_step && m_stepMode == StepOut)
		{
			if (!m_steppingOver)
				m_steppingOver = GetJumpRetAddr(pc, opcode, m_stepOverAddr);
			if (!m_steppingOver)
				m_steppingOut = GetReturnAddr(pc, opcode, m_stepOutAddr);
		}

		return m_stateUpdated;
	}
}

#endif	// INCLUDED_CPUDEBUG_H
#endif  // SUPERMODEL_DEBUGGER