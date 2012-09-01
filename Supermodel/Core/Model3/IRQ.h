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
 * IRQ.h
 * 
 * Header file defining the CIRQ class: Model 3 IRQ controller.
 */

#ifndef INCLUDED_IRQ_H
#define INCLUDED_IRQ_H


/*
 * CIRQ:
 *
 * Model 3 IRQ controller.
 */
class CIRQ
{
public:
	/*
	 * SaveState(SaveState):
	 *
	 * Saves an image of the current device state.
	 *
	 * Parameters:
	 *		SaveState	Block file to save state information to.
	 */
	void SaveState(CBlockFile *SaveState);

	/*
	 * LoadState(SaveState):
	 *
	 * Loads and a state image.
	 *
	 * Parameters:
	 *		SaveState	Block file to load state information from.
	 */
	void LoadState(CBlockFile *SaveState);
	
	/*
	 * Assert(irqBits):
	 *
	 * Assert IRQ(s). Also asserts the CPU IRQ line (unless no IRQs are 
	 * actually asserted). The IRQ controller only tracks 8 state bits but for
	 * non-maskable interrupts, higher bits (bits 9 and above) can be used.
	 * They will be tracked internally but not visible when external hardware
	 * tries reading the IRQ state.
	 *
	 * Parameters:
	 *		irqBits		IRQ bits corresponding to the IRQ state register (1
	 *					indicates assertion, 0 does nothing).
	 */
	void Assert(unsigned irqBits);
	
	/*
	 * Deassert(irqBits):
	 *
	 * Deasserts the specified IRQs and, if there are no pending IRQs left,
	 * deasserts the CPU IRQ line.
	 *
	 * Parameters:
	 *		irqBits		IRQ bits corresponding to the IRQ state register (1
	 *					indicates deassertion, 0 does nothing).
	 */
	void Deassert(unsigned irqBits);
	
	/*
	 * ReadIRQEnable(void):
	 *
	 * Returns:
	 *		The 8-bit IRQ enable register.
	 */
	UINT8 ReadIRQEnable(void);
	
	/*
	 * WriteIRQEnable(data):
	 *
	 * Write to the IRQ enable register. A 1 indicates the IRQ is enabled. Only
	 * the first 8 IRQs can be masked.
	 *
	 * Parameters:
	 *		data	IRQ enable bits (8 bits).
	 */
	void WriteIRQEnable(UINT8 data);
	
	/*
	 * ReadIRQState(void):
	 *
	 * Accesses the IRQ state register. Set bits indicate pending IRQs. Only
	 * the low 8 bits are returned. Any IRQs occupying bits above this are 
	 * effectively invisible and non-maskable.
	 *
	 * Returns:
	 *		The 8-bit IRQ state register.
	 */
	UINT8 ReadIRQState(void);
	
	/*
	 * Reset(void):
	 *
	 * Resets the IRQ controller. All IRQs are disabled.
	 */
	void Reset(void);
	
	/*
	 * Init(void):
	 *
	 * One-time initialization of the context. Must be called prior to all
	 * other members.
	 */
	void Init(void);
	 
	/*
	 * CIRQ(void):
	 * ~CIRQ(void):
	 *
	 * Constructor and destructor.
	 */
	CIRQ(void);
	~CIRQ(void);
	
private:
	unsigned	irqEnable;	// 8 bits, 1=enabled, 0=disabled
	unsigned	irqState;	// bits correspond to irqEnable, 1=pending, 0=not pending
};


#endif	// INCLUDED_IRQ_H
