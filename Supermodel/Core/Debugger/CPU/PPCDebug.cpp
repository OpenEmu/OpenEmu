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
 * PPCDebug.cpp
 */

#ifdef SUPERMODEL_DEBUGGER

#include "PPCDebug.h"
#include "CPU/PowerPC/ppc.h"
#include "CPU/PowerPC/PPCDisasm.h"

#include <cctype>
#include <string>

#define M_AA 0x00000002
#define M_LK 0x00000001
#define M_BO 0x03E00000
#define M_BD 0x0000FFFC
#define M_LI 0x03FFFFFC

#define MSR_IP 0x00000040

namespace Debugger
{
	static UINT32 GetSpecialReg(CCPUDebug *cpu, unsigned id) 
	{
		switch (id)
		{
			case PPCSPECIAL_LR:    return ::ppc_get_lr();
			case PPCSPECIAL_FPSCR: return 0; // TODO
			default:               return 0;
		}
	}

	static bool SetSpecialReg(CCPUDebug *cpu, unsigned id, UINT32 data)
	{
		switch (id)
		{
			case PPCSPECIAL_LR:    /* TODO */ return false;
			case PPCSPECIAL_FPSCR: /* TODO */ return false;
			default:               return false;
		}
	}

	static UINT8 GetCR(CCPUDebug *cpu, unsigned id)
	{
		return ::ppc_get_cr(id);
	}

	static bool SetCR(CCPUDebug *cpu, unsigned id, UINT8 data)
	{
		::ppc_set_cr(id, data);
		return true;
	}

	static UINT32 GetSPR(CCPUDebug *cpu, unsigned id)
	{
		return ::ppc_read_spr(id);
	}

	static bool SetSPR(CCPUDebug *cpu, unsigned id, UINT32 data)
	{
		::ppc_write_spr(id, data);
		return true;
	}

	static UINT32 GetGPR(CCPUDebug *cpu, unsigned id)
	{
		return ::ppc_get_gpr(id);
	}

	static bool SetGPR(CCPUDebug *cpu, unsigned id, UINT32 data)
	{
		::ppc_set_gpr(id, data);
		return true;
	}

	static double GetFPR(CCPUDebug *cpu, unsigned id)
	{
		return ::ppc_get_fpr(id);
	}

	static bool SetFPR(CCPUDebug *cpu, unsigned id, double data)
	{
		::ppc_set_fpr(id, data);
		return true;
	}

	static const char *srGroup = "Special Registers";
	static const char *crGroup = "Condition Registers";
	static const char *grGroup = "GPR Registers";
	static const char *frGroup = "FPR Registers";

	CPPCDebug::CPPCDebug(const char *name) : CCPUDebug("PPC", name, 4, 4, true, 32, 7), m_irqState(0)
	{
		// PC & Link registers
		AddPCRegister  ("pc", srGroup);
		AddAddrRegister("lr", srGroup, PPCSPECIAL_LR, GetSpecialReg, SetSpecialReg);

		// SPR registers
		AddInt32Register   ("ctr",  srGroup, SPR_LR,   GetSPR, SetSPR);
		AddStatus32Register("xer",  srGroup, SPR_XER,  "SOC", GetSPR, SetSPR);
		AddInt32Register   ("srr0", srGroup, SPR_SRR0, GetSPR, SetSPR);
		AddInt32Register   ("srr1", srGroup, SPR_SRR1, GetSPR, SetSPR);
		// etc...
		
		// Condition registers
		for (unsigned id = 0; id < 8; id++)
		{
			sprintf(m_crNames[id], "cr%u", id);
			AddStatus8Register(m_crNames[id], crGroup, id, "O=><", GetCR, SetCR);
		}
		//AddStatus16Register("fpscr", "Condition Registers", PPCSPECIAL_FPSCR, "FEVOUZX789ABCRI 0123", GetSpecial, SetSpecial);
		
		// GPR registers
		for (unsigned id = 0; id < 32; id++)
		{
			sprintf(m_gprNames[id], "r%u", id);
			AddInt32Register(m_gprNames[id], grGroup, id, GetGPR, SetGPR);
		}
		
		// FPR registers
		for (unsigned id = 0; id < 32; id++)
		{
			sprintf(m_fprNames[id], "f%u", id);
			AddFPointRegister(m_fprNames[id], frGroup, id, GetFPR, SetFPR);
		}

		// Exceptions
		AddException("IRQ",     EXCEPTION_IRQ,         "External Interrupt");
		AddException("DEC",     EXCEPTION_DECREMENTER, "Decrement Overflow");
		AddException("TRAP",    EXCEPTION_TRAP,        "Program Exception/Trap");
		AddException("SYSCALL", EXCEPTION_SYSTEM_CALL, "System Call");
		AddException("SMI",     EXCEPTION_SMI,         "SMI");
		AddException("DSI",     EXCEPTION_DSI,         "DSI");
		AddException("ISI",     EXCEPTION_ISI,         "ISI");
	}

	CPPCDebug::~CPPCDebug()
	{
		DetachFromCPU();	
	}

	void CPPCDebug::AttachToCPU()
	{
		::ppc_attach_debugger(this);
	}

	::CBus *CPPCDebug::AttachBus(::CBus *bus)
	{
		m_bus = bus;
		return this;
	}

	void CPPCDebug::DetachFromCPU()
	{
		::ppc_detach_debugger();
	}

	::CBus *CPPCDebug::DetachBus()
	{
		::CBus *bus = m_bus;
		m_bus = NULL;
		return bus;
	}

	UINT32 CPPCDebug::GetResetAddr()
	{
		// Reset address appears to be hardcoded to 0xFFF00100
		return 0xFFF00100;
	}

	bool CPPCDebug::UpdatePC(UINT32 pc)
	{
		::ppc_set_pc(pc);
		return true;
	}

	bool CPPCDebug::ForceException(CException *ex)
	{
		// TODO - no way to force exceptions
		return false;
	}

	bool CPPCDebug::ForceInterrupt(CInterrupt *in)
	{
		if (in->code > 7)
			return false;
		UINT8 irqState = m_bus->Read8(0xF0100018) | 1<<in->code;
		m_bus->Write8(0xF0100018, irqState);
		::ppc_set_irq_line(1); // TODO - what is irqline arg for?  not actually used?
		return true;
	}

	UINT64 CPPCDebug::ReadMem(UINT32 addr, unsigned dataSize)
	{
		switch (dataSize)
		{
			case 1:  return (UINT64)m_bus->Read8(addr);
			case 2:  return (UINT64)m_bus->Read16(addr);
			case 4:  return (UINT64)m_bus->Read32(addr);
			case 8:  return m_bus->Read64(addr);
			default: return 0;
		}
	}

	bool CPPCDebug::WriteMem(UINT32 addr, unsigned dataSize, UINT64 data)
	{
		switch (dataSize)
		{
			case 1:  m_bus->Write8(addr, (UINT8)data); return true;
			case 2:  m_bus->Write16(addr, (UINT16)data); return true;
			case 4:  m_bus->Write32(addr, (UINT32)data); return true;
			case 8:  m_bus->Write64(addr, data); return true;
			default: return false;
		}
	}

	void CPPCDebug::CheckException(UINT16 exCode)
	{
		CCPUDebug::CPUException(exCode);
		
		if (exCode == EXCEPTION_IRQ)
		{
			UINT8 irqState = m_bus->Read8(0xF0100018); // TODO - replace this with function pointer
			
			UINT8 newIRQs = (irqState^m_irqState)&irqState;
			for (int intCode = 0; newIRQs && intCode < 8; intCode++)
			{
				if (newIRQs&0x01)
					CPUInterrupt(intCode);
				newIRQs >>= 1;
			}
			m_irqState = irqState;
		}
	}

	int CPPCDebug::Disassemble(UINT32 addr, char *mnemonic, char *operands)
	{
		char opStr[255];
		char valStr[40];
		UINT32 opcode = m_bus->Read32(addr);
		operands[0] = '\0';
		if (!::DisassemblePowerPC(opcode, addr, mnemonic, opStr, true))
		{
			char *o = opStr;
			char *s = strstr(o, "0x");
			while (s)
			{
				strncpy(operands, o, s - o);
				operands[s - o] = '\0';
				s += 2;

				char *p = s;
				unsigned len = 0;
				UINT64 data = 0;
				while (p)
				{
					char c = toupper(*(p++));
					if (c >= '0' && c <= '9')
					{
						data <<= 4;
						data += (UINT64)(c - '0');
					}
					else if (c >= 'A' && c <= 'F')
					{
						data <<= 4;
						data += (UINT64)(10 + c - 'A');
					}
					else
						break;
					len++;
				}

				unsigned dataSize = (p - s) / 2;
				if (dataSize == (unsigned)(memBusWidth / 8))
				{
					EOpFlags opFlags = GetOpFlags(addr, opcode);
					FormatJumpAddress(valStr, (UINT32)data, opFlags);
				}
				else
					FormatData(valStr, dataSize, data);
				
				strcat(operands, valStr);
				operands += strlen(operands);
				o = p - 1;
				s = strstr(o, "0x");
			}
			strcat(operands, o);
			return 4;
		}
		else
			return -4;
	}

	EOpFlags CPPCDebug::GetOpFlags(UINT32 addr, UINT32 opcode)
	{
		EOpFlags opFlags;
		
		UINT32 op = opcode>>26;
		if (op == 0x10)
		{
			// Instruction is branch conditional: bc, bca, bcl or bcla
			UINT32 bo = (opcode&M_BO)>>21;
			if (opcode&M_LK)
			{
				if (opcode&M_AA)
					opFlags = JumpSub; // bcla
				else if (bo&0x04)
					opFlags = (EOpFlags)(JumpSub | Relative); // bcl without counter
				else	
					opFlags = (EOpFlags)(JumpSub | Relative); // bcl with counter
			}
			else
			{	
				if (opcode&M_AA)
					opFlags = JumpSimple; // bca
				else if (bo&0x04)
					opFlags = (EOpFlags)(JumpSimple | Relative); // bc without counter
				else
					opFlags = (EOpFlags)(JumpLoop | Relative); // bc with counter
			}
			// Check BO is not just branch always
			return ((bo&0x14) == 0x14 ? opFlags : (EOpFlags)(opFlags | Conditional));
		}
		else if (op == 0x12)
		{
			// Instruction is branch: b, ba, bl or bla
			if (opcode&M_LK)
			{
				if (opcode&M_AA)
					return JumpSub; // bla
				else
					return (EOpFlags)(JumpSub | Relative); // bl
			}
			else
			{
				if (opcode&M_AA)
					return JumpSimple; // ba
				else
					return (EOpFlags)(JumpSimple | Relative); // b
			}
		}
		else if (op == 0x13)
		{
			UINT32 exOp = (opcode>>1)&0x3ff;
			UINT32 bo = (opcode&M_BO)>>21;
			if (exOp == 0x0210)
			{
				// Instruction is branch conditional to count register: bcctr or bcctrl
				if (opcode&M_LK)
					opFlags = (EOpFlags)(JumpSub | Relative); // bcctrl
				else if (bo&0x04)
					opFlags = (EOpFlags)(JumpSimple | Relative); // bcctr without counter
				else
					opFlags = (EOpFlags)(JumpLoop | Relative); // bcctr with counter
				// Check BO is not just branch always
				return ((bo&0x14) == 0x14 ? opFlags : (EOpFlags)(opFlags | Conditional));
			}
			else if (exOp == 0x0010)
			{
				// Instruction is branch conditional to link register: bclr or bclrl
				if (opcode&M_LK)
					opFlags = (EOpFlags)(JumpSub | ReturnSub); // bclrl
				else
					opFlags = ReturnSub; // bclr
				// Check BO is not just branch always
				return ((bo&0x14) == 0x14 ? opFlags : (EOpFlags)(opFlags | Conditional));
			}
			else if (exOp == 0x0032)
			{
				// Instruction is return from interrupt: rfi
				return ReturnEx;
			}
			// TODO - traps etc
		}
		return NormalOp;
	}

	bool CPPCDebug::GetJumpAddr(UINT32 addr, UINT32 opcode, UINT32 &jumpAddr)
	{
		// Check instruction is one of following branches: b, ba, bl, bla, bc, bca, bcl or bcla
		UINT32 disp;
		UINT32 op = opcode>>26;
		if (op == 0x10)
		{
			// Instruction is b, ba, bl or bla, so calculate branch displacement
			disp = ((opcode&M_BD)>>2) * 4;
			if (disp & 0x00008000)
				disp |= 0xFFFF0000; // Sign extended
			if (opcode&M_AA)
				jumpAddr = disp; // ba or bla
			else
				jumpAddr = addr + disp; // b or bl
			return true;
		}
		else if (op == 0x12)
		{
			// Instruction is bc, bca, bcl or bcla, so calculate branch displacement
			disp = ((opcode&M_LI) >> 2) * 4;
			if (disp & 0x02000000)
				disp |= 0xFC000000; // Sign extended
			if (opcode&M_AA)
				jumpAddr = disp; // bca or bcla
			else
				jumpAddr = addr + disp; // bc or bcl
			return true;
		}
		return false;	
	}

	bool CPPCDebug::GetJumpRetAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr)
	{
		UINT32 op = opcode>>26;
		if ((op == 0x10 || op == 0x12) && (opcode&M_LK))
		{
			// Instruction is bl, bla, bcl or bcla (TODO - add bclrl?)
			retAddr = addr + 4;
			return true;
		}
		return false;
	}

	bool CPPCDebug::GetReturnAddr(UINT32 addr, UINT32 opcode, UINT32 &retAddr)
	{
		// Check instruction is one of following: bclr, bclrl or rfi
		if ((opcode>>26) != 0x13)
			return false;
		
		UINT32 exOp = (opcode>>1)&0x3ff;
		if (exOp == 0x0010)
		{
			// For bclr and blclr, return address is in link register
			retAddr = ::ppc_get_lr();
			return true;
		}
		else if (exOp == 0x0032)
		{
			// For rfi, return address is in SRR0
			retAddr = ::ppc_read_spr(SPR_SRR0);
			return true;
		}
		return false;
	}

	bool CPPCDebug::GetHandlerAddr(CException *ex, UINT32 &handlerAddr)
	{
		UINT32 msr  = ::ppc_read_msr();
		UINT32 base = (msr&MSR_IP ? 0xFFF00000 : 0x00000000);
		switch (ex->code)
		{
			case EXCEPTION_DSI:         handlerAddr = base + 0x0300; return true;
			case EXCEPTION_ISI:         handlerAddr = base + 0x0400; return true;
			case EXCEPTION_IRQ:         handlerAddr = base + 0x0500; return true;
			case EXCEPTION_TRAP:        handlerAddr = base + 0x0700; return true;
			case EXCEPTION_DECREMENTER: handlerAddr = base + 0x0900; return true;
			case EXCEPTION_SYSTEM_CALL: handlerAddr = base + 0x0C00; return true;
			case EXCEPTION_SMI:         handlerAddr = base + 0x1400; return true;
			default:                    return false;
		}
	}

	bool CPPCDebug::GetHandlerAddr(CInterrupt *in, UINT32 &handlerAddr)
	{
		UINT32 msr = ::ppc_read_msr();
		handlerAddr = (msr&MSR_IP ? 0xFFF00500 : 0x00000500);
		return true;
	}

	// CBus methods

	UINT8 CPPCDebug::Read8(UINT32 addr)
	{
		UINT8 data = m_bus->Read8(addr);
		CheckRead8(addr, data);
		return data;
	}

	UINT16 CPPCDebug::Read16(UINT32 addr)
	{
		UINT16 data = m_bus->Read16(addr);
		CheckRead16(addr, data);
		return data;
	}

	UINT32 CPPCDebug::Read32(UINT32 addr)
	{
		UINT32 data = m_bus->Read32(addr);
		CheckRead32(addr, data);
		return data;
	}

	UINT64 CPPCDebug::Read64(UINT32 addr)
	{
		UINT64 data = m_bus->Read64(addr);
		CheckRead64(addr, data);
		return data;
	}

	void CPPCDebug::Write8(UINT32 addr, UINT8 data)
	{
		m_bus->Write8(addr, data);
		CheckWrite8(addr, data);
	}

	void CPPCDebug::Write16(UINT32 addr, UINT16 data)
	{
		m_bus->Write16(addr, data);
		CheckWrite16(addr, data);
	}

	void CPPCDebug::Write32(UINT32 addr, UINT32 data)
	{
		m_bus->Write32(addr, data);
		CheckWrite32(addr, data);
	}

	void CPPCDebug::Write64(UINT32 addr, UINT64 data)
	{
		m_bus->Write64(addr, data);
		CheckWrite64(addr, data);
	}
}

#endif  // SUPERMODEL_DEBUGGER