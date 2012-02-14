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
// Lynx cartridge class header file                                         //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and code for some of  //
// the simpler cartridge API.                                               //
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

#ifndef CART_H
#define CART_H

#define DEFAULT_CART_CONTENTS	0x11

enum CTYPE {UNUSED,C64K,C128K,C256K,C512K,C1024K};

#define CART_NO_ROTATE		0
#define CART_ROTATE_LEFT	1
#define	CART_ROTATE_RIGHT	2

struct LYNX_HEADER
{
   uint8   magic[4];
   uint16   page_size_bank0;
   uint16   page_size_bank1;
   uint16   version;
   uint8   cartname[32];
   uint8   manufname[16];
   uint8   rotation; 
   uint8   spare[5];
};

class CCart : public CLynxBase
{

	// Function members

	public:
		CCart(const uint8 *gamedata, uint32 gamesize);
		~CCart();

	public:

// Access for sensible members of the clan
		enum { HEADER_RAW_SIZE = 64 };

		static LYNX_HEADER DecodeHeader(const uint8 *data);
		static bool TestMagic(const uint8 *data, uint32 size);

		void	Reset(void);
		void	Poke(uint32 addr,uint8 data);
		uint8	Peek(uint32 addr);
		uint32	ReadCycle(void) {return 15;};
		uint32	WriteCycle(void) {return 15;};
		void	BankSelect(EMMODE newbank) {mBank=newbank;}
		uint32	ObjectSize(void) {return (mBank==bank0)?mMaskBank0+1:mMaskBank1+1;};

		const char*	CartGetName(void) { return mName;};
		const char*	CartGetManufacturer(void) { return mManufacturer; };
		uint32	CartGetRotate(void) { return mRotation;};
		uint32	CRC32(void) { return mCRC32; };

		int StateAction(StateMem *sm, int load, int data_only);

// Access for the lynx itself, it has no idea of address etc as this is done by the
// cartridge emulation hardware 
		void	CartAddressStrobe(bool strobe);
		void	CartAddressData(bool data);
		void	Poke0(uint8 data);
		void	Poke1(uint8 data);
		uint8	Peek0(void);
		uint8	Peek1(void);

	// Data members

	public:
		uint32	mWriteEnableBank0;
		uint32	mWriteEnableBank1;
		uint32	mCartRAM;

		uint32  InfoROMSize;
                uint8   MD5[16];
	private:
		EMMODE	mBank;
		uint32	mMaskBank0;
		uint32	mMaskBank1;
		uint8	*mCartBank0;
		uint8	*mCartBank1;
		char	mName[33];
		char	mManufacturer[17];
		uint32	mRotation;

		uint32	mCounter;
		uint32	mShifter;
		uint32	mAddrData;
		uint32	mStrobe;

		uint32	mShiftCount0;
		uint32	mCountMask0;
		uint32	mShiftCount1;
		uint32	mCountMask1;

		uint32	mCRC32;
		int8 last_strobe;
};

#endif

