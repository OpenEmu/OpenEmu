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
// RAM object header file                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the RAM class     //
// that emulates the Handy system RAM (64K)                                 //
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

#ifndef RAM_H
#define RAM_H

#define RAM_SIZE				65536
#define RAM_ADDR_MASK			0xffff
#define DEFAULT_RAM_CONTENTS	0xff

struct HOME_HEADER
{
   uint16   jump;
   uint16   load_address;
   uint16   size;
   uint8   magic[4];
};

class CRam : public CLynxBase
{

	// Function members

	public:
		CRam(const uint8 *filememory,uint32 filesize);
		~CRam();

	public:

		void	Reset(void);

		void	Poke(uint32 addr, uint8 data){ mRamData[addr]=data;};
		uint8	Peek(uint32 addr){ return(mRamData[addr]);};
		uint32	ReadCycle(void) {return 5;};
		uint32	WriteCycle(void) {return 5;};
		uint32   ObjectSize(void) {return RAM_SIZE;};
		uint8*	GetRamPointer(void) { return mRamData; };

	// Data members

	private:
		uint8	mRamData[RAM_SIZE];
		uint8	*mFileData;
		uint32	mFileSize;

};

#endif

