/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - ops.h                                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

typedef struct _cpu_instruction_table
{
	/* All jump/branch instructions (except JR and JALR) have three versions:
	 * - JUMPNAME() which for jumps inside the current block.
	 * - JUMPNAME_OUT() which jumps outside the current block.
	 * - JUMPNAME_IDLE() which does busy wait optimization.
	 *
	 * Busy wait optimization is used when a jump jumps to itself,
	 * and the instruction on the delay slot is a NOP.
	 * The program is waiting for the next interrupt, so we can just
	 * increase Count until the point where the next interrupt happens. */

	// Load and store instructions
	void (*LB)(void);
	void (*LBU)(void);
	void (*LH)(void);
	void (*LHU)(void);
	void (*LW)(void);
	void (*LWL)(void);
	void (*LWR)(void);
	void (*SB)(void);
	void (*SH)(void);
	void (*SW)(void);
	void (*SWL)(void);
	void (*SWR)(void);

	void (*LD)(void);
	void (*LDL)(void);
	void (*LDR)(void);
	void (*LL)(void);
	void (*LWU)(void);
	void (*SC)(void);
	void (*SD)(void);
	void (*SDL)(void);
	void (*SDR)(void);
	void (*SYNC)(void);

	// Arithmetic instructions (ALU immediate)
	void (*ADDI)(void);
	void (*ADDIU)(void);
	void (*SLTI)(void);
	void (*SLTIU)(void);
	void (*ANDI)(void);
	void (*ORI)(void);
	void (*XORI)(void);
	void (*LUI)(void);

	void (*DADDI)(void);
	void (*DADDIU)(void);

	// Arithmetic instructions (3-operand)
	void (*ADD)(void);
	void (*ADDU)(void);
	void (*SUB)(void);
	void (*SUBU)(void);
	void (*SLT)(void);
	void (*SLTU)(void);
	void (*AND)(void);
	void (*OR)(void);
	void (*XOR)(void);
	void (*NOR)(void);

	void (*DADD)(void);
	void (*DADDU)(void);
	void (*DSUB)(void);
	void (*DSUBU)(void);

	// Multiply and divide instructions
	void (*MULT)(void);
	void (*MULTU)(void);
	void (*DIV)(void);
	void (*DIVU)(void);
	void (*MFHI)(void);
	void (*MTHI)(void);
	void (*MFLO)(void);
	void (*MTLO)(void);

	void (*DMULT)(void);
	void (*DMULTU)(void);
	void (*DDIV)(void);
	void (*DDIVU)(void);

	// Jump and branch instructions
	void (*J)(void);
	void (*J_OUT)(void);
	void (*J_IDLE)(void);
	void (*JAL)(void);
	void (*JAL_OUT)(void);
	void (*JAL_IDLE)(void);
	void (*JR)(void);
	void (*JALR)(void);
	void (*BEQ)(void);
	void (*BEQ_OUT)(void);
	void (*BEQ_IDLE)(void);
	void (*BNE)(void);
	void (*BNE_OUT)(void);
	void (*BNE_IDLE)(void);
	void (*BLEZ)(void);
	void (*BLEZ_OUT)(void);
	void (*BLEZ_IDLE)(void);
	void (*BGTZ)(void);
	void (*BGTZ_OUT)(void);
	void (*BGTZ_IDLE)(void);
	void (*BLTZ)(void);
	void (*BLTZ_OUT)(void);
	void (*BLTZ_IDLE)(void);
	void (*BGEZ)(void);
	void (*BGEZ_OUT)(void);
	void (*BGEZ_IDLE)(void);
	void (*BLTZAL)(void);
	void (*BLTZAL_OUT)(void);
	void (*BLTZAL_IDLE)(void);
	void (*BGEZAL)(void);
	void (*BGEZAL_OUT)(void);
	void (*BGEZAL_IDLE)(void);

	void (*BEQL)(void);
	void (*BEQL_OUT)(void);
	void (*BEQL_IDLE)(void);
	void (*BNEL)(void);
	void (*BNEL_OUT)(void);
	void (*BNEL_IDLE)(void);
	void (*BLEZL)(void);
	void (*BLEZL_OUT)(void);
	void (*BLEZL_IDLE)(void);
	void (*BGTZL)(void);
	void (*BGTZL_OUT)(void);
	void (*BGTZL_IDLE)(void);
	void (*BLTZL)(void);
	void (*BLTZL_OUT)(void);
	void (*BLTZL_IDLE)(void);
	void (*BGEZL)(void);
	void (*BGEZL_OUT)(void);
	void (*BGEZL_IDLE)(void);
	void (*BLTZALL)(void);
	void (*BLTZALL_OUT)(void);
	void (*BLTZALL_IDLE)(void);
	void (*BGEZALL)(void);
	void (*BGEZALL_OUT)(void);
	void (*BGEZALL_IDLE)(void);
	void (*BC1TL)(void);
	void (*BC1TL_OUT)(void);
	void (*BC1TL_IDLE)(void);
	void (*BC1FL)(void);
	void (*BC1FL_OUT)(void);
	void (*BC1FL_IDLE)(void);

	// Shift instructions
	void (*SLL)(void);
	void (*SRL)(void);
	void (*SRA)(void);
	void (*SLLV)(void);
	void (*SRLV)(void);
	void (*SRAV)(void);

	void (*DSLL)(void);
	void (*DSRL)(void);
	void (*DSRA)(void);
	void (*DSLLV)(void);
	void (*DSRLV)(void);
	void (*DSRAV)(void);
	void (*DSLL32)(void);
	void (*DSRL32)(void);
	void (*DSRA32)(void);

	// COP0 instructions
	void (*MTC0)(void);
	void (*MFC0)(void);

	void (*TLBR)(void);
	void (*TLBWI)(void);
	void (*TLBWR)(void);
	void (*TLBP)(void);
	void (*CACHE)(void);
	void (*ERET)(void);

	// COP1 instructions
	void (*LWC1)(void);
	void (*SWC1)(void);
	void (*MTC1)(void);
	void (*MFC1)(void);
	void (*CTC1)(void);
	void (*CFC1)(void);
	void (*BC1T)(void);
	void (*BC1T_OUT)(void);
	void (*BC1T_IDLE)(void);
	void (*BC1F)(void);
	void (*BC1F_OUT)(void);
	void (*BC1F_IDLE)(void);

	void (*DMFC1)(void);
	void (*DMTC1)(void);
	void (*LDC1)(void);
	void (*SDC1)(void);

	void (*CVT_S_D)(void);
	void (*CVT_S_W)(void);
	void (*CVT_S_L)(void);
	void (*CVT_D_S)(void);
	void (*CVT_D_W)(void);
	void (*CVT_D_L)(void);
	void (*CVT_W_S)(void);
	void (*CVT_W_D)(void);
	void (*CVT_L_S)(void);
	void (*CVT_L_D)(void);

	void (*ROUND_W_S)(void);
	void (*ROUND_W_D)(void);
	void (*ROUND_L_S)(void);
	void (*ROUND_L_D)(void);

	void (*TRUNC_W_S)(void);
	void (*TRUNC_W_D)(void);
	void (*TRUNC_L_S)(void);
	void (*TRUNC_L_D)(void);

	void (*CEIL_W_S)(void);
	void (*CEIL_W_D)(void);
	void (*CEIL_L_S)(void);	
	void (*CEIL_L_D)(void);	

	void (*FLOOR_W_S)(void);
	void (*FLOOR_W_D)(void);
	void (*FLOOR_L_S)(void);
	void (*FLOOR_L_D)(void);

	void (*ADD_S)(void);
	void (*ADD_D)(void);

	void (*SUB_S)(void);
	void (*SUB_D)(void);

	void (*MUL_S)(void);
	void (*MUL_D)(void);

	void (*DIV_S)(void);
	void (*DIV_D)(void);
	
	void (*ABS_S)(void);
	void (*ABS_D)(void);

	void (*MOV_S)(void);
	void (*MOV_D)(void);

	void (*NEG_S)(void);
	void (*NEG_D)(void);

	void (*SQRT_S)(void);
	void (*SQRT_D)(void);

	void (*C_F_S)(void);
	void (*C_F_D)(void);
	void (*C_UN_S)(void);
	void (*C_UN_D)(void);
	void (*C_EQ_S)(void);
	void (*C_EQ_D)(void);
	void (*C_UEQ_S)(void);
	void (*C_UEQ_D)(void);
	void (*C_OLT_S)(void);
	void (*C_OLT_D)(void);
	void (*C_ULT_S)(void);
	void (*C_ULT_D)(void);
	void (*C_OLE_S)(void);
	void (*C_OLE_D)(void);
	void (*C_ULE_S)(void);
	void (*C_ULE_D)(void);
	void (*C_SF_S)(void);
	void (*C_SF_D)(void);
	void (*C_NGLE_S)(void);
	void (*C_NGLE_D)(void);
	void (*C_SEQ_S)(void);
	void (*C_SEQ_D)(void);
	void (*C_NGL_S)(void);
	void (*C_NGL_D)(void);
	void (*C_LT_S)(void);
	void (*C_LT_D)(void);
	void (*C_NGE_S)(void);
	void (*C_NGE_D)(void);
	void (*C_LE_S)(void);
	void (*C_LE_D)(void);
	void (*C_NGT_S)(void);	
	void (*C_NGT_D)(void);	

	// Special instructions
	void (*SYSCALL)(void);

	// Exception instructions
	void (*TEQ)(void);

	// Emulator helper functions
	void (*NOP)(void);          // No operation (used to nullify R0 writes)
	void (*RESERVED)(void);     // Reserved instruction handler
	void (*NI)(void);	        // Not implemented instruction handler

	void (*FIN_BLOCK)(void);    // Handler for the end of a block
	void (*NOTCOMPILED)(void);  // Handler for not yet compiled code
	void (*NOTCOMPILED2)(void); // TODOXXX
} cpu_instruction_table;

extern const cpu_instruction_table cached_interpreter_table;
extern cpu_instruction_table current_instruction_table;
