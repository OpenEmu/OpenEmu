/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - gcop1_d.c                                               *
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

void genadd_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[119]);
#endif
#ifdef INTERPRET_ADD_D
    gencallinterp((unsigned long long)cached_interpreter_table.ADD_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fadd_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

void gensub_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[120]);
#endif
#ifdef INTERPRET_SUB_D
   gencallinterp((unsigned long long)cached_interpreter_table.SUB_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fsub_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

void genmul_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[121]);
#endif
#ifdef INTERPRET_MUL_D
   gencallinterp((unsigned long long)cached_interpreter_table.MUL_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fmul_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

void gendiv_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[122]);
#endif
#ifdef INTERPRET_DIV_D
   gencallinterp((unsigned long long)cached_interpreter_table.DIV_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fdiv_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

void gensqrt_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[123]);
#endif
#ifdef INTERPRET_SQRT_D
   gencallinterp((unsigned long long)cached_interpreter_table.SQRT_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fsqrt();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

void genabs_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[124]);
#endif
#ifdef INTERPRET_ABS_D
   gencallinterp((unsigned long long)cached_interpreter_table.ABS_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fabs_();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

void genmov_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[125]);
#endif
#ifdef INTERPRET_MOV_D
   gencallinterp((unsigned long long)cached_interpreter_table.MOV_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   mov_reg32_preg64(EBX, RAX);
   mov_reg32_preg64pimm32(ECX, RAX, 4);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   mov_preg64_reg32(RAX, EBX);
   mov_preg64pimm32_reg32(RAX, 4, ECX);
#endif
}

void genneg_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[126]);
#endif
#ifdef INTERPRET_NEG_D
   gencallinterp((unsigned long long)cached_interpreter_table.NEG_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fchs();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fstp_preg64_qword(RAX);
#endif
}

void genround_l_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[127]);
#endif
#ifdef INTERPRET_ROUND_L_D
   gencallinterp((unsigned long long)cached_interpreter_table.ROUND_L_D, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&round_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void gentrunc_l_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[128]);
#endif
#ifdef INTERPRET_TRUNC_L_D
   gencallinterp((unsigned long long)cached_interpreter_table.TRUNC_L_D, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&trunc_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genceil_l_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[129]);
#endif
#ifdef INTERPRET_CEIL_L_D
   gencallinterp((unsigned long long)cached_interpreter_table.CEIL_L_D, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&ceil_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genfloor_l_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[130]);
#endif
#ifdef INTERPRET_FLOOR_L_D
   gencallinterp((unsigned long long)cached_interpreter_table.FLOOR_L_D, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&floor_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genround_w_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[127]);
#endif
#ifdef INTERPRET_ROUND_W_D
   gencallinterp((unsigned long long)cached_interpreter_table.ROUND_W_D, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&round_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void gentrunc_w_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[128]);
#endif
#ifdef INTERPRET_TRUNC_W_D
   gencallinterp((unsigned long long)cached_interpreter_table.TRUNC_W_D, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&trunc_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genceil_w_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[129]);
#endif
#ifdef INTERPRET_CEIL_W_D
   gencallinterp((unsigned long long)cached_interpreter_table.CEIL_W_D, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&ceil_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void genfloor_w_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[130]);
#endif
#ifdef INTERPRET_FLOOR_W_D
   gencallinterp((unsigned long long)cached_interpreter_table.FLOOR_W_D, 0);
#else
   gencheck_cop1_unusable();
   fldcw_m16rel((unsigned short*)&floor_mode);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
   fldcw_m16rel((unsigned short*)&rounding_mode);
#endif
}

void gencvt_s_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[117]);
#endif
#ifdef INTERPRET_CVT_S_D
   gencallinterp((unsigned long long)cached_interpreter_table.CVT_S_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fstp_preg64_dword(RAX);
#endif
}

void gencvt_w_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[117]);
#endif
#ifdef INTERPRET_CVT_W_D
   gencallinterp((unsigned long long)cached_interpreter_table.CVT_W_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_simple[dst->f.cf.fd]));
   fistp_preg64_dword(RAX);
#endif
}

void gencvt_l_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[117]);
#endif
#ifdef INTERPRET_CVT_L_D
   gencallinterp((unsigned long long)cached_interpreter_table.CVT_L_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fd]));
   fistp_preg64_qword(RAX);
#endif
}

void genc_f_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_F_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_F_D, 0);
#else
   gencheck_cop1_unusable();
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000);
#endif
}

void genc_un_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_UN_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_UN_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(13);
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
   jmp_imm_short(11); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
#endif
}

void genc_eq_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_EQ_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_EQ_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jne_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ueq_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_UEQ_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_UEQ_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   jne_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_olt_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_OLT_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_OLT_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jae_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ult_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_ULT_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_ULT_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   jae_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ole_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_OLE_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_OLE_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   ja_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ule_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_ULE_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_ULE_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fucomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   ja_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_sf_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_SF_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_SF_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000);
#endif
}

void genc_ngle_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_NGLE_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_NGLE_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(13);
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
   jmp_imm_short(11); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
#endif
}

void genc_seq_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_SEQ_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_SEQ_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jne_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ngl_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_NGL_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_NGL_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   jne_rj(13);
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_lt_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_LT_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_LT_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jae_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_nge_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_NGE_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_NGE_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   jae_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_le_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_LE_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_LE_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   ja_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

void genc_ngt_d(void)
{
#if defined(COUNT_INSTR)
   inc_m32rel(&instr_count[118]);
#endif
#ifdef INTERPRET_C_NGT_D
   gencallinterp((unsigned long long)cached_interpreter_table.C_NGT_D, 0);
#else
   gencheck_cop1_unusable();
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.ft]));
   fld_preg64_qword(RAX);
   mov_xreg64_m64rel(RAX, (unsigned long long *)(&reg_cop1_double[dst->f.cf.fs]));
   fld_preg64_qword(RAX);
   fcomip_fpreg(1);
   ffree_fpreg(0);
   jp_rj(15);
   ja_rj(13); // 2
   or_m32rel_imm32((unsigned int*)&FCR31, 0x800000); // 11
   jmp_imm_short(11); // 2
   and_m32rel_imm32((unsigned int*)&FCR31, ~0x800000); // 11
#endif
}

