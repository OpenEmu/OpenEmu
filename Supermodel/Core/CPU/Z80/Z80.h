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
 * Z80.h
 *
 * Z80 emulator header file. Based on the Z80 instruction set simulator by
 * Frank D. Cringle and taken from YAZE-AG by Andreas Gerlich.
 *
 * Known inaccuracies:
 *
 *		- Interrupts are accepted immediately after EI instruction. In reality,
 *		  interrupts become enabled after the instruction following EI. This
 *		  could be implemented with a state machine in Run() if needed.
 *		  This feature exists to prevent interrupts from occuring before a RETI
 *		  instruction executes in the interrupt handler. Interrupt callbacks
 *		  should take care to immediately clear the interrupt line, otherwise,
 *		  an improperly timed interrupt may occur right after an EI instruction
 *		  but before the service routine has a chance to return.
 *		- HALT instruction is not implemented (it just exits).
 *		- 16-bit words are read as two bytes but these reads may not occur in
 *		  the exact same order as the real device. Needs to be checked.
 */

#ifndef INCLUDED_Z80_H
#define INCLUDED_Z80_H

#include "Types.h"
#include "CPU/Bus.h"
#include "BlockFile.h"

/*
 * Special Return Codes for Interrupt Callbacks
 *
 * When the Z80 takes an interrupt in mode 0, it expects an instruction (up to
 * 3 bytes) to be presented on the bus. Typically, an RST instruction is
 * supplied, and this is the only supported behavior here. The interrupt
 * callback must return one of these values when the Z80 is in mode 0.
 */
#define Z80_INT_RST_00	0xC7	// RST 0x00 (jumps to 0x0000)
#define Z80_INT_RST_08	0xCF	// RST 0x08 (jumps to 0x0008)
#define Z80_INT_RST_10	0xD7	// RST 0x10 (...)
#define Z80_INT_RST_18	0xDF	// RST 0x18
#define Z80_INT_RST_20	0xE7	// RST 0x20
#define Z80_INT_RST_28	0xEF	// RST 0x28
#define Z80_INT_RST_30	0xF7	// RST 0x30
#define Z80_INT_RST_38	0xFF	// RST 0x38

#ifdef SUPERMODEL_DEBUGGER
// Extra interrupt types
#define Z80_EX_NMI 0
#define Z80_IM1_IRQ 1
#define Z80_IM2_VECTOR 2

// 8-bit registers
#define Z80_REG8_IFF 0
#define Z80_REG8_IM 1
#define Z80_REG8_I 2
#define Z80_REG8_R 3
#define Z80_REG8_A 4
#define Z80_REG8_F 5
#define Z80_REG8_B 6
#define Z80_REG8_C 7
#define Z80_REG8_D 8
#define Z80_REG8_E 9
#define Z80_REG8_H 10
#define Z80_REG8_L 11

// 16-bit registers
#define Z80_REG16_SP 0
#define Z80_REG16_PC 1
#define Z80_REG16_IR 2
#define Z80_REG16_AF 3
#define Z80_REG16_BC 4
#define Z80_REG16_DE 5
#define Z80_REG16_HL 6
#define Z80_REG16_IX 7
#define Z80_REG16_IY 8
#define Z80_REG16_AF_ 9
#define Z80_REG16_BC_ 10
#define Z80_REG16_DE_ 11
#define Z80_REG16_HL_ 12

class Debugger::CZ80Debug;
#endif // SUPERMODEL_DEBUGGER

/*
 * CZ80:
 *
 * A Z80 CPU.
 */
class CZ80
{
public:
	/*
	 * Run(numCycles):
	 *
	 * Runs the Z80 for the specified number of instruction cycles.
	 *
	 * Parameters:
	 *		numCycles	Number of instruction cycles to execute.
	 *
	 * Returns:
	 *		Number of instruction cycles actually executed.
	 */
	int Run(int numCycles);
	
	/*
	 * TriggerNMI(void):
	 *
	 * Triggers a non-maskable interrupt. This is equivalent to a high-to-low
	 * transition on the NMI pin (NMI pin is triggered on falling edges). This
	 * may be called while the Z80 is running. NMIs are always higher priority
	 * than interrupts and are taken first.
	 */
	void TriggerNMI(void);
	
	/*
	 * SetINT(state):
	 *
	 * Set or clear the interrupt request. This may be called from memory 
	 * handlers while the Z80 is running and should be used by interrupt 
	 * callbacks to clear interrupts.
	 *
	 * Parameters:
	 *		state	If TRUE, this is equivalent to /INT being asserted on the
	 *				Z80 (INT line low, which triggers an interrupt). If FALSE,
	 *				this deasserts /INT (INT line high, no interrupt pending).
	 */
	void SetINT(bool state);

	/*
	 * GetPC(void):
	 *
	 * Returns the current PC value.
	 *
	 * Returns:
	 *		Current value of PC register.
	 */
	UINT16 GetPC(void);

#ifdef SUPERMODEL_DEBUGGER
	/*
	 * GetReg8(reg8):
	 *
	 * Reads an 8-bit register.
	 *
	 * Parameters:
	 *		reg8	Register number (use Z80_REG8_* macros).
	 *
	 * Returns:
	 * 		Value of given 8-bit register.
	 */
	UINT8 GetReg8(unsigned reg8);

	/*
	 * SetReg8(reg8, value):
	 *
	 * Sets value of given 8-bit register.
	 *
	 * Parameters:
	 *		reg8	Register number.
	 *		value	Value to write.
	 *
	 * Returns:
	 *		True if succeeded, false if invalid register.
	 */
	bool SetReg8(unsigned reg8, UINT8 value);

	/*
	 * GetReg16(reg16, value):
	 *
	 * Reads a 16-bit register.
	 *
	 * Parameters:
	 *		reg16	Register number (use Z80_REG16_* macros).
	 *
	 * Returns:
	 * 		Value of given 16-bit register.
	 */
	UINT16 GetReg16(unsigned reg16);

	/*
	 * SetReg16(state):
	 *
	 * Sets value of given 16-bit register.
	 *
	 * Parameters:
	 *		reg16	Register.
	 *		value	Value to write.
	 *
	 * Returns:
	 *		True if succeeded, false if invalid register.
	 */
	bool SetReg16(unsigned reg16, UINT16 value);
#endif // SUPERMODEL_DEBUGGER

	/*
	 * Reset(void):
	 *
	 * Resets the Z80, clearing all registers and pending interrupt requests.
	 */
	void Reset(void);
	
	/*
	 * SaveState(StateFile, name):
	 *
	 * Saves the CPU state to the block file.
	 *
	 * Parameters:
	 *		StateFile	Block file.
	 *		name		Name of block to create (e.g. "Main Z80"), to facilitate
	 *					multiple Z80 states in the same file.
	 */
	void SaveState(CBlockFile *StateFile, const char *name);

	/*
	 * LoadState(StateFile, name):
	 *
	 * Loads the CPU state.
	 *
	 * Parameters:
	 *		StateFile	Block file.
	 *		name		Name of block to load.
	 */
	void LoadState(CBlockFile *StateFile, const char *name);

	/*
	 * Init(BusPtr, INTF):
	 *
	 * One-time initialization of the Z80 emulator. Must be called first. 
	 *
	 * A bus object must be supplied which will be used to handle all memory 
	 * and IO accesses. The Read8, Write8, IORead8, and IOWrite8 members need 
	 * to be defined. Addresses are automatically clamped to 16 bits for memory
	 * and 8 bits for IO accesses.
	 *
	 * An interrupt callback, which is called each time an interrupt occurs,
	 * should also be supplied. The interrupt callback should explicitly clear
	 * the INT status (using SetINT(FALSE)) and then return the appropriate 
	 * vector depending on the interrupt mode that is used by the system.
	 *
	 * For mode 0, only Z80_INT_RST_* values are acceptable. For mode 1, the
	 * return value is discarded because the Z80 will always use vector 0x0038.
	 * In mode 2, an 8-bit value supplying the lower 8-bits of the interrupt
	 * vector address must be returned. Bit 0 will be ignored in this case (0
	 * is used).
	 *
	 * If a callback is not installed, the interrupt status will automatically
	 * be cleared and interrupts will not be taken when the Z80 is in a mode 
	 * that requires vector data from the callback. In mode 1, the interrupt 
	 * callback (if one is provided) should explicitly clear the interrupt. The
	 * value returned will be unused.
	 *
	 * Parameters:
	 *		BusPtr	Pointer to a bus object.
	 *		INTF	Pointer to callback function. The function accepts a
	 *				a pointer to the Z80 object that received the interrupt.
	 */
	void Init(CBus *BusPtr, int (*INTF)(CZ80 *Z80));

#ifdef SUPERMODEL_DEBUGGER
	/*
	 * AttachDebugger(DebugPtr):
	 *
	 * Attaches debugger to CPU.
	 */
	void AttachDebugger(Debugger::CZ80Debug *DebugPtr);

	/*
	 * DetachDebugger():
	 *
	 * Detaches debugger from CPU.
	 */
	void DetachDebugger();
#endif // SUPERMODEL_DEBUGGER
		
	/*
	 * CZ80(void):
	 * ~CZ80(void):
	 *
	 * Constructor and destructor.
	 */
	CZ80(void);
	~CZ80(void);

private:
	// Registers
	struct GPR {	// general purpose registers
		UINT16	bc;
		UINT16	de;
		UINT16	hl;
	} regs[2];		// two sets: primary (0) and alternate (1)
	UINT16	af[2];	// AF (primary and alternate)
	UINT16	ir;		// interrupt vector (I) and memory refresh (R)
	UINT16	ix;		// index register X
	UINT16	iy;		// index register Y
	UINT16	sp;		// stack pointer
	UINT16	pc;		// program counter
	UINT8	iff;	// interrupt flip flops (bit 1: IFF2, 0: IFF1)
	UINT8	im;		// interrupt mode (0, 1, or 2 only)
	int		regs_sel;	// active register set (primary or alternate)
	int		af_sel;		// active AF
	
	// Memory and IO bus
	CBus	*Bus;
	
	// Interrupts
	bool	nmiTrigger;
	bool	intLine;
	int		(*INTCallback)(CZ80 *Z80);

#ifdef SUPERMODEL_DEBUGGER
	int     lastCycles;

	Debugger::CZ80Debug *Debug;
#endif // SUPERMODEL_DEBUGGER
};

#endif	// INCLUDED_Z80_H