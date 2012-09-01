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
 * PPCDisasm.cpp
 *
 * PowerPC 603e disassembler from the original Supermodel project.
 * Copyright 2003 Bart Trzynadlowski, Ville Linde, and Stefano Teso.
 *
 * When possible, invalid forms of instructions are checked for. To the best
 * of my knowledge, all appropriate load/store instructions are checked. I'm
 * not sure whether any other kinds of instructions need checking.
 */

#include <cstdio>
#include <cstring>
#ifdef STANDALONE
#include <cstdlib>
#endif
#include "Supermodel.h"

#define DISASM_VERSION  "1.0"


/******************************************************************************
 Instruction Descriptions
 
 The disassembler is primarily table-driven making it easily modifiable.
******************************************************************************/

/*
 * Masks
 *
 * These masks isolate fields in an instruction word.
 */
#define M_LI    0x03fffffc
#define M_AA    0x00000002
#define M_LK    0x00000001
#define M_BO    0x03e00000
#define M_BI    0x001f0000
#define M_BD    0x0000fffc
#define M_RT    0x03e00000
#define M_RA    0x001f0000
#define M_RB    0x0000f800
#define M_CRFD  0x03800000
#define M_L     0x00200000
#define M_TO    0x03e00000
#define M_D     0x0000ffff
#define M_SIMM  0x0000ffff
#define M_UIMM  0x0000ffff
#define M_NB    0x0000f800
#define M_SR    0x000f0000
#define M_SH    0x0000f800
#define M_CRFS  0x001c0000
#define M_IMM   0x0000f000
#define M_CRBD  0x03e00000
#define M_RC    0x00000001
#define M_CRBA  0x001f0000
#define M_CRBB  0x0000f800
#define M_SPR   0x001FF800
#define M_TBR   0x001FF800
#define M_CRM   0x000FF000
#define M_FM    0x01FE0000
#define M_OE    0x00000400
#define M_REGC  0x000007c0
#define M_MB    0x000007c0
#define M_ME    0x0000003e
#define M_XO    0x000007fe

/*
 * Field Defining Macros
 *
 * These macros generate instruction words with their associated fields filled
 * in with the passed value.
 */
#define D_OP(op)    ((op & 0x3f) << 26)
#define D_XO(xo)    ((xo & 0x3ff) << 1)
#define D_RT(r)     ((r & 0x1f) << (31 - 10))
#define D_RA(r)		((r & 0x1f) << (31 - 15))
#define D_UIMM(u)	(u & 0xffff)

/*
 * Macros to Get Field Values
 *
 * These macros return the values of fields in an opcode. They all return
 * unsigned values and do not perform any sign extensions.
 */
#define G_RT(op)    ((op & M_RT) >> (31 - 10))
#define G_RA(op)    ((op & M_RA) >> (31 - 15))
#define G_RB(op)    ((op & M_RB) >> (31 - 20))
#define G_SIMM(op)  (op & M_SIMM)
#define G_UIMM(op)  (op & M_UIMM)
#define G_LI(op)    ((op & M_LI) >> 2)
#define G_BO(op)    ((op & M_BO) >> (31 - 10))
#define G_BI(op)    ((op & M_BI) >> (31 - 15))
#define G_BD(op)    ((op & M_BD) >> 2)
#define G_CRFD(op)  ((op & M_CRFD) >> (31 - 8))
#define G_L(op)     ((op & M_L) >> (31 - 10))
#define G_CRBD(op)  ((op & M_CRBD) >> (31 - 10))
#define G_CRBA(op)  ((op & M_CRBA) >> (31 - 15))
#define G_CRBB(op)  ((op & M_CRBB) >> (31 - 20))
#define G_REGC(op)  ((op & M_REGC) >> (31 - 25))
#define G_D(op)     (op & M_D)
#define G_NB(op)    ((op & M_NB) >> (31 - 20))
#define G_CRFS(op)  ((op & M_CRFS) >> (31 - 13))
#define G_SPR(op)   ((op & M_SPR) >> (31 - 20))
#define G_SR(op)    ((op & M_SR) >> (31 - 15))
#define G_CRM(op)   ((op & M_CRM) >> (31 - 19))
#define G_FM(op)    ((op & M_FM) >> (31 - 14))
#define G_IMM(op)   ((op & M_IMM) >> (31 - 19))
#define G_SH(op)    ((op & M_SH) >> (31 - 20))
#define G_MB(op)    ((op & M_MB) >> (31 - 25))
#define G_ME(op)    ((op & M_ME) >> 1)
#define G_TO(op)    ((op & M_TO) >> (31 - 10))

/*
 * Operand Formats
 *
 * These convey information on what operand fields are present and how they
 * ought to be printed.
 *
 * I'm fairly certain all of these are used, but that is not guaranteed.
 */
enum
{
    F_NONE,         // <no operands>
    F_LI,           // LI*4+PC if AA=0 else LI*4
    F_BCx,          // BO, BI, target_addr  used only by BCx
    F_RT_RA_0_SIMM, // rT, rA|0, SIMM       rA|0 means if rA == 0, print 0
    F_ADDIS,        // rT, rA, SIMM (printed as unsigned)   only used by ADDIS
    F_RT_RA_SIMM,   // rT, rA, SIMM         
    F_RA_RT_UIMM,   // rA, rT, UIMM         
    F_CMP_SIMM,     // crfD, L, A, SIMM
    F_CMP_UIMM,     // crfD, L, A, UIMM
    F_RT_RA_0_RB,   // rT, rA|0, rB
    F_RT_RA_RB,     // rT, rA, rB
    F_RT_D_RA_0,    // rT, d(rA|0)
    F_RT_D_RA,      // rT, d(rA)
    F_RA_RT_RB,     // rA, rT, rB
    F_FRT_D_RA_0,   // frT, d(RA|0)
    F_FRT_D_RA,     // frT, d(RA)
    F_FRT_RA_0_RB,  // frT, rA|0, rB
    F_FRT_RA_RB,    // frT, rA, rB
    F_TWI,          // TO, rA, SIMM         only used by TWI instruction
    F_CMP,          // crfD, L, rA, rB
    F_RA_RT,        // rA, rT
    F_RA_0_RB,      // rA|0, rB
    F_FRT_FRB,      // frT, frB
    F_FCMP,         // crfD, frA, frB
    F_CRFD_CRFS,    // crfD, crfS
    F_MCRXR,        // crfD                 only used by MCRXR
    F_RT,           // rT
    F_MFSR,         // rT, SR               only used by MFSR
    F_MTSR,         // SR, rT               only used by MTSR
    F_MFFSx,        // frT                  only used by MFFSx
    F_FCRBD,        // crbD                 FPSCR[crbD]
    F_MTFSFIx,      // crfD, IMM            only used by MTFSFIx
    F_RB,           // rB
    F_TW,           // TO, rA, rB           only used by TW
    F_RT_RA_0_NB,   // rT, rA|0, NB         print 32 if NB == 0
    F_SRAWIx,       // rA, rT, SH           only used by SRAWIx
    F_BO_BI,        // BO, BI
    F_CRBD_CRBA_CRBB,   // crbD, crbA, crbB
    F_RT_SPR,       // rT, SPR              and TBR
    F_MTSPR,        // SPR, rT              only used by MTSPR
    F_MTCRF,        // CRM, rT              only used by MTCRF
    F_MTFSFx,       // FM, frB              only used by MTFSFx
    F_RT_RA,        // rT, rA
    F_FRT_FRA_FRC_FRB,  // frT, frA, frC, frB
    F_FRT_FRA_FRB,  // frT, frA, frB
    F_FRT_FRA_FRC,  // frT, frA, frC
    F_RA_RT_SH_MB_ME,   // rA, rT, SH, MB, ME
    F_RLWNMx,       // rT, rA, rB, MB, ME   only used by RLWNMx
    F_RT_RB,        // rT, rB
};

/*
 * Flags
 */
#define FL_OE           (1 << 0)    // if there is an OE field
#define FL_RC           (1 << 1)    // if there is an RC field
#define FL_LK           (1 << 2)    // if there is an LK field
#define FL_AA           (1 << 3)    // if there is an AA field
#define FL_CHECK_RA_RT  (1 << 4)    // assert rA!=0 and rA!=rT
#define FL_CHECK_RA     (1 << 5)    // assert rA!=0
#define FL_CHECK_LSWI   (1 << 6)    // specific check for LSWI validity
#define FL_CHECK_LSWX   (1 << 7)    // specific check for LSWX validity


/*
 * Instruction Descriptor
 *
 * Describes the layout of an instruction.
 */
typedef struct
{
    char    	*mnem;  // mnemonic
    UINT32  	match;  // bit pattern of instruction after it has been masked
    UINT32  	mask;   // mask of variable fields (AND with ~mask to compare w/
                    	// bit pattern to determine a match)
    int     	format; // operand format
    unsigned	flags;  // flags
} IDESCR;

/*
 * Instruction Table
 *
 * Table of instruction descriptors which allows the disassembler to decode
 * and print instructions.
 */
static const IDESCR itab[] =
{
    { "add",    D_OP(31)|D_XO(266), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "addc",   D_OP(31)|D_XO(10),  M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "adde",   D_OP(31)|D_XO(138), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "addi",   D_OP(14),           M_RT|M_RA|M_SIMM,           F_RT_RA_0_SIMM, 0           },
    { "addic",  D_OP(12),           M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0           },
    { "addic.", D_OP(13),           M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0           },
    { "addis",  D_OP(15),           M_RT|M_RA|M_SIMM,           F_ADDIS,        0           },
    { "addme",  D_OP(31)|D_XO(234), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
    { "addze",  D_OP(31)|D_XO(202), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
    { "and",    D_OP(31)|D_XO(28),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "andc",   D_OP(31)|D_XO(60),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "andi.",  D_OP(28),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
    { "andis.", D_OP(29),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
    { "b",      D_OP(18),           M_LI|M_AA|M_LK,             F_LI,           FL_AA|FL_LK },
    { "bc",     D_OP(16),           M_BO|M_BI|M_BD|M_AA|M_LK,   F_BCx,          FL_AA|FL_LK },
    { "bcctr",  D_OP(19)|D_XO(528), M_BO|M_BI|M_LK,             F_BO_BI,        FL_LK       },
    { "bclr",   D_OP(19)|D_XO(16),  M_BO|M_BI|M_LK,             F_BO_BI,        FL_LK       },
    { "cmp",    D_OP(31)|D_XO(0),   M_CRFD|M_L|M_RA|M_RB,       F_CMP,          0           },
    { "cmpi",   D_OP(11),           M_CRFD|M_L|M_RA|M_SIMM,     F_CMP_SIMM,     0           },
    { "cmpl",   D_OP(31)|D_XO(32),  M_CRFD|M_L|M_RA|M_RB,       F_CMP,          0           },
    { "cmpli",  D_OP(10),           M_CRFD|M_L|M_RA|M_UIMM,     F_CMP_UIMM,     0           },
    { "cntlzw", D_OP(31)|D_XO(26),  M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC       },
    { "crand",  D_OP(19)|D_XO(257), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
    { "crandc", D_OP(19)|D_XO(129), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
    { "creqv",  D_OP(19)|D_XO(289), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
    { "crnand", D_OP(19)|D_XO(225), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
    { "crnor",  D_OP(19)|D_XO(33),  M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
    { "cror",   D_OP(19)|D_XO(449), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
    { "crorc",  D_OP(19)|D_XO(417), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
    { "crxor",  D_OP(19)|D_XO(193), M_CRBD|M_CRBA|M_CRBB,       F_CRBD_CRBA_CRBB,   0       },
    { "dcba",   D_OP(31)|D_XO(758), M_RA|M_RB,                  F_RA_0_RB,      0           },
    { "dcbf",   D_OP(31)|D_XO(86),  M_RA|M_RB,                  F_RA_0_RB,      0           },
    { "dcbi",   D_OP(31)|D_XO(470), M_RA|M_RB,                  F_RA_0_RB,      0           },
    { "dcbst",  D_OP(31)|D_XO(54),  M_RA|M_RB,                  F_RA_0_RB,      0           },
    { "dcbt",   D_OP(31)|D_XO(278), M_RA|M_RB,                  F_RA_0_RB,      0           },
    { "dcbtst", D_OP(31)|D_XO(246), M_RA|M_RB,                  F_RA_0_RB,      0           },
    { "dcbz",   D_OP(31)|D_XO(1014),M_RA|M_RB,                  F_RA_0_RB,      0           },
    { "divw",   D_OP(31)|D_XO(491), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "divwu",  D_OP(31)|D_XO(459), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "eciwx",  D_OP(31)|D_XO(310), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "ecowx",  D_OP(31)|D_XO(438), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "eieio",  D_OP(31)|D_XO(854), 0,                          F_NONE,         0           },
    { "eqv",    D_OP(31)|D_XO(284), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "extsb",  D_OP(31)|D_XO(954), M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC       },
    { "extsh",  D_OP(31)|D_XO(922), M_RT|M_RA|M_RC,             F_RA_RT,        FL_RC       },
    { "fabs",   D_OP(63)|D_XO(264), M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fadd",   D_OP(63)|D_XO(21),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
    { "fadds",  D_OP(59)|D_XO(21),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
    { "fcmpo",  D_OP(63)|D_XO(32),  M_CRFD|M_RA|M_RB,           F_FCMP,         0           },
    { "fcmpu",  D_OP(63)|D_XO(0),   M_CRFD|M_RA|M_RB,           F_FCMP,         0           },
    { "fctiw",  D_OP(63)|D_XO(14),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fctiwz", D_OP(63)|D_XO(15),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fdiv",   D_OP(63)|D_XO(18),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
    { "fdivs",  D_OP(59)|D_XO(18),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
    { "fmadd",  D_OP(63)|D_XO(29),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fmadds", D_OP(59)|D_XO(29),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fmr",    D_OP(63)|D_XO(72),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fmsub",  D_OP(63)|D_XO(28),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fmsubs", D_OP(59)|D_XO(28),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fmul",   D_OP(63)|D_XO(25),  M_RT|M_RA|M_REGC|M_RC,      F_FRT_FRA_FRC,  FL_RC       },
    { "fmuls",  D_OP(59)|D_XO(25),  M_RT|M_RA|M_REGC|M_RC,      F_FRT_FRA_FRC,  FL_RC       },
    { "fnabs",  D_OP(63)|D_XO(136), M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fneg",   D_OP(63)|D_XO(40),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fnmadd", D_OP(63)|D_XO(31),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fnmadds",D_OP(59)|D_XO(31),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fnmsub", D_OP(63)|D_XO(30),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fnmsubs",D_OP(59)|D_XO(30),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fres",   D_OP(59)|D_XO(24),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "frsp",   D_OP(63)|D_XO(12),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "frsqrte",D_OP(63)|D_XO(26),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fsel",   D_OP(63)|D_XO(23),  M_RT|M_RA|M_RB|M_REGC|M_RC, F_FRT_FRA_FRC_FRB,  FL_RC   },
    { "fsqrt",  D_OP(63)|D_XO(22),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fsqrts", D_OP(59)|D_XO(22),  M_RT|M_RB|M_RC,             F_FRT_FRB,      FL_RC       },
    { "fsub",   D_OP(63)|D_XO(20),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
    { "fsubs",  D_OP(59)|D_XO(20),  M_RT|M_RA|M_RB|M_RC,        F_FRT_FRA_FRB,  FL_RC       },
    { "icbi",   D_OP(31)|D_XO(982), M_RA|M_RB,                  F_RA_0_RB,      0           },
    { "isync",  D_OP(19)|D_XO(150), 0,                          F_NONE,         0           },
    { "lbz",    D_OP(34),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "lbzu",   D_OP(35),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT },
    { "lbzux",  D_OP(31)|D_XO(119), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT },
    { "lbzx",   D_OP(31)|D_XO(87),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "lfd",    D_OP(50),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0           },
    { "lfdu",   D_OP(51),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA },
    { "lfdux",  D_OP(31)|D_XO(631), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA },
    { "lfdx",   D_OP(31)|D_XO(599), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
    { "lfs",    D_OP(48),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0           },
    { "lfsu",   D_OP(49),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA },
    { "lfsux",  D_OP(31)|D_XO(567), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA },
    { "lfsx",   D_OP(31)|D_XO(535), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
    { "lha",    D_OP(42),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "lhau",   D_OP(43),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT },
    { "lhaux",  D_OP(31)|D_XO(375), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT },
    { "lhax",   D_OP(31)|D_XO(343), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "lhbrx",  D_OP(31)|D_XO(790), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "lhz",    D_OP(40),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "lhzu",   D_OP(41),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT },
    { "lhzux",  D_OP(31)|D_XO(311), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT },
    { "lhzx",   D_OP(31)|D_XO(279), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "lmw",    D_OP(46),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "lswi",   D_OP(31)|D_XO(597), M_RT|M_RA|M_NB,             F_RT_RA_0_NB,   FL_CHECK_LSWI },
    { "lswx",   D_OP(31)|D_XO(533), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   FL_CHECK_LSWX },
    { "lwarx",  D_OP(31)|D_XO(20),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "lwbrx",  D_OP(31)|D_XO(534), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "lwz",    D_OP(32),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "lwzu",   D_OP(33),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA_RT },
    { "lwzux",  D_OP(31)|D_XO(55),  M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA_RT },
    { "lwzx",   D_OP(31)|D_XO(23),  M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "mcrf",   D_OP(19)|D_XO(0),   M_CRFD|M_CRFS,              F_CRFD_CRFS,    0           },
    { "mcrfs",  D_OP(63)|D_XO(64),  M_CRFD|M_CRFS,              F_CRFD_CRFS,    0           },
    { "mcrxr",  D_OP(31)|D_XO(512), M_CRFD,                     F_MCRXR,        0           },
    { "mfcr",   D_OP(31)|D_XO(19),  M_RT,                       F_RT,           0           },
    { "mffs",   D_OP(63)|D_XO(583), M_RT|M_RC,                  F_MFFSx,        FL_RC       },
    { "mfmsr",  D_OP(31)|D_XO(83),  M_RT,                       F_RT,           0           },
    { "mfspr",  D_OP(31)|D_XO(339), M_RT|M_SPR,                 F_RT_SPR,       0           },
    { "mfsr",   D_OP(31)|D_XO(595), M_RT|M_SR,                  F_MFSR,         0           },
    { "mfsrin", D_OP(31)|D_XO(659), M_RT|M_RB,                  F_RT_RB,        0           },
    { "mftb",   D_OP(31)|D_XO(371), M_RT|M_TBR,                 F_RT_SPR,       0           },
    { "mtcrf",  D_OP(31)|D_XO(144), M_RT|M_CRM,                 F_MTCRF,        0           },
    { "mtfsb0", D_OP(63)|D_XO(70),  M_CRBD|M_RC,                F_FCRBD,        FL_RC       },
    { "mtfsb1", D_OP(63)|D_XO(38),  M_CRBD|M_RC,                F_FCRBD,        FL_RC       },
    { "mtfsf",  D_OP(63)|D_XO(711), M_FM|M_RB|M_RC,             F_MTFSFx,       FL_RC       },
    { "mtfsfi", D_OP(63)|D_XO(134), M_CRFD|M_IMM|M_RC,          F_MTFSFIx,      FL_RC       },
    { "mtmsr",  D_OP(31)|D_XO(146), M_RT,                       F_RT,           0           },
    { "mtspr",  D_OP(31)|D_XO(467), M_RT|M_SPR,                 F_MTSPR,        0           },
    { "mtsr",   D_OP(31)|D_XO(210), M_RT|M_SR,                  F_MTSR,         0           },
    { "mtsrin", D_OP(31)|D_XO(242), M_RT|M_RB,                  F_RT_RB,        0           },
    { "mulhw",  D_OP(31)|D_XO(75),  M_RT|M_RA|M_RB|M_RC,        F_RT_RA_RB,     FL_RC       },
    { "mulhwu", D_OP(31)|D_XO(11),  M_RT|M_RA|M_RB|M_RC,        F_RT_RA_RB,     FL_RC       },
    { "mulli",  D_OP(7),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0           },
    { "mullw",  D_OP(31)|D_XO(235), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "nand",   D_OP(31)|D_XO(476), M_RA|M_RT|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "neg",    D_OP(31)|D_XO(104), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
    { "nor",    D_OP(31)|D_XO(124), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "or",     D_OP(31)|D_XO(444), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "orc",    D_OP(31)|D_XO(412), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "ori",    D_OP(24),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
    { "oris",   D_OP(25),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
    { "rfi",    D_OP(19)|D_XO(50),  0,                          F_NONE,         0           },
    { "rlwimi", D_OP(20),           M_RT|M_RA|M_SH|M_MB|M_ME|M_RC,  F_RA_RT_SH_MB_ME,   FL_RC   },
    { "rlwinm", D_OP(21),           M_RT|M_RA|M_SH|M_MB|M_ME|M_RC,  F_RA_RT_SH_MB_ME,   FL_RC   },
    { "rlwnm",  D_OP(23),           M_RT|M_RA|M_RB|M_MB|M_ME|M_RC,  F_RLWNMx,   FL_RC       },
    { "sc",     D_OP(17)|2,         0,                          F_NONE,         0           },
    { "slw",    D_OP(31)|D_XO(24),  M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "sraw",   D_OP(31)|D_XO(792), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "srawi",  D_OP(31)|D_XO(824), M_RT|M_RA|M_SH|M_RC,        F_SRAWIx,       FL_RC       },
    { "srw",    D_OP(31)|D_XO(536), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "stb",    D_OP(38),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "stbu",   D_OP(39),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA },
    { "stbux",  D_OP(31)|D_XO(247), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA },
    { "stbx",   D_OP(31)|D_XO(215), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "stfd",   D_OP(54),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0           },
    { "stfdu",  D_OP(55),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA },
    { "stfdux", D_OP(31)|D_XO(759), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA },
    { "stfdx",  D_OP(31)|D_XO(727), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
    { "stfiwx", D_OP(31)|D_XO(983), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
    { "stfs",   D_OP(52),           M_RT|M_RA|M_D,              F_FRT_D_RA_0,   0           },
    { "stfsu",  D_OP(53),           M_RT|M_RA|M_D,              F_FRT_D_RA,     FL_CHECK_RA },
    { "stfsux", D_OP(31)|D_XO(695), M_RT|M_RA|M_RB,             F_FRT_RA_RB,    FL_CHECK_RA },
    { "stfsx",  D_OP(31)|D_XO(663), M_RT|M_RA|M_RB,             F_FRT_RA_0_RB,  0           },
    { "sth",    D_OP(44),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "sthbrx", D_OP(31)|D_XO(918), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "sthu",   D_OP(45),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA },
    { "sthux",  D_OP(31)|D_XO(439), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA },
    { "sthx",   D_OP(31)|D_XO(407), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "stmw",   D_OP(47),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "stswi",  D_OP(31)|D_XO(725), M_RT|M_RA|M_NB,             F_RT_RA_0_NB,   0           },
    { "stswx",  D_OP(31)|D_XO(661), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "stw",    D_OP(36),           M_RT|M_RA|M_D,              F_RT_D_RA_0,    0           },
    { "stwbrx", D_OP(31)|D_XO(662), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "stwcx.", D_OP(31)|D_XO(150)|1,   M_RT|M_RA|M_RB,         F_RT_RA_0_RB,   0           },
    { "stwu",   D_OP(37),           M_RT|M_RA|M_D,              F_RT_D_RA,      FL_CHECK_RA },
    { "stwux",  D_OP(31)|D_XO(183), M_RT|M_RA|M_RB,             F_RT_RA_RB,     FL_CHECK_RA },
    { "stwx",   D_OP(31)|D_XO(151), M_RT|M_RA|M_RB,             F_RT_RA_0_RB,   0           },
    { "subf",   D_OP(31)|D_XO(40),  M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "subfc",  D_OP(31)|D_XO(8),   M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "subfe",  D_OP(31)|D_XO(136), M_RT|M_RA|M_RB|M_OE|M_RC,   F_RT_RA_RB,     FL_OE|FL_RC },
    { "subfic", D_OP(8),            M_RT|M_RA|M_SIMM,           F_RT_RA_SIMM,   0           },
    { "subfme", D_OP(31)|D_XO(232), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
    { "subfze", D_OP(31)|D_XO(200), M_RT|M_RA|M_OE|M_RC,        F_RT_RA,        FL_OE|FL_RC },
    { "sync",   D_OP(31)|D_XO(598), 0,                          F_NONE,         0           },
    { "tlbia",  D_OP(31)|D_XO(370), 0,                          F_NONE,         0           },
    { "tlbie",  D_OP(31)|D_XO(306), M_RB,                       F_RB,           0           },
    { "tlbsync",D_OP(31)|D_XO(566), 0,                          F_NONE,         0           },
    { "tw",     D_OP(31)|D_XO(4),   M_TO|M_RA|M_RB,             F_TW,           0           },
    { "twi",    D_OP(3),            M_TO|M_RA|M_SIMM,           F_TWI,          0           },
    { "xor",    D_OP(31)|D_XO(316), M_RT|M_RA|M_RB|M_RC,        F_RA_RT_RB,     FL_RC       },
    { "xori",   D_OP(26),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },
    { "xoris",  D_OP(27),           M_RT|M_RA|M_UIMM,           F_RA_RT_UIMM,   0           },

    /*
     * PowerPC 603e/EC603e-specific instructions
     */

    { "tlbld",  D_OP(31)|D_XO(978), M_RB,                       F_RB,           0           },
    { "tlbli",  D_OP(31)|D_XO(1010),M_RB,                       F_RB,           0           }
};


/******************************************************************************
 Instruction Decoding and Disassembly Functions
******************************************************************************/

/*
 * CR Bits
 *
 * Use an index of BI&3 into this table to obtain the CR field bit name.
 */
static char *crbit[] = { "lt", "gt", "eq", "so" };

/*
 * SPR():
 *
 * Decode the SPR (or TBR) field and append the register name to dest. If
 * no name is associated with the field value, the value itself is printed.
 */
static void SPR(char *dest, unsigned spr_field)
{
    unsigned	spr;

    /*
     * Construct the SPR number -- SPR field is 2 5-bit fields
     */

    spr = (spr_field >> 5) & 0x1f;
    spr |= (spr_field & 0x1f) << 5;

    /*
     * Append the SPR name to the destination string using strcat()
     */

    switch (spr)
    {
    case 1:     strcat(dest, "xer");    break;
    case 8:     strcat(dest, "lr");     break;
    case 9:     strcat(dest, "ctr");    break;
    case 18:    strcat(dest, "dsisr");  break;
    case 19:    strcat(dest, "dar");    break;
    case 22:    strcat(dest, "dec");    break;
    case 25:    strcat(dest, "sdr1");   break;
    case 26:    strcat(dest, "srr0");   break;
    case 27:    strcat(dest, "srr1");   break;
    case 272:   strcat(dest, "sprg0");  break;
    case 273:   strcat(dest, "sprg1");  break;
    case 274:   strcat(dest, "sprg2");  break;
    case 275:   strcat(dest, "sprg3");  break;
    case 282:   strcat(dest, "ear");    break;
    case 287:   strcat(dest, "pvr");    break;
    case 528:   strcat(dest, "ibat0u"); break;
    case 529:   strcat(dest, "ibat0l"); break;
    case 530:   strcat(dest, "ibat1u"); break;
    case 531:   strcat(dest, "ibat1l"); break;
    case 532:   strcat(dest, "ibat2u"); break;
    case 533:   strcat(dest, "ibat2l"); break;
    case 534:   strcat(dest, "ibat3u"); break;
    case 535:   strcat(dest, "ibat3l"); break;
    case 536:   strcat(dest, "dbat0u"); break;
    case 537:   strcat(dest, "dbat0l"); break;
    case 538:   strcat(dest, "dbat1u"); break;
    case 539:   strcat(dest, "dbat1l"); break;
    case 540:   strcat(dest, "dbat2u"); break;
    case 541:   strcat(dest, "dbat2l"); break;
    case 542:   strcat(dest, "dbat3u"); break;
    case 543:   strcat(dest, "dbat3l"); break;
    case 1013:  strcat(dest, "dabr");   break;  // unsupported on 603e/EC603e

    /*
     * Some PowerPC implementations may implement MFTB and MFSPR identically,
     * therefore TBR registers are also decoded here
     */

    case 268:   strcat(dest, "tbl");    break;
    case 269:   strcat(dest, "tbu");    break;

    /*
     * PowerPC 603e/EC603e-specific registers
     */

    case 1008:  strcat(dest, "hid0");   break;
    case 1009:  strcat(dest, "hid1");   break;
    case 976:   strcat(dest, "dmiss");  break;
    case 977:   strcat(dest, "dcmp");   break;
    case 978:   strcat(dest, "hash2");  break;
    case 979:   strcat(dest, "hash2");  break;
    case 980:   strcat(dest, "imiss");  break;
    case 981:   strcat(dest, "icmp");   break;
    case 982:   strcat(dest, "rpa");    break;
    case 1010:  strcat(dest, "iabr");   break;

    default:    sprintf(dest, "%s%d", dest, spr);
                break;
    }
}

/*
 * DecodeSigned16():
 *
 * Predecodes the SIMM field for us. If do_unsigned, it is printed as an
 * unsigned 32-bit integer.
 */
static void DecodeSigned16(char *outbuf, UINT32 op, bool do_unsigned)
{
    INT16   s;

    s = G_SIMM(op);
    if (do_unsigned)    // sign extend to unsigned 32-bits
        sprintf(outbuf, "0x%04X", (UINT32) s);
    else                // print as signed 16 bits
    {
        if (s < 0)
            sprintf(outbuf, "-0x%02X", -s);
        else
            sprintf(outbuf, "0x%02X",s);
    }
}

/*
 * Mask():
 *
 * Generate a mask from bit MB through ME (PPC-style backwards bit numbering.)
 */
static UINT32 Mask(unsigned mb, unsigned me)
{
	UINT32	i, mask;

    mb &= 31;
    me &= 31;

    i = mb;
    mask = 0;
    while (1)
    {
        mask |= (1 << (31 - i));
        if (i == me)
            break;
        i = (i + 1) & 31;
    }

    return mask;
}

/*
 * Check():
 *
 * Perform checks on the instruction as required by the flags. Returns 1 if
 * the instruction failed.
 */
static bool Check(UINT32 op, unsigned flags)
{
    unsigned	nb, rt, ra;

    if (!flags) return OKAY;	// nothing to check for!

    rt = G_RT(op);
    ra = G_RA(op);

    if (flags & FL_CHECK_RA_RT) // invalid if rA==0 or rA==rT
    {
        if ((G_RA(op) == 0) || (G_RA(op) == G_RT(op)))
            return FAIL;
    }

    if (flags & FL_CHECK_RA)    // invalid if rA==0
    {
        if (G_RA(op) == 0)
            return FAIL;
    }

    if (flags & FL_CHECK_LSWI)
    {
        /*
         * Check that rA is not in the range of registers to be loaded (even
         * if rA == 0)
         */

        nb = G_NB(op);

        if (ra >= rt && ra <= (rt + nb - 1))    return FAIL;
        if ((rt + nb - 1) > 31) // register wrap-around!
        {
            if (ra < ((rt + nb - 1) - 31))
                return FAIL;
        }
    }

    if (flags & FL_CHECK_LSWX)
    {
        /*
         * Check that rT != rA, rT != rB, and rD and rA both do not specify
         * R0.
         *
         * We cannot check fully whether rA or rB are in the range of
         * registers specified to be loaded because that depends on XER.
         */

        if (rt == ra || rt == G_RB(op) || ((rt == 0) && (ra == 0)))
            return FAIL;
    }

    return OKAY;	// passed checks
}

/*
 * Simplified():
 *
 * Handles all simplified instruction forms. Returns 1 if one was decoded,
 * otherwise 0 to indicate disassembly should carry on as normal.
 */
static bool Simplified(UINT32 op, UINT32 vpc, char *signed16, char *mnem, char *oprs)
{
    UINT32  value, disp;

    value = G_SIMM(op); // value is fully sign-extended SIMM field
    if (value & 0x8000)
        value |= 0xffff0000;

    if (op == (D_OP(24)|D_RT(0)|D_RA(0)|D_UIMM(0)))
        strcat(mnem, "nop");        // ori r0,r0,0 -> nop
    else if ((op & ~(M_RT|M_RA|M_RB|M_RC)) == (D_OP(31)|D_XO(444)))
    {
        if (G_RT(op) == G_RB(op))
        {
            strcat(mnem, "mr");     // orx rA,rT,rT -> mrx rA,rT
            if (op & M_RC)  strcat(mnem, ".");
            sprintf(oprs, "r%d,r%d", G_RA(op), G_RT(op));
        }
        else
            return 0;
    }
    else if ((op & ~(M_RT|M_RA|M_RB|M_RC)) == (D_OP(31)|D_XO(124)))
    {
        if (G_RT(op) == G_RB(op))
        {
            strcat(mnem, "not");    // nor rA,rT,rT -> not rA,rT
            if (op & M_RC)  strcat(mnem, ".");
            sprintf(oprs, "r%d,r%d", G_RA(op), G_RT(op));
        }
        else
            return 0;
    }
    else if ((op & ~(M_RT|M_RA|M_SIMM)) == D_OP(14))
    {
        if (G_RA(op) == 0)
        {
            strcat(mnem, "li");     // addi rT,0,value -> li rT,value
            sprintf(oprs, "r%d,0x%08X", G_RT(op), value);
        }
        else
            return 0;
    }
    else if ((op & ~(M_RT|M_RA|M_SIMM)) == D_OP(15))
    {
        if (G_RA(op) == 0)
        {
            strcat(mnem, "li"); // addis rT,0,value -> li rT,(value<<16)
            sprintf(oprs, "r%d,0x%08X", G_RT(op), value << 16);
        }
        else
        {
            strcat(mnem, "addi");   // addis rT,rA,SIMM -> addi rT,rA,SIMM<<16
            sprintf(oprs, "r%d,r%d,0x%08X", G_RT(op), G_RA(op), value << 16);
        }
    }
    else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(29))
    {
        strcat(mnem, "andi.");  // andis. rA,rT,UIMM -> andi. rA,rT,UIMM<<16
        sprintf(oprs, "r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
    }
    else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(25))
    {
        strcat(mnem, "ori");    // oris rA,rT,UIMM -> ori rA,rT,UIMM<<16
        sprintf(oprs, "r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
    }
    else if ((op & ~(M_RT|M_RA|M_UIMM)) == D_OP(27))
    {
        strcat(mnem, "xori");   // xoris rA,rT,UIMM -> xori rA,rT,UIMM<<16
        sprintf(oprs, "r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_UIMM(op) << 16);
    }        
    else if ((op & ~(M_RT|M_RA|M_SH|M_MB|M_ME|M_RC)) == D_OP(20))
    {
        value = Mask(G_MB(op), G_ME(op));
        strcat(mnem, "rlwimi"); // rlwimi[.] rA,rT,SH,MB,ME -> rlwimi[.] rA,rT,SH,MASK
        if (op & M_RC) strcat(mnem, ".");
        sprintf(oprs, "r%d,r%d,%d,0x%08X", G_RA(op), G_RT(op), G_SH(op), value);
    }
    else if ((op & ~(M_RT|M_RA|M_SH|M_MB|M_ME|M_RC)) == D_OP(21))
    {
        value = Mask(G_MB(op), G_ME(op));
        if (G_SH(op) == 0)      // rlwinm[.] rA,rT,0,MB,ME -> and[.] rA,rT,MASK
        {
            strcat(mnem, "and");
        	if (op & M_RC) strcat(mnem, ".");
            sprintf(oprs, "r%d,r%d,0x%08X", G_RA(op), G_RT(op), value);
        }
        else                    // rlwinm[.] rA,rT,SH,MASK
        {
            strcat(mnem, "rlwinm");
        	if (op & M_RC) strcat(mnem, ".");
            sprintf(oprs, "r%d,r%d,%d,0x%08X", G_RA(op), G_RT(op), G_SH(op), value);
        }
    }
    else if ((op & ~(M_RT|M_RA|M_RB|M_MB|M_ME|M_RC)) == D_OP(23))
    {
        value = Mask(G_MB(op), G_ME(op));
        strcat(mnem, "rlwnm");  // rlwnm[.] rA,rT,SH,MB,ME -> rlwnm[.] rA,rT,SH,MASK
        if (op & M_RC) strcat(mnem, ".");
        sprintf(oprs, "r%d,r%d,r%d,0x%08X", G_RA(op), G_RT(op), G_RB(op), value);
    }
    else if ((op & ~(M_BO|M_BI|M_BD|M_AA|M_LK)) == D_OP(16))
    {
        disp = G_BD(op) * 4;
        if (disp & 0x00008000)
            disp |= 0xffff0000;

        switch (G_BO(op))
        {
        case 0x04:  // branch if condition is false
        case 0x05:
        case 0x06:
        case 0x07:
            strcat(mnem, "bf");
            break;
        case 0x0c:
        case 0x0d:
        case 0x0e:
        case 0x0f:
            strcat(mnem, "bt");
            break;
        default:
            return 0;
        }

        if (op & M_LK)  strcat(mnem, "l");
        if (op & M_AA)  strcat(mnem, "a");

        sprintf(oprs, "cr%d[%s],0x%08X", G_BI(op) / 4, crbit[G_BI(op) & 3], disp + ((op & M_AA) ? 0 : vpc));
    }
    else if ((op & ~(M_RT|M_RA|M_RB|M_OE|M_RC)) == (D_OP(31)|D_XO(40)))
    {
        strcat(mnem, "sub");
        if (op & M_OE) strcat(mnem, "o");
        if (op & M_RC) strcat(mnem, ".");
        sprintf(oprs, "r%d,r%d,r%d", G_RT(op), G_RB(op), G_RA(op));
    }
    else if ((op & ~(M_RT|M_RA|M_RB|M_OE|M_RC)) == (D_OP(31)|D_XO(8)))
    {
        strcat(mnem, "subc");
        if (op & M_OE) strcat(mnem, "o");
        if (op & M_RC) strcat(mnem, ".");
		sprintf(oprs, "r%d,r%d,r%d", G_RT(op), G_RB(op), G_RA(op));
    }
    else
        return 0;   // no match
	return 1;
}

/*
 * DisassemblePowerPC(op, vpc, mnem, oprs, simplify):
 *
 * Disassembles one PowerPC 603e instruction. 
 *
 * A non-zero return code indicates that the instruction could not be
 * recognized or that the operands to an instruction were invalid. To
 * determine which case occured, check if mnem[0] == '\0'. If it does not,
 * then the latter case happened.
 *
 * Arguments:
 *      op         Instruction word to disassemble.
 *      vpc        Current instruction address.
 *      mnem       Buffer to write instruction mnemonic to. If no
 *                 instruction was decoded, mnem[0] and oprs[0] will be set
 *                 to '\0'.
 *      oprs       Buffer to write any operands to.
 *      simplify   If non-zero, simplified forms of instructions will be
 *                 printed in certain cases.
 *
 * Returns:
 *      Zero if successful, non-zero if the instruction was unrecognized or
 *      had an invalid form (see note above in function description.)
 */ 
bool DisassemblePowerPC(UINT32 op, UINT32 vpc, char *mnem, char *oprs,
                        bool simplify)
{
    char    signed16[12];
    UINT32  disp;
    int     i;

    mnem[0] = '\0'; // so we can use strcat()
    oprs[0] = '\0';

    /*
     * Decode signed 16-bit fields (SIMM and d) to spare us the work later
     */

    DecodeSigned16(signed16, op, 0);

    /*
     * Try simplified forms first, then real instructions
     */

    if (simplify)
    {
        if (Simplified(op, vpc, signed16, mnem, oprs))
            return OKAY;
    }

    /*
     * Search for the instruction in the list and print it if there's a match
     */

    for (i = 0; i < sizeof(itab) / sizeof(IDESCR); i++)
    {
        if ((op & ~itab[i].mask) == itab[i].match)  // check for match
        {
            /*
             * Base mnemonic followed be O, ., L, A
             */

            strcat(mnem, itab[i].mnem);
            if (itab[i].flags & FL_OE)  if (op & M_OE) strcat(mnem, "o");
            if (itab[i].flags & FL_RC)  if (op & M_RC) strcat(mnem, ".");
            if (itab[i].flags & FL_LK)  if (op & M_LK) strcat(mnem, "l");
            if (itab[i].flags & FL_AA)  if (op & M_AA) strcat(mnem, "a");

            /*
             * Print operands
             */

            switch (itab[i].format)
            {
            case F_RT_RA_RB:
                sprintf(oprs, "r%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
                break;

            case F_RT_RA_0_SIMM:
                if (G_RA(op))
                    sprintf(oprs, "r%d,r%d,%s", G_RT(op), G_RA(op), signed16);
                else
                    sprintf(oprs, "r%d,0,%s", G_RT(op), signed16);
                break;

            case F_ADDIS:
                if (G_RA(op))
                    sprintf(oprs, "r%d,r%d,0x%04X", G_RT(op), G_RA(op), G_SIMM(op));
                else
                    sprintf(oprs, "r%d,0,0x%04X", G_RT(op), G_SIMM(op));
                break;

            case F_RT_RA_SIMM:
                sprintf(oprs, "r%d,r%d,%s", G_RT(op), G_RA(op), signed16);
                break;

            case F_RT_RA:
                sprintf(oprs, "r%d,r%d", G_RT(op), G_RA(op));
                break;

            case F_RA_RT_RB:
                sprintf(oprs, "r%d,r%d,r%d", G_RA(op), G_RT(op), G_RB(op));
                break;

            case F_RA_RT_UIMM:
                sprintf(oprs, "r%d,r%d,0x%04X", G_RA(op), G_RT(op), G_UIMM(op));
                break;

            case F_LI:
                disp = G_LI(op) * 4;
                if (disp & 0x02000000)  // sign extend
                    disp |= 0xfc000000;
                sprintf(oprs, "0x%08X", disp + ((op & M_AA) ? 0 : vpc));
                break;

            case F_BCx:
                disp = G_BD(op) * 4;
                if (disp & 0x00008000)
                    disp |= 0xffff0000;

                if (G_BO(op) & 0x10)    // BI is ignored (don't print CR bit)
                    sprintf(oprs, "0x%02X,%d,0x%08X", G_BO(op), G_BI(op), disp + ((op & M_AA) ? 0 : vpc));
                else                    // BI gives us the condition bit
                    sprintf(oprs, "0x%02X,cr%d[%s],0x%08X", G_BO(op), G_BI(op) / 4, crbit[G_BI(op) & 3], disp + ((op & M_AA) ? 0 : vpc));
                break;

            case F_BO_BI:
                if (G_BO(op) & 0x10)    // BI is ignored (don't print CR bit)
                    sprintf(oprs, "0x%02X,%d", G_BO(op), G_BI(op));
                else
                    sprintf(oprs, "0x%02X,cr%d[%s]", G_BO(op), G_BI(op) / 4, crbit[G_BI(op) & 3]);
                break;

            case F_CMP:
                sprintf(oprs, "cr%d,%d,r%d,r%d", G_CRFD(op), G_L(op), G_RA(op), G_RB(op));
                break;

            case F_CMP_SIMM:
                sprintf(oprs, "cr%d,%d,r%d,%s", G_CRFD(op), G_L(op), G_RA(op), signed16);
                break;

            case F_CMP_UIMM:
                sprintf(oprs, "cr%d,%d,r%d,0x%04X", G_CRFD(op), G_L(op), G_RA(op), G_UIMM(op));
                break;

            case F_RA_RT:
                sprintf(oprs, "r%d,r%d", G_RA(op), G_RT(op));
                break;

            case F_CRBD_CRBA_CRBB:
                sprintf(oprs, "cr%d[%s],cr%d[%s],cr%d[%s]", G_CRBD(op) / 4, crbit[G_CRBD(op) & 3], G_CRBA(op) / 4, crbit[G_CRBA(op) & 3], G_CRBB(op) / 4, crbit[G_CRBB(op) & 3]);
                break;

            case F_RA_0_RB:
                if (G_RA(op))
                    sprintf(oprs, "r%d,r%d", G_RA(op), G_RB(op));
                else
                    sprintf(oprs, "0,r%d", G_RB(op));
                break;

            case F_RT_RA_0_RB:
                if (G_RA(op))
                    sprintf(oprs, "r%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
                else
                    sprintf(oprs, "r%d,0,r%d", G_RT(op), G_RB(op));
                break;

            case F_FRT_FRB:
                sprintf(oprs, "f%d,f%d", G_RT(op), G_RB(op));
                break;

            case F_FRT_FRA_FRB:
                sprintf(oprs, "f%d,f%d,f%d", G_RT(op), G_RA(op), G_RB(op));
                break;

            case F_FCMP:
                sprintf(oprs, "cr%d,f%d,f%d", G_CRFD(op), G_RA(op), G_RB(op));
                break;

            case F_FRT_FRA_FRC_FRB:
                sprintf(oprs, "f%d,f%d,f%d,f%d", G_RT(op), G_RA(op), G_REGC(op), G_RB(op));
                break;

            case F_FRT_FRA_FRC:
                sprintf(oprs, "f%d,f%d,f%d", G_RT(op), G_RA(op), G_REGC(op));
                break;

            case F_RT_D_RA_0:
                if (G_RA(op))
                    sprintf(oprs, "r%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
                else
                    sprintf(oprs, "r%d,0x%08X", G_RT(op), (UINT32) ((INT16) G_D(op)));
                break;

            case F_RT_D_RA:
                sprintf(oprs, "r%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
                break;

            case F_FRT_D_RA_0:
                if (G_RA(op))
                    sprintf(oprs, "f%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
                else
                    sprintf(oprs, "f%d,0x%08X", G_RT(op), (UINT32) ((INT16) G_D(op)));
                break;

            case F_FRT_D_RA:
                sprintf(oprs, "f%d,%s(r%d)", G_RT(op), signed16, G_RA(op));
                break;

            case F_FRT_RA_RB:
                sprintf(oprs, "f%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
                break;

            case F_FRT_RA_0_RB:
                if (G_RA(op))
                    sprintf(oprs, "f%d,r%d,r%d", G_RT(op), G_RA(op), G_RB(op));
                else
                    sprintf(oprs, "f%d,0,r%d", G_RT(op), G_RB(op));
                break;

            case F_RT_RA_0_NB:
                if (G_RA(op))
                    sprintf(oprs, "r%d,r%d,%d", G_RT(op), G_RA(op), G_NB(op) ? G_NB(op) : 32);
                else
                    sprintf(oprs, "r%d,0,%d", G_RT(op), G_NB(op) ? G_NB(op) : 32);
                break;

            case F_CRFD_CRFS:
                sprintf(oprs, "cr%d,cr%d", G_CRFD(op), G_CRFS(op));
                break;

            case F_MCRXR:
                sprintf(oprs, "cr%d", G_CRFD(op));
                break;

            case F_RT:
                sprintf(oprs, "r%d", G_RT(op));
                break;

            case F_MFFSx:
                sprintf(oprs, "f%d", G_RT(op));
                break;

            case F_FCRBD:
                sprintf(oprs, "fpscr[%d]", G_CRBD(op));
                break;

            case F_RT_SPR:
                sprintf(oprs, "r%d,", G_RT(op));
                SPR(oprs, G_SPR(op));
                break;

            case F_MFSR:
                sprintf(oprs, "r%d,sr%d", G_RT(op), G_SR(op));
                break;

            case F_MTCRF:
                sprintf(oprs, "0x%02X,r%d", G_CRM(op), G_RT(op));
                break;

            case F_MTFSFx:
                sprintf(oprs, "0x%02X,f%d", G_FM(op), G_RB(op));
                break;

            case F_MTFSFIx:
                sprintf(oprs, "cr%d,0x%X", G_CRFD(op), G_IMM(op));
                break;

            case F_MTSPR:
                SPR(oprs, G_SPR(op));
                sprintf(oprs, "%s,r%d", oprs, G_RT(op));
                break;

            case F_MTSR:
                sprintf(oprs, "sr%d,r%d", G_SR(op), G_RT(op));
                break;

            case F_RT_RB:
                sprintf(oprs, "r%d,r%d", G_RT(op), G_RB(op));
                break;

            case F_RA_RT_SH_MB_ME:
                sprintf(oprs, "r%d,r%d,%d,%d,%d", G_RA(op), G_RT(op), G_SH(op), G_MB(op), G_ME(op));
                break;

            case F_RLWNMx:
                sprintf(oprs, "r%d,r%d,r%d,%d,%d", G_RA(op), G_RT(op), G_RB(op), G_MB(op), G_ME(op));
                break;

            case F_SRAWIx:
                sprintf(oprs, "r%d,r%d,%d", G_RA(op), G_RT(op), G_SH(op));
                break;

            case F_RB:
                sprintf(oprs, "r%d", G_RB(op));
                break;

            case F_TW:
                sprintf(oprs, "%d,r%d,r%d", G_TO(op), G_RA(op), G_RB(op));
                break;

            case F_TWI:
                sprintf(oprs, "%d,r%d,%s", G_TO(op), G_RA(op), signed16);
                break;

            case F_NONE:
            default:
                break;
            }

            return Check(op, itab[i].flags);
        }
    }

    return FAIL;	// no match found
}


/******************************************************************************
 Standalone Disassembler
 
 Define STANDALONE to build a command line-driven PowerPC disassembler.
******************************************************************************/

#ifdef STANDALONE

static void PrintUsage(void)
{
    puts("ppcd Version "DISASM_VERSION" by Bart Trzynadlowski: PowerPC 603e Disassembler");
    puts("Usage:    ppcd <file> [options]");
    puts("Options:  -?,-h       Show this help text");
    puts("          -s <offset> Start offset (hexadecimal)");
    puts("          -l <num>    Number of instructions");
    puts("          -o <addr>   Set origin (hexadecimal)");
    puts("          -big        Big endian [Default]");
    puts("          -little     Little endian");
    puts("          -simple     Use simplified instruction forms [Default]");
    puts("          -nosimple   Do not use simplified forms");
    exit(0);
}

/*
 * main(argc, argv):
 *
 * Standalone PowerPC disassembler.
 */
int main(int argc, char **argv)
{
    char    	mnem[16], oprs[48];
    FILE    	*fp;
    UINT8   	*buffer;
    unsigned	i, fsize, start = 0, len, org, file = 0;
    UINT32  	op;
    bool    	len_specified = 0, org_specified = 0, little = 0, simple = 1;
    char    	*c;


    if (argc <= 1)
        PrintUsage();

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-?"))
            PrintUsage();
        else if (!strcmp(argv[i], "-s"))
        {
            ++i;
            if (i >= argc)
                fprintf(stderr, "ppcd: warning: no argument to %s\n", "-s");
            else
                start = strtoul(argv[i], &c, 16);
        }
        else if (!strcmp(argv[i], "-l"))
        {
            ++i;
            if (i >= argc)
                fprintf(stderr, "ppcd: warning: no argument to %s\n", "-l");
            else
            {
                len = atoi(argv[i]);
                len_specified = 1;
            }
        }
        else if (!strcmp(argv[i], "-o"))
        {
            ++i;
            if (i >= argc)
                fprintf(stderr, "ppcd: warning: no argument to %s\n", "-o");
            else
            {
                org = strtoul(argv[i], &c, 16);
                org_specified = 1;
            }
        }
        else if (!strcmp(argv[i], "-big"))
            little = 0;
        else if (!strcmp(argv[i], "-little"))
            little = 1;
        else if (!strcmp(argv[i], "-simple"))
            simple = 1;
        else if (!strcmp(argv[i], "-nosimple"))
            simple = 0;
        else
            file = i;
    }

    if (!file)
    {
        fprintf(stderr, "ppcd: no input file specified\n");
        exit(1);
    }
            
    /*
     * Load file
     */

    if ((fp = fopen(argv[file], "rb")) == NULL)
    {
        fprintf(stderr, "ppcd: failed to open file: %s\n", argv[file]);
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    rewind(fp);

    if ((buffer = (UINT8 *) calloc(fsize, sizeof(UINT8))) == NULL)
    {              
        fprintf(stderr, "ppcd: not enough memory to load input file: %s, %lu bytes\n", argv[file], (unsigned long) fsize);
        fclose(fp);
        exit(1);
    }
    fread(buffer, sizeof(UINT8), fsize, fp);
    fclose(fp);

    if (!len_specified)
        len = fsize - start;
    else
        len *= 4;   // each instruction == 4 bytes

    if (!org_specified)
        org = start;

    /*
     * Disassemble!
     */

    for (i = start; i < fsize && i < (start + len); i += 4, org += 4)
    {
        if (!little)
            op = (buffer[i] << 24) | (buffer[i + 1] << 16) |
                 (buffer[i + 2] << 8) | buffer[i + 3];
        else
            op = (buffer[i + 3] << 24) | (buffer[i + 2] << 16) |
                 (buffer[i + 1] << 8) | buffer[i + 0];

        if (DisassemblePowerPC(op, org, mnem, oprs, simple))
        {
            if (mnem[0] != '\0')    // invalid form
                printf("0x%08X: 0x%08X\t%s*\t%s\n", org, op, mnem, oprs);
            else
                printf("0x%08X: 0x%08X\t?\n", org, op);
        }
        else
            printf("0x%08X: 0x%08X\t%s\t%s\n", org, op, mnem, oprs);
    }

    free(buffer);

    return 0;
}

#endif

