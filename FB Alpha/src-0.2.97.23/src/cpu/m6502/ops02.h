/*****************************************************************************
 *
 *   ops02.h
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


/* 6502 flags */
#define F_C 0x01
#define F_Z 0x02
#define F_I 0x04
#define F_D 0x08
#define F_B 0x10
#define F_T 0x20
#define F_V 0x40
#define F_N 0x80

/* some shortcuts for improved readability */
#define A	m6502.a
#define X	m6502.x
#define Y	m6502.y
#define P	m6502.p
#define S	m6502.sp.b.l
#define SPD m6502.sp.d

#define NZ	m6502.nz

#define SET_NZ(n)				\
	if ((n) == 0) P = (P & ~F_N) | F_Z; else P = (P & ~(F_N | F_Z)) | ((n) & F_N)

#define SET_Z(n)				\
	if ((n) == 0) P |= F_Z; else P &= ~F_Z

#define EAL m6502.ea.b.l
#define EAH m6502.ea.b.h
#define EAW m6502.ea.w.l
#define EAD m6502.ea.d

#define ZPL m6502.zp.b.l
#define ZPH m6502.zp.b.h
#define ZPW m6502.zp.w.l
#define ZPD m6502.zp.d

#define PCL m6502.pc.b.l
#define PCH m6502.pc.b.h
#define PCW m6502.pc.w.l
#define PCD m6502.pc.d

#define PPC m6502.ppc.d

//#define RDMEM_ID(a)		m6502.rdmem_id(Machine,a)
//#define WRMEM_ID(a,d)	m6502.wrmem_id(Machine,a,d)

#define RDMEM_ID(a)		M6502ReadMemIndex(a)
#define WRMEM_ID(a,d)	M6502WriteMemIndex(a, d)

#define CHANGE_PC change_pc(PCD)

/***************************************************************
 *  RDOP    read an opcode
 ***************************************************************/
//#define RDOP() cpu_readop(PCW++); m6502_ICount -= 1
#define RDOP() M6502ReadOp(PCW++); m6502_ICount -= 1

/***************************************************************
 *  RDOPARG read an opcode argument
 ***************************************************************/
//#define RDOPARG() cpu_readop_arg(PCW++); m6502_ICount -= 1
#define RDOPARG() M6502ReadOpArg(PCW++); m6502_ICount -= 1

/***************************************************************
 *  RDMEM   read memory
 ***************************************************************/
//#define RDMEM(addr) program_read_byte_8le(addr); m6502_ICount -= 1
#define RDMEM(addr) M6502ReadByte(addr); m6502_ICount -= 1

/***************************************************************
 *  WRMEM   write memory
 ***************************************************************/
//#define WRMEM(addr,data) program_write_byte_8le(addr,data); m6502_ICount -= 1
#define WRMEM(addr,data) M6502WriteByte(addr, data); m6502_ICount -= 1

/***************************************************************
 *  BRA  branch relative
 *  extra cycle if page boundary is crossed
 ***************************************************************/
#define BRA(cond)												\
	{																\
		INT8 tmp2 = RDOPARG();										\
		if (cond)													\
		{															\
			RDMEM(PCW);												\
			EAW = PCW + (signed char)tmp2;							\
			if ( EAH != PCH ) {										\
				RDMEM( (PCH << 8 ) | EAL) ;							\
			}														\
			PCD = EAD;												\
			CHANGE_PC;												\
		}															\
	}

/***************************************************************
 *
 * Helper macros to build the effective address
 *
 ***************************************************************/

/***************************************************************
 *  EA = zero page address
 ***************************************************************/
#define EA_ZPG													\
	ZPL = RDOPARG();											\
	EAD = ZPD

/***************************************************************
 *  EA = zero page address + X
 ***************************************************************/
#define EA_ZPX													\
	ZPL = RDOPARG();											\
	RDMEM(ZPD);													\
	ZPL = X + ZPL;												\
	EAD = ZPD

/***************************************************************
 *  EA = zero page address + Y
 ***************************************************************/
#define EA_ZPY													\
	ZPL = RDOPARG();											\
	RDMEM(ZPD);													\
	ZPL = Y + ZPL;												\
	EAD = ZPD

/***************************************************************
 *  EA = absolute address
 ***************************************************************/
#define EA_ABS													\
	EAL = RDOPARG();											\
	EAH = RDOPARG()

/***************************************************************
 *  EA = absolute address + X
 * one additional read if page boundary is crossed
 ***************************************************************/
#define EA_ABX_P												\
	EA_ABS; 													\
	if ( EAL + X > 0xff ) {										\
		RDMEM( ( EAH << 8 ) | ( ( EAL + X ) & 0xff ) );			\
	}															\
	EAW += X;

/***************************************************************
 *  EA = absolute address + X
 ***************************************************************/
#define EA_ABX_NP												\
	EA_ABS;														\
	RDMEM( ( EAH << 8 ) | ( ( EAL + X ) & 0xff ) );				\
	EAW += X

/***************************************************************
 *  EA = absolute address + Y
 * one additional read if page boundary is crossed
 ***************************************************************/
#define EA_ABY_P												\
	EA_ABS; 													\
	if ( EAL + Y > 0xff ) {										\
		RDMEM( ( EAH << 8 ) | ( ( EAL + Y ) & 0xff ) );			\
	}															\
	EAW += Y;

/***************************************************************
 *  EA = absolute address + Y
 ***************************************************************/
#define EA_ABY_NP												\
	EA_ABS;														\
	RDMEM( ( EAH << 8 ) | ( ( EAL + Y ) & 0xff ) );				\
	EAW += Y

/***************************************************************
 *  EA = zero page + X indirect (pre indexed)
 ***************************************************************/
#define EA_IDX													\
	ZPL = RDOPARG();											\
	RDMEM(ZPD);													\
	ZPL = ZPL + X;												\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD)

/***************************************************************
 *  EA = zero page indirect + Y (post indexed)
 *  subtract 1 cycle if page boundary is crossed
 ***************************************************************/
#define EA_IDY_P												\
	ZPL = RDOPARG();											\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD);											\
	if (EAL + Y > 0xff) {										\
		RDMEM( ( EAH << 8 ) | ( ( EAL + Y ) & 0xff ) );			\
	}															\
	EAW += Y;

/***************************************************************
 *  EA = zero page indirect + Y
 ***************************************************************/
#define EA_IDY_NP												\
	ZPL = RDOPARG();											\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD);											\
	RDMEM( ( EAH << 8 ) | ( ( EAL + Y ) & 0xff ) );				\
	EAW += Y

/***************************************************************
 *  EA = zero page indirect (65c02 pre indexed w/o X)
 ***************************************************************/
#define EA_ZPI													\
        ZPL = RDOPARG();										\
        EAL = RDMEM(ZPD);										\
        ZPL++;													\
        EAH = RDMEM(ZPD)

/***************************************************************
 *  EA = indirect (only used by JMP)
 ***************************************************************/
#define EA_IND													\
	EA_ABS; 													\
	tmp = RDMEM(EAD);											\
	EAL++;	/* booby trap: stay in same page! ;-) */			\
	EAH = RDMEM(EAD);											\
	EAL = tmp


/* read a value into tmp */
/* Base number of cycles taken for each mode (including reading of opcode):
   RD_IMM       2
   RD_DUM       2
   RD_ACC       0
   RD_ZPG/WR_ZPG    3
   RD_ZPX/WR_ZPX    4
   RD_ZPY/WR_ZPY    4
   RD_ABS/WR_ABS    4
   RD_ABX_P     4/5
   RD_ABX_NP/WR_ABX_NP  5
   RD_ABY_P     4/5
   RD_ABY_NP/WR_ABY_NP  5
   RD_IDX/WR_IDX    6
   RD_IDY_P     5/6
   RD_IDY_NP/WR_IDY_NP  6
   RD_ZPI/WR_ZPI    5
 */
#define RD_IMM		tmp = RDOPARG()
#define RD_DUM		RDMEM(PCW)
#define RD_ACC		tmp = A
#define RD_ZPG		EA_ZPG; tmp = RDMEM(EAD)
#define RD_ZPX		EA_ZPX; tmp = RDMEM(EAD)
#define RD_ZPY		EA_ZPY; tmp = RDMEM(EAD)
#define RD_ABS		EA_ABS; tmp = RDMEM(EAD)
#define RD_ABX_P	EA_ABX_P; tmp = RDMEM(EAD)
#define RD_ABX_NP	EA_ABX_NP; tmp = RDMEM(EAD)
#define RD_ABY_P	EA_ABY_P; tmp = RDMEM(EAD)
#define RD_ABY_NP	EA_ABY_NP; tmp = RDMEM(EAD)
#define RD_IDX		EA_IDX; tmp = RDMEM_ID(EAD); m6502_ICount -= 1
#define RD_IDY_P	EA_IDY_P; tmp = RDMEM_ID(EAD); m6502_ICount -= 1
#define RD_IDY_NP	EA_IDY_NP; tmp = RDMEM_ID(EAD); m6502_ICount -= 1
#define RD_ZPI		EA_ZPI; tmp = RDMEM(EAD)

/* write a value from tmp */
#define WR_ZPG		EA_ZPG; WRMEM(EAD, tmp)
#define WR_ZPX		EA_ZPX; WRMEM(EAD, tmp)
#define WR_ZPY		EA_ZPY; WRMEM(EAD, tmp)
#define WR_ABS		EA_ABS; WRMEM(EAD, tmp)
#define WR_ABX_NP	EA_ABX_NP; WRMEM(EAD, tmp)
#define WR_ABY_NP	EA_ABY_NP; WRMEM(EAD, tmp)
#define WR_IDX		EA_IDX; WRMEM_ID(EAD, tmp); m6502_ICount -= 1
#define WR_IDY_NP	EA_IDY_NP; WRMEM_ID(EAD, tmp); m6502_ICount -= 1
#define WR_ZPI		EA_ZPI; WRMEM(EAD, tmp)

/* dummy read from the last EA */
#define RD_EA	RDMEM(EAD)

/* write back a value from tmp to the last EA */
#define WB_ACC	A = (UINT8)tmp;
#define WB_EA	WRMEM(EAD, tmp)

/***************************************************************
 ***************************************************************
 *          Macros to emulate the plain 6502 opcodes
 ***************************************************************
 ***************************************************************/

/***************************************************************
 * push a register onto the stack
 ***************************************************************/
#define PUSH(Rg) WRMEM(SPD, Rg); S--

/***************************************************************
 * pull a register from the stack
 ***************************************************************/
#define PULL(Rg) S++; Rg = RDMEM(SPD)

/* 6502 ********************************************************
 *  ADC Add with carry
 ***************************************************************/
#define ADC 													\
	if (P & F_D) {												\
	int c = (P & F_C);											\
	int lo = (A & 0x0f) + (tmp & 0x0f) + c; 					\
	int hi = (A & 0xf0) + (tmp & 0xf0); 						\
		P &= ~(F_V | F_C|F_N|F_Z);								\
		if (!((lo+hi)&0xff)) P|=F_Z;							\
		if (lo > 0x09) {										\
			hi += 0x10; 										\
			lo += 0x06; 										\
		}														\
		if (hi&0x80) P|=F_N;									\
		if (~(A^tmp) & (A^hi) & F_N)							\
			P |= F_V;											\
		if (hi > 0x90)											\
			hi += 0x60; 										\
		if (hi & 0xff00)										\
			P |= F_C;											\
		A = (lo & 0x0f) + (hi & 0xf0);							\
	} else {													\
		int c = (P & F_C);										\
		int sum = A + tmp + c;									\
		P &= ~(F_V | F_C);										\
		if (~(A^tmp) & (A^sum) & F_N)							\
			P |= F_V;											\
		if (sum & 0xff00)										\
			P |= F_C;											\
		A = (UINT8) sum;										\
		SET_NZ(A);												\
	}

/* 6502 ********************************************************
 *  AND Logical and
 ***************************************************************/
#define AND 													\
	A = (UINT8)(A & tmp);										\
	SET_NZ(A)

/* 6502 ********************************************************
 *  ASL Arithmetic shift left
 ***************************************************************/
#define ASL 													\
	P = (P & ~F_C) | ((tmp >> 7) & F_C);						\
	tmp = (UINT8)(tmp << 1);									\
	SET_NZ(tmp)

/* 6502 ********************************************************
 *  BCC Branch if carry clear
 ***************************************************************/
#define BCC BRA(!(P & F_C))

/* 6502 ********************************************************
 *  BCS Branch if carry set
 ***************************************************************/
#define BCS BRA(P & F_C)

/* 6502 ********************************************************
 *  BEQ Branch if equal
 ***************************************************************/
#define BEQ BRA(P & F_Z)

/* 6502 ********************************************************
 *  BIT Bit test
 ***************************************************************/
#undef BIT
#define BIT 													\
	P &= ~(F_N|F_V|F_Z);										\
	P |= tmp & (F_N|F_V);										\
	if ((tmp & A) == 0) 										\
		P |= F_Z

/* 6502 ********************************************************
 *  BMI Branch if minus
 ***************************************************************/
#define BMI BRA(P & F_N)

/* 6502 ********************************************************
 *  BNE Branch if not equal
 ***************************************************************/
#define BNE BRA(!(P & F_Z))

/* 6502 ********************************************************
 *  BPL Branch if plus
 ***************************************************************/
#define BPL BRA(!(P & F_N))

/* 6502 ********************************************************
 *  BRK Break
 *  increment PC, push PC hi, PC lo, flags (with B bit set),
 *  set I flag, jump via IRQ vector
 ***************************************************************/
#define BRK 													\
	RDOPARG();													\
	PUSH(PCH);													\
	PUSH(PCL);													\
	PUSH(P | F_B);												\
	P = (P | F_I);												\
	PCL = RDMEM(M6502_IRQ_VEC); 								\
	PCH = RDMEM(M6502_IRQ_VEC+1);								\
	CHANGE_PC

/* 6502 ********************************************************
 * BVC  Branch if overflow clear
 ***************************************************************/
#define BVC BRA(!(P & F_V))

/* 6502 ********************************************************
 * BVS  Branch if overflow set
 ***************************************************************/
#define BVS BRA(P & F_V)

/* 6502 ********************************************************
 * CLC  Clear carry flag
 ***************************************************************/
#define CLC 													\
	P &= ~F_C

/* 6502 ********************************************************
 * CLD  Clear decimal flag
 ***************************************************************/
#define CLD 													\
	P &= ~F_D

/* 6502 ********************************************************
 * CLI  Clear interrupt flag
 ***************************************************************/
#define CLI 													\
	if ((m6502.irq_state != M6502_CLEAR_LINE) && (P & F_I)) { 		\
		m6502.after_cli = 1;									\
	}															\
	P &= ~F_I

/* 6502 ********************************************************
 * CLV  Clear overflow flag
 ***************************************************************/
#define CLV 													\
	P &= ~F_V

/* 6502 ********************************************************
 *  CMP Compare accumulator
 ***************************************************************/
#define CMP 													\
	P &= ~F_C;													\
	if (A >= tmp)												\
		P |= F_C;												\
	SET_NZ((UINT8)(A - tmp))

/* 6502 ********************************************************
 *  CPX Compare index X
 ***************************************************************/
#define CPX 													\
	P &= ~F_C;													\
	if (X >= tmp)												\
		P |= F_C;												\
	SET_NZ((UINT8)(X - tmp))

/* 6502 ********************************************************
 *  CPY Compare index Y
 ***************************************************************/
#define CPY 													\
	P &= ~F_C;													\
	if (Y >= tmp)												\
		P |= F_C;												\
	SET_NZ((UINT8)(Y - tmp))

/* 6502 ********************************************************
 *  DEC Decrement memory
 ***************************************************************/
#define DEC 													\
	tmp = (UINT8)(tmp-1); 										\
	SET_NZ(tmp)

/* 6502 ********************************************************
 *  DEX Decrement index X
 ***************************************************************/
#define DEX 													\
	X = (UINT8)(X-1); 											\
	SET_NZ(X)

/* 6502 ********************************************************
 *  DEY Decrement index Y
 ***************************************************************/
#define DEY 													\
	Y = (UINT8)(Y-1); 											\
	SET_NZ(Y)

/* 6502 ********************************************************
 *  EOR Logical exclusive or
 ***************************************************************/
#define EOR 													\
	A = (UINT8)(A ^ tmp);										\
	SET_NZ(A)

/* 6502 ********************************************************
 *  ILL Illegal opcode
 ***************************************************************/
#define ILL 													
//	logerror("M6502 illegal opcode %04x: %02x\n",(PCW-1)&0xffff, cpu_readop((PCW-1)&0xffff))

/* 6502 ********************************************************
 *  INC Increment memory
 ***************************************************************/
#define INC 													\
	tmp = (UINT8)(tmp+1); 										\
	SET_NZ(tmp)

/* 6502 ********************************************************
 *  INX Increment index X
 ***************************************************************/
#define INX 													\
	X = (UINT8)(X+1); 											\
	SET_NZ(X)

/* 6502 ********************************************************
 *  INY Increment index Y
 ***************************************************************/
#define INY 													\
	Y = (UINT8)(Y+1); 											\
	SET_NZ(Y)

/* 6502 ********************************************************
 *  JMP Jump to address
 *  set PC to the effective address
 ***************************************************************/
#define JMP 													\
	if( EAD == PPC && !m6502.pending_irq && !m6502.after_cli )	\
		if( m6502_ICount > 0 ) m6502_ICount = 0;				\
	PCD = EAD;													\
	CHANGE_PC

/* 6502 ********************************************************
 *  JSR Jump to subroutine
 *  decrement PC (sic!) push PC hi, push PC lo and set
 *  PC to the effective address
 ***************************************************************/
#define JSR 													\
	EAL = RDOPARG();											\
	RDMEM(SPD);													\
	PUSH(PCH);													\
	PUSH(PCL);													\
	EAH = RDOPARG();											\
	PCD = EAD;													\
	CHANGE_PC

/* 6502 ********************************************************
 *  LDA Load accumulator
 ***************************************************************/
#define LDA 													\
	A = (UINT8)tmp; 											\
	SET_NZ(A)

/* 6502 ********************************************************
 *  LDX Load index X
 ***************************************************************/
#define LDX 													\
	X = (UINT8)tmp; 											\
	SET_NZ(X)

/* 6502 ********************************************************
 *  LDY Load index Y
 ***************************************************************/
#define LDY 													\
	Y = (UINT8)tmp; 											\
	SET_NZ(Y)

/* 6502 ********************************************************
 *  LSR Logic shift right
 *  0 -> [7][6][5][4][3][2][1][0] -> C
 ***************************************************************/
#define LSR 													\
	P = (P & ~F_C) | (tmp & F_C);								\
	tmp = (UINT8)tmp >> 1;										\
	SET_NZ(tmp)

/* 6502 ********************************************************
 *  NOP No operation
 ***************************************************************/
#define NOP

/* 6502 ********************************************************
 *  ORA Logical inclusive or
 ***************************************************************/
#define ORA 													\
	A = (UINT8)(A | tmp);										\
	SET_NZ(A)

/* 6502 ********************************************************
 *  PHA Push accumulator
 ***************************************************************/
#define PHA 													\
	PUSH(A)

/* 6502 ********************************************************
 *  PHP Push processor status (flags)
 ***************************************************************/
#define PHP 													\
	PUSH(P)

/* 6502 ********************************************************
 *  PLA Pull accumulator
 ***************************************************************/
#define PLA 													\
	RDMEM(SPD);													\
	PULL(A);													\
	SET_NZ(A)


/* 6502 ********************************************************
 *  PLP Pull processor status (flags)
 ***************************************************************/
#define PLP 													\
	RDMEM(SPD);													\
	if ( P & F_I ) {											\
		PULL(P);												\
		if ((m6502.irq_state != M6502_CLEAR_LINE) && !(P & F_I)) {	\
			m6502.after_cli = 1;								\
		}														\
	} else {													\
		PULL(P);												\
	}															\
	P |= (F_T|F_B);

/* 6502 ********************************************************
 * ROL  Rotate left
 *  new C <- [7][6][5][4][3][2][1][0] <- C
 ***************************************************************/
#define ROL 													\
	tmp = (tmp << 1) | (P & F_C);								\
	P = (P & ~F_C) | ((tmp >> 8) & F_C);						\
	tmp = (UINT8)tmp;											\
	SET_NZ(tmp)

/* 6502 ********************************************************
 * ROR  Rotate right
 *  C -> [7][6][5][4][3][2][1][0] -> new C
 ***************************************************************/
#define ROR 													\
	tmp |= (P & F_C) << 8;										\
	P = (P & ~F_C) | (tmp & F_C);								\
	tmp = (UINT8)(tmp >> 1);									\
	SET_NZ(tmp)

/* 6502 ********************************************************
 * RTI  Return from interrupt
 * pull flags, pull PC lo, pull PC hi and increment PC
 *  PCW++;
 ***************************************************************/
#define RTI 													\
	RDOPARG();													\
	RDMEM(SPD);													\
	PULL(P);													\
	PULL(PCL);													\
	PULL(PCH);													\
	P |= F_T | F_B; 											\
	if( (m6502.irq_state != M6502_CLEAR_LINE) && !(P & F_I) )			\
	{															\
		m6502.after_cli = 1;									\
	}															\
	CHANGE_PC

/* 6502 ********************************************************
 *  RTS Return from subroutine
 *  pull PC lo, PC hi and increment PC
 ***************************************************************/
#define RTS 													\
	RDOPARG();													\
	RDMEM(SPD);													\
	PULL(PCL);													\
	PULL(PCH);													\
	RDMEM(PCW); PCW++;											\
	CHANGE_PC

/* 6502 ********************************************************
 *  SBC Subtract with carry
 ***************************************************************/
#define SBC 													\
	if (P & F_D)												\
	{															\
		int c = (P & F_C) ^ F_C;								\
		int sum = A - tmp - c;									\
		int lo = (A & 0x0f) - (tmp & 0x0f) - c; 				\
		int hi = (A & 0xf0) - (tmp & 0xf0); 					\
		if (lo & 0x10)											\
		{														\
			lo -= 6;											\
			hi--;												\
		}														\
		P &= ~(F_V | F_C|F_Z|F_N);								\
		if( (A^tmp) & (A^sum) & F_N )							\
			P |= F_V;											\
		if( hi & 0x0100 )										\
			hi -= 0x60; 										\
		if( (sum & 0xff00) == 0 )								\
			P |= F_C;											\
		if( !((A-tmp-c) & 0xff) )								\
			P |= F_Z;											\
		if( (A-tmp-c) & 0x80 )									\
			P |= F_N;											\
		A = (lo & 0x0f) | (hi & 0xf0);							\
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
		SET_NZ(A);												\
	}

/* 6502 ********************************************************
 *  SEC Set carry flag
 ***************************************************************/
#if defined(SEC)
#undef SEC
#endif
#define SEC 													\
	P |= F_C

/* 6502 ********************************************************
 *  SED Set decimal flag
 ***************************************************************/
#define SED 													\
	P |= F_D

/* 6502 ********************************************************
 *  SEI Set interrupt flag
 ***************************************************************/
#define SEI 													\
	P |= F_I

/* 6502 ********************************************************
 * STA  Store accumulator
 ***************************************************************/
#define STA 													\
	tmp = A

/* 6502 ********************************************************
 * STX  Store index X
 ***************************************************************/
#define STX 													\
	tmp = X

/* 6502 ********************************************************
 * STY  Store index Y
 ***************************************************************/
#define STY 													\
	tmp = Y

/* 6502 ********************************************************
 * TAX  Transfer accumulator to index X
 ***************************************************************/
#define TAX 													\
	X = A;														\
	SET_NZ(X)

/* 6502 ********************************************************
 * TAY  Transfer accumulator to index Y
 ***************************************************************/
#define TAY 													\
	Y = A;														\
	SET_NZ(Y)

/* 6502 ********************************************************
 * TSX  Transfer stack LSB to index X
 ***************************************************************/
#define TSX 													\
	X = S;														\
	SET_NZ(X)

/* 6502 ********************************************************
 * TXA  Transfer index X to accumulator
 ***************************************************************/
#define TXA 													\
	A = X;														\
	SET_NZ(A)

/* 6502 ********************************************************
 * TXS  Transfer index X to stack LSB
 * no flags changed (sic!)
 ***************************************************************/
#define TXS 													\
	S = X

/* 6502 ********************************************************
 * TYA  Transfer index Y to accumulator
 ***************************************************************/
#define TYA 													\
	A = Y;														\
	SET_NZ(A)
