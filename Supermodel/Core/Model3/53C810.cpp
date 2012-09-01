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
 * 53C810.cpp
 * 
 * Implementation of the C53C810 class: NCR 53C810 SCSI controller.
 *
 * TO-DO List:
 * -----------
 * - VF3 does something weird: it writes DSP (triggering automatic code
 *   execution because MAN=0) and THEN sets single step mode, expecting an
 *   interrupt to occur. I suspect this is incorrect operation and that
 *   the SCRIPTS processor either enters single-step mode while the memory
 *   transfer is underway (though it is unlikely because it's such a short
 *   transfer), or that single step can occur even when the device is "halted"
 *	 (which would mean the SCRIPTS processor executes an invalid instruction
 *   once or twice without the VF3 SCSI driver noticing). Unfortunately, the
 *   former is not feasible to emulate. If automatic SCRIPTS execution is
 *	 disabled when single-stepping is enabled, Scud Race breaks (glitchy,
 *	 jerky graphics). Enabling automatic execution and also allowing single
 *	 stepping to occur when the processor is halted seems to work, but it 
 *	 causes invalid instructions to be hit each time.
 * - Check to ensure the invalid instructions hit above would never be decoded
 *	 as real SCRIPTS instructions (in some cases, appears they can  be...)
 * - Is the SIP bit supposed to be set after single stepping?
 * - pg 2-22 (42) of the manual has description of how to clear interrupts.
 * - Another way to fix VF3 is to just set the single-step and DMA interrupt
 *   flags at all times.
 *
 */

#include <cstring>
#include "Supermodel.h"


/******************************************************************************
 Save States
******************************************************************************/

void C53C810::SaveState(CBlockFile *SaveState)
{
	SaveState->NewBlock("53C810", __FILE__);
	SaveState->Write(Ctx.regs, sizeof(Ctx.regs));
	SaveState->Write(&Ctx.regTEMP, sizeof(Ctx.regTEMP));
	SaveState->Write(&Ctx.regDSP, sizeof(Ctx.regDSP));
	SaveState->Write(&Ctx.regDSPS, sizeof(Ctx.regDSPS));
	SaveState->Write(&Ctx.regDBC, sizeof(Ctx.regDBC));
	SaveState->Write(&Ctx.regDCMD, sizeof(Ctx.regDCMD));
	SaveState->Write(&Ctx.regDCNTL, sizeof(Ctx.regDCNTL));
	SaveState->Write(&Ctx.regDMODE, sizeof(Ctx.regDMODE));
	SaveState->Write(&Ctx.regDSTAT, sizeof(Ctx.regDSTAT));
	SaveState->Write(&Ctx.regISTAT, sizeof(Ctx.regISTAT));
}

void C53C810::LoadState(CBlockFile *SaveState)
{
	if (OKAY != SaveState->FindBlock("53C810"))
	{
		ErrorLog("Unable to load 53C810 state. Save state file is corrupt.");
		return;
	}
	
	SaveState->Read(Ctx.regs, sizeof(Ctx.regs));
	SaveState->Read(&Ctx.regTEMP, sizeof(Ctx.regTEMP));
	SaveState->Read(&Ctx.regDSP, sizeof(Ctx.regDSP));
	SaveState->Read(&Ctx.regDSPS, sizeof(Ctx.regDSPS));
	SaveState->Read(&Ctx.regDBC, sizeof(Ctx.regDBC));
	SaveState->Read(&Ctx.regDCMD, sizeof(Ctx.regDCMD));
	SaveState->Read(&Ctx.regDCNTL, sizeof(Ctx.regDCNTL));
	SaveState->Read(&Ctx.regDMODE, sizeof(Ctx.regDMODE));
	SaveState->Read(&Ctx.regDSTAT, sizeof(Ctx.regDSTAT));
	SaveState->Read(&Ctx.regISTAT, sizeof(Ctx.regISTAT));
}


/******************************************************************************
 SCRIPTS Emulation
******************************************************************************/

static inline UINT32 Fetch(struct NCR53C810Context *Ctx)
{
	UINT32 data = Ctx->Bus->Read32(Ctx->regDSP);
	Ctx->regDSP += 4;
	return FLIPENDIAN32(data);	// remember: bus is big endian, need to convert to little endian
}

//TO-DO: check if this ever occurs in single-step mode (if so, we would need to stack interrupts)
static bool SCRIPTS_Int_IntFly(struct NCR53C810Context *Ctx)
{
	Ctx->halt = true;	// halt SCRIPTS execution
	Ctx->regISTAT |= 1;	// DMA interrupt pending
	Ctx->regDSTAT |= 4;	// SCRIPTS interrupt instruction received
	Ctx->IRQ->Assert(Ctx->scsiIRQ);

	if ((Ctx->regDBC&0x100000))	// INTFLY
		return ErrorLog("53C810 INTFLY instruction not emulated!");
	return OKAY;
}

static bool SCRIPTS_MoveMemory(struct NCR53C810Context *Ctx)
{
	UINT32		src, dest;
    unsigned	numBytes, i;

	// Get operands
	src = Ctx->regDSPS;
    dest = Ctx->regTEMP = Fetch(Ctx);
    numBytes = Ctx->regDBC;
    // not implemented: illegal instruction interrupt when src and dest are not aligned the same way
    
    DebugLog("53C810: Move Memory %08X -> %08X, %X\n", src, dest, numBytes);
    
    // Perform a 32-bit copy if possible
    for (i = 0; i < (numBytes/4); i++)
    {
        Ctx->Bus->Write32(dest, Ctx->Bus->Read32(src));
        dest += 4;
        src += 4;
    }

    // Finish off the last few odd bytes
    numBytes &= 3;
    while (numBytes)
   	{
		Ctx->Bus->Write8(dest++, Ctx->Bus->Read8(src++));
		--numBytes;
    }

    // Update registers
    Ctx->regDBC = 0;
    Ctx->regDSPS = src;
    Ctx->regTEMP = dest;

    return OKAY;
}

// Invalid instruction handler
static bool SCRIPTS_Invalid(struct NCR53C810Context *Ctx)
{
	DebugLog("53C810 encountered an unrecognized instruction (%02X%06X, DSP=%08X)\n!", Ctx->regDCMD, Ctx->regDBC, Ctx->regDSP);
	return FAIL;
}

void C53C810::Run(bool singleStep)
{
	UINT32	op;
	int		i;
	
	if (singleStep)// && !Ctx.halt)
	{
		// Fetch instruction (first two words are always fetched)
		op = Fetch(&Ctx);			// word 1
		Ctx.regDBC = op&0x00FFFFFF;
		Ctx.regDCMD = (op>>24)&0xFF;
		Ctx.regDSPS = Fetch(&Ctx);	// word 2
		
		// Single step
		OpTable[Ctx.regDCMD](&Ctx);
		
		// Issue IRQ and finish
		Ctx.regISTAT |= 3;				// DMA interrupt pending (NOTE: should SIP be set? I don't think so...)
		Ctx.regDSTAT |= 8;				// single step interrupt
		Ctx.IRQ->Assert(Ctx.scsiIRQ);	// generate an interrupt
		DebugLog("53C810: Asserted IRQ\n");
	}
	else
	{
		// Automatic mode: run (as long as the processor is not halted)
		for (i = 0; (i < 100) && !Ctx.halt; i++)
		{
			// Fetch instruction (first two words are always fetched)
			op = Fetch(&Ctx);			// word 1
			Ctx.regDBC = op&0x00FFFFFF;
			Ctx.regDCMD = (op>>24)&0xFF;
			Ctx.regDSPS = Fetch(&Ctx);	// word 2
		
			// Execute!
			if (OpTable[Ctx.regDCMD](&Ctx) != OKAY)
				break;
		}
	}
}

// Insert instructions into the LUT under control of the mask
void C53C810::Insert(UINT8 mask, UINT8 op, bool (*Handler)(struct NCR53C810Context *))
{
    UINT32  i;

    for (i = 0; i < 256; i++)
    {
        if ((i&mask) == op)
            OpTable[i] = Handler;
    }
}

void C53C810::BuildOpTable(void)
{
	Insert(0, 0, &SCRIPTS_Invalid);
	Insert(0xE0, 0xC0, &SCRIPTS_MoveMemory);
	Insert(0xF8, 0x98, &SCRIPTS_Int_IntFly);
}


/******************************************************************************
 Register and PCI Access Handlers
******************************************************************************/

void C53C810::WriteRegister(unsigned reg, UINT8 data)
{
	if (reg >= 0x60)
	{
		ErrorLog("Write to invalid 53C810 register (%02X).", reg);
		return;
	}
	
	DebugLog("53C810 write: %02X=%02X (PC=%08X, LR=%08X)\n", reg, data, ppc_get_pc(), ppc_get_lr());
	
	// Dump everything into the register file
	Ctx.regs[reg&0xFF] = data;
	
	// Do something extra with the ones that we actually care about
	// TO-DO: prevent invalid/reserved/read-only registers from being written?
	switch(reg)
	{
	case 0x14:		// ISTAT
		Ctx.regISTAT = data;
		DebugLog("ISTAT=%02X\n", data);
		break;
	case 0x1C:		// TEMP 7-0
		Ctx.regTEMP &= 0xFFFFFF00;
		Ctx.regTEMP |= data;
		break;
	case 0x1D:		// TEMP 15-8
		Ctx.regTEMP &= 0xFFFF00FF;
		Ctx.regTEMP |= (data<<8);
		break;
	case 0x1E:		// TEMP 23-16
		Ctx.regTEMP &= 0xFF00FFFF;
		Ctx.regTEMP |= (data<<16);
		break;
	case 0x1F:		// TEMP 31-24
		Ctx.regTEMP &= 0x00FFFFFF;
		Ctx.regTEMP |= (data<<24);
		break;
	case 0x24:		// DBC 7-0
		Ctx.regDBC &= 0xFFFFFF00;
		Ctx.regDBC |= data;
		break;
	case 0x25:		// DBC 15-8
		Ctx.regDBC &= 0xFFFF00FF;
		Ctx.regDBC |= (data<<8);
		break;
	case 0x26:		// DBC 23-16
		Ctx.regDBC &= 0xFF00FFFF;
		Ctx.regDBC |= (data<<16);
		break;
	case 0x27:		// DCMD
		Ctx.regDCMD = data;
		break;	
	case 0x2C:		// DSP 7-0
		Ctx.regDSP &= 0xFFFFFF00;
		Ctx.regDSP |= data;
		break;
	case 0x2D:		// DSP 15-8
		Ctx.regDSP &= 0xFFFF00FF;
		Ctx.regDSP |= (data<<8);
		break;
	case 0x2E:		// DSP 23-16
		Ctx.regDSP &= 0xFF00FFFF;
		Ctx.regDSP |= (data<<16);
		break;
	case 0x2F:		// DSP 31-24
		Ctx.regDSP &= 0x00FFFFFF;
		Ctx.regDSP |= (data<<24);
		Ctx.halt = false;	// writing this register un-halts 53C810 operation (pg.6-31 of LSI manual)
		if (!(Ctx.regDMODE&1))	// if MAN=0, start SCRIPTS automatically
		// To-Do: is this correct? Should single step really be tested first?
		//if (!(Ctx.regDCNTL&0x10) && !(Ctx.regDMODE&1))	// if MAN=0 and not single stepping, start SCRIPTS automatically
		{
			DebugLog("53C810: Automatically starting (PC=%08X, LR=%08X, single step=%d)\n", ppc_get_pc(), ppc_get_lr(), !!(Ctx.regDCNTL&0x10));
			Run(false);				// automatic
		}
		break;	
	case 0x30:		// DSPS 7-0
		Ctx.regDSPS &= 0xFFFFFF00;
		Ctx.regDSPS |= data;
		break;
	case 0x31:		// DSPS 15-8
		Ctx.regDSPS &= 0xFFFF00FF;
		Ctx.regDSPS |= (data<<8);
		break;
	case 0x32:		// DSPS 23-16
		Ctx.regDSPS &= 0xFF00FFFF;
		Ctx.regDSPS |= (data<<16);
		break;
	case 0x33:		// DSPS 31-24
		Ctx.regDSPS &= 0x00FFFFFF;
		Ctx.regDSPS |= (data<<24);
		break;
	case 0x38:		// DMODE
		Ctx.regDMODE = data;
		break;
	case 0x3B:		// DCNTL
		Ctx.regDCNTL = data;
		if ((Ctx.regDCNTL&0x14) == 0x14)		// single step
		{
			DebugLog("53C810: single step: %08X, (halt=%d)\n", Ctx.regDSP, Ctx.halt);
			Run(true);
		}
		else if ((Ctx.regDCNTL&0x04))			// start DMA bit
		{
			DebugLog("53C810: Manually starting\n");
			Run(false);
		}
		break;
	default:
		break;
	}
}	

UINT8 C53C810::ReadRegister(unsigned reg)
{
	UINT8	ret;
	
	if (reg >= 0x60)
	{
		ErrorLog("Read from invalid 53C810 register (%02X).", reg);
		return 0;
	}
	
	DebugLog("53C810 read: %02X (PC=%08X, LR=%08X)\n", reg, ppc_get_pc(), ppc_get_lr());
	
	// Some registers require special handling
	switch(reg)
	{
	case 0x0C:		// DSTAT
		// For now, we don't generate stacked interrupts, so always clear IRQ status
		ret = Ctx.regDSTAT;
		//TO-DO: manual says these should be cleared here but MAME never clears them. What's up with that?
		Ctx.regISTAT &= 0xFE;	// clear DIP bit (DMA interrupt)
		Ctx.regDSTAT &= 0xF7;	// clear SSI (single step interrupt)
		//Ctx.regISTAT |= 1;	// doing this is another way to fix VF3
		//Ctx.regDSTAT |= 8;
		Ctx.IRQ->Deassert(Ctx.scsiIRQ);
		//DebugLog("53C810: DSTAT read\n");
		return ret;
	case 0x14:		// ISTAT
		//DebugLog("53C810: ISTAT read\n");
		return Ctx.regISTAT;
	case 0x1C:		// TEMP 7-0
		return Ctx.regTEMP&0xFF;
	case 0x1D:		// TEMP 15-8
		return (Ctx.regTEMP>>8)&0xFF;
	case 0x1E:		// TEMP 23-16
		return (Ctx.regTEMP>>16)&0xFF;
	case 0x1F:		// TEMP 31-24
		return (Ctx.regTEMP>>24)&0xFF;
	case 0x24:		// DBC 7-0
		return Ctx.regDBC&0xFF;
	case 0x25:		// DBC 15-8
		return (Ctx.regDBC>>8)&0xFF;
	case 0x26:		// DBC 23-16
		return (Ctx.regDBC>>16)&0xFF;
	case 0x27:		// DCMD
		return Ctx.regDCMD;
	case 0x2C:		// DSP 7-0
		return Ctx.regDSP&0xFF;
	case 0x2D:		// DSP 15-8
		return (Ctx.regDSP>>8)&0xFF;
	case 0x2E:		// DSP 23-16
		return (Ctx.regDSP>>16)&0xFF;
	case 0x2F:		// DSP 31-24
		return (Ctx.regDSP>>24)&0xFF;
	case 0x30:		// DSPS 7-0
		return Ctx.regDSPS&0xFF;
	case 0x31:		// DSPS 15-8
		return (Ctx.regDSPS>>8)&0xFF;
	case 0x32:		// DSPS 23-16
		return (Ctx.regDSPS>>16)&0xFF;
	case 0x33:		// DSPS 31-24
		return (Ctx.regDSPS>>24)&0xFF;
	case 0x38:
		return Ctx.regDMODE;
	case 0x3B:		// DCNTL
		return Ctx.regDCNTL;
	default:	// get it from the register file
		break;
	}
	
	// Register file should be up to date
	return Ctx.regs[reg&0xFF];
}	

UINT32 C53C810::ReadPCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset)
{
	UINT32	d;
	
	if ((bits==8))
	{
		DebugLog("53C810 %d-bit PCI read request for reg=%02X\n", bits, reg);
		return 0;
	}
	
	switch (reg)
	{
	case 0x00:	// Device ID and Vendor ID
		d = 0x00011000;	// 0x1000 = LSI Logic
		switch (bits)
		{
		case 8:
			d >>= (3-offset)*8;	// offset will be 0-3; select appropriate byte
			d &= 0xFF;
			break;
		case 16:
			d >>= (2-offset)*8;	// offset will be 0 or 2 only; select either high or low word
			d &= 0xFFFF;
			break;
		default:
			break;
		}
		return d;
	default:
		DebugLog("53C810 PCI read request for reg=%02X (%d-bit)\n", reg, bits);
		break;
	}

	return 0;
}
	
void C53C810::WritePCIConfigSpace(unsigned device, unsigned reg, unsigned bits, unsigned offset, UINT32 data)
{
	DebugLog("53C810 PCI %d-bit write request for reg=%02X, data=%08X\n", bits, reg, data);
}

void C53C810::Reset(void)
{
	memset(Ctx.regs, 0, sizeof(Ctx.regs));
	Ctx.regs[0x00] = 0xC0;	// SCNTL0
	Ctx.regs[0x0C] = 0x80;	// DSTAT
	Ctx.regs[0x0F] = 0x02;	// SSTAT2
	Ctx.regs[0x18] = 0xFF;	// reserved
	Ctx.regs[0x19] = 0xF0;	// CTEST1
	Ctx.regs[0x1A] = 0x01;	// CTEST2
	Ctx.regs[0x46] = 0x60;	// MACNTL
	Ctx.regs[0x47] = 0x0F;	// GPCNTL
	Ctx.regs[0x4C] = 0x03;	// STEST0	
	Ctx.regTEMP = 0;
	Ctx.regDSP = 0;
	Ctx.regDSPS = 0;
	Ctx.regDBC = 0;
	Ctx.regDCMD = 0;
	Ctx.regDCNTL = 0;
	Ctx.regDMODE = 0;
	Ctx.regDSTAT = 0x80;	// DMA FIFO empty
	Ctx.regISTAT = 0;
	Ctx.halt = false;
	
	DebugLog("53C810 reset\n");
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

void C53C810::Init(CBus *BusObjectPtr, CIRQ *IRQObjectPtr, unsigned scsiIRQBit)
{
	Ctx.Bus = BusObjectPtr; 
	Ctx.IRQ = IRQObjectPtr;
	Ctx.scsiIRQ = scsiIRQBit;
}

C53C810::C53C810(void)
{	
	BuildOpTable();
	Ctx.Bus = NULL;
	Ctx.IRQ = NULL;
	scsiIRQ = 0;
	DebugLog("Built 53C810\n");
}

C53C810::~C53C810(void)
{	
	Ctx.Bus = NULL;
	Ctx.IRQ = NULL;
	DebugLog("Destroyed 53C810\n");
}
