/*****************************************************************************
 *
 *   arm7core.h
 *   Portable ARM7TDMI Core Emulator
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************

 This file contains everything related to the arm7 core itself, and is presumed
 to be cpu implementation non-specific, ie, applies to only the core.

 ******************************************************************************/

#ifndef __ARM7CORE_H__
#define __ARM7CORE_H__


/****************************************************************************************************
 *  INTERRUPT LINES/EXCEPTIONS
 ***************************************************************************************************/
enum
{
    ARM7_IRQ_LINE=0, ARM7_FIRQ_LINE,
    ARM7_ABORT_EXCEPTION, ARM7_ABORT_PREFETCH_EXCEPTION, ARM7_UNDEFINE_EXCEPTION,
    ARM7_NUM_LINES
};
// Really there's only 1 ABORT Line.. and cpu decides whether it's during data fetch or prefetch, but we let the user specify

/****************************************************************************************************
 *  ARM7 CORE REGISTERS
 ***************************************************************************************************/
enum
{
    ARM7_PC = 0,
    ARM7_R0, ARM7_R1, ARM7_R2, ARM7_R3, ARM7_R4, ARM7_R5, ARM7_R6, ARM7_R7,
    ARM7_R8, ARM7_R9, ARM7_R10, ARM7_R11, ARM7_R12, ARM7_R13, ARM7_R14, ARM7_R15,
    ARM7_FR8, ARM7_FR9, ARM7_FR10, ARM7_FR11, ARM7_FR12, ARM7_FR13, ARM7_FR14,
    ARM7_IR13, ARM7_IR14, ARM7_SR13, ARM7_SR14, ARM7_FSPSR, ARM7_ISPSR, ARM7_SSPSR,
    ARM7_CPSR, ARM7_AR13, ARM7_AR14, ARM7_ASPSR, ARM7_UR13, ARM7_UR14, ARM7_USPSR
};

#define ARM7CORE_REGS                   \
    UINT32 sArmRegister[kNumRegisters]; \
    UINT8 pendingIrq;                   \
    UINT8 pendingFiq;                   \
    UINT8 pendingAbtD;                  \
    UINT8 pendingAbtP;                  \
    UINT8 pendingUnd;                   \
    UINT8 pendingSwi;                   \
    int (*irq_callback)(int);


/****************************************************************************************************
 *  VARIOUS INTERNAL STRUCS/DEFINES/ETC..
 ***************************************************************************************************/
// Mode values come from bit 4-0 of CPSR, but we are ignoring bit 4 here, since bit 4 always = 1 for valid modes
enum
{
    eARM7_MODE_USER = 0x0,      // Bit: 4-0 = 10000
    eARM7_MODE_FIQ  = 0x1,      // Bit: 4-0 = 10001
    eARM7_MODE_IRQ  = 0x2,      // Bit: 4-0 = 10010
    eARM7_MODE_SVC  = 0x3,      // Bit: 4-0 = 10011
    eARM7_MODE_ABT  = 0x7,      // Bit: 4-0 = 10111
    eARM7_MODE_UND  = 0xb,      // Bit: 4-0 = 11011
    eARM7_MODE_SYS  = 0xf       // Bit: 4-0 = 11111
};

#define ARM7_NUM_MODES 0x10

/* There are 36 Unique - 32 bit processor registers */
/* Each mode has 17 registers (except user & system, which have 16) */
/* This is a list of each *unique* register */
enum
{
    /* All modes have the following */
    eR0 = 0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
    eR8, eR9, eR10, eR11, eR12,
    eR13, /* Stack Pointer */
    eR14, /* Link Register (holds return address) */
    eR15, /* Program Counter */
    eCPSR, /* Current Status Program Register */

    /* Fast Interrupt - Bank switched registers */
    eR8_FIQ, eR9_FIQ, eR10_FIQ, eR11_FIQ, eR12_FIQ, eR13_FIQ, eR14_FIQ, eSPSR_FIQ,

    /* IRQ - Bank switched registers */
    eR13_IRQ, eR14_IRQ, eSPSR_IRQ,

    /* Supervisor/Service Mode - Bank switched registers */
    eR13_SVC, eR14_SVC, eSPSR_SVC,

    /* Abort Mode - Bank switched registers */
    eR13_ABT, eR14_ABT, eSPSR_ABT,

    /* Undefined Mode - Bank switched registers */
    eR13_UND, eR14_UND, eSPSR_UND,

    kNumRegisters
};

static const int thumbCycles[256] =
{
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 1
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 2
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 3
    1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 4
    2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 5
    2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 6
    2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 7
    2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 8
    2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 9
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // a
    1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 4, 1, 1,  // b
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // c
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3,  // d
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // e
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2   // f
};

/* 17 processor registers are visible at any given time,
 * banked depending on processor mode.
 */

static const int sRegisterTable[ARM7_NUM_MODES][18] =
{
    { /* USR */
        eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
        eR8, eR9, eR10, eR11, eR12,
        eR13, eR14,
        eR15, eCPSR  // No SPSR in this mode
    },
    { /* FIQ */
        eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
        eR8_FIQ, eR9_FIQ, eR10_FIQ, eR11_FIQ, eR12_FIQ,
        eR13_FIQ, eR14_FIQ,
        eR15, eCPSR, eSPSR_FIQ
    },
    { /* IRQ */
        eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
        eR8, eR9, eR10, eR11, eR12,
        eR13_IRQ, eR14_IRQ,
        eR15, eCPSR, eSPSR_IRQ
    },
    { /* SVC */
        eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
        eR8, eR9, eR10, eR11, eR12,
        eR13_SVC, eR14_SVC,
        eR15, eCPSR, eSPSR_SVC
    },
    {0}, {0}, {0},        // values for modes 4,5,6 are not valid
    { /* ABT */
        eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
        eR8, eR9, eR10, eR11, eR12,
        eR13_ABT, eR14_ABT,
        eR15, eCPSR, eSPSR_ABT
    },
    {0}, {0}, {0},        // values for modes 8,9,a are not valid!
    { /* UND */
        eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
        eR8, eR9, eR10, eR11, eR12,
        eR13_UND, eR14_UND,
        eR15, eCPSR, eSPSR_UND
    },
    {0}, {0}, {0},        // values for modes c,d, e are not valid!
    { /* SYS */
        eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
        eR8, eR9, eR10, eR11, eR12,
        eR13, eR14,
        eR15, eCPSR  // No SPSR in this mode
    }
};

#define N_BIT   31
#define Z_BIT   30
#define C_BIT   29
#define V_BIT   28
#define I_BIT   7
#define F_BIT   6
#define T_BIT   5   // Thumb mode

#define N_MASK  ((UINT32)(1 << N_BIT)) /* Negative flag */
#define Z_MASK  ((UINT32)(1 << Z_BIT)) /* Zero flag */
#define C_MASK  ((UINT32)(1 << C_BIT)) /* Carry flag */
#define V_MASK  ((UINT32)(1 << V_BIT)) /* oVerflow flag */
#define I_MASK  ((UINT32)(1 << I_BIT)) /* Interrupt request disable */
#define F_MASK  ((UINT32)(1 << F_BIT)) /* Fast interrupt request disable */
#define T_MASK  ((UINT32)(1 << T_BIT)) /* Thumb Mode flag */

#define N_IS_SET(pc)    ((pc) & N_MASK)
#define Z_IS_SET(pc)    ((pc) & Z_MASK)
#define C_IS_SET(pc)    ((pc) & C_MASK)
#define V_IS_SET(pc)    ((pc) & V_MASK)
#define I_IS_SET(pc)    ((pc) & I_MASK)
#define F_IS_SET(pc)    ((pc) & F_MASK)
#define T_IS_SET(pc)    ((pc) & T_MASK)

#define N_IS_CLEAR(pc)  (!N_IS_SET(pc))
#define Z_IS_CLEAR(pc)  (!Z_IS_SET(pc))
#define C_IS_CLEAR(pc)  (!C_IS_SET(pc))
#define V_IS_CLEAR(pc)  (!V_IS_SET(pc))
#define I_IS_CLEAR(pc)  (!I_IS_SET(pc))
#define F_IS_CLEAR(pc)  (!F_IS_SET(pc))
#define T_IS_CLEAR(pc)  (!T_IS_SET(pc))

/* Deconstructing an instruction */
// todo: use these in all places (including dasm file)
#define INSN_COND           ((UINT32)0xf0000000u)
#define INSN_SDT_L          ((UINT32)0x00100000u)
#define INSN_SDT_W          ((UINT32)0x00200000u)
#define INSN_SDT_B          ((UINT32)0x00400000u)
#define INSN_SDT_U          ((UINT32)0x00800000u)
#define INSN_SDT_P          ((UINT32)0x01000000u)
#define INSN_BDT_L          ((UINT32)0x00100000u)
#define INSN_BDT_W          ((UINT32)0x00200000u)
#define INSN_BDT_S          ((UINT32)0x00400000u)
#define INSN_BDT_U          ((UINT32)0x00800000u)
#define INSN_BDT_P          ((UINT32)0x01000000u)
#define INSN_BDT_REGS       ((UINT32)0x0000ffffu)
#define INSN_SDT_IMM        ((UINT32)0x00000fffu)
#define INSN_MUL_A          ((UINT32)0x00200000u)
#define INSN_MUL_RM         ((UINT32)0x0000000fu)
#define INSN_MUL_RS         ((UINT32)0x00000f00u)
#define INSN_MUL_RN         ((UINT32)0x0000f000u)
#define INSN_MUL_RD         ((UINT32)0x000f0000u)
#define INSN_I              ((UINT32)0x02000000u)
#define INSN_OPCODE         ((UINT32)0x01e00000u)
#define INSN_S              ((UINT32)0x00100000u)
#define INSN_BL             ((UINT32)0x01000000u)
#define INSN_BRANCH         ((UINT32)0x00ffffffu)
#define INSN_SWI            ((UINT32)0x00ffffffu)
#define INSN_RN             ((UINT32)0x000f0000u)
#define INSN_RD             ((UINT32)0x0000f000u)
#define INSN_OP2            ((UINT32)0x00000fffu)
#define INSN_OP2_SHIFT      ((UINT32)0x00000f80u)
#define INSN_OP2_SHIFT_TYPE ((UINT32)0x00000070u)
#define INSN_OP2_RM         ((UINT32)0x0000000fu)
#define INSN_OP2_ROTATE     ((UINT32)0x00000f00u)
#define INSN_OP2_IMM        ((UINT32)0x000000ffu)
#define INSN_OP2_SHIFT_TYPE_SHIFT   4
#define INSN_OP2_SHIFT_SHIFT        7
#define INSN_OP2_ROTATE_SHIFT       8
#define INSN_MUL_RS_SHIFT           8
#define INSN_MUL_RN_SHIFT           12
#define INSN_MUL_RD_SHIFT           16
#define INSN_OPCODE_SHIFT           21
#define INSN_RN_SHIFT               16
#define INSN_RD_SHIFT               12
#define INSN_COND_SHIFT             28

#define THUMB_INSN_TYPE     ((UINT16)0xf000)
#define THUMB_COND_TYPE     ((UINT16)0x0f00)
#define THUMB_GROUP4_TYPE   ((UINT16)0x0c00)
#define THUMB_GROUP5_TYPE   ((UINT16)0x0e00)
#define THUMB_GROUP5_RM     ((UINT16)0x01c0)
#define THUMB_GROUP5_RN     ((UINT16)0x0038)
#define THUMB_GROUP5_RD     ((UINT16)0x0007)
#define THUMB_ADDSUB_RNIMM  ((UINT16)0x01c0)
#define THUMB_ADDSUB_RS     ((UINT16)0x0038)
#define THUMB_ADDSUB_RD     ((UINT16)0x0007)
#define THUMB_INSN_ADDSUB   ((UINT16)0x0800)
#define THUMB_INSN_CMP      ((UINT16)0x0800)
#define THUMB_INSN_SUB      ((UINT16)0x0800)
#define THUMB_INSN_IMM_RD   ((UINT16)0x0700)
#define THUMB_INSN_IMM_S    ((UINT16)0x0080)
#define THUMB_INSN_IMM      ((UINT16)0x00ff)
#define THUMB_ADDSUB_TYPE   ((UINT16)0x0600)
#define THUMB_HIREG_OP      ((UINT16)0x0300)
#define THUMB_HIREG_H       ((UINT16)0x00c0)
#define THUMB_HIREG_RS      ((UINT16)0x0038)
#define THUMB_HIREG_RD      ((UINT16)0x0007)
#define THUMB_STACKOP_TYPE  ((UINT16)0x0f00)
#define THUMB_STACKOP_L     ((UINT16)0x0800)
#define THUMB_STACKOP_RD    ((UINT16)0x0700)
#define THUMB_ALUOP_TYPE    ((UINT16)0x03c0)
#define THUMB_BLOP_LO       ((UINT16)0x0800)
#define THUMB_BLOP_OFFS     ((UINT16)0x07ff)
#define THUMB_SHIFT_R       ((UINT16)0x0800)
#define THUMB_SHIFT_AMT     ((UINT16)0x07c0)
#define THUMB_HALFOP_L      ((UINT16)0x0800)
#define THUMB_HALFOP_OFFS   ((UINT16)0x07c0)
#define THUMB_BRANCH_OFFS   ((UINT16)0x07ff)
#define THUMB_LSOP_L        ((UINT16)0x0800)
#define THUMB_LSOP_OFFS     ((UINT16)0x07c0)
#define THUMB_MULTLS        ((UINT16)0x0800)
#define THUMB_MULTLS_BASE   ((UINT16)0x0700)
#define THUMB_RELADDR_SP    ((UINT16)0x0800)
#define THUMB_RELADDR_RD    ((UINT16)0x0700)
#define THUMB_INSN_TYPE_SHIFT       12
#define THUMB_COND_TYPE_SHIFT       8
#define THUMB_GROUP4_TYPE_SHIFT     10
#define THUMB_GROUP5_TYPE_SHIFT     9
#define THUMB_ADDSUB_TYPE_SHIFT     9
#define THUMB_INSN_IMM_RD_SHIFT     8
#define THUMB_STACKOP_TYPE_SHIFT    8
#define THUMB_HIREG_OP_SHIFT        8
#define THUMB_STACKOP_RD_SHIFT      8
#define THUMB_MULTLS_BASE_SHIFT     8
#define THUMB_RELADDR_RD_SHIFT      8
#define THUMB_HIREG_H_SHIFT         6
#define THUMB_HIREG_RS_SHIFT        3
#define THUMB_ALUOP_TYPE_SHIFT      6
#define THUMB_SHIFT_AMT_SHIFT       6
#define THUMB_HALFOP_OFFS_SHIFT     6
#define THUMB_LSOP_OFFS_SHIFT       6
#define THUMB_GROUP5_RM_SHIFT       6
#define THUMB_GROUP5_RN_SHIFT       3
#define THUMB_GROUP5_RD_SHIFT       0
#define THUMB_ADDSUB_RNIMM_SHIFT    6
#define THUMB_ADDSUB_RS_SHIFT       3
#define THUMB_ADDSUB_RD_SHIFT       0

enum
{
    OPCODE_AND, /* 0000 */
    OPCODE_EOR, /* 0001 */
    OPCODE_SUB, /* 0010 */
    OPCODE_RSB, /* 0011 */
    OPCODE_ADD, /* 0100 */
    OPCODE_ADC, /* 0101 */
    OPCODE_SBC, /* 0110 */
    OPCODE_RSC, /* 0111 */
    OPCODE_TST, /* 1000 */
    OPCODE_TEQ, /* 1001 */
    OPCODE_CMP, /* 1010 */
    OPCODE_CMN, /* 1011 */
    OPCODE_ORR, /* 1100 */
    OPCODE_MOV, /* 1101 */
    OPCODE_BIC, /* 1110 */
    OPCODE_MVN  /* 1111 */
};

enum
{
    COND_EQ = 0,          /*  Z           equal                   */
    COND_NE,              /* ~Z           not equal               */
    COND_CS, COND_HS = 2, /*  C           unsigned higher or same */
    COND_CC, COND_LO = 3, /* ~C           unsigned lower          */
    COND_MI,              /*  N           negative                */
    COND_PL,              /* ~N           positive or zero        */
    COND_VS,              /*  V           overflow                */
    COND_VC,              /* ~V           no overflow             */
    COND_HI,              /*  C && ~Z     unsigned higher         */
    COND_LS,              /* ~C ||  Z     unsigned lower or same  */
    COND_GE,              /*  N == V      greater or equal        */
    COND_LT,              /*  N != V      less than               */
    COND_GT,              /* ~Z && N == V greater than            */
    COND_LE,              /*  Z || N != V less than or equal      */
    COND_AL,              /*  1           always                  */
    COND_NV               /*  0           never                   */
};

#define LSL(v, s) ((v) << (s))
#define LSR(v, s) ((v) >> (s))
#define ROL(v, s) (LSL((v), (s)) | (LSR((v), 32u - (s))))
#define ROR(v, s) (LSR((v), (s)) | (LSL((v), 32u - (s))))

/* Convenience Macros */
#define R15                     ARM7REG(eR15)
#define SPSR                    17                     // SPSR is always the 18th register in our 0 based array sRegisterTable[][18]
#define GET_CPSR                ARM7REG(eCPSR)
#define SET_CPSR(v)             (GET_CPSR = (v))
#define MODE_FLAG               0xF                    // Mode bits are 4:0 of CPSR, but we ignore bit 4.
#define GET_MODE                (GET_CPSR & MODE_FLAG)
#define SIGN_BIT                ((UINT32)(1 << 31))
#define SIGN_BITS_DIFFER(a, b)  (((a) ^ (b)) >> 31)
/* I really don't know why these were set to 16-bit, the thumb registers are still 32-bit ... */
#define THUMB_SIGN_BIT               ((UINT32)(1 << 31))
#define THUMB_SIGN_BITS_DIFFER(a, b) (((a)^(b)) >> 31)

/* At one point I thought these needed to be cpu implementation specific, but they don't.. */
#define GET_REGISTER(reg)       GetRegister(reg)
#define SET_REGISTER(reg, val)  SetRegister(reg, val)
#define ARM7_CHECKIRQ           arm7_check_irq_state()

extern void((*arm7_coproc_do_callback)(unsigned int, unsigned int));
extern unsigned int((*arm7_coproc_rt_r_callback)(unsigned int));
extern void((*arm7_coproc_rt_w_callback)(unsigned int, unsigned int));
extern void (*arm7_coproc_dt_r_callback)(UINT32 insn, UINT32* prn, UINT32 (*read32)(UINT32 addr));
extern void (*arm7_coproc_dt_w_callback)(UINT32 insn, UINT32* prn, void (*write32)(UINT32 addr, UINT32 data));

#endif /* __ARM7CORE_H__ */
