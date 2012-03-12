/*****************************************************************************
 *
 *   arm7exec.c
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
 *****************************************************************************/

/******************************************************************************
 *  Notes:
 *         This file contains the code to run during the CPU EXECUTE METHOD.
 *         It has been split into it's own file (from the arm7core.c) so it can be
 *         directly compiled into any cpu core that wishes to use it.
 *
 *         It should be included as follows in your cpu core:
 *
 *         int arm7_execute(int cycles)
 *         {
 *         #include "arm7exec.c"
 *         }
 *
*****************************************************************************/

/* This implementation uses an improved switch() for hopefully faster opcode fetches compared to my last version
.. though there's still room for improvement. */
{
    UINT32 pc;
    UINT32 insn;

    ARM7_ICOUNT = cycles;
    curr_cycles = total_cycles;

    do
    {
        /* handle Thumb instructions if active */
        if (T_IS_SET(GET_CPSR))
        {
            UINT32 readword;
            UINT32 addr;
            UINT32 rm, rn, rs, rd, op2, imm, rrs, rrd;
            INT32 offs;

            pc = R15;
            insn = cpu_readop16(pc & (~1));
            ARM7_ICOUNT -= (3 - thumbCycles[insn >> 8]);
            switch ((insn & THUMB_INSN_TYPE) >> THUMB_INSN_TYPE_SHIFT)
            {
                case 0x0: /* Logical shifting */
                    SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
                    if (insn & THUMB_SHIFT_R) /* Shift right */
                    {
                        rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                        rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                        rrs = GET_REGISTER(rs);
                        offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
                        if (offs != 0)
                        {
                            SET_REGISTER(rd, rrs >> offs);
                            if (rrs & (1 << (offs - 1)))
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                        }
                        else
                        {
                            SET_REGISTER(rd, 0);
                            if (rrs & 0x80000000)
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                        }
                        SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                        SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                        R15 += 2;
                    }
                    else /* Shift left */
                    {
                        rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                        rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                        rrs = GET_REGISTER(rs);
                        offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
                        if (offs != 0)
                        {
                            SET_REGISTER(rd, rrs << offs);
                            if (rrs & (1 << (31 - (offs - 1))))
                            {
                                SET_CPSR(GET_CPSR | C_MASK);
                            }
                            else
                            {
                                SET_CPSR(GET_CPSR & ~C_MASK);
                            }
                        }
                        else
                        {
                            SET_REGISTER(rd, rrs);
                        }
                        SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                        SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                        R15 += 2;
                    }
                    break;
                case 0x1: /* Arithmetic */
                    if (insn & THUMB_INSN_ADDSUB)
                    {
                        switch ((insn & THUMB_ADDSUB_TYPE) >> THUMB_ADDSUB_TYPE_SHIFT)
                        {
                            case 0x0: /* ADD Rd, Rs, Rn */
                                rn = GET_REGISTER((insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
                                rs = GET_REGISTER((insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
                                rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                SET_REGISTER(rd, rs + rn);
                                HandleThumbALUAddFlags(GET_REGISTER(rd), rs, rn);
                                break;
                            case 0x1: /* SUB Rd, Rs, Rn */
                                rn = GET_REGISTER((insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT);
                                rs = GET_REGISTER((insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
                                rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                SET_REGISTER(rd, rs - rn);
                                HandleThumbALUSubFlags(GET_REGISTER(rd), rs, rn);
                                break;
                            case 0x2: /* ADD Rd, Rs, #imm */
                                imm = (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
                                rs = GET_REGISTER((insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
                                rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                SET_REGISTER(rd, rs + imm);
                                HandleThumbALUAddFlags(GET_REGISTER(rd), rs, imm);
                                break;
                            case 0x3: /* SUB Rd, Rs, #imm */
                                imm = (insn & THUMB_ADDSUB_RNIMM) >> THUMB_ADDSUB_RNIMM_SHIFT;
                                rs = GET_REGISTER((insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT);
                                rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                SET_REGISTER(rd, rs - imm);
                                HandleThumbALUSubFlags(GET_REGISTER(rd), rs,imm);
                                break;
                            default:
                                fatalerror("%08x: G1 Undefined Thumb instruction: %04x\n", pc, insn);
                                R15 += 2;
                                break;
                        }
                    }
                    else
                    {
                        /* ASR.. */
                        //if (insn & THUMB_SHIFT_R) /* Shift right */
                        {
                            rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                            rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                            rrs = GET_REGISTER(rs);
                            offs = (insn & THUMB_SHIFT_AMT) >> THUMB_SHIFT_AMT_SHIFT;
                            if (offs == 0)
                            {
                                offs = 32;
                            }
                            if (offs >= 32)
                            {
                                if (rrs >> 31)
                                {
                                    SET_CPSR(GET_CPSR | C_MASK);
                                }
                                else
                                {
                                    SET_CPSR(GET_CPSR & ~C_MASK);
                                }
                                SET_REGISTER(rd, (rrs & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
                            }
                            else
                            {
                                if ((rrs >> (offs - 1)) & 1)
                                {
                                    SET_CPSR(GET_CPSR | C_MASK);
                                }
                                else
                                {
                                    SET_CPSR(GET_CPSR & ~C_MASK);
                                }
                                SET_REGISTER(rd,
                                             (rrs & 0x80000000)
                                             ? ((0xFFFFFFFF << (32 - offs)) | (rrs >> offs))
                                             : (rrs >> offs));
                            }
                            SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
                            SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                            R15 += 2;
                        }
                    }
                    break;
                case 0x2: /* CMP / MOV */
                    if (insn & THUMB_INSN_CMP)
                    {
                        rn = GET_REGISTER((insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
                        op2 = insn & THUMB_INSN_IMM;
                        rd = rn - op2;
                        HandleThumbALUSubFlags(rd, rn, op2);
                        //mame_printf_debug("%08x: xxx Thumb instruction: CMP R%d (%08x), %02x (N=%d, Z=%d, C=%d, V=%d)\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, GET_REGISTER((insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT), op2, N_IS_SET(GET_CPSR) ? 1 : 0, Z_IS_SET(GET_CPSR) ? 1 : 0, C_IS_SET(GET_CPSR) ? 1 : 0, V_IS_SET(GET_CPSR) ? 1 : 0);
                    }
                    else
                    {
                        rd = (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT;
                        op2 = (insn & THUMB_INSN_IMM);
                        SET_REGISTER(rd, op2);
                        SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                        SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                        R15 += 2;
                    }
                    break;
                case 0x3: /* ADD/SUB immediate */
                    if (insn & THUMB_INSN_SUB) /* SUB Rd, #Offset8 */
                    {
                        rn = GET_REGISTER((insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
                        op2 = insn & THUMB_INSN_IMM;
                        //mame_printf_debug("%08x:  Thumb instruction: SUB R%d, %02x\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, op2);
                        rd = rn - op2;
                        SET_REGISTER((insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
                        HandleThumbALUSubFlags(rd, rn, op2);
                    }
                    else /* ADD Rd, #Offset8 */
                    {
                        rn = GET_REGISTER((insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT);
                        op2 = insn & THUMB_INSN_IMM;
                        rd = rn + op2;
                        //mame_printf_debug("%08x:  Thumb instruction: ADD R%d, %02x\n", pc, (insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, op2);
                        SET_REGISTER((insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, rd);
                        HandleThumbALUAddFlags(rd, rn, op2);
                    }
                    break;
                case 0x4: /* Rd & Rm instructions */
                    switch ((insn & THUMB_GROUP4_TYPE) >> THUMB_GROUP4_TYPE_SHIFT)
                    {
                        case 0x0:
                            switch ((insn & THUMB_ALUOP_TYPE) >> THUMB_ALUOP_TYPE_SHIFT)
                            {
                                case 0x0: /* AND Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    SET_REGISTER(rd, GET_REGISTER(rd) & GET_REGISTER(rs));
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                case 0x1: /* EOR Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    SET_REGISTER(rd, GET_REGISTER(rd) ^ GET_REGISTER(rs));
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                case 0x2: /* LSL Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    rrd = GET_REGISTER(rd);
                                    offs = GET_REGISTER(rs) & 0x000000ff;
                                    if (offs > 0)
                                    {
                                        if (offs < 32)
                                        {
                                            SET_REGISTER(rd, rrd << offs);
                                            if (rrd & (1 << (31 - (offs - 1))))
                                            {
                                                SET_CPSR(GET_CPSR | C_MASK);
                                            }
                                            else
                                            {
                                                SET_CPSR(GET_CPSR & ~C_MASK);
                                            }
                                        }
                                        else if (offs == 32)
                                        {
                                            SET_REGISTER(rd, 0);
                                            if (rrd & 1)
                                            {
                                                SET_CPSR(GET_CPSR | C_MASK);
                                            }
                                            else
                                            {
                                                SET_CPSR(GET_CPSR & ~C_MASK);
                                            }
                                        }
                                        else
                                        {
                                            SET_REGISTER(rd, 0);
                                            SET_CPSR(GET_CPSR & ~C_MASK);
                                        }
                                    }
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                case 0x3: /* LSR Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    rrd = GET_REGISTER(rd);
                                    offs = GET_REGISTER(rs) & 0x000000ff;
                                    if (offs >  0)
                                    {
                                        if (offs < 32)
                                        {
                                            SET_REGISTER(rd, rrd >> offs);
                                            if (rrd & (1 << (offs - 1)))
                                            {
                                                SET_CPSR(GET_CPSR | C_MASK);
                                            }
                                            else
                                            {
                                                SET_CPSR(GET_CPSR & ~C_MASK);
                                            }
                                        }
                                        else if (offs == 32)
                                        {
                                            SET_REGISTER(rd, 0);
                                            if (rrd & 0x80000000)
                                            {
                                                SET_CPSR(GET_CPSR | C_MASK);
                                            }
                                            else
                                            {
                                                SET_CPSR(GET_CPSR & ~C_MASK);
                                            }
                                        }
                                        else
                                        {
                                            SET_REGISTER(rd, 0);
                                            SET_CPSR(GET_CPSR & ~C_MASK);
                                        }
                                    }
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                case 0x4: /* ASR Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    rrs = GET_REGISTER(rs)&0xff;
                                    rrd = GET_REGISTER(rd);
                                    if (rrs != 0)
                                    {
                                        if (rrs >= 32)
                                        {
                                            if (rrd >> 31)
                                            {
                                                SET_CPSR(GET_CPSR | C_MASK);
                                            }
                                            else
                                            {
                                                SET_CPSR(GET_CPSR & ~C_MASK);
                                            }
                                            SET_REGISTER(rd, (GET_REGISTER(rd) & 0x80000000) ? 0xFFFFFFFF : 0x00000000);
                                        }
                                        else
                                        {
                                            if ((rrd >> (rrs-1)) & 1)
                                            {
                                                SET_CPSR(GET_CPSR | C_MASK);
                                            }
                                            else
                                            {
                                                SET_CPSR(GET_CPSR & ~C_MASK);
                                            }
                                            SET_REGISTER(rd, (rrd & 0x80000000)
                                                         ? ((0xFFFFFFFF << (32 - rrs)) | (rrd >> rrs))
                                                         : (rrd >> rrs));
                                        }
                                    }
                                    SET_CPSR(GET_CPSR & ~(N_MASK | Z_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                case 0x5: /* ADC Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    op2=(GET_CPSR & C_MASK) ? 1 : 0;
                                    rn=GET_REGISTER(rd) + GET_REGISTER(rs) + op2;
                                    HandleThumbALUAddFlags(rn, GET_REGISTER(rd), (GET_REGISTER(rs))); // ?
                                    SET_REGISTER(rd, rn);
                                    break;
                                case 0x6: /* SBC Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    op2=(GET_CPSR & C_MASK) ? 0 : 1;
                                    rn=GET_REGISTER(rd) - GET_REGISTER(rs) - op2;
                                    HandleThumbALUSubFlags(rn, GET_REGISTER(rd), (GET_REGISTER(rs))); //?
                                    SET_REGISTER(rd, rn);
                                    break;
                                case 0x7: /* ROR Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    rrd = GET_REGISTER(rd);
                                    imm = GET_REGISTER(rs) & 0x0000001f;
                                    SET_REGISTER(rd, (rrd >> imm) | (rrd << (32 - imm)));
                                    if (rrd & (1 << (imm - 1)))
                                    {
                                        SET_CPSR(GET_CPSR | C_MASK);
                                    }
                                    else
                                    {
                                        SET_CPSR(GET_CPSR & ~C_MASK);
                                    }
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                case 0x8: /* TST Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd) & GET_REGISTER(rs)));
                                    R15 += 2;
                                    break;
                                case 0x9: /* NEG Rd, Rs - todo: check me */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    rrs = GET_REGISTER(rs);
                                    rn = 0 - rrs;
                                    SET_REGISTER(rd, rn);
                                    HandleThumbALUSubFlags(GET_REGISTER(rd), 0, rrs);
                                    break;
                                case 0xa: /* CMP Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    rn = GET_REGISTER(rd) - GET_REGISTER(rs);
                                    HandleThumbALUSubFlags(rn, GET_REGISTER(rd), GET_REGISTER(rs));
                                    break;
                                case 0xb: /* CMN Rd, Rs - check flags, add dasm */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    rn = GET_REGISTER(rd) + GET_REGISTER(rs);
                                    HandleThumbALUAddFlags(rn, GET_REGISTER(rd), GET_REGISTER(rs));
                                    break;
                                case 0xc: /* ORR Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    SET_REGISTER(rd, GET_REGISTER(rd) | GET_REGISTER(rs));
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                case 0xd: /* MUL Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    rn = GET_REGISTER(rd) * GET_REGISTER(rs);
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_REGISTER(rd, rn);
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(rn));
                                    R15 += 2;
                                    break;
                                case 0xe: /* BIC Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    SET_REGISTER(rd, GET_REGISTER(rd) & (~GET_REGISTER(rs)));
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                case 0xf: /* MVN Rd, Rs */
                                    rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                                    rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                                    op2 = GET_REGISTER(rs);
                                    SET_REGISTER(rd, ~op2);
                                    SET_CPSR(GET_CPSR & ~(Z_MASK | N_MASK));
                                    SET_CPSR(GET_CPSR | HandleALUNZFlags(GET_REGISTER(rd)));
                                    R15 += 2;
                                    break;
                                default:
                                    fatalerror("%08x: G4-0 Undefined Thumb instruction: %04x %x\n", pc, insn, (insn & THUMB_ALUOP_TYPE) >> THUMB_ALUOP_TYPE_SHIFT);
                                    R15 += 2;
                                    break;
                            }
                            break;
                        case 0x1:
                            switch ((insn & THUMB_HIREG_OP) >> THUMB_HIREG_OP_SHIFT)
                            {
                                case 0x0: /* ADD Rd, Rs */
                                    rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                                    rd = insn & THUMB_HIREG_RD;
                                    switch ((insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT)
                                    {
                                        case 0x1: /* ADD Rd, HRs */
                                            SET_REGISTER(rd, GET_REGISTER(rd) + GET_REGISTER(rs+8));
                                            // emulate the effects of pre-fetch
                                            if (rs == 7)
                                            {
                                                SET_REGISTER(rd, GET_REGISTER(rd) + 4);
                                            }
                                            break;
                                        case 0x2: /* ADD HRd, Rs */
                                            SET_REGISTER(rd+8, GET_REGISTER(rd+8) + GET_REGISTER(rs));
                                            if (rd == 7)
                                            {
                                                R15 += 2;
                                            //    change_pc(R15);
                                            }
                                            break;
                                        case 0x3: /* Add HRd, HRs */
                                            SET_REGISTER(rd+8, GET_REGISTER(rd+8) + GET_REGISTER(rs+8));
                                            // emulate the effects of pre-fetch
                                            if (rs == 7)
                                            {
                                                SET_REGISTER(rd+8, GET_REGISTER(rd+8) + 4);
                                            }
                                            if (rd == 7)
                                            {
                                                R15 += 2;
                                            //    change_pc(R15);
                                            }
                                            break;
                                        default:
                                            fatalerror("%08x: G4-1-0 Undefined Thumb instruction: %04x %x\n", pc, insn, (insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);
                                            break;
                                    }
                                    R15 += 2;
                                    break;
                                case 0x1: /* CMP */
                                    switch ((insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT)
                                    {
                                        case 0x0: /* CMP Rd, Rs */
                                            rs = GET_REGISTER(((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
                                            rd = GET_REGISTER(insn & THUMB_HIREG_RD);
                                            rn = rd - rs;
                                            HandleThumbALUSubFlags(rn, rd, rs);
                                            break;
                                        case 0x1: /* CMP Rd, Hs */
                                            rs = GET_REGISTER(((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
                                            rd = GET_REGISTER(insn & THUMB_HIREG_RD);
                                            rn = rd - rs;
                                            HandleThumbALUSubFlags(rn, rd, rs);
                                            break;
                                        case 0x2: /* CMP Hd, Rs */
                                            rs = GET_REGISTER(((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT));
                                            rd = GET_REGISTER((insn & THUMB_HIREG_RD) + 8);
                                            rn = rd - rs;
                                            HandleThumbALUSubFlags(rn, rd, rs);
                                            break;
                                        case 0x3: /* CMP Hd, Hs */
                                            rs = GET_REGISTER(((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
                                            rd = GET_REGISTER((insn & THUMB_HIREG_RD) + 8);
                                            rn = rd - rs;
                                            HandleThumbALUSubFlags(rn, rd, rs);
                                            break;
                                        default:
                                            fatalerror("%08x: G4-1 Undefined Thumb instruction: %04x %x\n", pc, insn, (insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);
                                            R15 += 2;
                                            break;
                                    }
                                    break;
                                case 0x2: /* MOV */
                                    switch ((insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT)
                                    {
                                        case 0x1:       // MOV Rd, Hs
                                            rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                                            rd = insn & THUMB_HIREG_RD;
                                            if (rs == 7)
                                            {
                                                SET_REGISTER(rd, GET_REGISTER(rs + 8) + 4);
                                            }
                                            else
                                            {
                                                SET_REGISTER(rd, GET_REGISTER(rs + 8));
                                            }
                                            R15 += 2;
                                            break;
                                        case 0x2:       // MOV Hd, Rs
                                            rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                                            rd = insn & THUMB_HIREG_RD;
                                            SET_REGISTER(rd + 8, GET_REGISTER(rs));
                                            if (rd != 7)
                                            {
                                                R15 += 2;
                                            }
                                            else
                                            {
                                                R15 &= ~1;
                                           //     change_pc(R15);
                                            }
                                            break;
                                        case 0x3:       // MOV Hd, Hs
                                            rs = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                                            rd = insn & THUMB_HIREG_RD;
                                            if (rs == 7)
                                            {
                                                SET_REGISTER(rd + 8, GET_REGISTER(rs+8)+4);
                                            }
                                            else
                                            {
                                                SET_REGISTER(rd + 8, GET_REGISTER(rs+8));
                                            }
                                            if (rd != 7)
                                            {
                                                R15 += 2;
                                            }
                                            if (rd == 7)
                                            {
                                                R15 &= ~1;
                                           //     change_pc(R15);
                                            }
                                            break;
                                        default:
                                            fatalerror("%08x: G4-2 Undefined Thumb instruction: %04x (%x)\n", pc, insn, (insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT);
                                            R15 += 2;
                                            break;
                                    }
                                    break;
                                case 0x3:
                                    switch ((insn & THUMB_HIREG_H) >> THUMB_HIREG_H_SHIFT)
                                    {
                                        case 0x0:
                                            rd = (insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT;
                                            addr = GET_REGISTER(rd);
                                            if (addr & 1)
                                            {
                                                addr &= ~1;
                                            }
                                            else
                                            {
                                                SET_CPSR(GET_CPSR & ~T_MASK);
                                                if (addr & 2)
                                                {
                                                    addr += 2;
                                                }
                                            }
                                            R15 = addr;
                                            break;
                                        case 0x1:
                                            addr = GET_REGISTER(((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8);
                                            if ((((insn & THUMB_HIREG_RS) >> THUMB_HIREG_RS_SHIFT) + 8) == 15)
                                            {
                                                addr += 2;
                                            }
                                            if (addr & 1)
                                            {
                                                addr &= ~1;
                                            }
                                            else
                                            {
                                                SET_CPSR(GET_CPSR & ~T_MASK);
                                                if (addr & 2)
                                                {
                                                    addr += 2;
                                                }
                                            }
                                            R15 = addr;
                                            break;
                                        default:
                                            fatalerror("%08x: G4-3 Undefined Thumb instruction: %04x\n", pc, insn);
                                            R15 += 2;
                                            break;
                                    }
                                    break;
                                default:
                                    fatalerror("%08x: G4-x Undefined Thumb instruction: %04x\n", pc, insn);
                                    R15 += 2;
                                    break;
                            }
                            break;
                        case 0x2:
                        case 0x3:
                            readword = READ32((R15 & ~2) + 4 + ((insn & THUMB_INSN_IMM) << 2));
                            SET_REGISTER((insn & THUMB_INSN_IMM_RD) >> THUMB_INSN_IMM_RD_SHIFT, readword);
                            R15 += 2;
                            break;
                        default:
                            fatalerror("%08x: G4-y Undefined Thumb instruction: %04x\n", pc, insn);
                            R15 += 2;
                            break;
                    }
                    break;
                case 0x5: /* LDR* STR* */
                    switch ((insn & THUMB_GROUP5_TYPE) >> THUMB_GROUP5_TYPE_SHIFT)
                    {
                        case 0x0: /* STR Rd, [Rn, Rm] */
                            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
                            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
                            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
                            addr = GET_REGISTER(rn) + GET_REGISTER(rm);
                            WRITE32(addr, GET_REGISTER(rd));
                            R15 += 2;
                            break;
                        case 0x1: /* STRH Rd, [Rn, Rm] */
                            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
                            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
                            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
                            addr = GET_REGISTER(rn) + GET_REGISTER(rm);
                            WRITE16(addr, GET_REGISTER(rd));
                            R15 += 2;
                            break;
                        case 0x2: /* STRB Rd, [Rn, Rm] */
                            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
                            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
                            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
                            addr = GET_REGISTER(rn) + GET_REGISTER(rm);
                            WRITE8(addr, GET_REGISTER(rd));
                            R15 += 2;
                            break;
                        case 0x3: /* LDSB Rd, [Rn, Rm] todo, add dasm */
                            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
                            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
                            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
                            addr = GET_REGISTER(rn) + GET_REGISTER(rm);
                            op2 = READ8(addr);
                            if (op2 & 0x00000080)
                            {
                                op2 |= 0xffffff00;
                            }
                            SET_REGISTER(rd, op2);
                            R15 += 2;
                            break;
                        case 0x4: /* LDR Rd, [Rn, Rm] */
                            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
                            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
                            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
                            addr = GET_REGISTER(rn) + GET_REGISTER(rm);
                            op2 = READ32(addr);
                            SET_REGISTER(rd, op2);
                            R15 += 2;
                            break;
                        case 0x5: /* LDRH Rd, [Rn, Rm] */
                            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
                            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
                            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
                            addr = GET_REGISTER(rn) + GET_REGISTER(rm);
                            op2 = READ16(addr);
                            SET_REGISTER(rd, op2);
                            R15 += 2;
                            break;
                        case 0x6: /* LDRB Rd, [Rn, Rm] */
                            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
                            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
                            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
                            addr = GET_REGISTER(rn) + GET_REGISTER(rm);
                            op2 = READ8(addr);
                            SET_REGISTER(rd, op2);
                            R15 += 2;
                            break;
                        case 0x7: /* LDSH Rd, [Rn, Rm] */
                            rm = (insn & THUMB_GROUP5_RM) >> THUMB_GROUP5_RM_SHIFT;
                            rn = (insn & THUMB_GROUP5_RN) >> THUMB_GROUP5_RN_SHIFT;
                            rd = (insn & THUMB_GROUP5_RD) >> THUMB_GROUP5_RD_SHIFT;
                            addr = GET_REGISTER(rn) + GET_REGISTER(rm);
                            op2 = READ16(addr);
                            if (op2 & 0x00008000)
                            {
                                op2 |= 0xffff0000;
                            }
                            SET_REGISTER(rd, op2);
                            R15 += 2;
                            break;
                        default:
                            fatalerror("%08x: G5 Undefined Thumb instruction: %04x\n", pc, insn);
                            R15 += 2;
                            break;
                    }
                    break;
                case 0x6: /* Word Store w/ Immediate Offset */
                    if (insn & THUMB_LSOP_L) /* Load */
                    {
                        rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                        rd = insn & THUMB_ADDSUB_RD;
                        offs = ((insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
                        SET_REGISTER(rd, READ32(GET_REGISTER(rn) + offs)); // fix
                        R15 += 2;
                    }
                    else /* Store */
                    {
                        rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                        rd = insn & THUMB_ADDSUB_RD;
                        offs = ((insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT) << 2;
                        WRITE32(GET_REGISTER(rn) + offs, GET_REGISTER(rd));
                        R15 += 2;
                    }
                    break;
                case 0x7: /* Byte Store w/ Immeidate Offset */
                    if (insn & THUMB_LSOP_L) /* Load */
                    {
                        rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                        rd = insn & THUMB_ADDSUB_RD;
                        offs = (insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
                        SET_REGISTER(rd, READ8(GET_REGISTER(rn) + offs));
                        R15 += 2;
                    }
                    else /* Store */
                    {
                        rn = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                        rd = insn & THUMB_ADDSUB_RD;
                        offs = (insn & THUMB_LSOP_OFFS) >> THUMB_LSOP_OFFS_SHIFT;
                        WRITE8(GET_REGISTER(rn) + offs, GET_REGISTER(rd));
                        R15 += 2;
                    }
                    break;
                case 0x8: /* Load/Store Halfword */
                    if (insn & THUMB_HALFOP_L) /* Load */
                    {
                        imm = (insn & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
                        rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                        rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                        SET_REGISTER(rd, READ16(GET_REGISTER(rs) + (imm << 1)));
                        R15 += 2;
                    }
                    else /* Store */
                    {
                        imm = (insn & THUMB_HALFOP_OFFS) >> THUMB_HALFOP_OFFS_SHIFT;
                        rs = (insn & THUMB_ADDSUB_RS) >> THUMB_ADDSUB_RS_SHIFT;
                        rd = (insn & THUMB_ADDSUB_RD) >> THUMB_ADDSUB_RD_SHIFT;
                        WRITE16(GET_REGISTER(rs) + (imm << 1), GET_REGISTER(rd));
                        R15 += 2;
                    }
                    break;
                case 0x9: /* Stack-Relative Load/Store */
                    if (insn & THUMB_STACKOP_L)
                    {
                        rd = (insn & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
                        offs = (UINT8)(insn & THUMB_INSN_IMM);
                        readword = READ32(GET_REGISTER(13) + ((UINT32)offs << 2));
                        SET_REGISTER(rd, readword);
                        R15 += 2;
                    }
                    else
                    {
                        rd = (insn & THUMB_STACKOP_RD) >> THUMB_STACKOP_RD_SHIFT;
                        offs = (UINT8)(insn & THUMB_INSN_IMM);
                        WRITE32(GET_REGISTER(13) + ((UINT32)offs << 2), GET_REGISTER(rd));
                        R15 += 2;
                    }
                    break;
                case 0xa: /* Get relative address */
                    if (insn & THUMB_RELADDR_SP) /* ADD Rd, SP, #nn */
                    {
                        rd = (insn & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
                        offs = (UINT8)(insn & THUMB_INSN_IMM) << 2;
                        SET_REGISTER(rd, GET_REGISTER(13) + offs);
                        R15 += 2;
                    }
                    else /* ADD Rd, PC, #nn */
                    {
                        rd = (insn & THUMB_RELADDR_RD) >> THUMB_RELADDR_RD_SHIFT;
                        offs = (UINT8)(insn & THUMB_INSN_IMM) << 2;
                        SET_REGISTER(rd, ((R15 + 4) & ~2) + offs);
                        R15 += 2;
                    }
                    break;
                case 0xb: /* Stack-Related Opcodes */
                    switch ((insn & THUMB_STACKOP_TYPE) >> THUMB_STACKOP_TYPE_SHIFT)
                    {
                        case 0x0: /* ADD SP, #imm */
                            addr = (insn & THUMB_INSN_IMM);
                            addr &= ~THUMB_INSN_IMM_S;
                            SET_REGISTER(13, GET_REGISTER(13) + ((insn & THUMB_INSN_IMM_S) ? -(addr << 2) : (addr << 2)));
                            R15 += 2;
                            break;
                        case 0x4: /* PUSH {Rlist} */
                            for (offs = 7; offs >= 0; offs--)
                            {
                                if (insn & (1 << offs))
                                {
                                    SET_REGISTER(13, GET_REGISTER(13) - 4);
                                    WRITE32(GET_REGISTER(13), GET_REGISTER(offs));
                                }
                            }
                            R15 += 2;
                            break;
                        case 0x5: /* PUSH {Rlist}{LR} */
                            SET_REGISTER(13, GET_REGISTER(13) - 4);
                            WRITE32(GET_REGISTER(13), GET_REGISTER(14));
                            for (offs = 7; offs >= 0; offs--)
                            {
                                if (insn & (1 << offs))
                                {
                                    SET_REGISTER(13, GET_REGISTER(13) - 4);
                                    WRITE32(GET_REGISTER(13), GET_REGISTER(offs));
                                }
                            }
                            R15 += 2;
                            break;
                        case 0xc: /* POP {Rlist} */
                            for (offs = 0; offs < 8; offs++)
                            {
                                if (insn & (1 << offs))
                                {
                                    SET_REGISTER(offs, READ32(GET_REGISTER(13)));
                                    SET_REGISTER(13, GET_REGISTER(13) + 4);
                                }
                            }
                            R15 += 2;
                            break;
                        case 0xd: /* POP {Rlist}{PC} */
                            for (offs = 0; offs < 8; offs++)
                            {
                                if (insn & (1 << offs))
                                {
                                    SET_REGISTER(offs, READ32(GET_REGISTER(13)));
                                    SET_REGISTER(13, GET_REGISTER(13) + 4);
                                }
                            }
                            R15 = READ32(GET_REGISTER(13)) & ~1;
                            SET_REGISTER(13, GET_REGISTER(13) + 4);
                            break;
                        default:
                            fatalerror("%08x: Gb Undefined Thumb instruction: %04x\n", pc, insn);
                            R15 += 2;
                            break;
                    }
                    break;
                case 0xc: /* Multiple Load/Store */
                    {
                        UINT32 ld_st_address;

                        rd = (insn & THUMB_MULTLS_BASE) >> THUMB_MULTLS_BASE_SHIFT;
                        ld_st_address = GET_REGISTER(rd) & 0xfffffffc;

                        if (insn & THUMB_MULTLS) /* Load */
                        {
                            int rd_in_list;

                            rd_in_list = insn & (1 << rd);
                            for (offs = 0; offs < 8; offs++)
                            {
                                if (insn & (1 << offs))
                                {
                                    SET_REGISTER(offs, READ32(ld_st_address));
                                    ld_st_address += 4;
                                }
                            }
                            if (!rd_in_list)
                                SET_REGISTER(rd, ld_st_address);
                            R15 += 2;
                        }
                        else /* Store */
                        {
                            for (offs = 0; offs < 8; offs++)
                            {
                                if (insn & (1 << offs))
                                {
                                    WRITE32(ld_st_address, GET_REGISTER(offs));
                                    ld_st_address += 4;
                                }
                            }
                            SET_REGISTER(rd, ld_st_address);
                            R15 += 2;
                        }
                    }
                    break;
                case 0xd: /* Conditional Branch */
                    offs = (INT8)(insn & THUMB_INSN_IMM);
                    switch ((insn & THUMB_COND_TYPE) >> THUMB_COND_TYPE_SHIFT)
                    {
                        case COND_EQ:
                            if (Z_IS_SET(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_NE:
                            if (Z_IS_CLEAR(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_CS:
                            if (C_IS_SET(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_CC:
                            if (C_IS_CLEAR(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_MI:
                            if (N_IS_SET(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_PL:
                            if (N_IS_CLEAR(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_VS:
                            if (V_IS_SET(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_VC:
                            if (V_IS_CLEAR(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_HI:
                            if (C_IS_SET(GET_CPSR) && Z_IS_CLEAR(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_LS:
                            if (C_IS_CLEAR(GET_CPSR) || Z_IS_SET(GET_CPSR))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_GE:
                            if (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_LT:
                            if (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_GT:
                            if (Z_IS_CLEAR(GET_CPSR) && !(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_LE:
                            if (Z_IS_SET(GET_CPSR) || !(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK))
                            {
                                R15 += 4 + (offs << 1);
                            }
                            else
                            {
                                R15 += 2;
                            }
                            break;
                        case COND_AL:
                            fatalerror("%08x: Undefined Thumb instruction: %04x (ARM9 reserved)\n", pc, insn);
                            R15 += 2;
                            break;
                        case COND_NV:   // SWI (this is sort of a "hole" in the opcode encoding)
                            ARM7.pendingSwi = 1;
                            ARM7_CHECKIRQ;
                            break;
                    }
                    break;
                case 0xe: /* B #offs */
                    if (insn & THUMB_BLOP_LO)
                    {
                        addr = GET_REGISTER(14);
                        addr += (insn & THUMB_BLOP_OFFS) << 1;
                        addr &= 0xfffffffc;
                        SET_REGISTER(14, (R15 + 4) | 1);
                        R15 = addr;
                    }
                    else
                    {
                        offs = (insn & THUMB_BRANCH_OFFS) << 1;
                        if (offs & 0x00000800)
                        {
                            offs |= 0xfffff800;
                        }
                        R15 += 4 + offs;
                    }
                    break;
                case 0xf: /* BL */
                    if (insn & THUMB_BLOP_LO)
                    {
                        addr = GET_REGISTER(14);
                        addr += (insn & THUMB_BLOP_OFFS) << 1;
                        SET_REGISTER(14, (R15 + 2) | 1);
                        R15 = addr;
                        //R15 += 2;
                    }
                    else
                    {
                        addr = (insn & THUMB_BLOP_OFFS) << 12;
                        if (addr & (1 << 22))
                        {
                            addr |= 0xff800000;
                        }
                        addr += R15 + 4;
                        SET_REGISTER(14, addr);
                        R15 += 2;
                    }
                    break;
                default:
                    fatalerror("%08x: Undefined Thumb instruction: %04x\n", pc, insn);
                    R15 += 2;
                    break;
            }
        }
        else
        {

            /* load 32 bit instruction */
            pc = R15;
            insn = cpu_readop32(pc);

            /* process condition codes for this instruction */
            switch (insn >> INSN_COND_SHIFT)
            {
            case COND_EQ:
                if (Z_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_NE:
                if (Z_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_CS:
                if (C_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_CC:
                if (C_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_MI:
                if (N_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_PL:
                if (N_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_VS:
                if (V_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_VC:
                if (V_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_HI:
                if (C_IS_CLEAR(GET_CPSR) || Z_IS_SET(GET_CPSR))
                    goto L_Next;
                break;
            case COND_LS:
                if (C_IS_SET(GET_CPSR) && Z_IS_CLEAR(GET_CPSR))
                    goto L_Next;
                break;
            case COND_GE:
                if (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK)) /* Use x ^ (x >> ...) method */
                    goto L_Next;
                break;
            case COND_LT:
                if (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK))
                    goto L_Next;
                break;
            case COND_GT:
                if (Z_IS_SET(GET_CPSR) || (!(GET_CPSR & N_MASK) != !(GET_CPSR & V_MASK)))
                    goto L_Next;
                break;
            case COND_LE:
                if (Z_IS_CLEAR(GET_CPSR) && (!(GET_CPSR & N_MASK) == !(GET_CPSR & V_MASK)))
                  goto L_Next;
                break;
            case COND_NV:
                goto L_Next;
            }
            /*******************************************************************/
            /* If we got here - condition satisfied, so decode the instruction */
            /*******************************************************************/
            switch ((insn & 0xF000000) >> 24)
            {
                case 0:
                case 1:
                case 2:
                case 3:
                    /* Branch and Exchange (BX) */
                    if ((insn & 0x0ffffff0) == 0x012fff10)     // bits 27-4 == 000100101111111111110001
                    {
                        R15 = GET_REGISTER(insn & 0x0f);
                        // If new PC address has A0 set, switch to Thumb mode
                        if (R15 & 1) {
                            SET_CPSR(GET_CPSR|T_MASK);
                            R15--;
                        }
                    }
                    else
                    /* Multiply OR Swap OR Half Word Data Transfer */
                    if ((insn & 0x0e000000) == 0 && (insn & 0x80) && (insn & 0x10))  // bits 27-25=000 bit 7=1 bit 4=1
                    {
                        /* Half Word Data Transfer */
                        if (insn & 0x60)         // bits = 6-5 != 00
                        {
                            HandleHalfWordDT(insn);
                        }
                        else
                        /* Swap */
                        if (insn & 0x01000000)   // bit 24 = 1
                        {
                            HandleSwap(insn);
                        }
                        /* Multiply Or Multiply Long */
                        else
                        {
                            /* multiply long */
                            if (insn & 0x800000) // Bit 23 = 1 for Multiply Long
                            {
                                /* Signed? */
                                if (insn & 0x00400000)
                                    HandleSMulLong(insn);
                                else
                                    HandleUMulLong(insn);
                            }
                            /* multiply */
                            else
                            {
                                HandleMul(insn);
                            }
                            R15 += 4;
                        }
                    }
                    else
                    /* Data Processing OR PSR Transfer */
                    if ((insn & 0x0c000000) == 0)   // bits 27-26 == 00 - This check can only exist properly after Multiplication check above
                    {
                        /* PSR Transfer (MRS & MSR) */
                        if (((insn & 0x00100000) == 0) && ((insn & 0x01800000) == 0x01000000)) // S bit must be clear, and bit 24,23 = 10
                        {
                            HandlePSRTransfer(insn);
                            ARM7_ICOUNT += 2;       // PSR only takes 1 - S Cycle, so we add + 2, since at end, we -3..
                            R15 += 4;
                        }
                        /* Data Processing */
                        else
                        {
                            HandleALU(insn);
                        }
                    }
                    break;
                /* Data Transfer - Single Data Access */
                case 4:
                case 5:
                case 6:
                case 7:
                    HandleMemSingle(insn);
                    R15 += 4;
                    break;
                /* Block Data Transfer/Access */
                case 8:
                case 9:
                    HandleMemBlock(insn);
                    R15 += 4;
                    break;
                /* Branch or Branch & Link */
                case 0xa:
                case 0xb:
                    HandleBranch(insn);
                    break;
                /* Co-Processor Data Transfer */
                case 0xc:
                case 0xd:
                    HandleCoProcDT(insn);
                    R15 += 4;
                    break;
                /* Co-Processor Data Operation or Register Transfer */
                case 0xe:
                    if (insn & 0x10)
                        HandleCoProcRT(insn);
                    else
                        HandleCoProcDO(insn);
                    R15 += 4;
                    break;
                /* Software Interrupt */
                case 0x0f:
                    ARM7.pendingSwi = 1;
                    ARM7_CHECKIRQ;
                    //couldn't find any cycle counts for SWI
                    break;
                /* Undefined */
                default:
                    ARM7.pendingSwi = 1;
                    ARM7_CHECKIRQ;
                    ARM7_ICOUNT -= 1;               //undefined takes 4 cycles (page 77)
                    LOG(("%08x:  Undefined instruction\n",pc-4));
                    L_Next:
                        R15 += 4;
                        ARM7_ICOUNT +=2;    //Any unexecuted instruction only takes 1 cycle (page 193)
            }
        }

        ARM7_CHECKIRQ;

        /* All instructions remove 3 cycles.. Others taking less / more will have adjusted this # prior to here */
        ARM7_ICOUNT -= 3;
    	total_cycles = curr_cycles + cycles - ARM7_ICOUNT;
    } while (ARM7_ICOUNT > 0);

    return cycles - ARM7_ICOUNT;
}
