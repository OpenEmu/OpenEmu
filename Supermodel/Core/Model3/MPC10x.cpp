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
 * MPC10x.cpp
 * 
 * Implementation of the CMPC10x class: Motorola MPC105 and MPC106 PCI/bridge
 * controllers. For now, only a single PCI bus can be attached and all requests
 * are forwarded to it.
 *
 * This is a minimal implementation designed with the Model 3 in mind. It does
 * not properly emulate the device and there are numerous inaccuracies in this 
 * code. 
 *
 * Important Problems To Be Aware Of
 * ---------------------------------
 * 
 * The handling of accesses smaller than 32 bits is funky and probably very 
 * incorrect, particularly for non-32-bit-aligned register numbers. This
 * problem extends to the entire PCI emulation.
 *
 * Little endian mode is not supported (the internal registers, however, seem 
 * to be stored in little endian format). The registers are implemented as byte
 * read/writeable but this is not accurate. In fact, some are read only, and 
 * some can only be accessed as 16-bit or 32-bit words.
 *
 * This code assumes we are running on a little endian machine and that the
 * PowerPC is a big endian machine. Therefore, bytes can be written to the 
 * register space directly but multi-byte words must be flipped before being
 * interpreted internally.
 *
 * External configuration registers are not implemented.
 *
 * Multiple PCI buses are not yet supported (everything is assumed to be on
 * bus 0).
 *
 * ... And lots more!
 *
 * References
 * ----------
 * 1. "MPC106 PCI/Bridge/Memory Controller User's Manual" (MPC106UM/D Rev.1)
 *
 * To-Do List
 * ----------
 * - The whole PCI system desperately needs a re-write to deal with different
 *	 bit-width accesses in a cleaner manner.
 * - Fix endian confusion. Should assume everything written is in the correct
 *   endian, pushing the responsibility onto the caller.
 */

#include <cstring>
#include "Supermodel.h"


/******************************************************************************
 Save States
******************************************************************************/

void CMPC10x::SaveState(CBlockFile *SaveState)
{
	SaveState->NewBlock("MPC10x", __FILE__);
	SaveState->Write(regs, sizeof(regs));
	SaveState->Write(&pciBus, sizeof(pciBus));
	SaveState->Write(&pciDevice, sizeof(pciDevice));
	SaveState->Write(&pciFunction, sizeof(pciFunction));
	SaveState->Write(&pciReg, sizeof(pciReg));
}

void CMPC10x::LoadState(CBlockFile *SaveState)
{
	if (OKAY != SaveState->FindBlock("MPC10x"))
	{
		ErrorLog("Unable to load MPC%X state. Save state file is corrupt.", model);
		return;
	}
	
	SaveState->Read(regs, sizeof(regs));
	SaveState->Read(&pciBus, sizeof(pciBus));
	SaveState->Read(&pciDevice, sizeof(pciDevice));
	SaveState->Read(&pciFunction, sizeof(pciFunction));
	SaveState->Read(&pciReg, sizeof(pciReg));
}


/******************************************************************************
 Emulation Functions
******************************************************************************/

/*
 * CMPC10x::WritePCIConfigAddress(data):
 *
 * Writes to the PCI configuration space address (CONFIG_ADDR) register, which
 * selects a PCI device and configuration space register. 
 */
void CMPC10x::WritePCIConfigAddress(UINT32 data)
{
	UINT32	d = FLIPENDIAN32(data);
	
	pciBus = (d>>16)&0xFF;
	pciDevice = (d>>11)&0x1F;
	pciFunction = (d>>8)&7;
	pciReg = d&0xFF;	// NOTE: for actual PCI devices (device>0), register must be shifted right by 2 and clamped to 0x3F
	
	// The manual is unclear as to whether device 0 (MPC10x) register #s are clamped to 0x3F or not. Pay attention to this!
#ifdef DEBUG
	if (pciDevice == 0)
	{
		DebugLog("MPC10x: Device 0 configuration access: %08X\n", d);
		if ((d&3))
			ErrorLog("MPC10x: Device 0 configuration address with low bits set: %08X\n", d);
	}
	//DebugLog("MPC10x: Bus=%X, Device=%X, Function=%X, Reg=%X\n", pciBus, pciDevice, pciFunction, pciReg);
#endif
	
	if (pciBus != 0)
	{
		//printf("Multiple PCI buses detected!\n");
		DebugLog("Multiple PCI buses detected!\n");
	}
}

/*
 * CMPC10x::ReadPCIConfigData(bits, offset):
 *
 * Reads from the PCI configuration space data (CONFIG_DATA) register, which in
 * turn calls upon the selected PCI device to return the data.
 */
UINT32 CMPC10x::ReadPCIConfigData(unsigned bits, unsigned offset)
{
	// Handle self-access first
	if (pciDevice == 0)
	{
		// Alignment check
#ifdef DEBUG
		if (((bits==16)&&(offset&1)) || ((bits==32)&&(offset&3)))
			ErrorLog("Misaligned MPC10x read request (bits=%d,reg=%X,offset=%d)\n", bits, pciReg, offset);
#endif
		
		switch (bits)
		{
		case 8:
			return regs[pciReg+offset];
		case 16:
			return (regs[pciReg+offset+0]<<8) | regs[pciReg+offset+1];
		case 32:
			return 	(regs[pciReg+offset+0]<<24) | 
					(regs[pciReg+offset+1]<<16) |
					(regs[pciReg+offset+2]<<8) |
					regs[pciReg+offset+3];
		default:
			ErrorLog("MPC10x internal error: invalid access size (%d-bits)", bits);
			break;
		}
	}
	
	// All other PCI devices passed to PCI bus
	return PCIBus->ReadConfigSpace(pciDevice, (pciReg>>2)&0x3C, bits, offset);
}

/*
 * CMPC10x::WritePCIConfigData(bits, offset, data):
 *
 * Writes to the PCI configuration space data (CONFIG_DATA) register, which in
 * turn passes the data to the selected PCI device.
 */
void CMPC10x::WritePCIConfigData(unsigned bits, unsigned offset, UINT32 data)
{
	// Handle self-access first
	if (pciDevice == 0)
	{
		// Alignment check
#ifdef DEBUG
		if (((bits==16)&&(offset&1)) || ((bits==32)&&(offset&3)))
			ErrorLog("Misaligned MPC10x read request (bits=%d,reg=%X,offset=%d)\n", bits, pciReg, offset);
#endif
		
		switch (bits)
		{
		case 8:
			regs[pciReg+offset] = data&0xFF;
			break;
		case 16:
			regs[pciReg+offset+0] = (data>>8)&0xFF;
			regs[pciReg+offset+1] = data&0xFF;
			break;
		case 32:
			regs[pciReg+offset+0] = (data>>24)&0xFF;
			regs[pciReg+offset+1] = (data>>16)&0xFF;
			regs[pciReg+offset+2] = (data>>8)&0xFF;
			regs[pciReg+offset+3] = data&0xFF;
			break;
		default:
			ErrorLog("MPC10x internal error: invalid access size (%d-bits)", bits);
			break;
		}
		
		return;
	}
	
	PCIBus->WriteConfigSpace(pciDevice, (pciReg>>2)&0x3C, bits, offset, data);
}

/*
 * CMPC10x::WriteRegister(reg, data):
 *
 * Writes to the MPC10x register space. Accesses one byte at a time so it
 * should be endian-neutral (the caller ends up being responsible for this).
 */
void CMPC10x::WriteRegister(unsigned reg, UINT8 data)
{
	regs[reg&0xFF] = data;
	
	if ((reg&0xFF)==0xA8)
	{
		if ((data&0x20))
			ErrorLog("MPC10x little endian mode not yet implemented!");
	}
}

/*
 * CMPC10x::Reset(void):
 *
 * Resets the device.
 */
void CMPC10x::Reset(void)
{
	memset(regs, 0, sizeof(regs));
	
	// Data is actually stored in little endian format, so we can write directly here
	*(UINT16 *) &regs[0x00] = 0x1057;		// vendor ID (Motorola)
	*(UINT16 *) &regs[0x02] = (model==0x106)?0x0002:0x0001;	// device ID (MPC105 or MPC106)
	
	if (model == 0x106)	// MPC106
	{
		*(UINT32 *) &regs[0x04] = 0x00800006;	// PCI command and PCI status
		*(UINT32 *) &regs[0x08] = 0x00060000;	// class code and revision ID
		*(UINT32 *) &regs[0x0C] = 0x00000800;	// cache line size
		*(UINT32 *) &regs[0x70] = 0x00CD0000;	// output driver control
		*(UINT32 *) &regs[0xA8] = 0x0010FF00;	// processor interface config. 1
		*(UINT32 *) &regs[0xAC] = 0x060C000C;	// processor interface config. 2
		*(UINT32 *) &regs[0xB8] = 0x04000000;	// TO-DO: CHECK MANUAL
		*(UINT32 *) &regs[0xC0] = 0x00000100;	// error enabling 1
		*(UINT32 *) &regs[0xE0] = 0x00420FFF;	// emulation support configuration 1
		*(UINT32 *) &regs[0xE8] = 0x00200000;	// emulation support configuration 2
		*(UINT32 *) &regs[0xF0] = 0x0000FF02;	// memory control config. 1
		*(UINT32 *) &regs[0xF4] = 0x00030000;	// memory control config. 2
		*(UINT32 *) &regs[0xFC] = 0x00000010;	// memory control config. 4
	}
	else				// MPC105
	{
		*(UINT32 *) &regs[0x04] = 0x00800006;	// PCI command and PCI status
		*(UINT32 *) &regs[0x08] = 0x00060000;	// class code and revision ID
		*(UINT32 *) &regs[0xA8] = 0x0010FF00;	// processor interface config. 1
		*(UINT32 *) &regs[0xAC] = 0x060C000C;	// processor interface config. 2
		*(UINT32 *) &regs[0xB8] = 0x04000000;	// TO-DO: CHECK MANUAL
		*(UINT32 *) &regs[0xF0] = 0x0000FF02;	// memory control config. 1
		*(UINT32 *) &regs[0xF4] = 0x00030000;	// memory control config. 2
		*(UINT32 *) &regs[0xFC] = 0x00000010;	// memory control config. 4
		// To-do: any more??
	}

	pciBus = 0;
	pciDevice = 0;
	pciFunction = 0;
	pciReg = 0;
	
	DebugLog("MPC%X reset\n", model);
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

/*
 * CMPC10x:AttachPCIBus(BusObjectPtr):
 *
 * Attaches a PCI bus object which will handle all PCI register requests.
 */
void CMPC10x::AttachPCIBus(CPCIBus *BusObjectPtr)
{
	PCIBus = BusObjectPtr;
	DebugLog("MPC10x connected to a PCI bus\n");
}

/*
 * CMPC10x::SetModel(modelNum):
 *
 * Sets the device behavior to either MPC105 or MPC106.
 */
void CMPC10x::SetModel(int modelNum)
{
	model = modelNum;
	
	if ((modelNum!=0x105) && (modelNum!=0x106))
	{
		ErrorLog("%s:%d: Invalid MPC10x model number (%X).", __FILE__, __LINE__, modelNum);
		model = 0x105;
	}
	
	DebugLog("MPC10x set to MPC%X\n", model);
}

/*
 * CMPC10x::Init():
 *
 * This must be called first and only once during the lifetime of the class.
 */
void CMPC10x::Init(void)
{
	// this function really only exists for consistency with other device classes
}

/*
 * CMPC10x::CMPC10x(void):
 *
 * Constructor.
 */
CMPC10x::CMPC10x(void)
{	
	PCIBus = NULL;
	model = 0x105;	// default to MPC105
	pciBus = 0;
	pciDevice = 0;
	pciFunction = 0;
	pciReg = 0;
	DebugLog("Built MPC10x\n");
}

/*
 * CMPC10x::~CMPC10x(void):
 *
 * Destructor.
 */
CMPC10x::~CMPC10x(void)
{
	PCIBus = NULL;
	DebugLog("Destroyed MPC10x\n");
}
