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
 * SCSPDSP.h
 * 
 * Header file for SCSP DSP emulation. Used only by SCSP.cpp; do not use
 * externally.
 */

#ifndef INCLUDED_SCSPDSP_H
#define INCLUDED_SCSPDSP_H

//#define DYNDSP
#define DYNOPT	1		//set to 1 to enable optimization of recompiler


//the DSP Context
struct _SCSPDSP
{
//Config
	UINT16 *SCSPRAM;
	unsigned int RBP;	//Ring buf pointer
	unsigned int RBL;	//Delay ram (Ring buffer) size in words

//context	
	
	INT16 COEF[64];		//16 bit signed
	UINT16 MADRS[32];	//offsets (in words), 16 bit
	UINT16 MPRO[128*4];	//128 steps 64 bit
	INT32 TEMP[128];	//TEMP regs,24 bit signed
	INT32 MEMS[32];	//MEMS regs,24 bit signed
	unsigned int DEC;

//input
	INT32 MIXS[16];	//MIXS, 24 bit signed
	INT16 EXTS[2];	//External inputs (CDDA)	16 bit signed

//output
	INT16 EFREG[16];	//EFREG, 16 bit signed
	
	bool Stopped;
	int LastStep;
#ifdef DYNDSP
	INT32 ACC;	//26 bit
	INT32 SHIFTED;	//24 bit
	INT32 X;	//24 bit
	INT32 Y;	//13 bit
	INT32 B;	//26 bit
	INT32 INPUTS;	//24 bit
	INT32 MEMVAL;
	INT32 FRC_REG;	//13 bit
	INT32 Y_REG;		//24 bit
	UINT32 ADDR;
	UINT32 ADRS_REG;	//13 bit

	void (*DoSteps)();
#endif
};

void SCSPDSP_Init(_SCSPDSP *DSP);
void SCSPDSP_SetSample(_SCSPDSP *DSP,INT32 sample,int SEL,int MXL);
void SCSPDSP_Step(_SCSPDSP *DSP);
void SCSPDSP_Start(_SCSPDSP *DSP);




#endif	// INCLUDED_SCSPDSP_H