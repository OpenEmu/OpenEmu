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

//#include <crtdbg.h>
//#define   TRACE_CART

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "cart.h"
#include "./zlib-113/zlib.h"
#include <iostream>

#ifdef WII
#include "wii_handy.h"
#include "wii_hash.h"
#endif

CCart::CCart(UBYTE *gamedata,ULONG gamesize)
{
	TRACE_CART1("CCart() called with %s",gamefile);
	LYNX_HEADER	header;

	mWriteEnableBank0=FALSE;
	mWriteEnableBank1=FALSE;
	mCartRAM=FALSE;
	mHeaderLess=FALSE;
	mCRC32=0;
	mCRC32=crc32(mCRC32,gamedata,gamesize);
	
	// Open up the file

	if(gamesize)
	{
		// Checkout the header bytes
		memcpy(&header,gamedata,sizeof(LYNX_HEADER));

#ifdef MSB_FIRST
		header.page_size_bank0 = ((header.page_size_bank0>>8) | (header.page_size_bank0<<8));
		header.page_size_bank1 = ((header.page_size_bank1>>8) | (header.page_size_bank1<<8));
		header.version         = ((header.version>>8) | (header.version<<8));
#endif
		// Sanity checks on the header

		if(header.magic[0]!='L' || header.magic[1]!='Y' || header.magic[2]!='N' || header.magic[3]!='X' || header.version!=1)
		{
			CLynxException lynxerr;

			lynxerr.Message() << "Handy Error: File format invalid (Magic No)";
			lynxerr.Description()
				<< "The image you selected was not a recognised game cartridge format." << endl
				<< "(see the Handy User Guide for more information).";
			throw(lynxerr);
		}

		// Setup name & manufacturer

		strncpy(mName,(char*)&header.cartname,32);;
		strncpy(mManufacturer,(char*)&header.manufname,16);

		// Setup rotation

		mRotation=header.rotation;
		if(mRotation!=CART_NO_ROTATE && mRotation!=CART_ROTATE_LEFT && mRotation!=CART_ROTATE_RIGHT) mRotation=CART_NO_ROTATE;

#ifdef WII
    // Calculate the hash minus header information
    int headersize = sizeof(LYNX_HEADER);
    if( gamesize > headersize )
    {
      wii_hash_compute( 
        gamedata+headersize, gamesize-headersize, wii_cartridge_hash );
    }
#endif
	}
	else
	{
		header.page_size_bank0=0x000;
		header.page_size_bank1=0x000;

		// Setup name & manufacturer

		strcpy(mName,"<No cart loaded>");
		strcpy(mManufacturer,"<No cart loaded>");

		// Setup rotation

		mRotation=CART_NO_ROTATE;
	}
	

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
			CLynxException lynxerr;
			lynxerr.Message() << "Handy Error: File format invalid (Bank0)";
			lynxerr.Description()
				<< "The image you selected was not a recognised game cartridge format." << endl
				<< "(see the Handy User Guide for more information).";
			throw(lynxerr);
			break;
	}
	TRACE_CART1("CCart() - Bank0 = $%06x",mMaskBank0);

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
			CLynxException lynxerr;
			lynxerr.Message() << "Handy Error: File format invalid (Bank1)";
			lynxerr.Description()
				<< "The image you selected was not a recognised game cartridge format." << endl
				<< "(see the Handy User Guide for more information).";
			throw(lynxerr);
			break;
	}
	TRACE_CART1("CCart() - Bank1 = $%06x",mMaskBank1);

	// Make some space for the new carts

	mCartBank0 = (UBYTE*) new UBYTE[mMaskBank0+1];
	mCartBank1 = (UBYTE*) new UBYTE[mMaskBank1+1];

	// Set default bank

	mBank=bank0;

	// Initialiase

	int cartsize = __max(0, int(gamesize - sizeof(LYNX_HEADER)));
	int bank0size = __min(cartsize, (int)(mMaskBank0+1));
	int bank1size = __min(cartsize, (int)(mMaskBank1+1));

	memcpy(
		mCartBank0,
		gamedata+(sizeof(LYNX_HEADER)),
		bank0size);
	memset(
		mCartBank0 + bank0size,
		DEFAULT_CART_CONTENTS,
		mMaskBank0+1 - bank0size);
	memcpy(
		mCartBank1,
		gamedata+(sizeof(LYNX_HEADER)),
		bank1size);
	memset(
		mCartBank1 + bank0size,
		DEFAULT_CART_CONTENTS,
		mMaskBank1+1 - bank1size);

	// Copy the cart banks from the image
	if(gamesize)
	{
		// As this is a cartridge boot unset the boot address

		gCPUBootAddress=0;

		//
		// Check if this is a headerless cart
		//
		mHeaderLess=TRUE;
		for(int loop=0;loop<32;loop++)
		{
			if(mCartBank0[loop&mMaskBank0]!=0x00) mHeaderLess=FALSE;
		}
		TRACE_CART1("CCart() - mHeaderLess=%d",mHeaderLess);		
	}

	// Dont allow an empty Bank1 - Use it for shadow SRAM/EEPROM
	if(banktype1==UNUSED)
	{
		// Delete the single byte allocated  earlier
		delete[] mCartBank1;
		// Allocate some new memory for us
		TRACE_CART0("CCart() - Bank1 being converted to 64K SRAM");
		banktype1=C64K;
		mMaskBank1=0x00ffff;
		mShiftCount1=8;
		mCountMask1=0x0ff;
		mCartBank1 = (UBYTE*) new UBYTE[mMaskBank1+1];
		memset(mCartBank1, DEFAULT_RAM_CONTENTS, mMaskBank1+1);
		mWriteEnableBank1=TRUE;
		mCartRAM=TRUE;
	}
}

CCart::~CCart()
{
	TRACE_CART0("~CCart()");
	delete[] mCartBank0;
	delete[] mCartBank1;
}


void CCart::Reset(void)
{
	TRACE_CART0("Reset()");
	mCounter=0;
	mShifter=0;
	mAddrData=0;
	mStrobe=0;
}

bool CCart::ContextSave(FILE *fp)
{	
	TRACE_CART0("ContextSave()");
	if(!fprintf(fp,"CCart::ContextSave")) return 0;
	if(!fwrite(&mCounter,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mShifter,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAddrData,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mStrobe,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mShiftCount0,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mCountMask0,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mShiftCount1,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mCountMask1,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mBank,sizeof(EMMODE),1,fp)) return 0;
	if(!fwrite(&mWriteEnableBank0,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mWriteEnableBank1,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mCartRAM,sizeof(ULONG),1,fp)) return 0;
	if(mCartRAM)
	{
		if(!fwrite(&mMaskBank1,sizeof(ULONG),1,fp)) return 0;
		if(!fwrite(mCartBank1,sizeof(UBYTE),mMaskBank1+1,fp)) return 0;
	}
	return 1;
}

#ifdef WII
bool CCart::ContextSaveLegacy(FILE *fp)
{
  TRACE_CART0("ContextSave()");
  if(!fprintf(fp,"CCart::ContextSave")) return 0;
  if(!fwrite(&mRotation,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mHeaderLess,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mCounter,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mShifter,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mAddrData,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mStrobe,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mShiftCount0,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mCountMask0,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mShiftCount1,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mCountMask1,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mBank,sizeof(EMMODE),1,fp)) return 0;
  if(!fwrite(&mMaskBank0,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(&mMaskBank1,sizeof(ULONG),1,fp)) return 0;
  if(!fwrite(mCartBank0,sizeof(UBYTE),mMaskBank0+1,fp)) return 0;
  if(!fwrite(mCartBank1,sizeof(UBYTE),mMaskBank1+1,fp)) return 0;

	return 1;
}
#endif

bool CCart::ContextLoad(LSS_FILE *fp)
{
	TRACE_CART0("ContextLoad()");
	char teststr[100]="XXXXXXXXXXXXXXXXXX";
	if(!lss_read(teststr,sizeof(char),18,fp)) return 0;
	if(strcmp(teststr,"CCart::ContextSave")!=0) return 0;
	if(!lss_read(&mCounter,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mShifter,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAddrData,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mStrobe,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mShiftCount0,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mCountMask0,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mShiftCount1,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mCountMask1,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mBank,sizeof(EMMODE),1,fp)) return 0;
	if(!lss_read(&mWriteEnableBank0,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mWriteEnableBank1,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mCartRAM,sizeof(ULONG),1,fp)) return 0;
	if(mCartRAM)
	{
		if(!lss_read(&mMaskBank1,sizeof(ULONG),1,fp)) return 0;
		delete[] mCartBank1;
		mCartBank1 = new UBYTE[mMaskBank1+1];
		if(!lss_read(mCartBank1,sizeof(UBYTE),mMaskBank1+1,fp)) return 0;
	}
	return 1;
}

bool CCart::ContextLoadLegacy(LSS_FILE *fp)
{
	TRACE_CART0("ContextLoadLegacy()");
	strcpy(mName,"<** IMAGE **>");
	strcpy(mManufacturer,"<** RESTORED **>");
	char teststr[100]="XXXXXXXXXXXXXXXXXX";
	if(!lss_read(teststr,sizeof(char),18,fp)) return 0;
	if(strcmp(teststr,"CCart::ContextSave")!=0) return 0;
	if(!lss_read(&mRotation,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mHeaderLess,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mCounter,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mShifter,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAddrData,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mStrobe,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mShiftCount0,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mCountMask0,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mShiftCount1,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mCountMask1,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mBank,sizeof(EMMODE),1,fp)) return 0;

	if(!lss_read(&mMaskBank0,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mMaskBank1,sizeof(ULONG),1,fp)) return 0;

	delete[] mCartBank0;
	delete[] mCartBank1;
	mCartBank0 = new UBYTE[mMaskBank0+1];
	mCartBank1 = new UBYTE[mMaskBank1+1];
	if(!lss_read(mCartBank0,sizeof(UBYTE),mMaskBank0+1,fp)) return 0;
	if(!lss_read(mCartBank1,sizeof(UBYTE),mMaskBank1+1,fp)) return 0;
	return 1;
}

inline void CCart::Poke(ULONG addr, UBYTE data)
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


inline UBYTE CCart::Peek(ULONG addr)
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
	static int last_strobe=0;

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
	TRACE_CART2("CartAddressStrobe(strobe=%d) mShifter=$%06x",strobe,mShifter);
}

void CCart::CartAddressData(bool data)
{
	TRACE_CART1("CartAddressData($%02x)",data);
	mAddrData=data;
}


void CCart::Poke0(UBYTE data)
{
	if(mWriteEnableBank0)
	{
		ULONG address=(mShifter<<mShiftCount0)+(mCounter&mCountMask0);
		mCartBank0[address&mMaskBank0]=data;		
	}
	if(!mStrobe)
	{
		mCounter++;
		mCounter&=0x07ff;
	}
}

void CCart::Poke1(UBYTE data)
{
	if(mWriteEnableBank1)
	{
		ULONG address=(mShifter<<mShiftCount1)+(mCounter&mCountMask1);
		mCartBank1[address&mMaskBank1]=data;		
	}
	if(!mStrobe)
	{
		mCounter++;
		mCounter&=0x07ff;
	}
}


UBYTE CCart::Peek0(void)
{
	ULONG address=(mShifter<<mShiftCount0)+(mCounter&mCountMask0);
	UBYTE data=mCartBank0[address&mMaskBank0];		

	if(!mStrobe)
	{
		mCounter++;
		mCounter&=0x07ff;
	}

	return data;
}

UBYTE CCart::Peek1(void)
{
	ULONG address=(mShifter<<mShiftCount1)+(mCounter&mCountMask1);
	UBYTE data=mCartBank1[address&mMaskBank1];		

	if(!mStrobe)
	{
		mCounter++;
		mCounter&=0x07ff;
	}

	return data;
}


