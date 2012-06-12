/*  Copyright 2003-2004 Stephane Dallongeville

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*********************************************************************************
 * GEN68K.C :
 *
 * C68K generator source file
 *
 ********************************************************************************/

#include <stdio.h>
#include <inttypes.h>

#include "c68k.h"
#include "gen68k.h"

#ifdef C68K_GEN

#include "gen68k.inc"

// to do :
// need accurate cycles calculations in MUL and DIV instruction
// some bugs to fix

// opcode generation function
//////////////////////////////

static void GenLogicI(char op)
{
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES | GEN_SRC);
    else
        start_all(GEN_ADR | GEN_RES | GEN_SRC);

    add_CCnt(8);

    if (current_ea != EA_DREG) current_cycle += 4;
    switch (current_size)
    {
        case SIZE_BYTE:
            wf_op("\tsrc = FETCH_BYTE;\n");
            wf_op("\tPC += 2;\n");
            break;

        case SIZE_WORD:
            wf_op("\tsrc = FETCH_WORD;\n");
            wf_op("\tPC += 2;\n");
            break;

        case SIZE_LONG:
            wf_op("\tsrc = FETCH_LONG;\n");
            wf_op("\tPC += 4;\n");
            current_cycle += 8;
            break;
    }

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // op
    wf_op("\tres %c= src;\n", op);
    // flag calculation
    set_logic_flag();
    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(0); //8);
}

static void GenLogicICCR(char op)
{
    // generate jump table & opcode declaration
    start_all(GEN_RES);

    wf_op("\tres = FETCH_BYTE & C68K_CCR_MASK;\n");
    wf_op("\tPC += 2;\n");
    wf_op("\tres %c= GET_CCR;\n", op);
    wf_op("\tSET_CCR(res)\n");

    terminate_op(20);
}

static void GenLogicISR(char op)
{
    // generate jump table & opcode declaration
    start_all(GEN_RES);
    
    wf_op("\tif (CPU->flag_S)\n");
    wf_op("\t{\n");
    wf_op("\t\tres = FETCH_WORD & C68K_SR_MASK;\n");
    wf_op("\t\tPC += 2;\n");
    wf_op("\t\tres %c= GET_SR;\n", op);
    wf_op("\t\tSET_SR(res)\n");
    if (op != '|')
    {
        wf_op("\t\tif (!CPU->flag_S)\n");
        wf_op("\t\t{\n");
        wf_op("\t\t\tres = CPU->A[7];\n");
        wf_op("\t\t\tCPU->A[7] = CPU->USP;\n");
        wf_op("\t\t\tCPU->USP = res;\n");
        wf_op("\t\t}\n");
    }
    wf_op("\t}\n");
    wf_op("\telse\n");
    wf_op("\t{\n");
        gen_privilege_exception("\t\t");
    wf_op("\t}\n");

    // check for interrupt
    fterminate_op(20);
}

static void GenORI()
{
    GenLogicI('|');
}

static void GenORICCR()
{
    GenLogicICCR('|');
}

static void GenORISR()
{
    GenLogicISR('|');
}

static void GenANDI()
{
    GenLogicI('&');
}

static void GenANDICCR()
{
    GenLogicICCR('&');
}

static void GenANDISR()
{
    GenLogicISR('&');
}

static void GenEORI()
{
    GenLogicI('^');
}

static void GenEORICCR()
{
    GenLogicICCR('^');
}

static void GenEORISR()
{
    GenLogicISR('^');
}

static void GenArithI(char op)
{
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES | GEN_SRC | GEN_DST);
    else
        start_all(GEN_ALL);

    add_CCnt(8);

    if ((op != ' ') && (current_ea != EA_DREG)) current_cycle += 4;
    switch (current_size)
    {
        case SIZE_BYTE:
            wf_op("\tsrc = FETCH_BYTE;\n");
            wf_op("\tPC += 2;\n");
            break;

        case SIZE_WORD:
            wf_op("\tsrc = FETCH_WORD;\n");
            wf_op("\tPC += 2;\n");
            break;

        case SIZE_LONG:
            wf_op("\tsrc = FETCH_LONG;\n");
            wf_op("\tPC += 4;\n");
            if (op == ' ')
            {
                if (current_ea == EA_DREG) current_cycle += 6;
                else current_cycle += 4;
            } else current_cycle += 8;
            break;
    }

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_dst(current_ea, current_op->reg_sft);
    if (op == ' ')
    {
        // op
        wf_op("\tres = dst - src;\n");
        // flag calculation
        set_cmp_flag();
    }
    else
    {
        // op
        wf_op("\tres = dst %c src;\n", op);
        // flag calculation
        if (op == '+') set_add_flag();
        else set_sub_flag();
        // write
        _ea_write(current_ea, current_op->reg_sft);
    }

    terminate_op(0); //8);
}

static void GenSUBI()
{
    GenArithI('-');
}

static void GenADDI()
{
    GenArithI('+');
}

static void GenCMPI()
{
    GenArithI(' ');
}

static void GenBitsOp(char op, u32 dyn)
{
    // generate jump table & opcode declaration
    if (dyn) current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES | GEN_SRC);
    else
        start_all(GEN_ADR | GEN_RES | GEN_SRC);

    add_CCnt(4);

    if (current_ea == EA_DREG)
    {
        set_current_size(SIZE_LONG);
        if ((op == 'c') || (op == ' ')) current_cycle += 2;
    }
    else set_current_size(SIZE_BYTE);

    // get shift value in src
    if (dyn)
    {
        _ea_calc(current_ea2, current_op->reg2_sft);
        _ea_read_src(current_ea2, current_op->reg2_sft);
    }
    else
    {
        wf_op("\tsrc = FETCH_BYTE;\n");
        wf_op("\tPC += 2;\n");
        current_cycle += 4;
    }
    wf_op("\tsrc = 1 << (src & %d);\n", current_sft_mask);
    
    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // flag calculation
    wf_op("\tCPU->flag_notZ = res & src;\n");
    // op
    switch(op)
    {
        case 'c':
            wf_op("\tres &= ~src;\n");
            break;

        case 'g':
            wf_op("\tres ^= src;\n");
            break;

        case 's':
            wf_op("\tres |= src;\n");
            break;
    }
    // write
    if (op != ' ')
    {
        _ea_write(current_ea, current_op->reg_sft);
        current_cycle += 4;
    }

    terminate_op(0); //4);
}

static void GenBTSTn()
{
    GenBitsOp(' ', 0);
}

static void GenBCHGn()
{
    GenBitsOp('g', 0);
}

static void GenBCLRn()
{
    GenBitsOp('c', 0);
}

static void GenBSETn()
{
    GenBitsOp('s', 0);
}

static void GenBTST()
{
    GenBitsOp(' ', 1);
}

static void GenBCHG()
{
    GenBitsOp('g', 1);
}

static void GenBCLR()
{
    GenBitsOp('c', 1);
}

static void GenBSET()
{
    GenBitsOp('s', 1);
}

static void GenMOVEPWaD()
{
    // generate jump table & opcode declaration
    current_ea = EA_D16A;
    current_ea2 = EA_DREG;
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    set_current_size(SIZE_BYTE);
    _ea_calc(current_ea, current_op->reg_sft);
    mem_op("\tREAD_BYTE_F(adr + 0, res)\n");
    mem_op("\tREAD_BYTE_F(adr + 2, src)\n");
    // write
    wf_op("\t*(WORD_OFF + (u16*)(&CPU->D[(Opcode >> %d) & 7])) = (res << 8) | src;\n", current_op->reg2_sft);

    terminate_op(16);
}

static void GenMOVEPLaD()
{
    // generate jump table & opcode declaration
    current_ea = EA_D16A;
    current_ea2 = EA_DREG;
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    set_current_size(SIZE_BYTE);
    _ea_calc(EA_D16A, current_op->reg_sft);
    mem_op("\tREAD_BYTE_F(adr, res)\n");
    wf_op("\tres <<= 24;\n");
    wf_op("\tadr += 2;\n");
    mem_op("\tREAD_BYTE_F(adr, src)\n");
    wf_op("\tres |= src << 16;\n");
    wf_op("\tadr += 2;\n");
    mem_op("\tREAD_BYTE_F(adr, src)\n");
    wf_op("\tres |= src << 8;\n");
    wf_op("\tadr += 2;\n");
    mem_op("\tREAD_BYTE_F(adr, src)\n");
    // write
    wf_op("\tCPU->D[(Opcode >> %d) & 7] = res | src;\n", current_op->reg2_sft);

    terminate_op(24);
}

static void GenMOVEPWDa()
{
    // generate jump table & opcode declaration
    current_ea = EA_D16A;
    current_ea2 = EA_DREG;
    start_all(GEN_ADR | GEN_RES);

    // read
    set_current_size(SIZE_LONG);
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read(current_ea2, current_op->reg2_sft);
    // write
    set_current_size(SIZE_BYTE);
    _ea_calc(current_ea, current_op->reg_sft);
    mem_op("\tWRITE_BYTE_F(adr + 0, res >> 8)\n");
    mem_op("\tWRITE_BYTE_F(adr + 2, res >> 0)\n");

    terminate_op(16);
}

static void GenMOVEPLDa()
{
    // generate jump table & opcode declaration
    current_ea = EA_D16A;
    current_ea2 = EA_DREG;
    start_all(GEN_ADR | GEN_RES);

    // read
    set_current_size(SIZE_LONG);
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read(current_ea2, current_op->reg2_sft);
    // write
    set_current_size(SIZE_BYTE);
    _ea_calc(current_ea, current_op->reg_sft);
    mem_op("\tWRITE_BYTE_F(adr, res >> 24)\n");
    wf_op("\tadr += 2;\n");
    mem_op("\tWRITE_BYTE_F(adr, res >> 16)\n");
    wf_op("\tadr += 2;\n");
    mem_op("\tWRITE_BYTE_F(adr, res >> 8)\n");
    wf_op("\tadr += 2;\n");
    mem_op("\tWRITE_BYTE_F(adr, res >> 0)\n");

    terminate_op(24);
}

static void GenMOVE(u32 size)
{
    set_current_size(size);
    
    // generate jump table & opcode declaration
    if (((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM)) &&
        ((current_ea2 == EA_AREG) || (current_ea2 == EA_DREG) || (current_ea2 == EA_IMM)))
        start_all(GEN_RES);
    else
        start_all(GEN_ADR | GEN_RES);

    add_CCnt(4);
    
    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // flag calculation
    set_logic_flag();
    if ((current_ea2 == EA_ADEC) || (current_ea2 == EA_ADEC7)) current_cycle -= 2;
    // write
    _ea_calc(current_ea2, current_op->reg2_sft);

    if ((current_ea2 == EA_ADEC) || (current_ea2 == EA_ADEC7))
     _ea_write_sarsie(current_ea2, current_op->reg2_sft, 1);
    else
     _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(0); //4);
}

static void GenMOVEB()
{
    GenMOVE(SIZE_BYTE);
}

static void GenMOVEW()
{
    GenMOVE(SIZE_WORD);
}

static void GenMOVEL()
{
    GenMOVE(SIZE_LONG);
}

static void GenMOVEA(u32 size)
{
    set_current_size(size);

    // generate jump table & opcode declaration
    current_ea2 = EA_AREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES);
    else
        start_all(GEN_ADR | GEN_RES);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_sx(current_ea, current_op->reg_sft);
    // write (dst = Ax)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(4);
}

static void GenMOVEAW()
{
    GenMOVEA(SIZE_WORD);
}

static void GenMOVEAL()
{
    GenMOVEA(SIZE_LONG);
}

static void GenMOVEQ()
{
    u32 base = get_current_opcode_base();

    // generate jump table
    current_ea = EA_DREG;
    gen_opjumptable_ext(base, 0x00, 0xFF, 1, base);

    // generate label & declarations
    start_op(base, GEN_RES);

    // read
    set_current_size(SIZE_BYTE);
	wf_op("\tres = (s32)(s8)Opcode;\n");
    // fast flag calculation for moveQ
	wf_op("\tCPU->flag_C = CPU->flag_V = 0;\n");
	wf_op("\tCPU->flag_N = CPU->flag_notZ = res;\n");
    // write
    set_current_size(SIZE_LONG);
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(4);
}

static void GenSingle(char op)
{
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM)) {
        if (op == 'c')
            start_all(GEN_RES);
        else
            start_all(GEN_RES | GEN_SRC);
    } else {
        if (op == 'c')
            start_all(GEN_ADR | GEN_RES);
        else
            start_all(GEN_ADR | GEN_RES | GEN_SRC);
    }

    if (current_size == SIZE_LONG) current_cycle = 6;
    else current_cycle= 4;
    if (is_ea_memory(current_ea)) current_cycle *= 2;
    
    // read
    _ea_calc(current_ea, current_op->reg_sft);
    if (op != 'c') _ea_read_src(current_ea, current_op->reg_sft);
    // op
    switch (op)
    {
        case 'x':   // negx
            wf_op("\tres = -src - ((CPU->flag_X >> 8) & 1);\n");
            break;
            
        case 'g':   // neg
            wf_op("\tres = -src;\n");
            break;

        case 'n':   // not
            wf_op("\tres = ~src;\n");
            break;

        case 'c':   // clr
            wf_op("\tres = 0;\n");
            break;
    }
    // flag calculation
    switch (op)
    {
        case 'x':   // negx
            set_negx_flag();
            break;

        case 'g':   // neg
            set_neg_flag();
            break;

        case 'n':   // not
            set_logicl_flag();
            break;

        case 'c':   // clr
            wf_op("\tCPU->flag_N = CPU->flag_notZ = CPU->flag_V = CPU->flag_C = 0;\n");
            break;
    }
    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(0);
}

static void GenCLR()
{
    GenSingle('c');
}

static void GenNEGX()
{
    GenSingle('x');
}

static void GenNEG()
{
    GenSingle('g');
}

static void GenNOT()
{
    GenSingle('n');
}

static void GenMOVESRa()
{
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES);
    else
        start_all(GEN_ADR | GEN_RES);

    // read
    wf_op("\tres = GET_SR;\n");
    // write
    set_current_size(SIZE_WORD);
    if (is_ea_memory(current_ea)) current_cycle += 2;
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenMOVEaSR()
{
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES);
    else
        start_all(GEN_ADR | GEN_RES);

    wf_op("\tif (CPU->flag_S)\n");
    wf_op("\t{\n");
        // read
        set_current_size(SIZE_WORD);
        _ea_calc(current_ea, current_op->reg_sft);
        _ea_read(current_ea, current_op->reg_sft);
        wf_op("\t\tSET_SR(res)\n");
        wf_op("\t\tif (!CPU->flag_S)\n");
        wf_op("\t\t{\n");
            wf_op("\t\t\tres = CPU->A[7];\n");
            wf_op("\t\t\tCPU->A[7] = CPU->USP;\n");
            wf_op("\t\t\tCPU->USP = res;\n");
        wf_op("\t\t}\n");
    wf_op("\t}\n");
    wf_op("\telse\n");
    wf_op("\t{\n");
        gen_privilege_exception("\t\t");
    wf_op("\t}\n");

    // force terminaison to check for interrupt
    fterminate_op(12);
}

static void GenMOVEaCCR()
{
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES);
    else
        start_all(GEN_ADR | GEN_RES);

    // read
    set_current_size(SIZE_WORD);
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // write
    wf_op("\tSET_CCR(res)\n");

    terminate_op(12);
}

static void GenMOVEAUSP()
{
    current_ea = EA_AREG;

    // generate jump table & opcode declaration
    start_all(GEN_RES);

    wf_op("\tif (!CPU->flag_S)\n");
    wf_op("\t{\n");
        gen_privilege_exception("\t\t");
        quick_terminate_op(4);
    wf_op("\t}\n");

    // read
    set_current_size(SIZE_LONG);
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // write
     wf_op("\tCPU->USP = res;\n");

    terminate_op(4);
}

static void GenMOVEUSPA()
{
    current_ea = EA_AREG;
    
    // generate jump table & opcode declaration
    start_all(GEN_RES);

    wf_op("\tif (!CPU->flag_S)\n");
    wf_op("\t{\n");
        gen_privilege_exception("\t\t");
        quick_terminate_op(4);
    wf_op("\t}\n");

    // read
     wf_op("\tres = CPU->USP;\n");
    // write
    set_current_size(SIZE_LONG);
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(4);
}

static void GenPEA()
{
    set_current_size(SIZE_LONG);
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(0);
    else
        start_all(GEN_ADR);
    
    _ea_calc_free(current_ea, current_op->reg_sft);
    mem_op("\tPUSH_32_F(adr)\n");
    
    terminate_op(lea_pea_cycle_table[current_ea] + 12);
}

static void GenSWAP()
{
    current_ea = EA_DREG;
    set_current_size(SIZE_LONG);
    // generate jump table & opcode declaration
    start_all(GEN_RES);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // op
    wf_op("\tres = (res >> 16) | (res << 16);\n");
    // flag calculation
    set_logic_flag();
    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(4);
}

static void GenMOVEMaR()
{
    // generate jump table & opcode declaration
    start_all(GEN_ALL);
    
    // get register mask
    wf_op("\tres = FETCH_WORD;\n");
    wf_op("\tPC += 2;\n");
    // get adr
    if (current_ea == EA_AINC) wf_op("\tadr = CPU->A[(Opcode >> %d) & 7];\n", current_op->reg_sft);
    else if (current_ea == EA_AINC7) wf_op("\tadr = CPU->A[7];\n");
    else _ea_calc(current_ea, current_op->reg_sft);
    wf_op("\tsrc = 0;\n");

    wf_op("\tdst = adr;\n");
    
    wf_op("\tdo\n");
    wf_op("\t{\n");
    wf_op("\t\tif (res & 1)\n");
    wf_op("\t\t{\n");

    if (current_size == SIZE_WORD)
    {
        wf_op("\t\t\tREADSX_WOat_F(adr, CPU->DA[src])\n");
        wf_op("\t\t\tadr += 2;\n");
    }
    else
    {
        wf_op("\t\t\tREAD_LOat_F(adr, CPU->DA[src])\n");
        wf_op("\t\t\tadr += 4;\n");
    }
    wf_op("\t\t}\n");
    wf_op("\t\tsrc ++;\n");
    wf_op("\t} while (res >>= 1);\n");

    if (current_ea == EA_AINC) wf_op("\tCPU->A[(Opcode >> %d) & 7] = adr;\n", current_op->reg_sft);
    else if (current_ea == EA_AINC7) wf_op("\tCPU->A[7] = adr;\n");

    terminate_op(movem_cycle_table[current_ea] + 12);
}

static void GenMOVEMRa()
{
    // generate jump table & opcode declaration
    start_all(GEN_ALL);

    // get register mask
    wf_op("\tres = FETCH_WORD;\n");
    wf_op("\tPC += 2;\n");
    // get adr
    if (current_ea == EA_ADEC) wf_op("\tadr = CPU->A[(Opcode >> %d) & 7];\n", current_op->reg_sft);
    else if (current_ea == EA_ADEC7) wf_op("\tadr = CPU->A[7];\n");
    else _ea_calc(current_ea, current_op->reg_sft);
    if ((current_ea == EA_ADEC) || (current_ea == EA_ADEC7)) wf_op("\tsrc = 15;\n");
    else wf_op("\tsrc = 0;\n");

    wf_op("\tdst = adr;\n");
    
    wf_op("\tdo\n");
    wf_op("\t{\n");
    wf_op("\t\tif (res & 1)\n");
    wf_op("\t\t{\n");

    if (current_size == SIZE_WORD)
    {
        if ((current_ea == EA_ADEC) || (current_ea == EA_ADEC7)) wf_op("\t\t\tadr -= 2;\n");
        wf_op("\t\t\tWRITE_WOat_F(adr, CPU->DA[src])\n");
        if (!((current_ea == EA_ADEC) || (current_ea == EA_ADEC7))) wf_op("\t\t\tadr += 2;\n");
    }
    else
    {
        if ((current_ea == EA_ADEC) || (current_ea == EA_ADEC7))
        {
            wf_op("\t\t\tadr -= 4;\n");
            wf_op("\t\t\tWRITE_LOat_DEC_F(adr, CPU->DA[src])\n");
        }
        else
        {
            wf_op("\t\t\tWRITE_LOat_F(adr, CPU->DA[src])\n");
            wf_op("\t\t\tadr += 4;\n");
        }
    }
    wf_op("\t\t}\n");
    if ((current_ea == EA_ADEC) || (current_ea == EA_ADEC7)) wf_op("\t\tsrc --;\n");
    else wf_op("\t\tsrc ++;\n");
    wf_op("\t} while (res >>= 1);\n");

    if (current_ea == EA_ADEC) wf_op("\tCPU->A[(Opcode >> %d) & 7] = adr;\n", current_op->reg_sft);
    else if (current_ea == EA_ADEC7) wf_op("\tCPU->A[7] = adr;\n");

    terminate_op(movem_cycle_table[current_ea] + 8);
}

static void GenEXT()
{
    current_ea = EA_DREG;
    // generate jump table & opcode declaration
    start_all(GEN_RES);

    // read
    set_current_size(current_size - 1);
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_sx(current_ea, current_op->reg_sft);
    // flag calculation
    set_logic_flag();
    // write
    set_current_size(current_size + 1);
    _ea_write(current_ea, current_op->reg_sft);
    
    terminate_op(4);
}

static void GenTST()
{
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES);
    else
        start_all(GEN_ADR | GEN_RES);

    add_CCnt(4);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // flag calculation
    set_logic_flag();

    terminate_op(0); //4);
}

static void GenTAS()
{
    set_current_size(SIZE_BYTE);
    
    if (is_ea_memory(current_ea)) current_cycle += 6;

    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES);
    else
        start_all(GEN_ADR | GEN_RES);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // flag calculation
    set_logic_flag();

    if(current_ea >= EA_AIND)
    {
     wf_op("if(!CPU->TAS_Hack)\n{\n");
    }

    // flag calculation
    wf_op("\tres |= 0x80;\n");
    // write
    _ea_write(current_ea, current_op->reg_sft);

    if(current_ea >= EA_AIND)
    {
     wf_op("}\n");
    }

    terminate_op(4);
}

static void GenTRAP()
{
    u32 base;

    base = get_current_opcode_base();

    // generate jump table
    gen_opjumptable_ext(base, (0 << 0), (15 << 0), (1 << 0), base);

    // generate label & declarations
    start_op(base, GEN_RES);

    gen_exception("\t", "C68K_TRAP_BASE_EX + (Opcode & 0xF)");

    terminate_op(4);
}

static void GenTRAPV()
{
    // generate label & declarations
    start_all(GEN_RES);

    wf_op("\tif %s\n", get_cond_as_cond(COND_VS, 0));
    wf_op("\t{\n");
        gen_exception("\t\t", "C68K_TRAPV_EX");
    wf_op("\t}\n");

    terminate_op(4);
}

static void GenLINK()
{
    current_ea = EA_AREG;
    set_current_size(SIZE_LONG);
    // generate jump table & opcode declaration
    start_all(GEN_RES);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // push
    mem_op("\tPUSH_32_F(res)\n");
    wf_op("\tres = CPU->A[7];\n");
    // write
    _ea_write(current_ea, current_op->reg_sft);
    // update SP
    wf_op("\tCPU->A[7] += (s32)(s16)FETCH_WORD;\n");
    wf_op("\tPC += 2;\n");

    terminate_op(16);
}

static void GenLINKA7()
{
    current_ea = EA_AREG;
    set_current_size(SIZE_LONG);
    // generate jump table & opcode declaration
    start_all(0);

    // push A7
    wf_op("\tCPU->A[7] -= 4;\n");
    mem_op("\tWRITE_LONG_DEC_F(CPU->A[7], CPU->A[7])\n");
    // update A7
    wf_op("\tCPU->A[7] += (s32)(s16)FETCH_WORD;\n");
    wf_op("\tPC += 2;\n");

    terminate_op(16);
}

static void GenULNK()
{
    current_ea = EA_AREG;
    set_current_size(SIZE_LONG);
    // generate jump table & opcode declaration
    start_all(GEN_RES | GEN_SRC);
    
    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // pop
    wf_op("\tCPU->A[7] = src + 4;\n");
    mem_op("\tREAD_LONG_F(src, res)\n");
    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(12);
}

static void GenULNKA7()
{
    current_ea = EA_AREG;
    set_current_size(SIZE_LONG);
    // generate jump table & opcode declaration
    start_all(0);

    mem_op("\tREAD_LONG_F(CPU->A[7], CPU->A[7])\n");
    
    terminate_op(12);
}

static void GenRESET()
{
    // generate jump table & opcode declaration
    start_all(GEN_RES);

    wf_op("\tif (!CPU->flag_S)\n");
    wf_op("\t{\n");
        gen_privilege_exception("\t\t");
        quick_terminate_op(4);
    wf_op("\t}\n");
    
    // Reset callback function
    mem_op("\tCPU->Reset_CallBack();\n");

    terminate_op(132);
}

static void GenLEA()
{
    current_ea2 = EA_AREG;
    set_current_size(SIZE_LONG);
    // generate jump table & opcode declaration
    start_all(GEN_ADR | GEN_RES);

    _ea_calc_free(current_ea, current_op->reg_sft);
    wf_op("\tres = adr;\n");
    current_cycle = lea_pea_cycle_table[current_ea];
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(4);
}

static void GenNOP()
{
    start_all(0);
    terminate_op(4);
}

static void Gen1010()
{
    u32 base;

    base = get_current_opcode_base();

     // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0FFF, 0x1, base);

    // generate label & declarations
    start_op(base, GEN_RES);

    wf_op("\tPC -= 2;\n");
    gen_exception("\t", "C68K_1010_EX");

    terminate_op(4);
}

static void Gen1111()
{
    u32 base;

    base = get_current_opcode_base();

    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0FFF, 0x1, base);

    // generate label & declarations
    start_op(base, GEN_RES);

    wf_op("\tPC -= 2;\n");
    gen_exception("\t", "C68K_1111_EX");

    terminate_op(4);
}



static void GenILLEGAL()
{
    start_all(GEN_RES);

    gen_exception("\t\t", "C68K_ILLEGAL_INSTRUCTION_EX");
    
    terminate_op(4);
}

static void GenCHK()
{
    current_ea2 = EA_DREG;
    set_current_size(SIZE_WORD);
    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES | GEN_SRC);
    else
        start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read Src
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read Dx
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read(current_ea2, current_op->reg2_sft);

	wf_op("\tif (((s32)res < 0) || (res > src))\n");
    wf_op("\t{\n");
        wf_op("\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
        gen_exception("\t\t", "C68K_CHK_EX");
    wf_op("\t}\n");

    terminate_op(10);
}

static void GenSTOP()
{
    // generate jump table & opcode declaration
    start_all(GEN_RES);

    wf_op("\tif (!CPU->flag_S)\n");
    wf_op("\t{\n");
        wf_op("\t\tPC += 2;\n");
        gen_privilege_exception("\t\t");
        quick_terminate_op(4);
    wf_op("\t}\n");

    // read & set SR
    wf_op("\tres = FETCH_WORD & C68K_SR_MASK;\n");
    wf_op("\tPC += 2;\n");
    wf_op("\tSET_SR(res)\n");

    // if S flag not set --> we swap stack pointer
    wf_op("\tif (!CPU->flag_S)\n");
    wf_op("\t{\n");
        wf_op("\t\tres = CPU->A[7];\n");
        wf_op("\t\tCPU->A[7] = CPU->USP;\n");
        wf_op("\t\tCPU->USP = res;\n");
    wf_op("\t}\n");

    wf_op("\tCPU->Status |= C68K_HALTED;\n");
    //wf_op("\tCCnt = 0;\n");
    
    // force end execution
    fterminate_op(4);
}

static void GenRTE()
{
    start_all(GEN_RES);

    wf_op("\tif (!CPU->flag_S)\n");
    wf_op("\t{\n");
        gen_privilege_exception("\t\t");
        quick_terminate_op(4);
    wf_op("\t}\n");

    // restore SR and PC
    mem_op("\tPOP_16_F(res)\n");
    wf_op("\tSET_SR(res)\n");
    mem_op("\tPOP_32_F(res)\n");
    wf_op("\tSET_PC(res)\n");

    // if S flag not set --> we swap stack pointer
    wf_op("\tif (!CPU->flag_S)\n");
    wf_op("\t{\n");
        wf_op("\t\tres = CPU->A[7];\n");
        wf_op("\t\tCPU->A[7] = CPU->USP;\n");
        wf_op("\t\tCPU->USP = res;\n");
    wf_op("\t}\n");

    // check for interrupt
    fterminate_op(20);
}

static void GenRTS()
{
    start_all(GEN_RES);

    mem_op("\tPOP_32_F(res)\n");
    wf_op("\tSET_PC(res)\n");

    terminate_op(16);
}

static void GenRTR()
{
    start_all(GEN_RES);

    mem_op("\tPOP_16_F(res)\n");
    wf_op("\tSET_CCR(res)\n");
    mem_op("\tPOP_32_F(res)\n");
    wf_op("\tSET_PC(res)\n");

    terminate_op(20);
}

static void GenJSR()
{
    start_all(GEN_ADR);
    
    // get adr
    _ea_calc_free(current_ea, current_op->reg_sft);
    mem_op("\tPUSH_32_F(PC)\n");
    wf_op("\tSET_PC(adr)\n");
    
    terminate_op(jmp_jsr_cycle_table[current_ea] + 12);
}

static void GenJMP()
{
    start_all(GEN_ADR);

    // get adr
    _ea_calc_free(current_ea, current_op->reg_sft);
    wf_op("\tSET_PC(adr)\n");

    terminate_op(jmp_jsr_cycle_table[current_ea] + 4);
}

static void GenSTCC()
{
    u32 base, cond;
    
    base = get_current_opcode_base();
    
    for(cond = 0; cond < 0x10; cond++)
    {
        // generate jump table
        gen_opjumptable(base + (cond << 8));
        // generate label & declarations
        if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
            start_op(base + (cond << 8), GEN_RES);
        else
            start_op(base + (cond << 8), GEN_ADR | GEN_RES);
        
        set_current_size(SIZE_BYTE);

        if (is_ea_memory(current_ea)) current_cycle += 4;
        
        // op
        _ea_calc(current_ea, current_op->reg_sft);
        if ((cond != COND_TR) && (cond != COND_FA))
        {
            wf_op("\tif %s\n", get_cond_as_cond(cond, 0));
            wf_op("\t{\n");
        }
        if (cond != COND_FA)
        {
            wf_op("\tres = 0xFF;\n");
            // write
            _ea_write(current_ea, current_op->reg_sft);
            if (!is_ea_memory(current_ea)) quick_terminate_op(6);
            else quick_terminate_op(4);
        }
        if ((cond != COND_TR) && (cond != COND_FA))
        {
            wf_op("\t}\n");
        }
        if (cond != COND_TR)
        {
            wf_op("\tres = 0;\n");
            // write
            _ea_write(current_ea, current_op->reg_sft);
            quick_terminate_op(4);
        }

        wf_op("}\n");
    }
}

static void GenDBCC()
{
    u32 base, cond;

    base = get_current_opcode_base();
    
    current_ea = EA_DREG;
    set_current_size(SIZE_WORD);

    for(cond = 0; cond < 0x10; cond++)
    {
        // generate jump table
        gen_opjumptable(base + (cond << 8));
        // generate label & declarations
        start_op(base + (cond << 8), GEN_RES);
        
        if (cond != COND_TR)
        {
            if (cond != COND_FA)
            {
                wf_op("\tif %s\n", get_cond_as_cond(cond, 1));
                wf_op("\t{\n");
            }
            
            // read Dx
            _ea_calc(current_ea, current_op->reg_sft);
            _ea_read(current_ea, current_op->reg_sft);
            // dec Dx
            wf_op("\tres--;\n");
            // write Dx
            _ea_write(current_ea, current_op->reg_sft);
            wf_op("\tif ((s32)res != -1)\n");
            wf_op("\t{\n");
                wf_op("\t\tPC += (s32)(s16)FETCH_WORD;\n");
                // rebase PC
                wf_op("\t\tSET_PC(PC);\n");
                quick_terminate_op(10);
            wf_op("\t}\n");

            if (cond != COND_FA)
            {
                wf_op("\t}\n");
                wf_op("\telse\n");
                wf_op("\t{\n");
                    wf_op("\t\tPC += 2;\n");
                    quick_terminate_op(12);
                wf_op("\t}\n");
            }
        }
        
        wf_op("\tPC += 2;\n");
        
        if (cond == COND_TR) terminate_op(12);
        else terminate_op(14);
    }
}

static void GenBCC()
{
    u32 base, cond;

    base = get_current_opcode_base();
    
    for(cond = 2; cond < 0x10; cond++)
    {
        // generate jump table
        gen_opjumptable_ext(base + (cond << 8), 0x01, 0xFF, 1, base + (cond << 8) + 0x01);
        // generate label & declarations
        start_op(base + (cond << 8) + 0x01, 0);

        // op
        wf_op("\tif %s\n", get_cond_as_cond(cond, 0));
        wf_op("\t{\n");
            wf_op("\t\tPC += (s32)(s8)Opcode;\n");     // no rebase needed for 8 bits deplacement
            add_CCnt(2);
        wf_op("\t}\n");

        terminate_op(8);
    }
}

static void GenBCC16()
{
    u32 base, cond;

    base = get_current_opcode_base();

    for(cond = 2; cond < 0x10; cond++)
    {
        // generate jump table
        gen_opjumptable(base + (cond << 8));
        // generate label & declarations
        start_op(base + (cond << 8), 0);

        // op
        wf_op("\tif %s\n", get_cond_as_cond(cond, 0));
        wf_op("\t{\n");
            wf_op("\t\tPC += (s32)(s16)FETCH_WORD;\n");
            // rebase PC
            wf_op("\t\tSET_PC(PC);\n");
            quick_terminate_op(10);
        wf_op("\t}\n");
        
        wf_op("\tPC += 2;\n");

        terminate_op(12);
    }
}

static void GenBRA()
{
    u32 base = get_current_opcode_base();

    // generate jump table
    gen_opjumptable_ext(base, 0x01, 0xFF, 1, base + 0x01);
    // generate label & declarations
    start_op(base + 0x01, 0);

    wf_op("\tPC += (s32)(s8)Opcode;\n");     // no rebase needed for 8 bits deplacement

    terminate_op(10);
}

static void GenBRA16()
{
    u32 base = get_current_opcode_base();

    // generate jump table
    gen_opjumptable(base + 0x00);
    // generate label & declarations
    start_op(base + 0x00, 0);

    wf_op("\tPC += (s32)(s16)FETCH_WORD;\n");
    // rebase PC
    wf_op("\tSET_PC(PC);\n");

    terminate_op(10);
}

static void GenBSR()
{
    u32 base = get_current_opcode_base();

    // generate jump table
    gen_opjumptable_ext(base, 0x01, 0xFF, 1, base + 0x01);
    // generate label & declarations
    start_op(base + 0x01, 0);

    mem_op("\tPUSH_32_F(PC)\n");
    wf_op("\tPC += (s32)(s8)Opcode;\n");     // no rebase needed for 8 bits deplacement

    terminate_op(18);
}

static void GenBSR16()
{
    u32 base = get_current_opcode_base();

    // generate jump table
    gen_opjumptable(base + 0x00);
    // generate label & declarations
    start_op(base + 0x00, GEN_RES);

    wf_op("\tres = (s32)(s16)FETCH_WORD;\n");
    mem_op("\tPUSH_32_F(PC + 2)\n");
    wf_op("\tPC += (s32) res;\n");
    wf_op("\tSET_PC(PC);\n");

    terminate_op(18);
}

static void GenArithQ(char op)
{
    u32 base;

    if ((current_ea == EA_AREG) && (current_size == SIZE_BYTE)) return;

    base = get_current_opcode_base();
    
    // generate jump table
    gen_opjumptable_ext(base, (0 << 9), (7 << 9), (1 << 9), base);

    // generate label & declarations
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_op(base, GEN_DST | GEN_RES | GEN_SRC);
    else
        start_op(base, GEN_ALL);

    if (current_ea == EA_AREG) set_current_size(SIZE_LONG);

    if (is_ea_memory(current_ea)) current_cycle += 4;
    if (current_size == SIZE_LONG) current_cycle += 4;

    // read src
	wf_op("\tsrc = (((Opcode >> 9) - 1) & 7) + 1;\n");
    // read dst
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_dst(current_ea, current_op->reg_sft);
    // op
    wf_op("\tres = dst %c src;\n", op);
    // flag calculation
    if (current_ea != EA_AREG)
    {
        if (op == '+') set_add_flag();
        else set_sub_flag();
    }
    // write dst
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(4);
}

static void GenADDQ()
{
    GenArithQ('+');
}

static void GenSUBQ()
{
    GenArithQ('-');
}

static void GenLogicaD(char op)
{
    // generate jump table & opcode declaration
    current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES | GEN_SRC);
    else
        start_all(GEN_ADR | GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG)
    {
        if (!is_ea_memory(current_ea)) current_cycle += 2;
        else current_cycle += 4;
    }

    // read src
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read(current_ea2, current_op->reg2_sft);
    // op
    wf_op("\tres %c= src;\n", op);
    // flag calculation
    set_logic_flag();
    // write dst (Dx)
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(4);
}

static void GenLogicDa(char op)
{
    // generate jump table & opcode declaration
    current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES | GEN_SRC);
    else
        start_all(GEN_ADR | GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 4;

    // read src (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_src(current_ea2, current_op->reg2_sft);
    // read dst
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // op
    wf_op("\tres %c= src;\n", op);
    // flag calculation
    set_logic_flag();
    // write dst
    _ea_write(current_ea, current_op->reg_sft);

    if (current_ea == EA_DREG) terminate_op(4);
    else terminate_op(8);
}

static void GenANDaD()
{
    GenLogicaD('&');
}

static void GenANDDa()
{
    GenLogicDa('&');
}

static void GenORaD()
{
    GenLogicaD('|');
}

static void GenORDa()
{
    GenLogicDa('|');
}

static void GenEORDa()
{
    GenLogicDa('^');
}

static void GenNBCD()
{
    set_current_size(SIZE_BYTE);

    // generate jump table & opcode declaration
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES);
    else
        start_all(GEN_ADR | GEN_RES);

    if (is_ea_memory(current_ea)) current_cycle += 2;

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    
	// op
    wf_op("\tres = 0x9a - res - ((CPU->flag_X >> C68K_SR_X_SFT) & 1);\n");
    wf_op("\n");
    wf_op("\tif (res != 0x9a)\n");
    wf_op("\t{\n");
        wf_op("\t\tif ((res & 0x0f) == 0xa) res = (res & 0xf0) + 0x10;\n");
        wf_op("\t\tres &= 0xFF;\n");

        // write
        _ea_write(current_ea, current_op->reg_sft);

        // flag calculation
       	wf_op("\t\tCPU->flag_notZ |= res;\n");
        wf_op("\t\tCPU->flag_X = CPU->flag_C = C68K_SR_C;\n");

    wf_op("\t}\n");
    wf_op("\telse CPU->flag_X = CPU->flag_C = 0;\n");
   	wf_op("\tCPU->flag_N = res;\n");

    terminate_op(6);
}

static void GenBCD(char op)
{
    // op
    wf_op("\tres = (dst & 0xF) %c (src & 0xF) %c ((CPU->flag_X >> C68K_SR_X_SFT) & 1);\n", op, op);
    wf_op("\tif (res > 9) res %c= 6;\n", op);
    wf_op("\tres += (dst & 0xF0) %c (src & 0xF0);\n", op, op);

    // flag calculation
    wf_op("\tif (res > 0x99)\n");
    wf_op("\t{\n");
        switch (op)
        {
            case '+':
                wf_op("\t\tres -= 0xA0;\n");
                break;

            case '-':
                wf_op("\t\tres += 0xA0;\n");
                break;
        }
        wf_op("\t\tCPU->flag_X = CPU->flag_C = C68K_SR_C;\n");
    wf_op("\t}\n");
    wf_op("\telse CPU->flag_X = CPU->flag_C = 0;\n");
   	wf_op("\tCPU->flag_notZ |= res & 0xFF;\n");
   	wf_op("\tCPU->flag_N = res;\n");
}

static void GenxBCD(char op)
{
    set_current_size(SIZE_BYTE);

    // generate jump table & opcode declaration
    current_ea = EA_DREG;
    current_ea2 = EA_DREG;
    start_all(GEN_DST | GEN_RES | GEN_SRC);

    // read src (Dx)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    
    // op & flag calculation
    GenBCD(op);

    // write dst (Dx)
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(6);
}

static void GenxBCDM(char op)
{
    set_current_size(SIZE_BYTE);

    // generate jump table & opcode declaration
    current_ea = EA_ADEC;
    current_ea2 = EA_ADEC;
    start_all(GEN_ALL);

    // read src (ADEC)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (ADEC)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    
    // op & flag calculation
    GenBCD(op);

    // write dst (ADEC)
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(6);
}

static void GenxBCD7M(char op)
{
    set_current_size(SIZE_BYTE);

    // generate jump table & opcode declaration
    current_ea = EA_ADEC7;
    current_ea2 = EA_ADEC;
    start_all(GEN_ALL);

    // read src (ADEC7)
    _ea_calc(current_ea, 0);
    _ea_read_src(current_ea, 0);
    // read dst (ADEC)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);

    // op & flag calculation
    GenBCD(op);

    // write dst (ADEC)
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(6);
}

static void GenxBCDM7(char op)
{
    set_current_size(SIZE_BYTE);

    // generate jump table & opcode declaration
    current_ea = EA_ADEC;
    current_ea2 = EA_ADEC7;
    start_all(GEN_ALL);

    // read src (ADEC)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (ADEC7)
    _ea_calc(current_ea2, 0);
    _ea_read_dst(current_ea2, 0);

    // op & flag calculation
    GenBCD(op);

    // write dst (ADEC7)
    _ea_write(current_ea2, 0);

    terminate_op(6);
}

static void GenxBCD7M7(char op)
{
    set_current_size(SIZE_BYTE);

    // generate jump table & opcode declaration
    current_ea = EA_ADEC7;
    current_ea2 = EA_ADEC7;
    start_all(GEN_ALL);

    // read src (ADEC7)
    _ea_calc(current_ea, 0);
    _ea_read_src(current_ea, 0);
    // read dst (ADEC7)
    _ea_calc(current_ea2, 0);
    _ea_read_dst(current_ea2, 0);

    // op & flag calculation
    GenBCD(op);

    // write dst (ADEC7)
    _ea_write(current_ea2, 0);

    terminate_op(6);
}

static void GenABCD()
{
    GenxBCD('+');
}

static void GenABCDM()
{
    GenxBCDM('+');
}

static void GenABCD7M()
{
    GenxBCD7M('+');
}

static void GenABCDM7()
{
    GenxBCDM7('+');
}

static void GenABCD7M7()
{
    GenxBCD7M7('+');
}

static void GenSBCD()
{
    GenxBCD('-');
}

static void GenSBCDM()
{
    GenxBCDM('-');
}

static void GenSBCD7M()
{
    GenxBCD7M('-');
}

static void GenSBCDM7()
{
    GenxBCDM7('-');
}

static void GenSBCD7M7()
{
    GenxBCD7M7('-');
}

static void GenDIVU()
{
    // generate jump table & opcode declaration
    current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_DST | GEN_RES | GEN_SRC);
    else
        start_all(GEN_ALL);

    set_current_size(SIZE_WORD);
    
    // read src
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    
    // division by zero
    wf_op("\tif (src == 0)\n");
    wf_op("\t{\n");
        gen_exception("\t\t", "C68K_ZERO_DIVIDE_EX");
        quick_terminate_op(10);
    wf_op("\t}\n");
    
    set_current_size(SIZE_LONG);

    // read dst (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);

    wf_op("\t{\n");
        wf_op("\t\tu32 q, r;\n");
        wf_op("\n");
        wf_op("\t\tq = dst / src;\n");
        wf_op("\t\tr = dst %% src;\n");
        wf_op("\n");
        
        wf_op("\t\tif (q & 0xFFFF0000)\n");
        wf_op("\t\t{\n");
            // overflow occured
            wf_op("\t\t\tCPU->flag_V = C68K_SR_V;\n");
            quick_terminate_op(70);
        wf_op("\t\t}\n");
        
        // quotient size = word
        set_current_size(SIZE_WORD);

        wf_op("\t\tq &= 0x%.8X;\n", current_bits_mask);
        wf_op("\t\tCPU->flag_notZ = q;\n");
        wf_op("\t\tCPU->flag_N = q >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
        wf_op("\t\tCPU->flag_V = CPU->flag_C = 0;\n");

        wf_op("\t\tres = q | (r << 16);\n");
        
        set_current_size(SIZE_LONG);

        // write dst (Dx)
        _ea_write(current_ea2, current_op->reg2_sft);
    wf_op("\t}\n");

    // max cycle = 140
    terminate_op(140 - 50);
}

static void GenDIVS()
{
    // generate jump table & opcode declaration
    current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_DST | GEN_RES | GEN_SRC);
    else
        start_all(GEN_ALL);

    set_current_size(SIZE_WORD);

    // read src
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src_sx(current_ea, current_op->reg_sft);

    // division by zero
    wf_op("\tif (src == 0)\n");
    wf_op("\t{\n");
        gen_exception("\t\t", "C68K_ZERO_DIVIDE_EX");
        quick_terminate_op(10);
    wf_op("\t}\n");

    set_current_size(SIZE_LONG);

    // read dst (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);

    // division by zero
    wf_op("\tif ((dst == 0x80000000) && (src == -1))\n");
    wf_op("\t{\n");
        wf_op("\t\tCPU->flag_notZ = CPU->flag_N = 0;\n");
        wf_op("\t\tCPU->flag_V = CPU->flag_C = 0;\n");
        wf_op("\t\tres = 0;\n");

        // write dst (Dx)
        _ea_write(current_ea2, current_op->reg2_sft);

        quick_terminate_op(50);
    wf_op("\t}\n");

    wf_op("\t{\n");
        wf_op("\t\ts32 q, r;\n");
        wf_op("\n");
        wf_op("\t\tq = (s32)dst / (s32)src;\n");
        wf_op("\t\tr = (s32)dst %% (s32)src;\n");
        wf_op("\n");

        wf_op("\t\tif ((q > 0x7FFF) || (q < -0x8000))\n");
        wf_op("\t\t{\n");
            // overflow occured
            wf_op("\t\t\tCPU->flag_V = C68K_SR_V;\n");
            quick_terminate_op(80);
        wf_op("\t\t}\n");

        // quotient size = word
        set_current_size(SIZE_WORD);
        
        wf_op("\t\tq &= 0x%.8X;\n", current_bits_mask);
        wf_op("\t\tCPU->flag_notZ = q;\n");
        wf_op("\t\tCPU->flag_N = q >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
        wf_op("\t\tCPU->flag_V = CPU->flag_C = 0;\n");

        wf_op("\t\tres = q | (r << 16);\n");

        set_current_size(SIZE_LONG);

        // write dst (Dx)
        _ea_write(current_ea2, current_op->reg2_sft);
    wf_op("\t}\n");

    // max cycle = 158
    terminate_op(158 - 50);
}

static void GenMULU()
{
    // generate jump table & opcode declaration
    current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES | GEN_SRC);
    else
        start_all(GEN_ADR | GEN_RES | GEN_SRC);

    set_current_size(SIZE_WORD);

    // read src
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read(current_ea2, current_op->reg2_sft);

    set_current_size(SIZE_LONG);
    // op
    wf_op("\tres *= src;\n");

    // flag calculation
   	wf_op("\tCPU->flag_N = res >> 24;\n");
   	wf_op("\tCPU->flag_notZ = res;\n");
    wf_op("\tCPU->flag_V = CPU->flag_C = 0;\n");

    // write dst (Dx)
    _ea_write(current_ea2, current_op->reg2_sft);

    // min cycle = 38; max cycle = 70
    terminate_op(38 + (2 * 6));
}

static void GenMULS()
{
    // generate jump table & opcode declaration
    current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_RES | GEN_SRC);
    else
        start_all(GEN_ADR | GEN_RES | GEN_SRC);

    set_current_size(SIZE_WORD);
    // read src signed
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src_sx(current_ea, current_op->reg_sft);
    // read dst signed (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_sx(current_ea2, current_op->reg2_sft);
    
    set_current_size(SIZE_LONG);
    // op
    //wf_op("\t(s32)res *= (s32)src;\n");
    wf_op("\tres *= (s32)src;\n"); // antime fix
    
    // flag calculation
   	wf_op("\tCPU->flag_N = res >> 24;\n");
   	wf_op("\tCPU->flag_notZ = res;\n");
    wf_op("\tCPU->flag_V = CPU->flag_C = 0;\n");
    
    // write dst (Dx)
    _ea_write(current_ea2, current_op->reg2_sft);

    // min cycle = 38; max cycle = 70
    terminate_op(38 + (2 * 6));
}

static void GenArithaD(char op)
{
    // generate jump table & opcode declaration
    current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_DST | GEN_RES | GEN_SRC);
    else
        start_all(GEN_ALL);

    if (current_size == SIZE_LONG)
    {
        if (!is_ea_memory(current_ea)) current_cycle += 2;
        else current_cycle += 4;
    }

    // read src
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    if (op == ' ')
    {
        // op
        wf_op("\tres = dst - src;\n");
        // flag calculation
        set_cmp_flag();
    }
    else
    {
        // op
        wf_op("\tres = dst %c src;\n", op);
        // flag calculation
        if (op == '+') set_add_flag();
        else set_sub_flag();
        // write dst (Dx)
        _ea_write(current_ea2, current_op->reg2_sft);
    }

    terminate_op(4);
}

static void GenArithDa(char op)
{
    // generate jump table & opcode declaration
    current_ea2 = EA_DREG;
    start_all(GEN_ALL);

    if (current_size == SIZE_LONG) current_cycle += 4;

    // read src (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_src(current_ea2, current_op->reg2_sft);
    // read dst
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_dst(current_ea, current_op->reg_sft);
    // op
    wf_op("\tres = dst %c src;\n", op);
    // flag calculation
    if (op == '+') set_add_flag();
    else set_sub_flag();
    // write dst
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

static void GenArithA(char op)
{
    // generate jump table & opcode declaration
    current_ea2 = EA_AREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_DST | GEN_RES | GEN_SRC);
    else
        start_all(GEN_ALL);

    if ((op != ' ') && ((current_size == SIZE_WORD) || (is_ea_memory(current_ea)))) current_cycle += 2;

    // read src
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src_sx(current_ea, current_op->reg_sft);
    // read dst (Ax)
    set_current_size(SIZE_LONG);
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    // op
    if (op == ' ')
    {
        // op
        wf_op("\tres = dst - src;\n");
        // flag calculation
        set_cmp_flag();
    }
    else
    {
        // op
        wf_op("\tres = dst %c src;\n", op);
        // write dst (Ax)
        _ea_write(current_ea2, current_op->reg2_sft);
    }

    terminate_op(6);
}

static void GenArithX(char op)
{
    // generate jump table & opcode declaration
    current_ea = EA_DREG;
    current_ea2 = EA_DREG;
    if ((current_ea == EA_AREG) || (current_ea == EA_DREG) || (current_ea == EA_IMM))
        start_all(GEN_DST | GEN_RES | GEN_SRC);
    else
        start_all(GEN_ALL);

    if (current_size == SIZE_LONG) current_cycle += 4;

    // read src (Dx)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (Dx)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    // op
    wf_op("\tres = dst %c src %c ((CPU->flag_X >> 8) & 1);\n", op, op);
    // flag calculation
    if (op == '+') set_addx_flag();
    else set_subx_flag();
    // write dst (Dx)
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(4);
}

static void GenArithXM(char op)
{
    // generate jump table & opcode declaration
    current_ea = EA_ADEC;
    current_ea2 = EA_ADEC;
    start_all(GEN_ALL);

    if (current_size == SIZE_LONG) current_cycle += 4;

    // read src (ADEC)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (ADEC)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    // op
    wf_op("\tres = dst %c src %c ((CPU->flag_X >> 8) & 1);\n", op, op);
    // flag calculation
    if (op == '+') set_addx_flag();
    else set_subx_flag();
    // write dst (ADEC)
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(6);
}

static void GenArithX7M(char op)
{
    // generate jump table & opcode declaration
    current_ea = EA_ADEC7;
    current_ea2 = EA_ADEC;
    start_all(GEN_ALL);

    if (current_size == SIZE_LONG) current_cycle += 4;

    // read src (ADEC7)
    _ea_calc(current_ea, 0);
    _ea_read_src(current_ea, 0);
    // read dst (ADEC)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    // op
    wf_op("\tres = dst %c src %c ((CPU->flag_X >> 8) & 1);\n", op, op);
    // flag calculation
    if (op == '+') set_addx_flag();
    else set_subx_flag();
    // write dst (ADEC)
    _ea_write(current_ea2, current_op->reg2_sft);

    terminate_op(6);
}

static void GenArithXM7(char op)
{
    // generate jump table & opcode declaration
    current_ea = EA_ADEC;
    current_ea2 = EA_ADEC7;
    start_all(GEN_ALL);

    if (current_size == SIZE_LONG) current_cycle += 4;

    // read src (ADEC)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (ADEC7)
    _ea_calc(current_ea2, 0);
    _ea_read_dst(current_ea2, 0);
    // op
    wf_op("\tres = dst %c src %c ((CPU->flag_X >> 8) & 1);\n", op, op);
    // flag calculation
    if (op == '+') set_addx_flag();
    else set_subx_flag();
    // write dst (ADEC7)
    _ea_write(current_ea2, 0);

    terminate_op(6);
}

static void GenArithX7M7(char op)
{
    // generate jump table & opcode declaration
    current_ea = EA_ADEC7;
    current_ea2 = EA_ADEC7;
    start_all(GEN_ALL);

    if (current_size == SIZE_LONG) current_cycle += 4;

    // read src (ADEC7)
    _ea_calc(current_ea, 0);
    _ea_read_src(current_ea, 0);
    // read dst (ADEC7)
    _ea_calc(current_ea2, 0);
    _ea_read_dst(current_ea2, 0);
    // op
    wf_op("\tres = dst %c src %c ((CPU->flag_X >> 8) & 1);\n", op, op);
    // flag calculation
    if (op == '+') set_addx_flag();
    else set_subx_flag();
    // write dst (ADEC7)
    _ea_write(current_ea2, 0);

    terminate_op(6);
}

static void GenADDaD()
{
    GenArithaD('+');
}

static void GenADDDa()
{
    GenArithDa('+');
}

static void GenADDA()
{
    GenArithA('+');
}

static void GenADDX()
{
    GenArithX('+');
}

static void GenADDXM()
{
    GenArithXM('+');
}

static void GenADDX7M()
{
    GenArithX7M('+');
}

static void GenADDXM7()
{
    GenArithXM7('+');
}

static void GenADDX7M7()
{
    GenArithX7M7('+');
}

static void GenSUBaD()
{
    GenArithaD('-');
}

static void GenSUBDa()
{
    GenArithDa('-');
}

static void GenSUBA()
{
    GenArithA('-');
}

static void GenSUBX()
{
    GenArithX('-');
}

static void GenSUBXM()
{
    GenArithXM('-');
}

static void GenSUBX7M()
{
    GenArithX7M('-');
}

static void GenSUBXM7()
{
    GenArithXM7('-');
}

static void GenSUBX7M7()
{
    GenArithX7M7('-');
}

static void GenCMP()
{
    GenArithaD(' ');
}

static void GenCMPA()
{
    GenArithA(' ');
}

static void GenCMPM()
{
    // generate jump table & opcode declaration
    current_ea = EA_AINC;
    current_ea2 = EA_AINC;
    start_all(GEN_ALL);

    // read src (ADEC)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (ADEC)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    // op
    wf_op("\tres = dst - src;\n");
    // flag calculation
    set_cmp_flag();

    terminate_op(4);

}

static void GenCMP7M()
{
    // generate jump table & opcode declaration
    current_ea = EA_AINC7;
    current_ea2 = EA_AINC;
    start_all(GEN_ALL);

    // read src (ADEC)
    _ea_calc(current_ea, 0);
    _ea_read_src(current_ea, 0);
    // read dst (ADEC)
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_dst(current_ea2, current_op->reg2_sft);
    // op
    wf_op("\tres = dst - src;\n");
    // flag calculation
    set_cmp_flag();

    terminate_op(4);

}

static void GenCMPM7()
{
    // generate jump table & opcode declaration
    current_ea = EA_AINC;
    current_ea2 = EA_AINC7;
    start_all(GEN_ALL);

    // read src (ADEC)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);
    // read dst (ADEC)
    _ea_calc(current_ea2, 0);
    _ea_read_dst(current_ea2, 0);
    // op
    wf_op("\tres = dst - src;\n");
    // flag calculation
    set_cmp_flag();

    terminate_op(4);

}

static void GenCMP7M7()
{
    // generate jump table & opcode declaration
    current_ea = EA_AINC7;
    current_ea2 = EA_AINC7;
    start_all(GEN_ALL);

    // read src (ADEC)
    _ea_calc(current_ea, 0);
    _ea_read_src(current_ea, 0);
    // read dst (ADEC)
    _ea_calc(current_ea2, 0);
    _ea_read_dst(current_ea2, 0);
    // op
    wf_op("\tres = dst - src;\n");
    // flag calculation
    set_cmp_flag();

    terminate_op(4);

}

static void GenEXGDD()
{
    // generate jump table & opcode declaration
    set_current_size(SIZE_LONG);
    current_ea = EA_DREG;
    current_ea2 = EA_DREG;
    start_all(GEN_RES | GEN_SRC);
    
    // read R1
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // read R2
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_src(current_ea2, current_op->reg2_sft);
    // write R1
    _ea_write(current_ea2, current_op->reg2_sft);
    wf_op("\tres = src;\n");
    // write R2
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenEXGAA()
{
    // generate jump table & opcode declaration
    set_current_size(SIZE_LONG);
    current_ea = EA_AREG;
    current_ea2 = EA_AREG;
    start_all(GEN_RES | GEN_SRC);

    // read R1
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // read R2
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_src(current_ea2, current_op->reg2_sft);
    // write R1
    _ea_write(current_ea2, current_op->reg2_sft);
    wf_op("\tres = src;\n");
    // write R2
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenEXGAD()
{
    // generate jump table & opcode declaration
    set_current_size(SIZE_LONG);
    current_ea = EA_AREG;
    current_ea2 = EA_DREG;
    start_all(GEN_RES | GEN_SRC);

    // read R1
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read(current_ea, current_op->reg_sft);
    // read R2
    _ea_calc(current_ea2, current_op->reg2_sft);
    _ea_read_src(current_ea2, current_op->reg2_sft);
    // write R1
    _ea_write(current_ea2, current_op->reg2_sft);
    wf_op("\tres = src;\n");
    // write R2
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenASRk()
{
    u32 base;
    
    current_ea = EA_DREG;               // dst = Dx

    base = get_current_opcode_base();
    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0E00, 0x0200, base);
    // generate label & declarations
    start_op(base, GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = (((Opcode >> 9) - 1) & 7) + 1;\n");
    adds_CCnt("sft * 2");

    // read (sign extend)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src_sx(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);\n");
    wf_op("\tres = ((s32)src) >> sft;\n");
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res;\n");
    
    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenLSRk()
{
    u32 base;

    current_ea = EA_DREG;               // dst = Dx

    base = get_current_opcode_base();
    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0E00, 0x0200, base);
    // generate label & declarations
    start_op(base, GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = (((Opcode >> 9) - 1) & 7) + 1;\n");
    adds_CCnt("sft * 2");

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_N = CPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);\n");
    wf_op("\tres = src >> sft;\n");
    wf_op("\tCPU->flag_notZ = res;\n");

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenROXRk()
{
    u32 base;

    current_ea = EA_DREG;               // dst = Dx

    base = get_current_opcode_base();
    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0E00, 0x0200, base);
    // generate label & declarations
    start_op(base, GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = (((Opcode >> 9) - 1) & 7) + 1;\n");
    adds_CCnt("sft * 2");

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & C / X flags calculation
    if (current_size != SIZE_LONG)
    {
        wf_op("\tsrc |= (CPU->flag_X & C68K_SR_X) << %d;\n", (current_sft_mask + 1) - C68K_SR_X_SFT);
        wf_op("\tres = (src >> sft) | (src << (%d - sft));\n", current_sft_mask + 2);
        wf_op("\tCPU->flag_X = CPU->flag_C = res >> %d;\n", (current_sft_mask + 1) - C68K_SR_X_SFT);
    }
    else
    {
        wf_op("\tCPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);\n");
        wf_op("\tif (sft == 1) res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << (32 - (C68K_SR_X_SFT + 1)));\n");
        wf_op("\telse res = (src >> sft) | (src << (33 - sft)) | ((CPU->flag_X & C68K_SR_X) << (32 - (C68K_SR_X_SFT + sft)));\n");
        wf_op("\tCPU->flag_X = CPU->flag_C;\n");
    }

    // V / N / Z flags calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    if (current_size == SIZE_LONG) wf_op("\tCPU->flag_notZ = res;\n");
    else wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenRORk()
{
    u32 base;

    current_ea = EA_DREG;               // dst = Dx

    base = get_current_opcode_base();
    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0E00, 0x0200, base);
    // generate label & declarations
    start_op(base, GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = (((Opcode >> 9) - 1) & 7) + 1;\n");
    adds_CCnt("sft * 2");

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);\n");
    wf_op("\tres = (src >> sft) | (src << (%d - sft));\n", current_sft_mask + 1);
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    if (current_size == SIZE_LONG) wf_op("\tCPU->flag_notZ = res;\n");
    else wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenASLk()
{
    u32 base;

    current_ea = EA_DREG;               // dst = Dx
    
    base = get_current_opcode_base();
    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0E00, 0x0200, base);
    // generate label & declarations
    start_op(base, GEN_RES | GEN_SRC);
    
    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = (((Opcode >> 9) - 1) & 7) + 1;\n");
    adds_CCnt("sft * 2");

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // if (shift < size op) ... only for BYTE here
    if (current_size == SIZE_BYTE)
    {
        wf_op("\tif (sft < %d)\n", current_sft_mask + 1);
        wf_op("\t{\n");
    }

        // op & flag X, C, N, Z calculation
        if (((current_sft_mask + 1) - C68K_SR_C_SFT) < 8)
            wf_op("\t\tCPU->flag_X = CPU->flag_C = src << (%d + sft);\n", current_sft_mask + 1 - C68K_SR_C_SFT);
        else wf_op("\t\tCPU->flag_X = CPU->flag_C = src >> (%d - sft);\n", current_sft_mask + 1 - C68K_SR_C_SFT);
        wf_op("\t\tres = src << sft;\n");
        wf_op("\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
        wf_op("\t\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

        // write
        _ea_write(current_ea, current_op->reg_sft);

        // we do V flag calculation at end for a better register usage
        wf_op("\t\tCPU->flag_V = 0;\n");
        if (current_size == SIZE_BYTE)
        {
            wf_op("\t\tif ((sft > %d) && (src)) CPU->flag_V = C68K_SR_V;\n", current_sft_mask);
            wf_op("\t\telse\n");
            }
        wf_op("\t\t{\n");
            wf_op("\t\t\tu32 msk = (((s32)0x80000000) >> (sft + %d)) & 0x%.8X;\n", 31 - current_sft_mask, current_bits_mask);
            wf_op("\t\t\tsrc &= msk;\n");
            wf_op("\t\t\tif ((src) && (src != msk)) CPU->flag_V = C68K_SR_V;\n");
        wf_op("\t\t}\n");
        
    if (current_size == SIZE_BYTE)
    {
        quick_terminate_op(6);
        wf_op("\t}\n");
        wf_op("\n");
    
        // special case of shift == size op (sft = 8 for byte operation)
        wf_op("\tif (src) CPU->flag_V = C68K_SR_V;\n");
        wf_op("\telse CPU->flag_V = 0;\n");
        wf_op("\tCPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;\n");

        // write
        wf_op("\tres = 0;\n");
        _ea_write(current_ea, current_op->reg_sft);

        // others flags
        wf_op("\tCPU->flag_N = 0;\n");
        wf_op("\tCPU->flag_notZ = 0;\n");
    }
    
    terminate_op(6);
}

static void GenLSLk()
{
    u32 base;

    current_ea = EA_DREG;               // dst = Dx

    base = get_current_opcode_base();
    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0E00, 0x0200, base);
    // generate label & declarations
    start_op(base, GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = (((Opcode >> 9) - 1) & 7) + 1;\n");
    adds_CCnt("sft * 2");

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    if (((current_sft_mask + 1) - C68K_SR_C_SFT) < 8)
        wf_op("\tCPU->flag_X = CPU->flag_C = src << (%d + sft);\n", current_sft_mask + 1 - C68K_SR_C_SFT);
    else wf_op("\tCPU->flag_X = CPU->flag_C = src >> (%d - sft);\n", current_sft_mask + 1 - C68K_SR_C_SFT);
    wf_op("\tres = src << sft;\n");
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenROXLk()
{
    u32 base;

    current_ea = EA_DREG;               // dst = Dx

    base = get_current_opcode_base();
    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0E00, 0x0200, base);
    // generate label & declarations
    start_op(base, GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = (((Opcode >> 9) - 1) & 7) + 1;\n");
    adds_CCnt("sft * 2");

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & C / X flags calculation
    if (current_size != SIZE_LONG)
    {
        wf_op("\tsrc |= (CPU->flag_X & C68K_SR_X) << %d;\n", (current_sft_mask + 1) - C68K_SR_X_SFT);
        wf_op("\tres = (src << sft) | (src >> (%d - sft));\n", current_sft_mask + 2);
        wf_op("\tCPU->flag_X = CPU->flag_C = res >> %d;\n", (current_sft_mask + 1) - C68K_SR_X_SFT);
    }
    else
    {
        wf_op("\tCPU->flag_C = src >> ((32 - C68K_SR_C_SFT) - sft);\n");
        wf_op("\tif (sft == 1) res = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> ((C68K_SR_X_SFT + 1) - 1));\n");
        wf_op("\telse res = (src << sft) | (src >> (33 - sft)) | ((CPU->flag_X & C68K_SR_X) >> ((C68K_SR_X_SFT + 1) - sft));\n");
        wf_op("\tCPU->flag_X = CPU->flag_C;\n");
    }

    // V / N / Z flags calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    if (current_size == SIZE_LONG) wf_op("\tCPU->flag_notZ = res;\n");
    else wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenROLk()
{
    u32 base;

    current_ea = EA_DREG;               // dst = Dx

    base = get_current_opcode_base();
    // generate jump table
    gen_opjumptable_ext(base, 0x0000, 0x0E00, 0x0200, base);
    // generate label & declarations
    start_op(base, GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = (((Opcode >> 9) - 1) & 7) + 1;\n");
    adds_CCnt("sft * 2");

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    if (((current_sft_mask + 1) - C68K_SR_C_SFT) < 8)
        wf_op("\tCPU->flag_C = src << (%d + sft);\n", current_sft_mask + 1 - C68K_SR_C_SFT);
    else wf_op("\tCPU->flag_C = src >> (%d - sft);\n", current_sft_mask + 1 - C68K_SR_C_SFT);
    wf_op("\tres = (src << sft) | (src >> (%d - sft));\n", current_sft_mask + 1);
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    if (current_size == SIZE_LONG) wf_op("\tCPU->flag_notZ = res;\n");
    else wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(6);
}

static void GenASRD()
{
//    u32 base = get_current_opcode_base();

    current_ea = EA_DREG;               // dst = Dx

    start_all(GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = CPU->D[(Opcode >> %d) & 7] & 0x3F;\n", current_op->reg2_sft);

    // read (sign extend)
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src_sx(current_ea, current_op->reg_sft);

    // if (shift != 0)
    wf_op("\tif (sft)\n");
    wf_op("\t{\n");
    
        adds_CCnt("sft * 2");
        
        // if (shift < size op)
        wf_op("\t\tif (sft < %d)\n", current_sft_mask + 1);
        wf_op("\t\t{\n");

            // op & flag calculation
            wf_op("\t\t\tCPU->flag_V = 0;\n");
            if (current_size == SIZE_BYTE) wf_op("\t\t\tCPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);\n");
            else wf_op("\t\t\tCPU->flag_X = CPU->flag_C = (src >> (sft - 1)) << C68K_SR_C_SFT;\n");
            wf_op("\t\t\tres = ((s32)src) >> sft;\n", szcs);
            wf_op("\t\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
            wf_op("\t\t\tCPU->flag_notZ = res;\n");
            
            // write
            _ea_write(current_ea, current_op->reg_sft);

            quick_terminate_op(6);
        wf_op("\t\t}\n");
        wf_op("\n");

        // special case of shift >= size op
        
        // if signed
        wf_op("\t\tif (src & (1 << %d))\n", current_sft_mask);
        wf_op("\t\t{\n");
        
            // op & flag calculation
            wf_op("\t\t\tCPU->flag_N = C68K_SR_N;\n");
            wf_op("\t\t\tCPU->flag_notZ = 1;\n");
            wf_op("\t\t\tCPU->flag_V = 0;\n");
            wf_op("\t\t\tCPU->flag_C = C68K_SR_C;\n");
            wf_op("\t\t\tCPU->flag_X = C68K_SR_X;\n");
            wf_op("\t\t\tres = 0x%.8X;\n", current_bits_mask);

            // write
            _ea_write(current_ea, current_op->reg_sft);
            
            quick_terminate_op(6);
        wf_op("\t\t}\n");
        wf_op("\n");

        // if not signed
        wf_op("\t\tCPU->flag_N = 0;\n");
        wf_op("\t\tCPU->flag_notZ = 0;\n");
        wf_op("\t\tCPU->flag_V = 0;\n");
        wf_op("\t\tCPU->flag_C = 0;\n");
        wf_op("\t\tCPU->flag_X = 0;\n");
        wf_op("\t\tres = 0;\n");

        // write
        _ea_write(current_ea, current_op->reg_sft);

        quick_terminate_op(6);
    wf_op("\t}\n");
    wf_op("\n");

    // special case of (shift == 0)
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = 0;\n");
    wf_op("\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = src;\n");

    terminate_op(6);
}

static void GenLSRD()
{
//    u32 base = get_current_opcode_base();

    current_ea = EA_DREG;               // dst = Dx

    start_all(GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = CPU->D[(Opcode >> %d) & 7] & 0x3F;\n", current_op->reg2_sft);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // if (shift != 0)
    wf_op("\tif (sft)\n");
    wf_op("\t{\n");

        adds_CCnt("sft * 2");

        // if (shift <= size op)
        if (current_size == SIZE_LONG) wf_op("\t\tif (sft < 32)\n");
        else wf_op("\t\tif (sft <= %d)\n", current_sft_mask + 1);
        wf_op("\t\t{\n");

            // op & flag calculation
            wf_op("\t\t\tCPU->flag_N = CPU->flag_V = 0;\n");
            if (current_size == SIZE_BYTE) wf_op("\t\t\tCPU->flag_X = CPU->flag_C = src << ((C68K_SR_C_SFT + 1) - sft);\n");
            else wf_op("\t\t\tCPU->flag_X = CPU->flag_C = (src >> (sft - 1)) << C68K_SR_C_SFT;\n");
            wf_op("\t\t\tres = src >> sft;\n", szcs);
            wf_op("\t\t\tCPU->flag_notZ = res;\n");

            // write
            _ea_write(current_ea, current_op->reg_sft);

            quick_terminate_op(6);
        wf_op("\t\t}\n");
        wf_op("\n");

        // special case of shift > size op
        if (current_size == SIZE_LONG)
        {
            wf_op("\t\tif (sft == 32) CPU->flag_C = src >> (31 - C68K_SR_C_SFT);\n");
            wf_op("\t\telse CPU->flag_C = 0;\n");
            wf_op("\t\tCPU->flag_X = CPU->flag_C;\n");
        }
        else wf_op("\t\tCPU->flag_X = CPU->flag_C = 0;\n");
        wf_op("\t\tCPU->flag_N = 0;\n");
        wf_op("\t\tCPU->flag_notZ = 0;\n");
        wf_op("\t\tCPU->flag_V = 0;\n");
        wf_op("\t\tres = 0;\n");

        // write
        _ea_write(current_ea, current_op->reg_sft);

        quick_terminate_op(6);
    wf_op("\t}\n");
    wf_op("\n");

    // special case of (shift == 0)
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = 0;\n");
    wf_op("\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = src;\n");

    terminate_op(6);
}

static void GenROXRD()
{
//    u32 base = get_current_opcode_base();

    current_ea = EA_DREG;               // dst = Dx

    start_all(GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = CPU->D[(Opcode >> %d) & 7] & 0x3F;\n", current_op->reg2_sft);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // if (shift != 0)
    wf_op("\tif (sft)\n");
    wf_op("\t{\n");

        adds_CCnt("sft * 2");

        wf_op("\t\tsft %%= %d;\n", current_sft_mask + 2);
        wf_op("\n");

        // op & C / X flag calculation
        if (current_size != SIZE_LONG)
        {
            wf_op("\t\tsrc |= (CPU->flag_X & C68K_SR_X) << %d;\n", (current_sft_mask + 1) - C68K_SR_X_SFT);
            wf_op("\t\tres = (src >> sft) | (src << (%d - sft));\n", current_sft_mask + 2);
            wf_op("\t\tCPU->flag_X = CPU->flag_C = res >> %d;\n", (current_sft_mask + 1) - C68K_SR_X_SFT);
        }
        else
        {
            wf_op("\t\tif (sft != 0)\n");
            wf_op("\t\t{\n");
            wf_op("\t\t\tif (sft == 1) res = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << (32 - (C68K_SR_X_SFT + 1)));\n");
            wf_op("\t\t\telse res = (src >> sft) | (src << (33 - sft)) | (((CPU->flag_X & C68K_SR_X) << (32 - (C68K_SR_X_SFT + 1))) >> (sft - 1));\n");
            wf_op("\t\t\tCPU->flag_X = (src >> (32 - sft)) << C68K_SR_X_SFT;\n");
            wf_op("\t\t}\n");
            wf_op("\t\telse res = src;\n");
            wf_op("\t\tCPU->flag_C = CPU->flag_X;\n");
        }

        // V / N / Z flag calculation
        wf_op("\t\tCPU->flag_V = 0;\n");
        wf_op("\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
        if (current_size == SIZE_LONG) wf_op("\t\tCPU->flag_notZ = res;\n");
        else wf_op("\t\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

        // write
        _ea_write(current_ea, current_op->reg_sft);

        quick_terminate_op(6);
    wf_op("\t}\n");
    wf_op("\n");

    // special case of (shift == 0)
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = CPU->flag_X;\n");
    wf_op("\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = src;\n");

    terminate_op(6);
}

static void GenRORD()
{
//    u32 base = get_current_opcode_base();

    current_ea = EA_DREG;               // dst = Dx

    start_all(GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = CPU->D[(Opcode >> %d) & 7] & 0x3F;\n", current_op->reg2_sft);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // if (shift != 0)
    wf_op("\tif (sft)\n");
    wf_op("\t{\n");

        adds_CCnt("sft * 2");

        wf_op("\t\tsft &= 0x%.2X;\n", current_sft_mask);
        wf_op("\t\t\n");

        // op & flag calculation
        if (current_size == SIZE_BYTE)
            wf_op("\t\tCPU->flag_C = src << (C68K_SR_C_SFT - ((sft - 1) & 7));\n");
        else
            wf_op("\t\tCPU->flag_C = (src >> ((sft - 1) & %d)) << C68K_SR_C_SFT;\n", current_sft_mask);
        wf_op("\t\tres = (src >> sft) | (src << (%d - sft));\n", current_sft_mask + 1);
        wf_op("\t\tCPU->flag_V = 0;\n");
        wf_op("\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
        if (current_size == SIZE_LONG) wf_op("\t\tCPU->flag_notZ = res;\n");
        else wf_op("\t\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

        // write
        _ea_write(current_ea, current_op->reg_sft);

        quick_terminate_op(6);
    wf_op("\t}\n");
    wf_op("\n");

    // special case of (shift == 0)
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = 0;\n");
    wf_op("\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = src;\n");

    terminate_op(6);
}

static void GenASLD()
{
//    u32 base = get_current_opcode_base();

    current_ea = EA_DREG;               // dst = Dx

    start_all(GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = CPU->D[(Opcode >> %d) & 7] & 0x3F;\n", current_op->reg2_sft);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // if (shift != 0)
    wf_op("\tif (sft)\n");
    wf_op("\t{\n");

        adds_CCnt("sft * 2");

        // if (shift < size op)
        wf_op("\t\tif (sft < %d)\n", current_sft_mask + 1);
        wf_op("\t\t{\n");

            // op & flag calculation
            if (current_size != SIZE_LONG)
            {
                wf_op("\t\t\tCPU->flag_X = CPU->flag_C = (src << sft) >> %d;\n", (current_sft_mask + 1) - C68K_SR_C_SFT);
                wf_op("\t\t\tres = (src << sft) & 0x%.8X;\n", current_bits_mask);
            }
            else
            {
                wf_op("\t\t\tCPU->flag_X = CPU->flag_C = (src >> (32 - sft)) << C68K_SR_C_SFT;\n");
                wf_op("\t\t\tres = src << sft;\n");
            }
            wf_op("\t\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
            wf_op("\t\t\tCPU->flag_notZ = res;\n", current_bits_mask);

            // write
            _ea_write(current_ea, current_op->reg_sft);
            
            // we do V flag calculation at end for a better register usage
            wf_op("\t\t\tCPU->flag_V = 0;\n");
            wf_op("\t\t\t{\n");
                wf_op("\t\t\t\tu32 msk = (((s32)0x80000000) >> (sft + %d)) & 0x%.8X;\n", 31 - current_sft_mask, current_bits_mask);
                wf_op("\t\t\t\tsrc &= msk;\n");
                wf_op("\t\t\t\tif ((src) && (src != msk)) CPU->flag_V = C68K_SR_V;\n");
            wf_op("\t\t\t}\n");

            quick_terminate_op(6);
        wf_op("\t\t}\n");
        wf_op("\n");

        // special case of shift >= size op
        {
	 unsigned bugfixo = 0;

	 if(0xFF == current_bits_mask) bugfixo = 8;
	 else if(0xFFFF == current_bits_mask) bugfixo = 16;
	 else if(0xFFFFFFFF == current_bits_mask) bugfixo = 32;

	 assert(bugfixo == 8 || bugfixo == 16 || bugfixo == 32);

         wf_op("\t\tif (sft == %d) CPU->flag_C = src << C68K_SR_C_SFT;\n", bugfixo); //current_bits_mask + 1);
	}
        wf_op("\t\telse CPU->flag_C = 0;\n");
        wf_op("\t\tCPU->flag_X = CPU->flag_C;\n");
        wf_op("\t\tif (src) CPU->flag_V = C68K_SR_V;\n");
        wf_op("\t\telse CPU->flag_V = 0;\n");
        
        wf_op("\t\tres = 0;\n");
        // write
        _ea_write(current_ea, current_op->reg_sft);
        
        // others flags
        wf_op("\t\tCPU->flag_N = 0;\n");
        wf_op("\t\tCPU->flag_notZ = 0;\n");

        quick_terminate_op(6);
    wf_op("\t}\n");
    wf_op("\n");

    // special case of (shift == 0)
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = 0;\n");
    wf_op("\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = src;\n");

    terminate_op(6);
}

static void GenLSLD()
{
//    u32 base = get_current_opcode_base();

    current_ea = EA_DREG;               // dst = Dx

    start_all(GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = CPU->D[(Opcode >> %d) & 7] & 0x3F;\n", current_op->reg2_sft);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // if (shift != 0)
    wf_op("\tif (sft)\n");
    wf_op("\t{\n");

        adds_CCnt("sft * 2");

        // if (shift <= size op)
        if (current_size == SIZE_LONG) wf_op("\t\tif (sft < 32)\n");
        else wf_op("\t\tif (sft <= %d)\n", current_sft_mask + 1);
        wf_op("\t\t{\n");

            // op & flag calculation
            if (current_size != SIZE_LONG)
            {
                wf_op("\t\t\tCPU->flag_X = CPU->flag_C = (src << sft) >> %d;\n", (current_sft_mask + 1) - C68K_SR_C_SFT);
                wf_op("\t\t\tres = (src << sft) & 0x%.8X;\n", current_bits_mask);
            }
            else
            {
                wf_op("\t\t\tCPU->flag_X = CPU->flag_C = (src >> (32 - sft)) << C68K_SR_C_SFT;\n");
                wf_op("\t\t\tres = src << sft;\n");
            }
            wf_op("\t\t\tCPU->flag_V = 0;\n");
            wf_op("\t\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
            wf_op("\t\t\tCPU->flag_notZ = res;\n", current_bits_mask);

            // write
            _ea_write(current_ea, current_op->reg_sft);

            quick_terminate_op(6);
        wf_op("\t\t}\n");
        wf_op("\n");

        // special case of shift > size op
        if (current_size == SIZE_LONG)
        {
            wf_op("\t\tif (sft == 32) CPU->flag_C = src << C68K_SR_C_SFT;\n");
            wf_op("\t\telse CPU->flag_C = 0;\n");
            wf_op("\t\tCPU->flag_X = CPU->flag_C;\n");
        }
        else wf_op("\t\tCPU->flag_X = CPU->flag_C = 0;\n");
        wf_op("\t\tCPU->flag_N = 0;\n");
        wf_op("\t\tCPU->flag_notZ = 0;\n");
        wf_op("\t\tCPU->flag_V = 0;\n");
        wf_op("\t\tres = 0;\n");

        // write
        _ea_write(current_ea, current_op->reg_sft);

        quick_terminate_op(6);
    wf_op("\t}\n");
    wf_op("\n");

    // special case of (shift == 0)
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = 0;\n");
    wf_op("\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = src;\n");

    terminate_op(6);
}

static void GenROXLD()
{
//    u32 base = get_current_opcode_base();

    current_ea = EA_DREG;               // dst = Dx

    start_all(GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = CPU->D[(Opcode >> %d) & 7] & 0x3F;\n", current_op->reg2_sft);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // if (shift != 0)
    wf_op("\tif (sft)\n");
    wf_op("\t{\n");

        adds_CCnt("sft * 2");

        wf_op("\t\tsft %%= %d;\n", current_sft_mask + 2);
        wf_op("\n");
        
        // op & C/X flags calculation
        if (current_size != SIZE_LONG)
        {
            wf_op("\t\tsrc |= (CPU->flag_X & C68K_SR_X) << %d;\n", (current_sft_mask + 1) - C68K_SR_X_SFT);
            wf_op("\t\tres = (src << sft) | (src >> (%d - sft));\n", current_sft_mask + 2);
            wf_op("\t\tCPU->flag_X = CPU->flag_C = res >> %d;\n", (current_sft_mask + 1) - C68K_SR_X_SFT);
        }
        else
        {
            wf_op("\t\tif (sft != 0)\n");
            wf_op("\t\t{\n");
            wf_op("\t\t\tif (sft == 1) res = (src << 1) | ((CPU->flag_X >> ((C68K_SR_X_SFT + 1) - 1)) & 1);\n");
            wf_op("\t\t\telse res = (src << sft) | (src >> (33 - sft)) | (((CPU->flag_X >> ((C68K_SR_X_SFT + 1) - 1)) & 1) << (sft - 1));\n");
            wf_op("\t\t\tCPU->flag_X = (src >> (32 - sft)) << C68K_SR_X_SFT;\n");
            wf_op("\t\t}\n");
            wf_op("\t\telse res = src;\n");
            wf_op("\t\tCPU->flag_C = CPU->flag_X;\n");
        }

        // V / N / Z flags calculation
        wf_op("\t\tCPU->flag_V = 0;\n");
        wf_op("\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
        if (current_size == SIZE_LONG) wf_op("\t\tCPU->flag_notZ = res;\n");
        else wf_op("\t\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

        // write
        _ea_write(current_ea, current_op->reg_sft);

        quick_terminate_op(6);
    wf_op("\t}\n");
    wf_op("\n");

    // special case of (shift == 0)
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = CPU->flag_X;\n");
    wf_op("\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = src;\n");

    terminate_op(6);
}

static void GenROLD()
{
//    u32 base = get_current_opcode_base();

    current_ea = EA_DREG;               // dst = Dx

    start_all(GEN_RES | GEN_SRC);

    if (current_size == SIZE_LONG) current_cycle += 2;

    wf_op("\tu32 sft;\n");
    wf_op("\n");
    wf_op("\tsft = CPU->D[(Opcode >> %d) & 7] & 0x3F;\n", current_op->reg2_sft);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // if (shift != 0)
    wf_op("\tif (sft)\n");
    wf_op("\t{\n");

        adds_CCnt("sft * 2");

        // if ((shift & size op) != 0)
        wf_op("\t\tif (sft &= 0x%.2X)\n", current_sft_mask);
        wf_op("\t\t{\n");

            // op & flag calculation
            if (current_size != SIZE_LONG)
            {
                wf_op("\t\t\tCPU->flag_C = (src << sft) >> %d;\n", (current_sft_mask + 1) - C68K_SR_C_SFT);
                wf_op("\t\t\tres = ((src << sft) | (src >> (%d - sft))) & 0x%.8X;\n", current_sft_mask + 1, current_bits_mask);
            }
            else
            {
                wf_op("\t\t\tCPU->flag_C = (src >> (32 - sft)) << C68K_SR_C_SFT;\n");
                wf_op("\t\t\tres = (src << sft) | (src >> (%d - sft));\n", current_sft_mask + 1);
            }
            wf_op("\t\t\tCPU->flag_V = 0;\n");
            wf_op("\t\t\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
            wf_op("\t\t\tCPU->flag_notZ = res;\n");

            // write
            _ea_write(current_ea, current_op->reg_sft);

            quick_terminate_op(6);
        wf_op("\t\t}\n");
        wf_op("\n");

        // special case of ((shift & size op) == 0)
        wf_op("\t\tCPU->flag_V = 0;\n");
        wf_op("\t\tCPU->flag_C = src << C68K_SR_C_SFT;\n");
        wf_op("\t\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
        wf_op("\t\tCPU->flag_notZ = src;\n");

        quick_terminate_op(6);
    wf_op("\t}\n");
    wf_op("\n");

    // special case of (shift == 0)
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = 0;\n");
    wf_op("\tCPU->flag_N = src >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = src;\n");

    terminate_op(6);
}

static void GenASR()
{
    set_current_size(SIZE_WORD);        // dst = mem (word operation)
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;\n");
    wf_op("\tres = (src >> 1) | (src & (1 << %d));\n", current_sft_mask);
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res;\n");
    
    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

static void GenLSR()
{
    set_current_size(SIZE_WORD);        // dst = mem (word operation)
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_N = CPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_X = CPU->flag_C = src << C68K_SR_C_SFT;\n");
    wf_op("\tres = src >> 1;\n");
    wf_op("\tCPU->flag_notZ = res;\n");

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

static void GenROXR()
{
    set_current_size(SIZE_WORD);        // dst = mem (word operation)
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tres = (src >> 1) | ((CPU->flag_X & C68K_SR_X) << %d);\n", current_sft_mask - C68K_SR_X_SFT);
    wf_op("\tCPU->flag_C = CPU->flag_X = src << C68K_SR_C_SFT;\n");
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res;\n");

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

static void GenROR()
{
    set_current_size(SIZE_WORD);        // dst = mem (word operation)
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = src << C68K_SR_C_SFT;\n");
    wf_op("\tres = (src >> 1) | (src << %d);\n", current_sft_mask);
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

static void GenASL()
{
    set_current_size(SIZE_WORD);        // dst = mem (word operation)
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_X = CPU->flag_C = src >> %d;\n", current_sft_mask - C68K_SR_C_SFT);
    wf_op("\tres = src << 1;\n");
    wf_op("\tCPU->flag_V = (src ^ res) >> %d;\n", current_sft_mask - C68K_SR_V_SFT);
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

static void GenLSL()
{
    set_current_size(SIZE_WORD);        // dst = mem (word operation)
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_X = CPU->flag_C = src >> %d;\n", current_sft_mask - C68K_SR_C_SFT);
    wf_op("\tres = src << 1;\n");
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

static void GenROXL()
{
    set_current_size(SIZE_WORD);        // dst = mem (word operation)
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tres = (src << 1) | ((CPU->flag_X & C68K_SR_X) >> %d);\n", C68K_SR_X_SFT);
    wf_op("\tCPU->flag_X = CPU->flag_C = src >> %d;\n", current_sft_mask - C68K_SR_C_SFT);
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

static void GenROL()
{
    set_current_size(SIZE_WORD);        // dst = mem (word operation)
    start_all(GEN_ADR | GEN_RES | GEN_SRC);

    // read
    _ea_calc(current_ea, current_op->reg_sft);
    _ea_read_src(current_ea, current_op->reg_sft);

    // op & flag calculation
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_C = src >> %d;\n", current_sft_mask - C68K_SR_C_SFT);
    wf_op("\tres = (src << 1) | (src >> %d);\n", current_sft_mask);
    wf_op("\tCPU->flag_N = res >> %d;\n", current_sft_mask - C68K_SR_N_SFT);
    wf_op("\tCPU->flag_notZ = res & 0x%.8X;\n", current_bits_mask);

    // write
    _ea_write(current_ea, current_op->reg_sft);

    terminate_op(8);
}

// main function
/////////////////
int main(int argc, char *argv[])
{
    u32 i;
    u32 s;
    u32 smax;
    
    // clear opcode files
    for(i = 0; i < 0x10; i++)
    {
        char fn[16];
        
        sprintf(fn, "c68k_op%.1X.inc", (int)i);
        opcode_file = fopen(fn, "wt");
        if (opcode_file != NULL)
        {
            fclose(opcode_file);
            opcode_file = NULL;
        }
    }

    // init opcode jump table
    ini_file = fopen("c68k_ini.inc", "wt");
#ifndef C68K_NO_JUMP_TABLE
#ifdef C68K_CONST_JUMP_TABLE
    for(i = 0; i < 0x10000; i++) op_jump_table[i] = OP_ILLEGAL;
#else
    // defaut ILLEGAL instruction
    gen_jumptable(0x0000, 0x0000, 0xFFFF, 1, 0, 0, 0, 0, 0, 0, 0x4AFC);
#endif
#endif
    // generate opcode files
    for(i = 0; i < OP_INFO_TABLE_LEN; i++)
    {
        current_op = &(op_info_table[i]);
        if (prepare_generate()) return 1;

        // s = size to start
        current_size = 0;
        smax = SIZE_LONG;
        if (current_op->size_type == 0) smax = 0;
        else if (current_op->size_type == 1) current_size = 1;
        
        for(s = current_size; s <= smax; s++)
        {
            if (current_op->eam_sft != -1)
            {
                for(current_ea = 0; current_ea <= EA_ADEC7; current_ea++)
                {
                    if (!has_ea(current_ea)) continue;
                    current_eam = _ea_to_eamreg(current_ea) >> 3;
                    current_reg = _ea_to_eamreg(current_ea) & 7;
                    
                    if (op_info_table[i].eam2_sft != -1)
                    {
                        for(current_ea2 = 0; current_ea2 <= EA_ADEC7; current_ea2++)
                        {
                            if (!has_ea2(current_ea2)) continue;
                            current_eam2 = _ea_to_eamreg(current_ea2) >> 3;
                            current_reg2 = _ea_to_eamreg(current_ea2) & 7;
                            
                            set_current_size(s);
                            current_op->genfunc();
                        }
                    }
                    else
                    {
                        current_reg2 = 0;
                        set_current_size(s);
                        current_op->genfunc();
                    }
                }
            }
            else
            {
                current_reg = 0;
                set_current_size(s);
                current_op->genfunc();
            }
        }
    }

    // generate jumptable file
#ifdef C68K_CONST_JUMP_TABLE
    if (ini_file != NULL)
    {
        fprintf(ini_file, "\tstatic const void *JumpTable[0x10000] =\n");
        fprintf(ini_file, "\t{\n");
        
        for(i = 0; i < (0x10000 - 4); i += 4)
            fprintf(ini_file, "\t\t&&OP_0x%.4X, &&OP_0x%.4X, &&OP_0x%.4X, &&OP_0x%.4X,\n", op_jump_table[i + 0], op_jump_table[i + 1], op_jump_table[i + 2], op_jump_table[i + 3]);
        fprintf(ini_file, "\t\t&&OP_0x%.4X, &&OP_0x%.4X, &&OP_0x%.4X, &&OP_0x%.4X\n", op_jump_table[0xFFFC], op_jump_table[0xFFFD], op_jump_table[0xFFFE], op_jump_table[0xFFFF]);
        
        fprintf(ini_file, "\t};\n\n");
    }
#endif

    // close handle
    if (ini_file != NULL) fclose(ini_file);
    if (opcode_file != NULL) fclose(opcode_file);

	return 0;
}
#endif
