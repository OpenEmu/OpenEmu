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
 * Interrupt.h
 */

#ifdef SUPERMODEL_DEBUGGER
#ifndef INCLUDED_INTERRUPT_H
#define INCLUDED_INTERRUPT_H

#include "Types.h"

namespace Debugger
{
	class CCPUDebug;

	/*
	 * Class that represents a CPU interrupt (just a specialised CPU exception).
	 */
	class CInterrupt
	{
	public:
		CCPUDebug *cpu;
		const char *id;
		const UINT16 code;
		const char *name;

		bool trap;
		unsigned long count;
		
		CInterrupt(CCPUDebug *intCPU, const char *intId, UINT16 intCode, const char *intName);
	};
}

#endif	// INCLUDED_INTERRUPT_H
#endif  // SUPERMODEL_DEBUGGER