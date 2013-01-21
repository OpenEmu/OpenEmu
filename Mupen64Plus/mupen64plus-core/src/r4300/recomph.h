/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - recomph.h                                               *
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

#ifndef RECOMPH_H
#define RECOMPH_H

#include "recomp.h"

#if defined(COUNT_INSTR)
extern unsigned int instr_count[132];
extern unsigned int instr_type[131];
extern char instr_name[][10];
extern char instr_typename[][20];
#endif

extern int code_length;
extern int max_code_length;
extern unsigned char **inst_pointer;
extern precomp_block* dst_block;
extern int fast_memory;
extern int src;   /* opcode of r4300 instruction being recompiled */

#if defined(PROFILE_R4300)
  #include <stdio.h>
  extern FILE *pfProfile;
#endif

void passe2(precomp_instr *dest, int start, int end, precomp_block* block);
void init_assembler(void *block_jumps_table, int block_jumps_number, void *block_riprel_table, int block_riprel_number);
void free_assembler(void **block_jumps_table, int *block_jumps_number, void **block_riprel_table, int *block_riprel_number);

void gencallinterp(unsigned long addr, int jump);

void genupdate_system(int type);
void genbnel(void);
void genblezl(void);
void genlw(void);
void genlbu(void);
void genlhu(void);
void gensb(void);
void gensh(void);
void gensw(void);
void gencache(void);
void genlwc1(void);
void genld(void);
void gensd(void);
void genbeq(void);
void genbne(void);
void genblez(void);
void genaddi(void);
void genaddiu(void);
void genslti(void);
void gensltiu(void);
void genandi(void);
void genori(void);
void genxori(void);
void genlui(void);
void genbeql(void);
void genmul_s(void);
void gendiv_s(void);
void gencvt_d_s(void);
void genadd_d(void);
void gentrunc_w_d(void);
void gencvt_s_w(void);
void genmfc1(void);
void gencfc1(void);
void genmtc1(void);
void genctc1(void);
void genj(void);
void genjal(void);
void genslt(void);
void gensltu(void);
void gendsll32(void);
void gendsra32(void);
void genbgez(void);
void genbgezl(void);
void genbgezal(void);
void gentlbwi(void);
void generet(void);
void genmfc0(void);
void genadd_s(void);
void genmult(void);
void genmultu(void);
void genmflo(void);
void genmtlo(void);
void gendiv(void);
void gendmultu(void);
void genddivu(void);
void genadd(void);
void genaddu(void);
void gensubu(void);
void genand(void);
void genor(void);
void genxor(void);
void genreserved(void);
void gennop(void);
void gensll(void);
void gensrl(void);
void gensra(void);
void gensllv(void);
void gensrlv(void);
void genjr(void);
void genni(void);
void genmfhi(void);
void genmthi(void);
void genmtc0(void);
void genbltz(void);
void genlwl(void);
void genswl(void);
void gentlbp(void);
void gentlbr(void);
void genswr(void);
void genlwr(void);
void gensrav(void);
void genbgtz(void);
void genlb(void);
void genswc1(void);
void genldc1(void);
void gencvt_d_w(void);
void genmul_d(void);
void gensub_d(void);
void gendiv_d(void);
void gencvt_s_d(void);
void genmov_s(void);
void genc_le_s(void);
void genbc1t(void);
void gentrunc_w_s(void);
void genbc1tl(void);
void genc_lt_s(void);
void genbc1fl(void);
void genneg_s(void);
void genc_le_d(void);
void genbgezal_idle(void);
void genj_idle(void);
void genbeq_idle(void);
void genlh(void);
void genmov_d(void);
void genc_lt_d(void);
void genbc1f(void);
void gennor(void);
void genneg_d(void);
void gensub(void);
void genblez_idle(void);
void gendivu(void);
void gencvt_w_s(void);
void genbltzl(void);
void gensdc1(void);
void genc_eq_s(void);
void genjalr(void);
void gensub_s(void);
void gensqrt_s(void);
void genc_eq_d(void);
void gencvt_w_d(void);
void genfin_block(void);
void genddiv(void);
void gendaddiu(void);
void genbgtzl(void);
void gendsrav(void);
void gendsllv(void);
void gencvt_s_l(void);
void gendmtc1(void);
void gendsrlv(void);
void gendsra(void);
void gendmult(void);
void gendsll(void);
void genabs_s(void);
void gensc(void);
void gennotcompiled(void);
void genjal_idle(void);
void genjal_out(void);
void genbeq_out(void);
void gensyscall(void);
void gensync(void);
void gendadd(void);
void gendaddu(void);
void gendsub(void);
void gendsubu(void);
void genteq(void);
void gendsrl(void);
void gendsrl32(void);
void genbltz_idle(void);
void genbltz_out(void);
void genbgez_idle(void);
void genbgez_out(void);
void genbltzl_idle(void);
void genbltzl_out(void);
void genbgezl_idle(void);
void genbgezl_out(void);
void genbltzal_idle(void);
void genbltzal_out(void);
void genbltzal(void);
void genbgezal_out(void);
void genbltzall_idle(void);
void genbltzall_out(void);
void genbltzall(void);
void genbgezall_idle(void);
void genbgezall_out(void);
void genbgezall(void);
void gentlbwr(void);
void genbc1f_idle(void);
void genbc1f_out(void);
void genbc1t_idle(void);
void genbc1t_out(void);
void genbc1fl_idle(void);
void genbc1fl_out(void);
void genbc1tl_idle(void);
void genbc1tl_out(void);
void genround_l_s(void);
void gentrunc_l_s(void);
void genceil_l_s(void);
void genfloor_l_s(void);
void genround_w_s(void);
void genceil_w_s(void);
void genfloor_w_s(void);
void gencvt_l_s(void);
void genc_f_s(void);
void genc_un_s(void);
void genc_ueq_s(void);
void genc_olt_s(void);
void genc_ult_s(void);
void genc_ole_s(void);
void genc_ule_s(void);
void genc_sf_s(void);
void genc_ngle_s(void);
void genc_seq_s(void);
void genc_ngl_s(void);
void genc_nge_s(void);
void genc_ngt_s(void);
void gensqrt_d(void);
void genabs_d(void);
void genround_l_d(void);
void gentrunc_l_d(void);
void genceil_l_d(void);
void genfloor_l_d(void);
void genround_w_d(void);
void genceil_w_d(void);
void genfloor_w_d(void);
void gencvt_l_d(void);
void genc_f_d(void);
void genc_un_d(void);
void genc_ueq_d(void);
void genc_olt_d(void);
void genc_ult_d(void);
void genc_ole_d(void);
void genc_ule_d(void);
void genc_sf_d(void);
void genc_ngle_d(void);
void genc_seq_d(void);
void genc_ngl_d(void);
void genc_nge_d(void);
void genc_ngt_d(void);
void gencvt_d_l(void);
void gendmfc1(void);
void genj_out(void);
void genbne_idle(void);
void genbne_out(void);
void genblez_out(void);
void genbgtz_idle(void);
void genbgtz_out(void);
void genbeql_idle(void);
void genbeql_out(void);
void genbnel_idle(void);
void genbnel_out(void);
void genblezl_idle(void);
void genblezl_out(void);
void genbgtzl_idle(void);
void genbgtzl_out(void);
void gendaddi(void);
void genldl(void);
void genldr(void);
void genlwu(void);
void gensdl(void);
void gensdr(void);
void genlink_subblock(void);
void gendelayslot(void);
void gencheck_interupt_reg(void);
void gentest(void);
void gentest_out(void);
void gentest_idle(void);
void gentestl(void);
void gentestl_out(void);
void gencheck_cop1_unusable(void);
void genll(void);

#ifdef COMPARE_CORE
void gendebug(void);
#endif

#endif

