/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dbg_memory.h                                            *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 DarkJeztr                                          *
 *   Copyright (C) 2002 davFr                                              *
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

#ifndef __DEBUGGER_MEMORY_H__
#define __DEBUGGER_MEMORY_H__

#define MEM_INVALID 0xFFFFFFFF
#define MEM_FLAG_READABLE    0x01
#define MEM_FLAG_WRITABLE    0x02

#define MAX_DISASSEMBLY 64

/* The following three macros create all the function calls for catching memory breakpoints
 * these do not occur until after the PC is incremented, but jumps & branches are not yet taken.
 */

#define MEMBREAKREAD(name,size) \
    void name##_break(void) { \
        check_breakpoints_on_mem_access((PC->addr)-0x4, address, size, BPT_FLAG_ENABLED | BPT_FLAG_READ); \
        name (); \
    }

#define MEMBREAKWRITE(name,size) \
    void name##_break(void) { \
        check_breakpoints_on_mem_access((PC->addr)-0x4, address, size, BPT_FLAG_ENABLED | BPT_FLAG_WRITE); \
        name (); \
    }

#define MEMBREAKALL(name) \
    MEMBREAKREAD( read_##name , 4); \
    MEMBREAKREAD( read_##name##b , 1); \
    MEMBREAKREAD( read_##name##h , 2); \
    MEMBREAKREAD( read_##name##d , 8); \
    MEMBREAKWRITE( write_##name , 4); \
    MEMBREAKWRITE( write_##name##b , 1); \
    MEMBREAKWRITE( write_##name##h , 2); \
    MEMBREAKWRITE( write_##name##d , 8);

enum {
  MEM_NOMEM = 0,
  MEM_NOTHING,
  MEM_RDRAM,
  MEM_RDRAMREG,
  MEM_RSPMEM,
  MEM_RSPREG,
  MEM_RSP,
  MEM_DP,
  MEM_DPS,
  MEM_VI,
  MEM_AI,
  MEM_PI,
  MEM_RI,
  MEM_SI,
  MEM_FLASHRAMSTAT,
  MEM_ROM,
  MEM_PIF,
  MEM_MI,
  MEM_BREAKPOINT
};

void init_host_disassembler(void);

char* get_recompiled_opcode( uint32 address, int index );
char* get_recompiled_args( uint32 address, int index );
void* get_recompiled_addr( uint32 address, int index );
int get_num_recompiled( uint32 address );
int get_has_recompiled( uint32 address );

uint64 read_memory_64(uint32 addr);
uint64 read_memory_64_unaligned(uint32 addr);
void write_memory_64(uint32 addr, uint64 value);
void write_memory_64_unaligned(uint32 addr, uint64 value);
uint32 read_memory_32(uint32);
uint32 read_memory_32_unaligned(uint32 addr);
void write_memory_32(uint32, uint32);
void write_memory_32_unaligned(uint32 addr, uint32 value);
uint16 read_memory_16(uint32 addr);
void write_memory_16(uint32 addr, uint16 value);
uint8 read_memory_8(uint32 addr);
void write_memory_8(uint32 addr, uint8 value);
uint32 get_memory_flags(uint32);
int get_memory_type(uint32);

void activate_memory_break_read(uint32 addr);
void deactivate_memory_break_read(uint32 addr);
void activate_memory_break_write(uint32 addr);
void deactivate_memory_break_write(uint32 addr);

/* Following are the prototypes for the memory breakpoint functions */
void read_nothing_break(void);
void read_nothingh_break(void);
void read_nothingb_break(void);
void read_nothingd_break(void);
void read_nomem_break(void);
void read_nomemb_break(void);
void read_nomemh_break(void);
void read_nomemd_break(void);
void read_rdram_break(void);
void read_rdramb_break(void);
void read_rdramh_break(void);
void read_rdramd_break(void);
void read_rdramFB_break(void);
void read_rdramFBb_break(void);
void read_rdramFBh_break(void);
void read_rdramFBd_break(void);
void read_rdramreg_break(void);
void read_rdramregb_break(void);
void read_rdramregh_break(void);
void read_rdramregd_break(void);
void read_rsp_mem_break(void);
void read_rsp_memb_break(void);
void read_rsp_memh_break(void);
void read_rsp_memd_break(void);
void read_rsp_reg_break(void);
void read_rsp_regb_break(void);
void read_rsp_regh_break(void);
void read_rsp_regd_break(void);
void read_rsp_break(void);
void read_rspb_break(void);
void read_rsph_break(void);
void read_rspd_break(void);
void read_dp_break(void);
void read_dpb_break(void);
void read_dph_break(void);
void read_dpd_break(void);
void read_dps_break(void);
void read_dpsb_break(void);
void read_dpsh_break(void);
void read_dpsd_break(void);
void read_mi_break(void);
void read_mib_break(void);
void read_mih_break(void);
void read_mid_break(void);
void read_vi_break(void);
void read_vib_break(void);
void read_vih_break(void);
void read_vid_break(void);
void read_ai_break(void);
void read_aib_break(void);
void read_aih_break(void);
void read_aid_break(void);
void read_pi_break(void);
void read_pib_break(void);
void read_pih_break(void);
void read_pid_break(void);
void read_ri_break(void);
void read_rib_break(void);
void read_rih_break(void);
void read_rid_break(void);
void read_si_break(void);
void read_sib_break(void);
void read_sih_break(void);
void read_sid_break(void);
void read_flashram_status_break(void);
void read_flashram_statusb_break(void);
void read_flashram_statush_break(void);
void read_flashram_statusd_break(void);
void read_rom_break(void);
void read_romb_break(void);
void read_romh_break(void);
void read_romd_break(void);
void read_pif_break(void);
void read_pifb_break(void);
void read_pifh_break(void);
void read_pifd_break(void);

void write_nothing_break(void);
void write_nothingb_break(void);
void write_nothingh_break(void);
void write_nothingd_break(void);
void write_nomem_break(void);
void write_nomemb_break(void);
void write_nomemd_break(void);
void write_nomemh_break(void);
void write_rdram_break(void);
void write_rdramb_break(void);
void write_rdramh_break(void);
void write_rdramd_break(void);
void write_rdramFB_break(void);
void write_rdramFBb_break(void);
void write_rdramFBh_break(void);
void write_rdramFBd_break(void);
void write_rdramreg_break(void);
void write_rdramregb_break(void);
void write_rdramregh_break(void);
void write_rdramregd_break(void);
void write_rsp_mem_break(void);
void write_rsp_memb_break(void);
void write_rsp_memh_break(void);
void write_rsp_memd_break(void);
void write_rsp_reg_break(void);
void write_rsp_regb_break(void);
void write_rsp_regh_break(void);
void write_rsp_regd_break(void);
void write_rsp_break(void);
void write_rspb_break(void);
void write_rsph_break(void);
void write_rspd_break(void);
void write_dp_break(void);
void write_dpb_break(void);
void write_dph_break(void);
void write_dpd_break(void);
void write_dps_break(void);
void write_dpsb_break(void);
void write_dpsh_break(void);
void write_dpsd_break(void);
void write_mi_break(void);
void write_mib_break(void);
void write_mih_break(void);
void write_mid_break(void);
void write_vi_break(void);
void write_vib_break(void);
void write_vih_break(void);
void write_vid_break(void);
void write_ai_break(void);
void write_aib_break(void);
void write_aih_break(void);
void write_aid_break(void);
void write_pi_break(void);
void write_pib_break(void);
void write_pih_break(void);
void write_pid_break(void);
void write_ri_break(void);
void write_rib_break(void);
void write_rih_break(void);
void write_rid_break(void);
void write_si_break(void);
void write_sib_break(void);
void write_sih_break(void);
void write_sid_break(void);
void write_flashram_dummy_break(void);
void write_flashram_dummyb_break(void);
void write_flashram_dummyh_break(void);
void write_flashram_dummyd_break(void);
void write_flashram_command_break(void);
void write_flashram_commandb_break(void);
void write_flashram_commandh_break(void);
void write_flashram_commandd_break(void);
void write_rom_break(void);
void write_pif_break(void);
void write_pifb_break(void);
void write_pifh_break(void);
void write_pifd_break(void);


#endif /* __DEBUGGER_MEMORY_H__ */

