/* arm.c

    ARM 2/3/6 Emulation (26 bit address bus)

    Todo:
      Timing - Currently very approximated, nothing relies on proper timing so far.
      IRQ timing not yet correct (again, nothing is affected by this so far).

    Recent changes (2005):
      Fixed software interrupts
      Fixed various mode change bugs
      Added preliminary co-processor support.

    By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "state.h"
//#include "driver.h"
#include "burnint.h"
#include "arm_intf.h"

#define READ8(addr)		cpu_read8(addr)
#define WRITE8(addr,data)	cpu_write8(addr,data)
#define READ32(addr)		cpu_read32(addr)
#define WRITE32(addr,data)	cpu_write32(addr,data)

#define change_pc(x)	R15 = (R15&~ADDRESS_MASK)|((x)&ADDRESS_MASK)

#define ARM_DEBUG_CORE 0
#define ARM_DEBUG_COPRO 0

enum
{
	ARM32_PC=0,
	ARM32_R0, ARM32_R1, ARM32_R2, ARM32_R3, ARM32_R4, ARM32_R5, ARM32_R6, ARM32_R7,
	ARM32_R8, ARM32_R9, ARM32_R10, ARM32_R11, ARM32_R12, ARM32_R13, ARM32_R14, ARM32_R15,
	ARM32_FR8, ARM32_FR9, ARM32_FR10, ARM32_FR11, ARM32_FR12, ARM32_FR13, ARM32_FR14,
	ARM32_IR13, ARM32_IR14, ARM32_SR13, ARM32_SR14
};

enum
{
	eARM_MODE_USER	= 0x0,
	eARM_MODE_FIQ	= 0x1,
	eARM_MODE_IRQ	= 0x2,
	eARM_MODE_SVC	= 0x3,

	kNumModes
};

/* There are 27 32 bit processor registers */
enum
{
	eR0=0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
	eR8,eR9,eR10,eR11,eR12,
	eR13, /* Stack Pointer */
	eR14, /* Link Register (holds return address) */
	eR15, /* Program Counter */

	/* Fast Interrupt */
	eR8_FIQ,eR9_FIQ,eR10_FIQ,eR11_FIQ,eR12_FIQ,eR13_FIQ,eR14_FIQ,

	/* IRQ */
	eR13_IRQ,eR14_IRQ,

	/* Software Interrupt */
	eR13_SVC,eR14_SVC,

	kNumRegisters
};

/* 16 processor registers are visible at any given time,
 * banked depending on processor mode.
 */
static const int sRegisterTable[kNumModes][16] =
{
	{ /* USR */
		eR0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
		eR8,eR9,eR10,eR11,eR12,
		eR13,eR14,
		eR15
	},
	{ /* FIQ */
		eR0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
		eR8_FIQ,eR9_FIQ,eR10_FIQ,eR11_FIQ,eR12_FIQ,
		eR13_FIQ,eR14_FIQ,
		eR15
	},
	{ /* IRQ */
		eR0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
		eR8,eR9,eR10,eR11,eR12,
		eR13_IRQ,eR14_IRQ,
		eR15
	},
	{ /* SVC */
		eR0,eR1,eR2,eR3,eR4,eR5,eR6,eR7,
		eR8,eR9,eR10,eR11,eR12,
		eR13_SVC,eR14_SVC,
		eR15
	}
};

#define N_BIT	31
#define Z_BIT	30
#define C_BIT	29
#define V_BIT	28
#define I_BIT	27
#define F_BIT	26

#define N_MASK	((UINT32)(1<<N_BIT)) /* Negative flag */
#define Z_MASK	((UINT32)(1<<Z_BIT)) /* Zero flag */
#define C_MASK	((UINT32)(1<<C_BIT)) /* Carry flag */
#define V_MASK	((UINT32)(1<<V_BIT)) /* oVerflow flag */
#define I_MASK	((UINT32)(1<<I_BIT)) /* Interrupt request disable */
#define F_MASK	((UINT32)(1<<F_BIT)) /* Fast interrupt request disable */

#define N_IS_SET(pc)	((pc) & N_MASK)
#define Z_IS_SET(pc)	((pc) & Z_MASK)
#define C_IS_SET(pc)	((pc) & C_MASK)
#define V_IS_SET(pc)	((pc) & V_MASK)
#define I_IS_SET(pc)	((pc) & I_MASK)
#define F_IS_SET(pc)	((pc) & F_MASK)

#define N_IS_CLEAR(pc)	(!N_IS_SET(pc))
#define Z_IS_CLEAR(pc)	(!Z_IS_SET(pc))
#define C_IS_CLEAR(pc)	(!C_IS_SET(pc))
#define V_IS_CLEAR(pc)	(!V_IS_SET(pc))
#define I_IS_CLEAR(pc)	(!I_IS_SET(pc))
#define F_IS_CLEAR(pc)	(!F_IS_SET(pc))

#define PSR_MASK			((UINT32) 0xf0000000u)
#define IRQ_MASK			((UINT32) 0x0c000000u)
#define ADDRESS_MASK			((UINT32) 0x03fffffcu)
#define MODE_MASK			((UINT32) 0x00000003u)

#define R15				arm.sArmRegister[eR15]
#define MODE				(R15&0x03)
#define SIGN_BIT			((UINT32)(1<<31))
#define SIGN_BITS_DIFFER(a,b)		(((a)^(b)) >> 31)

/* Deconstructing an instruction */

#define INSN_COND			((UINT32) 0xf0000000u)
#define INSN_SDT_L			((UINT32) 0x00100000u)
#define INSN_SDT_W			((UINT32) 0x00200000u)
#define INSN_SDT_B			((UINT32) 0x00400000u)
#define INSN_SDT_U			((UINT32) 0x00800000u)
#define INSN_SDT_P			((UINT32) 0x01000000u)
#define INSN_BDT_L			((UINT32) 0x00100000u)
#define INSN_BDT_W			((UINT32) 0x00200000u)
#define INSN_BDT_S			((UINT32) 0x00400000u)
#define INSN_BDT_U			((UINT32) 0x00800000u)
#define INSN_BDT_P			((UINT32) 0x01000000u)
#define INSN_BDT_REGS			((UINT32) 0x0000ffffu)
#define INSN_SDT_IMM			((UINT32) 0x00000fffu)
#define INSN_MUL_A			((UINT32) 0x00200000u)
#define INSN_MUL_RM			((UINT32) 0x0000000fu)
#define INSN_MUL_RS			((UINT32) 0x00000f00u)
#define INSN_MUL_RN			((UINT32) 0x0000f000u)
#define INSN_MUL_RD			((UINT32) 0x000f0000u)
#define INSN_I				((UINT32) 0x02000000u)
#define INSN_OPCODE			((UINT32) 0x01e00000u)
#define INSN_S				((UINT32) 0x00100000u)
#define INSN_BL				((UINT32) 0x01000000u)
#define INSN_BRANCH			((UINT32) 0x00ffffffu)
#define INSN_SWI			((UINT32) 0x00ffffffu)
#define INSN_RN				((UINT32) 0x000f0000u)
#define INSN_RD				((UINT32) 0x0000f000u)
#define INSN_OP2			((UINT32) 0x00000fffu)
#define INSN_OP2_SHIFT			((UINT32) 0x00000f80u)
#define INSN_OP2_SHIFT_TYPE		((UINT32) 0x00000070u)
#define INSN_OP2_RM			((UINT32) 0x0000000fu)
#define INSN_OP2_ROTATE			((UINT32) 0x00000f00u)
#define INSN_OP2_IMM			((UINT32) 0x000000ffu)
#define INSN_OP2_SHIFT_TYPE_SHIFT	4
#define INSN_OP2_SHIFT_SHIFT		7
#define INSN_OP2_ROTATE_SHIFT		8
#define INSN_MUL_RS_SHIFT		8
#define INSN_MUL_RN_SHIFT		12
#define INSN_MUL_RD_SHIFT		16
#define INSN_OPCODE_SHIFT		21
#define INSN_RN_SHIFT			16
#define INSN_RD_SHIFT			12
#define INSN_COND_SHIFT			28

#define S_CYCLE 1
#define N_CYCLE 1
#define I_CYCLE 1

enum
{
	OPCODE_AND,	/* 0000 */
	OPCODE_EOR,	/* 0001 */
	OPCODE_SUB,	/* 0010 */
	OPCODE_RSB,	/* 0011 */
	OPCODE_ADD,	/* 0100 */
	OPCODE_ADC,	/* 0101 */
	OPCODE_SBC,	/* 0110 */
	OPCODE_RSC,	/* 0111 */
	OPCODE_TST,	/* 1000 */
	OPCODE_TEQ,	/* 1001 */
	OPCODE_CMP,	/* 1010 */
	OPCODE_CMN,	/* 1011 */
	OPCODE_ORR,	/* 1100 */
	OPCODE_MOV,	/* 1101 */
	OPCODE_BIC,	/* 1110 */
	OPCODE_MVN	/* 1111 */
};

enum
{
	COND_EQ = 0,	/* Z: equal */
	COND_NE,		/* ~Z: not equal */
	COND_CS, COND_HS = 2,	/* C: unsigned higher or same */
	COND_CC, COND_LO = 3,	/* ~C: unsigned lower */
	COND_MI,		/* N: negative */
	COND_PL,		/* ~N: positive or zero */
	COND_VS,		/* V: overflow */
	COND_VC,		/* ~V: no overflow */
	COND_HI,		/* C && ~Z: unsigned higher */
	COND_LS,		/* ~C || Z: unsigned lower or same */
	COND_GE,		/* N == V: greater or equal */
	COND_LT,		/* N != V: less than */
	COND_GT,		/* ~Z && (N == V): greater than */
	COND_LE,		/* Z || (N != V): less than or equal */
	COND_AL,		/* always */
	COND_NV			/* never */
};

#define LSL(v,s) ((v) << (s))
#define LSR(v,s) ((v) >> (s))
#define ROL(v,s) (LSL((v),(s)) | (LSR((v),32u - (s))))
#define ROR(v,s) (LSR((v),(s)) | (LSL((v),32u - (s))))

/* Private Data */

/* sArmRegister defines the CPU state */
typedef struct
{
	UINT32 sArmRegister[kNumRegisters];
	UINT32 coproRegister[16];
	UINT8 pendingIrq;
	UINT8 pendingFiq;

	UINT32 ArmTotalCycles;
	UINT32 ArmLeftCycles;
} ARM_REGS;

static ARM_REGS arm;

static int arm_icount;

/* Prototypes */
static void HandleALU( UINT32 insn);
static void HandleMul( UINT32 insn);
static void HandleBranch( UINT32 insn);
static void HandleMemSingle( UINT32 insn);
static void HandleMemBlock( UINT32 insn);
static void HandleCoPro( UINT32 insn);
static UINT32 decodeShift( UINT32 insn, UINT32 *pCarry);
static void arm_check_irq_state(void);

/***************************************************************************/

inline void cpu_write32( int addr, UINT32 data )
{
	/* Unaligned writes are treated as normal writes */
	Arm_program_write_dword_32le(addr&ADDRESS_MASK,data);
}

inline void cpu_write8( int addr, UINT8 data )
{
	Arm_program_write_byte_32le(addr,data);
}

inline UINT32 cpu_read32( int addr )
{
	UINT32 result = Arm_program_read_dword_32le(addr&ADDRESS_MASK);

	/* Unaligned reads rotate the word, they never combine words */
	if (addr&3) {
		if ((addr&3)==1)
			return ((result&0x000000ff)<<24)|((result&0xffffff00)>> 8);
		if ((addr&3)==2)
			return ((result&0x0000ffff)<<16)|((result&0xffff0000)>>16);
		if ((addr&3)==3)
			return ((result&0x00ffffff)<< 8)|((result&0xff000000)>>24);
	}

	return result;
}

inline UINT8 cpu_read8( int addr )
{
	return Arm_program_read_byte_32le(addr);
}

inline UINT32 GetRegister( int rIndex )
{
	return arm.sArmRegister[sRegisterTable[MODE][rIndex]];
}

inline void SetRegister( int rIndex, UINT32 value )
{
	arm.sArmRegister[sRegisterTable[MODE][rIndex]] = value;
	if (rIndex == eR15)
		change_pc(value & ADDRESS_MASK);
}

/***************************************************************************/

void ArmReset(void)
{
	memset(&arm, 0, sizeof(arm));

	/* start up in SVC mode with interrupts disabled. */
	R15 = eARM_MODE_SVC|I_MASK|F_MASK;
	change_pc(R15 & ADDRESS_MASK);
}

int ArmRun( int cycles )
{
	UINT32 pc;
	UINT32 insn;

	arm_icount = cycles;
	arm.ArmLeftCycles = cycles;

	do
	{
		/* load instruction */
		pc = R15;
		insn = Arm_program_opcode_dword_32le( pc & ADDRESS_MASK );

		switch (insn >> INSN_COND_SHIFT)
		{
		case COND_EQ:
			if (Z_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_NE:
			if (Z_IS_SET(pc)) goto L_Next;
			break;
		case COND_CS:
			if (C_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_CC:
			if (C_IS_SET(pc)) goto L_Next;
			break;
		case COND_MI:
			if (N_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_PL:
			if (N_IS_SET(pc)) goto L_Next;
			break;
		case COND_VS:
			if (V_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_VC:
			if (V_IS_SET(pc)) goto L_Next;
			break;
		case COND_HI:
			if (C_IS_CLEAR(pc) || Z_IS_SET(pc)) goto L_Next;
			break;
		case COND_LS:
			if (C_IS_SET(pc) && Z_IS_CLEAR(pc)) goto L_Next;
			break;
		case COND_GE:
			if (!(pc & N_MASK) != !(pc & V_MASK)) goto L_Next; /* Use x ^ (x >> ...) method */
			break;
		case COND_LT:
			if (!(pc & N_MASK) == !(pc & V_MASK)) goto L_Next;
			break;
		case COND_GT:
			if (Z_IS_SET(pc) || (!(pc & N_MASK) != !(pc & V_MASK))) goto L_Next;
			break;
		case COND_LE:
			if (Z_IS_CLEAR(pc) && (!(pc & N_MASK) == !(pc & V_MASK))) goto L_Next;
			break;
		case COND_NV:
			goto L_Next;
		}
		/* Condition satisfied, so decode the instruction */
		if ((insn & 0x0fc000f0u) == 0x00000090u)	/* Multiplication */
		{
			HandleMul(insn);
			R15 += 4;
		}
		else if (!(insn & 0x0c000000u)) /* Data processing */
		{
			HandleALU(insn);
		}
		else if ((insn & 0x0c000000u) == 0x04000000u) /* Single data access */
		{
			HandleMemSingle(insn);
			R15 += 4;
		}
		else if ((insn & 0x0e000000u) == 0x08000000u ) /* Block data access */
		{
			HandleMemBlock(insn);
			R15 += 4;
		}
		else if ((insn & 0x0e000000u) == 0x0a000000u)	/* Branch */
		{
			HandleBranch(insn);
		}
		else if ((insn & 0x0f000000u) == 0x0e000000u)	/* Coprocessor */
		{
			HandleCoPro(insn);
			R15 += 4;
		}
		else if ((insn & 0x0f000000u) == 0x0f000000u)	/* Software interrupt */
		{
			pc=R15+4;
			R15 = eARM_MODE_SVC;	/* Set SVC mode so PC is saved to correct R14 bank */
			SetRegister( 14, pc );	/* save PC */
			R15 = (pc&PSR_MASK)|(pc&IRQ_MASK)|0x8|eARM_MODE_SVC|I_MASK|(pc&MODE_MASK);
			change_pc(pc&ADDRESS_MASK);
			arm_icount -= 2 * S_CYCLE + N_CYCLE;
		}
		else /* Undefined */
		{
			L_Next:
			arm_icount -= S_CYCLE;
			R15 += 4;
		}

		arm_check_irq_state();

	} while( arm_icount > 0 );

	arm.ArmTotalCycles += (cycles - arm_icount);

	return cycles - arm_icount;
} /* arm_execute */

static void arm_check_irq_state(void)
{
	UINT32 pc = R15+4; /* save old pc (already incremented in pipeline) */;

	/* Exception priorities (from ARM6, not specifically ARM2/3):

        Reset
        Data abort
        FIRQ
        IRQ
        Prefetch abort
        Undefined instruction
    */

	if (arm.pendingFiq && (pc&F_MASK)==0) {
		R15 = eARM_MODE_FIQ;	/* Set FIQ mode so PC is saved to correct R14 bank */
		SetRegister( 14, pc );	/* save PC */
		R15 = (pc&PSR_MASK)|(pc&IRQ_MASK)|0x1c|eARM_MODE_FIQ|I_MASK|F_MASK; /* Mask both IRQ & FIRQ, set PC=0x1c */
		change_pc(R15 & ADDRESS_MASK);
		arm.pendingFiq=0;
		return;
	}

	if (arm.pendingIrq && (pc&I_MASK)==0) {
		R15 = eARM_MODE_IRQ;	/* Set IRQ mode so PC is saved to correct R14 bank */
		SetRegister( 14, pc );	/* save PC */
		R15 = (pc&PSR_MASK)|(pc&IRQ_MASK)|0x18|eARM_MODE_IRQ|I_MASK|(pc&F_MASK); /* Mask only IRQ, set PC=0x18 */
		change_pc(R15 & ADDRESS_MASK);
		arm.pendingIrq=0;
		return;
	}
}

void arm_set_irq_line(int irqline, int state)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("arm_set_irq_line called without init\n"));
#endif

	switch (irqline) {

	case ARM_IRQ_LINE: /* IRQ */
		if (state && (R15&0x3)!=eARM_MODE_IRQ) /* Don't allow nested IRQs */
			arm.pendingIrq=1;
		else
			arm.pendingIrq=0;
		break;

	case ARM_FIRQ_LINE: /* FIRQ */
		if (state && (R15&0x3)!=eARM_MODE_FIQ) /* Don't allow nested FIRQs */
			arm.pendingFiq=1;
		else
			arm.pendingFiq=0;
		break;
	}

	arm_check_irq_state();
}

/***************************************************************************/

static void HandleBranch(  UINT32 insn )
{
	UINT32 off = (insn & INSN_BRANCH) << 2;

	/* Save PC into LR if this is a branch with link */
	if (insn & INSN_BL)
	{
		SetRegister(14,R15 + 4);
	}

	/* Sign-extend the 24-bit offset in our calculations */
	if (off & 0x2000000u)
	{
		R15 -= ((~(off | 0xfc000000u)) + 1) - 8;
	}
	else
	{
		R15 += off + 8;
	}
	change_pc(R15 & ADDRESS_MASK);
	arm_icount -= 2 * S_CYCLE + N_CYCLE;
}

static void HandleMemSingle( UINT32 insn )
{
	UINT32 rn, rnv, off, rd;

	/* Fetch the offset */
	if (insn & INSN_I)
	{
		off = decodeShift(insn, NULL);
	}
	else
	{
		off = insn & INSN_SDT_IMM;
	}

	/* Calculate Rn, accounting for PC */
	rn = (insn & INSN_RN) >> INSN_RN_SHIFT;

	if (insn & INSN_SDT_P)
	{
		/* Pre-indexed addressing */
		if (insn & INSN_SDT_U)
		{
			rnv = (GetRegister(rn) + off);
		}
		else
		{
			rnv = (GetRegister(rn) - off);
		}

		if (insn & INSN_SDT_W)
		{
			SetRegister(rn,rnv);
		}
		else if (rn == eR15)
		{
			rnv = (rnv & ADDRESS_MASK) + 8;
		}
	}
	else
	{
		/* Post-indexed addressing */
		if (rn == eR15)
		{
			rnv = (R15 & ADDRESS_MASK) + 8;
		}
		else
		{
			rnv = GetRegister(rn);
		}
	}

	/* Do the transfer */
	rd = (insn & INSN_RD) >> INSN_RD_SHIFT;
	if (insn & INSN_SDT_L)
	{
		/* Load */
		arm_icount -= S_CYCLE + I_CYCLE + N_CYCLE;
		if (insn & INSN_SDT_B)
		{
			SetRegister(rd,(UINT32) READ8(rnv));
		}
		else
		{
			if (rd == eR15)
			{
				R15 = (READ32(rnv) & ADDRESS_MASK) | (R15 & PSR_MASK) | (R15 & MODE_MASK);
				change_pc(R15 & ADDRESS_MASK);

				/*
		                The docs are explicit in that the bottom bits should be masked off
		                when writing to R15 in this way, however World Cup Volleyball 95 has
		                an example of an unaligned jump (bottom bits = 2) where execution
		                should definitely continue from the rounded up address.
		
		                In other cases, 4 is subracted from R15 here to account for pipelining.
		                */
				if ((READ32(rnv)&3)==0)
					R15 -= 4;

				arm_icount -= S_CYCLE + N_CYCLE;
			}
			else
			{
				SetRegister(rd,READ32(rnv));
			}
		}
	}
	else
	{
		/* Store */
		arm_icount -= 2 * N_CYCLE;
		if (insn & INSN_SDT_B)
		{
			WRITE8(rnv, (UINT8) GetRegister(rd) & 0xffu);
		}
		else
		{
			WRITE32(rnv, rd == eR15 ? R15 + 8 : GetRegister(rd));
		}
	}

	/* Do post-indexing writeback */
	if (!(insn & INSN_SDT_P)/* && (insn&INSN_SDT_W)*/)
	{
		if (insn & INSN_SDT_U)
		{
			/* Writeback is applied in pipeline, before value is read from mem,
                so writeback is effectively ignored */
			if (rd==rn) {
				SetRegister(rn,GetRegister(rd));
			}
			else {
				SetRegister(rn,(rnv + off));
			}
		}
		else
		{
			/* Writeback is applied in pipeline, before value is read from mem,
                so writeback is effectively ignored */
			if (rd==rn) {
				SetRegister(rn,GetRegister(rd));
			}
			else {
				SetRegister(rn,(rnv - off));
			}
		}
	}
} /* HandleMemSingle */

#define IsNeg(i) ((i) >> 31)
#define IsPos(i) ((~(i)) >> 31)

/* Set NZCV flags for ADDS / SUBS */

#define HandleALUAddFlags(rd, rn, op2) \
  if (insn & INSN_S) \
    R15 = \
      ((R15 &~ (N_MASK | Z_MASK | V_MASK | C_MASK)) \
      | (((!SIGN_BITS_DIFFER(rn, op2)) && SIGN_BITS_DIFFER(rn, rd)) \
          << V_BIT) \
      | (((~(rn)) < (op2)) << C_BIT) \
      | HandleALUNZFlags(rd)) \
      + 4; \
  else R15 += 4;

#define HandleALUSubFlags(rd, rn, op2) \
  if (insn & INSN_S) \
    R15 = \
      ((R15 &~ (N_MASK | Z_MASK | V_MASK | C_MASK)) \
      | ((SIGN_BITS_DIFFER(rn, op2) && SIGN_BITS_DIFFER(rn, rd)) \
          << V_BIT) \
      | (((IsNeg(rn) & IsPos(op2)) | (IsNeg(rn) & IsPos(rd)) | (IsPos(op2) & IsPos(rd))) ? C_MASK : 0) \
      | HandleALUNZFlags(rd)) \
      + 4; \
  else R15 += 4;

/* Set NZC flags for logical operations. */

#define HandleALUNZFlags(rd) \
  (((rd) & SIGN_BIT) | ((!(rd)) << Z_BIT))

#define HandleALULogicalFlags(rd, sc) \
  if (insn & INSN_S) \
    R15 = ((R15 &~ (N_MASK | Z_MASK | C_MASK)) \
                     | HandleALUNZFlags(rd) \
                     | (((sc) != 0) << C_BIT)) + 4; \
  else R15 += 4;

static void HandleALU( UINT32 insn )
{
	UINT32 op2, sc=0, rd, rn, opcode;
	UINT32 by, rdn;

	opcode = (insn & INSN_OPCODE) >> INSN_OPCODE_SHIFT;
	arm_icount -= S_CYCLE;

	rd = 0;
	rn = 0;

	/* Construct Op2 */
	if (insn & INSN_I)
	{
		/* Immediate constant */
		by = (insn & INSN_OP2_ROTATE) >> INSN_OP2_ROTATE_SHIFT;
		if (by)
		{
			op2 = ROR(insn & INSN_OP2_IMM, by << 1);
			sc = op2 & SIGN_BIT;
		}
		else
		{
			op2 = insn & INSN_OP2;
			sc = R15 & C_MASK;
		}
	}
	else
	{
		op2 = decodeShift(insn, (insn & INSN_S) ? &sc : NULL);

        	if (!(insn & INSN_S))
			sc=0;
	}

	/* Calculate Rn to account for pipelining */
	if ((opcode & 0xd) != 0xd) /* No Rn in MOV */
	{
		if ((rn = (insn & INSN_RN) >> INSN_RN_SHIFT) == eR15)
		{
			/* Docs strongly suggest the mode bits should be included here, but it breaks Captain
            America, as it starts doing unaligned reads */
			rn=(R15+8)&ADDRESS_MASK;
		}
		else
		{
			rn = GetRegister(rn);
		}
	}

	/* Perform the operation */
	switch ((insn & INSN_OPCODE) >> INSN_OPCODE_SHIFT)
	{
	/* Arithmetic operations */
	case OPCODE_SBC:
		rd = (rn - op2 - (R15 & C_MASK ? 0 : 1));
		HandleALUSubFlags(rd, rn, op2);
		break;
	case OPCODE_CMP:
	case OPCODE_SUB:
		rd = (rn - op2);
		HandleALUSubFlags(rd, rn, op2);
		break;
	case OPCODE_RSC:
		rd = (op2 - rn - (R15 & C_MASK ? 0 : 1));
		HandleALUSubFlags(rd, op2, rn);
		break;
	case OPCODE_RSB:
		rd = (op2 - rn);
		HandleALUSubFlags(rd, op2, rn);
		break;
	case OPCODE_ADC:
		rd = (rn + op2 + ((R15 & C_MASK) >> C_BIT));
		HandleALUAddFlags(rd, rn, op2);
		break;
	case OPCODE_CMN:
	case OPCODE_ADD:
		rd = (rn + op2);
		HandleALUAddFlags(rd, rn, op2);
		break;

	/* Logical operations */
	case OPCODE_AND:
	case OPCODE_TST:
		rd = rn & op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_BIC:
		rd = rn &~ op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_TEQ:
	case OPCODE_EOR:
		rd = rn ^ op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_ORR:
		rd = rn | op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_MOV:
		rd = op2;
		HandleALULogicalFlags(rd, sc);
		break;
	case OPCODE_MVN:
		rd = (~op2);
		HandleALULogicalFlags(rd, sc);
		break;
	}

	/* Put the result in its register if not a test */
	rdn = (insn & INSN_RD) >> INSN_RD_SHIFT;
	if ((opcode & 0xc) != 0x8)
	{
		if (rdn == eR15 && !(insn & INSN_S))
		{
			/* Merge the old NZCV flags into the new PC value */
			R15 = (rd & ADDRESS_MASK) | (R15 & PSR_MASK) | (R15 & IRQ_MASK) | (R15&MODE_MASK);
			change_pc(R15 & ADDRESS_MASK);
			arm_icount -= S_CYCLE + N_CYCLE;
		}
		else
		{
			if (rdn==eR15)
			{
				/* S Flag is set - update PSR & mode if in non-user mode only */
				if ((R15&MODE_MASK)!=0)
				{
					SetRegister(rdn,rd);
				}
				else
				{
					SetRegister(rdn,(rd&ADDRESS_MASK) | (rd&PSR_MASK) | (R15&IRQ_MASK) | (R15&MODE_MASK));
				}
				arm_icount -= S_CYCLE + N_CYCLE;
			}
			else
			{
				SetRegister(rdn,rd);
			}
		}
	/* TST & TEQ can affect R15 (the condition code register) with the S bit set */
	} else if (rdn==eR15) {
		if (insn & INSN_S) {
		/* Dubious hack for 'TEQS R15, #$3', the docs suggest execution
                should continue two instructions later (because pipelined R15
                is read back as already being incremented), but it seems the
                hardware should execute the instruction in the delay slot.
                Simulate it by just setting the PC back to the previously
                skipped instruction.

                See Heavy Smash (Data East) at 0x1c4
                */
			if (insn==0xe33ff003)
				rd-=4;

			arm_icount -= S_CYCLE + N_CYCLE;
			if ((R15&MODE_MASK)!=0)
			{
				SetRegister(15, rd);
			}
			else
			{
				SetRegister(15, (rd&ADDRESS_MASK) | (rd&PSR_MASK) | (R15&IRQ_MASK) | (R15&MODE_MASK));
			}
		}
	}
}

static void HandleMul( UINT32 insn)
{
	UINT32 r;

	arm_icount -= S_CYCLE + I_CYCLE;
	/* should be:
            Range of Rs            Number of cycles

               &0 -- &1            1S + 1I
               &2 -- &7            1S + 2I
               &8 -- &1F           1S + 3I
              &20 -- &7F           1S + 4I
              &80 -- &1FF          1S + 5I
             &200 -- &7FF          1S + 6I
             &800 -- &1FFF         1S + 7I
            &2000 -- &7FFF         1S + 8I
            &8000 -- &1FFFF        1S + 9I
           &20000 -- &7FFFF        1S + 10I
           &80000 -- &1FFFFF       1S + 11I
          &200000 -- &7FFFFF       1S + 12I
          &800000 -- &1FFFFFF      1S + 13I
         &2000000 -- &7FFFFFF      1S + 14I
         &8000000 -- &1FFFFFFF     1S + 15I
        &20000000 -- &FFFFFFFF     1S + 16I
  */

	/* Do the basic multiply of Rm and Rs */
	r =	GetRegister( insn&INSN_MUL_RM ) *
	  	GetRegister( (insn&INSN_MUL_RS)>>INSN_MUL_RS_SHIFT );

	/* Add on Rn if this is a MLA */
	if (insn & INSN_MUL_A)
	{
		r += GetRegister((insn&INSN_MUL_RN)>>INSN_MUL_RN_SHIFT);
	}

	/* Write the result */
	SetRegister((insn&INSN_MUL_RD)>>INSN_MUL_RD_SHIFT,r);

	/* Set N and Z if asked */
	if( insn & INSN_S )
	{
		R15 = (R15 &~ (N_MASK | Z_MASK)) | HandleALUNZFlags(r);
	}
}

static int loadInc ( UINT32 pat, UINT32 rbv, UINT32 s)
{
	int i,result;

	result = 0;
	for( i=0; i<16; i++ )
	{
		if( (pat>>i)&1 )
		{
			if (i==15) {
				if (s) /* Pull full contents from stack */
					SetRegister( 15, READ32(rbv+=4) );
				else /* Pull only address, preserve mode & status flags */
					SetRegister( 15, (R15&PSR_MASK) | (R15&IRQ_MASK) | (R15&MODE_MASK) | ((READ32(rbv+=4))&ADDRESS_MASK) );
			} else
				SetRegister( i, READ32(rbv+=4) );

			result++;
		}
	}
	return result;
}

static int loadDec( UINT32 pat, UINT32 rbv, UINT32 s, UINT32* deferredR15, int* defer)
{
	int i,result;

	result = 0;
	for( i=15; i>=0; i-- )
	{
		if( (pat>>i)&1 )
		{
			if (i==15) {
				*defer=1;
				if (s) /* Pull full contents from stack */
					*deferredR15=READ32(rbv-=4);
				else /* Pull only address, preserve mode & status flags */
					*deferredR15=(R15&PSR_MASK) | (R15&IRQ_MASK) | (R15&MODE_MASK) | ((READ32(rbv-=4))&ADDRESS_MASK);
			}
			else
				SetRegister( i, READ32(rbv -=4) );
			result++;
		}
	}
	return result;
}

static int storeInc( UINT32 pat, UINT32 rbv)
{
	int i,result;

	result = 0;
	for( i=0; i<16; i++ )
	{
		if( (pat>>i)&1 )
		{
			WRITE32( rbv += 4, GetRegister(i) );
			result++;
		}
	}
	return result;
} /* storeInc */

static int storeDec( UINT32 pat, UINT32 rbv)
{
	int i,result;

	result = 0;
	for( i=15; i>=0; i-- )
	{
		if( (pat>>i)&1 )
		{
			WRITE32( rbv -= 4, GetRegister(i) );
			result++;
		}
	}
	return result;
} /* storeDec */

static void HandleMemBlock( UINT32 insn)
{
	UINT32 rb = (insn & INSN_RN) >> INSN_RN_SHIFT;
	UINT32 rbp = GetRegister(rb);
	int result;

	if (insn & INSN_BDT_L)
	{
		/* Loading */
		if (insn & INSN_BDT_U)
		{
			/* Incrementing */
			if (!(insn & INSN_BDT_P)) rbp = rbp + (- 4);

			result = loadInc( insn & 0xffff, rbp, insn&INSN_BDT_S );

			if (insn & 0x8000) {
				R15-=4;
				arm_icount -= S_CYCLE + N_CYCLE;
			}

			if (insn & INSN_BDT_W)
			{
				/* Arm docs notes: The base register can always be loaded without any problems.
		                However, don't specify writeback if the base register is being loaded -
		                you can't end up with both a written-back value and a loaded value in the base register!
		
		                However - Fighter's History does exactly that at 0x121e4 (LDMUW [R13], { R13-R15 })!
		
		                This emulator implementation skips applying writeback in this case, which is confirmed
		                correct for this situation, but that is not necessarily true for all ARM hardware
		                implementations (the results are officially undefined).
		                */
				if ((insn&(1<<rb))==0)
					SetRegister(rb,GetRegister(rb)+result*4);
			}
		}
		else
		{
			UINT32 deferredR15=0;
			int defer=0;

			/* Decrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp - (- 4);
			}

			result = loadDec( insn&0xffff, rbp, insn&INSN_BDT_S, &deferredR15, &defer );

			if (insn & INSN_BDT_W)
			{
				SetRegister(rb,GetRegister(rb)-result*4);
			}

			// If R15 is pulled from memory we defer setting it until after writeback
			// is performed, else we may writeback to the wrong context (ie, the new
			// context if the mode has changed as a result of the R15 read)
			if (defer)
				SetRegister(15, deferredR15);

			if (insn & 0x8000) {
				arm_icount -= S_CYCLE + N_CYCLE;
				R15-=4;
			}
		}
		arm_icount -= result * S_CYCLE + N_CYCLE + I_CYCLE;
	} /* Loading */
	else
	{
		/* Storing

           	 ARM docs notes: Storing a list of registers including the base register using writeback
           	 will write the value of the base register before writeback to memory only if the base
           	 register is the first in the list. Otherwise, the value which is used is not defined.

        	*/
		if (insn & (1<<eR15))
		{
			/* special case handling if writing to PC */
			R15 += 12;
		}
		if (insn & INSN_BDT_U)
		{
			/* Incrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp + (- 4);
			}
			result = storeInc( insn&0xffff, rbp );
			if( insn & INSN_BDT_W )
			{
				SetRegister(rb,GetRegister(rb)+result*4);
			}
		}
		else
		{
			/* Decrementing */
			if (!(insn & INSN_BDT_P))
			{
				rbp = rbp - (- 4);
			}
			result = storeDec( insn&0xffff, rbp );
			if( insn & INSN_BDT_W )
			{
				SetRegister(rb,GetRegister(rb)-result*4);
			}
		}
		if( insn & (1<<eR15) )
			R15 -= 12;

		arm_icount -= (result - 1) * S_CYCLE + 2 * N_CYCLE;
	}
} /* HandleMemBlock */



/* Decodes an Op2-style shifted-register form.  If @carry@ is non-zero the
 * shifter carry output will manifest itself as @*carry == 0@ for carry clear
 * and @*carry != 0@ for carry set.
 */
static UINT32 decodeShift( UINT32 insn, UINT32 *pCarry)
{
	UINT32 k	= (insn & INSN_OP2_SHIFT) >> INSN_OP2_SHIFT_SHIFT;
	UINT32 rm	= GetRegister( insn & INSN_OP2_RM );
	UINT32 t	= (insn & INSN_OP2_SHIFT_TYPE) >> INSN_OP2_SHIFT_TYPE_SHIFT;

	if ((insn & INSN_OP2_RM)==0xf) {
		/* If hardwired shift, then PC is 8 bytes ahead, else if register shift
        is used, then 12 bytes - TODO?? */
		rm+=8;
	}

	/* All shift types ending in 1 are Rk, not #k */
	if( t & 1 )
	{
		//see p35 for check on this
		k = GetRegister(k >> 1)&0x1f;
		arm_icount -= S_CYCLE;
		if( k == 0 ) /* Register shift by 0 is a no-op */
		{
			if (pCarry) *pCarry = R15 & C_MASK;
			return rm;
		}
	}
	/* Decode the shift type and perform the shift */
	switch (t >> 1)
	{
	case 0:						/* LSL */
		if (pCarry)
		{
			*pCarry = k ? (rm & (1 << (32 - k))) : (R15 & C_MASK);
		}
		return k ? LSL(rm, k) : rm;
		break;

	case 1:			       			/* LSR */
		if (k == 0 || k == 32)
		{
			if (pCarry) *pCarry = rm & SIGN_BIT;
			return 0;
		}
		else if (k > 32)
		{
			if (pCarry) *pCarry = 0;
			return 0;
		}
		else
		{
			if (pCarry) *pCarry = (rm & (1 << (k - 1)));
			return LSR(rm, k);
		}
		break;

	case 2:						/* ASR */
		if (k == 0 || k > 32)
			k = 32;
		if (pCarry) *pCarry = (rm & (1 << (k - 1)));
		if (k >= 32)
			return rm & SIGN_BIT ? 0xffffffffu : 0;
		else
		{
			if (rm & SIGN_BIT)
				return LSR(rm, k) | (0xffffffffu << (32 - k));
			else
				return LSR(rm, k);
		}
		break;

	case 3:						/* ROR and RRX */
		if (k)
		{
			while (k > 32) k -= 32;
			if (pCarry) *pCarry = rm & SIGN_BIT;
			return ROR(rm, k);
		}
		else
		{
			if (pCarry) *pCarry = (rm & 1);
			return LSR(rm, 1) | ((R15 & C_MASK) << 2);
		}
		break;
	}

	return 0;
} /* decodeShift */


static UINT32 BCDToDecimal(UINT32 value)
{
	UINT32	accumulator = 0;
	UINT32	multiplier = 1;
	int		i;

	for(i = 0; i < 8; i++)
	{
		accumulator += (value & 0xF) * multiplier;

		multiplier *= 10;
		value >>= 4;
	}

	return accumulator;
}

static UINT32 DecimalToBCD(UINT32 value)
{
	UINT32	accumulator = 0;
	UINT32	divisor = 10;
	int		i;

	for(i = 0; i < 8; i++)
	{
		UINT32	temp;

		temp = value % divisor;
		value -= temp;
		temp /= divisor / 10;

		accumulator += temp << (i * 4);

		divisor *= 10;
	}

	return accumulator;
}

static void HandleCoPro( UINT32 insn)
{
	UINT32 rn=(insn>>12)&0xf;
	UINT32 crn=(insn>>16)&0xf;

	arm_icount -= S_CYCLE;

	/* MRC - transfer copro register to main register */
	if( (insn&0x0f100010)==0x0e100010 )
	{
		SetRegister(rn, arm.coproRegister[crn]);
	}
	/* MCR - transfer main register to copro register */
	else if( (insn&0x0f100010)==0x0e000010 )
	{
		arm.coproRegister[crn]=GetRegister(rn);

		/* Data East 156 copro specific - trigger BCD operation */
		if (crn==2)
		{
			if (arm.coproRegister[crn]==0)
			{
				/* Unpack BCD */
				int v0=BCDToDecimal(arm.coproRegister[0]);
				int v1=BCDToDecimal(arm.coproRegister[1]);

				/* Repack vcd */
				arm.coproRegister[5]=DecimalToBCD(v0+v1);
			}
			else if (arm.coproRegister[crn]==1)
			{
				/* Unpack BCD */
				int v0=BCDToDecimal(arm.coproRegister[0]);
				int v1=BCDToDecimal(arm.coproRegister[1]);

				/* Repack vcd */
				arm.coproRegister[5]=DecimalToBCD(v0*v1);
			}
			else if (arm.coproRegister[crn]==3)
			{
				/* Unpack BCD */
				int v0=BCDToDecimal(arm.coproRegister[0]);
				int v1=BCDToDecimal(arm.coproRegister[1]);

				/* Repack vcd */
				arm.coproRegister[5]=DecimalToBCD(v0-v1);
			}
		}
	}
	/* CDP - perform copro operation */
	else if( (insn&0x0f000010)==0x0e000000 )
	{
		/* Data East 156 copro specific divider - result in reg 3/4 */
		if (arm.coproRegister[1])
		{
			arm.coproRegister[3]=arm.coproRegister[0] / arm.coproRegister[1];
			arm.coproRegister[4]=arm.coproRegister[0] % arm.coproRegister[1];
		}
		else
		{
			/* Unverified */
			arm.coproRegister[3]=0xffffffff;
			arm.coproRegister[4]=0xffffffff;
		}
	}
}

// burn some cycles
void ArmIdleCycles(int cycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmIdleCycles called without init\n"));
#endif

	if (arm_icount > cycles) {
		arm_icount -= cycles;
	} else {
		arm_icount = 0;
	}
}

// get the current position
unsigned int ArmGetPc()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmGetPc called without init\n"));
#endif

	return arm.sArmRegister[15]&ADDRESS_MASK;
}

// get the remaining cycles left to run
unsigned int ArmRemainingCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmRemainingCycles called without init\n"));
#endif

	return (arm.ArmLeftCycles - arm_icount);
}

// get the total of cycles run
int ArmGetTotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmGetTotalCycles called without init\n"));
#endif

	return arm.ArmTotalCycles + (arm.ArmLeftCycles - arm_icount);
}

// stop the current cpu slice
void ArmRunEnd()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmRunEnd called without init\n"));
#endif

	arm_icount = 0;
}

// start a new frame
void ArmNewFrame()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmNewFrame called without init\n"));
#endif

	arm.ArmTotalCycles = 0;
	arm_icount = 0;
	arm.ArmLeftCycles = 0;
}

int ArmScan(int nAction, int *)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmScan called without init\n"));
#endif

	struct BurnArea ba;

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = &arm;
		ba.nLen	  = sizeof(ARM_REGS);
		ba.szName = "ARM Registers";
		BurnAcb(&ba);

		SCAN_VAR(arm_icount);
	}

	return 0;
}
