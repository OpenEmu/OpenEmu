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
 * IRQ.cpp
 * 
 * Model 3 IRQ controller. Implementation of the CIRQ class.
 *
 * To-Do List:
 * -----------
 * - When a proper OO CPU core is added, the CPU object should be hooked to the
 *   IRQ controller to assert/deassert the PowerPC IRQ line. Right now, we just
 *   call the PPC core directly, which should not happen in proper OO code.
 */

#include "Supermodel.h"


/******************************************************************************
 Save States
******************************************************************************/

void CIRQ::SaveState(CBlockFile *SaveState)
{
	SaveState->NewBlock("IRQ", __FILE__);
	SaveState->Write(&irqEnable, sizeof(irqEnable));
	SaveState->Write(&irqState, sizeof(irqState));
}

void CIRQ::LoadState(CBlockFile *SaveState)
{
	if (OKAY != SaveState->FindBlock("IRQ"))
	{
		ErrorLog("Unable to load IRQ controller state. Save state file is corrupt.");
		return;
	}
	
	SaveState->Read(&irqEnable, sizeof(irqEnable));
	SaveState->Read(&irqState, sizeof(irqState));
}


/******************************************************************************
 Emulation Functions
******************************************************************************/

void CIRQ::Assert(unsigned irqBits)
{
	irqState |= irqBits;
	if ((irqState&irqEnable))	// low 8 bits are maskable interrupts
		//ppc_set_irq_line(0);
		ppc_set_irq_line(1);
	if ((irqState&(~0xFF)))		// any non-maskable interrupts pending?
		//ppc_set_irq_line(0);
		ppc_set_irq_line(1);
}

//TO-DO: CPU needs to have deassert logic!
void CIRQ::Deassert(unsigned irqBits)
{
	irqState &= ~irqBits;
}

void CIRQ::WriteIRQEnable(UINT8 data)
{
	irqEnable = (unsigned) data;
}

UINT8 CIRQ::ReadIRQEnable(void)
{
	return (UINT8) (irqEnable&0xFF);
}

UINT8 CIRQ::ReadIRQState(void)
{
	return (UINT8) (irqState&0xFF);
}

void CIRQ::Reset(void)
{
	irqEnable = 0;	// disable all
	irqState = 0;	// no IRQs pending
}


/******************************************************************************
 Configuration, Initialization, and Shutdown
******************************************************************************/

void CIRQ::Init(void)
{
	// this function really only exists for consistency with other device classes
}

CIRQ::CIRQ(void)
{	
	DebugLog("Built IRQ controller\n");
}

/*
 * CIRQ::~CIRQ(void):
 *
 * Destructor.
 */
CIRQ::~CIRQ(void)
{	
	DebugLog("Destroyed IRQ controller\n");
}
