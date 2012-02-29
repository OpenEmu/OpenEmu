// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __BIOS_H__
#define __BIOS_H__

#include "ameteor.hpp"

namespace AMeteor
{
	namespace Bios
	{
		// Entry point
		void Bios000h ();
		// Software IRQ
		void Bios008h ();
		void Bios168h ();
		// Return from IntrWait (after the IRQ)
		void Bios338h ();
		// IRQ
		void Bios018h ();
		void Bios130h ();

		void SoftReset (); // 00
		void RegisterRamReset (); // 01
		void Halt (); // 02
		void IntrWait (); // 04
		void VBlankIntrWait (); // 05
		void Div (); // 06
		void DivArm (); // 07
		void ArcTan (); // 09
		void ArcTan2 (); // 0A
		void CpuSet ();  // 0B
		void CpuFastSet ();  // 0C
		void BgAffineSet ();  // 0E
		void ObjAffineSet ();  // 0F
		void LZ77UnCompWram (); // 11
		void LZ77UnCompVram (); // 12
		void HuffUnComp (); // 13
		void RLUnCompWram (); // 14
		void RLUnCompVram (); // 15
	}
}

#endif
