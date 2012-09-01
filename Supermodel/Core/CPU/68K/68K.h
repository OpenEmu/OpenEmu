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
 * 68K.h
 * 
 * Header file for 68K CPU interface. Caution: 68K emulator is not thread-safe.
 *
 * TO-DO List:
 * -----------
 * - Optimize things, perhaps by using FASTCALL
 */

#ifndef INCLUDED_68K_H
#define INCLUDED_68K_H

#include <cstring>
#include "Types.h"
#include "Musashi/m68k.h"
#include "Musashi/m68kctx.h"
#include "CPU/Bus.h"

// This doesn't work for now (needs to be added to the prototypes in m68k.h for m68k_read_memory*)
//#ifndef FASTCALL
	#undef FASTCALL
	#define FASTCALL
//#endif

#ifdef SUPERMODEL_DEBUGGER
class CMusashi68KDebug;
#endif // SUPERMODEL_DEBUGGER

/******************************************************************************
 Definitions 
******************************************************************************/

// IRQ callback special return values
#define M68K_IRQ_AUTOVECTOR	M68K_INT_ACK_AUTOVECTOR	// signals an autovectored interrupt
#define M68K_IRQ_SPURIOUS	M68K_INT_ACK_SPURIOUS	// signals a spurious interrupt


/******************************************************************************
 CPU Context
******************************************************************************/

/*
 * M68KCtx:
 *
 * Complete state of a single 68K. Do NOT manipulate these directly. Set the
 * context and then use the M68K* functions below to attach a bus and IRQ
 * callback to the active context.
 */
typedef struct SM68KCtx
{
public:
	m68ki_cpu_core	musashiCtx;		// CPU context
	CBus			*Bus;			// memory handlers
	int				(*IRQAck)(int);	// IRQ acknowledge callback
#ifdef SUPERMODEL_DEBUGGER
	CMusashi68KDebug *Debug;        // holds debugger (if attached)
#endif // SUPERMODEL_DEBUGGER

	SM68KCtx(void)
	{
		Bus = NULL;
		IRQAck = NULL;
		memset(&musashiCtx, 0, sizeof(musashiCtx));	// very important! garbage in context at reset can cause very strange bugs
#ifdef SUPERMODEL_DEBUGGER
		Debug = NULL;
#endif // SUPERMODEL_DEBUGGER
	}
	
	~SM68KCtx(void)
	{
		Bus = NULL;
		IRQAck = NULL;
	}
} M68KCtx;


/******************************************************************************
 68K Interface
 
 Unless otherwise noted, all functions operate on the active context.
******************************************************************************/
	
/*
 * M68KGetARegister(n):
 *
 * Parameters:
 *		n	Register number (0-7).
 *
 * Returns:
 *		Register An.
 */
extern UINT32 M68KGetARegister(int n);

/*
 * M68KSetARegister(n, v):
 *
 * Parameters:
 *		n	Register number (0-7).
 *      v   Value to set register An to.
 */
extern void M68KSetARegister(int n, UINT32 v);

/*
 * M68KGetDRegister(n):
 *
 * Parameters:
 *		n	Register number (0-7).
 *
 * Returns:
 *		Register Dn.
 */
extern UINT32 M68KGetDRegister(int n);

/*
 * M68KGetDRegister(n, v):
 *
 * Parameters:
 *		n	Register number (0-7).
 *      v   Value to set register Dn to.
 */
extern void M68KSetDRegister(int n, UINT32 v);

/*
 * M68KGetPC():
 *
 * Returns:
 *		Current program counter.
 */
extern UINT32 M68KGetPC(void);

/*
 * M68KSaveState(StateFile, name):
 *
 * Saves the CPU state to the block file.
 *
 * Parameters:
 *		StateFile	Block file.
 *		name		Name of block to create (e.g. "Main 68K"), to facilitate
 *					multiple 68K states in the same file.
 */
extern void M68KSaveState(CBlockFile *StateFile, const char *name);

/*
 * M68KLoadState(StateFile, name):
 *
 * Loads the CPU state.
 *
 * Parameters:
 *		StateFile	Block file.
 *		name		Name of block to load.
 */
extern void M68KLoadState(CBlockFile *StateFile, const char *name);

/*
 * M68KSetIRQ(irqLevel):
 *
 * Sets the interrupt level (IPL2-IPL0 pins).
 *
 * Parameters:
 *		irqLevel	The interrupt level (1-7) or 0 to clear the interrupt.
 */
extern void M68KSetIRQ(int irqLevel);

/*
 * M68KRun(numCycles):
 *
 * Runs the 68K.
 *
 * Parameters:
 *		numCycles	Number of cycles to run.
 *
 * Returns:
 *		Number of cycles executed.
 */
extern int M68KRun(int numCycles);

/*
 * M68KReset():
 *
 * Resets the 68K.
 */
extern void M68KReset(void);

/*
 * M68KSetIRQCallback(F):
 *
 * Installs an interrupt acknowledge callback for the currently active CPU. The
 * default behavior is to always assume autovectored interrupts.
 *
 * Parameters:
 *		F	Callback function.
 */
extern void M68KSetIRQCallback(int (*F)(int));

/*
 * M68KAttachBus(CBus *BusPtr):
 *
 * Attaches a bus object to the 68K, which will be used to perform all address
 * space accesses. The 8, 16, and 32-bit read and write handlers are used.
 * This must be set up before any 68K-related emulation functions are invoked
 * or the program will crash!
 *
 * Parameters:
 *		BusPtr	Pointer to bus object to use for all address space accesses.
 */
extern void M68KAttachBus(CBus *BusPtr);

/*
 * M68KInit():
 *
 * Initializes the 68K emulator. Must be called once per program session prior
 * to any 68K emulation functions. A context must be mapped before calling
 * this.
 *
 * Returns:
 *		Always returns OKAY.
 */
extern bool M68KInit(void);

/*
 * M68KGetContext(M68KCtx *Dest):
 *
 * Copies the internal (active) 68K context back to the destination.
 *
 * Parameters:
 *		Dest	Location to which to copy 68K context.
 */
extern void M68KGetContext(M68KCtx *Dest);

/*
 * M68KSetContext(M68KCtx *Src):
 *
 * Sets the specified 68K context by copying it to the internal context.
 *
 * Parameters:
 *		Src		Location from which to copy 68K context.
 */
extern void M68KSetContext(M68KCtx *Src);

#ifdef SUPERMODEL_DEBUGGER
#define DBG68K_REG_PC 0
#define DBG68K_REG_SR 1
#define DBG68K_REG_SP 2
#define DBG68K_REG_D0 3
#define DBG68K_REG_D1 4
#define DBG68K_REG_D2 5
#define DBG68K_REG_D3 6
#define DBG68K_REG_D4 7
#define DBG68K_REG_D5 8
#define DBG68K_REG_D6 9
#define DBG68K_REG_D7 10
#define DBG68K_REG_A0 11
#define DBG68K_REG_A1 12
#define DBG68K_REG_A2 13
#define DBG68K_REG_A3 14
#define DBG68K_REG_A4 15
#define DBG68K_REG_A5 16
#define DBG68K_REG_A6 17
#define DBG68K_REG_A7 18

static int lastCycles = 0;

/*
 * M68KGetRegister(ctx, reg):
 *
 * Parameters:
 *		ctx		Musashi 68K context.
 *		reg		Register (use DBG68K_REG_* macros).
 *
 * Returns:
 *		Register value.
 */
UINT32 M68KGetRegister(SM68KCtx *ctx, unsigned reg);

/*
 * M68KSetRegister(ctx, reg, value):
 *
 * Parameters:
 *		ctx		Musashi 68K context.
 *		reg		Register (use DBG68K_REG_* macros).
 *		value	Vale to write.
 *
 * Returns:
 *		True if successful, false if register does not exist.
 */
UINT32 M68KSetRegister(SM68KCtx *ctx, unsigned reg, UINT32 val);
#endif // SUPERMODEL_DEBUGGER

/******************************************************************************
 68K Handlers
 
 Intended for use directly by the 68K core.
******************************************************************************/


extern "C" {

#ifdef SUPERMODEL_DEBUGGER
extern void M68KDebugCallback();
#endif // SUPERMODEL_DEBUGGER
	
/*
 * M68KIRQCallback(nIRQ):
 *
 * Interrupt acknowledge callback. Called when an interrupt is being serviced.
 * Actually implemented by calling a user-supplied callback.
 *
 * Parameters:
 *		nIRQ	IRQ level (1-7).
 *
 * Returns:
 *		The interrupt vector to use, M68K_IRQ_AUTOVECTOR, or
 *		M68K_IRQ_SPURIOUS. If no callback has been installed, the default
 *		callback is used and always returns M68K_IRQ_AUTOVECTOR.
 */
extern int M68KIRQCallback(int nIRQ);

/*
 * M68KFetch8(a):
 * M68KFetch16(a):
 * M68KFetch32(a):
 *
 * Read data from the program address space.
 *
 * Parameters:
 *		a		Address to read from.
 *
 * Returns:
 *		The 8, 16, or 32-bit value read.
 */
unsigned int FASTCALL M68KFetch8(unsigned int a);
unsigned int FASTCALL M68KFetch16(unsigned int a);
unsigned int FASTCALL M68KFetch32(unsigned int a);

/*
 * M68KRead8(a):
 * M68KRead16(a):
 * M68KRead32(a):
 *
 * Read data from the data address space.
 *
 * Parameters:
 *		a		Address to read from.
 *
 * Returns:
 *		The 8, 16, or 32-bit value read.
 */
unsigned int FASTCALL M68KRead8(unsigned int a);
unsigned int FASTCALL M68KRead16(unsigned int a);
unsigned int FASTCALL M68KRead32(unsigned int a);

/*
 * M68KWrite8(a, d):
 * M68KWrite16(a, d):
 * M68KWrite32(a, d):
 *
 * Writes to the data address space.
 *
 * Parameters:
 *		a		Address to write to.
 *		d		Data to write.
 */
void FASTCALL M68KWrite8(unsigned int a, unsigned int d);
void FASTCALL M68KWrite16(unsigned int a, unsigned int d);
void FASTCALL M68KWrite32(unsigned int a, unsigned int d);

}	// extern "C"


#endif	// INCLUDED_68K_H