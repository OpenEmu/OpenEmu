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
// ROM object header file                                                   //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and inline code for   //
// the class the emulates the internal 512 byte ROM embedded in Mikey       //
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

#ifndef ROM_H
#define ROM_H

#define ROM_SIZE				0x200
#define ROM_ADDR_MASK			0x01ff
#define DEFAULT_ROM_CONTENTS	0x88

#define BROM_START		0xfe00
#define BROM_SIZE		0x200
#define VECTOR_START	0xfffa
#define VECTOR_SIZE		0x6

class CRom : public CLynxBase
{

	// Function members

	public:
		CRom(const char *);

	public:
		void	Reset(void);
		void	Poke(uint32 addr,uint8 data) { if(mWriteEnable) mRomData[addr&ROM_ADDR_MASK]=data;};
		uint8	Peek(uint32 addr) { return(mRomData[addr&ROM_ADDR_MASK]);};
		uint32	ReadCycle(void) {return 5;};
		uint32	WriteCycle(void) {return 5;};
		uint32	ObjectSize(void) {return ROM_SIZE;};

	// Data members

	public:
		bool	mWriteEnable;
	private:
		uint8	mRomData[ROM_SIZE];
};

#endif

