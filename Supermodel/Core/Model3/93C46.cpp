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
 * 93C46.cpp
 * 
 * Implementation of the 93C46 serial EEPROM device.
 *
 * To-Do List
 * ----------
 * - Manual says that when a READ command is issued, the data will be 
 *   preceded by a leading 0. This seems to cause problems. Perhaps DO
 *   should just be set to 0?
 */

#include <string.h>
#include "Supermodel.h"


/******************************************************************************
 Save States
******************************************************************************/

void C93C46::SaveState(CBlockFile *SaveState)
{
	SaveState->NewBlock("93C46", __FILE__);
	SaveState->Write(regs, sizeof(regs));
	SaveState->Write(&CS, sizeof(CS));
	SaveState->Write(&CLK, sizeof(CLK));
	SaveState->Write(&DI, sizeof(DI));
	SaveState->Write(&DO, sizeof(DO));
	SaveState->Write(&bitBufferOut, sizeof(bitBufferOut));
	SaveState->Write(&bitBufferIn, sizeof(bitBufferIn));
	SaveState->Write(&bitsOut, sizeof(bitsOut));
	SaveState->Write(&receiving, sizeof(receiving));
	SaveState->Write(&addr, sizeof(addr));
	SaveState->Write(&busyCycles, sizeof(busyCycles));
	SaveState->Write(&locked, sizeof(locked));
}

void C93C46::LoadState(CBlockFile *SaveState)
{
	if (OKAY != SaveState->FindBlock("93C46"))
	{
		ErrorLog("Unable to load EEPROM state. File is corrupt.");
		return;
	}
	
	SaveState->Read(regs, sizeof(regs));
	SaveState->Read(&CS, sizeof(CS));
	SaveState->Read(&CLK, sizeof(CLK));
	SaveState->Read(&DI, sizeof(DI));
	SaveState->Read(&DO, sizeof(DO));
	SaveState->Read(&bitBufferOut, sizeof(bitBufferOut));
	SaveState->Read(&bitBufferIn, sizeof(bitBufferIn));
	SaveState->Read(&bitsOut, sizeof(bitsOut));
	SaveState->Read(&receiving, sizeof(receiving));
	SaveState->Read(&addr, sizeof(addr));
	SaveState->Read(&busyCycles, sizeof(busyCycles));
	SaveState->Read(&locked, sizeof(locked));
}


/******************************************************************************
 Emulation Functions
******************************************************************************/

// Reverse the bit ordering
static UINT16 ReverseBits16(UINT16 data)
{
	UINT16	dataOut = 0, hi, lo;
	
	for (int i = 0; i < 16/2; i++)
	{
		// Isolate corresponding bits from high and low halves of word
		lo = (data>>i)&1;
		hi = (data>>(15-i))&1;
		
		// Swap them
		dataOut |= (lo<<(15-i));
		dataOut |= (hi<<i);
	}

	return dataOut;
}

void C93C46::Write(unsigned pinCS, unsigned pinCLK, unsigned pinDI)
{
	unsigned	prevCLK;
	
	//printf("EEPROM: CS=%d CLK=%d DI=%d\n", pinCS, pinCLK, pinDI);
	prevCLK = CLK;
	
	// Save current inputs
	CS = !!pinCS;
	CLK = !!pinCLK;
	DI = !!pinDI;
	
	// Active high CS. When it's brought low, reset control logic.
	if (CS == 0)
	{
		bitBufferIn = 0;	// this must be cleared each time (only leading 0's can exist prior to commands)
		receiving = true;	// ready to accept commands
		busyCycles = 5;		// some applications require the chip to take time while writing
		return;
	}
	
	// Rising clock edge
	if (!prevCLK && CLK)
	{
		if (receiving == true)	// is the chip receiving commands?
		{
			// Shift in a new bit
			bitBufferIn	<<= 1;
			bitBufferIn |= DI;
			
			// Detect commands
			if ((bitBufferIn&0xFFFFFFC0) == 0x180)				// READ
			{
				addr = bitBufferIn&0x3F;
				bitBufferOut = ReverseBits16(regs[addr]);	// reverse so that D15 is shifted out first
				//bitBufferOut <<= 1;	// a leading 0 precedes the first word read (causes problems)
				bitsOut = 0;		// how many bits read out
				receiving = false;	// transmitting data now
				DebugLog("93C46: READ %X\n", addr);
			}
			else if (bitBufferIn == 0x13)						// WEN (write enable)
			{
				locked = false;
				DebugLog("93C46: WEN\n");
			}
			else if (bitBufferIn == 0x10)						// WDS (write disable)
			{
				locked = true;
				DebugLog("93C46: WDS\n");
			}
			else if ((bitBufferIn&0xFFC00000) == 0x01400000)	// WRITE
			{
				if (!locked)
					regs[(bitBufferIn>>16)&0x3F] = bitBufferIn&0xFFFF;
				DO = 1;	// ready (write completed)
				DebugLog("93C46: WRITE %X=%04X (lock=%d)\n", (bitBufferIn>>16)&0x3F, bitBufferIn&0xFFFF, locked);
			}
			else if ((bitBufferIn&0xFFF00000) == 0x01100000)	// WRALL (write all)
			{
				if (!locked)
				{
					for (int i = 0; i < 64; i++)
						regs[i] = bitBufferIn&0xFFFF;
				}
				DO = 1;
				DebugLog("93C46: WRALL %04X (lock=%d)\n", bitBufferIn&0xFFFF, locked);
			}
			else if ((bitBufferIn&0xFFFFFFC0) == 0x1C0)			// ERASE
			{
				if (!locked)
					regs[bitBufferIn&0x3F] = 0xFFFF;
				DO = 1;
				DebugLog("93C46: ERASE %X (lock=%d)\n", bitBufferIn&0x3F, locked);
			}
			else if ((bitBufferIn&0xFFFFFFF0) == 0x120)			// ERALL (erase all)
			{
				if (!locked)
				{
					for (int i = 0; i < 64; i++)
						regs[i] = 0xFFFF;
					DebugLog("93C46: ERALL (lock=%d)\n", locked);
				}
				DO = 1;
			}
		}
		else					// the chip is reading out data (transmitting)
		{
			// Shift out to DO pin
			DO = bitBufferOut&1;
			bitBufferOut >>= 1;
			++bitsOut;
			
			// If we've shifted out an entire 16-bit word, load up the next address (no preceding 0)
			if (bitsOut == 16)
			{
				addr = (addr+1)&0x3F;
				bitBufferOut = ReverseBits16(regs[addr]);
				bitsOut = 0;
				
				DebugLog("93C46: Next word loaded: %X\n", addr);
			}
		}
	} 
}

unsigned C93C46::Read(void)
{
	// When not transmitting, DO indicates whether busy or not
	if (receiving)
	{
		if (busyCycles > 0)	// simulate programming delay
		{
			--busyCycles;
			return 0;		// busy
		}
		else
			return 1;		// ready to accept new command
	}

	// Transmit data
	return DO;
}

void C93C46::Clear(void)
{
	memset(regs, 0xFF, sizeof(regs));
}

void C93C46::Reset(void)
{
	receiving = true;
	locked = true;
	bitBufferIn = 0;
	bitBufferOut = 0;
	addr = 0;
	busyCycles = 0;
	CS = 0;
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

void C93C46::Init(void)
{
	// this function really only exists for consistency with other device classes
}

C93C46::C93C46(void)
{	
	memset(regs, 0xFF, sizeof(regs));	
	DebugLog("Built 93C46 EEPROM\n");
}

C93C46::~C93C46(void)
{	
	DebugLog("Destroyed 93C46 EEPROM\n");
}
