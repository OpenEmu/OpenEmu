/*****************************************************************************
 *
 *   m6502ops.h
 *   Addressing mode and opcode macros for 6502,65c02,65sc02,6510,n2a03 CPUs
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


/***************************************************************
 ***************************************************************
 *          Macros to emulate the 65C02 opcodes
 ***************************************************************
 ***************************************************************/

/* 65C02 *******************************************************
 *  EA = absolute address + X
 * one additional read if page boundary is crossed
 ***************************************************************/
#define EA_ABX_C02_P											\
	EA_ABS;														\
	if ( EAL + X > 0xff ) {										\
		RDMEM( PCW - 1 );										\
	}															\
	EAW += X;

/* 65C02 *******************************************************
 *  EA = absolute address + X
 ***************************************************************/
#define EA_ABX_C02_NP											\
	EA_ABS;														\
	RDMEM( PCW - 1 );											\
	EAW += X;

/***************************************************************
 *  EA = absolute address + Y
 * one additional read if page boundary is crossed
 ***************************************************************/
#define EA_ABY_C02_P											\
	EA_ABS;														\
	if ( EAL + Y > 0xff ) {										\
		RDMEM( PCW - 1 );										\
	}															\
	EAW += Y;

/* 65C02 *******************************************************
 *  EA = absolute address + Y
 ***************************************************************/
#define EA_ABY_C02_NP											\
	EA_ABS;														\
	RDMEM( PCW - 1 );											\
	EAW += Y

/* 65C02 *******************************************************
 *  EA = zero page indirect + Y (post indexed)
 *  subtract 1 cycle if page boundary is crossed
 ***************************************************************/
#define EA_IDY_C02_P											\
	ZPL = RDOPARG();											\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD);											\
	if (EAL + Y > 0xff) {										\
		RDMEM( PCW - 1 );										\
	}															\
	EAW += Y;

/* 65C02 *******************************************************
 *  EA = zero page indirect + Y
 ***************************************************************/
#define EA_IDY_C02_NP											\
	ZPL = RDOPARG();											\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD);											\
	RDMEM( PCW - 1 );											\
	EAW += Y

/* 65C02 *******************************************************
 *  EA = indirect (only used by JMP)
 * correct overflow handling
 ***************************************************************/
#define EA_IND_C02												\
	EA_ABS;														\
	tmp = RDMEM(EAD);											\
	RDMEM(PCW-1);												\
	EAD++;														\
	EAH = RDMEM(EAD);											\
	EAL = tmp

/* 65C02 *******************************************************
 *  EA = indirect plus x (only used by 65c02 JMP)
 ***************************************************************/
#define EA_IAX													\
	EA_ABS;														\
	RDMEM( PCW - 1 );											\
	if (EAL + X > 0xff) {										\
		RDMEM( PCW - 1 );										\
	}															\
	EAW += X;													\
	tmp = RDMEM(EAD);											\
	EAD++;														\
	EAH = RDMEM(EAD);											\
	EAL = tmp

/* read a value into tmp */
/* Base number of cycles taken for each mode (including reading of opcode):
   RD_ABX_C02_P         4/5
   RD_ABX_C02_NP/WR_ABX_C02_NP  5
   RD_ABY_C02_P         4/5
   RD_IDY_C02_P         5/6
   WR_IDY_C02_NP        6
 */
#define RD_ABX_C02_P	EA_ABX_C02_P; tmp = RDMEM(EAD)
#define RD_ABX_C02_NP	EA_ABX_C02_NP; tmp = RDMEM(EAD)
#define RD_ABX_C02_NP_DISCARD	EA_ABX_C02_NP; RDMEM(EAD)
#define RD_ABY_C02_P	EA_ABY_C02_P; tmp = RDMEM(EAD)
#define RD_IDY_C02_P	EA_IDY_C02_P; tmp = RDMEM_ID(EAD); m6502_ICount -= 1

#define WR_ABX_C02_NP	EA_ABX_C02_NP; WRMEM(EAD, tmp)
#define WR_ABY_C02_NP	EA_ABY_C02_NP; WRMEM(EAD, tmp)
#define WR_IDY_C02_NP	EA_IDY_C02_NP; WRMEM_ID(EAD, tmp); m6502_ICount -= 1


/* 65C02********************************************************
 *  BRA  branch relative
 *  extra cycle if page boundary is crossed
 ***************************************************************/
#define BRA_C02(cond)											\
	tmp = RDOPARG();											\
	if (cond)													\
	{															\
		RDMEM(PCW);												\
		EAW = PCW + (signed char)tmp;							\
		if ( EAH != PCH ) {										\
			RDMEM( PCW - 1 );									\
		}														\
		PCD = EAD;												\
		CHANGE_PC;												\
	}

/* 65C02 ********************************************************
 *  ADC Add with carry
 * different setting of flags in decimal mode
 ***************************************************************/
#define ADC_C02 												\
	if (P & F_D)												\
	{															\
		int c = (P & F_C);										\
		int lo = (A & 0x0f) + (tmp & 0x0f) + c; 				\
		int hi = (A & 0xf0) + (tmp & 0xf0); 					\
		P &= ~(F_V | F_C);										\
		if( lo > 0x09 ) 										\
		{														\
			hi += 0x10; 										\
			lo += 0x06; 										\
		}														\
		if( ~(A^tmp) & (A^hi) & F_N )							\
			P |= F_V;											\
		if( hi > 0x90 ) 										\
			hi += 0x60; 										\
		if( hi & 0xff00 )										\
			P |= F_C;											\
		A = (lo & 0x0f) + (hi & 0xf0);							\
		RDMEM( PCW - 1 );										\
	}															\
	else														\
	{															\
		int c = (P & F_C);										\
		int sum = A + tmp + c;									\
		P &= ~(F_V | F_C);										\
		if( ~(A^tmp) & (A^sum) & F_N )							\
			P |= F_V;											\
		if( sum & 0xff00 )										\
			P |= F_C;											\
		A = (UINT8) sum;										\
	}															\
	SET_NZ(A)

/* 65C02 ********************************************************
 *  SBC Subtract with carry
 * different setting of flags in decimal mode
 ***************************************************************/
#define SBC_C02													\
	if (P & F_D)												\
	{															\
		int c = (P & F_C) ^ F_C;								\
		int sum = A - tmp - c;									\
		int lo = (A & 0x0f) - (tmp & 0x0f) - c; 				\
		int hi = (A & 0xf0) - (tmp & 0xf0); 					\
		P &= ~(F_V | F_C);										\
		if( (A^tmp) & (A^sum) & F_N )							\
			P |= F_V;											\
		if( lo & 0xf0 ) 										\
			lo -= 6;											\
		if( lo & 0x80 ) 										\
			hi -= 0x10; 										\
		if( hi & 0x0f00 )										\
			hi -= 0x60; 										\
		if( (sum & 0xff00) == 0 )								\
			P |= F_C;											\
		A = (lo & 0x0f) + (hi & 0xf0);							\
		RDMEM( PCW - 1 );										\
	}															\
	else														\
	{															\
		int c = (P & F_C) ^ F_C;								\
		int sum = A - tmp - c;									\
		P &= ~(F_V | F_C);										\
		if( (A^tmp) & (A^sum) & F_N )							\
			P |= F_V;											\
		if( (sum & 0xff00) == 0 )								\
			P |= F_C;											\
		A = (UINT8) sum;										\
	}															\
	SET_NZ(A)

/* 65C02 *******************************************************
 *  BBR Branch if bit is reset
 ***************************************************************/
#define BBR(bit)												\
	BRA(!(tmp & (1<<bit)))

/* 65C02 *******************************************************
 *  BBS Branch if bit is set
 ***************************************************************/
#define BBS(bit)												\
	BRA(tmp & (1<<bit))

/* 65c02 ********************************************************
 *  BRK Break
 *  increment PC, push PC hi, PC lo, flags (with B bit set),
 *  set I flag, reset D flag and jump via IRQ vector
 ***************************************************************/
#define BRK_C02 												\
	RDOPARG();													\
	PUSH(PCH);													\
	PUSH(PCL);													\
	PUSH(P | F_B);												\
	P = (P | F_I) & ~F_D;										\
	PCL = RDMEM(M6502_IRQ_VEC); 								\
	PCH = RDMEM(M6502_IRQ_VEC+1);								\
	CHANGE_PC


/* 65C02 *******************************************************
 *  DEA Decrement accumulator
 ***************************************************************/
#define DEA 													\
	A = (UINT8)--A; 											\
	SET_NZ(A)

/* 65C02 *******************************************************
 *  INA Increment accumulator
 ***************************************************************/
#define INA 													\
	A = (UINT8)++A; 											\
	SET_NZ(A)

/* 65C02 *******************************************************
 *  PHX Push index X
 ***************************************************************/
#define PHX 													\
	PUSH(X)

/* 65C02 *******************************************************
 *  PHY Push index Y
 ***************************************************************/
#define PHY 													\
	PUSH(Y)

/* 65C02 *******************************************************
 *  PLX Pull index X
 ***************************************************************/
#define PLX 													\
	RDMEM(SPD);													\
	PULL(X);													\
	SET_NZ(X)

/* 65C02 *******************************************************
 *  PLY Pull index Y
 ***************************************************************/
#define PLY 													\
	RDMEM(SPD);													\
	PULL(Y);													\
	SET_NZ(Y)

/* 65C02 *******************************************************
 *  RMB Reset memory bit
 ***************************************************************/
#define RMB(bit)												\
	tmp &= ~(1<<bit)

/* 65C02 *******************************************************
 *  SMB Set memory bit
 ***************************************************************/
#define SMB(bit)												\
	tmp |= (1<<bit)

/* 65C02 *******************************************************
 * STZ  Store zero
 ***************************************************************/
#define STZ 													\
	tmp = 0

/* 65C02 *******************************************************
 * TRB  Test and reset bits
 ***************************************************************/
#define TRB 													\
	SET_Z(tmp&A);												\
	tmp &= ~A

/* 65C02 *******************************************************
 * TSB  Test and set bits
 ***************************************************************/
#define TSB 													\
	SET_Z(tmp&A);												\
	tmp |= A


/* 6502 ********************************************************
 *  BIT Bit test Immediate, only Z affected
 ***************************************************************/
#undef BIT_IMM_C02
#define BIT_IMM_C02												\
	P &= ~(F_Z);												\
	if ((tmp & A) == 0) 										\
		P |= F_Z


/***************************************************************
 ***************************************************************
 *          Macros to emulate the 65sc02 opcodes
 ***************************************************************
 ***************************************************************/


/* 65sc02 ********************************************************
 *  BSR Branch to subroutine
 ***************************************************************/
#define BSR 													\
	EAL = RDOPARG();											\
	RDMEM(SPD);													\
	PUSH(PCH);													\
	PUSH(PCL);													\
	EAH = RDOPARG();											\
	EAW = PCW + (INT16)(EAW-1); 								\
	PCD = EAD;													\
	CHANGE_PC
