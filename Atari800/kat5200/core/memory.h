/******************************************************************************
*
* FILENAME: memory.h
*
* DESCRIPTION:  This contains function declartions for memory functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
******************************************************************************/
#ifndef memory_h
#define memory_h

#include "console.h"
#include "debug.h"

/*
 * Memory functions
 */
#define rd_6502(addr)    ((cpu6502.read_mem[((addr) >> 8) & 0xff])(addr))
#define rd_antic(addr)    ((antic.read_mem[((addr) >> 8) & 0xff])(addr))
#define rd_z_6502(addr)    ((cpu6502.read_mem[((addr) >> 8) & 0xff])(addr))
#define wr_6502(addr,value)    ((cpu6502.write_mem[(addr >> 8) & 0xff])(addr, value))

/*
 * Defines for memory functions
 *   Odd numbers are debug...
 */
#define FUNC_READ_RAM     0
#define FUNC_READ_ROM     2
#define FUNC_READ_STACK   4
#define FUNC_READ_OS      6
#define FUNC_READ_BASIC   8
#define FUNC_READ_SELF    10
#define FUNC_READ_MEM_IMG 12
#define FUNC_READ_XE_BANK 14
#define FUNC_READ_BB      16
#define FUNC_READ_BB_PC   18
#define FUNC_READ_64K     20
#define FUNC_READ_32K_DB  22
#define FUNC_READ_XEGS    24
#define FUNC_READ_SWXEGS  26
#define FUNC_READ_MEGA    28
#define FUNC_READ_OSS     30
#define FUNC_READ_ATMAX   32
#define FUNC_READ_WILL    34
#define FUNC_READ_SDX     36
#define FUNC_READ_BLIZ    38
#define FUNC_READ_ATRAX   40
#define FUNC_READ_OSS2    42

#define FUNC_WRITE_RAM     0
#define FUNC_WRITE_ROM     2
#define FUNC_WRITE_STACK   4
#define FUNC_WRITE_OS      6
#define FUNC_WRITE_BASIC   8
#define FUNC_WRITE_SELF    10
#define FUNC_WRITE_MEM_IMG 12
#define FUNC_WRITE_XE_BANK 14
#define FUNC_WRITE_BB      16
#define FUNC_WRITE_BB_PC   18
#define FUNC_WRITE_64K     20
#define FUNC_WRITE_32K_DB  22
#define FUNC_WRITE_XEGS    24
#define FUNC_WRITE_SWXEGS  26
#define FUNC_WRITE_MEGA    28
#define FUNC_WRITE_OSS     30
#define FUNC_WRITE_ATMAX   32
#define FUNC_WRITE_WILL    34
#define FUNC_WRITE_SDX     36
#define FUNC_WRITE_BLIZ    38
#define FUNC_WRITE_ATRAX   40
#define FUNC_WRITE_OSS2    42

#define FUNC_MEM_MAX       44

void mem_init ( void );
void mem_get_state_values ( BYTE *portb, BYTE *mem_image  );
void mem_set_state_values ( BYTE portb, BYTE mem_image  );
void mem_reset ( void  );
void mem_assign_ops ( struct cpu *cpu  );
void mem_assign_ops_debug ( struct cpu *cpu, struct debug *p_debug  );
void mem_assign_portb_ops ( struct cpu *cpu,  BYTE portb  );

void memory_write_rom_bytes ( WORD addr, unsigned char *buffer, int size );
void memory_write_bank_bytes ( WORD addr, unsigned char *buffer, int size );
void memory_write_os_bytes ( WORD addr, unsigned char *buffer, int size );
void memory_write_basic_bytes ( WORD addr, unsigned char *buffer, int size );

BYTE read_5200_ram ( WORD addr );
BYTE read_5200_bank_switched ( WORD addr );
BYTE read_5200_stack ( WORD addr );
BYTE read_5200_rom ( WORD addr );
BYTE read_5200_rom_bb ( WORD addr );
BYTE read_5200_rom_bb_pc ( WORD addr );
BYTE read_5200_rom_64k ( WORD addr );
BYTE read_cart_32k_db ( WORD addr );
BYTE read_cart_xegs ( WORD addr );
BYTE read_cart_will ( WORD addr );
BYTE read_cart_sdx ( WORD addr );
BYTE read_cart_oss ( WORD addr );
BYTE read_cart_oss2 ( WORD addr );
BYTE read_cart_blizzard ( WORD addr );
BYTE read_os_rom ( WORD addr );
BYTE read_basic_rom ( WORD addr );
BYTE read_self_test_rom ( WORD addr );
BYTE read_xe_bank_ram ( WORD addr );
BYTE read_mem_image_ram ( WORD addr );

BYTE read_5200_ram_debug ( WORD addr );
BYTE read_5200_rom_debug ( WORD addr );
BYTE read_5200_stack_debug ( WORD addr );
BYTE read_5200_rom_bb_debug ( WORD addr );
BYTE read_5200_rom_bb_pc_debug ( WORD addr );
BYTE read_5200_rom_64k_debug ( WORD addr );
BYTE read_cart_32k_db_debug ( WORD addr );
BYTE read_cart_xegs_debug ( WORD addr );
BYTE read_cart_will_debug ( WORD addr );
BYTE read_cart_sdx_debug ( WORD addr );
BYTE read_cart_oss_debug ( WORD addr );
BYTE read_cart_oss2_debug ( WORD addr );
BYTE read_cart_blizzard_debug ( WORD addr );
BYTE read_os_rom_debug ( WORD addr );
BYTE read_basic_rom_debug ( WORD addr );
BYTE read_self_test_rom_debug ( WORD addr );
BYTE read_xe_bank_ram_debug ( WORD addr );
BYTE read_mem_image_ram_debug ( WORD addr );

void write_5200_ram ( WORD addr, BYTE value );
void write_5200_bank_switched ( WORD addr, BYTE value );
void write_os_rom ( WORD addr, BYTE value );
void write_basic_rom ( WORD addr, BYTE value );
void write_xe_bank_state ( WORD addr, BYTE value );
BYTE read_xe_bank_state ( WORD addr );
void write_5200_stack ( WORD addr, BYTE value );
void write_5200_stack_watch ( WORD addr, BYTE value );
void write_xe_bank_ram ( WORD addr, BYTE value );
void write_mem_image_ram ( WORD addr, BYTE value );
void write_5200_rom ( WORD addr, BYTE value );
void write_5200_rom_bb ( WORD addr, BYTE value );
void write_5200_rom_bb_pc ( WORD addr, BYTE value );
void write_5200_rom_64k ( WORD addr, BYTE value );
void write_cart_32k_db ( WORD addr, BYTE value );
void write_cart_xegs ( WORD addr, BYTE value );
void write_cart_swxegs ( WORD addr, BYTE value );
void write_cart_mega ( WORD addr, BYTE value );
void write_cart_atmax ( WORD addr, BYTE value );
void write_cart_will ( WORD addr, BYTE value );
void write_cart_sdx ( WORD addr, BYTE value );
void write_cart_oss ( WORD addr, BYTE value );
void write_cart_oss2 ( WORD addr, BYTE value );
void write_cart_blizzard ( WORD addr, BYTE value );
void write_cart_atrax ( WORD addr, BYTE value );

void write_5200_ram_debug ( WORD addr, BYTE value );
void write_5200_stack_debug ( WORD addr, BYTE value );
void write_xe_bank_ram_debug ( WORD addr, BYTE value );
void write_mem_image_ram_debug ( WORD addr, BYTE value );
void write_5200_rom_bb_debug ( WORD addr, BYTE value );
void write_5200_rom_bb_pc_debug ( WORD addr, BYTE value );
void write_5200_rom_64k_debug ( WORD addr, BYTE value );
void write_cart_32k_db_debug ( WORD addr, BYTE value );
void write_cart_xegs_debug ( WORD addr, BYTE value );
void write_cart_swxegs_debug ( WORD addr, BYTE value );
void write_cart_mega_debug ( WORD addr, BYTE value );
void write_cart_atmax_debug ( WORD addr, BYTE value );
void write_cart_will_debug ( WORD addr, BYTE value );
void write_cart_sdx_debug ( WORD addr, BYTE value );
void write_cart_oss_debug ( WORD addr, BYTE value );
void write_cart_oss2_debug ( WORD addr, BYTE value );
void write_cart_blizzard_debug ( WORD addr, BYTE value );
void write_cart_atrax_debug ( WORD addr, BYTE value );
void write_5200_ram_watch ( WORD addr, BYTE value );
void write_5200_rom_debug ( WORD addr, BYTE value );

#endif
