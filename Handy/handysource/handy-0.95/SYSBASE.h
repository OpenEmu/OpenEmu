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
// Systembase object class definition                                       //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the systembase    //
// class that is required to get around cross dependencies between          //
// cpu/mikie/system classes                                                 //
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

#ifndef SYSBASE_H
#define SYSBASE_H


class CSystemBase
{
	// Function members

	public:
		virtual ~CSystemBase() {};

	public:
		virtual void	Reset(void)=0;
		virtual void	Poke_CPU(ULONG addr,UBYTE data)=0;
		virtual UBYTE	Peek_CPU(ULONG addr)=0;
		virtual void	PokeW_CPU(ULONG addr,UWORD data)=0;
		virtual UWORD	PeekW_CPU(ULONG addr)=0;

		virtual void	Poke_RAM(ULONG addr,UBYTE data)=0;
		virtual UBYTE	Peek_RAM(ULONG addr)=0;
		virtual void	PokeW_RAM(ULONG addr,UWORD data)=0;
		virtual UWORD	PeekW_RAM(ULONG addr)=0;

		virtual UBYTE*	GetRamPointer(void)=0;

#ifdef _LYNXDBG
		virtual	void	DebugTrace(int address)=0;
#endif
};

#endif
