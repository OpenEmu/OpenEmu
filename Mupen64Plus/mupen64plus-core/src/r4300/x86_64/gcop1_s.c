/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gcop1_s.c                                               *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2007 Richard Goedeken (Richard42)                       *
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

#include <stdio.h>

#include "assemble.h"
#include "interpret.h"

#include "r4300/recomph.h"
#include "r4300/r4300.h"
#include "r4300/ops.h"
#include "r4300/macros.h"

void genadd_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[119]);
#endif
#ifdef INTERPRET_ADD_S
    gencallinterp((unsigned long long)cached_interpreter_table.ADD_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fadd_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void gensub_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[120]);
#endif
#ifdef INTERPRET_SUB_S
    gencallinterp((unsigned long long)cached_interpreter_table.SUB_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fsub_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void genmul_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[121]);
#endif
#ifdef INTERPRET_MUL_S
    gencallinterp((unsigned long long)cached_interpreter_table.MUL_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fmul_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void gendiv_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[122]);
#endif
#ifdef INTERPRET_DIV_S
    gencallinterp((unsigned long long)cached_interpreter_table.DIV_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fdiv_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void gensqrt_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[123]);
#endif
#ifdef INTERPRET_SQRT_S
   gencallinterp((unsigned long long)cached_interpreter_table.SQRT_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fsqrt();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void genabs_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[124]);
#endif
#ifdef INTERPRET_ABS_S
   gencallinterp((unsigned long long)cached_interpreter_table.ABS_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fabs_();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void genmov_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[125]);
#endif
#ifdef INTERPRET_MOV_S
   gencallinterp((unsigned long long)cached_interpreter_table.MOV_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   mov_reg32_preg64(EBX, RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   mov_preg64_reg32(RAX, EBX);
#endif
}

void genneg_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[126]);
#endif
#ifdef INTERPRET_NEG_S
   gencallinterp((unsigned long long)cached_interpreter_table.NEG_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fchs();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void genround_l_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[127]);
#endif
#ifdef INTERPRET_ROUND_L_S
   gencallinterp((unsigned long long)cached_interpreter_table.ROUND_L_S, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&round_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void gentrunc_l_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[128]);
#endif
#ifdef INTERPRET_TRUNC_L_S
   gencallinterp((unsigned long long)cached_interpreter_table.TRUNC_L_S, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&trunc_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genceil_l_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[129]);
#endif
#ifdef INTERPRET_CEIL_L_S
   gencallinterp((unsigned long long)cached_interpreter_table.CEIL_L_S, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&ceil_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genfloor_l_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[130]);
#endif
#ifdef INTERPRET_FLOOR_L_S
   gencallinterp((unsigned long long)cached_interpreter_table.FLOOR_L_S, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&floor_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genround_w_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[127]);
#endif
#ifdef INTERPRET_ROUND_W_S
   gencallinterp((unsigned long long)cached_interpreter_table.ROUND_W_S, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&round_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void gentrunc_w_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[128]);
#endif
#ifdef INTERPRET_TRUNC_W_S
   gencallinterp((unsigned long long)cached_interpreter_table.TRUNC_W_S, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&trunc_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genceil_w_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[129]);
#endif
#ifdef INTERPRET_CEIL_W_S
   gencallinterp((unsigned long long)cached_interpreter_table.CEIL_W_S, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&ceil_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genfloor_w_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[130]);
#endif
#ifdef INTERPRET_FLOOR_W_S
   gencallinterp((unsigned long long)cached_interpreter_table.FLOOR_W_S, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&floor_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void gencvt_d_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[117]);
#endif
#ifdef INTERPRET_CVT_D_S
   gencallinterp((unsigned long long)cached_interpreter_table.CVT_D_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

void gencvt_w_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[117]);
#endif
#ifdef INTERPRET_CVT_W_S
   gencallinterp((unsigned long long)cached_interpreter_table.CVT_W_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
#endif
}

void gencvt_l_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[117]);
#endif
#ifdef INTERPRET_CVT_L_S
   gencallinterp((unsigned long long)cached_interpreter_table.CVT_L_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
#endif
}

void genc_f_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_F_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_F_S, 0);
#else
   gencheck_cop1_unusable();
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000);
#endif
}

void genc_un_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_UN_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_UN_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(13);
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
   jmp_imm_short(11); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
#endif
}

void genc_eq_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_EQ_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_EQ_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jne_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ueq_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_UEQ_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_UEQ_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   jne_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_olt_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_OLT_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_OLT_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jae_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ult_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_ULT_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_ULT_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   jae_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ole_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_OLE_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_OLE_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   ja_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ule_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_ULE_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_ULE_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   ja_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_sf_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_SF_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_SF_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000);
#endif
}

void genc_ngle_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_NGLE_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_NGLE_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(13);
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
   jmp_imm_short(11); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
#endif
}

void genc_seq_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_SEQ_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_SEQ_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jne_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ngl_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_NGL_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_NGL_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   jne_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_lt_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_LT_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_LT_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jae_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_nge_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_NGE_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_NGE_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   jae_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_le_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_LE_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_LE_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   ja_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ngt_s(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_NGT_S
   gencallinterp((unsigned long long)cached_interpreter_table.C_NGT_S, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.ft]));
   fld_preg64_dword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fs]));
   fld_preg64_dword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   ja_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

