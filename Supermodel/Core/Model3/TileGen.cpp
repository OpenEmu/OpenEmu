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
 * TileGen.cpp
 * 
 * Implementation of the CTileGen class: 2D tile generator.
 *
 * TO-DO List:
 * -----------
 * - For consistency, the registers should probably be byte reversed (this is a
 *   little endian device), forcing the Model3 Read32/Write32 handlers to
 *   manually reverse the data. This keeps with the convention for VRAM.
 */

#include <cstring>
#include "Supermodel.h"


/******************************************************************************
 Save States
******************************************************************************/

void CTileGen::SaveState(CBlockFile *SaveState)
{
	SaveState->NewBlock("Tile Generator", __FILE__);
	SaveState->Write(memoryPool, 0x100000+0x20000);
	SaveState->Write(regs, sizeof(regs));
}

void CTileGen::LoadState(CBlockFile *SaveState)
{
	if (OKAY != SaveState->FindBlock("Tile Generator"))
	{
		ErrorLog("Unable to load tile generator state. Save state file is corrupt.");
		return;
	}
	
	// Load memory one word at a time
	for (int i = 0; i < (0x100000+0x20000); i += 4)
	{
		UINT32 data;
		
		SaveState->Read(&data, sizeof(data));
		Render2D->WriteVRAM(i, data);
		*(UINT32 *) &memoryPool[i] = data;
	}
	
	SaveState->Read(regs, sizeof(regs));
}


/******************************************************************************
 Rendering
******************************************************************************/

void CTileGen::BeginFrame(void)
{
	Render2D->BeginFrame();
/*
	printf("08: %X\n", regs[0x08/4]);
	printf("0C: %X\n", regs[0x0C/4]);
	printf("20: %X\n", regs[0x20/4]);
	printf("40: %X\n", regs[0x40/4]);
	printf("44: %X\n", regs[0x44/4]);
	printf("60: %08X\n", regs[0x60/4]);
	printf("64: %08X\n", regs[0x64/4]);
	printf("68: %08X\n", regs[0x68/4]);
	printf("6C: %08X\n", regs[0x6C/4]);
*/
}

void CTileGen::EndFrame(void)
{
	Render2D->EndFrame();
}


/******************************************************************************
 Emulation Functions
******************************************************************************/

UINT32 CTileGen::ReadRAM(unsigned addr)
{
	return *(UINT32 *) &memoryPool[addr];
}

void CTileGen::WriteRAM(unsigned addr, UINT32 data)
{
	Render2D->WriteVRAM(addr,data);	// inform renderer of update first
	*(UINT32 *) &memoryPool[addr] = data;
}

void CTileGen::WriteRegister(unsigned reg, UINT32 data)
{
	reg &= 0xFF;
	regs[reg/4] = data;
	
	switch (reg)
	{
	case 0x10:	// IRQ acknowledge
		IRQ->Deassert(data&0xFF);
		break;
	case 0x60:
		break;
	case 0x64:
		break;
	case 0x68:
		break;
	case 0x6C:
		break;
	default:
		DebugLog("Tile Generator reg %02X = %08X\n", reg, data);
		//printf("%02X = %08X\n", reg, data);
		break;
	}
}

void CTileGen::Reset(void)
{
	memset(regs, 0, sizeof(regs));
	memset(memoryPool, 0, 0x120000);
	DebugLog("Tile Generator reset\n");
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

void CTileGen::AttachRenderer(CRender2D *Render2DPtr)
{
	Render2D = Render2DPtr;
	Render2D->AttachVRAM(memoryPool);
	Render2D->AttachRegisters(regs);
	DebugLog("Tile Generator attached a Render2D object\n");
}

#define MEMORY_POOL_SIZE	0x120000

bool CTileGen::Init(CIRQ *IRQObjectPtr)
{
	float	memSizeMB = (float)MEMORY_POOL_SIZE/(float)0x100000;
	
	// Allocate all memory for ROMs and PPC RAM
	memoryPool = new(std::nothrow) UINT8[MEMORY_POOL_SIZE];
	if (NULL == memoryPool)
		return ErrorLog("Insufficient memory for tile generator object (needs %1.1f MB).", memSizeMB);
	
	// Hook up the IRQ controller
	IRQ = IRQObjectPtr;
	
	DebugLog("Initialized Tile Generator (allocated %1.1f MB and connected to IRQ controller)\n", memSizeMB);
	return OKAY;
}

CTileGen::CTileGen(void)
{
	IRQ = NULL;
	memoryPool = NULL;
	DebugLog("Built Tile Generator\n");
}

CTileGen::~CTileGen(void)
{
	// Dump tile generator RAM
#if 0
	FILE *fp;
	fp = fopen("tileram", "wb");
	if (NULL != fp)
	{
		fwrite(memoryPool, sizeof(UINT8), 0x120000, fp);
		fclose(fp);
		printf("dumped %s\n", "tileram");
	}
	else
		printf("unable to dump %s\n", "tileram");
#endif
		
	IRQ = NULL;
	if (memoryPool != NULL)
	{
		delete [] memoryPool;
		memoryPool = NULL;
	}
	DebugLog("Destroyed Tile Generator\n");
}
