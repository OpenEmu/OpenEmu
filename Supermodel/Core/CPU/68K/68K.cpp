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
 * 68K.cpp
 * 
 * 68K CPU interface. This is presently just a wrapper for the Musashi 68K core
 * and therefore, only a single CPU is supported. In the future, we may want to
 * add in another 68K core (eg., Turbo68K, A68K, or a recompiler). 
 *
 * To-Do List
 * ----------
 * - Registers may not completely describe the 68K state. Musashi also has
 *   additional CPU state information in the context that its MAME interface
 *   accesses (interrupts pending, halted status, etc.)
 */

#include "Supermodel.h"
#include "Musashi/m68k.h"	// Musashi 68K core


/******************************************************************************
 Internal Context
 
 An active context must be mapped before calling M68K interface functions. Only
 the bus and IRQ handlers are copied here; the CPU context is passed directly
 to Musashi.
******************************************************************************/

// Bus
static CBus	*Bus = NULL;

#ifdef SUPERMODEL_DEBUGGER
// Debugger
static CMusashi68KDebug *Debug = NULL;
#endif

// IRQ callback
static int	(*IRQAck)(int nIRQ) = NULL;


/******************************************************************************
 68K Interface
******************************************************************************/

// CPU state
	
UINT32 M68KGetARegister(int n)
{
	m68k_register_t	r;
	
	switch (n)
	{
	case 0:		r = M68K_REG_A0; break;
	case 1:		r = M68K_REG_A1; break;
	case 2:		r = M68K_REG_A2; break;
	case 3:		r = M68K_REG_A3; break;
	case 4:		r = M68K_REG_A4; break;
	case 5:		r = M68K_REG_A5; break;
	case 6:		r = M68K_REG_A6; break;
	case 7:		r = M68K_REG_A7; break;
	default:	r = M68K_REG_A7; break;
	}
	
	return m68k_get_reg(NULL, r);
}

UINT32 M68KGetDRegister(int n)
{
	m68k_register_t	r;
	
	switch (n)
	{
	case 0:		r = M68K_REG_D0; break;
	case 1:		r = M68K_REG_D1; break;
	case 2:		r = M68K_REG_D2; break;
	case 3:		r = M68K_REG_D3; break;
	case 4:		r = M68K_REG_D4; break;
	case 5:		r = M68K_REG_D5; break;
	case 6:		r = M68K_REG_D6; break;
	case 7:		r = M68K_REG_D7; break;
	default:	r = M68K_REG_D7; break;
	}
	
	return m68k_get_reg(NULL, r);
}

UINT32 M68KGetPC(void)
{
	return m68k_get_reg(NULL, M68K_REG_PC);
}

void M68KSaveState(CBlockFile *StateFile, const char *name)
{
	StateFile->NewBlock(name, __FILE__);
	
	/*
	 * Rather than writing the context directly, the get/set register
	 * functions are used, ensuring that all context members are packed/
	 * unpacked correctly.
	 *
	 * Note: Some of these are undoubtedly 68010 or 68020 registers and not
	 * really necessary. But if the layout is changed now, the save state 
	 * version has to be changed, so don't do it!
	 */
	 
	UINT32			data[34];
	m68ki_cpu_core	Ctx;
	
	m68k_get_context(&Ctx);
	
	data[0] = Ctx.int_level;
	data[1] = Ctx.int_cycles;
	data[2] = Ctx.stopped;
	data[3] = m68k_get_reg(NULL, M68K_REG_D0);
	data[4] = m68k_get_reg(NULL, M68K_REG_D1);
	data[5] = m68k_get_reg(NULL, M68K_REG_D2);
	data[6] = m68k_get_reg(NULL, M68K_REG_D3);
	data[7] = m68k_get_reg(NULL, M68K_REG_D4);
	data[8] = m68k_get_reg(NULL, M68K_REG_D5);
	data[9] = m68k_get_reg(NULL, M68K_REG_D6);
	data[10] = m68k_get_reg(NULL, M68K_REG_D7);
	data[11] = m68k_get_reg(NULL, M68K_REG_A0);
	data[12] = m68k_get_reg(NULL, M68K_REG_A1);
	data[13] = m68k_get_reg(NULL, M68K_REG_A2);
	data[14] = m68k_get_reg(NULL, M68K_REG_A3);
	data[15] = m68k_get_reg(NULL, M68K_REG_A4);
	data[16] = m68k_get_reg(NULL, M68K_REG_A5);
	data[17] = m68k_get_reg(NULL, M68K_REG_A6);
	data[18] = m68k_get_reg(NULL, M68K_REG_A7);
	data[19] = m68k_get_reg(NULL, M68K_REG_PC);
	data[20] = m68k_get_reg(NULL, M68K_REG_SR);
	data[21] = m68k_get_reg(NULL, M68K_REG_SP);
	data[22] = m68k_get_reg(NULL, M68K_REG_USP);
	data[23] = m68k_get_reg(NULL, M68K_REG_ISP);
	data[24] = m68k_get_reg(NULL, M68K_REG_MSP);
	data[25] = m68k_get_reg(NULL, M68K_REG_SFC);
	data[26] = m68k_get_reg(NULL, M68K_REG_DFC);
	data[27] = m68k_get_reg(NULL, M68K_REG_VBR);
	data[28] = m68k_get_reg(NULL, M68K_REG_CACR);
	data[29] = m68k_get_reg(NULL, M68K_REG_CAAR);
	data[30] = m68k_get_reg(NULL, M68K_REG_PREF_ADDR);
	data[31] = m68k_get_reg(NULL, M68K_REG_PREF_DATA);
	data[32] = m68k_get_reg(NULL, M68K_REG_PPC);
	data[33] = m68k_get_reg(NULL, M68K_REG_IR);
	
	StateFile->Write(data, sizeof(data));
}

void M68KLoadState(CBlockFile *StateFile, const char *name)
{
	if (OKAY != StateFile->FindBlock(name))
	{
		ErrorLog("Unable to load 68K state. Save state file is corrupt.");
		return;
	}

	UINT32			data[34];
	m68ki_cpu_core	Ctx;
		
	StateFile->Read(data, sizeof(data));
	
	// These must be set first, to ensure another contexts' IRQs aren't active when PC is changed 
	m68k_get_context(&Ctx);
	Ctx.int_level = data[0];
	Ctx.int_cycles = data[1];
	Ctx.stopped = data[2];
	m68k_set_context(&Ctx);	// write them back to context
	m68k_set_reg(M68K_REG_D0, data[3]);
	m68k_set_reg(M68K_REG_D1, data[4]);
	m68k_set_reg(M68K_REG_D2, data[5]);
	m68k_set_reg(M68K_REG_D3, data[6]);
	m68k_set_reg(M68K_REG_D4, data[7]);
	m68k_set_reg(M68K_REG_D5, data[8]);
	m68k_set_reg(M68K_REG_D6, data[9]);
	m68k_set_reg(M68K_REG_D7, data[10]);
	m68k_set_reg(M68K_REG_A0, data[11]);
	m68k_set_reg(M68K_REG_A1, data[12]);
	m68k_set_reg(M68K_REG_A2, data[13]);
	m68k_set_reg(M68K_REG_A3, data[14]);
	m68k_set_reg(M68K_REG_A4, data[15]);
	m68k_set_reg(M68K_REG_A5, data[16]);
	m68k_set_reg(M68K_REG_A6, data[17]);
	m68k_set_reg(M68K_REG_A7, data[18]);
	m68k_set_reg(M68K_REG_PC, data[19]);
	m68k_set_reg(M68K_REG_SR, data[20]);
	m68k_set_reg(M68K_REG_SP, data[21]);
	m68k_set_reg(M68K_REG_USP, data[22]);
	m68k_set_reg(M68K_REG_ISP, data[23]);
	m68k_set_reg(M68K_REG_MSP, data[24]);
	m68k_set_reg(M68K_REG_SFC, data[25]);
	m68k_set_reg(M68K_REG_DFC, data[26]);
	m68k_set_reg(M68K_REG_VBR, data[27]);
	m68k_set_reg(M68K_REG_CACR, data[28]);
	m68k_set_reg(M68K_REG_CAAR, data[29]);
	m68k_set_reg(M68K_REG_PREF_ADDR, data[30]);
	m68k_set_reg(M68K_REG_PREF_DATA, data[31]);
	m68k_set_reg(M68K_REG_PPC, data[32]);
	m68k_set_reg(M68K_REG_IR, data[33]);
}

// Emulation functions

void M68KSetIRQ(int irqLevel)
{
	m68k_set_irq(irqLevel);
}

int M68KRun(int numCycles)
{
#ifdef SUPERMODEL_DEBUGGER
	if (Debug != NULL)
	{
		Debug->CPUActive();
		lastCycles += numCycles;
	}
#endif // SUPERMODEL_DEBUGGER
	int doneCycles = m68k_execute(numCycles);
#ifdef SUPERMODEL_DEBUGGER
	if (Debug != NULL)
	{
		Debug->CPUInactive();
		lastCycles -= m68k_cycles_remaining();
	}
#endif // SUPERMODEL_DEBUGGER
	return doneCycles;
}

void M68KReset(void)
{
	m68k_pulse_reset();
#ifdef SUPERMODEL_DEBUGGER
	lastCycles = 0;
#endif
	DebugLog("68K reset\n");
}

// Callback setup

void M68KSetIRQCallback(int (*F)(int nIRQ))
{
	IRQAck = F;
}

void M68KAttachBus(CBus *BusPtr)
{
	Bus = BusPtr;
	DebugLog("Attached bus to 68K\n");
}

// Context switching

void M68KGetContext(M68KCtx *Dest)
{
	Dest->IRQAck = IRQAck;
	Dest->Bus = Bus;
#ifdef SUPERMODEL_DEBUGGER
	Dest->Debug = Debug;
#endif // SUPERMODEL_DEBUGGER
	m68k_get_context(&(Dest->musashiCtx));
}

void M68KSetContext(M68KCtx *Src)
{
	IRQAck = Src->IRQAck;
	Bus = Src->Bus;
#ifdef SUPERMODEL_DEBUGGER
	Debug = Src->Debug;
#endif // SUPERMODEL_DEBUGGER
	m68k_set_context(&(Src->musashiCtx));
}

// One-time initialization

bool M68KInit(void)
{
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	m68k_set_int_ack_callback(M68KIRQCallback);
	Bus = NULL;
#ifdef SUPERMODEL_DEBUGGER
	Debug = NULL;
#endif // SUPERMODEL_DEBUGGER
	DebugLog("Initialized 68K\n");
	return OKAY;
}

#ifdef SUPERMODEL_DEBUGGER
UINT32 M68KGetRegister(M68KCtx *Src, unsigned reg)
{
	switch (reg)
	{
		case DBG68K_REG_PC: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_PC);
		case DBG68K_REG_SR: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_SR);
		case DBG68K_REG_SP: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_SP);
		case DBG68K_REG_D0: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_D0);
		case DBG68K_REG_D1: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_D1);
		case DBG68K_REG_D2: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_D2);
		case DBG68K_REG_D3: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_D3);
		case DBG68K_REG_D4: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_D4);
		case DBG68K_REG_D5: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_D5);
		case DBG68K_REG_D6: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_D6);
		case DBG68K_REG_D7: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_D7);
		case DBG68K_REG_A0: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_A0);
		case DBG68K_REG_A1: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_A1);
		case DBG68K_REG_A2: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_A2);
		case DBG68K_REG_A3: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_A3);
		case DBG68K_REG_A4: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_A4);
		case DBG68K_REG_A5: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_A5);
		case DBG68K_REG_A6: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_A6);
		case DBG68K_REG_A7: return m68k_get_reg(&(Src->musashiCtx), M68K_REG_A7);
		default:            return 0;
	}
}

UINT32 M68KSetRegister(M68KCtx *Src, unsigned reg, UINT32 val)
{
	switch (reg)
	{
		case DBG68K_REG_PC: m68k_set_reg(M68K_REG_PC, val); return true;
		case DBG68K_REG_SR: m68k_set_reg(M68K_REG_SR, val); return true;
		case DBG68K_REG_SP: m68k_set_reg(M68K_REG_SP, val); return true;
		case DBG68K_REG_D0: m68k_set_reg(M68K_REG_D0, val); return true;
		case DBG68K_REG_D1: m68k_set_reg(M68K_REG_D1, val); return true;
		case DBG68K_REG_D2: m68k_set_reg(M68K_REG_D2, val); return true;
		case DBG68K_REG_D3: m68k_set_reg(M68K_REG_D3, val); return true;
		case DBG68K_REG_D4: m68k_set_reg(M68K_REG_D4, val); return true;
		case DBG68K_REG_D5: m68k_set_reg(M68K_REG_D5, val); return true;
		case DBG68K_REG_D6: m68k_set_reg(M68K_REG_D6, val); return true;
		case DBG68K_REG_D7: m68k_set_reg(M68K_REG_D7, val); return true;
		case DBG68K_REG_A0: m68k_set_reg(M68K_REG_A0, val); return true;
		case DBG68K_REG_A1: m68k_set_reg(M68K_REG_A1, val); return true;
		case DBG68K_REG_A2: m68k_set_reg(M68K_REG_A2, val); return true;
		case DBG68K_REG_A3: m68k_set_reg(M68K_REG_A3, val); return true;
		case DBG68K_REG_A4: m68k_set_reg(M68K_REG_A4, val); return true;
		case DBG68K_REG_A5: m68k_set_reg(M68K_REG_A5, val); return true;
		case DBG68K_REG_A6: m68k_set_reg(M68K_REG_A6, val); return true;
		case DBG68K_REG_A7: m68k_set_reg(M68K_REG_A7, val); return true;
		default:            return false;
	}
}
#endif // SUPERMODEL_DEBUGGER

/******************************************************************************
 Musashi 68K Handlers
 
 Musashi/m68kconf.h has been configured to call these directly.
******************************************************************************/

extern "C" {

#ifdef SUPERMODEL_DEBUGGER
void M68KDebugCallback()
{
	if (Debug != NULL)
	{
		UINT32 pc = m68k_get_reg(NULL, M68K_REG_PC);
		UINT32 opcode = Bus->Read16(pc);
		Debug->CPUExecute(pc, opcode, lastCycles - m68k_cycles_remaining());
		lastCycles = m68k_cycles_remaining();
	}
}
#endif // SUPERMODEL_DEBUGGER

int M68KIRQCallback(int nIRQ)
{
#ifdef SUPERMODEL_DEBUGGER
	if (Debug != NULL)
	{
		Debug->CPUException(25);
		Debug->CPUInterrupt(nIRQ - 1);
	}
#endif // SUPERMODEL_DEBUGGER
	if (NULL == IRQAck)	// no handler, use default behavior
	{
		m68k_set_irq(0);	// clear line
		return M68K_IRQ_AUTOVECTOR;
	}
	else
		return IRQAck(nIRQ);
}

unsigned int FASTCALL M68KFetch8(unsigned int a)
{
	return Bus->Read8(a);
}

unsigned int FASTCALL M68KFetch16(unsigned int a)
{
	return Bus->Read16(a);
}

unsigned int FASTCALL M68KFetch32(unsigned int a)
{
	return Bus->Read32(a);
}

unsigned int FASTCALL M68KRead8(unsigned int a)
{
	return Bus->Read8(a);
}

unsigned int FASTCALL M68KRead16(unsigned int a)
{
	return Bus->Read16(a);
}

unsigned int FASTCALL M68KRead32(unsigned int a)
{
	return Bus->Read32(a);
}

void FASTCALL M68KWrite8(unsigned int a, unsigned int d)
{
	Bus->Write8(a, d);
}

void FASTCALL M68KWrite16(unsigned int a, unsigned int d)
{
	Bus->Write16(a, d);
}

void FASTCALL M68KWrite32(unsigned int a, unsigned int d)
{
	Bus->Write32(a, d);
}

}	// extern "C"
