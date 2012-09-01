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
 * Z80.cpp
 *
 * Z80 instruction set simulator.
 * Copyright (C) 1995  Frank D. Cringle.
 * Adapted for use in Supermodel by Bart Trzynadlowski (July 15, 2011).
 *
 * Please see Z80.h for a discussion of known inaccuracies.
 */

#include <cstdio>	// for NULL
#include "Supermodel.h"
#include "Z80.h"	// must include this first to define CZ80


/******************************************************************************
 Internal Helper Macros
******************************************************************************/

// Address space access
#define GetBYTE(a)		( Bus->Read8(a&0xFFFF) )
#define GetBYTE_pp(a)	( Bus->Read8(((a)++)&0xFFFF) )
#define GetBYTE_mm(a)	( Bus->Read8(((a)--)&0xFFFF) )
#define mm_GetBYTE(a)	( Bus->Read8((--(a))&0xFFFF) )

#define PutBYTE(a,v)	Bus->Write8((a)&0xFFFF,v)
#define PutBYTE_pp(a,v)	Bus->Write8(((a)++)&0xFFFF,v)
#define PutBYTE_mm(a,v)	Bus->Write8(((a)--)&0xFFFF,v)
#define mm_PutBYTE(a,v)	Bus->Write8((--(a))&0xFFFF,v)

#define GetWORD(a)		(Bus->Read8((a)&0xFFFF) | (Bus->Read8(((a)+1)&0xFFFF)<<8))

#define PutWORD(a, v)				\
	do								\
	{								\
		PutBYTE((a),(v)&0xFF);		\
		PutBYTE((a)+1,((v)>>8));	\
	} while (0)

#define OUTPUT(a,v)		Bus->IOWrite8((a)&0xFF,v)
#define INPUT(a)		( Bus->IORead8((a)&0xFF) )

// Flags
#define FLAG_C	1
#define FLAG_N	2
#define FLAG_P	4
#define FLAG_H	16
#define FLAG_Z	64
#define FLAG_S	128

#define SETFLAG(f,c)	AF = (c) ? AF | FLAG_ ## f : AF & ~FLAG_ ## f
#define TSTFLAG(f)		((AF & FLAG_ ## f) != 0)
	
// Piecewise register access
#define ldig(x)		((x) & 0xf)
#define hdig(x)		(((x)>>4)&0xf)
#define lreg(x)		((x)&0xff)
#define hreg(x)		(((x)>>8)&0xff)

#define Setlreg(x, v)	x = (((x)&0xff00) | ((v)&0xff))
#define Sethreg(x, v)	x = (((x)&0xff) | (((v)&0xff) << 8))

// Parity bit calculation
static const unsigned char partab[256] = {
	4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
	0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
	0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
	4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
	0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
	4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
	4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
	0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
	0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
	4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
	4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
	0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
	4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
	0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
	0,4,4,0,4,0,0,4,4,0,0,4,0,4,4,0,
	4,0,0,4,0,4,4,0,0,4,4,0,4,0,0,4,
};

// Instruction cycle tables
static const unsigned char cycleTables[5][256] = {
	{
		// Table 0: single byte instructions
		4,10,7,6,4,4,7,4,4,11,7,6,4,4,7,4,
		8,10,7,6,4,4,7,4,12,11,7,6,4,4,7,4,
		7,10,16,6,4,4,7,4,7,11,16,6,4,4,7,4,
		7,10,13,6,11,11,10,4,7,11,13,6,4,4,7,4,
		4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
		4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
		4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
		7,7,7,7,7,7,4,7,4,4,4,4,4,4,7,4,
		4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
		4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
		4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
		4,4,4,4,4,4,7,4,4,4,4,4,4,4,7,4,
		5,10,10,10,10,11,7,11,5,10,10,0,10,17,7,11,
		5,10,10,11,10,11,7,11,5,4,10,11,10,0,7,11,
		5,10,10,19,10,11,7,11,5,4,10,4,10,0,7,11,
		5,10,10,4,10,11,7,11,5,6,10,4,10,0,7,11 
	}, {
		// Table 1: two byte instructions of form CB-XX
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,
		8,8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,
		8,8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,
		8,8,8,8,8,8,12,8,8,8,8,8,8,8,12,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8,
		8,8,8,8,8,8,15,8,8,8,8,8,8,8,15,8 
	}, {
		// Table 2: two byte instructions of form ED-XX
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		12,12,15,20,8,14,8,9,12,12,15,20,0,14,0,9,
		12,12,15,20,0,0,8,9,12,12,15,20,0,0,8,9,
		12,12,15,20,0,0,0,18,12,12,15,20,0,0,0,18,
		12,0,15,20,0,0,0,0,12,12,15,20,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		16,16,16,16,0,0,0,0,16,16,16,16,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	}, {
		// Table 3: two byte instructions of form DD-XX or FD-XX
		0,0,0,0,0,0,0,0,0,15,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,15,0,0,0,0,0,0,
		0,14,20,10,9,9,9,0,0,15,20,10,9,9,9,0,
		0,0,0,0,23,23,19,0,0,15,0,0,0,0,0,0,
		0,0,0,0,9,9,19,0,0,0,0,0,9,9,19,0,
		0,0,0,0,9,9,19,0,0,0,0,0,9,9,19,0,
		9,9,9,9,9,9,19,9,9,9,9,9,9,9,19,9,
		19,19,19,19,19,19,19,19,0,0,0,0,9,9,19,0,
		0,0,0,0,9,9,19,0,0,0,0,0,9,9,19,0,
		0,0,0,0,9,9,19,0,0,0,0,0,9,9,19,0,
		0,0,0,0,9,9,19,0,0,0,0,0,9,9,19,0,
		0,0,0,0,9,9,19,0,0,0,0,0,9,9,19,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,14,0,23,0,15,0,0,0,8,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,10,0,0,0,0,0,0
	}, {
		// Table 4: three byte instructions of form DD-CB-XX or FD-CB-XX
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
		20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
		20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
		20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0,
		0,0,0,0,0,0,23,0,0,0,0,0,0,0,23,0
	}
};

#define parity(x)	partab[(x)&0xff]

// Stack
#define POP(x)								\
	do										\
	{										\
		unsigned int y = GetBYTE_pp(SP);	\
		x = y + (GetBYTE_pp(SP) << 8);		\
	} while (0)

#define PUSH(x) 					\
	do 								\
	{								\
		mm_PutBYTE(SP,((x)>>8));	\
		mm_PutBYTE(SP,(x)&0xFF);	\
	} while (0)

// Branching
#define Jpc(cond) pc = cond ? GetWORD(pc) : pc+2

#define CALLC(cond) 							\
	{											\
		if (cond) 								\
		{										\
			unsigned int adrr = GetWORD(pc);	\
			PUSH(pc+2);							\
			pc = adrr;							\
		}										\
		else									\
			pc += 2;							\
	}
	

/*******************************************************************************
 Functions
*******************************************************************************/

int CZ80::Run(int numCycles)
{
#ifdef SUPERMODEL_DEBUGGER
	// If debugging enabled, don't optimize access to registers as they need to be accesible to debugger during execution
#define AF af[af_sel]
#define BC regs[regs_sel].bc
#define DE regs[regs_sel].de
#define HL regs[regs_sel].hl
#define SP sp
#define IX ix
#define IY iy
#else
    // Optimization: copy registers into native word-sized local variables
    unsigned int AF = af[af_sel];
    unsigned int BC = regs[regs_sel].bc;
    unsigned int DE = regs[regs_sel].de;
    unsigned int HL = regs[regs_sel].hl;
    unsigned int SP = sp;
    unsigned int IX = ix;
    unsigned int IY = iy;
#endif 
    unsigned int temp, acu, sum, cbits;
    unsigned int op, adr;

	int cycles = numCycles;
#ifdef SUPERMODEL_DEBUGGER
	if (Debug != NULL)
	{
		Debug->CPUActive();
		lastCycles += numCycles;
	}
#endif // SUPERMODEL_DEBUGGER

	while (cycles > 0)
	{
	op = GetBYTE_pp(pc);
#ifdef SUPERMODEL_DEBUGGER
	if (Debug != NULL)
	{
		while (Debug->CPUExecute(pc - 1, op, lastCycles - cycles))
			op = GetBYTE_pp(pc);
		lastCycles = cycles;
	}
#endif // SUPERMODEL_DEBUGGER
	switch(op) {
	case 0x00:			/* NOP */
		cycles -= cycleTables[0][0x00];
		break;
	case 0x01:			/* LD BC,nnnn */
		cycles -= cycleTables[0][0x01];
		BC = GetWORD(pc);
		pc += 2;
		break;
	case 0x02:			/* LD (BC),A */
		cycles -= cycleTables[0][0x02];
		PutBYTE(BC, hreg(AF));
		break;
	case 0x03:			/* INC BC */
		cycles -= cycleTables[0][0x03];
		++BC;
		break;
	case 0x04:			/* INC B */
		cycles -= cycleTables[0][0x04];
		BC += 0x100;
		temp = hreg(BC);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0) << 4) |
			((temp == 0x80) << 2);
		break;
	case 0x05:			/* DEC B */
		cycles -= cycleTables[0][0x05];
		BC -= 0x100;
		temp = hreg(BC);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0xf) << 4) |
			((temp == 0x7f) << 2) | 2;
		break;
	case 0x06:			/* LD B,nn */
		cycles -= cycleTables[0][0x06];
		Sethreg(BC, GetBYTE_pp(pc));
		break;
	case 0x07:			/* RLCA */
		cycles -= cycleTables[0][0x07];
		AF = ((AF >> 7) & 0x0128) | ((AF << 1) & ~0x1ff) |
			(AF & 0xc4) | ((AF >> 15) & 1);
		break;
	case 0x08:			/* EX AF,AF' */
		cycles -= cycleTables[0][0x08];
		af[af_sel] = AF;
		af_sel = 1 - af_sel;
		AF = af[af_sel];
		break;
	case 0x09:			/* ADD HL,BC */
		cycles -= cycleTables[0][0x09];
		HL &= 0xffff;
		BC &= 0xffff;
		sum = HL + BC;
		cbits = (HL ^ BC ^ sum) >> 8;
		HL = sum;
		AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0x0A:			/* LD A,(BC) */
		cycles -= cycleTables[0][0x0A];
		Sethreg(AF, GetBYTE(BC));
		break;
	case 0x0B:			/* DEC BC */
		cycles -= cycleTables[0][0x0B];
		--BC;
		break;
	case 0x0C:			/* INC C */
		cycles -= cycleTables[0][0x0C];
		temp = lreg(BC)+1;
		Setlreg(BC, temp);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0) << 4) |
			((temp == 0x80) << 2);
		break;
	case 0x0D:			/* DEC C */
		cycles -= cycleTables[0][0x0D];
		temp = lreg(BC)-1;
		Setlreg(BC, temp);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0xf) << 4) |
			((temp == 0x7f) << 2) | 2;
		break;
	case 0x0E:			/* LD C,nn */
		cycles -= cycleTables[0][0x0E];
		Setlreg(BC, GetBYTE_pp(pc));
		break;
	case 0x0F:			/* RRCA */
		cycles -= cycleTables[0][0x0F];
		temp = hreg(AF);
		sum = temp >> 1;
		AF = ((temp & 1) << 15) | (sum << 8) |
			(sum & 0x28) | (AF & 0xc4) | (temp & 1);
		break;
	case 0x10:			/* DJNZ dd */
		cycles -= cycleTables[0][0x10];
		pc += ((BC -= 0x100) & 0xff00) ? (signed char) GetBYTE(pc) + 1 : 1;
		break;
	case 0x11:			/* LD DE,nnnn */
		cycles -= cycleTables[0][0x11];
		DE = GetWORD(pc);
		pc += 2;
		break;
	case 0x12:			/* LD (DE),A */
		cycles -= cycleTables[0][0x12];
		PutBYTE(DE, hreg(AF));
		break;
	case 0x13:			/* INC DE */
		cycles -= cycleTables[0][0x13];
		++DE;
		break;
	case 0x14:			/* INC D */
		cycles -= cycleTables[0][0x14];
		DE += 0x100;
		temp = hreg(DE);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0) << 4) |
			((temp == 0x80) << 2);
		break;
	case 0x15:			/* DEC D */
		cycles -= cycleTables[0][0x15];
		DE -= 0x100;
		temp = hreg(DE);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0xf) << 4) |
			((temp == 0x7f) << 2) | 2;
		break;
	case 0x16:			/* LD D,nn */
		cycles -= cycleTables[0][0x16];
		Sethreg(DE, GetBYTE_pp(pc));
		break;
	case 0x17:			/* RLA */
		cycles -= cycleTables[0][0x17];
		AF = ((AF << 8) & 0x0100) | ((AF >> 7) & 0x28) | ((AF << 1) & ~0x01ff) |
			(AF & 0xc4) | ((AF >> 15) & 1);
		break;
	case 0x18:			/* JR dd */
		cycles -= cycleTables[0][0x18];
		pc += (1) ? (signed char) GetBYTE(pc) + 1 : 1;
		break;
	case 0x19:			/* ADD HL,DE */
		cycles -= cycleTables[0][0x19];
		HL &= 0xffff;
		DE &= 0xffff;
		sum = HL + DE;
		cbits = (HL ^ DE ^ sum) >> 8;
		HL = sum;
		AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0x1A:			/* LD A,(DE) */
		cycles -= cycleTables[0][0x1A];
		Sethreg(AF, GetBYTE(DE));
		break;
	case 0x1B:			/* DEC DE */
		cycles -= cycleTables[0][0x1B];
		--DE;
		break;
	case 0x1C:			/* INC E */
		cycles -= cycleTables[0][0x1C];
		temp = lreg(DE)+1;
		Setlreg(DE, temp);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0) << 4) |
			((temp == 0x80) << 2);
		break;
	case 0x1D:			/* DEC E */
		cycles -= cycleTables[0][0x1D];
		temp = lreg(DE)-1;
		Setlreg(DE, temp);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0xf) << 4) |
			((temp == 0x7f) << 2) | 2;
		break;
	case 0x1E:			/* LD E,nn */
		cycles -= cycleTables[0][0x1E];
		Setlreg(DE, GetBYTE_pp(pc));
		break;
	case 0x1F:			/* RRA */
		cycles -= cycleTables[0][0x1F];
		temp = hreg(AF);
		sum = temp >> 1;
		AF = ((AF & 1) << 15) | (sum << 8) |
			(sum & 0x28) | (AF & 0xc4) | (temp & 1);
		break;
	case 0x20:			/* JR NZ,dd */
		cycles -= cycleTables[0][0x20];
		pc += (!TSTFLAG(Z)) ? (signed char) GetBYTE(pc) + 1 : 1;
		break;
	case 0x21:			/* LD HL,nnnn */
		cycles -= cycleTables[0][0x21];
		HL = GetWORD(pc);
		pc += 2;
		break;
	case 0x22:			/* LD (nnnn),HL */
		cycles -= cycleTables[0][0x22];
		temp = GetWORD(pc);
		PutWORD(temp, HL);
		pc += 2;
		break;
	case 0x23:			/* INC HL */
		cycles -= cycleTables[0][0x23];
		++HL;
		break;
	case 0x24:			/* INC H */
		cycles -= cycleTables[0][0x24];
		HL += 0x100;
		temp = hreg(HL);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0) << 4) |
			((temp == 0x80) << 2);
		break;
	case 0x25:			/* DEC H */
		cycles -= cycleTables[0][0x25];
		HL -= 0x100;
		temp = hreg(HL);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0xf) << 4) |
			((temp == 0x7f) << 2) | 2;
		break;
	case 0x26:			/* LD H,nn */
		cycles -= cycleTables[0][0x26];
		Sethreg(HL, GetBYTE_pp(pc));
		break;
	case 0x27:			/* DAA */
		cycles -= cycleTables[0][0x27];
		acu = hreg(AF);
		temp = ldig(acu);
		cbits = TSTFLAG(C);
		if (TSTFLAG(N)) {	/* last operation was a subtract */
			int hd = cbits || acu > 0x99;
			if (TSTFLAG(H) || (temp > 9)) { /* adjust low digit */
				if (temp > 5)
					SETFLAG(H, 0);
				acu -= 6;
				acu &= 0xff;
			}
			if (hd)		/* adjust high digit */
				acu -= 0x160;
		}
		else {			/* last operation was an add */
			if (TSTFLAG(H) || (temp > 9)) { /* adjust low digit */
				SETFLAG(H, (temp > 9));
				acu += 6;
			}
			if (cbits || ((acu & 0x1f0) > 0x90)) /* adjust high digit */
				acu += 0x60;
		}
		cbits |= (acu >> 8) & 1;
		acu &= 0xff;
		AF = (acu << 8) | (acu & 0xa8) | ((acu == 0) << 6) |
			(AF & 0x12) | partab[acu] | cbits;
		break;
	case 0x28:			/* JR Z,dd */
		cycles -= cycleTables[0][0x28];
		pc += (TSTFLAG(Z)) ? (signed char) GetBYTE(pc) + 1 : 1;
		break;
	case 0x29:			/* ADD HL,HL */
		cycles -= cycleTables[0][0x29];
		HL &= 0xffff;
		sum = HL + HL;
		cbits = (HL ^ HL ^ sum) >> 8;
		HL = sum;
		AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0x2A:			/* LD HL,(nnnn) */
		cycles -= cycleTables[0][0x2A];
		temp = GetWORD(pc);
		HL = GetWORD(temp);
		pc += 2;
		break;
	case 0x2B:			/* DEC HL */
		cycles -= cycleTables[0][0x2B];
		--HL;
		break;
	case 0x2C:			/* INC L */
		cycles -= cycleTables[0][0x2C];
		temp = lreg(HL)+1;
		Setlreg(HL, temp);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0) << 4) |
			((temp == 0x80) << 2);
		break;
	case 0x2D:			/* DEC L */
		cycles -= cycleTables[0][0x2D];
		temp = lreg(HL)-1;
		Setlreg(HL, temp);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0xf) << 4) |
			((temp == 0x7f) << 2) | 2;
		break;
	case 0x2E:			/* LD L,nn */
		cycles -= cycleTables[0][0x2E];
		Setlreg(HL, GetBYTE_pp(pc));
		break;
	case 0x2F:			/* CPL */
		cycles -= cycleTables[0][0x2F];
		AF = (~AF & ~0xff) | (AF & 0xc5) | ((~AF >> 8) & 0x28) | 0x12;
		break;
	case 0x30:			/* JR NC,dd */
		cycles -= cycleTables[0][0x30];
		pc += (!TSTFLAG(C)) ? (signed char) GetBYTE(pc) + 1 : 1;
		break;
	case 0x31:			/* LD SP,nnnn */
		cycles -= cycleTables[0][0x31];
		SP = GetWORD(pc);
		pc += 2;
		break;
	case 0x32:			/* LD (nnnn),A */
		cycles -= cycleTables[0][0x32];
		temp = GetWORD(pc);
		PutBYTE(temp, hreg(AF));
		pc += 2;
		break;
	case 0x33:			/* INC SP */
		cycles -= cycleTables[0][0x33];
		++SP;
		break;
	case 0x34:			/* INC (HL) */
		cycles -= cycleTables[0][0x34];
		temp = GetBYTE(HL)+1;
		PutBYTE(HL, temp);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0) << 4) |
			((temp == 0x80) << 2);
		break;
	case 0x35:			/* DEC (HL) */
		cycles -= cycleTables[0][0x35];
		temp = GetBYTE(HL)-1;
		PutBYTE(HL, temp);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0xf) << 4) |
			((temp == 0x7f) << 2) | 2;
		break;
	case 0x36:			/* LD (HL),nn */
		cycles -= cycleTables[0][0x36];
		PutBYTE(HL, GetBYTE_pp(pc));
		break;
	case 0x37:			/* SCF */
		cycles -= cycleTables[0][0x37];
		AF = (AF&~0x3b)|((AF>>8)&0x28)|1;
		break;
	case 0x38:			/* JR C,dd */
		cycles -= cycleTables[0][0x38];
		pc += (TSTFLAG(C)) ? (signed char) GetBYTE(pc) + 1 : 1;
		break;
	case 0x39:			/* ADD HL,SP */
		cycles -= cycleTables[0][0x39];
		HL &= 0xffff;
		SP &= 0xffff;
		sum = HL + SP;
		cbits = (HL ^ SP ^ sum) >> 8;
		HL = sum;
		AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0x3A:			/* LD A,(nnnn) */
		cycles -= cycleTables[0][0x3A];
		temp = GetWORD(pc);
		Sethreg(AF, GetBYTE(temp));
		pc += 2;
		break;
	case 0x3B:			/* DEC SP */
		cycles -= cycleTables[0][0x3B];
		--SP;
		break;
	case 0x3C:			/* INC A */
		cycles -= cycleTables[0][0x3C];
		AF += 0x100;
		temp = hreg(AF);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0) << 4) |
			((temp == 0x80) << 2);
		break;
	case 0x3D:			/* DEC A */
		cycles -= cycleTables[0][0x3D];
		AF -= 0x100;
		temp = hreg(AF);
		AF = (AF & ~0xfe) | (temp & 0xa8) |
			(((temp & 0xff) == 0) << 6) |
			(((temp & 0xf) == 0xf) << 4) |
			((temp == 0x7f) << 2) | 2;
		break;
	case 0x3E:			/* LD A,nn */
		cycles -= cycleTables[0][0x3E];
		Sethreg(AF, GetBYTE_pp(pc));
		break;
	case 0x3F:			/* CCF */
		cycles -= cycleTables[0][0x3F];
		AF = (AF&~0x3b)|((AF>>8)&0x28)|((AF&1)<<4)|(~AF&1);
		break;
	case 0x40:			/* LD B,B */
		cycles -= cycleTables[0][0x40];
		/* nop */
		break;
	case 0x41:			/* LD B,C */
		cycles -= cycleTables[0][0x41];
		BC = (BC & 255) | ((BC & 255) << 8);
		break;
	case 0x42:			/* LD B,D */
		cycles -= cycleTables[0][0x42];
		BC = (BC & 255) | (DE & ~255);
		break;
	case 0x43:			/* LD B,E */
		cycles -= cycleTables[0][0x43];
		BC = (BC & 255) | ((DE & 255) << 8);
		break;
	case 0x44:			/* LD B,H */
		cycles -= cycleTables[0][0x44];
		BC = (BC & 255) | (HL & ~255);
		break;
	case 0x45:			/* LD B,L */
		cycles -= cycleTables[0][0x45];
		BC = (BC & 255) | ((HL & 255) << 8);
		break;
	case 0x46:			/* LD B,(HL) */
		cycles -= cycleTables[0][0x46];
		Sethreg(BC, GetBYTE(HL));
		break;
	case 0x47:			/* LD B,A */
		cycles -= cycleTables[0][0x47];
		BC = (BC & 255) | (AF & ~255);
		break;
	case 0x48:			/* LD C,B */
		cycles -= cycleTables[0][0x48];
		BC = (BC & ~255) | ((BC >> 8) & 255);
		break;
	case 0x49:			/* LD C,C */
		cycles -= cycleTables[0][0x49];
		/* nop */
		break;
	case 0x4A:			/* LD C,D */
		cycles -= cycleTables[0][0x4A];
		BC = (BC & ~255) | ((DE >> 8) & 255);
		break;
	case 0x4B:			/* LD C,E */
		cycles -= cycleTables[0][0x4B];
		BC = (BC & ~255) | (DE & 255);
		break;
	case 0x4C:			/* LD C,H */
		cycles -= cycleTables[0][0x4C];
		BC = (BC & ~255) | ((HL >> 8) & 255);
		break;
	case 0x4D:			/* LD C,L */
		cycles -= cycleTables[0][0x4D];
		BC = (BC & ~255) | (HL & 255);
		break;
	case 0x4E:			/* LD C,(HL) */
		cycles -= cycleTables[0][0x4E];
		Setlreg(BC, GetBYTE(HL));
		break;
	case 0x4F:			/* LD C,A */
		cycles -= cycleTables[0][0x4F];
		BC = (BC & ~255) | ((AF >> 8) & 255);
		break;
	case 0x50:			/* LD D,B */
		cycles -= cycleTables[0][0x50];
		DE = (DE & 255) | (BC & ~255);
		break;
	case 0x51:			/* LD D,C */
		cycles -= cycleTables[0][0x51];
		DE = (DE & 255) | ((BC & 255) << 8);
		break;
	case 0x52:			/* LD D,D */
		cycles -= cycleTables[0][0x52];
		/* nop */
		break;
	case 0x53:			/* LD D,E */
		cycles -= cycleTables[0][0x53];
		DE = (DE & 255) | ((DE & 255) << 8);
		break;
	case 0x54:			/* LD D,H */
		cycles -= cycleTables[0][0x54];
		DE = (DE & 255) | (HL & ~255);
		break;
	case 0x55:			/* LD D,L */
		cycles -= cycleTables[0][0x55];
		DE = (DE & 255) | ((HL & 255) << 8);
		break;
	case 0x56:			/* LD D,(HL) */
		cycles -= cycleTables[0][0x56];
		Sethreg(DE, GetBYTE(HL));
		break;
	case 0x57:			/* LD D,A */
		cycles -= cycleTables[0][0x57];
		DE = (DE & 255) | (AF & ~255);
		break;
	case 0x58:			/* LD E,B */
		cycles -= cycleTables[0][0x58];
		DE = (DE & ~255) | ((BC >> 8) & 255);
		break;
	case 0x59:			/* LD E,C */
		cycles -= cycleTables[0][0x59];
		DE = (DE & ~255) | (BC & 255);
		break;
	case 0x5A:			/* LD E,D */
		cycles -= cycleTables[0][0x5A];
		DE = (DE & ~255) | ((DE >> 8) & 255);
		break;
	case 0x5B:			/* LD E,E */
		cycles -= cycleTables[0][0x5B];
		/* nop */
		break;
	case 0x5C:			/* LD E,H */
		cycles -= cycleTables[0][0x5C];
		DE = (DE & ~255) | ((HL >> 8) & 255);
		break;
	case 0x5D:			/* LD E,L */
		cycles -= cycleTables[0][0x5D];
		DE = (DE & ~255) | (HL & 255);
		break;
	case 0x5E:			/* LD E,(HL) */
		cycles -= cycleTables[0][0x5E];
		Setlreg(DE, GetBYTE(HL));
		break;
	case 0x5F:			/* LD E,A */
		cycles -= cycleTables[0][0x5F];
		DE = (DE & ~255) | ((AF >> 8) & 255);
		break;
	case 0x60:			/* LD H,B */
		cycles -= cycleTables[0][0x60];
		HL = (HL & 255) | (BC & ~255);
		break;
	case 0x61:			/* LD H,C */
		cycles -= cycleTables[0][0x61];
		HL = (HL & 255) | ((BC & 255) << 8);
		break;
	case 0x62:			/* LD H,D */
		cycles -= cycleTables[0][0x62];
		HL = (HL & 255) | (DE & ~255);
		break;
	case 0x63:			/* LD H,E */
		cycles -= cycleTables[0][0x63];
		HL = (HL & 255) | ((DE & 255) << 8);
		break;
	case 0x64:			/* LD H,H */
		cycles -= cycleTables[0][0x64];
		/* nop */
		break;
	case 0x65:			/* LD H,L */
		cycles -= cycleTables[0][0x65];
		HL = (HL & 255) | ((HL & 255) << 8);
		break;
	case 0x66:			/* LD H,(HL) */
		cycles -= cycleTables[0][0x66];
		Sethreg(HL, GetBYTE(HL));
		break;
	case 0x67:			/* LD H,A */
		cycles -= cycleTables[0][0x67];
		HL = (HL & 255) | (AF & ~255);
		break;
	case 0x68:			/* LD L,B */
		cycles -= cycleTables[0][0x68];
		HL = (HL & ~255) | ((BC >> 8) & 255);
		break;
	case 0x69:			/* LD L,C */
		cycles -= cycleTables[0][0x69];
		HL = (HL & ~255) | (BC & 255);
		break;
	case 0x6A:			/* LD L,D */
		cycles -= cycleTables[0][0x6A];
		HL = (HL & ~255) | ((DE >> 8) & 255);
		break;
	case 0x6B:			/* LD L,E */
		cycles -= cycleTables[0][0x6B];
		HL = (HL & ~255) | (DE & 255);
		break;
	case 0x6C:			/* LD L,H */
		cycles -= cycleTables[0][0x6C];
		HL = (HL & ~255) | ((HL >> 8) & 255);
		break;
	case 0x6D:			/* LD L,L */
		cycles -= cycleTables[0][0x6D];
		/* nop */
		break;
	case 0x6E:			/* LD L,(HL) */
		cycles -= cycleTables[0][0x6E];
		Setlreg(HL, GetBYTE(HL));
		break;
	case 0x6F:			/* LD L,A */
		cycles -= cycleTables[0][0x6F];
		HL = (HL & ~255) | ((AF >> 8) & 255);
		break;
	case 0x70:			/* LD (HL),B */
		cycles -= cycleTables[0][0x70];
		PutBYTE(HL, hreg(BC));
		break;
	case 0x71:			/* LD (HL),C */
		cycles -= cycleTables[0][0x71];
		PutBYTE(HL, lreg(BC));
		break;
	case 0x72:			/* LD (HL),D */
		cycles -= cycleTables[0][0x72];
		PutBYTE(HL, hreg(DE));
		break;
	case 0x73:			/* LD (HL),E */
		cycles -= cycleTables[0][0x73];
		PutBYTE(HL, lreg(DE));
		break;
	case 0x74:			/* LD (HL),H */
		cycles -= cycleTables[0][0x74];
		PutBYTE(HL, hreg(HL));
		break;
	case 0x75:			/* LD (HL),L */
		cycles -= cycleTables[0][0x75];
		PutBYTE(HL, lreg(HL));
		break;
	case 0x76:			/* HALT */
		cycles -= cycleTables[0][0x76];
//		ErrorLog("Z80 encountered an unemulated instruction at 0x%04X", (pc-1)&0xFFFF);
		goto HALTExit;
	case 0x77:			/* LD (HL),A */
		cycles -= cycleTables[0][0x77];
		PutBYTE(HL, hreg(AF));
		break;
	case 0x78:			/* LD A,B */
		cycles -= cycleTables[0][0x78];
		AF = (AF & 255) | (BC & ~255);
		break;
	case 0x79:			/* LD A,C */
		cycles -= cycleTables[0][0x79];
		AF = (AF & 255) | ((BC & 255) << 8);
		break;
	case 0x7A:			/* LD A,D */
		cycles -= cycleTables[0][0x7A];
		AF = (AF & 255) | (DE & ~255);
		break;
	case 0x7B:			/* LD A,E */
		cycles -= cycleTables[0][0x7B];
		AF = (AF & 255) | ((DE & 255) << 8);
		break;
	case 0x7C:			/* LD A,H */
		cycles -= cycleTables[0][0x7C];
		AF = (AF & 255) | (HL & ~255);
		break;
	case 0x7D:			/* LD A,L */
		cycles -= cycleTables[0][0x7D];
		AF = (AF & 255) | ((HL & 255) << 8);
		break;
	case 0x7E:			/* LD A,(HL) */
		cycles -= cycleTables[0][0x7E];
		Sethreg(AF, GetBYTE(HL));
		break;
	case 0x7F:			/* LD A,A */
		cycles -= cycleTables[0][0x7F];
		/* nop */
		break;
	case 0x80:			/* ADD A,B */
		cycles -= cycleTables[0][0x80];
		temp = hreg(BC);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x81:			/* ADD A,C */
		cycles -= cycleTables[0][0x81];
		temp = lreg(BC);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x82:			/* ADD A,D */
		cycles -= cycleTables[0][0x82];
		temp = hreg(DE);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x83:			/* ADD A,E */
		cycles -= cycleTables[0][0x83];
		temp = lreg(DE);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x84:			/* ADD A,H */
		cycles -= cycleTables[0][0x84];
		temp = hreg(HL);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x85:			/* ADD A,L */
		cycles -= cycleTables[0][0x85];
		temp = lreg(HL);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x86:			/* ADD A,(HL) */
		cycles -= cycleTables[0][0x86];
		temp = GetBYTE(HL);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x87:			/* ADD A,A */
		cycles -= cycleTables[0][0x87];
		temp = hreg(AF);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x88:			/* ADC A,B */
		cycles -= cycleTables[0][0x88];
		temp = hreg(BC);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x89:			/* ADC A,C */
		cycles -= cycleTables[0][0x89];
		temp = lreg(BC);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x8A:			/* ADC A,D */
		cycles -= cycleTables[0][0x8A];
		temp = hreg(DE);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x8B:			/* ADC A,E */
		cycles -= cycleTables[0][0x8B];
		temp = lreg(DE);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x8C:			/* ADC A,H */
		cycles -= cycleTables[0][0x8C];
		temp = hreg(HL);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x8D:			/* ADC A,L */
		cycles -= cycleTables[0][0x8D];
		temp = lreg(HL);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x8E:			/* ADC A,(HL) */
		cycles -= cycleTables[0][0x8E];
		temp = GetBYTE(HL);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x8F:			/* ADC A,A */
		cycles -= cycleTables[0][0x8F];
		temp = hreg(AF);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0x90:			/* SUB B */
		cycles -= cycleTables[0][0x90];
		temp = hreg(BC);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x91:			/* SUB C */
		cycles -= cycleTables[0][0x91];
		temp = lreg(BC);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x92:			/* SUB D */
		cycles -= cycleTables[0][0x92];
		temp = hreg(DE);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x93:			/* SUB E */
		cycles -= cycleTables[0][0x93];
		temp = lreg(DE);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x94:			/* SUB H */
		cycles -= cycleTables[0][0x94];
		temp = hreg(HL);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x95:			/* SUB L */
		cycles -= cycleTables[0][0x95];
		temp = lreg(HL);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x96:			/* SUB (HL) */
		cycles -= cycleTables[0][0x96];
		temp = GetBYTE(HL);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x97:			/* SUB A */
		cycles -= cycleTables[0][0x97];
		temp = hreg(AF);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x98:			/* SBC A,B */
		cycles -= cycleTables[0][0x98];
		temp = hreg(BC);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x99:			/* SBC A,C */
		cycles -= cycleTables[0][0x99];
		temp = lreg(BC);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x9A:			/* SBC A,D */
		cycles -= cycleTables[0][0x9A];
		temp = hreg(DE);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x9B:			/* SBC A,E */
		cycles -= cycleTables[0][0x9B];
		temp = lreg(DE);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x9C:			/* SBC A,H */
		cycles -= cycleTables[0][0x9C];
		temp = hreg(HL);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x9D:			/* SBC A,L */
		cycles -= cycleTables[0][0x9D];
		temp = lreg(HL);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x9E:			/* SBC A,(HL) */
		cycles -= cycleTables[0][0x9E];
		temp = GetBYTE(HL);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0x9F:			/* SBC A,A */
		cycles -= cycleTables[0][0x9F];
		temp = hreg(AF);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0xA0:			/* AND B */
		cycles -= cycleTables[0][0xA0];
		sum = ((AF & (BC)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) |
			((sum == 0) << 6) | 0x10 | partab[sum];
		break;
	case 0xA1:			/* AND C */
		cycles -= cycleTables[0][0xA1];
		sum = ((AF >> 8) & BC) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | 0x10 |
			((sum == 0) << 6) | partab[sum];
		break;
	case 0xA2:			/* AND D */
		cycles -= cycleTables[0][0xA2];
		sum = ((AF & (DE)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) |
			((sum == 0) << 6) | 0x10 | partab[sum];
		break;
	case 0xA3:			/* AND E */
		cycles -= cycleTables[0][0xA3];
		sum = ((AF >> 8) & DE) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | 0x10 |
			((sum == 0) << 6) | partab[sum];
		break;
	case 0xA4:			/* AND H */
		cycles -= cycleTables[0][0xA4];
		sum = ((AF & (HL)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) |
			((sum == 0) << 6) | 0x10 | partab[sum];
		break;
	case 0xA5:			/* AND L */
		cycles -= cycleTables[0][0xA5];
		sum = ((AF >> 8) & HL) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | 0x10 |
			((sum == 0) << 6) | partab[sum];
		break;
	case 0xA6:			/* AND (HL) */
		cycles -= cycleTables[0][0xA6];
		sum = ((AF >> 8) & GetBYTE(HL)) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | 0x10 |
			((sum == 0) << 6) | partab[sum];
		break;
	case 0xA7:			/* AND A */
		cycles -= cycleTables[0][0xA7];
		sum = ((AF & (AF)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) |
			((sum == 0) << 6) | 0x10 | partab[sum];
		break;
	case 0xA8:			/* XOR B */
		cycles -= cycleTables[0][0xA8];
		sum = ((AF ^ (BC)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xA9:			/* XOR C */
		cycles -= cycleTables[0][0xA9];
		sum = ((AF >> 8) ^ BC) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xAA:			/* XOR D */
		cycles -= cycleTables[0][0xAA];
		sum = ((AF ^ (DE)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xAB:			/* XOR E */
		cycles -= cycleTables[0][0xAB];
		sum = ((AF >> 8) ^ DE) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xAC:			/* XOR H */
		cycles -= cycleTables[0][0xAC];
		sum = ((AF ^ (HL)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xAD:			/* XOR L */
		cycles -= cycleTables[0][0xAD];
		sum = ((AF >> 8) ^ HL) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xAE:			/* XOR (HL) */
		cycles -= cycleTables[0][0xAE];
		sum = ((AF >> 8) ^ GetBYTE(HL)) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xAF:			/* XOR A */
		cycles -= cycleTables[0][0xAF];
		sum = ((AF ^ (AF)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB0:			/* OR B */
		cycles -= cycleTables[0][0xB0];
		sum = ((AF | (BC)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB1:			/* OR C */
		cycles -= cycleTables[0][0xB1];
		sum = ((AF >> 8) | BC) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB2:			/* OR D */
		cycles -= cycleTables[0][0xB2];
		sum = ((AF | (DE)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB3:			/* OR E */
		cycles -= cycleTables[0][0xB3];
		sum = ((AF >> 8) | DE) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB4:			/* OR H */
		cycles -= cycleTables[0][0xB4];
		sum = ((AF | (HL)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB5:			/* OR L */
		cycles -= cycleTables[0][0xB5];
		sum = ((AF >> 8) | HL) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB6:			/* OR (HL) */
		cycles -= cycleTables[0][0xB6];
		sum = ((AF >> 8) | GetBYTE(HL)) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB7:			/* OR A */
		cycles -= cycleTables[0][0xB7];
		sum = ((AF | (AF)) >> 8) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xB8:			/* CP B */
		cycles -= cycleTables[0][0xB8];
		temp = hreg(BC);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xB9:			/* CP C */
		cycles -= cycleTables[0][0xB9];
		temp = lreg(BC);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xBA:			/* CP D */
		cycles -= cycleTables[0][0xBA];
		temp = hreg(DE);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xBB:			/* CP E */
		cycles -= cycleTables[0][0xBB];
		temp = lreg(DE);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xBC:			/* CP H */
		cycles -= cycleTables[0][0xBC];
		temp = hreg(HL);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xBD:			/* CP L */
		cycles -= cycleTables[0][0xBD];
		temp = lreg(HL);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xBE:			/* CP (HL) */
		cycles -= cycleTables[0][0xBE];
		temp = GetBYTE(HL);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xBF:			/* CP A */
		cycles -= cycleTables[0][0xBF];
		temp = hreg(AF);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xC0:			/* RET NZ */
		cycles -= cycleTables[0][0xC0];
		if (!TSTFLAG(Z)) POP(pc);
		break;
	case 0xC1:			/* POP BC */
		cycles -= cycleTables[0][0xC1];
		POP(BC);
		break;
	case 0xC2:			/* JP NZ,nnnn */
		cycles -= cycleTables[0][0xC2];
		Jpc(!TSTFLAG(Z));
		break;
	case 0xC3:			/* JP nnnn */
		cycles -= cycleTables[0][0xC3];
		Jpc(1);
		break;
	case 0xC4:			/* CALL NZ,nnnn */
		cycles -= cycleTables[0][0xC4];
		CALLC(!TSTFLAG(Z));
		break;
	case 0xC5:			/* PUSH BC */
		cycles -= cycleTables[0][0xC5];
		PUSH(BC);
		break;
	case 0xC6:			/* ADD A,nn */
		cycles -= cycleTables[0][0xC6];
		temp = GetBYTE_pp(pc);
		acu = hreg(AF);
		sum = acu + temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0xC7:			/* RST 0 */
		cycles -= cycleTables[0][0xC7];
		PUSH(pc); pc = 0;
		break;
	case 0xC8:			/* RET Z */
		cycles -= cycleTables[0][0xC8];
		if (TSTFLAG(Z)) POP(pc);
		break;
	case 0xC9:			/* RET */
		cycles -= cycleTables[0][0xC9];
		POP(pc);
		break;
	case 0xCA:			/* JP Z,nnnn */
		cycles -= cycleTables[0][0xCA];
		Jpc(TSTFLAG(Z));
		break;
	case 0xCB:			/* CB prefix */
		adr = HL;
		op = GetBYTE(pc);
		cycles -= cycleTables[1][op];
		switch (op & 7) {
		case 0: ++pc; acu = hreg(BC); break;
		case 1: ++pc; acu = lreg(BC); break;
		case 2: ++pc; acu = hreg(DE); break;
		case 3: ++pc; acu = lreg(DE); break;
		case 4: ++pc; acu = hreg(HL); break;
		case 5: ++pc; acu = lreg(HL); break;
		case 6: ++pc; acu = GetBYTE(adr);  break;
		case 7: ++pc; acu = hreg(AF); break;
		}
		switch (op & 0xc0) {
		case 0x00:		/* shift/rotate */
			switch (op & 0x38) {
			case 0x00:	/* RLC */
				temp = (acu << 1) | (acu >> 7);
				cbits = temp & 1;
				goto cbshflg1;
			case 0x08:	/* RRC */
				temp = (acu >> 1) | (acu << 7);
				cbits = temp & 0x80;
				goto cbshflg1;
			case 0x10:	/* RL */
				temp = (acu << 1) | TSTFLAG(C);
				cbits = acu & 0x80;
				goto cbshflg1;
			case 0x18:	/* RR */
				temp = (acu >> 1) | (TSTFLAG(C) << 7);
				cbits = acu & 1;
				goto cbshflg1;
			case 0x20:	/* SLA */
				temp = acu << 1;
				cbits = acu & 0x80;
				goto cbshflg1;
			case 0x28:	/* SRA */
				temp = (acu >> 1) | (acu & 0x80);
				cbits = acu & 1;
				goto cbshflg1;
			case 0x30:	/* SLIA */
				temp = (acu << 1) | 1;
				cbits = acu & 0x80;
				goto cbshflg1;
			case 0x38:	/* SRL */
				temp = acu >> 1;
				cbits = acu & 1;
			cbshflg1:
				AF = (AF & ~0xff) | (temp & 0xa8) |
					(((temp & 0xff) == 0) << 6) |
					parity(temp) | !!cbits;
			}
			break;
		case 0x40:		/* BIT */
			if (acu & (1 << ((op >> 3) & 7)))
				AF = (AF & ~0xfe) | 0x10 |
				(((op & 0x38) == 0x38) << 7);
			else
				AF = (AF & ~0xfe) | 0x54;
			if ((op&7) != 6)
				AF |= (acu & 0x28);
			temp = acu;
			break;
		case 0x80:		/* RES */
			temp = acu & ~(1 << ((op >> 3) & 7));
			break;
		case 0xc0:		/* SET */
			temp = acu | (1 << ((op >> 3) & 7));
			break;
		}
		switch (op & 7) {
		case 0: Sethreg(BC, temp); break;
		case 1: Setlreg(BC, temp); break;
		case 2: Sethreg(DE, temp); break;
		case 3: Setlreg(DE, temp); break;
		case 4: Sethreg(HL, temp); break;
		case 5: Setlreg(HL, temp); break;
		case 6: PutBYTE(adr, temp);  break;
		case 7: Sethreg(AF, temp); break;
		}
		break;
	case 0xCC:			/* CALL Z,nnnn */
		cycles -= cycleTables[0][0xCC];
		CALLC(TSTFLAG(Z));
		break;
	case 0xCD:			/* CALL nnnn */
		cycles -= cycleTables[0][0xCD];
		CALLC(1);
		break;
	case 0xCE:			/* ADC A,nn */
		cycles -= cycleTables[0][0xCE];
		temp = GetBYTE_pp(pc);
		acu = hreg(AF);
		sum = acu + temp + TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) |
			((cbits >> 8) & 1);
		break;
	case 0xCF:			/* RST 8 */
		cycles -= cycleTables[0][0xCF];
		PUSH(pc); pc = 8;
		break;
	case 0xD0:			/* RET NC */
		cycles -= cycleTables[0][0xD0];
		if (!TSTFLAG(C)) POP(pc);
		break;
	case 0xD1:			/* POP DE */
		cycles -= cycleTables[0][0xD1];
		POP(DE);
		break;
	case 0xD2:			/* JP NC,nnnn */
		cycles -= cycleTables[0][0xD2];
		Jpc(!TSTFLAG(C));
		break;
	case 0xD3:			/* OUT (nn),A */
		cycles -= cycleTables[0][0xD3];
		OUTPUT(GetBYTE_pp(pc), hreg(AF));
		break;
	case 0xD4:			/* CALL NC,nnnn */
		cycles -= cycleTables[0][0xD4];
		CALLC(!TSTFLAG(C));
		break;
	case 0xD5:			/* PUSH DE */
		cycles -= cycleTables[0][0xD5];
		PUSH(DE);
		break;
	case 0xD6:			/* SUB nn */
		cycles -= cycleTables[0][0xD6];
		temp = GetBYTE_pp(pc);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0xD7:			/* RST 10H */
		cycles -= cycleTables[0][0xD7];
		PUSH(pc); pc = 0x10;
		break;
	case 0xD8:			/* RET C */
		cycles -= cycleTables[0][0xD8];
		if (TSTFLAG(C)) POP(pc);
		break;
	case 0xD9:			/* EXX */
		cycles -= cycleTables[0][0xD9];
		regs[regs_sel].bc = BC;
		regs[regs_sel].de = DE;
		regs[regs_sel].hl = HL;
		regs_sel = 1 - regs_sel;
		BC = regs[regs_sel].bc;
		DE = regs[regs_sel].de;
		HL = regs[regs_sel].hl;
		break;
	case 0xDA:			/* JP C,nnnn */
		cycles -= cycleTables[0][0xDA];
		Jpc(TSTFLAG(C));
		break;
	case 0xDB:			/* IN A,(nn) */
		cycles -= cycleTables[0][0xDB];
		Sethreg(AF, INPUT(GetBYTE_pp(pc)));
		break;
	case 0xDC:			/* CALL C,nnnn */
		cycles -= cycleTables[0][0xDC];
		CALLC(TSTFLAG(C));
		break;
	case 0xDD:			/* DD prefix */
		op = GetBYTE_pp(pc);
		switch (op) {
		case 0x09:			/* ADD IX,BC */
			cycles -= cycleTables[3][0x09];
			IX &= 0xffff;
			BC &= 0xffff;
			sum = IX + BC;
			cbits = (IX ^ BC ^ sum) >> 8;
			IX = sum;
			AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x19:			/* ADD IX,DE */
			cycles -= cycleTables[3][0x19];
			IX &= 0xffff;
			DE &= 0xffff;
			sum = IX + DE;
			cbits = (IX ^ DE ^ sum) >> 8;
			IX = sum;
			AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x21:			/* LD IX,nnnn */
			cycles -= cycleTables[3][0x21];
			IX = GetWORD(pc);
			pc += 2;
			break;
		case 0x22:			/* LD (nnnn),IX */
			cycles -= cycleTables[3][0x22];
			temp = GetWORD(pc);
			PutWORD(temp, IX);
			pc += 2;
			break;
		case 0x23:			/* INC IX */
			cycles -= cycleTables[3][0x23];
			++IX;
			break;
		case 0x24:			/* INC IXH */
			cycles -= cycleTables[3][0x24];
			IX += 0x100;
			temp = hreg(IX);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0) << 4) |
				((temp == 0x80) << 2);
			break;
		case 0x25:			/* DEC IXH */
			cycles -= cycleTables[3][0x25];
			IX -= 0x100;
			temp = hreg(IX);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0xf) << 4) |
				((temp == 0x7f) << 2) | 2;
			break;
		case 0x26:			/* LD IXH,nn */
			cycles -= cycleTables[3][0x26];
			Sethreg(IX, GetBYTE_pp(pc));
			break;
		case 0x29:			/* ADD IX,IX */
			cycles -= cycleTables[3][0x29];
			IX &= 0xffff;
			sum = IX + IX;
			cbits = (IX ^ IX ^ sum) >> 8;
			IX = sum;
			AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x2A:			/* LD IX,(nnnn) */
			cycles -= cycleTables[3][0x2A];
			temp = GetWORD(pc);
			IX = GetWORD(temp);
			pc += 2;
			break;
		case 0x2B:			/* DEC IX */
			cycles -= cycleTables[3][0x2B];
			--IX;
			break;
		case 0x2C:			/* INC IXL */
			cycles -= cycleTables[3][0x2C];
			temp = lreg(IX)+1;
			Setlreg(IX, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0) << 4) |
				((temp == 0x80) << 2);
			break;
		case 0x2D:			/* DEC IXL */
			cycles -= cycleTables[3][0x2D];
			temp = lreg(IX)-1;
			Setlreg(IX, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0xf) << 4) |
				((temp == 0x7f) << 2) | 2;
			break;
		case 0x2E:			/* LD IXL,nn */
			cycles -= cycleTables[3][0x2E];
			Setlreg(IX, GetBYTE_pp(pc));
			break;
		case 0x34:			/* INC (IX+dd) */
			cycles -= cycleTables[3][0x34];
			adr = IX + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr)+1;
			PutBYTE(adr, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0) << 4) |
				((temp == 0x80) << 2);
			break;
		case 0x35:			/* DEC (IX+dd) */
			cycles -= cycleTables[3][0x35];
			adr = IX + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr)-1;
			PutBYTE(adr, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0xf) << 4) |
				((temp == 0x7f) << 2) | 2;
			break;
		case 0x36:			/* LD (IX+dd),nn */
			cycles -= cycleTables[3][0x36];
			adr = IX + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, GetBYTE_pp(pc));
			break;
		case 0x39:			/* ADD IX,SP */
			cycles -= cycleTables[3][0x39];
			IX &= 0xffff;
			SP &= 0xffff;
			sum = IX + SP;
			cbits = (IX ^ SP ^ sum) >> 8;
			IX = sum;
			AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x44:			/* LD B,IXH */
			cycles -= cycleTables[3][0x44];
			Sethreg(BC, hreg(IX));
			break;
		case 0x45:			/* LD B,IXL */
			cycles -= cycleTables[3][0x45];
			Sethreg(BC, lreg(IX));
			break;
		case 0x46:			/* LD B,(IX+dd) */
			cycles -= cycleTables[3][0x46];
			adr = IX + (signed char) GetBYTE_pp(pc);
			Sethreg(BC, GetBYTE(adr));
			break;
		case 0x4C:			/* LD C,IXH */
			cycles -= cycleTables[3][0x4C];
			Setlreg(BC, hreg(IX));
			break;
		case 0x4D:			/* LD C,IXL */
			cycles -= cycleTables[3][0x4D];
			Setlreg(BC, lreg(IX));
			break;
		case 0x4E:			/* LD C,(IX+dd) */
			cycles -= cycleTables[3][0x4E];
			adr = IX + (signed char) GetBYTE_pp(pc);
			Setlreg(BC, GetBYTE(adr));
			break;
		case 0x54:			/* LD D,IXH */
			cycles -= cycleTables[3][0x54];
			Sethreg(DE, hreg(IX));
			break;
		case 0x55:			/* LD D,IXL */
			cycles -= cycleTables[3][0x55];
			Sethreg(DE, lreg(IX));
			break;
		case 0x56:			/* LD D,(IX+dd) */
			cycles -= cycleTables[3][0x56];
			adr = IX + (signed char) GetBYTE_pp(pc);
			Sethreg(DE, GetBYTE(adr));
			break;
		case 0x5C:			/* LD E,H */
			cycles -= cycleTables[3][0x5C];
			Setlreg(DE, hreg(IX));
			break;
		case 0x5D:			/* LD E,L */
			cycles -= cycleTables[3][0x5D];
			Setlreg(DE, lreg(IX));
			break;
		case 0x5E:			/* LD E,(IX+dd) */
			cycles -= cycleTables[3][0x5E];
			adr = IX + (signed char) GetBYTE_pp(pc);
			Setlreg(DE, GetBYTE(adr));
			break;
		case 0x60:			/* LD IXH,B */
			cycles -= cycleTables[3][0x60];
			Sethreg(IX, hreg(BC));
			break;
		case 0x61:			/* LD IXH,C */
			cycles -= cycleTables[3][0x61];
			Sethreg(IX, lreg(BC));
			break;
		case 0x62:			/* LD IXH,D */
			cycles -= cycleTables[3][0x62];
			Sethreg(IX, hreg(DE));
			break;
		case 0x63:			/* LD IXH,E */
			cycles -= cycleTables[3][0x63];
			Sethreg(IX, lreg(DE));
			break;
		case 0x64:			/* LD IXH,IXH */
			cycles -= cycleTables[3][0x64];
			/* nop */
			break;
		case 0x65:			/* LD IXH,IXL */
			cycles -= cycleTables[3][0x65];
			Sethreg(IX, lreg(IX));
			break;
		case 0x66:			/* LD H,(IX+dd) */
			cycles -= cycleTables[3][0x66];
			adr = IX + (signed char) GetBYTE_pp(pc);
			Sethreg(HL, GetBYTE(adr));
			break;
		case 0x67:			/* LD IXH,A */
			cycles -= cycleTables[3][0x67];
			Sethreg(IX, hreg(AF));
			break;
		case 0x68:			/* LD IXL,B */
			cycles -= cycleTables[3][0x68];
			Setlreg(IX, hreg(BC));
			break;
		case 0x69:			/* LD IXL,C */
			cycles -= cycleTables[3][0x69];
			Setlreg(IX, lreg(BC));
			break;
		case 0x6A:			/* LD IXL,D */
			cycles -= cycleTables[3][0x6A];
			Setlreg(IX, hreg(DE));
			break;
		case 0x6B:			/* LD IXL,E */
			cycles -= cycleTables[3][0x6B];
			Setlreg(IX, lreg(DE));
			break;
		case 0x6C:			/* LD IXL,IXH */
			cycles -= cycleTables[3][0x6C];
			Setlreg(IX, hreg(IX));
			break;
		case 0x6D:			/* LD IXL,IXL */
			cycles -= cycleTables[3][0x6D];
			/* nop */
			break;
		case 0x6E:			/* LD L,(IX+dd) */
			cycles -= cycleTables[3][0x6E];
			adr = IX + (signed char) GetBYTE_pp(pc);
			Setlreg(HL, GetBYTE(adr));
			break;
		case 0x6F:			/* LD IXL,A */
			cycles -= cycleTables[3][0x6F];
			Setlreg(IX, hreg(AF));
			break;
		case 0x70:			/* LD (IX+dd),B */
			cycles -= cycleTables[3][0x70];
			adr = IX + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, hreg(BC));
			break;
		case 0x71:			/* LD (IX+dd),C */
			cycles -= cycleTables[3][0x71];
			adr = IX + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, lreg(BC));
			break;
		case 0x72:			/* LD (IX+dd),D */
			cycles -= cycleTables[3][0x72];
			adr = IX + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, hreg(DE));
			break;
		case 0x73:			/* LD (IX+dd),E */
			cycles -= cycleTables[3][0x73];
			adr = IX + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, lreg(DE));
			break;
		case 0x74:			/* LD (IX+dd),H */
			cycles -= cycleTables[3][0x74];
			adr = IX + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, hreg(HL));
			break;
		case 0x75:			/* LD (IX+dd),L */
			cycles -= cycleTables[3][0x75];
			adr = IX + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, lreg(HL));
			break;
		case 0x77:			/* LD (IX+dd),A */
			cycles -= cycleTables[3][0x77];
			adr = IX + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, hreg(AF));
			break;
		case 0x7C:			/* LD A,IXH */
			cycles -= cycleTables[3][0x7C];
			Sethreg(AF, hreg(IX));
			break;
		case 0x7D:			/* LD A,IXL */
			cycles -= cycleTables[3][0x7D];
			Sethreg(AF, lreg(IX));
			break;
		case 0x7E:			/* LD A,(IX+dd) */
			cycles -= cycleTables[3][0x7E];
			adr = IX + (signed char) GetBYTE_pp(pc);
			Sethreg(AF, GetBYTE(adr));
			break;
		case 0x84:			/* ADD A,IXH */
			cycles -= cycleTables[3][0x84];
			temp = hreg(IX);
			acu = hreg(AF);
			sum = acu + temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x85:			/* ADD A,IXL */
			cycles -= cycleTables[3][0x85];
			temp = lreg(IX);
			acu = hreg(AF);
			sum = acu + temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x86:			/* ADD A,(IX+dd) */
			cycles -= cycleTables[3][0x86];
			adr = IX + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			acu = hreg(AF);
			sum = acu + temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x8C:			/* ADC A,IXH */
			cycles -= cycleTables[3][0x8C];
			temp = hreg(IX);
			acu = hreg(AF);
			sum = acu + temp + TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x8D:			/* ADC A,IXL */
			cycles -= cycleTables[3][0x8D];
			temp = lreg(IX);
			acu = hreg(AF);
			sum = acu + temp + TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x8E:			/* ADC A,(IX+dd) */
			cycles -= cycleTables[3][0x8E];
			adr = IX + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			acu = hreg(AF);
			sum = acu + temp + TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x94:			/* SUB IXH */
			cycles -= cycleTables[3][0x94];
			temp = hreg(IX);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x95:			/* SUB IXL */
			cycles -= cycleTables[3][0x95];
			temp = lreg(IX);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x96:			/* SUB (IX+dd) */
			cycles -= cycleTables[3][0x96];
			adr = IX + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x9C:			/* SBC A,IXH */
			cycles -= cycleTables[3][0x9C];
			temp = hreg(IX);
			acu = hreg(AF);
			sum = acu - temp - TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x9D:			/* SBC A,IXL */
			cycles -= cycleTables[3][0x9D];
			temp = lreg(IX);
			acu = hreg(AF);
			sum = acu - temp - TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x9E:			/* SBC A,(IX+dd) */
			cycles -= cycleTables[3][0x9E];
			adr = IX + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			acu = hreg(AF);
			sum = acu - temp - TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0xA4:			/* AND IXH */
			cycles -= cycleTables[3][0xA4];
			sum = ((AF & (IX)) >> 8) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) |
				((sum == 0) << 6) | 0x10 | partab[sum];
			break;
		case 0xA5:			/* AND IXL */
			cycles -= cycleTables[3][0xA5];
			sum = ((AF >> 8) & IX) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | 0x10 |
				((sum == 0) << 6) | partab[sum];
			break;
		case 0xA6:			/* AND (IX+dd) */
			cycles -= cycleTables[3][0xA6];
			adr = IX + (signed char) GetBYTE_pp(pc);
			sum = ((AF >> 8) & GetBYTE(adr)) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | 0x10 |
				((sum == 0) << 6) | partab[sum];
			break;
		case 0xAC:			/* XOR IXH */
			cycles -= cycleTables[3][0xAC];
			sum = ((AF ^ (IX)) >> 8) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xAD:			/* XOR IXL */
			cycles -= cycleTables[3][0xAD];
			sum = ((AF >> 8) ^ IX) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xAE:			/* XOR (IX+dd) */
			cycles -= cycleTables[3][0xAE];
			adr = IX + (signed char) GetBYTE_pp(pc);
			sum = ((AF >> 8) ^ GetBYTE(adr)) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xB4:			/* OR IXH */
			cycles -= cycleTables[3][0xB4];
			sum = ((AF | (IX)) >> 8) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xB5:			/* OR IXL */
			cycles -= cycleTables[3][0xB5];
			sum = ((AF >> 8) | IX) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xB6:			/* OR (IX+dd) */
			cycles -= cycleTables[3][0xB6];
			adr = IX + (signed char) GetBYTE_pp(pc);
			sum = ((AF >> 8) | GetBYTE(adr)) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xBC:			/* CP IXH */
			cycles -= cycleTables[3][0xBC];
			temp = hreg(IX);
			AF = (AF & ~0x28) | (temp & 0x28);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xff) | (sum & 0x80) |
				(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0xBD:			/* CP IXL */
			cycles -= cycleTables[3][0xBD];
			temp = lreg(IX);
			AF = (AF & ~0x28) | (temp & 0x28);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xff) | (sum & 0x80) |
				(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0xBE:			/* CP (IX+dd) */
			cycles -= cycleTables[3][0xBE];
			adr = IX + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			AF = (AF & ~0x28) | (temp & 0x28);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xff) | (sum & 0x80) |
				(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0xCB:			/* CB prefix */
			adr = IX + (signed char) GetBYTE_pp(pc);
			adr = adr;
			op = GetBYTE(pc);
			cycles -= cycleTables[4][op];
			switch (op & 7) {
			case 0: ++pc; acu = hreg(BC); break;
			case 1: ++pc; acu = lreg(BC); break;
			case 2: ++pc; acu = hreg(DE); break;
			case 3: ++pc; acu = lreg(DE); break;
			case 4: ++pc; acu = hreg(HL); break;
			case 5: ++pc; acu = lreg(HL); break;
			case 6: ++pc; acu = GetBYTE(adr);  break;
			case 7: ++pc; acu = hreg(AF); break;
			}
			switch (op & 0xc0) {
			case 0x00:		/* shift/rotate */
				switch (op & 0x38) {
				case 0x00:	/* RLC */
					temp = (acu << 1) | (acu >> 7);
					cbits = temp & 1;
					goto cbshflg2;
				case 0x08:	/* RRC */
					temp = (acu >> 1) | (acu << 7);
					cbits = temp & 0x80;
					goto cbshflg2;
				case 0x10:	/* RL */
					temp = (acu << 1) | TSTFLAG(C);
					cbits = acu & 0x80;
					goto cbshflg2;
				case 0x18:	/* RR */
					temp = (acu >> 1) | (TSTFLAG(C) << 7);
					cbits = acu & 1;
					goto cbshflg2;
				case 0x20:	/* SLA */
					temp = acu << 1;
					cbits = acu & 0x80;
					goto cbshflg2;
				case 0x28:	/* SRA */
					temp = (acu >> 1) | (acu & 0x80);
					cbits = acu & 1;
					goto cbshflg2;
				case 0x30:	/* SLIA */
					temp = (acu << 1) | 1;
					cbits = acu & 0x80;
					goto cbshflg2;
				case 0x38:	/* SRL */
					temp = acu >> 1;
					cbits = acu & 1;
				cbshflg2:
					AF = (AF & ~0xff) | (temp & 0xa8) |
						(((temp & 0xff) == 0) << 6) |
						parity(temp) | !!cbits;
				}
				break;
			case 0x40:		/* BIT */
				if (acu & (1 << ((op >> 3) & 7)))
					AF = (AF & ~0xfe) | 0x10 |
					(((op & 0x38) == 0x38) << 7);
				else
					AF = (AF & ~0xfe) | 0x54;
				if ((op&7) != 6)
					AF |= (acu & 0x28);
				temp = acu;
				break;
			case 0x80:		/* RES */
				temp = acu & ~(1 << ((op >> 3) & 7));
				break;
			case 0xc0:		/* SET */
				temp = acu | (1 << ((op >> 3) & 7));
				break;
			}
			switch (op & 7) {
			case 0: Sethreg(BC, temp); break;
			case 1: Setlreg(BC, temp); break;
			case 2: Sethreg(DE, temp); break;
			case 3: Setlreg(DE, temp); break;
			case 4: Sethreg(HL, temp); break;
			case 5: Setlreg(HL, temp); break;
			case 6: PutBYTE(adr, temp);  break;
			case 7: Sethreg(AF, temp); break;
			}
			break;
		case 0xE1:			/* POP IX */
			cycles -= cycleTables[3][0xE1];
			POP(IX);
			break;
		case 0xE3:			/* EX (SP),IX */
			cycles -= cycleTables[3][0xE3];
			temp = IX; POP(IX); PUSH(temp);
			break;
		case 0xE5:			/* PUSH IX */
			cycles -= cycleTables[3][0xE5];
			PUSH(IX);
			break;
		case 0xE9:			/* JP (IX) */
			cycles -= cycleTables[3][0xE9];
			pc = IX;
			break;
		case 0xF9:			/* LD SP,IX */
			cycles -= cycleTables[3][0xF9];
			SP = IX;
			break;
		default: pc--;		/* ignore DD */
		}
		break;
	case 0xDE:			/* SBC A,nn */
		cycles -= cycleTables[0][0xDE];
		temp = GetBYTE_pp(pc);
		acu = hreg(AF);
		sum = acu - temp - TSTFLAG(C);
		cbits = acu ^ temp ^ sum;
		AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
			(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			((cbits >> 8) & 1);
		break;
	case 0xDF:			/* RST 18H */
		cycles -= cycleTables[0][0xDF];
		PUSH(pc); pc = 0x18;
		break;
	case 0xE0:			/* RET PO */
		cycles -= cycleTables[0][0xE0];
		if (!TSTFLAG(P)) POP(pc);
		break;
	case 0xE1:			/* POP HL */
		cycles -= cycleTables[0][0xE1];
		POP(HL);
		break;
	case 0xE2:			/* JP PO,nnnn */
		cycles -= cycleTables[0][0xE2];
		Jpc(!TSTFLAG(P));
		break;
	case 0xE3:			/* EX (SP),HL */
		cycles -= cycleTables[0][0xE3];
		temp = HL; POP(HL); PUSH(temp);
		break;
	case 0xE4:			/* CALL PO,nnnn */
		cycles -= cycleTables[0][0xE4];
		CALLC(!TSTFLAG(P));
		break;
	case 0xE5:			/* PUSH HL */
		cycles -= cycleTables[0][0xE5];
		PUSH(HL);
		break;
	case 0xE6:			/* AND nn */
		cycles -= cycleTables[0][0xE6];
		sum = ((AF >> 8) & GetBYTE_pp(pc)) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | 0x10 |
			((sum == 0) << 6) | partab[sum];
		break;
	case 0xE7:			/* RST 20H */
		cycles -= cycleTables[0][0xE7];
		PUSH(pc); pc = 0x20;
		break;
	case 0xE8:			/* RET PE */
		cycles -= cycleTables[0][0xE8];
		if (TSTFLAG(P)) POP(pc);
		break;
	case 0xE9:			/* JP (HL) */
		cycles -= cycleTables[0][0xE9];
		pc = HL;
		break;
	case 0xEA:			/* JP PE,nnnn */
		cycles -= cycleTables[0][0xEA];
		Jpc(TSTFLAG(P));
		break;
	case 0xEB:			/* EX DE,HL */
		cycles -= cycleTables[0][0xEB];
		temp = HL; HL = DE; DE = temp;
		break;
	case 0xEC:			/* CALL PE,nnnn */
		cycles -= cycleTables[0][0xEC];
		CALLC(TSTFLAG(P));
		break;
	case 0xED:			/* ED prefix */
		op = GetBYTE_pp(pc);
		switch (op) {
		case 0x40:			/* IN B,(C) */
			cycles -= cycleTables[2][0x40];
			temp = INPUT(lreg(BC));
			Sethreg(BC, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				parity(temp);
			break;
		case 0x41:			/* OUT (C),B */
			cycles -= cycleTables[2][0x41];
			OUTPUT(lreg(BC), BC);
			break;
		case 0x42:			/* SBC HL,BC */
			cycles -= cycleTables[2][0x42];
			HL &= 0xffff;
			BC &= 0xffff;
			sum = HL - BC - TSTFLAG(C);
			cbits = (HL ^ BC ^ sum) >> 8;
			HL = sum;
			AF = (AF & ~0xff) | ((sum >> 8) & 0xa8) |
				(((sum & 0xffff) == 0) << 6) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				(cbits & 0x10) | 2 | ((cbits >> 8) & 1);
			break;
		case 0x43:			/* LD (nnnn),BC */
			cycles -= cycleTables[2][0x43];
			temp = GetWORD(pc);
			PutWORD(temp, BC);
			pc += 2;
			break;
		case 0x44:			/* NEG */
			cycles -= cycleTables[2][0x44];
			temp = hreg(AF);
			AF = (-(AF & 0xff00) & 0xff00);
			AF |= ((AF >> 8) & 0xa8) | (((AF & 0xff00) == 0) << 6) |
				(((temp & 0x0f) != 0) << 4) | ((temp == 0x80) << 2) |
				2 | (temp != 0);
			break;
		case 0x45:			/* RETN */
			cycles -= cycleTables[2][0x45];
			iff |= iff >> 1;
			POP(pc);
			break;
		case 0x46:			/* IM 0 */
			cycles -= cycleTables[2][0x46];
			im = 0;	// interrupt mode 0
			break;
		case 0x47:			/* LD I,A */
			cycles -= cycleTables[2][0x47];
			ir = (ir & 255) | (AF & ~255);
			break;
		case 0x48:			/* IN C,(C) */
			cycles -= cycleTables[2][0x48];
			temp = INPUT(lreg(BC));
			Setlreg(BC, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				parity(temp);
			break;
		case 0x49:			/* OUT (C),C */
			cycles -= cycleTables[2][0x49];
			OUTPUT(lreg(BC), BC);
			break;
		case 0x4A:			/* ADC HL,BC */
			cycles -= cycleTables[2][0x4A];
			HL &= 0xffff;
			BC &= 0xffff;
			sum = HL + BC + TSTFLAG(C);
			cbits = (HL ^ BC ^ sum) >> 8;
			HL = sum;
			AF = (AF & ~0xff) | ((sum >> 8) & 0xa8) |
				(((sum & 0xffff) == 0) << 6) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x4B:			/* LD BC,(nnnn) */
			cycles -= cycleTables[2][0x4B];
			temp = GetWORD(pc);
			BC = GetWORD(temp);
			pc += 2;
			break;
		case 0x4D:			/* RETI */
			cycles -= cycleTables[2][0x4D];
			iff |= iff >> 1;
			POP(pc);
			break;
		case 0x4F:			/* LD R,A */
			cycles -= cycleTables[2][0x4F];
			ir = (ir & ~255) | ((AF >> 8) & 255);
			break;
		case 0x50:			/* IN D,(C) */
			cycles -= cycleTables[2][0x50];
			temp = INPUT(lreg(BC));
			Sethreg(DE, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				parity(temp);
			break;
		case 0x51:			/* OUT (C),D */
			cycles -= cycleTables[2][0x51];
			OUTPUT(lreg(BC), DE);
			break;
		case 0x52:			/* SBC HL,DE */
			cycles -= cycleTables[2][0x52];
			HL &= 0xffff;
			DE &= 0xffff;
			sum = HL - DE - TSTFLAG(C);
			cbits = (HL ^ DE ^ sum) >> 8;
			HL = sum;
			AF = (AF & ~0xff) | ((sum >> 8) & 0xa8) |
				(((sum & 0xffff) == 0) << 6) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				(cbits & 0x10) | 2 | ((cbits >> 8) & 1);
			break;
		case 0x53:			/* LD (nnnn),DE */
			cycles -= cycleTables[2][0x53];
			temp = GetWORD(pc);
			PutWORD(temp, DE);
			pc += 2;
			break;
		case 0x56:			/* IM 1 */
			cycles -= cycleTables[2][0x56];
			im = 1;	// interrupt mode 1
			break;
		case 0x57:			/* LD A,I */
			cycles -= cycleTables[2][0x57];
			AF = (AF & 0x29) | (ir & ~255) | ((ir >> 8) & 0x80) | (((ir & ~255) == 0) << 6) | ((iff & 2) << 1);
			break;
		case 0x58:			/* IN E,(C) */
			cycles -= cycleTables[2][0x58];
			temp = INPUT(lreg(BC));
			Setlreg(DE, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				parity(temp);
			break;
		case 0x59:			/* OUT (C),E */
			cycles -= cycleTables[2][0x59];
			OUTPUT(lreg(BC), DE);
			break;
		case 0x5A:			/* ADC HL,DE */
			cycles -= cycleTables[2][0x5A];
			HL &= 0xffff;
			DE &= 0xffff;
			sum = HL + DE + TSTFLAG(C);
			cbits = (HL ^ DE ^ sum) >> 8;
			HL = sum;
			AF = (AF & ~0xff) | ((sum >> 8) & 0xa8) |
				(((sum & 0xffff) == 0) << 6) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x5B:			/* LD DE,(nnnn) */
			cycles -= cycleTables[2][0x5B];
			temp = GetWORD(pc);
			DE = GetWORD(temp);
			pc += 2;
			break;
		case 0x5E:			/* IM 2 */
			cycles -= cycleTables[2][0x5E];
			im = 2;	// interrupt mode 2
			break;
		case 0x5F:			/* LD A,R */
			cycles -= cycleTables[2][0x5F];
			AF = (AF & 0x29) | ((ir & 255) << 8) | (ir & 0x80) | (((ir & 255) == 0) << 6) | ((iff & 2) << 1);
			break;
		case 0x60:			/* IN H,(C) */
			cycles -= cycleTables[2][0x60];
			temp = INPUT(lreg(BC));
			Sethreg(HL, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				parity(temp);
			break;
		case 0x61:			/* OUT (C),H */
			cycles -= cycleTables[2][0x61];
			OUTPUT(lreg(BC), HL);
			break;
		case 0x62:			/* SBC HL,HL */
			cycles -= cycleTables[2][0x62];
			HL &= 0xffff;
			sum = HL - HL - TSTFLAG(C);
			cbits = (HL ^ HL ^ sum) >> 8;
			HL = sum;
			AF = (AF & ~0xff) | ((sum >> 8) & 0xa8) |
				(((sum & 0xffff) == 0) << 6) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				(cbits & 0x10) | 2 | ((cbits >> 8) & 1);
			break;
		case 0x63:			/* LD (nnnn),HL */
			cycles -= cycleTables[2][0x63];
			temp = GetWORD(pc);
			PutWORD(temp, HL);
			pc += 2;
			break;
		case 0x67:			/* RRD */
			cycles -= cycleTables[2][0x67];
			temp = GetBYTE(HL);
			acu = hreg(AF);
			PutBYTE(HL, hdig(temp) | (ldig(acu) << 4));
			acu = (acu & 0xf0) | ldig(temp);
			AF = (acu << 8) | (acu & 0xa8) | (((acu & 0xff) == 0) << 6) |
				partab[acu] | (AF & 1);
			break;
		case 0x68:			/* IN L,(C) */
			cycles -= cycleTables[2][0x68];
			temp = INPUT(lreg(BC));
			Setlreg(HL, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				parity(temp);
			break;
		case 0x69:			/* OUT (C),L */
			cycles -= cycleTables[2][0x69];
			OUTPUT(lreg(BC), HL);
			break;
		case 0x6A:			/* ADC HL,HL */
			cycles -= cycleTables[2][0x6A];
			HL &= 0xffff;
			sum = HL + HL + TSTFLAG(C);
			cbits = (HL ^ HL ^ sum) >> 8;
			HL = sum;
			AF = (AF & ~0xff) | ((sum >> 8) & 0xa8) |
				(((sum & 0xffff) == 0) << 6) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x6B:			/* LD HL,(nnnn) */
			cycles -= cycleTables[2][0x6B];
			temp = GetWORD(pc);
			HL = GetWORD(temp);
			pc += 2;
			break;
		case 0x6F:			/* RLD */
			cycles -= cycleTables[2][0x6F];
			temp = GetBYTE(HL);
			acu = hreg(AF);
			PutBYTE(HL, (ldig(temp) << 4) | ldig(acu));
			acu = (acu & 0xf0) | hdig(temp);
			AF = (acu << 8) | (acu & 0xa8) | (((acu & 0xff) == 0) << 6) |
				partab[acu] | (AF & 1);
			break;
		case 0x70:			/* IN (C) */
			cycles -= cycleTables[2][0x70];
			temp = INPUT(lreg(BC));
			Setlreg(temp, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				parity(temp);
			break;
		case 0x71:			/* OUT (C),0 */
			cycles -= cycleTables[2][0x71];
			OUTPUT(lreg(BC), 0);
			break;
		case 0x72:			/* SBC HL,SP */
			cycles -= cycleTables[2][0x72];
			HL &= 0xffff;
			SP &= 0xffff;
			sum = HL - SP - TSTFLAG(C);
			cbits = (HL ^ SP ^ sum) >> 8;
			HL = sum;
			AF = (AF & ~0xff) | ((sum >> 8) & 0xa8) |
				(((sum & 0xffff) == 0) << 6) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				(cbits & 0x10) | 2 | ((cbits >> 8) & 1);
			break;
		case 0x73:			/* LD (nnnn),SP */
			cycles -= cycleTables[2][0x73];
			temp = GetWORD(pc);
			PutWORD(temp, SP);
			pc += 2;
			break;
		case 0x78:			/* IN A,(C) */
			cycles -= cycleTables[2][0x78];
			temp = INPUT(lreg(BC));
			Sethreg(AF, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				parity(temp);
			break;
		case 0x79:			/* OUT (C),A */
			cycles -= cycleTables[2][0x79];
			OUTPUT(lreg(BC), AF);
			break;
		case 0x7A:			/* ADC HL,SP */
			cycles -= cycleTables[2][0x7A];
			HL &= 0xffff;
			SP &= 0xffff;
			sum = HL + SP + TSTFLAG(C);
			cbits = (HL ^ SP ^ sum) >> 8;
			HL = sum;
			AF = (AF & ~0xff) | ((sum >> 8) & 0xa8) |
				(((sum & 0xffff) == 0) << 6) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x7B:			/* LD SP,(nnnn) */
			cycles -= cycleTables[2][0x7B];
			temp = GetWORD(pc);
			SP = GetWORD(temp);
			pc += 2;
			break;
		case 0xA0:			/* LDI */
			cycles -= cycleTables[2][0xA0];
			acu = GetBYTE_pp(HL);
			PutBYTE_pp(DE, acu);
			acu += hreg(AF);
			AF = (AF & ~0x3e) | (acu & 8) | ((acu & 2) << 4) |
				(((--BC & 0xffff) != 0) << 2);
			break;
		case 0xA1:			/* CPI */
			cycles -= cycleTables[2][0xA1];
			acu = hreg(AF);
			temp = GetBYTE_pp(HL);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xfe) | (sum & 0x80) | (!(sum & 0xff) << 6) |
				(((sum - ((cbits&16)>>4))&2) << 4) | (cbits & 16) |
				((sum - ((cbits >> 4) & 1)) & 8) |
				((--BC & 0xffff) != 0) << 2 | 2;
			if ((sum & 15) == 8 && (cbits & 16) != 0)
				AF &= ~8;
			break;
		case 0xA2:			/* INI */
			cycles -= cycleTables[2][0xA2];
			PutBYTE(HL, INPUT(lreg(BC))); ++HL;
			SETFLAG(N, 1);
			SETFLAG(P, (--BC & 0xffff) != 0);
			break;
		case 0xA3:			/* OUTI */
			cycles -= cycleTables[2][0xA3];
			OUTPUT(lreg(BC), GetBYTE(HL)); ++HL;
			SETFLAG(N, 1);
			Sethreg(BC, hreg(BC) - 1);
			SETFLAG(Z, hreg(BC) == 0);
			break;
		case 0xA8:			/* LDD */
			cycles -= cycleTables[2][0xA8];
			acu = GetBYTE_mm(HL);
			PutBYTE_mm(DE, acu);
			acu += hreg(AF);
			AF = (AF & ~0x3e) | (acu & 8) | ((acu & 2) << 4) |
				(((--BC & 0xffff) != 0) << 2);
			break;
		case 0xA9:			/* CPD */
			cycles -= cycleTables[2][0xA9];
			acu = hreg(AF);
			temp = GetBYTE_mm(HL);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xfe) | (sum & 0x80) | (!(sum & 0xff) << 6) |
				(((sum - ((cbits&16)>>4))&2) << 4) | (cbits & 16) |
				((sum - ((cbits >> 4) & 1)) & 8) |
				((--BC & 0xffff) != 0) << 2 | 2;
			if ((sum & 15) == 8 && (cbits & 16) != 0)
				AF &= ~8;
			break;
		case 0xAA:			/* IND */
			cycles -= cycleTables[2][0xAA];
			PutBYTE(HL, INPUT(lreg(BC))); --HL;
			SETFLAG(N, 1);
			Sethreg(BC, lreg(BC) - 1);
			SETFLAG(Z, lreg(BC) == 0);
			break;
		case 0xAB:			/* OUTD */
			cycles -= cycleTables[2][0xAB];
			OUTPUT(lreg(BC), GetBYTE(HL)); --HL;
			SETFLAG(N, 1);
			Sethreg(BC, hreg(BC) - 1);
			SETFLAG(Z, hreg(BC) == 0);
			break;
		case 0xB0:			/* LDIR */
			cycles -= cycleTables[2][0xB0];
			acu = hreg(AF);
			BC &= 0xffff;
			do {
				acu = GetBYTE_pp(HL);
				PutBYTE_pp(DE, acu);
			} while (--BC);
			acu += hreg(AF);
			AF = (AF & ~0x3e) | (acu & 8) | ((acu & 2) << 4);
			break;
		case 0xB1:			/* CPIR */
			cycles -= cycleTables[2][0xB1];
			acu = hreg(AF);
			BC &= 0xffff;
			do {
				temp = GetBYTE_pp(HL);
				op = --BC != 0;
				sum = acu - temp;
			} while (op && sum != 0);
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xfe) | (sum & 0x80) | (!(sum & 0xff) << 6) |
				(((sum - ((cbits&16)>>4))&2) << 4) |
				(cbits & 16) | ((sum - ((cbits >> 4) & 1)) & 8) |
				op << 2 | 2;
			if ((sum & 15) == 8 && (cbits & 16) != 0)
				AF &= ~8;
			break;
		case 0xB2:			/* INIR */
			cycles -= cycleTables[2][0xB2];
			temp = hreg(BC);
			do {
				PutBYTE(HL, INPUT(lreg(BC))); ++HL;
			} while (--temp);
			Sethreg(BC, 0);
			SETFLAG(N, 1);
			SETFLAG(Z, 1);
			break;
		case 0xB3:			/* OTIR */
			cycles -= cycleTables[2][0xB3];
			temp = hreg(BC);
			do {
				OUTPUT(lreg(BC), GetBYTE(HL)); ++HL;
			} while (--temp);
			Sethreg(BC, 0);
			SETFLAG(N, 1);
			SETFLAG(Z, 1);
			break;
		case 0xB8:			/* LDDR */
			cycles -= cycleTables[2][0xB8];
			BC &= 0xffff;
			do {
				acu = GetBYTE_mm(HL);
				PutBYTE_mm(DE, acu);
			} while (--BC);
			acu += hreg(AF);
			AF = (AF & ~0x3e) | (acu & 8) | ((acu & 2) << 4);
			break;
		case 0xB9:			/* CPDR */
			cycles -= cycleTables[2][0xB9];
			acu = hreg(AF);
			BC &= 0xffff;
			do {
				temp = GetBYTE_mm(HL);
				op = --BC != 0;
				sum = acu - temp;
			} while (op && sum != 0);
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xfe) | (sum & 0x80) | (!(sum & 0xff) << 6) |
				(((sum - ((cbits&16)>>4))&2) << 4) |
				(cbits & 16) | ((sum - ((cbits >> 4) & 1)) & 8) |
				op << 2 | 2;
			if ((sum & 15) == 8 && (cbits & 16) != 0)
				AF &= ~8;
			break;
		case 0xBA:			/* INDR */
			cycles -= cycleTables[2][0xBA];
			temp = hreg(BC);
			do {
				PutBYTE(HL, INPUT(lreg(BC))); --HL;
			} while (--temp);
			Sethreg(BC, 0);
			SETFLAG(N, 1);
			SETFLAG(Z, 1);
			break;
		case 0xBB:			/* OTDR */
			cycles -= cycleTables[2][0xBB];
			temp = hreg(BC);
			do {
				OUTPUT(lreg(BC), GetBYTE(HL)); --HL;
			} while (--temp);
			Sethreg(BC, 0);
			SETFLAG(N, 1);
			SETFLAG(Z, 1);
			break;
		default: if (0x40 <= op && op <= 0x7f) pc--;		/* ignore ED */
		}
		break;
	case 0xEE:			/* XOR nn */
		cycles -= cycleTables[0][0xEE];
		sum = ((AF >> 8) ^ GetBYTE_pp(pc)) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xEF:			/* RST 28H */
		cycles -= cycleTables[0][0xEF];
		PUSH(pc); pc = 0x28;
		break;
	case 0xF0:			/* RET P */
		cycles -= cycleTables[0][0xF0];
		if (!TSTFLAG(S)) POP(pc);
		break;
	case 0xF1:			/* POP AF */
		cycles -= cycleTables[0][0xF1];
		POP(AF);
		break;
	case 0xF2:			/* JP P,nnnn */
		cycles -= cycleTables[0][0xF2];
		Jpc(!TSTFLAG(S));
		break;
	case 0xF3:			/* DI */
		cycles -= cycleTables[0][0xF3];
		iff = 0;
		break;
	case 0xF4:			/* CALL P,nnnn */
		cycles -= cycleTables[0][0xF4];
		CALLC(!TSTFLAG(S));
		break;
	case 0xF5:			/* PUSH AF */
		cycles -= cycleTables[0][0xF5];
		PUSH(AF);
		break;
	case 0xF6:			/* OR nn */
		cycles -= cycleTables[0][0xF6];
		sum = ((AF >> 8) | GetBYTE_pp(pc)) & 0xff;
		AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
		break;
	case 0xF7:			/* RST 30H */
		cycles -= cycleTables[0][0xF7];
		PUSH(pc); pc = 0x30;
		break;
	case 0xF8:			/* RET M */
		cycles -= cycleTables[0][0xF8];
		if (TSTFLAG(S)) POP(pc);
		break;
	case 0xF9:			/* LD SP,HL */
		cycles -= cycleTables[0][0xF9];
		SP = HL;
		break;
	case 0xFA:			/* JP M,nnnn */
		cycles -= cycleTables[0][0xFA];
		Jpc(TSTFLAG(S));
		break;
	case 0xFB:			/* EI */
		cycles -= cycleTables[0][0xFB];
		iff = 3;
		break;
	case 0xFC:			/* CALL M,nnnn */
		cycles -= cycleTables[0][0xFC];
		CALLC(TSTFLAG(S));
		break;
	case 0xFD:			/* FD prefix */
		op = GetBYTE_pp(pc);
		switch (op) {
		case 0x09:			/* ADD IY,BC */
			cycles -= cycleTables[3][0x09];
			IY &= 0xffff;
			BC &= 0xffff;
			sum = IY + BC;
			cbits = (IY ^ BC ^ sum) >> 8;
			IY = sum;
			AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x19:			/* ADD IY,DE */
			cycles -= cycleTables[3][0x19];
			IY &= 0xffff;
			DE &= 0xffff;
			sum = IY + DE;
			cbits = (IY ^ DE ^ sum) >> 8;
			IY = sum;
			AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x21:			/* LD IY,nnnn */
			cycles -= cycleTables[3][0x21];
			IY = GetWORD(pc);
			pc += 2;
			break;
		case 0x22:			/* LD (nnnn),IY */
			cycles -= cycleTables[3][0x22];
			temp = GetWORD(pc);
			PutWORD(temp, IY);
			pc += 2;
			break;
		case 0x23:			/* INC IY */
			cycles -= cycleTables[3][0x23];
			++IY;
			break;
		case 0x24:			/* INC IYH */
			cycles -= cycleTables[3][0x24];
			IY += 0x100;
			temp = hreg(IY);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0) << 4) |
				((temp == 0x80) << 2);
			break;
		case 0x25:			/* DEC IYH */
			cycles -= cycleTables[3][0x25];
			IY -= 0x100;
			temp = hreg(IY);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0xf) << 4) |
				((temp == 0x7f) << 2) | 2;
			break;
		case 0x26:			/* LD IYH,nn */
			cycles -= cycleTables[3][0x26];
			Sethreg(IY, GetBYTE_pp(pc));
			break;
		case 0x29:			/* ADD IY,IY */
			cycles -= cycleTables[3][0x29];
			IY &= 0xffff;
			sum = IY + IY;
			cbits = (IY ^ IY ^ sum) >> 8;
			IY = sum;
			AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x2A:			/* LD IY,(nnnn) */
			cycles -= cycleTables[3][0x2A];
			temp = GetWORD(pc);
			IY = GetWORD(temp);
			pc += 2;
			break;
		case 0x2B:			/* DEC IY */
			cycles -= cycleTables[3][0x2B];
			--IY;
			break;
		case 0x2C:			/* INC IYL */
			cycles -= cycleTables[3][0x2C];
			temp = lreg(IY)+1;
			Setlreg(IY, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0) << 4) |
				((temp == 0x80) << 2);
			break;
		case 0x2D:			/* DEC IYL */
			cycles -= cycleTables[3][0x2D];
			temp = lreg(IY)-1;
			Setlreg(IY, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0xf) << 4) |
				((temp == 0x7f) << 2) | 2;
			break;
		case 0x2E:			/* LD IYL,nn */
			cycles -= cycleTables[3][0x2E];
			Setlreg(IY, GetBYTE_pp(pc));
			break;
		case 0x34:			/* INC (IY+dd) */
			cycles -= cycleTables[3][0x34];
			adr = IY + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr)+1;
			PutBYTE(adr, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0) << 4) |
				((temp == 0x80) << 2);
			break;
		case 0x35:			/* DEC (IY+dd) */
			cycles -= cycleTables[3][0x35];
			adr = IY + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr)-1;
			PutBYTE(adr, temp);
			AF = (AF & ~0xfe) | (temp & 0xa8) |
				(((temp & 0xff) == 0) << 6) |
				(((temp & 0xf) == 0xf) << 4) |
				((temp == 0x7f) << 2) | 2;
			break;
		case 0x36:			/* LD (IY+dd),nn */
			cycles -= cycleTables[3][0x36];
			adr = IY + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, GetBYTE_pp(pc));
			break;
		case 0x39:			/* ADD IY,SP */
			cycles -= cycleTables[3][0x39];
			IY &= 0xffff;
			SP &= 0xffff;
			sum = IY + SP;
			cbits = (IY ^ SP ^ sum) >> 8;
			IY = sum;
			AF = (AF & ~0x3b) | ((sum >> 8) & 0x28) |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0x44:			/* LD B,IYH */
			cycles -= cycleTables[3][0x44];
			Sethreg(BC, hreg(IY));
			break;
		case 0x45:			/* LD B,IYL */
			cycles -= cycleTables[3][0x45];
			Sethreg(BC, lreg(IY));
			break;
		case 0x46:			/* LD B,(IY+dd) */
			cycles -= cycleTables[3][0x46];
			adr = IY + (signed char) GetBYTE_pp(pc);
			Sethreg(BC, GetBYTE(adr));
			break;
		case 0x4C:			/* LD C,IYH */
			cycles -= cycleTables[3][0x4C];
			Setlreg(BC, hreg(IY));
			break;
		case 0x4D:			/* LD C,IYL */
			cycles -= cycleTables[3][0x4D];
			Setlreg(BC, lreg(IY));
			break;
		case 0x4E:			/* LD C,(IY+dd) */
			cycles -= cycleTables[3][0x4E];
			adr = IY + (signed char) GetBYTE_pp(pc);
			Setlreg(BC, GetBYTE(adr));
			break;
		case 0x54:			/* LD D,IYH */
			cycles -= cycleTables[3][0x54];
			Sethreg(DE, hreg(IY));
			break;
		case 0x55:			/* LD D,IYL */
			cycles -= cycleTables[3][0x55];
			Sethreg(DE, lreg(IY));
			break;
		case 0x56:			/* LD D,(IY+dd) */
			cycles -= cycleTables[3][0x56];
			adr = IY + (signed char) GetBYTE_pp(pc);
			Sethreg(DE, GetBYTE(adr));
			break;
		case 0x5C:			/* LD E,H */
			cycles -= cycleTables[3][0x5C];
			Setlreg(DE, hreg(IY));
			break;
		case 0x5D:			/* LD E,L */
			cycles -= cycleTables[3][0x5D];
			Setlreg(DE, lreg(IY));
			break;
		case 0x5E:			/* LD E,(IY+dd) */
			cycles -= cycleTables[3][0x5E];
			adr = IY + (signed char) GetBYTE_pp(pc);
			Setlreg(DE, GetBYTE(adr));
			break;
		case 0x60:			/* LD IYH,B */
			cycles -= cycleTables[3][0x60];
			Sethreg(IY, hreg(BC));
			break;
		case 0x61:			/* LD IYH,C */
			cycles -= cycleTables[3][0x61];
			Sethreg(IY, lreg(BC));
			break;
		case 0x62:			/* LD IYH,D */
			cycles -= cycleTables[3][0x62];
			Sethreg(IY, hreg(DE));
			break;
		case 0x63:			/* LD IYH,E */
			cycles -= cycleTables[3][0x63];
			Sethreg(IY, lreg(DE));
			break;
		case 0x64:			/* LD IYH,IYH */
			cycles -= cycleTables[3][0x64];
			/* nop */
			break;
		case 0x65:			/* LD IYH,IYL */
			cycles -= cycleTables[3][0x65];
			Sethreg(IY, lreg(IY));
			break;
		case 0x66:			/* LD H,(IY+dd) */
			cycles -= cycleTables[3][0x66];
			adr = IY + (signed char) GetBYTE_pp(pc);
			Sethreg(HL, GetBYTE(adr));
			break;
		case 0x67:			/* LD IYH,A */
			cycles -= cycleTables[3][0x67];
			Sethreg(IY, hreg(AF));
			break;
		case 0x68:			/* LD IYL,B */
			cycles -= cycleTables[3][0x68];
			Setlreg(IY, hreg(BC));
			break;
		case 0x69:			/* LD IYL,C */
			cycles -= cycleTables[3][0x69];
			Setlreg(IY, lreg(BC));
			break;
		case 0x6A:			/* LD IYL,D */
			cycles -= cycleTables[3][0x6A];
			Setlreg(IY, hreg(DE));
			break;
		case 0x6B:			/* LD IYL,E */
			cycles -= cycleTables[3][0x6B];
			Setlreg(IY, lreg(DE));
			break;
		case 0x6C:			/* LD IYL,IYH */
			cycles -= cycleTables[3][0x6C];
			Setlreg(IY, hreg(IY));
			break;
		case 0x6D:			/* LD IYL,IYL */
			cycles -= cycleTables[3][0x6D];
			/* nop */
			break;
		case 0x6E:			/* LD L,(IY+dd) */
			cycles -= cycleTables[3][0x6E];
			adr = IY + (signed char) GetBYTE_pp(pc);
			Setlreg(HL, GetBYTE(adr));
			break;
		case 0x6F:			/* LD IYL,A */
			cycles -= cycleTables[3][0x6F];
			Setlreg(IY, hreg(AF));
			break;
		case 0x70:			/* LD (IY+dd),B */
			cycles -= cycleTables[3][0x70];
			adr = IY + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, hreg(BC));
			break;
		case 0x71:			/* LD (IY+dd),C */
			cycles -= cycleTables[3][0x71];
			adr = IY + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, lreg(BC));
			break;
		case 0x72:			/* LD (IY+dd),D */
			cycles -= cycleTables[3][0x72];
			adr = IY + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, hreg(DE));
			break;
		case 0x73:			/* LD (IY+dd),E */
			cycles -= cycleTables[3][0x73];
			adr = IY + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, lreg(DE));
			break;
		case 0x74:			/* LD (IY+dd),H */
			cycles -= cycleTables[3][0x74];
			adr = IY + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, hreg(HL));
			break;
		case 0x75:			/* LD (IY+dd),L */
			cycles -= cycleTables[3][0x75];
			adr = IY + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, lreg(HL));
			break;
		case 0x77:			/* LD (IY+dd),A */
			cycles -= cycleTables[3][0x77];
			adr = IY + (signed char) GetBYTE_pp(pc);
			PutBYTE(adr, hreg(AF));
			break;
		case 0x7C:			/* LD A,IYH */
			cycles -= cycleTables[3][0x7C];
			Sethreg(AF, hreg(IY));
			break;
		case 0x7D:			/* LD A,IYL */
			cycles -= cycleTables[3][0x7D];
			Sethreg(AF, lreg(IY));
			break;
		case 0x7E:			/* LD A,(IY+dd) */
			cycles -= cycleTables[3][0x7E];
			adr = IY + (signed char) GetBYTE_pp(pc);
			Sethreg(AF, GetBYTE(adr));
			break;
		case 0x84:			/* ADD A,IYH */
			cycles -= cycleTables[3][0x84];
			temp = hreg(IY);
			acu = hreg(AF);
			sum = acu + temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x85:			/* ADD A,IYL */
			cycles -= cycleTables[3][0x85];
			temp = lreg(IY);
			acu = hreg(AF);
			sum = acu + temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x86:			/* ADD A,(IY+dd) */
			cycles -= cycleTables[3][0x86];
			adr = IY + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			acu = hreg(AF);
			sum = acu + temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x8C:			/* ADC A,IYH */
			cycles -= cycleTables[3][0x8C];
			temp = hreg(IY);
			acu = hreg(AF);
			sum = acu + temp + TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x8D:			/* ADC A,IYL */
			cycles -= cycleTables[3][0x8D];
			temp = lreg(IY);
			acu = hreg(AF);
			sum = acu + temp + TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x8E:			/* ADC A,(IY+dd) */
			cycles -= cycleTables[3][0x8E];
			adr = IY + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			acu = hreg(AF);
			sum = acu + temp + TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) |
				((cbits >> 8) & 1);
			break;
		case 0x94:			/* SUB IYH */
			cycles -= cycleTables[3][0x94];
			temp = hreg(IY);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x95:			/* SUB IYL */
			cycles -= cycleTables[3][0x95];
			temp = lreg(IY);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x96:			/* SUB (IY+dd) */
			cycles -= cycleTables[3][0x96];
			adr = IY + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x9C:			/* SBC A,IYH */
			cycles -= cycleTables[3][0x9C];
			temp = hreg(IY);
			acu = hreg(AF);
			sum = acu - temp - TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x9D:			/* SBC A,IYL */
			cycles -= cycleTables[3][0x9D];
			temp = lreg(IY);
			acu = hreg(AF);
			sum = acu - temp - TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0x9E:			/* SBC A,(IY+dd) */
			cycles -= cycleTables[3][0x9E];
			adr = IY + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			acu = hreg(AF);
			sum = acu - temp - TSTFLAG(C);
			cbits = acu ^ temp ^ sum;
			AF = ((sum & 0xff) << 8) | (sum & 0xa8) |
				(((sum & 0xff) == 0) << 6) | (cbits & 0x10) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				((cbits >> 8) & 1);
			break;
		case 0xA4:			/* AND IYH */
			cycles -= cycleTables[3][0xA4];
			sum = ((AF & (IY)) >> 8) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) |
				((sum == 0) << 6) | 0x10 | partab[sum];
			break;
		case 0xA5:			/* AND IYL */
			cycles -= cycleTables[3][0xA5];
			sum = ((AF >> 8) & IY) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | 0x10 |
				((sum == 0) << 6) | partab[sum];
			break;
		case 0xA6:			/* AND (IY+dd) */
			cycles -= cycleTables[3][0xA6];
			adr = IY + (signed char) GetBYTE_pp(pc);
			sum = ((AF >> 8) & GetBYTE(adr)) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | 0x10 |
				((sum == 0) << 6) | partab[sum];
			break;
		case 0xAC:			/* XOR IYH */
			cycles -= cycleTables[3][0xAC];
			sum = ((AF ^ (IY)) >> 8) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xAD:			/* XOR IYL */
			cycles -= cycleTables[3][0xAD];
			sum = ((AF >> 8) ^ IY) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xAE:			/* XOR (IY+dd) */
			cycles -= cycleTables[3][0xAE];
			adr = IY + (signed char) GetBYTE_pp(pc);
			sum = ((AF >> 8) ^ GetBYTE(adr)) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xB4:			/* OR IYH */
			cycles -= cycleTables[3][0xB4];
			sum = ((AF | (IY)) >> 8) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xB5:			/* OR IYL */
			cycles -= cycleTables[3][0xB5];
			sum = ((AF >> 8) | IY) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xB6:			/* OR (IY+dd) */
			cycles -= cycleTables[3][0xB6];
			adr = IY + (signed char) GetBYTE_pp(pc);
			sum = ((AF >> 8) | GetBYTE(adr)) & 0xff;
			AF = (sum << 8) | (sum & 0xa8) | ((sum == 0) << 6) | partab[sum];
			break;
		case 0xBC:			/* CP IYH */
			cycles -= cycleTables[3][0xBC];
			temp = hreg(IY);
			AF = (AF & ~0x28) | (temp & 0x28);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xff) | (sum & 0x80) |
				(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0xBD:			/* CP IYL */
			cycles -= cycleTables[3][0xBD];
			temp = lreg(IY);
			AF = (AF & ~0x28) | (temp & 0x28);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xff) | (sum & 0x80) |
				(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0xBE:			/* CP (IY+dd) */
			cycles -= cycleTables[3][0xBE];
			adr = IY + (signed char) GetBYTE_pp(pc);
			temp = GetBYTE(adr);
			AF = (AF & ~0x28) | (temp & 0x28);
			acu = hreg(AF);
			sum = acu - temp;
			cbits = acu ^ temp ^ sum;
			AF = (AF & ~0xff) | (sum & 0x80) |
				(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
				(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
				(cbits & 0x10) | ((cbits >> 8) & 1);
			break;
		case 0xCB:			/* CB prefix */
			adr = IY + (signed char) GetBYTE_pp(pc);
			adr = adr;
			op = GetBYTE(pc);
			cycles -= cycleTables[4][op];
			switch (op & 7) {
			case 0: ++pc; acu = hreg(BC); break;
			case 1: ++pc; acu = lreg(BC); break;
			case 2: ++pc; acu = hreg(DE); break;
			case 3: ++pc; acu = lreg(DE); break;
			case 4: ++pc; acu = hreg(HL); break;
			case 5: ++pc; acu = lreg(HL); break;
			case 6: ++pc; acu = GetBYTE(adr);  break;
			case 7: ++pc; acu = hreg(AF); break;
			}
			switch (op & 0xc0) {
			case 0x00:		/* shift/rotate */
				switch (op & 0x38) {
				case 0x00:	/* RLC */
					temp = (acu << 1) | (acu >> 7);
					cbits = temp & 1;
					goto cbshflg3;
				case 0x08:	/* RRC */
					temp = (acu >> 1) | (acu << 7);
					cbits = temp & 0x80;
					goto cbshflg3;
				case 0x10:	/* RL */
					temp = (acu << 1) | TSTFLAG(C);
					cbits = acu & 0x80;
					goto cbshflg3;
				case 0x18:	/* RR */
					temp = (acu >> 1) | (TSTFLAG(C) << 7);
					cbits = acu & 1;
					goto cbshflg3;
				case 0x20:	/* SLA */
					temp = acu << 1;
					cbits = acu & 0x80;
					goto cbshflg3;
				case 0x28:	/* SRA */
					temp = (acu >> 1) | (acu & 0x80);
					cbits = acu & 1;
					goto cbshflg3;
				case 0x30:	/* SLIA */
					temp = (acu << 1) | 1;
					cbits = acu & 0x80;
					goto cbshflg3;
				case 0x38:	/* SRL */
					temp = acu >> 1;
					cbits = acu & 1;
				cbshflg3:
					AF = (AF & ~0xff) | (temp & 0xa8) |
						(((temp & 0xff) == 0) << 6) |
						parity(temp) | !!cbits;
				}
				break;
			case 0x40:		/* BIT */
				if (acu & (1 << ((op >> 3) & 7)))
					AF = (AF & ~0xfe) | 0x10 |
					(((op & 0x38) == 0x38) << 7);
				else
					AF = (AF & ~0xfe) | 0x54;
				if ((op&7) != 6)
					AF |= (acu & 0x28);
				temp = acu;
				break;
			case 0x80:		/* RES */
				temp = acu & ~(1 << ((op >> 3) & 7));
				break;
			case 0xc0:		/* SET */
				temp = acu | (1 << ((op >> 3) & 7));
				break;
			}
			switch (op & 7) {
			case 0: Sethreg(BC, temp); break;
			case 1: Setlreg(BC, temp); break;
			case 2: Sethreg(DE, temp); break;
			case 3: Setlreg(DE, temp); break;
			case 4: Sethreg(HL, temp); break;
			case 5: Setlreg(HL, temp); break;
			case 6: PutBYTE(adr, temp);  break;
			case 7: Sethreg(AF, temp); break;
			}
			break;
		case 0xE1:			/* POP IY */
			cycles -= cycleTables[3][0xE1];
			POP(IY);
			break;
		case 0xE3:			/* EX (SP),IY */
			cycles -= cycleTables[3][0xE3];
			temp = IY; POP(IY); PUSH(temp);
			break;
		case 0xE5:			/* PUSH IY */
			cycles -= cycleTables[3][0xE5];
			PUSH(IY);
			break;
		case 0xE9:			/* JP (IY) */
			cycles -= cycleTables[3][0xE9];
			pc = IY;
			break;
		case 0xF9:			/* LD SP,IY */
			cycles -= cycleTables[3][0xF9];
			SP = IY;
			break;
		default: pc--;		/* ignore DD */
		}
		break;
	case 0xFE:			/* CP nn */
		cycles -= cycleTables[0][0xFE];
		temp = GetBYTE_pp(pc);
		AF = (AF & ~0x28) | (temp & 0x28);
		acu = hreg(AF);
		sum = acu - temp;
		cbits = acu ^ temp ^ sum;
		AF = (AF & ~0xff) | (sum & 0x80) |
			(((sum & 0xff) == 0) << 6) | (temp & 0x28) |
			(((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
			(cbits & 0x10) | ((cbits >> 8) & 1);
		break;
	case 0xFF:			/* RST 38H */
		cycles -= cycleTables[0][0xFF];
		PUSH(pc); pc = 0x38;
		break;
    }
    
	// Interrupts
    
    if (nmiTrigger)		// NMI triggered (higher priority than INT)
    {    	
    	/*
    	 * NMI sequence:
    	 *
    	 *	- Push pc on stack
    	 *	- Set pc to NMI vector (0x0066)
    	 *	- Copy IFF1 to IFF2 (save interrupt enable status)
    	 *	- Clear IFF1 (disable interrupts)
    	 *	- Un-halt CPU (if in HALT state)
    	 */

#ifdef SUPERMODEL_DEBUGGER
		if (Debug != NULL)
			Debug->CPUException(Z80_EX_NMI);
#endif // SUPERMODEL_DEBUGGER

    	PUSH(pc);
    	pc = 0x0066;
    	iff = (iff&~2) | ((iff&1)<<1);
    	iff &= ~1;
    	nmiTrigger = false;	// clear NMI
    	// TODO: if in HALTed state, un-halt
    }
    else if (intLine)	// INT asserted
    {
    	// If interrupts are enabled (IFF1 != 0)
    	if ((iff&1))
    	{
    		int	v;
    		
    		/*
    		 * INT sequence:
    		 *
    		 * 	- Disable interrupts (clear IFF1 and IFF2)
    		 *	- Push pc on stack
    		 *	- Un-halt CPU (if in HALT state)
    		 *	- Set pc to vector (which depends on mode)
    		 *
    		 * If no callback is provided when required, nothing happens and
    		 * the interrupt line is cleared. Otherwise, callbacks are
    		 * responsible for clearing the lines themselves.
    		 */
    		// TODO: if in HALTed state, un-halt
    		switch (im)	// interrupt mode (0, 1, or 2 only!)
    		{
    		case 0:
    			/*
    			 * Mode 0:
    			 *
    			 * Fetches up 3 bytes from bus and executes them directly.
    			 * Usually, this will just be an RST instruction, so this is
    			 * all that we accept here.
    			 */
    			if (NULL != INTCallback)
    			{
    				v = INTCallback(this);	

#ifdef SUPERMODEL_DEBUGGER
					if (Debug != NULL)
						Debug->CPUException(v);
#endif // SUPERMODEL_DEBUGGER

    				switch (v)
    				{
    				case Z80_INT_RST_00:	v = 0x0000;	break;
    				case Z80_INT_RST_08:	v = 0x0008;	break;
    				case Z80_INT_RST_10:	v = 0x0010;	break;
    				case Z80_INT_RST_18:	v = 0x0018;	break;
    				case Z80_INT_RST_20:	v = 0x0020;	break;
    				case Z80_INT_RST_28:	v = 0x0028;	break;
    				case Z80_INT_RST_30:	v = 0x0030;	break;
    				case Z80_INT_RST_38:	v = 0x0038;	break;
    				default:				v = -1;		break;	// invalid, do nothing
    				}
    				
    				if (v >= 0)	// valid vector
    				{
    					PUSH(pc);
    					pc = (UINT16) v;
    					iff = 0;
    				}
    			}
    			else	// if no callback, do nothing, clear INT line
    				intLine = false;
    			break;
    		case 1:
    			/*
    			 * Mode 1:
    			 *
    			 * Vector is 0x0038.
    			 */

#ifdef SUPERMODEL_DEBUGGER
				if (Debug != NULL)
					Debug->CPUException(Z80_IM1_IRQ);
#endif // SUPERMODEL_DEBUGGER

    			PUSH(pc);
    			pc = 0x0038;
    			iff = 0;
    			if (NULL != INTCallback)
    				INTCallback(this);
    			else	// no callback, clear INT line automatically
    				intLine = false;
    			break;
    		case 2:
    			/*
    			 * Mode 2:
    			 *
    			 * A 16-bit address is formed by concatenating the I register
    			 * and 8 bits read from the bus (with bit 0 cleared). The final
    			 * 16-bit vector is read from this address.
    			 */

#ifdef SUPERMODEL_DEBUGGER
				if (Debug != NULL)
					Debug->CPUException(Z80_IM2_VECTOR);
#endif // SUPERMODEL_DEBUGGER

    			if (NULL != INTCallback)
    			{
    				v = INTCallback(this);
    				v = (ir&0xFF00) | (v&0xFE);
    				PUSH(pc);
    				pc = GetWORD(v);
    				iff = 0;    				
    			}
    			else	// if no callback, do nothing, clear INT line
    				intLine = false;
    			break;
    		default:	// should never happen (nothing will be done)
    			intLine = false;
    			break;
    		}
    	}
    }
    
    
	}	// end while

	// write registers back to context
HALTExit:	
#ifdef SUPERMODEL_DEBUGGER
	if (Debug != NULL)
	{
		Debug->CPUInactive();
		lastCycles -= cycles;
	}
#else
	// Save local copies of Z80 registers back to context
	af[af_sel] = AF;
    regs[regs_sel].bc = BC;
    regs[regs_sel].de = DE;
    regs[regs_sel].hl = HL;
    ix = IX;
    iy = IY;
    sp = SP;
#endif // SUPERMODEL_DEBUGGER

	// Return number of cycles actually executed
    return numCycles - cycles;
}

void CZ80::TriggerNMI(void)
{
	nmiTrigger = true;
}

void CZ80::SetINT(bool state)
{
	intLine = state;
}

UINT16 CZ80::GetPC(void)
{
	return pc;
}

#ifdef SUPERMODEL_DEBUGGER
UINT8 CZ80::GetReg8(unsigned reg8)
{
	switch (reg8)
	{
		case Z80_REG8_IFF: return iff; 
		case Z80_REG8_IM:  return im; 
		case Z80_REG8_I:   return ir>>8; 
		case Z80_REG8_R:   return ir&0xFF;
		case Z80_REG8_A:   return af[0]>>8;
		case Z80_REG8_F:   return af[0]&0xFF;
		case Z80_REG8_B:   return regs[0].bc>>8; 
		case Z80_REG8_C:   return regs[0].bc&0xFF;
		case Z80_REG8_D:   return regs[0].de>>8;
		case Z80_REG8_E:   return regs[0].de&0xFF;
		case Z80_REG8_H:   return regs[0].hl>>8;
		case Z80_REG8_L:   return regs[0].hl&0xFF;
		default:           return 0;
	}
}

bool CZ80::SetReg8(unsigned reg8, UINT8 value)
{
	switch (reg8)
	{
		case Z80_REG8_IFF: iff = value; return true;
		case Z80_REG8_IM:  im = value; return true; 
		case Z80_REG8_I:   ir |= value<<8; return true; 
		case Z80_REG8_R:   ir |= value; return true;
		case Z80_REG8_A:   af[0] |= value<<8; return true;
		case Z80_REG8_F:   af[0] |= value; return true;
		case Z80_REG8_B:   regs[0].bc |= value<<8; return true; 
		case Z80_REG8_C:   regs[0].bc |= value; return true;
		case Z80_REG8_D:   regs[0].de |= value<<8; return true;
		case Z80_REG8_E:   regs[0].de |= value; return true;
		case Z80_REG8_H:   regs[0].hl |= value<<8; return true;
		case Z80_REG8_L:   regs[0].hl |= value; return true;
		default:           return false;
	}
}

UINT16 CZ80::GetReg16(unsigned reg16)
{
	switch (reg16)
	{
		case Z80_REG16_SP:  return sp;
		case Z80_REG16_PC:  return pc;
		case Z80_REG16_IR:  return ir;
		case Z80_REG16_AF:  return af[0];
		case Z80_REG16_BC:  return regs[0].bc;
		case Z80_REG16_DE:  return regs[0].de;
		case Z80_REG16_HL:  return regs[0].hl;
		case Z80_REG16_IX:  return ix;
		case Z80_REG16_IY:  return iy;
		case Z80_REG16_AF_: return af[1];
		case Z80_REG16_BC_: return regs[1].bc;
		case Z80_REG16_DE_: return regs[1].de;
		case Z80_REG16_HL_: return regs[1].hl;
		default:            return 0;
	}
}

bool CZ80::SetReg16(unsigned reg16, UINT16 value)
{
	switch (reg16)
	{
		case Z80_REG16_SP:  sp = value; return true;
		case Z80_REG16_PC:  pc = value; return true;
		case Z80_REG16_IR:  ir = value; return true;
		case Z80_REG16_AF:  af[0] = value; return true;
		case Z80_REG16_BC:  regs[0].bc = value; return true;
		case Z80_REG16_DE:  regs[0].de = value; return true;
		case Z80_REG16_HL:  regs[0].hl = value; return true;
		case Z80_REG16_IX:  ix = value; return true;
		case Z80_REG16_IY:  iy = value; return true;
		case Z80_REG16_AF_: af[1] = value; return true;
		case Z80_REG16_BC_: regs[1].bc = value; return true;
		case Z80_REG16_DE_: regs[1].de = value; return true;
		case Z80_REG16_HL_: regs[1].hl = value; return true;
		default:            return false;
	}
}
#endif // SUPERMODEL_DEBUGGER

void CZ80::Reset(void)
{
	pc			= 0x0000;
	sp			= 0xF000;
  	af[0]		= 0x0000;
  	af[1]		= 0x0000;
  	regs[0].bc	= 0x0000;
  	regs[0].de	= 0x0000;
  	regs[0].hl	= 0x0000;
  	regs[1].bc	= 0x0000;
  	regs[1].de	= 0x0000;
  	regs[1].hl	= 0x0000;
  	ix			= 0x0000;
  	iy			= 0x0000;
  	ir			= 0x0000;
  	iff			= 0;
  	im			= 0;
  	
  	af_sel 		= 0;
  	regs_sel	= 0;
  	
  	intLine		= false;
  	nmiTrigger	= false;
#ifdef SUPERMODEL_DEBUGGER
	lastCycles  = 0;
#endif // SUPERMODEL_DEBUGGER
}

void CZ80::SaveState(CBlockFile *StateFile, const char *name)
{
	StateFile->NewBlock(name, __FILE__);
	
	for (int i = 0; i < 2; i++)
	{
		StateFile->Write(&regs[i].bc, sizeof(regs[i].bc));
		StateFile->Write(&regs[i].de, sizeof(regs[i].de));
		StateFile->Write(&regs[i].hl, sizeof(regs[i].hl));
	}
	
	StateFile->Write(af, sizeof(af));
	StateFile->Write(&ir, sizeof(ir));
	StateFile->Write(&ix, sizeof(ix));
	StateFile->Write(&iy, sizeof(iy));
	StateFile->Write(&sp, sizeof(sp));
	StateFile->Write(&pc, sizeof(pc));
	StateFile->Write(&iff, sizeof(iff));
	StateFile->Write(&im, sizeof(im));
	StateFile->Write(&regs_sel, sizeof(regs_sel));
	StateFile->Write(&af_sel, sizeof(af_sel));
	StateFile->Write(&nmiTrigger, sizeof(nmiTrigger));
	StateFile->Write(&intLine, sizeof(intLine));
}

void CZ80::LoadState(CBlockFile *StateFile, const char *name)
{
	if (OKAY != StateFile->FindBlock(name))
	{
		ErrorLog("Unable to load Z80 state. Save state file is corrupt.");
		return;
	}
	
	for (int i = 0; i < 2; i++)
	{
		StateFile->Read(&regs[i].bc, sizeof(regs[i].bc));
		StateFile->Read(&regs[i].de, sizeof(regs[i].de));
		StateFile->Read(&regs[i].hl, sizeof(regs[i].hl));
	}
	
	StateFile->Read(af, sizeof(af));
	StateFile->Read(&ir, sizeof(ir));
	StateFile->Read(&ix, sizeof(ix));
	StateFile->Read(&iy, sizeof(iy));
	StateFile->Read(&sp, sizeof(sp));
	StateFile->Read(&pc, sizeof(pc));
	StateFile->Read(&iff, sizeof(iff));
	StateFile->Read(&im, sizeof(im));
	StateFile->Read(&regs_sel, sizeof(regs_sel));
	StateFile->Read(&af_sel, sizeof(af_sel));
	StateFile->Read(&nmiTrigger, sizeof(nmiTrigger));
	StateFile->Read(&intLine, sizeof(intLine));
}

void CZ80::Init(CBus *BusPtr, int (*INTF)(CZ80 *Z80))
{
	Bus 		= BusPtr;
	INTCallback	= INTF;
}

#ifdef SUPERMODEL_DEBUGGER
void CZ80::AttachDebugger(Debugger::CZ80Debug *DebugPtr)
{
	if (Debug != NULL)
		DetachDebugger();
	Debug = DebugPtr;
	Bus = Debug->AttachBus(Bus);
}

void CZ80::DetachDebugger()
{
	if (Debug == NULL)
		return;
	Bus = Debug->DetachBus();
	Debug = NULL;
}
#endif //SUPERMODEL_DEBUGGER

CZ80::CZ80(void)
{
	INTCallback	= NULL;	// so we can later check to see if one has been installed
	Bus 		= NULL;
#ifdef SUPERMODEL_DEBUGGER
	Debug       = NULL;
#endif //SUPERMODEL_DEBUGGER
}

CZ80::~CZ80(void)
{
	INTCallback	= NULL;
	Bus			= NULL;
#ifdef SUPERMODEL_DEBUGGER
	Debug       = NULL;
#endif //SUPERMODEL_DEBUGGER
}