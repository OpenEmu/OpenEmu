//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Lynx memory map class                                                    //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class provides the register $FFF9 functionality to the emulator, it //
// sets which devices can be seen by the CPU.                               //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define MEMMAP_CPP

//#include <crtdbg.h>
//#define	TRACE_MEMMAP

#include "system.h"
#include "memmap.h"

#include <string.h>

// IGNORE THIS TEXT, now overridden by new system
//
// We will hold 16 different memory maps for the "top" area which are selected
// on the basis of mMemMap->mSelector:
//
//				Code	Vect	ROM		Mikie	Susie
//----------------------------------------------------
//	(Default)	0000	V		R		M		S
//				0001	V		R		M		RAM
//				0001	V		R		RAM		S
//				0011	V		R		RAM		RAM
//				0100	V		RAM		M		S
//				..
//				..
//				1111	RAM		RAM		RAM		RAM
//
// Get it.....
//
// We can then index with mMemoryHandlers[mMemMap->mSelector][addr] for speed
//

CMemMap::CMemMap(CSystem& parent)
	:mSystem(parent)
{
	Reset();
}


void CMemMap::Reset(void)
{

	// Initialise ALL pointers to RAM then overload to correct
	for(int loop=0;loop<SYSTEM_SIZE;loop++) mSystem.mMemoryHandlers[loop]=mSystem.mRam;

	// Special case for ourselves.
	mSystem.mMemoryHandlers[0xFFF8]=mSystem.mRam;
	mSystem.mMemoryHandlers[0xFFF9]=mSystem.mMemMap;

	mSusieEnabled=-1;
	mMikieEnabled=-1;
	mRomEnabled=-1;
	mVectorsEnabled=-1;

	// Initialise everything correctly
	Poke(0,0);

}


INLINE void CMemMap::Poke(uint32 addr, uint8 data)
{
	TRACE_MEMMAP1("Poke() - Data %02x",data);

	int newstate,loop;

	// FC00-FCFF Susie area
	newstate=(data&0x01)?FALSE:TRUE;
	if(newstate!=mSusieEnabled)
	{
		mSusieEnabled=newstate;

		if(mSusieEnabled)
		{
			for(loop=SUSIE_START;loop<SUSIE_START+SUSIE_SIZE;loop++) mSystem.mMemoryHandlers[loop]=mSystem.mSusie;
		}
		else
		{
			for(loop=SUSIE_START;loop<SUSIE_START+SUSIE_SIZE;loop++) mSystem.mMemoryHandlers[loop]=mSystem.mRam;
		}
	}

	// FD00-FCFF Mikie area
	newstate=(data&0x02)?FALSE:TRUE;
	if(newstate!=mMikieEnabled)
	{
		mMikieEnabled=newstate;

		if(mMikieEnabled)
		{
			for(loop=MIKIE_START;loop<MIKIE_START+MIKIE_SIZE;loop++) mSystem.mMemoryHandlers[loop]=mSystem.mMikie;
		}
		else
		{
			for(loop=MIKIE_START;loop<MIKIE_START+MIKIE_SIZE;loop++) mSystem.mMemoryHandlers[loop]=mSystem.mRam;
		}
	}

	// FE00-FFF7 Rom area
	newstate=(data&0x04)?FALSE:TRUE;
	if(newstate!=mRomEnabled)
	{
		mRomEnabled=newstate;

		if(mRomEnabled)
		{
			for(loop=BROM_START;loop<BROM_START+(BROM_SIZE-8);loop++) mSystem.mMemoryHandlers[loop]=mSystem.mRom;
		}
		else
		{
			for(loop=BROM_START;loop<BROM_START+(BROM_SIZE-8);loop++) mSystem.mMemoryHandlers[loop]=mSystem.mRam;
		}
	}

	// FFFA-FFFF Vector area - Overload ROM space
	newstate=(data&0x08)?FALSE:TRUE;
	if(newstate!=mVectorsEnabled)
	{
		mVectorsEnabled=newstate;

		if(mVectorsEnabled)
		{
			for(loop=VECTOR_START;loop<VECTOR_START+VECTOR_SIZE;loop++) mSystem.mMemoryHandlers[loop]=mSystem.mRom;
		}
		else
		{
			for(loop=VECTOR_START;loop<VECTOR_START+VECTOR_SIZE;loop++) mSystem.mMemoryHandlers[loop]=mSystem.mRam;
		}
	}


}

INLINE uint8 CMemMap::Peek(uint32 addr)
{
	uint8 retval=0;

	retval+=(mSusieEnabled)?0:0x01;
	retval+=(mMikieEnabled)?0:0x02;
	retval+=(mRomEnabled)?0:0x04;
	retval+=(mVectorsEnabled)?0:0x08;
	TRACE_MEMMAP1("Peek() - Data %02x",retval);
	return retval;
}

int CMemMap::StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT MemMapRegs[] =
 {
        SFVAR(mMikieEnabled),
        SFVAR(mSusieEnabled),
        SFVAR(mRomEnabled),
        SFVAR(mVectorsEnabled),
	SFEND
 };
 std::vector <SSDescriptor> love;
 love.push_back(SSDescriptor(MemMapRegs, "MMAP"));
 int ret = MDFNSS_StateAction(sm, load, data_only, love);

 if(load)
 {
        // The peek will give us the correct value to put back
        uint8 mystate=Peek(0);

        // Now set to un-initialised so the poke will set correctly
        mSusieEnabled=-1;
        mMikieEnabled=-1;
        mRomEnabled=-1;
        mVectorsEnabled=-1;

        // Set banks correctly
        Poke(0,mystate);
 }

 return ret;
}


//END OF FILE
