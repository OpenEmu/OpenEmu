#pragma once

#ifndef __ILL02_H__
#define __ILL02_H__

/*****************************************************************************
 *
 *   ill02.h
 *   Addressing mode and opcode macros for the NMOS 6502 illegal opcodes
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   65sc02 core Copyright Peter Trauner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

/* test with the excellent C64 Emulator test suite
   ? at www.funet.fi/pub/cbm/documents/chipdata/tsuit215.zip
   good reference in the vice emulator (source) distribution doc/64doc.txt

   $ab=OAL like in 6502-NMOS.extra.opcodes, vice so in vice (lxa)
*/

/***************************************************************
 ***************************************************************
 *          Macros to emulate the 6510 opcodes
 ***************************************************************
 ***************************************************************/

/* 6510 ********************************************************
 *  ANC logical and, set carry from bit of A
 ***************************************************************/
#define ANC 													\
	P &= ~F_C;													\
	A = (UINT8)(A & tmp);										\
	if (A & 0x80)												\
		P |= F_C;												\
	SET_NZ(A)

/* 6510 ********************************************************
 *  ASR logical and, logical shift right
 ***************************************************************/
#define ASR 													\
	tmp &= A; 									\
	LSR

/* 6510 ********************************************************
 * AST  and stack; transfer to accumulator and index X
 * logical and stack (LSB) with data, transfer result to S
 * transfer result to accumulator and index X also
 ***************************************************************/
#define AST 													\
	S &= tmp;													\
	A = X = S;													\
	SET_NZ(A)

/* 6510 ********************************************************
 *  ARR logical and, rotate right
 ***************************************************************/
#define ARR 													\
	if( P & F_D )												\
	{															\
		int lo, hi, t;											\
		tmp &= A;												\
		t = tmp;												\
		hi = tmp &0xf0; 										\
		lo = tmp &0x0f; 										\
		if( P & F_C )											\
		{														\
			tmp = (tmp >> 1) | 0x80;							\
			P |= F_N;											\
		}														\
		else													\
		{														\
			tmp >>= 1;											\
			P &= ~F_N;											\
		}														\
		if( tmp )												\
			P &= ~F_Z;											\
		else													\
            P |= F_Z;                                           \
		if( (t^tmp) & 0x40 )									\
			P|=F_V; 											\
		else													\
			P &= ~F_V;											\
		if( lo + (lo & 0x01) > 0x05 )							\
			tmp = (tmp & 0xf0) | ((tmp+6) & 0xf);				\
		if( hi + (hi & 0x10) > 0x50 )							\
		{														\
			P |= F_C;											\
			tmp = (tmp+0x60) & 0xff;							\
		}														\
		else													\
			P &= ~F_C;											\
	}															\
	else														\
	{															\
		tmp &= A;												\
		ROR;													\
		P &=~(F_V|F_C); 										\
		if( tmp & 0x40 )										\
			P|=F_C; 											\
		if( (tmp & 0x60) == 0x20 || (tmp & 0x60) == 0x40 )		\
			P|=F_V; 											\
	}

/* 6510 ********************************************************
 *  ASX logical and X w/ A, subtract data from X
 ***************************************************************/
#define ASX 													\
	P &= ~F_C;													\
	X &= A; 													\
	if (X >= tmp)												\
		P |= F_C;												\
	X = (UINT8)(X - tmp);										\
	SET_NZ(X)

/* 6510 ********************************************************
 *  AXA transfer index X to accumulator, logical and
 * depends on the data of the dma device (videochip) fetched
 * between opcode read and operand read
 ***************************************************************/
#define AXA 													\
	A = (UINT8)( (A|0xee)& X & tmp);							\
	SET_NZ(A)

/* 6510 ********************************************************
 *  DCP decrement data and compare
 ***************************************************************/
#define DCP 													\
	tmp = (UINT8)(tmp-1); 										\
	P &= ~F_C;													\
	if (A >= tmp)												\
		P |= F_C;												\
	SET_NZ((UINT8)(A - tmp))

/* 6502 ********************************************************
 *  DOP double no operation
 ***************************************************************/
#define DOP 													\
	RDOPARG()

/* 6510 ********************************************************
 *  ISB increment and subtract with carry
 ***************************************************************/
#define ISB 													\
	tmp = (UINT8)(tmp+1); 										\
	SBC

/* 6510 ********************************************************
 *  LAX load accumulator and index X
 ***************************************************************/
#define LAX 													\
	A = X = (UINT8)tmp; 										\
	SET_NZ(A)

/* 6510 ********************************************************
 *  OAL load accumulator and index X
 ***************************************************************/
#define OAL 													\
	A = X = (UINT8)((A|0xee)&tmp);								\
	SET_NZ(A)

/* 6510 ********************************************************
 * RLA  rotate left and logical and accumulator
 *  new C <- [7][6][5][4][3][2][1][0] <- C
 ***************************************************************/
#define RLA 													\
	tmp = (tmp << 1) | (P & F_C);								\
	P = (P & ~F_C) | ((tmp >> 8) & F_C);						\
	tmp = (UINT8)tmp;											\
	A &= tmp;													\
	SET_NZ(A)

/* 6510 ********************************************************
 * RRA  rotate right and add with carry
 *  C -> [7][6][5][4][3][2][1][0] -> C
 ***************************************************************/
#define RRA 													\
	tmp |= (P & F_C) << 8;										\
	P = (P & ~F_C) | (tmp & F_C);								\
	tmp = (UINT8)(tmp >> 1);									\
	ADC

/* 6510 ********************************************************
 * SAX  logical and accumulator with index X and store
 ***************************************************************/
#define SAX 													\
	tmp = A & X

/* 6510 ********************************************************
 *  SLO shift left and logical or
 ***************************************************************/
#define SLO 													\
	P = (P & ~F_C) | ((tmp >> 7) & F_C);						\
	tmp = (UINT8)(tmp << 1);									\
	A |= tmp;													\
	SET_NZ(A)

/* 6510 ********************************************************
 *  SRE logical shift right and logical exclusive or
 *  0 -> [7][6][5][4][3][2][1][0] -> C
 ***************************************************************/
#define SRE 													\
	P = (P & ~F_C) | (tmp & F_C);								\
	tmp = (UINT8)tmp >> 1;										\
	A ^= tmp;													\
	SET_NZ(A)

/* 6510 ********************************************************
 * SAH  store accumulator and index X and high + 1
 * result = accumulator and index X and memory [PC+1] + 1
 ***************************************************************/
#define SAH tmp = A & X & (EAH+1)

/* 6510 ********************************************************
 * SSH  store stack high
 * logical and accumulator with index X, transfer result to S
 * logical and result with memory [PC+1] + 1
 ***************************************************************/
#define SSH 													\
	S = A & X;													\
    tmp = S & (EAH+1)
#if 0
#define SSH 													\
	tmp = S = A & X;											\
	tmp &= (UINT8)(cpu_readop_arg((PCW + 1) & 0xffff) + 1)
#endif

/* 6510 ********************************************************
 * SXH  store index X high
 * logical and index X with memory[PC+1] and store the result
 ***************************************************************/
#define SXH tmp = X & (EAH+1)

/* 6510 ********************************************************
 * SYH  store index Y and (high + 1)
 * logical and index Y with memory[PC+1] + 1 and store the result
 ***************************************************************/
#define SYH tmp = Y & (EAH+1)

/* 6510 ********************************************************
 *  TOP triple no operation
 ***************************************************************/
#define TOP 													\
	PCW+=2

/* 6510 ********************************************************
 *  KIL Illegal opcode
 * processor halted: no hardware interrupt will help,
 * only reset
 ***************************************************************/
#define KIL 													\
	PCW--;														\

/* N2A03 *******************************************************
 *  ARR logical and, rotate right - no decimal mode
 ***************************************************************/
#define ARR_NES												\
	{												\
		tmp &= A;										\
		ROR;											\
		P &=~(F_V|F_C);										\
		if( tmp & 0x40 )									\
			P|=F_C;										\
		if( (tmp & 0x60) == 0x20 || (tmp & 0x60) == 0x40 )					\
			P|=F_V;										\
	}

/* N2A03 *******************************************************
 *  ISB increment and subtract with carry
 ***************************************************************/
#define ISB_NES													\
	tmp = (UINT8)(tmp+1);										\
	SBC_NES

/* N2A03 *******************************************************
 * RRA  rotate right and add with carry
 *  C -> [7][6][5][4][3][2][1][0] -> C
 ***************************************************************/
#define RRA_NES													\
	tmp |= (P & F_C) << 8;										\
	P = (P & ~F_C) | (tmp & F_C);								\
	tmp = (UINT8)(tmp >> 1);									\
	ADC_NES

/* N2A03 *******************************************************
 *  OAL load accumulator and index X
 ***************************************************************/
#define OAL_NES													\
	A = X = (UINT8)((A|0xff)&tmp);								\
	SET_NZ(A)

/* N2A03 *******************************************************
 * SXH  store index X high
 * logical and index X with memory[PC+1] and store the result
 *
 * This instruction writes to an odd address when crossing
 * a page boundary. The one known test case can be explained
 * with a shift of Y. More testing will be needed to determine
 * if this is correct.
 *
 ***************************************************************/
#define SXH_NES													\
	if ( Y && Y > EAL )											\
		EAH |= ( Y << 1 );										\
	tmp = X & (EAH+1)

/* N2A03 *******************************************************
 * SYH  store index Y and (high + 1)
 * logical and index Y with memory[PC+1] + 1 and store the result
 *
 * This instruction writs to an odd address when crossing a
 * a page boundary. The one known test case can be explained
 * with a shoft of X. More testing will be needed to determine
 * if this is correct.
 *
 ***************************************************************/
#define SYH_NES													\
	if ( X && X > EAL )											\
		EAH |= ( X << 1 );										\
	tmp = Y & (EAH+1)

#endif /* __ILL02_H__ */
