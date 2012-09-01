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
 * SCSP.h
 * 
 * Header file defining for SCSP emulation.
 */

#ifndef INCLUDED_SCSP_H
#define INCLUDED_SCSP_H


void SCSP_w8(UINT32 addr,UINT8 val);
void SCSP_w16(UINT32 addr,UINT16 val);
void SCSP_w32(UINT32 addr,UINT32 val);
UINT8 SCSP_r8(UINT32 addr);
UINT16 SCSP_r16(UINT32 addr);
UINT32 SCSP_r32(UINT32 addr);

void SCSP_SetCB(int (*Run68k)(int cycles),void (*Int68k)(int irq));
void SCSP_Update();
void SCSP_MidiIn(UINT8);
void SCSP_MidiOutW(UINT8);
UINT8 SCSP_MidiOutFill();
UINT8 SCSP_MidiInFill();
void SCSP_CpuRunScanline();
UINT8 SCSP_MidiOutR();

/*
 * SCSP_Init(n):
 *
 * Initializes the SCSPs, allocates internal memory, and creates a mutex for 
 * MIDI FIFO access. Call SCSP_SetCB() and SCSP_SetBuffers() before calling
 * this.
 *
 * Parameters:
 *		n	Number of SCSPs to create. Always use 2! 
 *
 * Returns:
 *		FAIL if an error occured (prints own error messages), OKAY otherwise.
 */
bool SCSP_Init(int n);

void SCSP_SetRAM(int n,UINT8 *r);
void SCSP_RTECheck();
int SCSP_IRQCB(int);

void SCSP_Master_w8(UINT32 addr,UINT8 val);
void SCSP_Master_w16(UINT32 addr,UINT16 val);
void SCSP_Master_w32(UINT32 addr,UINT32 val);
void SCSP_Slave_w8(UINT32 addr,UINT8 val);
void SCSP_Slave_w16(UINT32 addr,UINT16 val);
void SCSP_Slave_w32(UINT32 addr,UINT32 val);
UINT8 SCSP_Master_r8(UINT32 addr);
UINT16 SCSP_Master_r16(UINT32 addr);
UINT32 SCSP_Master_r32(UINT32 addr);
UINT8 SCSP_Slave_r8(UINT32 addr);
UINT16 SCSP_Slave_r16(UINT32 addr);
UINT32 SCSP_Slave_r32(UINT32 addr);

// Supermodel interface functions
void SCSP_SaveState(CBlockFile *StateFile);
void SCSP_LoadState(CBlockFile *StateFile);
void SCSP_SetBuffers(INT16 *leftBufferPtr, INT16 *rightBufferPtr, int bufferLength);
void SCSP_Deinit(void);


#endif	// INCLUDED_SCSP_H