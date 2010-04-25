/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - memory.h                                                *
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

#ifndef MEMORY_H
#define MEMORY_H

#include "osal/preproc.h"

int init_memory(int DoByteSwap);
void free_memory(void);
#define read_word_in_memory() readmem[address>>16]()
#define read_byte_in_memory() readmemb[address>>16]()
#define read_hword_in_memory() readmemh[address>>16]()
#define read_dword_in_memory() readmemd[address>>16]()
#define write_word_in_memory() writemem[address>>16]()
#define write_byte_in_memory() writememb[address >>16]()
#define write_hword_in_memory() writememh[address >>16]()
#define write_dword_in_memory() writememd[address >>16]()
extern unsigned int SP_DMEM[0x1000/4*2];
extern unsigned char *SP_DMEMb;
extern unsigned int *SP_IMEM;
extern unsigned char *SP_IMEMb;
extern unsigned int PIF_RAM[0x40/4];
extern unsigned char *PIF_RAMb;

extern ALIGN(16, unsigned int rdram[0x800000/4]);

extern unsigned int address, word;
extern unsigned char cpu_byte;
extern unsigned short hword;
extern unsigned long long dword, *rdword;

extern void (*readmem[0x10000])(void);
extern void (*readmemb[0x10000])(void);
extern void (*readmemh[0x10000])(void);
extern void (*readmemd[0x10000])(void);
extern void (*writemem[0x10000])(void);
extern void (*writememb[0x10000])(void);
extern void (*writememh[0x10000])(void);
extern void (*writememd[0x10000])(void);

typedef struct _RDRAM_register
{
   unsigned int rdram_config;
   unsigned int rdram_device_id;
   unsigned int rdram_delay;
   unsigned int rdram_mode;
   unsigned int rdram_ref_interval;
   unsigned int rdram_ref_row;
   unsigned int rdram_ras_interval;
   unsigned int rdram_min_interval;
   unsigned int rdram_addr_select;
   unsigned int rdram_device_manuf;
} RDRAM_register;

typedef struct _SP_register
{
   unsigned int sp_mem_addr_reg;
   unsigned int sp_dram_addr_reg;
   unsigned int sp_rd_len_reg;
   unsigned int sp_wr_len_reg;
   unsigned int w_sp_status_reg;
   unsigned int sp_status_reg;
   char halt;
   char broke;
   char dma_busy;
   char dma_full;
   char io_full;
   char single_step;
   char intr_break;
   char signal0;
   char signal1;
   char signal2;
   char signal3;
   char signal4;
   char signal5;
   char signal6;
   char signal7;
   unsigned int sp_dma_full_reg;
   unsigned int sp_dma_busy_reg;
   unsigned int sp_semaphore_reg;
} SP_register;

typedef struct _RSP_register
{
   unsigned int rsp_pc;
   unsigned int rsp_ibist;
} RSP_register;

typedef struct _DPC_register
{
   unsigned int dpc_start;
   unsigned int dpc_end;
   unsigned int dpc_current;
   unsigned int w_dpc_status;
   unsigned int dpc_status;
   char xbus_dmem_dma;
   char freeze;
   char flush;
   char start_glck;
   char tmem_busy;
   char pipe_busy;
   char cmd_busy;
   char cbuf_busy;
   char dma_busy;
   char end_valid;
   char start_valid;
   unsigned int dpc_clock;
   unsigned int dpc_bufbusy;
   unsigned int dpc_pipebusy;
   unsigned int dpc_tmem;
} DPC_register;

typedef struct _DPS_register
{
   unsigned int dps_tbist;
   unsigned int dps_test_mode;
   unsigned int dps_buftest_addr;
   unsigned int dps_buftest_data;
} DPS_register;

typedef struct _mips_register
{
   unsigned int w_mi_init_mode_reg;
   unsigned int mi_init_mode_reg;
   char init_length;
   char init_mode;
   char ebus_test_mode;
   char RDRAM_reg_mode;
   unsigned int mi_version_reg;
   unsigned int mi_intr_reg;
   unsigned int mi_intr_mask_reg;
   unsigned int w_mi_intr_mask_reg;
   char SP_intr_mask;
   char SI_intr_mask;
   char AI_intr_mask;
   char VI_intr_mask;
   char PI_intr_mask;
   char DP_intr_mask;
} mips_register;

typedef struct _VI_register
{
   unsigned int vi_status;
   unsigned int vi_origin;
   unsigned int vi_width;
   unsigned int vi_v_intr;
   unsigned int vi_current;
   unsigned int vi_burst;
   unsigned int vi_v_sync;
   unsigned int vi_h_sync;
   unsigned int vi_leap;
   unsigned int vi_h_start;
   unsigned int vi_v_start;
   unsigned int vi_v_burst;
   unsigned int vi_x_scale;
   unsigned int vi_y_scale;
   unsigned int vi_delay;
} VI_register;

typedef struct _AI_register
{
   unsigned int ai_dram_addr;
   unsigned int ai_len;
   unsigned int ai_control;
   unsigned int ai_status;
   unsigned int ai_dacrate;
   unsigned int ai_bitrate;
   unsigned int next_delay;
   unsigned int next_len;
   unsigned int current_delay;
   unsigned int current_len;
} AI_register;

typedef struct _PI_register
{
   unsigned int pi_dram_addr_reg;
   unsigned int pi_cart_addr_reg;
   unsigned int pi_rd_len_reg;
   unsigned int pi_wr_len_reg;
   unsigned int read_pi_status_reg;
   unsigned int pi_bsd_dom1_lat_reg;
   unsigned int pi_bsd_dom1_pwd_reg;
   unsigned int pi_bsd_dom1_pgs_reg;
   unsigned int pi_bsd_dom1_rls_reg;
   unsigned int pi_bsd_dom2_lat_reg;
   unsigned int pi_bsd_dom2_pwd_reg;
   unsigned int pi_bsd_dom2_pgs_reg;
   unsigned int pi_bsd_dom2_rls_reg;
} PI_register;

typedef struct _RI_register
{
   unsigned int ri_mode;
   unsigned int ri_config;
   unsigned int ri_current_load;
   unsigned int ri_select;
   unsigned int ri_refresh;
   unsigned int ri_latency;
   unsigned int ri_error;
   unsigned int ri_werror;
} RI_register;

typedef struct _SI_register
{
   unsigned int si_dram_addr;
   unsigned int si_pif_addr_rd64b;
   unsigned int si_pif_addr_wr64b;
   unsigned int si_stat;
} SI_register;

extern RDRAM_register rdram_register;
extern PI_register pi_register;
extern mips_register MI_register;
extern SP_register sp_register;
extern SI_register si_register;
extern VI_register vi_register;
extern RSP_register rsp_register;
extern RI_register ri_register;
extern AI_register ai_register;
extern DPC_register dpc_register;
extern DPS_register dps_register;

extern unsigned char *rdramb;

#ifndef _BIG_ENDIAN
#define sl(mot) \
( \
((mot & 0x000000FF) << 24) | \
((mot & 0x0000FF00) <<  8) | \
((mot & 0x00FF0000) >>  8) | \
((mot & 0xFF000000) >> 24) \
)

#define S8 3
#define S16 2
#define Sh16 1

#else

#define sl(mot) mot
#define S8 0
#define S16 0
#define Sh16 0

#endif

void read_nothing(void);
void read_nothingh(void);
void read_nothingb(void);
void read_nothingd(void);
void read_nomem(void);
void read_nomemb(void);
void read_nomemh(void);
void read_nomemd(void);
void read_rdram(void);
void read_rdramb(void);
void read_rdramh(void);
void read_rdramd(void);
void read_rdramFB(void);
void read_rdramFBb(void);
void read_rdramFBh(void);
void read_rdramFBd(void);
void read_rdramreg(void);
void read_rdramregb(void);
void read_rdramregh(void);
void read_rdramregd(void);
void read_rsp_mem(void);
void read_rsp_memb(void);
void read_rsp_memh(void);
void read_rsp_memd(void);
void read_rsp_reg(void);
void read_rsp_regb(void);
void read_rsp_regh(void);
void read_rsp_regd(void);
void read_rsp(void);
void read_rspb(void);
void read_rsph(void);
void read_rspd(void);
void read_dp(void);
void read_dpb(void);
void read_dph(void);
void read_dpd(void);
void read_dps(void);
void read_dpsb(void);
void read_dpsh(void);
void read_dpsd(void);
void read_mi(void);
void read_mib(void);
void read_mih(void);
void read_mid(void);
void read_vi(void);
void read_vib(void);
void read_vih(void);
void read_vid(void);
void read_ai(void);
void read_aib(void);
void read_aih(void);
void read_aid(void);
void read_pi(void);
void read_pib(void);
void read_pih(void);
void read_pid(void);
void read_ri(void);
void read_rib(void);
void read_rih(void);
void read_rid(void);
void read_si(void);
void read_sib(void);
void read_sih(void);
void read_sid(void);
void read_flashram_status(void);
void read_flashram_statusb(void);
void read_flashram_statush(void);
void read_flashram_statusd(void);
void read_rom(void);
void read_romb(void);
void read_romh(void);
void read_romd(void);
void read_pif(void);
void read_pifb(void);
void read_pifh(void);
void read_pifd(void);

void write_nothing(void);
void write_nothingb(void);
void write_nothingh(void);
void write_nothingd(void);
void write_nomem(void);
void write_nomemb(void);
void write_nomemd(void);
void write_nomemh(void);
void write_rdram(void);
void write_rdramb(void);
void write_rdramh(void);
void write_rdramd(void);
void write_rdramFB(void);
void write_rdramFBb(void);
void write_rdramFBh(void);
void write_rdramFBd(void);
void write_rdramreg(void);
void write_rdramregb(void);
void write_rdramregh(void);
void write_rdramregd(void);
void write_rsp_mem(void);
void write_rsp_memb(void);
void write_rsp_memh(void);
void write_rsp_memd(void);
void write_rsp_reg(void);
void write_rsp_regb(void);
void write_rsp_regh(void);
void write_rsp_regd(void);
void write_rsp(void);
void write_rspb(void);
void write_rsph(void);
void write_rspd(void);
void write_dp(void);
void write_dpb(void);
void write_dph(void);
void write_dpd(void);
void write_dps(void);
void write_dpsb(void);
void write_dpsh(void);
void write_dpsd(void);
void write_mi(void);
void write_mib(void);
void write_mih(void);
void write_mid(void);
void write_vi(void);
void write_vib(void);
void write_vih(void);
void write_vid(void);
void write_ai(void);
void write_aib(void);
void write_aih(void);
void write_aid(void);
void write_pi(void);
void write_pib(void);
void write_pih(void);
void write_pid(void);
void write_ri(void);
void write_rib(void);
void write_rih(void);
void write_rid(void);
void write_si(void);
void write_sib(void);
void write_sih(void);
void write_sid(void);
void write_flashram_dummy(void);
void write_flashram_dummyb(void);
void write_flashram_dummyh(void);
void write_flashram_dummyd(void);
void write_flashram_command(void);
void write_flashram_commandb(void);
void write_flashram_commandh(void);
void write_flashram_commandd(void);
void write_rom(void);
void write_pif(void);
void write_pifb(void);
void write_pifh(void);
void write_pifd(void);

void update_SP(void);
void update_DPC(void);
void update_MI_init_mode_reg(void);
void update_MI_intr_mode_reg(void);
void update_MI_init_mask_reg(void);
void update_MI_intr_mask_reg(void);
void update_ai_dacrate(unsigned int word);
void update_vi_status(unsigned int word);
void update_vi_width(unsigned int word);

#endif

