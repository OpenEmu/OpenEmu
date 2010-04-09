/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "cpu.h"
#include "memory.h"
#include "savestate.h"

CPU::CPU() :
memory(Interrupter(SP, PC_, halted)),
cycleCounter_(0),
PC_(0x100),
SP(0xFFFE),
HF1(0xF),
HF2(0xF),
ZF(0),
CF(0x100),
A_(0x01),
B(0x00),
C(0x13),
D(0x00),
E(0xD8),
H(0x01),
L(0x4D),
skip(false),
halted(false)
{}

void CPU::runFor(const unsigned long cycles) {
	process(cycles/* << memory.isDoubleSpeed()*/);
	
	if (cycleCounter_ & 0x80000000)
		cycleCounter_ = memory.resetCounters(cycleCounter_);
}

bool CPU::load(const char* romfile, const bool forceDmg) {
	bool tmp = memory.loadROM(romfile, forceDmg);
	
	return tmp;
}

/*void CPU::halt() {
	while (halted) {
		const uint_fast32_t cycles = memory.next_eventtime - memory.CycleCounter;
		memory.CycleCounter += cycles + ((4 - (cycles & 3)) & 3);
		memory.event();
	}
}*/

//Push address of next instruction onto stack and then jump to interrupt address (0x40-0x60):
/*unsigned CPU::interrupt(const unsigned address, unsigned cycleCounter) {
	if (halted && memory.isCgb())
		cycleCounter += 4;
	
	halted = false;
	cycleCounter += 8;
	memory.write(--SP, PC_ >> 8, cycleCounter);
	cycleCounter += 4;
	memory.write(--SP, PC_ & 0xFF, cycleCounter);
	PC_ = address;
	cycleCounter += 8;
	
	return cycleCounter;
}*/

// (HF2 & 0x200) == true means HF is set.
// (HF2 & 0x400) marks the subtract flag.
// (HF2 & 0x800) is set for inc/dec.
// (HF2 & 0x100) is set if there's a carry to add.
static void calcHF(const unsigned HF1, unsigned& HF2) {
	unsigned arg1 = HF1 & 0xF;
	unsigned arg2 = (HF2 & 0xF) + (HF2 >> 8 & 1);
	
	if (HF2 & 0x800) {
		arg1 = arg2;
		arg2 = 1;
	}

	if (HF2 & 0x400)
		arg1 -= arg2;
	else
		arg1 = (arg1 + arg2) << 5;
	
	HF2 |= arg1 & 0x200;
}

#define F() (((HF2 & 0x600) | (CF & 0x100)) >> 4 | ((ZF & 0xFF) ? 0 : 0x80))

#define FROM_F(f_in) do { \
	unsigned from_f_var = f_in; \
\
	ZF = ~from_f_var & 0x80; \
	HF2 = from_f_var << 4 & 0x600; \
	CF = from_f_var << 4 & 0x100; \
} while (0)

void CPU::setStatePtrs(SaveState &state) {
	memory.setStatePtrs(state);
}

void CPU::saveState(SaveState &state) {
	cycleCounter_ = memory.saveState(state, cycleCounter_);
	
	calcHF(HF1, HF2);
	
	state.cpu.cycleCounter = cycleCounter_;
	state.cpu.PC = PC_;
	state.cpu.SP = SP;
	state.cpu.A = A_;
	state.cpu.B = B;
	state.cpu.C = C;
	state.cpu.D = D;
	state.cpu.E = E;
	state.cpu.F = F();
	state.cpu.H = H;
	state.cpu.L = L;
	state.cpu.skip = skip;
	state.cpu.halted = halted;
}

void CPU::loadState(const SaveState &state) {
	memory.loadState(state, cycleCounter_);
	
	cycleCounter_ = state.cpu.cycleCounter;
	PC_ = state.cpu.PC;
	SP = state.cpu.SP;
	A_ = state.cpu.A;
	B = state.cpu.B;
	C = state.cpu.C;
	D = state.cpu.D;
	E = state.cpu.E;
	FROM_F(state.cpu.F);
	H = state.cpu.H;
	L = state.cpu.L;
	skip = state.cpu.skip;
	halted = state.cpu.halted;
}

#define BC() ( B << 8 | C )
#define DE() ( D << 8 | E )
#define HL() ( H << 8 | L )

#define READ(dest, addr) do { (dest) = memory.read(addr, cycleCounter); cycleCounter += 4; } while (0)
// #define PC_READ(dest, addr) do { (dest) = memory.pc_read(addr, cycleCounter); cycleCounter += 4; } while (0)
#define PC_READ(dest) do { (dest) = memory.read(PC, cycleCounter); PC = (PC + 1) & 0xFFFF; cycleCounter += 4; } while (0)
#define FF_READ(dest, addr) do { (dest) = memory.ff_read(addr, cycleCounter); cycleCounter += 4; } while (0)

#define WRITE(addr, data) do { memory.write(addr, data, cycleCounter); cycleCounter += 4; } while (0)
#define FF_WRITE(addr, data) do { memory.ff_write(addr, data, cycleCounter); cycleCounter += 4; } while (0)

#define PC_MOD(data) do { PC = data; cycleCounter += 4; } while (0)

#define PUSH(r1, r2) do { \
	SP = (SP - 1) & 0xFFFF; \
	WRITE(SP, (r1)); \
	SP = (SP - 1) & 0xFFFF; \
	WRITE(SP, (r2)); \
} while (0)

//CB OPCODES (Shifts, rotates and bits):
//swap r (8 cycles):
//Swap upper and lower nibbles of 8-bit register, reset flags, check zero flag:
#define swap_r(r) do { \
	CF = HF2 = 0; \
	ZF = (r); \
	(r) = (ZF << 4 | ZF >> 4) & 0xFF; \
} while (0)

//rlc r (8 cycles):
//Rotate 8-bit register left, store old bit7 in CF. Reset SF and HCF, Check ZF:
#define rlc_r(r) do { \
	CF = (r) << 1; \
	ZF = CF | CF >> 8; \
	(r) = ZF & 0xFF; \
	HF2 = 0; \
} while (0)

//rl r (8 cycles):
//Rotate 8-bit register left through CF, store old bit7 in CF, old CF value becomes bit0. Reset SF and HCF, Check ZF:
#define rl_r(r) do { \
	const unsigned rl_r_var_oldcf = CF >> 8 & 1; \
	CF = (r) << 1; \
	ZF = CF | rl_r_var_oldcf; \
	(r) = ZF & 0xFF; \
	HF2 = 0; \
} while (0)

//rrc r (8 cycles):
//Rotate 8-bit register right, store old bit0 in CF. Reset SF and HCF, Check ZF:
#define rrc_r(r) do { \
	ZF = (r); \
	CF = ZF << 8; \
	(r) = (ZF | CF) >> 1 & 0xFF; \
	HF2 = 0; \
} while (0)

//rr r (8 cycles):
//Rotate 8-bit register right through CF, store old bit0 in CF, old CF value becomes bit7. Reset SF and HCF, Check ZF:
#define rr_r(r) do { \
	const unsigned rr_r_var_oldcf = CF & 0x100; \
	CF = (r) << 8; \
	(r) = ZF = ((r) | rr_r_var_oldcf) >> 1; \
	HF2 = 0; \
} while (0)

//sla r (8 cycles):
//Shift 8-bit register left, store old bit7 in CF. Reset SF and HCF, Check ZF:
#define sla_r(r) do { \
	ZF = CF = (r) << 1; \
	(r) = ZF & 0xFF; \
	HF2 = 0; \
} while (0)

//sra r (8 cycles):
//Shift 8-bit register right, store old bit0 in CF. bit7=old bit7. Reset SF and HCF, Check ZF:
#define sra_r(r) do { \
	CF = (r) << 8; \
	ZF = (r) >> 1; \
	(r) = ZF | ((r) & 0x80); \
	HF2 = 0; \
} while (0)

//srl r (8 cycles):
//Shift 8-bit register right, store old bit0 in CF. Reset SF and HCF, Check ZF:
#define srl_r(r) do { \
	ZF = (r); \
	CF = (r) << 8; \
	ZF >>= 1; \
	(r) = ZF; \
	HF2 = 0; \
} while (0)

//bit n,r (8 cycles):
//bit n,(hl) (12 cycles):
//Test bitn in 8-bit value, check ZF, unset SF, set HCF:
#define bitn_u8(bitmask, u8) do { \
	ZF = (u8) & (bitmask); \
	HF2 = 0x200; \
} while (0)

#define bit0_u8(u8) bitn_u8(1, (u8))
#define bit1_u8(u8) bitn_u8(2, (u8))
#define bit2_u8(u8) bitn_u8(4, (u8))
#define bit3_u8(u8) bitn_u8(8, (u8))
#define bit4_u8(u8) bitn_u8(0x10, (u8))
#define bit5_u8(u8) bitn_u8(0x20, (u8))
#define bit6_u8(u8) bitn_u8(0x40, (u8))
#define bit7_u8(u8) bitn_u8(0x80, (u8))

//set n,r (8 cycles):
//Set bitn of 8-bit register:
#define set0_r(r) ( (r) |= 0x1 )
#define set1_r(r) ( (r) |= 0x2 )
#define set2_r(r) ( (r) |= 0x4 )
#define set3_r(r) ( (r) |= 0x8 )
#define set4_r(r) ( (r) |= 0x10 )
#define set5_r(r) ( (r) |= 0x20 )
#define set6_r(r) ( (r) |= 0x40 )
#define set7_r(r) ( (r) |= 0x80 )

//set n,(hl) (16 cycles):
//Set bitn of value at address stored in HL:
#define setn_mem_hl(n) do { \
	const unsigned setn_mem_hl_var_addr = HL(); \
	unsigned setn_mem_hl_var_tmp; \
\
	READ(setn_mem_hl_var_tmp, setn_mem_hl_var_addr); \
	setn_mem_hl_var_tmp |= 1 << (n); \
\
	WRITE(setn_mem_hl_var_addr, setn_mem_hl_var_tmp); \
} while (0)

//res n,r (8 cycles):
//Unset bitn of 8-bit register:
#define res0_r(r) ( (r) &= 0xFE )
#define res1_r(r) ( (r) &= 0xFD )
#define res2_r(r) ( (r) &= 0xFB )
#define res3_r(r) ( (r) &= 0xF7 )
#define res4_r(r) ( (r) &= 0xEF )
#define res5_r(r) ( (r) &= 0xDF )
#define res6_r(r) ( (r) &= 0xBF )
#define res7_r(r) ( (r) &= 0x7F )

//res n,(hl) (16 cycles):
//Unset bitn of value at address stored in HL:
#define resn_mem_hl(n) do { \
	const unsigned resn_mem_hl_var_addr = HL(); \
	unsigned resn_mem_hl_var_tmp; \
\
	READ(resn_mem_hl_var_tmp, resn_mem_hl_var_addr); \
	resn_mem_hl_var_tmp &= ~(1 << (n)); \
\
	WRITE(resn_mem_hl_var_addr, resn_mem_hl_var_tmp); \
} while (0)


//16-BIT LOADS:
//ld rr,nn (12 cycles)
//set rr to 16-bit value of next 2 bytes in memory
#define ld_rr_nn(r1, r2) do { \
	PC_READ(r2); \
	PC_READ(r1); \
} while (0)

//push rr (16 cycles):
//Push value of register pair onto stack:
#define push_rr(r1, r2) do { \
	PUSH(r1, r2); \
	cycleCounter += 4; \
} while (0)

//pop rr (12 cycles):
//Pop two bytes off stack into register pair:
#define pop_rr(r1, r2) do { \
	READ(r2, SP); \
	SP = (SP + 1) & 0xFFFF; \
	READ(r1, SP); \
	SP = (SP + 1) & 0xFFFF; \
} while (0)

//8-BIT ALU:
//add a,r (4 cycles):
//add a,(addr) (8 cycles):
//Add 8-bit value to A, check flags:
#define add_a_u8(u8) do { \
	HF1 = A; \
	HF2 = u8; \
	ZF = CF = A + HF2; \
	A = ZF & 0xFF; \
} while (0)

//adc a,r (4 cycles):
//adc a,(addr) (8 cycles):
//Add 8-bit value+CF to A, check flags:
#define adc_a_u8(u8) do { \
	HF1 = A; \
	HF2 = (CF & 0x100) | (u8); \
	ZF = CF = (CF >> 8 & 1) + (u8) + A; \
	A = ZF & 0xFF; \
} while (0)

//sub a,r (4 cycles):
//sub a,(addr) (8 cycles):
//Subtract 8-bit value from A, check flags:
#define sub_a_u8(u8) do { \
	HF1 = A; \
	HF2 = u8; \
	ZF = CF = A - HF2; \
	A = ZF & 0xFF; \
	HF2 |= 0x400; \
} while (0)

//sbc a,r (4 cycles):
//sbc a,(addr) (8 cycles):
//Subtract CF and 8-bit value from A, check flags:
#define sbc_a_u8(u8) do { \
	HF1 = A; \
	HF2 = 0x400 | (CF & 0x100) | (u8); \
	ZF = CF = A - ((CF >> 8) & 1) - (u8); \
	A = ZF & 0xFF; \
} while (0)

//and a,r (4 cycles):
//and a,(addr) (8 cycles):
//bitwise and 8-bit value into A, check flags:
#define and_a_u8(u8) do { \
	HF2 = 0x200; \
	CF = 0; \
	A &= (u8); \
	ZF = A; \
} while (0)

//or a,r (4 cycles):
//or a,(hl) (8 cycles):
//bitwise or 8-bit value into A, check flags:
#define or_a_u8(u8) do { \
	CF = HF2 = 0; \
	A |= (u8); \
	ZF = A; \
} while (0)

//xor a,r (4 cycles):
//xor a,(hl) (8 cycles):
//bitwise xor 8-bit value into A, check flags:
#define xor_a_u8(u8) do { \
	CF = HF2 = 0; \
	A ^= (u8); \
	ZF = A; \
} while (0)

//cp a,r (4 cycles):
//cp a,(addr) (8 cycles):
//Compare (subtract without storing result) 8-bit value to A, check flags:
#define cp_a_u8(u8) do { \
	HF1 = A; \
	HF2 = u8; \
	ZF = CF = A - HF2; \
	HF2 |= 0x400; \
} while (0)

//inc r (4 cycles):
//Increment value of 8-bit register, check flags except CF:
#define inc_r(r) do { \
	HF2 = (r) | 0x800; \
	ZF = (r) + 1; \
	(r) = ZF & 0xFF; \
} while (0)

//dec r (4 cycles):
//Decrement value of 8-bit register, check flags except CF:
#define dec_r(r) do { \
	HF2 = (r) | 0xC00; \
	ZF = (r) - 1; \
	(r) = ZF & 0xFF; \
} while (0)

//16-BIT ARITHMETIC
//add hl,rr (8 cycles):
//add 16-bit register to HL, check flags except ZF:
/*#define add_hl_rr(rh, rl) do { \
	L = HF1 = L + (rl); \
	HF1 >>= 8; \
	HF1 += H; \
	HF2 = (rh); \
	H = CF = HF1 + (rh); \
	cycleCounter += 4; \
} while (0)*/

#define add_hl_rr(rh, rl) do { \
	CF = L + (rl); \
	L = CF & 0xFF; \
	HF1 = H; \
	HF2 = (CF & 0x100) | (rh); \
	CF = H + (CF >> 8) + (rh); \
	H = CF & 0xFF; \
	cycleCounter += 4; \
} while (0)

//inc rr (8 cycles):
//Increment 16-bit register:
#define inc_rr(rh, rl) do { \
	const unsigned inc_rr_var_tmp = (rl) + 1; \
	(rl) = inc_rr_var_tmp & 0xFF; \
	(rh) = ((rh) + (inc_rr_var_tmp >> 8)) & 0xFF; \
	cycleCounter += 4; \
} while (0)

//dec rr (8 cycles):
//Decrement 16-bit register:
#define dec_rr(rh, rl) do { \
	const unsigned dec_rr_var_tmp = (rl) - 1; \
	(rl) = dec_rr_var_tmp & 0xFF; \
	(rh) = ((rh) - (dec_rr_var_tmp >> 8 & 1)) & 0xFF; \
	cycleCounter += 4; \
} while (0)

#define sp_plus_n(sumout) do { \
	unsigned sp_plus_n_var_n; \
	PC_READ(sp_plus_n_var_n); \
	sp_plus_n_var_n = (sp_plus_n_var_n ^ 0x80) - 0x80; \
	\
	const unsigned sp_plus_n_var_sum = SP + sp_plus_n_var_n; \
	CF = SP ^ sp_plus_n_var_n ^ sp_plus_n_var_sum; \
	HF2 = CF << 5 & 0x200; \
	ZF = 1; \
	cycleCounter += 4; \
	(sumout) = sp_plus_n_var_sum & 0xFFFF; \
} while (0)

//JUMPS:
//jp nn (16 cycles):
//Jump to address stored in the next two bytes in memory:
#define jp_nn() do { \
	unsigned jp_nn_var_l, jp_nn_var_h; \
\
	PC_READ(jp_nn_var_l); \
	PC_READ(jp_nn_var_h); \
\
	PC_MOD(jp_nn_var_h << 8 | jp_nn_var_l); \
} while (0)

//jr disp (12 cycles):
//Jump to value of next (signed) byte in memory+current address:
#define jr_disp() do { \
	unsigned jr_disp_var_tmp; \
\
	PC_READ(jr_disp_var_tmp); \
	jr_disp_var_tmp = (jr_disp_var_tmp ^ 0x80) - 0x80; \
\
	PC_MOD((PC + jr_disp_var_tmp) & 0xFFFF); \
} while (0)

//CALLS, RESTARTS AND RETURNS:
//call nn (24 cycles):
//Push address of next instruction onto stack and then jump to address stored in next two bytes in memory:
#define call_nn() do { \
	PUSH(((PC + 2) >> 8) & 0xFF, (PC + 2) & 0xFF); \
	jp_nn(); \
} while (0)

//rst n (16 Cycles):
//Push present address onto stack, jump to address n (one of 00h,08h,10h,18h,20h,28h,30h,38h):
#define rst_n(n) do { \
	PUSH(PC >> 8, PC & 0xFF); \
	PC_MOD(n); \
} while (0)

//ret (16 cycles):
//Pop two bytes from the stack and jump to that address:
#define ret() do { \
	unsigned ret_var_l, ret_var_h; \
\
	pop_rr(ret_var_h, ret_var_l); \
\
	PC_MOD(ret_var_h << 8 | ret_var_l); \
} while (0)

void CPU::process(const unsigned long cycles) {
	memory.setEndtime(cycleCounter_, cycles);
	
	unsigned char A = A_;
	unsigned long cycleCounter = cycleCounter_;
	
	while (memory.isActive()) {
		unsigned short PC = PC_;
		
		if (halted) {
			if (cycleCounter < memory.getNextEventTime()) {
				const unsigned long cycles = memory.getNextEventTime() - cycleCounter;
				cycleCounter += cycles + ((4 - (cycles & 3)) & 3);
			}
		} else while (cycleCounter < memory.getNextEventTime()) {
			unsigned char opcode;
			
			PC_READ(opcode);
			
			if (skip) {
				PC = (PC - 1) & 0xFFFF;
				skip = false;
			}
			
			switch (opcode) {
				//nop (4 cycles):
				//Do nothing for 4 cycles:
			case 0x00:
				break;
			case 0x01:
				ld_rr_nn(B, C);
				break;
			case 0x02:
				WRITE(BC(), A);
				break;
			case 0x03:
				inc_rr(B, C);
				break;
			case 0x04:
				inc_r(B);
				break;
			case 0x05:
				dec_r(B);
				break;
			case 0x06:
				PC_READ(B);
				break;

				//rlca (4 cycles):
				//Rotate 8-bit register A left, store old bit7 in CF. Reset SF, HCF, ZF:
			case 0x07:
				CF = A << 1;
				A = (CF | CF >> 8) & 0xFF;
				HF2 = 0;
				ZF = 1;
				break;

				//ld (nn),SP (20 cycles):
				//Put value of SP into address given by next 2 bytes in memory:
			case 0x08:
				{
					unsigned l, h;
					
					PC_READ(l);
					PC_READ(h);
					
					const unsigned addr = h << 8 | l;
					
					WRITE(addr, SP & 0xFF);
					WRITE((addr + 1) & 0xFFFF, SP >> 8);
				}
				break;

			case 0x09:
				add_hl_rr(B, C);
				break;
			case 0x0A:
				READ(A, BC());
				break;
			case 0x0B:
				dec_rr(B, C);
				break;
			case 0x0C:
				inc_r(C);
				break;
			case 0x0D:
				dec_r(C);
				break;
			case 0x0E:
				PC_READ(C);
				break;

				//rrca (4 cycles):
				//Rotate 8-bit register A right, store old bit0 in CF. Reset SF, HCF, ZF:
			case 0x0F:
				CF = A << 8 | A;
				A = CF >> 1 & 0xFF;
				HF2 = 0;
				ZF = 1;
				break;

				//stop (4 cycles):
				//Halt CPU and LCD display until button pressed:
			case 0x10:
				memory.speedChange(cycleCounter);
				PC = (PC + 1) & 0xFFFF;
				break;
			case 0x11:
				ld_rr_nn(D, E);
				break;
			case 0x12:
				WRITE(DE(), A);
				break;
			case 0x13:
				inc_rr(D, E);
				break;
			case 0x14:
				inc_r(D);
				break;
			case 0x15:
				dec_r(D);
				break;
			case 0x16:
				PC_READ(D);
				break;

				//rla (4 cycles):
				//Rotate 8-bit register A left through CF, store old bit7 in CF, old CF value becomes bit0. Reset SF, HCF, ZF:
			case 0x17:
				{
					const unsigned oldcf = CF >> 8 & 1;
					CF = A << 1;
					A = (CF | oldcf) & 0xFF;
				}
				
				HF2 = 0;
				ZF = 1;
				break;

			case 0x18:
				jr_disp();
				break;
			case 0x19:
				add_hl_rr(D, E);
				break;
			case 0x1A:
				READ(A, DE());
				break;
			case 0x1B:
				dec_rr(D, E);
				break;
			case 0x1C:
				inc_r(E);
				break;
			case 0x1D:
				dec_r(E);
				break;
			case 0x1E:
				PC_READ(E);
				break;

				//rra (4 cycles):
				//Rotate 8-bit register A right through CF, store old bit0 in CF, old CF value becomes bit7. Reset SF, HCF, ZF:
			case 0x1F:
				{
					const unsigned oldcf = CF & 0x100;
					CF = A << 8;
					A = (A | oldcf) >> 1;
				}
				
				HF2 = 0;
				ZF = 1;
			break;

			//jr nz,disp (12;8 cycles):
			//Jump to value of next (signed) byte in memory+current address if ZF is unset:
			case 0x20:
				if (ZF & 0xFF) {
					jr_disp();
				} else {
					PC_MOD((PC + 1) & 0xFFFF);
				}
				break;

			case 0x21:
				ld_rr_nn(H, L);
				break;

				//ldi (hl),a (8 cycles):
				//Put A into memory address in hl. Increment HL:
			case 0x22:
				{
					unsigned addr = HL();
					
					WRITE(addr, A);
					
					addr = (addr + 1) & 0xFFFF;
					L = addr;
					H = addr >> 8;
				}
				break;

			case 0x23:
				inc_rr(H, L);
				break;
			case 0x24:
				inc_r(H);
				break;
			case 0x25:
				dec_r(H);
				break;
			case 0x26:
				PC_READ(H);
				break;


				//daa (4 cycles):
				//Adjust register A to correctly represent a BCD. Check ZF, HF and CF:
			case 0x27:
				/*{
					unsigned correction = ((A > 0x99) || (CF & 0x100)) ? 0x60 : 0x00;
					
					calcHF(HF1, HF2);
					
					if ((A & 0x0F) > 0x09 || (HF2 & 0x200))
						correction |= 0x06;
					
					HF1 = A;
					HF2 = (HF2 & 0x400) | correction;
					CF = (correction & 0x40) << 2;
					A = (HF2 & 0x400) ? A - correction : (A + correction);
					ZF = A;
				}*/
				
				calcHF(HF1, HF2);
				
				{
					unsigned correction = (CF & 0x100) ? 0x60 : 0x00;
					
					if (HF2 & 0x200)
						correction |= 0x06;
					
					if (!(HF2 &= 0x400)) {
						if ((A & 0x0F) > 0x09)
							correction |= 0x06;
						
						if (A > 0x99)
							correction |= 0x60;
						
						A += correction;
					} else
						A -= correction;
					
					CF = correction << 2 & 0x100;
					ZF = A;
					A &= 0xFF;
				}
				break;

			//jr z,disp (12;8 cycles):
			//Jump to value of next (signed) byte in memory+current address if ZF is set:
			case 0x28:
				if (ZF & 0xFF) {
					PC_MOD((PC + 1) & 0xFFFF);
				} else {
					jr_disp();
				}
				break;

				//add hl,hl (8 cycles):
				//add 16-bit register HL to HL, check flags except ZF:
			case 0x29:
				add_hl_rr(H, L);
				break;

				//ldi a,(hl) (8 cycles):
				//Put value at address in hl into A. Increment HL:
			case 0x2A:
				{
					unsigned addr = HL();
					
					READ(A, addr);
					
					addr = (addr + 1) & 0xFFFF;
					L = addr;
					H = addr >> 8;
				}
				break;

			case 0x2B:
				dec_rr(H, L);
				break;
			case 0x2C:
				inc_r(L);
				break;
			case 0x2D:
				dec_r(L);
				break;
			case 0x2E:
				PC_READ(L);
				break;

				//cpl (4 cycles):
				//Complement register A. (Flip all bits), set SF and HCF:
			case 0x2F: /*setSubtractFlag(); setHalfCarryFlag();*/
				HF2 = 0x600;
				A ^= 0xFF;
				break;

				//jr nc,disp (12;8 cycles):
				//Jump to value of next (signed) byte in memory+current address if CF is unset:
			case 0x30:
				if (CF & 0x100) {
					PC_MOD((PC + 1) & 0xFFFF);
				} else {
					jr_disp();
				}
				break;

				//ld sp,nn (12 cycles)
				//set sp to 16-bit value of next 2 bytes in memory
			case 0x31:
				{
					unsigned l, h;
					
					PC_READ(l);
					PC_READ(h);
					
					SP = h << 8 | l;
				}
				break;

				//ldd (hl),a (8 cycles):
				//Put A into memory address in hl. Decrement HL:
			case 0x32:
				{
					unsigned addr = HL();
					
					WRITE(addr, A);
					
					addr = (addr - 1) & 0xFFFF;
					L = addr;
					H = addr >> 8;
				}
				break;

			case 0x33:
				SP = (SP + 1) & 0xFFFF;
				cycleCounter += 4;
				break;

				//inc (hl) (12 cycles):
				//Increment value at address in hl, check flags except CF:
			case 0x34: 
				{
					const unsigned addr = HL();
					
					READ(HF2, addr);
					ZF = HF2 + 1;
					WRITE(addr, ZF & 0xFF);
					HF2 |= 0x800;
				}
				break;

			//dec (hl) (12 cycles):
			//Decrement value at address in hl, check flags except CF:
			case 0x35:
				{
					const unsigned addr = HL();
					
					READ(HF2, addr);
					ZF = HF2 - 1;
					WRITE(addr, ZF & 0xFF);
					HF2 |= 0xC00;
				}
				break;

			//ld (hl),n (12 cycles):
			//set memory at address in hl to value of next byte in memory:
			case 0x36:
				{
					unsigned tmp;
					
					PC_READ(tmp);
					WRITE(HL(), tmp);
				}
				break;

			//scf (4 cycles):
			//Set CF. Unset SF and HCF:
			case 0x37: /*setCarryFlag(); unsetSubtractFlag(); unsetHalfCarryFlag();*/
				CF = 0x100;
				HF2 = 0;
				break;

				//jr c,disp (12;8 cycles):
				//Jump to value of next (signed) byte in memory+current address if CF is set:
			case 0x38: //PC+=(((int8_t)memory.read(PC++))*CarryFlag()); Cycles(8); break;
				if (CF & 0x100) {
					jr_disp();
				} else {
					PC_MOD((PC + 1) & 0xFFFF);
				}
				break;

				//add hl,sp (8 cycles):
				//add SP to HL, check flags except ZF:
			case 0x39: /*add_hl_rr(SP>>8, SP); break;*/
				CF = L + SP;
				L = CF & 0xFF;
				HF1 = H;
				HF2 = ((CF ^ SP) & 0x100) | SP >> 8;
				CF >>= 8;
				CF += H;
				H = CF & 0xFF;
				cycleCounter += 4;
				break;

				//ldd a,(hl) (8 cycles):
				//Put value at address in hl into A. Decrement HL:
			case 0x3A:
				{
					unsigned addr = HL();
					
					A = memory.read(addr, cycleCounter);
					cycleCounter += 4;
					
					addr = (addr - 1) & 0xFFFF;
					L = addr;
					H = addr >> 8;
				}
				break;

			case 0x3B:
				SP = (SP - 1) & 0xFFFF;
				cycleCounter += 4;
				break;
			case 0x3C:
				inc_r(A);
				break;
			case 0x3D:
				dec_r(A);
				break;
			case 0x3E:
				PC_READ(A);
				break;

				//ccf (4 cycles):
				//Complement CF (unset if set vv.) Unset SF and HCF.
			case 0x3F: /*complementCarryFlag(); unsetSubtractFlag(); unsetHalfCarryFlag();*/
				CF ^= 0x100;
				HF2 = 0;
				break;

				//ld r,r (4 cycles):next_irqEventTime
				//ld r,(r) (8 cycles):
			case 0x40:
				B = B;
				break;
			case 0x41:
				B = C;
				break;
			case 0x42:
				B = D;
				break;
			case 0x43:
				B = E;
				break;
			case 0x44:
				B = H;
				break;
			case 0x45:
				B = L;
				break;
			case 0x46:
				READ(B, HL());
				break;
			case 0x47:
				B = A;
				break;
			case 0x48:
				C = B;
				break;
			case 0x49:
				C = C;
				break;
			case 0x4A:
				C = D;
				break;
			case 0x4B:
				C = E;
				break;
			case 0x4C:
				C = H;
				break;
			case 0x4D:
				C = L;
				break;
			case 0x4E:
				READ(C, HL());
				break;
			case 0x4F:
				C = A;
				break;
			case 0x50:
				D = B;
				break;
			case 0x51:
				D = C;
				break;
			case 0x52:
				D = D;
				break;
			case 0x53:
				D = E;
				break;
			case 0x54:
				D = H;
				break;
			case 0x55:
				D = L;
				break;
			case 0x56:
				READ(D, HL());
				break;
			case 0x57:
				D = A;
				break;
			case 0x58:
				E = B;
				break;
			case 0x59:
				E = C;
				break;
			case 0x5A:
				E = D;
				break;
			case 0x5B:
				E = E;
				break;
			case 0x5C:
				E = H;
				break;
			case 0x5D:
				E = L;
				break;
			case 0x5E:
				READ(E, HL());
				break;
			case 0x5F:
				E = A;
				break;
			case 0x60:
				H = B;
				break;
			case 0x61:
				H = C;
				break;
			case 0x62:
				H = D;
				break;
			case 0x63:
				H = E;
				break;
			case 0x64:
				H = H;
				break;
			case 0x65:
				H = L;
				break;
			case 0x66:
				READ(H, HL());
				break;
			case 0x67:
				H = A;
				break;
			case 0x68:
				L = B;
				break;
			case 0x69:
				L = C;
				break;
			case 0x6A:
				L = D;
				break;
			case 0x6B:
				L = E;
				break;
			case 0x6C:
				L = H;
				break;
			case 0x6D:
				L = L;
				break;
			case 0x6E:
				READ(L, HL());
				break;
			case 0x6F:
				L = A;
				break;
			case 0x70:
				WRITE(HL(), B);
				break;
			case 0x71:
				WRITE(HL(), C);
				break;
			case 0x72:
				WRITE(HL(), D);
				break;
			case 0x73:
				WRITE(HL(), E);
				break;
			case 0x74:
				WRITE(HL(), H);
				break;
			case 0x75:
				WRITE(HL(), L);
				break;

				//halt (4 cycles):
			case 0x76:
// 				printf("halt\n");
				if (memory.getIME()/* || memory.next_eitime*/) {
					halted = 1;

					if (cycleCounter < memory.getNextEventTime()) {
						const unsigned long cycles = memory.getNextEventTime() - cycleCounter;
						cycleCounter += cycles + ((4 - (cycles & 3)) & 3);
					}
				} else {
					if ((memory.ff_read(0xFF0F, cycleCounter) & memory.ff_read(0xFFFF, cycleCounter)) & 0x1F) {
						if (memory.isCgb())
							cycleCounter += 8; //two nops.
						else
							skip = true;
					} else {
						memory.schedule_unhalt();
						halted = 1;
						
						if (cycleCounter < memory.getNextEventTime()) {
							const unsigned long cycles = memory.getNextEventTime() - cycleCounter;
							cycleCounter += cycles + ((4 - (cycles & 3)) & 3);
						}
					}
				}
				break;
			case 0x77:
				WRITE(HL(), A);
				break;
			case 0x78:
				A = B;
				break;
			case 0x79:
				A = C;
				break;
			case 0x7A:
				A = D;
				break;
			case 0x7B:
				A = E;
				break;
			case 0x7C:
				A = H;
				break;
			case 0x7D:
				A = L;
				break;
			case 0x7E:
				READ(A, HL());
				break;
			case 0x7F:
				A = A;
				break;
			case 0x80:
				add_a_u8(B);
				break;
			case 0x81:
				add_a_u8(C);
				break;
			case 0x82:
				add_a_u8(D);
				break;
			case 0x83:
				add_a_u8(E);
				break;
			case 0x84:
				add_a_u8(H);
				break;
			case 0x85:
				add_a_u8(L);
				break;
			case 0x86:
				{
					unsigned data;

					READ(data, HL());
					
					add_a_u8(data);
				}
				break;
			case 0x87:
				add_a_u8(A);
				break;
			case 0x88:
				adc_a_u8(B);
				break;
			case 0x89:
				adc_a_u8(C);
				break;
			case 0x8A:
				adc_a_u8(D);
				break;
			case 0x8B:
				adc_a_u8(E);
				break;
			case 0x8C:
				adc_a_u8(H);
				break;
			case 0x8D:
				adc_a_u8(L);
				break;
			case 0x8E:
				{
					unsigned data;
					
					READ(data, HL());
					
					adc_a_u8(data);
				}
				break;
			case 0x8F:
				adc_a_u8(A);
				break;
			case 0x90:
				sub_a_u8(B);
				break;
			case 0x91:
				sub_a_u8(C);
				break;
			case 0x92:
				sub_a_u8(D);
				break;
			case 0x93:
				sub_a_u8(E);
				break;
			case 0x94:
				sub_a_u8(H);
				break;
			case 0x95:
				sub_a_u8(L);
				break;
			case 0x96:
				{
					unsigned data;
					
					READ(data, HL());
					
					sub_a_u8(data);
				}
				break;
				//A-A is always 0:
			case 0x97:
				HF2 = 0x400;
				CF = ZF = A = 0;
				break;
			case 0x98:
				sbc_a_u8(B);
				break;
			case 0x99:
				sbc_a_u8(C);
				break;
			case 0x9A:
				sbc_a_u8(D);
				break;
			case 0x9B:
				sbc_a_u8(E);
				break;
			case 0x9C:
				sbc_a_u8(H);
				break;
			case 0x9D:
				sbc_a_u8(L);
				break;
			case 0x9E:
				{
					unsigned data;
					
					READ(data, HL());
					
					sbc_a_u8(data);
				}
				break;
			case 0x9F:
				sbc_a_u8(A);
				break;
			case 0xA0:
				and_a_u8(B);
				break;
			case 0xA1:
				and_a_u8(C);
				break;
			case 0xA2:
				and_a_u8(D);
				break;
			case 0xA3:
				and_a_u8(E);
				break;
			case 0xA4:
				and_a_u8(H);
				break;
			case 0xA5:
				and_a_u8(L);
				break;
			case 0xA6:
				{
					unsigned data;
					
					READ(data, HL());
					
					and_a_u8(data);
				}
				break;
				//A&A will always be A:
			case 0xA7:
				ZF = A;
				CF = 0;
				HF2 = 0x200;
				break;
			case 0xA8:
				xor_a_u8(B);
				break;
			case 0xA9:
				xor_a_u8(C);
				break;
			case 0xAA:
				xor_a_u8(D);
				break;
			case 0xAB:
				xor_a_u8(E);
				break;
			case 0xAC:
				xor_a_u8(H);
				break;
			case 0xAD:
				xor_a_u8(L);
				break;
			case 0xAE:
				{
					unsigned data;
					
					READ(data, HL());
					
					xor_a_u8(data);
				}
				break;
				//A^A will always be 0:
			case 0xAF:
				CF = HF2 = ZF = A = 0;
				break;
			case 0xB0:
				or_a_u8(B);
				break;
			case 0xB1:
				or_a_u8(C);
				break;
			case 0xB2:
				or_a_u8(D);
				break;
			case 0xB3:
				or_a_u8(E);
				break;
			case 0xB4:
				or_a_u8(H);
				break;
			case 0xB5:
				or_a_u8(L);
				break;
			case 0xB6:
				{
					unsigned data;
					
					READ(data, HL());
					
					or_a_u8(data);
				}
				break;
				//A|A will always be A:
			case 0xB7:
				ZF = A;
				HF2 = CF = 0;
				break;
			case 0xB8:
				cp_a_u8(B);
				break;
			case 0xB9:
				cp_a_u8(C);
				break;
			case 0xBA:
				cp_a_u8(D);
				break;
			case 0xBB:
				cp_a_u8(E);
				break;
			case 0xBC:
				cp_a_u8(H);
				break;
			case 0xBD:
				cp_a_u8(L);
				break;
			case 0xBE:
				{
					unsigned data;
					
					READ(data, HL());
					
					cp_a_u8(data);
				}
				break;
				//A always equals A:
			case 0xBF:
				CF = ZF = 0;
				HF2 = 0x400;
				break;

				//ret nz (20;8 cycles):
				//Pop two bytes from the stack and jump to that address, if ZF is unset:
			case 0xC0:
				cycleCounter += 4;
				
				if (ZF & 0xFF) {
					ret();
				}
				break;

			case 0xC1:
				pop_rr(B, C);
				break;

				//jp nz,nn (16;12 cycles):
				//Jump to address stored in next two bytes in memory if ZF is unset:
			case 0xC2:
				if (ZF & 0xFF) {
					jp_nn();
				} else {
					PC_MOD((PC + 2) & 0xFFFF);
					cycleCounter += 4;
				}
				break;

			case 0xC3:
				jp_nn();
				break;

				//call nz,nn (24;12 cycles):
				//Push address of next instruction onto stack and then jump to address stored in next two bytes in memory, if ZF is unset:
			case 0xC4:
				if (ZF & 0xFF) {
					call_nn();
				} else {
					PC_MOD((PC + 2) & 0xFFFF);
					cycleCounter += 4;
				}
				break;

			case 0xC5:
				push_rr(B, C);
				break;
			case 0xC6:
				{
					unsigned data;
					
					PC_READ(data);
					
					add_a_u8(data);
				}
				break;
			case 0xC7:
				rst_n(0x00);
				break;

				//ret z (20;8 cycles):
				//Pop two bytes from the stack and jump to that address, if ZF is set:
			case 0xC8:
				cycleCounter += 4;
				
				if (!(ZF & 0xFF)) {
					ret();
				}
				
				break;

				//ret (16 cycles):
				//Pop two bytes from the stack and jump to that address:
			case 0xC9:
				ret();
				break;

				//jp z,nn (16;12 cycles):
				//Jump to address stored in next two bytes in memory if ZF is set:
			case 0xCA:
				if (ZF & 0xFF) {
					PC_MOD((PC + 2) & 0xFFFF);
					cycleCounter += 4;
				} else {
					jp_nn();
				}
				break;


				//CB OPCODES (Shifts, rotates and bits):
			case 0xCB:
				PC_READ(opcode);
				
				switch (opcode) {
				case 0x00:
					rlc_r(B);
					break;
				case 0x01:
					rlc_r(C);
					break;
				case 0x02:
					rlc_r(D);
					break;
				case 0x03:
					rlc_r(E);
					break;
				case 0x04:
					rlc_r(H);
					break;
				case 0x05:
					rlc_r(L);
					break;
					//rlc (hl) (16 cycles):
					//Rotate 8-bit value stored at address in HL left, store old bit7 in CF. Reset SF and HCF. Check ZF:
				case 0x06:
					{
						const unsigned addr = HL();
						
						READ(CF, addr);
						CF <<= 1;
						
						ZF = CF | (CF >> 8);

						WRITE(addr, ZF & 0xFF);
						
						HF2 = 0;
					}
					break;
				case 0x07:
					rlc_r(A);
					break;
				case 0x08:
					rrc_r(B);
					break;
				case 0x09:
					rrc_r(C);
					break;
				case 0x0A:
					rrc_r(D);
					break;
				case 0x0B:
					rrc_r(E);
					break;
				case 0x0C:
					rrc_r(H);
					break;
				case 0x0D:
					rrc_r(L);
					break;
					//rrc (hl) (16 cycles):
					//Rotate 8-bit value stored at address in HL right, store old bit0 in CF. Reset SF and HCF. Check ZF:
				case 0x0E:
					{
						const unsigned addr = HL();
						
						READ(ZF, addr);
						
						CF = ZF << 8;
						
						WRITE(addr, (ZF | CF) >> 1 & 0xFF);
						
						HF2 = 0;
					}
					break;
				case 0x0F:
					rrc_r(A);
					break;
				case 0x10:
					rl_r(B);
					break;
				case 0x11:
					rl_r(C);
					break;
				case 0x12:
					rl_r(D);
					break;
				case 0x13:
					rl_r(E);
					break;
				case 0x14:
					rl_r(H);
					break;
				case 0x15:
					rl_r(L);
					break;
					//rl (hl) (16 cycles):
					//Rotate 8-bit value stored at address in HL left thorugh CF, store old bit7 in CF, old CF value becoms bit0. Reset SF and HCF. Check ZF:
				case 0x16:
					{
						const unsigned addr = HL();
						const unsigned oldcf = CF >> 8 & 1;
						
						READ(CF, addr);
						CF <<= 1;
						
						ZF = CF | oldcf;
						
						WRITE(addr, ZF & 0xFF);
						
						HF2 = 0;
					}
					break;
				case 0x17:
					rl_r(A);
					break;
				case 0x18:
					rr_r(B);
					break;
				case 0x19:
					rr_r(C);
					break;
				case 0x1A:
					rr_r(D);
					break;
				case 0x1B:
					rr_r(E);
					break;
				case 0x1C:
					rr_r(H);
					break;
				case 0x1D:
					rr_r(L);
					break;
					//rr (hl) (16 cycles):
					//Rotate 8-bit value stored at address in HL right thorugh CF, store old bit0 in CF, old CF value becoms bit7. Reset SF and HCF. Check ZF:
				case 0x1E:
					{
						const unsigned addr = HL();
						
						READ(ZF, addr);
						
						const unsigned oldcf = CF & 0x100;
						CF = ZF << 8;
						ZF = (ZF | oldcf) >> 1;
						
						WRITE(addr, ZF);
						
						HF2 = 0;
					}
					break;
				case 0x1F:
					rr_r(A);
					break;
				case 0x20:
					sla_r(B);
					break;
				case 0x21:
					sla_r(C);
					break;
				case 0x22:
					sla_r(D);
					break;
				case 0x23:
					sla_r(E);
					break;
				case 0x24:
					sla_r(H);
					break;
				case 0x25:
					sla_r(L);
					break;
					//sla (hl) (16 cycles):
					//Shift 8-bit value stored at address in HL left, store old bit7 in CF. Reset SF and HCF. Check ZF:
				case 0x26:
					{
						const unsigned addr = HL();
						
						READ(CF, addr);
						CF <<= 1;
						
						ZF = CF;
						
						WRITE(addr, ZF & 0xFF);
						
						HF2 = 0;
					}
					break;
				case 0x27:
					sla_r(A);
					break;
				case 0x28:
					sra_r(B);
					break;
				case 0x29:
					sra_r(C);
					break;
				case 0x2A:
					sra_r(D);
					break;
				case 0x2B:
					sra_r(E);
					break;
				case 0x2C:
					sra_r(H);
					break;
				case 0x2D:
					sra_r(L);
					break;
					//sra (hl) (16 cycles):
					//Shift 8-bit value stored at address in HL right, store old bit0 in CF, bit7=old bit7. Reset SF and HCF. Check ZF:
				case 0x2E:
					{
						const unsigned addr = HL();
						
						READ(CF, addr);
						
						ZF = CF >> 1;
						
						WRITE(addr, ZF | (CF & 0x80));
						
						CF <<= 8;
						HF2 = 0;
					}
					break;
				case 0x2F:
					sra_r(A);
					break;
				case 0x30:
					swap_r(B);
					break;
				case 0x31:
					swap_r(C);
					break;
				case 0x32:
					swap_r(D);
					break;
				case 0x33:
					swap_r(E);
					break;
				case 0x34:
					swap_r(H);
					break;
				case 0x35:
					swap_r(L);
					break;
					//swap (hl) (16 cycles):
					//Swap upper and lower nibbles of 8-bit value stored at address in HL, reset flags, check zero flag:
				case 0x36:
					{
						const unsigned addr = HL();
						
						READ(ZF, addr);
						
						WRITE(addr, (ZF << 4 | ZF >> 4) & 0xFF);
						
						CF = HF2 = 0;
					}
					break;
				case 0x37:
					swap_r(A);
					break;
				case 0x38:
					srl_r(B);
					break;
				case 0x39:
					srl_r(C);
					break;
				case 0x3A:
					srl_r(D);
					break;
				case 0x3B:
					srl_r(E);
					break;
				case 0x3C:
					srl_r(H);
					break;
				case 0x3D:
					srl_r(L);
					break;
					//srl (hl) (16 cycles):
					//Shift 8-bit value stored at address in HL right, store old bit0 in CF. Reset SF and HCF. Check ZF:
				case 0x3E:
					{
						const unsigned addr = HL();
						
						READ(CF, addr);
						
						ZF = CF >> 1;
						
						WRITE(addr, ZF);
						
						CF <<= 8;
						HF2 = 0;
					}
					break;
				case 0x3F:
					srl_r(A);
					break;
				case 0x40:
					bit0_u8(B);
					break;
				case 0x41:
					bit0_u8(C);
					break;
				case 0x42:
					bit0_u8(D);
					break;
				case 0x43:
					bit0_u8(E);
					break;
				case 0x44:
					bit0_u8(H);
					break;
				case 0x45:
					bit0_u8(L);
					break;
				case 0x46:
					{
						unsigned data;
						
						READ(data, HL());
						
						bit0_u8(data);
					}
					break;
				case 0x47:
					bit0_u8(A);
					break;
				case 0x48:
					bit1_u8(B);
					break;
				case 0x49:
					bit1_u8(C);
					break;
				case 0x4A:
					bit1_u8(D);
					break;
				case 0x4B:
					bit1_u8(E);
					break;
				case 0x4C:
					bit1_u8(H);
					break;
				case 0x4D:
					bit1_u8(L);
					break;
				case 0x4E:
					{
						unsigned data;
						
						READ(data, HL());
						
						bit1_u8(data);
					}
					break;
				case 0x4F:
					bit1_u8(A);
					break;
				case 0x50:
					bit2_u8(B);
					break;
				case 0x51:
					bit2_u8(C);
					break;
				case 0x52:
					bit2_u8(D);
					break;
				case 0x53:
					bit2_u8(E);
					break;
				case 0x54:
					bit2_u8(H);
					break;
				case 0x55:
					bit2_u8(L);
					break;
				case 0x56:
					{
						unsigned data;
						
						READ(data, HL());
						
						bit2_u8(data);
					}
					break;
				case 0x57:
					bit2_u8(A);
					break;
				case 0x58:
					bit3_u8(B);
					break;
				case 0x59:
					bit3_u8(C);
					break;
				case 0x5A:
					bit3_u8(D);
					break;
				case 0x5B:
					bit3_u8(E);
					break;
				case 0x5C:
					bit3_u8(H);
					break;
				case 0x5D:
					bit3_u8(L);
					break;
				case 0x5E:
					{
						unsigned data;
						
						READ(data, HL());
						
						bit3_u8(data);
					}
					break;
				case 0x5F:
					bit3_u8(A);
					break;
				case 0x60:
					bit4_u8(B);
					break;
				case 0x61:
					bit4_u8(C);
					break;
				case 0x62:
					bit4_u8(D);
					break;
				case 0x63:
					bit4_u8(E);
					break;
				case 0x64:
					bit4_u8(H);
					break;
				case 0x65:
					bit4_u8(L);
					break;
				case 0x66:
					{
						unsigned data;
						
						READ(data, HL());
						
						bit4_u8(data);
					}
					break;
				case 0x67:
					bit4_u8(A);
					break;
				case 0x68:
					bit5_u8(B);
					break;
				case 0x69:
					bit5_u8(C);
					break;
				case 0x6A:
					bit5_u8(D);
					break;
				case 0x6B:
					bit5_u8(E);
					break;
				case 0x6C:
					bit5_u8(H);
					break;
				case 0x6D:
					bit5_u8(L);
					break;
				case 0x6E:
					{
						unsigned data;
						
						READ(data, HL());
						
						bit5_u8(data);
					}
					break;
				case 0x6F:
					bit5_u8(A);
					break;
				case 0x70:
					bit6_u8(B);
					break;
				case 0x71:
					bit6_u8(C);
					break;
				case 0x72:
					bit6_u8(D);
					break;
				case 0x73:
					bit6_u8(E);
					break;
				case 0x74:
					bit6_u8(H);
					break;
				case 0x75:
					bit6_u8(L);
					break;
				case 0x76:
					{
						unsigned data;
						
						READ(data, HL());
						
						bit6_u8(data);
					}
					break;
				case 0x77:
					bit6_u8(A);
					break;
				case 0x78:
					bit7_u8(B);
					break;
				case 0x79:
					bit7_u8(C);
					break;
				case 0x7A:
					bit7_u8(D);
					break;
				case 0x7B:
					bit7_u8(E);
					break;
				case 0x7C:
					bit7_u8(H);
					break;
				case 0x7D:
					bit7_u8(L);
					break;
				case 0x7E:
					{
						unsigned data;
						
						READ(data, HL());
						
						bit7_u8(data);
					}
					break;
				case 0x7F:
					bit7_u8(A);
					break;
				case 0x80:
					res0_r(B);
					break;
				case 0x81:
					res0_r(C);
					break;
				case 0x82:
					res0_r(D);
					break;
				case 0x83:
					res0_r(E);
					break;
				case 0x84:
					res0_r(H);
					break;
				case 0x85:
					res0_r(L);
					break;
				case 0x86:
					resn_mem_hl(0);
					break;
				case 0x87:
					res0_r(A);
					break;
				case 0x88:
					res1_r(B);
					break;
				case 0x89:
					res1_r(C);
					break;
				case 0x8A:
					res1_r(D);
					break;
				case 0x8B:
					res1_r(E);
					break;
				case 0x8C:
					res1_r(H);
					break;
				case 0x8D:
					res1_r(L);
					break;
				case 0x8E:
					resn_mem_hl(1);
					break;
				case 0x8F:
					res1_r(A);
					break;
				case 0x90:
					res2_r(B);
					break;
				case 0x91:
					res2_r(C);
					break;
				case 0x92:
					res2_r(D);
					break;
				case 0x93:
					res2_r(E);
					break;
				case 0x94:
					res2_r(H);
					break;
				case 0x95:
					res2_r(L);
					break;
				case 0x96:
					resn_mem_hl(2);
					break;
				case 0x97:
					res2_r(A);
					break;
				case 0x98:
					res3_r(B);
					break;
				case 0x99:
					res3_r(C);
					break;
				case 0x9A:
					res3_r(D);
					break;
				case 0x9B:
					res3_r(E);
					break;
				case 0x9C:
					res3_r(H);
					break;
				case 0x9D:
					res3_r(L);
					break;
				case 0x9E:
					resn_mem_hl(3);
					break;
				case 0x9F:
					res3_r(A);
					break;
				case 0xA0:
					res4_r(B);
					break;
				case 0xA1:
					res4_r(C);
					break;
				case 0xA2:
					res4_r(D);
					break;
				case 0xA3:
					res4_r(E);
					break;
				case 0xA4:
					res4_r(H);
					break;
				case 0xA5:
					res4_r(L);
					break;
				case 0xA6:
					resn_mem_hl(4);
					break;
				case 0xA7:
					res4_r(A);
					break;
				case 0xA8:
					res5_r(B);
					break;
				case 0xA9:
					res5_r(C);
					break;
				case 0xAA:
					res5_r(D);
					break;
				case 0xAB:
					res5_r(E);
					break;
				case 0xAC:
					res5_r(H);
					break;
				case 0xAD:
					res5_r(L);
					break;
				case 0xAE:
					resn_mem_hl(5);
					break;
				case 0xAF:
					res5_r(A);
					break;
				case 0xB0:
					res6_r(B);
					break;
				case 0xB1:
					res6_r(C);
					break;
				case 0xB2:
					res6_r(D);
					break;
				case 0xB3:
					res6_r(E);
					break;
				case 0xB4:
					res6_r(H);
					break;
				case 0xB5:
					res6_r(L);
					break;
				case 0xB6:
					resn_mem_hl(6);
					break;
				case 0xB7:
					res6_r(A);
					break;
				case 0xB8:
					res7_r(B);
					break;
				case 0xB9:
					res7_r(C);
					break;
				case 0xBA:
					res7_r(D);
					break;
				case 0xBB:
					res7_r(E);
					break;
				case 0xBC:
					res7_r(H);
					break;
				case 0xBD:
					res7_r(L);
					break;
				case 0xBE:
					resn_mem_hl(7);
					break;
				case 0xBF:
					res7_r(A);
					break;
				case 0xC0:
					set0_r(B);
					break;
				case 0xC1:
					set0_r(C);
					break;
				case 0xC2:
					set0_r(D);
					break;
				case 0xC3:
					set0_r(E);
					break;
				case 0xC4:
					set0_r(H);
					break;
				case 0xC5:
					set0_r(L);
					break;
				case 0xC6:
					setn_mem_hl(0);
					break;
				case 0xC7:
					set0_r(A);
					break;
				case 0xC8:
					set1_r(B);
					break;
				case 0xC9:
					set1_r(C);
					break;
				case 0xCA:
					set1_r(D);
					break;
				case 0xCB:
					set1_r(E);
					break;
				case 0xCC:
					set1_r(H);
					break;
				case 0xCD:
					set1_r(L);
					break;
				case 0xCE:
					setn_mem_hl(1);
					break;
				case 0xCF:
					set1_r(A);
					break;
				case 0xD0:
					set2_r(B);
					break;
				case 0xD1:
					set2_r(C);
					break;
				case 0xD2:
					set2_r(D);
					break;
				case 0xD3:
					set2_r(E);
					break;
				case 0xD4:
					set2_r(H);
					break;
				case 0xD5:
					set2_r(L);
					break;
				case 0xD6:
					setn_mem_hl(2);
					break;
				case 0xD7:
					set2_r(A);
					break;
				case 0xD8:
					set3_r(B);
					break;
				case 0xD9:
					set3_r(C);
					break;
				case 0xDA:
					set3_r(D);
					break;
				case 0xDB:
					set3_r(E);
					break;
				case 0xDC:
					set3_r(H);
					break;
				case 0xDD:
					set3_r(L);
					break;
				case 0xDE:
					setn_mem_hl(3);
					break;
				case 0xDF:
					set3_r(A);
					break;
				case 0xE0:
					set4_r(B);
					break;
				case 0xE1:
					set4_r(C);
					break;
				case 0xE2:
					set4_r(D);
					break;
				case 0xE3:
					set4_r(E);
					break;
				case 0xE4:
					set4_r(H);
					break;
				case 0xE5:
					set4_r(L);
					break;
				case 0xE6:
					setn_mem_hl(4);
					break;
				case 0xE7:
					set4_r(A);
					break;
				case 0xE8:
					set5_r(B);
					break;
				case 0xE9:
					set5_r(C);
					break;
				case 0xEA:
					set5_r(D);
					break;
				case 0xEB:
					set5_r(E);
					break;
				case 0xEC:
					set5_r(H);
					break;
				case 0xED:
					set5_r(L);
					break;
				case 0xEE:
					setn_mem_hl(5);
					break;
				case 0xEF:
					set5_r(A);
					break;
				case 0xF0:
					set6_r(B);
					break;
				case 0xF1:
					set6_r(C);
					break;
				case 0xF2:
					set6_r(D);
					break;
				case 0xF3:
					set6_r(E);
					break;
				case 0xF4:
					set6_r(H);
					break;
				case 0xF5:
					set6_r(L);
					break;
				case 0xF6:
					setn_mem_hl(6);
					break;
				case 0xF7:
					set6_r(A);
					break;
				case 0xF8:
					set7_r(B);
					break;
				case 0xF9:
					set7_r(C);
					break;
				case 0xFA:
					set7_r(D);
					break;
				case 0xFB:
					set7_r(E);
					break;
				case 0xFC:
					set7_r(H);
					break;
				case 0xFD:
					set7_r(L);
					break;
				case 0xFE:
					setn_mem_hl(7);
					break;
				case 0xFF:
					set7_r(A);
					break;
// 	default: break;
				}
				break;


				//call z,nn (24;12 cycles):
				//Push address of next instruction onto stack and then jump to address stored in next two bytes in memory, if ZF is set:
			case 0xCC:
				if (ZF & 0xFF) {
					PC_MOD((PC + 2) & 0xFFFF);
					cycleCounter += 4;
				} else {
					call_nn();
				}
				break;

			case 0xCD:
				call_nn();
				break;
			case 0xCE:
				{
					unsigned data;
					
					PC_READ(data);
					
					adc_a_u8(data);
				}
				break;
			case 0xCF:
				rst_n(0x08);
				break;

				//ret nc (20;8 cycles):
				//Pop two bytes from the stack and jump to that address, if CF is unset:
			case 0xD0:
				cycleCounter += 4;
				
				if (!(CF & 0x100)) {
					ret();
				}
				
				break;

			case 0xD1:
				pop_rr(D, E);
				break;

				//jp nc,nn (16;12 cycles):
				//Jump to address stored in next two bytes in memory if CF is unset:
			case 0xD2:
				if (CF & 0x100) {
					PC_MOD((PC + 2) & 0xFFFF);
					cycleCounter += 4;
				} else {
					jp_nn();
				}
				break;

			case 0xD3: /*doesn't exist*/
				break;

				//call nc,nn (24;12 cycles):
				//Push address of next instruction onto stack and then jump to address stored in next two bytes in memory, if CF is unset:
			case 0xD4:
				if (CF & 0x100) {
					PC_MOD((PC + 2) & 0xFFFF);
					cycleCounter += 4;
				} else {
					call_nn();
				}
				break;

			case 0xD5:
				push_rr(D, E);
				break;
			case 0xD6:
				{
					unsigned data;
					
					PC_READ(data);
					
					sub_a_u8(data);
				}
				break;
			case 0xD7:
				rst_n(0x10);
				break;

				//ret c (20;8 cycles):
				//Pop two bytes from the stack and jump to that address, if CF is set:
			case 0xD8:
				cycleCounter += 4;
				
				if (CF & 0x100) {
					ret();
				}
				
				break;

				//reti (16 cycles):
				//Pop two bytes from the stack and jump to that address, then enable interrupts:
			case 0xD9:
				{
					unsigned l, h;
					
					pop_rr(h, l);
					
					memory.ei(cycleCounter);
					
					PC_MOD(h << 8 | l);
				}
				break;

				//jp c,nn (16;12 cycles):
				//Jump to address stored in next two bytes in memory if CF is set:
			case 0xDA: //PC=( ((PC+2)*(1-CarryFlag())) + (((memory.read(PC+1)<<8)+memory.read(PC))*CarryFlag()) ); Cycles(12); break;
				if (CF & 0x100) {
					jp_nn();
				} else {
					PC_MOD((PC + 2) & 0xFFFF);
					cycleCounter += 4;
				}
				break;

			case 0xDB: /*doesn't exist*/
				break;

				//call z,nn (24;12 cycles):
				//Push address of next instruction onto stack and then jump to address stored in next two bytes in memory, if CF is set:
			case 0xDC:
				if (CF & 0x100) {
					call_nn();
				} else {
					PC_MOD((PC + 2) & 0xFFFF);
					cycleCounter += 4;
				}
				break;

			case 0xDE:
				{
					unsigned data;
					
					PC_READ(data);
					
					sbc_a_u8(data);
				}
				break;
			case 0xDF:
				rst_n(0x18);
				break;

				//ld ($FF00+n),a (12 cycles):
				//Put value in A into address (0xFF00 + next byte in memory):
			case 0xE0:
				{
					unsigned tmp;
					
					PC_READ(tmp);
					
					FF_WRITE(0xFF00 | tmp, A);
				}
				break;

			case 0xE1:
				pop_rr(H, L);
				break;

				//ld ($FF00+C),a (8 ycles):
				//Put A into address (0xFF00 + register C):
			case 0xE2:
				FF_WRITE(0xFF00 | C, A);
				break;
			case 0xE3: /*doesn't exist*/
				break;
			case 0xE4: /*doesn't exist*/
				break;
			case 0xE5:
				push_rr(H, L);
				break;
			case 0xE6:
				{
					unsigned data;
					
					PC_READ(data);
					
					and_a_u8(data);
				}
				break;
			case 0xE7:
				rst_n(0x20);
				break;

				//add sp,n (16 cycles):
				//Add next (signed) byte in memory to SP, reset ZF and SF, check HCF and CF:
			case 0xE8:
				/*{
					int8_t tmp = int8_t(memory.pc_read(PC++, cycleCounter));
					HF2 = (((SP & 0xFFF) + tmp) >> 3) & 0x200;
					CF = SP + tmp;
					SP = CF;
					CF >>= 8;
					ZF = 1;
					cycleCounter += 12;
				}*/
				sp_plus_n(SP);
				cycleCounter += 4;
			break;

			//jp hl (4 cycles):
			//Jump to address in hl:
			case 0xE9:
				PC = HL();
				break;

				//ld (nn),a (16 cycles):
				//set memory at address given by the next 2 bytes to value in A:
				//Incrementing PC before call, because of possible interrupt.
			case 0xEA:
				{
					unsigned l, h;
					
					PC_READ(l);
					PC_READ(h);
					
					WRITE(h << 8 | l, A);
				}
				break;

			case 0xEB: /*doesn't exist*/
				break;
			case 0xEC: /*doesn't exist*/
				break;
			case 0xED: /*doesn't exist*/
				break;
			case 0xEE:
				{
					unsigned data;
					
					PC_READ(data);
					
					xor_a_u8(data);
				}
				break;
			case 0xEF:
				rst_n(0x28);
				break;

				//ld a,($FF00+n) (12 cycles):
				//Put value at address (0xFF00 + next byte in memory) into A:
			case 0xF0:
				{
					unsigned tmp;
					
					PC_READ(tmp);
					
					FF_READ(A, 0xFF00 | tmp);
				}
				break;

			case 0xF1: /*pop_rr(A, F); Cycles(12); break;*/
				{
					unsigned F;
					
					pop_rr(A, F);
					
					FROM_F(F);
				}
				break;

				//ld a,($FF00+C) (8 cycles):
				//Put value at address (0xFF00 + register C) into A:
			case 0xF2:
				FF_READ(A, 0xFF00 | C);
				break;

				//di (4 cycles):
			case 0xF3:
				memory.di();
				break;

			case 0xF4: /*doesn't exist*/
				break;
			case 0xF5: /*push_rr(A, F); Cycles(16); break;*/
				calcHF(HF1, HF2);
				
				{
					unsigned F = F();
					
					push_rr(A, F);
				}
				break;

			case 0xF6:
				{
					unsigned data;

					PC_READ(data);
					
					or_a_u8(data);
				}
				break;
			case 0xF7:
				rst_n(0x30);
				break;

				//ldhl sp,n (12 cycles):
				//Put (sp+next (signed) byte in memory) into hl (unsets ZF and SF, may enable HF and CF):
			case 0xF8:
				/*{
					int8_t tmp = int8_t(memory.pc_read(PC++, cycleCounter));
					HF2 = (((SP & 0xFFF) + tmp) >> 3) & 0x200;
					CF = SP + tmp;
					L = CF;
					CF >>= 8;
					H = CF;
					ZF = 1;
					cycleCounter += 8;
				}*/
				{
					unsigned sum;
					sp_plus_n(sum);
					L = sum & 0xFF;
					H = sum >> 8;
				}
				break;

			//ld sp,hl (8 cycles):
			//Put value in HL into SP
			case 0xF9:
				SP = HL();
				cycleCounter += 4;
				break;

				//ld a,(nn) (16 cycles):
				//set A to value in memory at address given by the 2 next bytes.
			case 0xFA:
				{
					unsigned l, h;
					
					PC_READ(l);
					PC_READ(h);
					
					READ(A, h << 8 | l);
				}
				break;

				//ei (4 cycles):
				//Enable Interrupts after next instruction:
			case 0xFB:
				memory.ei(cycleCounter);
				break;

			case 0xFC: /*doesn't exist*/
				break;
			case 0xFD: /*doesn't exist*/
				break;
			case 0xFE:
				{
					unsigned data;

					PC_READ(data);
					
					cp_a_u8(data);
				}
				break;
			case 0xFF:
				rst_n(0x38);
				break;
//     default: break;
			}
		}
		
		PC_ = PC;
		cycleCounter = memory.event(cycleCounter);
	}
	
	A_ = A;
	cycleCounter_ = cycleCounter;
}
