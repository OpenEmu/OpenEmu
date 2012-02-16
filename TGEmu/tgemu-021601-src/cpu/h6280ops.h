
/*****************************************************************************

	h6280ops.h - Addressing modes and opcode macros for the Hu6820 cpu

	Copyright (c) 1999 Bryan McPhail, mish@tendril.co.uk

	This source code is based (with permission!) on the 6502 emulator by
	Juergen Buchmueller.  It is released as part of the Mame emulator project.
	Let me know if you intend to use this code in any other project.

******************************************************************************/

/* 6280 flags */
#define _fC 0x01
#define _fZ 0x02
#define _fI 0x04
#define _fD 0x08
#define _fB 0x10
#define _fT 0x20
#define _fV 0x40
#define _fN 0x80

/* some shortcuts for improved readability */
#define A	h6280.a
#define X	h6280.x
#define Y	h6280.y
#define P	h6280.p
#define S	h6280.sp.b.l

#if LAZY_FLAGS

#define NZ	h6280.NZ
#define SET_NZ(n)				\
	P &= ~_fT;					\
    NZ = ((n & _fN) << 8) | n

#else

#define SET_NZ(n)				\
	P = (P & ~(_fN|_fT|_fZ)) |	\
		(n & _fN) | 			\
		((n == 0) ? _fZ : 0)

#endif

#define EAL h6280.ea.b.l
#define EAH h6280.ea.b.h
#define EAW h6280.ea.w.l
#define EAD h6280.ea.d

#define ZPL h6280.zp.b.l
#define ZPH h6280.zp.b.h
#define ZPW h6280.zp.w.l
#define ZPD h6280.zp.d

#define PCL h6280.pc.b.l
#define PCH h6280.pc.b.h
#define PCW h6280.pc.w.l
#define PCD h6280.pc.d

#define DO_INTERRUPT(vector)									\
{																\
	h6280.extra_cycles += 7;	/* 7 cycles for an int */		\
	PUSH(PCH);													\
	PUSH(PCL);													\
	COMPOSE_P(0,_fB);											\
	PUSH(P);													\
	P = (P & ~_fD) | _fI;	/* knock out D and set I flag */	\
	PCL = RDMEM(vector);										\
	PCH = RDMEM((vector+1));									\
}

#define CHECK_IRQ_LINES 										\
	if( !(P & _fI) )											\
	{															\
		if ( h6280.irq_state[0] != CLEAR_LINE &&				\
			 !(h6280.irq_mask & 0x2) )							\
		{														\
			DO_INTERRUPT(H6280_IRQ1_VEC);						\
			(*h6280.irq_callback)(0);							\
		}														\
		else													\
		if ( h6280.irq_state[1] != CLEAR_LINE &&				\
			 !(h6280.irq_mask & 0x1) )							\
		{														\
			DO_INTERRUPT(H6280_IRQ2_VEC);						\
			(*h6280.irq_callback)(1);							\
        }                                                       \
		else													\
        if ( h6280.irq_state[2] != CLEAR_LINE &&                \
			 !(h6280.irq_mask & 0x4) )							\
		{														\
			h6280.irq_state[2] = CLEAR_LINE;					\
 			DO_INTERRUPT(H6280_TIMER_VEC);						\
       }                                                       	\
    }

#ifdef FAST_MEM

/* Not technically accurate, but I have yet to see a game that maps
   something other than RAM to MMR #1, and fetches opcodes/operands
   out of the I/O space. */

extern unsigned char ram[0x8000];
extern unsigned char *read_ptr[8];
extern unsigned char *write_ptr[8];
extern void io_page_w(int address, int value);
extern int io_page_r(int address);

#define cpu_readop21_fast(addr)         read_ptr[(addr) >> 13][(addr) & 0x1FFF]
#define cpu_readmem21_fast(addr)        (read_ptr[(addr) >> 13] == 0) ? io_page_r((addr) & 0x1FFF) : read_ptr[(addr) >> 13][(addr) & 0x1FFF]
#define cpu_writemem21_fast(addr,value) if(write_ptr[(addr) >> 13] == 0) io_page_w((addr) & 0x1FFF, value); else (write_ptr[(addr) >> 13][(addr) & 0x1FFF] = value)

#define RDMEMZ(addr)        ram[addr & 0x1FFF];
#define WRMEMZ(addr,data)   ram[addr & 0x1FFF] = data;
#define PUSH(Rg)            ram[h6280.sp.d] = Rg; S--
#define PULL(Rg)            S++; Rg = ram[h6280.sp.d]

#define RDZPWORD(addr)                              \
    ((addr&0xff)==0xff) ?                           \
        ram[(addr)&0x1fff]                          \
        +( ram[ ((addr-0xff)&0x1fff) ]    <<8) :    \
        ram[(addr)&0x1fff]                          \
        +( ram[ ((addr+1)&0x1fff)] <<8)

#define RDOP()													\
    cpu_readop21_fast(PCW)

#define RDOPARG()												\
    cpu_readop21_fast(PCW)

#define RDMEM(addr)                                             \
    cpu_readmem21_fast(addr)

#define WRMEM(addr,data)										\
    cpu_writemem21_fast(addr, data);

#define RDMEMW(addr)											\
    cpu_readmem21_fast(addr) \
| ( cpu_readmem21_fast(addr+1) << 8 )

#else

/***************************************************************
 *  RDMEM   read memory
 ***************************************************************/
#define RDMEM(addr) 											\
	cpu_readmem21( (h6280.mmr[(addr)>>13] << 13) | ((addr)&0x1fff))

/***************************************************************
 *  WRMEM   write memory
 ***************************************************************/
#define WRMEM(addr,data)										\
	cpu_writemem21( (h6280.mmr[(addr)>>13] << 13) | ((addr)&0x1fff),data);

/***************************************************************
 *  RDMEMW   read word from memory
 ***************************************************************/
#define RDMEMW(addr)											\
    cpu_readmem21( (h6280.mmr[(addr)  >>13] << 13) | ((addr  )&0x1fff)) \
| ( cpu_readmem21( (h6280.mmr[(addr+1)>>13] << 13) | ((addr+1)&0x1fff)) << 8 )

/***************************************************************
 *  RDMEMZ   read memory - zero page
 ***************************************************************/
#define RDMEMZ(addr) 											\
    cpu_readmem21( (h6280.mmr[1] << 13) | ((addr)&0x1fff));

/***************************************************************
 *  WRMEMZ   write memory - zero page
 ***************************************************************/
#define WRMEMZ(addr,data) 										\
    cpu_writemem21( (h6280.mmr[1] << 13) | ((addr)&0x1fff),data);

/***************************************************************
 *  RDZPWORD    read a word from a zero page address
 ***************************************************************/
#define RDZPWORD(addr)											\
	((addr&0xff)==0xff) ?										\
		cpu_readmem21( (h6280.mmr[1] << 13) | ((addr)&0x1fff))				\
		+(cpu_readmem21( (h6280.mmr[1] << 13) | ((addr-0xff)&0x1fff))<<8) : \
		cpu_readmem21( (h6280.mmr[1] << 13) | ((addr)&0x1fff))				\
		+(cpu_readmem21( (h6280.mmr[1] << 13) | ((addr+1)&0x1fff))<<8)

/***************************************************************
 * push a register onto the stack
 ***************************************************************/
#define PUSH(Rg) cpu_writemem21( (h6280.mmr[1] << 13) | h6280.sp.d,Rg); S--

/***************************************************************
 * pull a register from the stack
 ***************************************************************/
#define PULL(Rg) S++; Rg = cpu_readmem21( (h6280.mmr[1] << 13) | h6280.sp.d)

/***************************************************************
 *  RDOP    read an opcode
 ***************************************************************/
#define RDOP()													\
    cpu_readmem21((h6280.mmr[PCW>>13] << 13) | (PCW&0x1fff))

/***************************************************************
 *  RDOPARG read an opcode argument
 ***************************************************************/
#define RDOPARG()												\
    cpu_readmem21((h6280.mmr[PCW>>13] << 13) | (PCW&0x1fff))

#endif /* FAST_MEM */

/***************************************************************
 *	BRA  branch relative
 ***************************************************************/
#define BRA(cond)												\
	if (cond)													\
	{															\
		h6280_ICount -= 4;										\
		tmp = RDOPARG();										\
		PCW++;													\
		EAW = PCW + (signed char)tmp;							\
		PCD = EAD;												\
	}															\
	else														\
	{															\
		PCW++;													\
		h6280_ICount -= 2;										\
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
	PCW++;														\
	EAD = ZPD

/***************************************************************
 *  EA = zero page address + X
 ***************************************************************/
#define EA_ZPX													\
	ZPL = RDOPARG() + X;										\
	PCW++;														\
	EAD = ZPD

/***************************************************************
 *  EA = zero page address + Y
 ***************************************************************/
#define EA_ZPY													\
	ZPL = RDOPARG() + Y;										\
	PCW++;														\
	EAD = ZPD

/***************************************************************
 *  EA = absolute address
 ***************************************************************/
#define EA_ABS													\
	EAL = RDOPARG();											\
	PCW++;														\
	EAH = RDOPARG();											\
	PCW++

/***************************************************************
 *  EA = absolute address + X
 ***************************************************************/
#define EA_ABX                                                  \
	EA_ABS; 													\
	EAW += X

/***************************************************************
 *	EA = absolute address + Y
 ***************************************************************/
#define EA_ABY													\
	EA_ABS; 													\
	EAW += Y

/***************************************************************
 *	EA = zero page indirect (65c02 pre indexed w/o X)
 ***************************************************************/
#define EA_ZPI													\
	ZPL = RDOPARG();											\
	PCW++;														\
	EAD = RDZPWORD(ZPD)

/***************************************************************
 *  EA = zero page + X indirect (pre indexed)
 ***************************************************************/
#define EA_IDX													\
	ZPL = RDOPARG() + X;										\
	PCW++;														\
	EAD = RDZPWORD(ZPD);

/***************************************************************
 *  EA = zero page indirect + Y (post indexed)
 ***************************************************************/
#define EA_IDY													\
	ZPL = RDOPARG();											\
	PCW++;														\
	EAD = RDZPWORD(ZPD);										\
	EAW += Y

/***************************************************************
 *	EA = indirect (only used by JMP)
 ***************************************************************/
#define EA_IND													\
	EA_ABS; 													\
	tmp = RDMEM(EAD);											\
	EAD++; 														\
	EAH = RDMEM(EAD);											\
	EAL = tmp

/***************************************************************
 *	EA = indirect plus x (only used by JMP)
 ***************************************************************/
#define EA_IAX                                                  \
	EA_ABS;														\
	EAD+=X;														\
	tmp = RDMEM(EAD);											\
	EAD++; 	 													\
	EAH = RDMEM(EAD);											\
	EAL = tmp

/* read a value into tmp */
#define RD_IMM	tmp = RDOPARG(); PCW++
#define RD_IMM2	tmp2 = RDOPARG(); PCW++
#define RD_ACC	tmp = A
#define RD_ZPG	EA_ZPG; tmp = RDMEMZ(EAD)
#define RD_ZPX	EA_ZPX; tmp = RDMEMZ(EAD)
#define RD_ZPY	EA_ZPY; tmp = RDMEMZ(EAD)
#define RD_ABS	EA_ABS; tmp = RDMEM(EAD)
#define RD_ABX	EA_ABX; tmp = RDMEM(EAD)
#define RD_ABY	EA_ABY; tmp = RDMEM(EAD)
#define RD_ZPI	EA_ZPI; tmp = RDMEM(EAD)
#define RD_IDX	EA_IDX; tmp = RDMEM(EAD)
#define RD_IDY	EA_IDY; tmp = RDMEM(EAD)

/* write a value from tmp */
#define WR_ZPG	EA_ZPG; WRMEMZ(EAD, tmp)
#define WR_ZPX	EA_ZPX; WRMEMZ(EAD, tmp)
#define WR_ZPY	EA_ZPY; WRMEMZ(EAD, tmp)
#define WR_ABS	EA_ABS; WRMEM(EAD, tmp)
#define WR_ABX	EA_ABX; WRMEM(EAD, tmp)
#define WR_ABY	EA_ABY; WRMEM(EAD, tmp)
#define WR_ZPI	EA_ZPI; WRMEM(EAD, tmp)
#define WR_IDX	EA_IDX; WRMEM(EAD, tmp)
#define WR_IDY	EA_IDY; WRMEM(EAD, tmp)

/* write back a value from tmp to the last EA */
#define WB_ACC	A = (UINT8)tmp;
#define WB_EA	WRMEM(EAD, tmp)
#define WB_EAZ	WRMEMZ(EAD, tmp)

/***************************************************************
 *
 * Macros to emulate the 6280 opcodes
 *
 ***************************************************************/

/***************************************************************
 * compose the real flag register by
 * including N and Z and set any
 * SET and clear any CLR bits also
 ***************************************************************/
#if LAZY_FLAGS

#define COMPOSE_P(SET,CLR)										\
	P = (P & ~(_fN | _fZ | CLR)) |								\
		(NZ >> 8) | 											\
		((NZ & 0xff) ? 0 : _fZ) |								\
		SET

#else

#define COMPOSE_P(SET,CLR)										\
	P = (P & ~CLR) | SET

#endif

/* 6280 ********************************************************
 *	ADC Add with carry
 ***************************************************************/
#define ADC 													\
	if (P & _fD)												\
	{															\
	int c = (P & _fC);											\
	int lo = (A & 0x0f) + (tmp & 0x0f) + c; 					\
	int hi = (A & 0xf0) + (tmp & 0xf0); 						\
		P &= ~(_fV | _fC);										\
		if (lo > 0x09)											\
		{														\
			hi += 0x10; 										\
			lo += 0x06; 										\
		}														\
		if (~(A^tmp) & (A^hi) & _fN)							\
			P |= _fV;											\
		if (hi > 0x90)											\
			hi += 0x60; 										\
		if (hi & 0xff00)										\
			P |= _fC;											\
		A = (lo & 0x0f) + (hi & 0xf0);							\
	}															\
	else														\
	{															\
	int c = (P & _fC);											\
	int sum = A + tmp + c;										\
		P &= ~(_fV | _fC);										\
		if (~(A^tmp) & (A^sum) & _fN)							\
			P |= _fV;											\
		if (sum & 0xff00)										\
			P |= _fC;											\
		A = (UINT8) sum;										\
	}															\
	SET_NZ(A)

/* 6280 ********************************************************
 *	AND Logical and
 ***************************************************************/
#define AND 													\
	A = (UINT8)(A & tmp);										\
	SET_NZ(A)

/* 6280 ********************************************************
 *	ASL Arithmetic shift left
 ***************************************************************/
#define ASL 													\
	P = (P & ~_fC) | ((tmp >> 7) & _fC);						\
	tmp = (UINT8)(tmp << 1);									\
	SET_NZ(tmp)

/* 6280 ********************************************************
 *  BBR Branch if bit is reset
 ***************************************************************/
#define BBR(bit)                                                \
    BRA(!(tmp & (1<<bit)))

/* 6280 ********************************************************
 *  BBS Branch if bit is set
 ***************************************************************/
#define BBS(bit)                                                \
    BRA(tmp & (1<<bit))

/* 6280 ********************************************************
 *	BCC Branch if carry clear
 ***************************************************************/
#define BCC BRA(!(P & _fC))

/* 6280 ********************************************************
 *	BCS Branch if carry set
 ***************************************************************/
#define BCS BRA(P & _fC)

/* 6280 ********************************************************
 *	BEQ Branch if equal
 ***************************************************************/
#if LAZY_FLAGS
#define BEQ BRA(!(NZ & 0xff))
#else
#define BEQ BRA(P & _fZ)
#endif

/* 6280 ********************************************************
 *	BIT Bit test
 ***************************************************************/
#define BIT														\
	P = (P & ~(_fN|_fV|_fT|_fZ))								\
		| ((tmp&0x80) ? _fN:0)									\
		| ((tmp&0x40) ? _fV:0)									\
		| ((tmp&A)  ? 0:_fZ)

/* 6280 ********************************************************
 *	BMI Branch if minus
 ***************************************************************/
#if LAZY_FLAGS
#define BMI BRA(NZ & 0x8000)
#else
#define BMI BRA(P & _fN)
#endif

/* 6280 ********************************************************
 *	BNE Branch if not equal
 ***************************************************************/
#if LAZY_FLAGS
#define BNE BRA(NZ & 0xff)
#else
#define BNE BRA(!(P & _fZ))
#endif

/* 6280 ********************************************************
 *	BPL Branch if plus
 ***************************************************************/
#if LAZY_FLAGS
#define BPL BRA(!(NZ & 0x8000))
#else
#define BPL BRA(!(P & _fN))
#endif

/* 6280 ********************************************************
 *	BRK Break
 *	increment PC, push PC hi, PC lo, flags (with B bit set),
 *	set I flag, reset D flag and jump via IRQ vector
 ***************************************************************/
#define BRK 													\
	PCW++;														\
	PUSH(PCH);													\
	PUSH(PCL);													\
	PUSH(P | _fB);												\
	P = (P & ~_fD) | _fI;										\
	PCL = RDMEM(H6280_IRQ2_VEC); 								\
	PCH = RDMEM(H6280_IRQ2_VEC+1)

/* 6280 ********************************************************
 *	BSR Branch to subroutine
 ***************************************************************/
#define BSR 													\
	PUSH(PCH);													\
	PUSH(PCL);													\
	h6280_ICount -= 4; /* 4 cycles here, 4 in BRA */			\
	BRA(1)

/* 6280 ********************************************************
 *	BVC Branch if overflow clear
 ***************************************************************/
#define BVC BRA(!(P & _fV))

/* 6280 ********************************************************
 *	BVS Branch if overflow set
 ***************************************************************/
#define BVS BRA(P & _fV)

/* 6280 ********************************************************
 *  CLA Clear accumulator
 ***************************************************************/
#define CLA                                                     \
    A = 0

/* 6280 ********************************************************
 *	CLC Clear carry flag
 ***************************************************************/
#define CLC 													\
	P &= ~_fC

/* 6280 ********************************************************
 *	CLD Clear decimal flag
 ***************************************************************/
#define CLD 													\
	P &= ~_fD

/* 6280 ********************************************************
 *	CLI Clear interrupt flag
 ***************************************************************/
#define CLI 													\
	if( P & _fI )												\
	{															\
		P &= ~_fI;												\
		CHECK_IRQ_LINES;										\
	}


/* 6280 ********************************************************
 *	CLV Clear overflow flag
 ***************************************************************/
#define CLV 													\
	P &= ~_fV

/* 6280 ********************************************************
 *  CLX Clear index X
 ***************************************************************/
#define CLX                                                     \
    X = 0

/* 6280 ********************************************************
 *  CLY Clear index Y
 ***************************************************************/
#define CLY                                                     \
    Y = 0

/* 6280 ********************************************************
 *	CMP Compare accumulator
 ***************************************************************/
#define CMP 													\
	P &= ~_fC;													\
	if (A >= tmp)												\
		P |= _fC;												\
	SET_NZ((UINT8)(A - tmp))

/* 6280 ********************************************************
 *	CPX Compare index X
 ***************************************************************/
#define CPX 													\
	P &= ~_fC;													\
	if (X >= tmp)												\
		P |= _fC;												\
	SET_NZ((UINT8)(X - tmp))

/* 6280 ********************************************************
 *	CPY Compare index Y
 ***************************************************************/
#define CPY 													\
	P &= ~_fC;													\
	if (Y >= tmp)												\
		P |= _fC;												\
	SET_NZ((UINT8)(Y - tmp))

/* 6280 ********************************************************
 *  DEA Decrement accumulator
 ***************************************************************/
#define DEA                                                     \
	A = (UINT8)--A; 											\
    SET_NZ(A)

/* 6280 ********************************************************
 *	DEC Decrement memory
 ***************************************************************/
#define DEC 													\
	tmp = (UINT8)--tmp; 										\
	SET_NZ(tmp)

/* 6280 ********************************************************
 *	DEX Decrement index X
 ***************************************************************/
#define DEX 													\
	X = (UINT8)--X; 											\
	SET_NZ(X)

/* 6280 ********************************************************
 *	DEY Decrement index Y
 ***************************************************************/
#define DEY 													\
	Y = (UINT8)--Y; 											\
	SET_NZ(Y)

/* 6280 ********************************************************
 *	EOR Logical exclusive or
 ***************************************************************/
#define EOR 													\
	A = (UINT8)(A ^ tmp);										\
	SET_NZ(A)

/* 6280 ********************************************************
 *	ILL Illegal opcode
 ***************************************************************/
#define ILL 													\
    h6280_ICount -= 2; /* (assumed) */                          

/* 6280 ********************************************************
 *  INA Increment accumulator
 ***************************************************************/
#define INA                                                     \
	A = (UINT8)++A; 											\
    SET_NZ(A)

/* 6280 ********************************************************
 *	INC Increment memory
 ***************************************************************/
#define INC 													\
	tmp = (UINT8)++tmp; 										\
	SET_NZ(tmp)

/* 6280 ********************************************************
 *	INX Increment index X
 ***************************************************************/
#define INX 													\
	X = (UINT8)++X; 											\
	SET_NZ(X)

/* 6280 ********************************************************
 *	INY Increment index Y
 ***************************************************************/
#define INY 													\
	Y = (UINT8)++Y; 											\
	SET_NZ(Y)

/* 6280 ********************************************************
 *	JMP Jump to address
 *	set PC to the effective address
 ***************************************************************/
#define JMP 													\
	PCD = EAD

/* 6280 ********************************************************
 *	JSR Jump to subroutine
 *	decrement PC (sic!) push PC hi, push PC lo and set
 *	PC to the effective address
 ***************************************************************/
#define JSR 													\
	PCW--;														\
	PUSH(PCH);													\
	PUSH(PCL);													\
	PCD = EAD

/* 6280 ********************************************************
 *	LDA Load accumulator
 ***************************************************************/
#define LDA 													\
	A = (UINT8)tmp; 											\
	SET_NZ(A)

/* 6280 ********************************************************
 *	LDX Load index X
 ***************************************************************/
#define LDX 													\
	X = (UINT8)tmp; 											\
	SET_NZ(X)

/* 6280 ********************************************************
 *	LDY Load index Y
 ***************************************************************/
#define LDY 													\
	Y = (UINT8)tmp; 											\
	SET_NZ(Y)

/* 6280 ********************************************************
 *	LSR Logic shift right
 *	0 -> [7][6][5][4][3][2][1][0] -> C
 ***************************************************************/
#define LSR 													\
	P = (P & ~_fC) | (tmp & _fC);								\
	tmp = (UINT8)tmp >> 1;										\
	SET_NZ(tmp)

/* 6280 ********************************************************
 *	NOP No operation
 ***************************************************************/
#define NOP

/* 6280 ********************************************************
 *	ORA Logical inclusive or
 ***************************************************************/
#define ORA 													\
	A = (UINT8)(A | tmp);										\
	SET_NZ(A)

/* 6280 ********************************************************
 *	PHA Push accumulator
 ***************************************************************/
#define PHA 													\
	PUSH(A)

/* 6280 ********************************************************
 *	PHP Push processor status (flags)
 ***************************************************************/
#define PHP 													\
	COMPOSE_P(0,0); 											\
	PUSH(P)

/* 6280 ********************************************************
 *  PHX Push index X
 ***************************************************************/
#define PHX                                                     \
    PUSH(X)

/* 6280 ********************************************************
 *  PHY Push index Y
 ***************************************************************/
#define PHY                                                     \
    PUSH(Y)

/* 6280 ********************************************************
 *	PLA Pull accumulator
 ***************************************************************/
#define PLA 													\
	PULL(A);													\
	SET_NZ(A)

/* 6280 ********************************************************
 *	PLP Pull processor status (flags)
 ***************************************************************/
#if LAZY_FLAGS

#define PLP 													\
	PULL(P);													\
	NZ = ((P & _fN) << 8) | 									\
		 ((P & _fZ) ^ _fZ); 									\
	CHECK_IRQ_LINES

#else

#define PLP 													\
	PULL(P); 													\
	CHECK_IRQ_LINES
#endif

/* 6280 ********************************************************
 *  PLX Pull index X
 ***************************************************************/
#define PLX                                                     \
    PULL(X)

/* 6280 ********************************************************
 *  PLY Pull index Y
 ***************************************************************/
#define PLY                                                     \
    PULL(Y)

/* 6280 ********************************************************
 *  RMB Reset memory bit
 ***************************************************************/
#define RMB(bit)                                                \
    tmp &= ~(1<<bit)

/* 6280 ********************************************************
 *	ROL Rotate left
 *	new C <- [7][6][5][4][3][2][1][0] <- C
 ***************************************************************/
#define ROL 													\
	tmp = (tmp << 1) | (P & _fC);								\
	P = (P & ~_fC) | ((tmp >> 8) & _fC);						\
	tmp = (UINT8)tmp;											\
	SET_NZ(tmp)

/* 6280 ********************************************************
 *	ROR Rotate right
 *	C -> [7][6][5][4][3][2][1][0] -> new C
 ***************************************************************/
#define ROR 													\
	tmp |= (P & _fC) << 8;										\
	P = (P & ~_fC) | (tmp & _fC);								\
	tmp = (UINT8)(tmp >> 1);									\
	SET_NZ(tmp)

/* 6280 ********************************************************
 *	RTI Return from interrupt
 *	pull flags, pull PC lo, pull PC hi and increment PC
 ***************************************************************/
#if LAZY_FLAGS

#define RTI 													\
	PULL(P);													\
	NZ = ((P & _fN) << 8) | 									\
		 ((P & _fZ) ^ _fZ); 									\
	PULL(PCL);													\
	PULL(PCH);													\
	CHECK_IRQ_LINES
#else

#define RTI 													\
	PULL(P);													\
	PULL(PCL);													\
	PULL(PCH);													\
	CHECK_IRQ_LINES
#endif

/* 6280 ********************************************************
 *	RTS Return from subroutine
 *	pull PC lo, PC hi and increment PC
 ***************************************************************/
#define RTS 													\
	PULL(PCL);													\
	PULL(PCH);													\
	PCW++;														\

/* 6280 ********************************************************
 *  SAX Swap accumulator and index X
 ***************************************************************/
#define SAX                                                     \
    tmp = X;                                                    \
    X = A;                                                      \
    A = tmp

/* 6280 ********************************************************
 *  SAY Swap accumulator and index Y
 ***************************************************************/
#define SAY                                                     \
    tmp = Y;                                                    \
    Y = A;                                                      \
    A = tmp

/* 6280 ********************************************************
 *	SBC Subtract with carry
 ***************************************************************/
#define SBC 													\
	if (P & _fD)												\
	{															\
	int c = (P & _fC) ^ _fC;									\
	int sum = A - tmp - c;										\
	int lo = (A & 0x0f) - (tmp & 0x0f) - c; 					\
	int hi = (A & 0xf0) - (tmp & 0xf0); 						\
		P &= ~(_fV | _fC);										\
		if ((A^tmp) & (A^sum) & _fN)							\
			P |= _fV;											\
		if (lo & 0xf0)											\
			lo -= 6;											\
		if (lo & 0x80)											\
			hi -= 0x10; 										\
		if (hi & 0x0f00)										\
			hi -= 0x60; 										\
		if ((sum & 0xff00) == 0)								\
			P |= _fC;											\
		A = (lo & 0x0f) + (hi & 0xf0);							\
	}															\
	else														\
	{															\
	int c = (P & _fC) ^ _fC;									\
	int sum = A - tmp - c;										\
		P &= ~(_fV | _fC);										\
		if ((A^tmp) & (A^sum) & _fN)							\
			P |= _fV;											\
		if ((sum & 0xff00) == 0)								\
			P |= _fC;											\
		A = (UINT8) sum;										\
	}															\
	SET_NZ(A)

/* 6280 ********************************************************
 *	SEC Set carry flag
 ***************************************************************/
#define SEC 													\
	P |= _fC

/* 6280 ********************************************************
 *	SED Set decimal flag
 ***************************************************************/
#define SED 													\
	P |= _fD

/* 6280 ********************************************************
 *	SEI Set interrupt flag
 ***************************************************************/
#define SEI 													\
	P |= _fI

/* 6280 ********************************************************
 *  SET Set t flag
 *  T flag = Memory operation flag.
 *
 *  Instruction immediately after SET will use the byte in the zero
 *  page area indexed by X, if it is ADC, AND, ORA, EOR with an
 *  immediate operand.
 *
 *  Else, the function of the memory operation flag is ignored
 *  and the instruction executes like normal.
 *
 *  Other possible uses:
 *  Legendary Axe I/II (sed set sbc)
 *  Bravoman (set sbc)
 *  Magical Chase (set cpx)
 ***************************************************************/

#define SET                                                     \
{                                                               \
    int op, tmp;                                                \
    op = RDOP(); /* Get next instruction */                     \
    if((op & 0x9F) == 0x09)                                     \
    {                                                           \
        UINT8 acc;                                              \
        PCW++;                                                  \
        tmp = RDOPARG(); /* Immediate operand */                \
        PCW++;                                                  \
        acc = RDMEMZ(X); /* Used instead of accmulator */       \
                                                                \
        switch(op)                                              \
        {                                                       \
            case 0x09: acc = (UINT8)(acc | tmp); break; /* ORA */ \
            case 0x29: acc = (UINT8)(acc & tmp); break; /* AND */ \
            case 0x49: acc = (UINT8)(acc ^ tmp); break; /* EOR */ \
            case 0x69: /* ADC */                                \
                if(P & _fD)                                     \
                {                                               \
                    int c = (P & _fC);                          \
                    int lo = (acc & 0x0F) + (tmp & 0x0F) + c;   \
                    int hi = (acc & 0xF0) + (tmp & 0xF0);       \
                    P &= ~(_fV | _fC);                          \
                    if(lo > 0x09) { hi += 0x10; lo += 0x06; };  \
                    if(~(acc^tmp) & (acc^hi) & _fN) P |= _fV;   \
                    if(hi > 0x90) hi += 0x60;                   \
                    if(hi & 0xFF00) P |= _fC;                   \
                    acc = (lo & 0x0F) + (hi & 0xF0);            \
                }                                               \
                else                                            \
                {                                               \
                    int c = (P & _fC);                          \
                    int sum = acc + tmp + c;                    \
                    P &= ~(_fV | _fC);                          \
                    if (~(acc^tmp) & (acc^sum) & _fN)           \
                        P |= _fV;                               \
                    if (sum & 0xff00)                           \
                        P |= _fC;                               \
                    acc = (UINT8) sum;                          \
                }                                               \
                break;                                          \
        }                                                       \
                                                                \
        SET_NZ(acc);        /* Update flags */                  \
        WRMEMZ(X, acc);     /* Write result back */             \
        h6280_ICount -= 6;  /* Unsure of actual cycles used */  \
    }                                                           \
    P &= ~_fT;                                                  \
}                                                               




/* 6280 ********************************************************
 *  CSL Clock select low
 ***************************************************************/
#define CSL             \
    h6280_speed = 0

/* 6280 ********************************************************
 *  CSL Clock select high
 ***************************************************************/
#define CSH             \
    h6280_speed = 1    

/* 6280 ********************************************************
 *  SMB Set memory bit
 ***************************************************************/
#define SMB(bit)                                                \
    tmp |= (1<<bit)

#ifdef FAST_MEM

extern void vdc_w(int address, int data);

#define ST0     vdc_w(0, tmp)
#define ST1     vdc_w(2, tmp)
#define ST2     vdc_w(3, tmp)

#else

/* 6280 ********************************************************
 *  ST0 Store at hardware address 0
 ***************************************************************/
#define ST0                                                     \
    cpu_writeport16(0x0000,tmp)

/* 6280 ********************************************************
 *  ST1 Store at hardware address 2
 ***************************************************************/
#define ST1                                                     \
    cpu_writeport16(0x0002,tmp)

/* 6280 ********************************************************
 *  ST2 Store at hardware address 3
 ***************************************************************/
#define ST2                                                     \
    cpu_writeport16(0x0003,tmp)

#endif

/* 6280 ********************************************************
 *	STA Store accumulator
 ***************************************************************/
#define STA 													\
	tmp = A

/* 6280 ********************************************************
 *	STX Store index X
 ***************************************************************/
#define STX 													\
	tmp = X

/* 6280 ********************************************************
 *	STY Store index Y
 ***************************************************************/
#define STY 													\
	tmp = Y

/* 6280 ********************************************************
 * STZ  Store zero
 ***************************************************************/
#define STZ                                                     \
    tmp = 0

/* H6280 *******************************************************
 *  SXY Swap index X and index Y
 ***************************************************************/
#define SXY                                                    \
    tmp = X;                                                   \
    X = Y;                                                     \
    Y = tmp

/* H6280 *******************************************************
 *  TAI
 ***************************************************************/
#define TAI 													\
	from=RDMEMW(PCW);											\
	to  =RDMEMW(PCW+2);											\
	length=RDMEMW(PCW+4);										\
	PCW+=6; 													\
	alternate=0; 												\
	while ((length--) != 0) { 									\
		WRMEM(to,RDMEM(from+alternate)); 						\
		to++; 													\
		alternate ^= 1; 										\
	}		 													\
	h6280_ICount-=(6 * length) + 17;

/* H6280 *******************************************************
 *  TAM Transfer accumulator to memory mapper register(s)
 ***************************************************************/

#ifdef FAST_MEM

extern void bank_set(int bank, int value);
#define TAM                                                     \
    {                                                           \
        int shift;                                              \
        for(shift = 0; shift < 8; shift += 1)                   \
        {                                                       \
            if(tmp & (1 << shift))                              \
            {                                                   \
                h6280.mmr[shift] = A;                           \
                bank_set(shift, A);                             \
            }                                                   \
            if(tmp & (1 << shift)) break;                       \
        }                                                       \
    }                                                           

#else

#define TAM                                                     \
    if (tmp&0x01) h6280.mmr[0] = A;                             \
    if (tmp&0x02) h6280.mmr[1] = A;                             \
    if (tmp&0x04) h6280.mmr[2] = A;                             \
    if (tmp&0x08) h6280.mmr[3] = A;                             \
    if (tmp&0x10) h6280.mmr[4] = A;                             \
    if (tmp&0x20) h6280.mmr[5] = A;                             \
    if (tmp&0x40) h6280.mmr[6] = A;                             \
    if (tmp&0x80) h6280.mmr[7] = A

#endif /* FAST_MEM */

/* 6280 ********************************************************
 *	TAX Transfer accumulator to index X
 ***************************************************************/
#define TAX 													\
	X = A;														\
	SET_NZ(X)

/* 6280 ********************************************************
 *	TAY Transfer accumulator to index Y
 ***************************************************************/
#define TAY 													\
	Y = A;														\
	SET_NZ(Y)

/* 6280 ********************************************************
 *  TDD
 ***************************************************************/
#define TDD 													\
	from=RDMEMW(PCW);											\
	to  =RDMEMW(PCW+2);											\
	length=RDMEMW(PCW+4);										\
	PCW+=6; 													\
	while ((length--) != 0) { 									\
		WRMEM(to,RDMEM(from)); 									\
		to--; 													\
		from--;													\
	}		 													\
	h6280_ICount-=(6 * length) + 17;

/* 6280 ********************************************************
 *  TIA
 ***************************************************************/
#define TIA 													\
	from=RDMEMW(PCW);											\
	to  =RDMEMW(PCW+2);											\
	length=RDMEMW(PCW+4);										\
	PCW+=6; 													\
	alternate=0; 												\
	while ((length--) != 0) { 									\
		WRMEM(to+alternate,RDMEM(from));						\
		from++; 												\
		alternate ^= 1; 										\
	}		 													\
	h6280_ICount-=(6 * length) + 17;

/* 6280 ********************************************************
 *  TII
 ***************************************************************/
#define TII 													\
	from=RDMEMW(PCW);											\
	to  =RDMEMW(PCW+2);											\
	length=RDMEMW(PCW+4);										\
	PCW+=6; 													\
	while ((length--) != 0) { 									\
		WRMEM(to,RDMEM(from)); 									\
		to++; 													\
		from++;													\
	}		 													\
	h6280_ICount-=(6 * length) + 17;

/* 6280 ********************************************************
 *  TIN Transfer block, source increments every loop
 ***************************************************************/
#define TIN 													\
	from=RDMEMW(PCW);											\
	to  =RDMEMW(PCW+2);											\
	length=RDMEMW(PCW+4);										\
	PCW+=6; 													\
	while ((length--) != 0) { 									\
		WRMEM(to,RDMEM(from)); 									\
		from++;													\
	}		 													\
	h6280_ICount-=(6 * length) + 17;

/* 6280 ********************************************************
 *  TMA Transfer memory mapper register(s) to accumulator
 *  the highest bit set in tmp is the one that counts
 ***************************************************************/
#define TMA                                                     \
    if (tmp&0x01) A = h6280.mmr[0];                             \
    else                                                        \
    if (tmp&0x02) A = h6280.mmr[1];                             \
    else                                                        \
    if (tmp&0x04) A = h6280.mmr[2];                             \
    else                                                        \
    if (tmp&0x08) A = h6280.mmr[3];                             \
    else                                                        \
    if (tmp&0x10) A = h6280.mmr[4];                             \
    else                                                        \
    if (tmp&0x20) A = h6280.mmr[5];                             \
    else                                                        \
    if (tmp&0x40) A = h6280.mmr[6];                             \
    else                                                        \
    if (tmp&0x80) A = h6280.mmr[7]

/* 6280 ********************************************************
 * TRB  Test and reset bits
 ***************************************************************/
#define TRB                                                   	\
	P = (P & ~(_fN|_fV|_fT|_fZ))								\
		| ((tmp&0x80) ? _fN:0)									\
		| ((tmp&0x40) ? _fV:0)									\
		| ((tmp&A)  ? 0:_fZ);									\
    tmp &= ~A

/* 6280 ********************************************************
 * TSB  Test and set bits
 ***************************************************************/
#define TSB                                                     \
	P = (P & ~(_fN|_fV|_fT|_fZ))								\
		| ((tmp&0x80) ? _fN:0)									\
		| ((tmp&0x40) ? _fV:0)									\
		| ((tmp&A)  ? 0:_fZ);									\
    tmp |= A

/* 6280 ********************************************************
 *	TSX Transfer stack LSB to index X
 ***************************************************************/
#define TSX 													\
	X = S;														\
	SET_NZ(X)

/* 6280 ********************************************************
 *	TST
 ***************************************************************/
#define TST														\
	P = (P & ~(_fN|_fV|_fT|_fZ))								\
		| ((tmp2&0x80) ? _fN:0)									\
		| ((tmp2&0x40) ? _fV:0)									\
		| ((tmp2&tmp)  ? 0:_fZ)

/* 6280 ********************************************************
 *	TXA Transfer index X to accumulator
 ***************************************************************/
#define TXA 													\
	A = X;														\
	SET_NZ(A)

/* 6280 ********************************************************
 *	TXS Transfer index X to stack LSB
 *	no flags changed (sic!)
 ***************************************************************/
#define TXS 													\
	S = X

/* 6280 ********************************************************
 *	TYA Transfer index Y to accumulator
 ***************************************************************/
#define TYA 													\
	A = Y;														\
	SET_NZ(A)
