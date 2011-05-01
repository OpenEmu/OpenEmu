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
// Lynx Cartridge Class                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the Lynx cartridge interface, given a filename it    //
// will contstruct a cartridge object via the constructor.                  //
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

#define CART_CPP

#include "system.h"

#include <algorithm>
#include <string.h>
#include <zlib.h>
#include "cart.h"
#include "../state.h"
#include "../md5.h"

bool CCart::TestMagic(const uint8 *data, uint32 size)
{
 if(size <= (int)sizeof(LYNX_HEADER))
  return(FALSE);

 if(memcmp(data, "LYNX", 4) || data[8] != 0x01)
  return(FALSE);

 return(TRUE);
}

CCart::CCart(const uint8 *gamedata, uint32 gamesize)
{
	LYNX_HEADER	header;
	uint32 loop;

	mWriteEnableBank0=FALSE;
	mWriteEnableBank1=FALSE;
	mCartRAM=FALSE;

	mCRC32 = 0;
	mCRC32 = crc32(mCRC32,gamedata,gamesize);
	
	// Checkout the header bytes
	if(gamesize <= sizeof(LYNX_HEADER))
	 throw(-1);

	memcpy(&header, gamedata, sizeof(LYNX_HEADER));
	gamedata += sizeof(LYNX_HEADER);
	gamesize -= sizeof(LYNX_HEADER);

	InfoROMSize = gamesize;

	#ifdef MSB_FIRST
	header.page_size_bank0 = ((header.page_size_bank0>>8) | (header.page_size_bank0<<8));
	header.page_size_bank1 = ((header.page_size_bank1>>8) | (header.page_size_bank1<<8));
	header.version         = ((header.version>>8) | (header.version<<8));
	#endif
	// Sanity checks on the header

	if(header.magic[0]!='L' || header.magic[1]!='Y' || header.magic[2]!='N' || header.magic[3]!='X' || header.version!=1)
	{
		throw(-1);
	}
	// Setup name & manufacturer
	strncpy(mName,(char*)&header.cartname, 32);
	strncpy(mManufacturer,(char*)&header.manufname, 16);

	// Setup rotation
	mRotation=header.rotation;
	if(mRotation!=CART_NO_ROTATE && mRotation!=CART_ROTATE_LEFT && mRotation!=CART_ROTATE_RIGHT) mRotation=CART_NO_ROTATE;

	// Set the filetypes

	CTYPE banktype0,banktype1;

	switch(header.page_size_bank0)
	{
		case 0x000:
			banktype0=UNUSED;
			mMaskBank0=0;
			mShiftCount0=0;
			mCountMask0=0;
			break;
		case 0x100:
			banktype0=C64K;
			mMaskBank0=0x00ffff;
			mShiftCount0=8;
			mCountMask0=0x0ff;
			break;
		case 0x200:
			banktype0=C128K;
			mMaskBank0=0x01ffff;
			mShiftCount0=9;
			mCountMask0=0x1ff;
			break;
		case 0x400:
			banktype0=C256K;
			mMaskBank0=0x03ffff;
			mShiftCount0=10;
			mCountMask0=0x3ff;
			break;
		case 0x800:
			banktype0=C512K;
			mMaskBank0=0x07ffff;
			mShiftCount0=11;
			mCountMask0=0x7ff;
			break;
		default:
			//return(0);
			//CLynxException lynxerr;
			//lynxerr.Message() << "Handy Error: File format invalid (Bank0)";
			//lynxerr.Description()
			//	<< "The image you selected was not a recognised game cartridge format." << endl
			//	<< "(see the Handy User Guide for more information).";
			//throw(lynxerr);
			throw(0);
			break;
	}

	switch(header.page_size_bank1)
	{
		case 0x000:
			banktype1=UNUSED;
			mMaskBank1=0;
			mShiftCount1=0;
			mCountMask1=0;
			break;
		case 0x100:
			banktype1=C64K;
			mMaskBank1=0x00ffff;
			mShiftCount1=8;
			mCountMask1=0x0ff;
			break;
		case 0x200:
			banktype1=C128K;
			mMaskBank1=0x01ffff;
			mShiftCount1=9;
			mCountMask1=0x1ff;
			break;
		case 0x400:
			banktype1=C256K;
			mMaskBank1=0x03ffff;
			mShiftCount1=10;
			mCountMask1=0x3ff;
			break;
		case 0x800:
			banktype1=C512K;
			mMaskBank1=0x07ffff;
			mShiftCount1=11;
			mCountMask1=0x7ff;
			break;
		default:
			//return(0);
			//CLynxException lynxerr;
			//lynxerr.Message() << "Handy Error: File format invalid (Bank1)";
			//lynxerr.Description()
			//	<< "The image you selected was not a recognised game cartridge format." << endl
			//	<< "(see the Handy User Guide for more information).";
			//throw(lynxerr);
			throw(0);
			break;
	}

	// Make some space for the new carts

	mCartBank0 = (uint8*) new uint8[mMaskBank0+1];
	mCartBank1 = (uint8*) new uint8[mMaskBank1+1];

	// Set default bank

	mBank=bank0;

	// Initialiase

	for(loop=0;loop<mMaskBank0+1;loop++)
		mCartBank0[loop]=DEFAULT_CART_CONTENTS;

	for(loop=0;loop<mMaskBank1+1;loop++)
		mCartBank1[loop]=DEFAULT_CART_CONTENTS;

	// Read in the BANK0 bytes

        md5_context md5;
        md5.starts();

        if(mMaskBank0)
        {
         int size = std::min(gamesize, mMaskBank0+1);
         memcpy(mCartBank0, gamedata, size);
         md5.update(mCartBank0, size);
         gamedata += size;
         gamesize -= size;
        }

        // Read in the BANK0 bytes
        if(mMaskBank1)
        {
         int size = std::min(gamesize, mMaskBank1+1);
         memcpy(mCartBank1, gamedata, size);
         md5.update(mCartBank1, size);
         gamedata += size;
        }

        md5.finish(MD5);

	// As this is a cartridge boot unset the boot address

	gCPUBootAddress=0;

	// Dont allow an empty Bank1 - Use it for shadow SRAM/EEPROM
	if(banktype1==UNUSED)
	{
		// Delete the single byte allocated  earlier
		delete[] mCartBank1;
		// Allocate some new memory for us
		banktype1=C64K;
		mMaskBank1=0x00ffff;
		mShiftCount1=8;
		mCountMask1=0x0ff;
		mCartBank1 = (uint8*) new uint8[mMaskBank1+1];
		for(loop=0;loop<mMaskBank1+1;loop++) mCartBank1[loop]=DEFAULT_RAM_CONTENTS;
		mWriteEnableBank1=TRUE;
		mCartRAM=TRUE;
	}
}

CCart::~CCart()
{
	delete[] mCartBank0;
	delete[] mCartBank1;
}


void CCart::Reset(void)
{
	mCounter=0;
	mShifter=0;
	mAddrData=0;
	mStrobe=0;
	last_strobe = 0;
}

INLINE void CCart::Poke(uint32 addr, uint8 data)
{
	if(mBank==bank0)
	{
		if(mWriteEnableBank0) mCartBank0[addr&mMaskBank0]=data;
	}
	else
	{
		if(mWriteEnableBank1) mCartBank1[addr&mMaskBank1]=data;
	}
}


INLINE uint8 CCart::Peek(uint32 addr)
{
	if(mBank==bank0)
	{
		return(mCartBank0[addr&mMaskBank0]);
	}
	else
	{
		return(mCartBank1[addr&mMaskBank1]);
	}
}


void CCart::CartAddressStrobe(bool strobe)
{
	mStrobe=strobe;

	if(mStrobe) mCounter=0;

	//
	// Either of the two below seem to work OK.
	//
	// if(!strobe && last_strobe)
	//
	if(mStrobe && !last_strobe)
	{
		// Clock a bit into the shifter
		mShifter=mShifter<<1;
		mShifter+=mAddrData?1:0;
		mShifter&=0xff;
	}
	last_strobe=mStrobe;
}

void CCart::CartAddressData(bool data)
{
	mAddrData=data;
}


void CCart::Poke0(uint8 data)
{
	if(mWriteEnableBank0)
	{
		uint32 address=(mShifter<<mShiftCount0)+(mCounter&mCountMask0);
		mCartBank0[address&mMaskBank0]=data;		
	}
	if(!mStrobe)
	{
		mCounter++;
		mCounter&=0x07ff;
	}
}

void CCart::Poke1(uint8 data)
{
	if(mWriteEnableBank1)
	{
		uint32 address=(mShifter<<mShiftCount1)+(mCounter&mCountMask1);
		mCartBank1[address&mMaskBank1]=data;		
	}
	if(!mStrobe)
	{
		mCounter++;
		mCounter&=0x07ff;
	}
}


uint8 CCart::Peek0(void)
{
	uint32 address=(mShifter<<mShiftCount0)+(mCounter&mCountMask0);
	uint8 data=mCartBank0[address&mMaskBank0];		

	if(!mStrobe)
	{
		mCounter++;
		mCounter&=0x07ff;
	}

	return data;
}

uint8 CCart::Peek1(void)
{
	uint32 address=(mShifter<<mShiftCount1)+(mCounter&mCountMask1);
	uint8 data=mCartBank1[address&mMaskBank1];		

	if(!mStrobe)
	{
		mCounter++;
		mCounter&=0x07ff;
	}

	return data;
}


int CCart::StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT CartRegs[] =
 {
        SFVAR(mCounter),
        SFVAR(mShifter),
        SFVAR(mAddrData),
        SFVAR(mStrobe),
        SFVAR(mShiftCount0),
        SFVAR(mCountMask0),
        SFVAR(mShiftCount1),
        SFVAR(mCountMask1),
        SFVAR(mBank),
        SFVAR(mWriteEnableBank0),
        SFVAR(mWriteEnableBank1),
	SFVAR(last_strobe),
	SFARRAYN(mCartBank1, mCartRAM ? mMaskBank1 + 1 : 0, "mCartBank1"),
	SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, CartRegs, "CART");


 return(ret);
}

