/******************************************************************************
*
* FILENAME: memory.c
*
* DESCRIPTION:  This contains functions for handling 6502 memory for the system
*               Also handles redirection for memory mapped I/O
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.2     03/09/06  bberlin      Added 4K cart support
*                                Added support for Bounty Bob 3 file
*                                Added many more 16K one chip rom CRCs
* 0.2.3   03/19/06  bberlin      Added Beamrider to one chip rom list in 
*                                  'load_cart'
* 0.3.0   03/27/06  bberlin      Added Missle Command+ to one chip rom list in 
*                                  'load_cart'
* 0.4.0   07/12/06  bberlin      Added Time Runner to one chip rom list in 
*                                  'load_cart'
* 0.5.0   02/07/07  bberlin      Added functions for writing to memory, took
*                                  out cart loading
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "console.h"
#include "antic.h"
#include "gtia.h"
#include "pokey.h"
#include "pia.h"
#include "debug.h"

BYTE memory[0x10000];
static BYTE bank_image[0x20000];
static BYTE mem_image[0x4000];
static BYTE xe_bank_image[4][0x4000];
static BYTE os_rom[0x4000];
static BYTE basic_rom[0x2000];

static BYTE *g_xe_bank_ptr = xe_bank_image[0];
static BYTE *g_self_test_ptr = &os_rom[0x1000];

static BYTE g_portb_value = 0xff;
static BYTE g_mem_image = 0;

BYTE ( *mem_read_func[ FUNC_MEM_MAX ] ) ( WORD addr );
void ( *mem_write_func[ FUNC_MEM_MAX ] ) ( WORD addr, BYTE value );

/******************************************************************************
**  Function   :  mem_init
**                            
**  Objective  :  This function initializes the memory function array for use
**                in assignments
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void mem_init ( void ) {

	mem_read_func[FUNC_READ_RAM] = read_5200_ram;
	mem_read_func[FUNC_READ_RAM+1] = read_5200_ram_debug;
	mem_read_func[FUNC_READ_ROM] = read_5200_rom;
	mem_read_func[FUNC_READ_ROM+1] = read_5200_rom_debug;
	mem_read_func[FUNC_READ_STACK] = read_5200_stack;
	mem_read_func[FUNC_READ_STACK+1] = read_5200_stack_debug;
	mem_read_func[FUNC_READ_OS] = read_os_rom;
	mem_read_func[FUNC_READ_OS+1] = read_os_rom_debug;
	mem_read_func[FUNC_READ_BASIC] = read_basic_rom;
	mem_read_func[FUNC_READ_BASIC+1] = read_basic_rom_debug;
	mem_read_func[FUNC_READ_SELF] = read_self_test_rom;
	mem_read_func[FUNC_READ_SELF+1] = read_self_test_rom_debug;
	mem_read_func[FUNC_READ_MEM_IMG] = read_mem_image_ram;
	mem_read_func[FUNC_READ_MEM_IMG+1] = read_mem_image_ram_debug;
	mem_read_func[FUNC_READ_XE_BANK] = read_xe_bank_ram;
	mem_read_func[FUNC_READ_XE_BANK+1] = read_xe_bank_ram_debug;
	mem_read_func[FUNC_READ_BB] = read_5200_rom_bb;
	mem_read_func[FUNC_READ_BB+1] = read_5200_rom_bb_debug;
	mem_read_func[FUNC_READ_BB_PC] = read_5200_rom_bb_pc;
	mem_read_func[FUNC_READ_BB_PC+1] = read_5200_rom_bb_pc_debug;
	mem_read_func[FUNC_READ_64K] = read_5200_rom_64k;
	mem_read_func[FUNC_READ_64K+1] = read_5200_rom_64k_debug;
	mem_read_func[FUNC_READ_32K_DB] = read_cart_32k_db;
	mem_read_func[FUNC_READ_32K_DB+1] = read_cart_32k_db_debug;
	mem_read_func[FUNC_READ_XEGS] = read_cart_xegs;
	mem_read_func[FUNC_READ_XEGS+1] = read_cart_xegs_debug;
	mem_read_func[FUNC_READ_SWXEGS] = read_cart_xegs;
	mem_read_func[FUNC_READ_SWXEGS+1] = read_cart_xegs_debug;
	mem_read_func[FUNC_READ_MEGA] = read_cart_xegs;
	mem_read_func[FUNC_READ_MEGA+1] = read_cart_xegs_debug;
	mem_read_func[FUNC_READ_OSS] = read_cart_oss;
	mem_read_func[FUNC_READ_OSS+1] = read_cart_oss_debug;
	mem_read_func[FUNC_READ_OSS2] = read_cart_oss2;
	mem_read_func[FUNC_READ_OSS2+1] = read_cart_oss2_debug;
	mem_read_func[FUNC_READ_ATMAX] = read_cart_xegs;
	mem_read_func[FUNC_READ_ATMAX+1] = read_cart_xegs_debug;
	mem_read_func[FUNC_READ_WILL] = read_cart_will;
	mem_read_func[FUNC_READ_WILL+1] = read_cart_will_debug;
	mem_read_func[FUNC_READ_SDX] = read_cart_sdx;
	mem_read_func[FUNC_READ_SDX+1] = read_cart_sdx_debug;
	mem_read_func[FUNC_READ_BLIZ] = read_cart_blizzard;
	mem_read_func[FUNC_READ_BLIZ+1] = read_cart_blizzard_debug;
	mem_read_func[FUNC_READ_ATRAX] = read_cart_xegs;
	mem_read_func[FUNC_READ_ATRAX+1] = read_cart_xegs_debug;

	mem_write_func[FUNC_WRITE_RAM] = write_5200_ram;
	mem_write_func[FUNC_WRITE_RAM+1] = write_5200_ram_debug;
	mem_write_func[FUNC_WRITE_ROM] = write_5200_rom;
	mem_write_func[FUNC_WRITE_ROM+1] = write_5200_rom_debug;
	mem_write_func[FUNC_WRITE_STACK] = write_5200_stack;
	mem_write_func[FUNC_WRITE_STACK+1] = write_5200_stack_debug;
	mem_write_func[FUNC_WRITE_OS] = write_5200_rom;
	mem_write_func[FUNC_WRITE_OS+1] = write_5200_rom_debug;
	mem_write_func[FUNC_WRITE_BASIC] = write_5200_rom;
	mem_write_func[FUNC_WRITE_BASIC+1] = write_5200_rom_debug;
	mem_write_func[FUNC_WRITE_SELF] = write_5200_rom;
	mem_write_func[FUNC_WRITE_SELF+1] = write_5200_rom_debug;
	mem_write_func[FUNC_WRITE_MEM_IMG] = write_mem_image_ram;
	mem_write_func[FUNC_WRITE_MEM_IMG+1] = write_mem_image_ram_debug;
	mem_write_func[FUNC_WRITE_XE_BANK] = write_xe_bank_ram;
	mem_write_func[FUNC_WRITE_XE_BANK+1] = write_xe_bank_ram_debug;
	mem_write_func[FUNC_WRITE_BB] = write_5200_rom_bb;
	mem_write_func[FUNC_WRITE_BB+1] = write_5200_rom_bb_debug;
	mem_write_func[FUNC_WRITE_BB_PC] = write_5200_rom_bb_pc;
	mem_write_func[FUNC_WRITE_BB_PC+1] = write_5200_rom_bb_pc_debug;
	mem_write_func[FUNC_WRITE_64K] = write_5200_rom_64k;
	mem_write_func[FUNC_WRITE_64K+1] = write_5200_rom_64k_debug;
	mem_write_func[FUNC_WRITE_32K_DB] = write_cart_32k_db;
	mem_write_func[FUNC_WRITE_32K_DB+1] = write_cart_32k_db_debug;
	mem_write_func[FUNC_WRITE_XEGS] = write_cart_xegs;
	mem_write_func[FUNC_WRITE_XEGS+1] = write_cart_xegs_debug;
	mem_write_func[FUNC_WRITE_SWXEGS] = write_cart_swxegs;
	mem_write_func[FUNC_WRITE_SWXEGS+1] = write_cart_swxegs_debug;
	mem_write_func[FUNC_WRITE_MEGA] = write_cart_mega;
	mem_write_func[FUNC_WRITE_MEGA+1] = write_cart_mega_debug;
	mem_write_func[FUNC_WRITE_OSS] = write_cart_oss;
	mem_write_func[FUNC_WRITE_OSS+1] = write_cart_oss_debug;
	mem_write_func[FUNC_WRITE_OSS2] = write_cart_oss2;
	mem_write_func[FUNC_WRITE_OSS2+1] = write_cart_oss2_debug;
	mem_write_func[FUNC_WRITE_ATMAX] = write_cart_atmax;
	mem_write_func[FUNC_WRITE_ATMAX+1] = write_cart_atmax_debug;
	mem_write_func[FUNC_WRITE_WILL] = write_cart_will;
	mem_write_func[FUNC_WRITE_WILL+1] = write_cart_will_debug;
	mem_write_func[FUNC_WRITE_SDX] = write_cart_sdx;
	mem_write_func[FUNC_WRITE_SDX+1] = write_cart_sdx_debug;
	mem_write_func[FUNC_WRITE_BLIZ] = write_cart_blizzard;
	mem_write_func[FUNC_WRITE_BLIZ+1] = write_cart_blizzard_debug;
	mem_write_func[FUNC_WRITE_ATRAX] = write_cart_atrax;
	mem_write_func[FUNC_WRITE_ATRAX+1] = write_cart_atrax_debug;

} /* end mem_init */

/******************************************************************************
**  Function   :  mem_get_state_values
**                            
**  Objective  :  This grabs values needed for storage into a state save file
**
**  Parameters :  portb     - memory portb value
**                mem_image - 1 if ram behind cart is on
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void mem_get_state_values ( BYTE *portb, BYTE *mem_image  ) {

	*portb = g_portb_value;
	*mem_image = g_mem_image;

} /* mem_get_state_values */

/******************************************************************************
**  Function   :  mem_set_state_values
**                            
**  Objective  :  This sets values needed for setting up memory from the state
**                file
**
**  Parameters :  portb     - memory portb value
**                mem_image - 1 if ram behind cart is on
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void mem_set_state_values ( BYTE portb, BYTE mem_image  ) {

	g_portb_value = portb;
	g_mem_image = mem_image;

} /* mem_set_state_values */

/******************************************************************************
**  Function   :  mem_reset
**                            
**  Objective  :  This re-assigns functions and assigns portb to its initial
**                state for XL/XE.
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void mem_reset ( void  ) {

	g_portb_value = 0xff;
	g_mem_image = 0;

	/*
	 * Kill any bootloader stuff we may have
	 */
	memset ( memory, 0, sizeof(BYTE) * 0x800 );

	mem_assign_ops ( console_get_ptr()->cpu  );

} /* mem_reset */

/******************************************************************************
**  Function   :  mem_assign_ops
**                            
**  Objective  :  This function assigns functions to memory function pointers
**
**  Parameters :  cpu - pointer to cpu struct that contains array of pointers
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void mem_assign_ops ( struct cpu *cpu  ) {

	int i;
	int gtia_addr, antic_addr, pokey_addr;
	int ram_end;

	/*
	 * First determine chip address based on the machine
	 *   Also go ahead and assign rom sections
	 */
	for ( i = 0xc0; i < 0x100; i++ ) {
		cpu->read_mem[i] = read_5200_rom;
		cpu->write_mem[i] = write_5200_rom;
	}
	if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
		gtia_addr = 0xc0;
		antic_addr = 0xd4;
		pokey_addr = 0xe8;
		ram_end = 0x40;
	}
	else {
		gtia_addr = 0xd0;
		antic_addr = 0xd4;
		pokey_addr = 0xd2;
		if ( console_get_ptr()->ram_size == 16 )
			ram_end = 0x40;
		if ( console_get_ptr()->cart->loaded && 
		     (console_get_ptr()->cart->size > 0x2000) && 
		     (console_get_ptr()->cart->mapping != CART_OSS) )
			ram_end = 0x80;
		else if ( (!console_get_ptr()->basic_enable) && 
		          (console_get_ptr()->machine_type == MACHINE_TYPE_800) )
			ram_end = 0xc0;
		else
			ram_end = 0xa0;
	}

	/*
	 * assign for OS RAM and writable RAM
	 */
	for ( i = 0x00; i < ram_end; i++ ) {
		cpu->read_mem[i] = read_5200_ram;
		cpu->write_mem[i] = write_5200_ram;
	}

	/*
	 * assign for Stack
	 */
	cpu->read_mem[0x01] = read_5200_stack;
	cpu->write_mem[0x01] = write_5200_stack;

	/*
	 * assign for Cartridge ROM
	 */
	for ( i = ram_end; i < 0xc0; i++ ) {
		cpu->read_mem[i] = read_5200_rom;
		cpu->write_mem[i] = write_5200_rom;
	}

	/*
	 * assign for GTIA
	 */
	cpu->read_mem[gtia_addr] = gtia_read_register;
	cpu->write_mem[gtia_addr] = gtia_write_register;

	/*
	 * assign for ANTIC
	 */
	cpu->read_mem[antic_addr] = antic_read_register;
	cpu->write_mem[antic_addr] = antic_write_register;

	/*
	 * assign for POKEY
	 */
	cpu->read_mem[pokey_addr] = pokey_read_register;
	cpu->write_mem[pokey_addr] = pokey_write_register;

	if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
		for ( i = 0xe9; i < 0xf0; i++ ) {
			cpu->read_mem[i] = pokey_read_register;
			cpu->write_mem[i] = pokey_write_register;
		}
	}

	/*
	 * assign for PIA
	 */
	if ( console_get_machine_type() != MACHINE_TYPE_5200 ) {
		cpu->read_mem[0xd3] = pia_read_register;
		cpu->write_mem[0xd3] = pia_write_register;
	}

	/*
	 * Extra RAM space in the 800
	 */
	if ( (console_get_machine_type() == MACHINE_TYPE_800) && 
	     (console_get_ptr()->ram_size == 52) ) {
		for ( i = 0xc0; i < 0xd0; i++ ) {
			cpu->read_mem[i] = read_5200_ram;
			cpu->write_mem[i] = write_5200_ram;
		}
	}

	/***************************************/
	/* Now check for bank switching        */
	/***************************************/

	/*
	 * Bounty Bob
	 */
	if ( console_get_ptr()->cart->mapping == CART_40K ) {
		cpu->read_mem[0x4f] = read_5200_rom_bb;
		cpu->write_mem[0x4f] = write_5200_rom_bb;
		cpu->read_mem[0x5f] = read_5200_rom_bb;
		cpu->write_mem[0x5f] = write_5200_rom_bb;
	}

	if ( console_get_ptr()->cart->mapping == CART_PC_40K ) {
		cpu->read_mem[0x8f] = read_5200_rom_bb_pc;
		cpu->write_mem[0x8f] = write_5200_rom_bb_pc;
		cpu->read_mem[0x9f] = read_5200_rom_bb_pc;
		cpu->write_mem[0x9f] = write_5200_rom_bb_pc;
	}

	/*
	 * 64k bank switching mode
	 */
	if ( console_get_ptr()->cart->mapping == CART_64K ) {
		cpu->read_mem[0x3f] = read_5200_rom_64k;
		cpu->write_mem[0x3f] = write_5200_rom_64k;
		cpu->read_mem[0xbf] = read_5200_rom_64k;
		cpu->write_mem[0xbf] = write_5200_rom_64k;
	}

	/*
	 * Standard 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_PC_32K ) {
		cpu->read_mem[0xd5] = read_cart_32k_db;
		cpu->write_mem[0xd5] = write_cart_32k_db;
	}

	/*
	 * XEGS 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_XEGS ) {
		cpu->read_mem[0xd5] = read_cart_xegs;
		cpu->write_mem[0xd5] = write_cart_xegs;
	}

	/*
	 * SWXEGS 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_SWXEGS ) {
		cpu->read_mem[0xd5] = read_cart_xegs;
		cpu->write_mem[0xd5] = write_cart_swxegs;
	}

	/*
	 * MEGA 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_MEGA ) {
		cpu->read_mem[0xd5] = read_cart_xegs;
		cpu->write_mem[0xd5] = write_cart_mega;
	}

	/*
	 * ATMAX 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_ATMAX ) {
		cpu->read_mem[0xd5] = read_cart_xegs;
		cpu->write_mem[0xd5] = write_cart_atmax;
	}

	/*
	 * ATRAX 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_ATRAX ) {
		cpu->read_mem[0xd5] = read_cart_xegs;
		cpu->write_mem[0xd5] = write_cart_atrax;
	}

	/*
	 * WILL 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_WILL ) {
		cpu->read_mem[0xd5] = read_cart_will;
		cpu->write_mem[0xd5] = write_cart_will;
	}

	/*
	 * SDX,EXP,Diamond 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_SDX ||
	     console_get_ptr()->cart->mapping == CART_EXP ||
	     console_get_ptr()->cart->mapping == CART_DIAMOND ) {
		cpu->read_mem[0xd5] = read_cart_sdx;
		cpu->write_mem[0xd5] = write_cart_sdx;
	}

	/*
	 * Blizzard and Phoenix 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_BLIZZARD ||
	     console_get_ptr()->cart->mapping == CART_PHOENIX ) {
		cpu->read_mem[0xd5] = read_cart_blizzard;
		cpu->write_mem[0xd5] = write_cart_blizzard;
	}

	/*
	 * OSS 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_OSS ) {
		cpu->read_mem[0xd5] = read_cart_oss;
		cpu->write_mem[0xd5] = write_cart_oss;
	}

	/*
	 * OSS2 8-bit bank switching
	 */
	if ( console_get_ptr()->cart->mapping == CART_OSS2 ) {
		cpu->read_mem[0xd5] = read_cart_oss2;
		cpu->write_mem[0xd5] = write_cart_oss2;
	}

	/*
	 * Assign Memory behind cartridge if set
	 */
	if ( g_mem_image ) {
		for ( i = ram_end; i < 0xc0; ++i ) {
			cpu->read_mem[i] = read_mem_image_ram;
			cpu->write_mem[i] = write_mem_image_ram;
		}
	}

	/*
	 * Assign Port B Handler to current value
	 */
	if ( console_get_machine_type() == MACHINE_TYPE_XL )
		mem_assign_portb_ops ( console_get_ptr()->cpu, g_portb_value );

} /* mem_assign_ops */

/******************************************************************************
**  Function   :  mem_assign_ops_debug
**                            
**  Objective  :  This function assigns the debug functions based on current
**                breakpoints
**
**  Parameters :  cpu -     pointer to cpu struct that contains array of pointers
**                p_debug - pointer to debug struct that has breakpoints
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void mem_assign_ops_debug ( struct cpu *cpu, struct debug *p_debug  ) {

	int i;
	int page;
	int gtia_addr, antic_addr, pokey_addr;
	int ram_end;

	if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
		gtia_addr = 0xc0;
		antic_addr = 0xd4;
		pokey_addr = 0xe8;
		ram_end = 0x40;
	}
	else {
		gtia_addr = 0xd0;
		antic_addr = 0xd4;
		pokey_addr = 0xd2;
		if ( console_get_ptr()->cart->loaded && 
		     (console_get_ptr()->cart->size > 0x2000) && 
		     (console_get_ptr()->cart->mapping != CART_OSS) )
			ram_end = 0x80;
		else
			ram_end = 0xa0;
	}

	/*
	 * First assign ops as normal
	 */
	mem_assign_ops ( cpu );

	/*
	 * Now go through the breakpoints and reassign
	 */
	for ( i = 0; i < DEBUG_MAX_BK; ++i ) {

		if ( p_debug->breakpoint[i] == BK_WATCH ) {
			page = p_debug->breakpoint_place[i] >> 8;
			if ( (page >= 0x00 && page < ram_end) ) {
				cpu->write_mem[page] = write_5200_ram_watch;
			}
			if ( (page == 0x01) ) {
				cpu->write_mem[page] = write_5200_stack_watch;
			}
		}

		if ( p_debug->breakpoint[i] != BK_MEM )
			continue;

		/*
		 *  Ops are assigned based on memory page
		 */
		page = p_debug->breakpoint_place[i] >> 8;
	
		for ( i = 0; i < FUNC_MEM_MAX; i+=2 ) {
			if ( cpu->read_mem[page] == mem_read_func[i] )
				cpu->read_mem[page] = mem_read_func[i + 1];
			if ( cpu->write_mem[page] == mem_write_func[i] )
				cpu->write_mem[page] = mem_write_func[i + 1];
		}
	}

} /* mem_assign_ops_debug */

/******************************************************************************
**  Function   :  mem_assign_portb_ops
**                            
**  Objective  :  This function assigns functions to memory function pointers
**                according to the portb register.  Only used with XL/XE.
**
**  Parameters :  cpu - pointer to cpu struct that contains array of pointers
**                portb - PORTB register value
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void mem_assign_portb_ops ( struct cpu *cpu,  BYTE portb  ) {

	int i;
	int debug;
	struct antic *antic = console_get_ptr()->antic;
	int ram_size = console_get_ptr()->ram_size;

	g_portb_value = portb;

	/*
	 * OS Rom
	 */
	for ( i = 0xc0; i < 0x100; ++i ) {
		if ( i >= 0xd0 && i < 0xd8 )
			continue;
		debug = debug_is_page_in_debug ( i );
		if ( portb & 0x01 ) {
			cpu->read_mem[i] = mem_read_func[FUNC_READ_OS | debug];
			antic->read_mem[i] = mem_read_func[FUNC_READ_OS | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_OS | debug];
		}
		else {
			cpu->read_mem[i] = mem_read_func[FUNC_READ_RAM | debug];
			antic->read_mem[i] = mem_read_func[FUNC_READ_RAM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_RAM | debug];
		}
	}

	/*
	 * Basic Rom
	 */
	if ( !console_get_ptr()->cart->loaded ) {
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			if ( portb & 0x02 ) {
				cpu->read_mem[i] = mem_read_func[FUNC_READ_RAM | debug];
				antic->read_mem[i] = mem_read_func[FUNC_READ_RAM | debug];
				cpu->write_mem[i] = mem_write_func[FUNC_WRITE_RAM | debug];
			}
			else {
				cpu->read_mem[i] = mem_read_func[FUNC_READ_BASIC | debug];
				antic->read_mem[i] = mem_read_func[FUNC_READ_BASIC | debug];
				cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
			}
		}
	}

	g_xe_bank_ptr = xe_bank_image[ (portb & 0x0c) >> 2 ];

	/*
	 * CPU Switch
	 */ 
	if ( ram_size > 64 ) {
		for ( i = 0x40; i < 0x80; ++i ) {
			debug = debug_is_page_in_debug ( i );
			if ( (portb & 0x10) == 0 ) {
				cpu->read_mem[i] = mem_read_func[FUNC_READ_XE_BANK | debug];
				cpu->write_mem[i] = mem_write_func[FUNC_WRITE_XE_BANK | debug];
			}
			else {
				cpu->read_mem[i] = mem_read_func[FUNC_READ_RAM | debug];
				cpu->write_mem[i] = mem_write_func[FUNC_WRITE_RAM | debug];
			}
		}
	}

	/*
	 * Antic Switch
	 */ 
	if ( ram_size > 64 ) {
		for ( i = 0x40; i < 0x80; ++i ) {
			debug = debug_is_page_in_debug ( i );
			if ( (portb & 0x20) == 0 ) {
				antic->read_mem[i] = mem_read_func[FUNC_READ_XE_BANK | debug];
			}
			else {
				antic->read_mem[i] = mem_read_func[FUNC_READ_RAM | debug];
			}
		}
	}

	/*
	 * Self-Test Rom
	 */ 
	for ( i = 0x50; i < 0x58; ++i ) {
		debug = debug_is_page_in_debug ( i );
		if ( (portb & 0x80)  || !(portb & 0x01) ) {
			cpu->read_mem[i] = mem_read_func[FUNC_READ_RAM | debug];
			antic->read_mem[i] = mem_read_func[FUNC_READ_RAM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_RAM | debug];
		}
		else {
			cpu->read_mem[i] = mem_read_func[FUNC_READ_SELF | debug];
			antic->read_mem[i] = mem_read_func[FUNC_READ_SELF | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
	}

} /* mem_assign_portb_ops */

/******************************************************************************
**  Function   :  memory_write_rom_bytes
**                                  
**  Objective  :  This function will write the buffer of bytes to rom
**                                                                  
**  Parameters :  addr   - address in system to write to
**                buffer - buffer to write
**                size   - number of bytes to write
**                                                                
**  return     :  value at address                               
******************************************************************************/ 
void memory_write_rom_bytes ( WORD addr, unsigned char *buffer, int size ) {

	int i;

	for ( i = 0; i < size; ++i ) {
		memory[ addr + i ] = buffer[i];
	}
}

void memory_write_bank_bytes ( WORD addr, unsigned char *buffer, int size ) {

	int i;

	for ( i = 0; i < size; ++i ) {
		bank_image[ addr + i ] = buffer[i];
	}
}

void memory_write_os_bytes ( WORD addr, unsigned char *buffer, int size ) {

	int i;

	addr = addr & 0x3fff;
	for ( i = 0; i < size; ++i ) {
		os_rom[ addr + i ] = buffer[i];
	}
}

void memory_write_basic_bytes ( WORD addr, unsigned char *buffer, int size ) {

	int i;

	addr = addr & 0x1fff;
	for ( i = 0; i < size; ++i ) {
		basic_rom[ addr + i ] = buffer[i];
	}
}

/******************************************************************************
**  Function   :  read_5200_ram         
**                                  
**  Objective  :  This function will read from system memory and return value
**                                                                  
**  Parameters :  addr - address in system ram to read from        
**                                                                
**  return     :  value at address                               
******************************************************************************/ 
BYTE read_5200_ram ( WORD addr ) {

	return memory[ addr ];
}

BYTE read_5200_ram_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return memory[ addr ];
}

BYTE read_5200_bank_switched ( WORD addr ) {

	return bank_image[ addr ];
}

/******************************************************************************
**  Function   :  read_5200_stack         
**                                  
**  Objective  :  This function will read from the cpu stack (0x100-0x1ff)
**                                                                  
**  Parameters :  addr - address in system ram to read from        
**                                                                
**  return     :  value at address                               
******************************************************************************/ 
BYTE read_5200_stack ( WORD addr ) {

	return cpu_get_stack_value( addr & 0xff );
}

BYTE read_5200_stack_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return cpu_get_stack_value( addr & 0xff );
}

/******************************************************************************
**  Function   :  read_5200_rom 
**                                     
**  Objective  :  This function will read from rom and return value   
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_5200_rom ( WORD addr ) {

	return memory[ addr ];
}

BYTE read_5200_rom_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return memory[ addr];
}

/******************************************************************************
**  Function   :  read_os_rom 
**                                     
**  Objective  :  This function will read from the os portion of rom
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_os_rom ( WORD addr ) {

	return os_rom[ addr & 0x3fff ];
}

BYTE read_os_rom_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return os_rom[ addr & 0x3fff ];
}

/******************************************************************************
**  Function   :  read_basic_rom 
**                                     
**  Objective  :  This function will read from the os portion of rom
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_basic_rom ( WORD addr ) {

	return basic_rom[ addr & 0x1fff ];
}

BYTE read_basic_rom_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return basic_rom[ addr & 0x1fff ];
}

/******************************************************************************
**  Function   :  read_self_test_rom 
**                                     
**  Objective  :  This function will read from the self-test portion of rom
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_self_test_rom ( WORD addr ) {

	return g_self_test_ptr[ addr & 0x7ff ];
}

BYTE read_self_test_rom_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return g_self_test_ptr[ addr & 0x7ff ];
}

/******************************************************************************
**  Function   :  read_xe_bank_ram 
**                                     
**  Objective  :  This function will read from the bank switched ram of the XE
**                                      
**  Parameters :  addr - address in ram to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_xe_bank_ram ( WORD addr ) {

	return g_xe_bank_ptr[ addr & 0x3fff ];
}

BYTE read_xe_bank_ram_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return g_xe_bank_ptr[ addr & 0x3fff ];
}

BYTE read_mem_image_ram ( WORD addr ) {

	return mem_image[ addr & 0x7fff ];
}

BYTE read_mem_image_ram_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return mem_image[ addr & 0x7fff ];
}

/******************************************************************************
**  Function   :  write_5200_ram   
**                                
**  Objective  :  This function will write a value to system ram
** 
**  Parameters :  addr  - address in system ram to write to
**                value - value to write at address 
**                                             
**  return     :  NONE                        
******************************************************************************/ 
void write_5200_ram ( WORD addr, BYTE value ) {

	memory[ addr ] = value;
}

void write_5200_ram_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	memory[ addr ] = value;
}

void write_5200_ram_watch ( WORD addr, BYTE value ) {

	debug_break_for_watch (addr, value, memory[addr] );

	memory[ addr ] = value;
}

void write_5200_bank_switched ( WORD addr, BYTE value ) {

	bank_image[ addr ] = value;
}

void write_os_rom ( WORD addr, BYTE value ) {
	os_rom[ addr ] = value;
}

void write_basic_rom ( WORD addr, BYTE value ) {
	basic_rom[ addr ] = value;
}

void write_xe_bank_state ( WORD addr, BYTE value ) {
	xe_bank_image[ addr >> 14][ addr & 0x3fff ] = value;
}

BYTE read_xe_bank_state ( WORD addr ) {
	return xe_bank_image[ addr >> 14][ addr & 0x3fff ];
}

/******************************************************************************
**  Function   :  write_xe_bank_ram   
**                                
**  Objective  :  This function will write a value to bank switched ram for XE
** 
**  Parameters :  addr  - address in system ram to write to
**                value - value to write at address 
**                                             
**  return     :  NONE                        
******************************************************************************/ 
void write_xe_bank_ram ( WORD addr, BYTE value ) {

	g_xe_bank_ptr[ addr & 0x3fff ] = value;
}

void write_xe_bank_ram_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	g_xe_bank_ptr[ addr & 0x3fff ] = value;
}

void write_mem_image_ram ( WORD addr, BYTE value ) {

	mem_image[ addr & 0x7fff ] = value;
}

void write_mem_image_ram_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	mem_image[ addr & 0x7fff ] = value;
}

/******************************************************************************
**  Function   :  write_5200_stack 
**                                
**  Objective  :  This function will write a value to the cpu stack (0x100-0x1ff)
** 
**  Parameters :  addr  - address in system ram to write to
**                value - value to write at address 
**                                             
**  return     :  NONE                        
******************************************************************************/ 
void write_5200_stack ( WORD addr, BYTE value ) {

	cpu_set_stack_value ( addr & 0xff, value  );	
}

void write_5200_stack_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	cpu_set_stack_value ( addr & 0xff, value  );	
}

void write_5200_stack_watch ( WORD addr, BYTE value ) {

	debug_break_for_watch (addr, value, cpu_get_stack_value( addr & 0xff ) );

	cpu_set_stack_value ( addr & 0xff, value  );	
}

/******************************************************************************
**  Function   :  write_5200_rom                
**                                             
**  Objective  :  Placeholder function.  Can't really write to ROM
**
**  Parameters :  addr  - address in rom that had an attempted write
**                value - value to write at address
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
void write_5200_rom ( WORD addr, BYTE value ) {

	//print_addr_val ( WRITE_MEM, addr, value );
}

void write_5200_rom_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	//print_addr_val ( WRITE_MEM, addr, value );
}

/******************************************************************************
**  Function   :  read_5200_rom_bb 
**                                     
**  Objective  :  Bounty Bob bank switching check
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_5200_rom_bb ( WORD addr ) {

	if ( addr > 0x4ff5 && addr < 0x4ffa ) {
		addr -= 0x4ff6;
		memcpy ( &memory[0x4000], &bank_image[addr*0x1000] , 0x1000*sizeof(BYTE));
		return 0;
	}
	else if ( addr > 0x5ff5 && addr < 0x5ffa ) {
		addr -= 0x5ff6;
		memcpy ( &memory[0x5000], &bank_image[0x4000+addr*0x1000] , 0x1000*sizeof(BYTE));
		return 0;
	}
	else
		return memory[ addr ];
}

BYTE read_5200_rom_bb_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( addr > 0x4ff5 && addr < 0x4ffa ) {
		addr -= 0x4ff6;
		memcpy ( &memory[0x4000], &bank_image[addr*0x1000] , 0x1000*sizeof(BYTE));
		return 0;
	}
	else if ( addr > 0x5ff5 && addr < 0x5ffa ) {
		addr -= 0x5ff6;
		memcpy ( &memory[0x5000], &bank_image[0x4000+addr*0x1000] , 0x1000*sizeof(BYTE));
		return 0;
	}
	else
		return memory[ addr ];
}

BYTE read_5200_rom_bb_pc ( WORD addr ) {

	if ( addr > 0x8ff5 && addr < 0x8ffa ) {
		addr -= 0x8ff6;
		memcpy ( &memory[0x4000], &bank_image[addr*0x1000] , 0x1000*sizeof(BYTE));
		return 0;
	}
	else if ( addr > 0x9ff5 && addr < 0x9ffa ) {
		addr -= 0x9ff6;
		memcpy ( &memory[0x5000], &bank_image[0x4000+addr*0x1000] , 0x1000*sizeof(BYTE));
		return 0;
	}
	else
		return memory[ addr ];
}

BYTE read_5200_rom_bb_pc_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( addr > 0x8ff5 && addr < 0x8ffa ) {
		addr -= 0x8ff6;
		memcpy ( &memory[0x4000], &bank_image[addr*0x1000] , 0x1000*sizeof(BYTE));
		return 0;
	}
	else if ( addr > 0x9ff5 && addr < 0x9ffa ) {
		addr -= 0x9ff6;
		memcpy ( &memory[0x5000], &bank_image[0x4000+addr*0x1000] , 0x1000*sizeof(BYTE));
		return 0;
	}
	else
		return memory[ addr ];
}

/******************************************************************************
**  Function   :  write_5200_rom_bb                
**                                             
**  Objective  :  Bounty Bob bank switching check.
**
**  Parameters :  addr  - address in rom that had an attempted write
**                value - value to write at address
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
void write_5200_rom_bb ( WORD addr, BYTE value ) {

	if ( addr > 0x4ff5 && addr < 0x4ffa ) {
		addr -= 0x4ff6;
		memcpy ( &memory[0x4000], &bank_image[addr*0x1000] , 0x1000*sizeof(BYTE));
	}
	else if ( addr > 0x5ff5 && addr < 0x5ffa ) {
		addr -= 0x5ff6;
		memcpy ( &memory[0x5000], &bank_image[0x4000+addr*0x1000] , 0x1000*sizeof(BYTE));
	}
}

void write_5200_rom_bb_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( addr > 0x4ff5 && addr < 0x4ffa ) {
		addr -= 0x4ff6;
		memcpy ( &memory[0x4000], &bank_image[addr*0x1000] , 0x1000*sizeof(BYTE));
	}
	else if ( addr > 0x5ff5 && addr < 0x5ffa ) {
		addr -= 0x5ff6;
		memcpy ( &memory[0x5000], &bank_image[0x4000+addr*0x1000] , 0x1000*sizeof(BYTE));
	}
}

void write_5200_rom_bb_pc ( WORD addr, BYTE value ) {

	if ( addr > 0x8ff5 && addr < 0x8ffa ) {
		addr -= 0x8ff6;
		memcpy ( &memory[0x4000], &bank_image[addr*0x1000] , 0x1000*sizeof(BYTE));
	}
	else if ( addr > 0x9ff5 && addr < 0x9ffa ) {
		addr -= 0x9ff6;
		memcpy ( &memory[0x5000], &bank_image[0x4000+addr*0x1000] , 0x1000*sizeof(BYTE));
	}
}

void write_5200_rom_bb_pc_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( addr > 0x8ff5 && addr < 0x8ffa ) {
		addr -= 0x8ff6;
		memcpy ( &memory[0x4000], &bank_image[addr*0x1000] , 0x1000*sizeof(BYTE));
	}
	else if ( addr > 0x9ff5 && addr < 0x9ffa ) {
		addr -= 0x9ff6;
		memcpy ( &memory[0x5000], &bank_image[0x4000+addr*0x1000] , 0x1000*sizeof(BYTE));
	}
}

/******************************************************************************
**  Function   :  read_5200_rom_64k
**                                     
**  Objective  :  64k bank switching check.       
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_5200_rom_64k ( WORD addr ) {

	if ( (addr == 0x3ff8) || (addr == 0xbff8) ) {
		memcpy ( &memory[0x4000], &bank_image[0x0000] , 0x8000*sizeof(BYTE));
		return 0;
	}
	else if ( (addr == 0x3ff9) || (addr == 0xbff9) ) {
		memcpy ( &memory[0x4000], &bank_image[0x8000] , 0x8000*sizeof(BYTE));
		return 0;
	}
	else
		return memory[ addr ];
}

BYTE read_5200_rom_64k_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( (addr == 0x3ff8) || (addr == 0xbff8) ) {
		memcpy ( &memory[0x4000], &bank_image[0x0000] , 0x8000*sizeof(BYTE));
		return 0;
	}
	else if ( (addr == 0x3ff9) || (addr == 0xbff9) ) {
		memcpy ( &memory[0x4000], &bank_image[0x8000] , 0x8000*sizeof(BYTE));
		return 0;
	}
	else
		return memory[ addr ];
}

/******************************************************************************
**  Function   :  write_5200_rom_64k                
**                                             
**  Objective  :  64k bank switching check.         
**
**  Parameters :  addr  - address in rom that had an attempted write
**                value - value to write at address
**                                                 
**  return     :  NONE                            
******************************************************************************/ 
void write_5200_rom_64k ( WORD addr, BYTE value ) {

	if ( (addr == 0x3ff8) || (addr == 0xbff8) ) {
		memcpy ( &memory[0x4000], &bank_image[0x0000] , 0x8000*sizeof(BYTE));
	}
	else if ( (addr == 0x3ff9) || (addr == 0xbff9) ) {
		memcpy ( &memory[0x4000], &bank_image[0x8000] , 0x8000*sizeof(BYTE));
	}
}

void write_5200_rom_64k_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( (addr == 0x3ff8) || (addr == 0xbff8) ) {
		memcpy ( &memory[0x4000], &bank_image[0x0000] , 0x8000*sizeof(BYTE));
	}
	else if ( (addr == 0x3ff9) || (addr == 0xbff9) ) {
		memcpy ( &memory[0x4000], &bank_image[0x8000] , 0x8000*sizeof(BYTE));
	}
}

/******************************************************************************
**  Function   :  read_cart_32k_db ; write_cart_32k_db
**                                     
**  Objective  :  32k DB Bank Switching
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_cart_32k_db ( WORD addr ) {

	memcpy ( &memory[0x8000], &bank_image[(addr & 0x3) * 0x2000] , 0x2000*sizeof(BYTE));
	return 0xff;
}

BYTE read_cart_32k_db_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	memcpy ( &memory[0x8000], &bank_image[(addr & 0x3) * 0x2000] , 0x2000*sizeof(BYTE));
	return 0xff;
}

void write_cart_32k_db ( WORD addr, BYTE value ) {

	memcpy ( &memory[0x8000], &bank_image[(addr & 0x3) * 0x2000] , 0x2000*sizeof(BYTE));
}

void write_cart_32k_db_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	memcpy ( &memory[0x8000], &bank_image[(addr & 0x3) * 0x2000] , 0x2000*sizeof(BYTE));
}

/******************************************************************************
**  Function   :  read_cart_xegs ; write_cart_xegs
**                                     
**  Objective  :  XEGS Bank Switching
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_cart_xegs ( WORD addr ) {

	return 0xff;
}

BYTE read_cart_xegs_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return 0xff;
}

void write_cart_xegs ( WORD addr, BYTE value ) {

	int mask = (console_get_ptr()->cart->size >> 13) - 1;

	memcpy ( &memory[0x8000], &bank_image[(value & mask) * 0x2000] , 0x2000*sizeof(BYTE));
}

void write_cart_xegs_debug ( WORD addr, BYTE value ) {

	int mask = (console_get_ptr()->cart->size >> 13) - 1;

	debug_search_for_breakpoint (BK_MEM, addr );

	memcpy ( &memory[0x8000], &bank_image[(value & mask) * 0x2000] , 0x2000*sizeof(BYTE));
}

/******************************************************************************
**  Function   :  write_cart_swxegs
**                                     
**  Objective  :  SWXEGS Bank Switching (read is same as XEGS)
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
void write_cart_swxegs ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 13) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( value & 0x80 ) {
		g_mem_image = 1;
		for ( i = 0x80; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0x80; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0x8000], &bank_image[(value & mask) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

void write_cart_swxegs_debug ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 13) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( value & 0x80 ) {
		g_mem_image = 1;
		for ( i = 0x80; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0x80; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0x8000], &bank_image[(value & mask) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

/******************************************************************************
**  Function   :  write_cart_mega
**                                     
**  Objective  :  MEGA Cart Bank Switching (read is same as XEGS)
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
void write_cart_mega ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 14) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( value & 0x80 ) {
		g_mem_image = 1;
		for ( i = 0x80; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0x80; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0x8000], &bank_image[(value & mask) * 0x4000], 0x4000*sizeof(BYTE));
	}
}

void write_cart_mega_debug ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 14) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( value & 0x80 ) {
		g_mem_image = 1;
		for ( i = 0x80; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0x80; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0x8000], &bank_image[(value & mask) * 0x4000], 0x4000*sizeof(BYTE));
	}
}

/******************************************************************************
**  Function   :  write_cart_atmax
**                                     
**  Objective  :  ATMAX Cart Bank Switching (read is same as XEGS)
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
void write_cart_atmax ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 13) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;
	int bank;
	int threshold;

	if ( mask == 0x7f ) {
		bank = addr & 0xff;
		threshold = 0x80;
	}
	else {
		bank = addr & 0x3f;
		threshold = 0x10;
		if ( bank >= 0x20 )
			return;
	}

	if ( bank >= threshold ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(bank & mask) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

void write_cart_atmax_debug ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 13) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;
	int bank;
	int threshold;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( mask == 0x7f ) {
		bank = addr & 0xff;
		threshold = 0x80;
	}
	else {
		bank = addr & 0x3f;
		threshold = 0x10;
		if ( bank >= 0x20 )
			return;
	}

	if ( bank >= threshold ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(bank & mask) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

/******************************************************************************
**  Function   :  write_cart_atrax
**                                     
**  Objective  :  ATRAX Cart Bank Switching (read is same as XEGS)
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
void write_cart_atrax ( WORD addr, BYTE value ) {

	int i;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( value & 0x80 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(value & 0xf) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

void write_cart_atrax_debug ( WORD addr, BYTE value ) {

	int i;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( value & 0x80 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(value & 0xf) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

/******************************************************************************
**  Function   :  write_cart_will;read_cart_will
**                                     
**  Objective  :  WILL Cart Bank Switching 
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
void write_cart_will ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 13) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(addr & mask) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

void write_cart_will_debug ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 13) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(addr & mask) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

BYTE read_cart_will ( WORD addr ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 13) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(addr & mask) * 0x2000], 0x2000*sizeof(BYTE));
	}
	
	return 0xff;
}

BYTE read_cart_will_debug ( WORD addr ) {

	int i;
	int debug;
	int mask = ((console_get_ptr()->cart->size >> 13) - 1);
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(addr & mask) * 0x2000], 0x2000*sizeof(BYTE));
	}

	return 0xff;
}

/******************************************************************************
**  Function   :  write_cart_sdx;read_cart_sdx
**                                     
**  Objective  :  EXP, SDX, and Diamond Cart Bank Switching 
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
void write_cart_sdx ( WORD addr, BYTE value ) {

	int i;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( console_get_ptr()->mapping == CART_SDX ) {
		if ( (addr & 0xf0) != 0xe0 ) 
			return;
	}
	else if ( console_get_ptr()->mapping == CART_EXP ) {
		if ( (addr & 0xf0) != 0x70 ) 
			return;
	}
	else {
		if ( (addr & 0xf0) != 0xd0 ) 
			return;
	}

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(~addr & 0x07) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

void write_cart_sdx_debug ( WORD addr, BYTE value ) {

	int i;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( console_get_ptr()->mapping == CART_SDX ) {
		if ( (addr & 0xf0) != 0xe0 ) 
			return;
	}
	else if ( console_get_ptr()->mapping == CART_EXP ) {
		if ( (addr & 0xf0) != 0x70 ) 
			return;
	}
	else {
		if ( (addr & 0xf0) != 0xd0 ) 
			return;
	}

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(~addr & 0x07) * 0x2000], 0x2000*sizeof(BYTE));
	}
}

BYTE read_cart_sdx ( WORD addr ) {

	int i;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( console_get_ptr()->mapping == CART_SDX ) {
		if ( (addr & 0xf0) != 0xe0 ) 
			return 0xff;
	}
	else if ( console_get_ptr()->mapping == CART_EXP ) {
		if ( (addr & 0xf0) != 0x70 ) 
			return 0xff;
	}
	else {
		if ( (addr & 0xf0) != 0xd0 ) 
			return 0xff;
	}

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(~addr & 0x07) * 0x2000], 0x2000*sizeof(BYTE));
	}
	
	return 0xff;
}

BYTE read_cart_sdx_debug ( WORD addr ) {

	int i;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( console_get_ptr()->mapping == CART_SDX ) {
		if ( (addr & 0xf0) != 0xe0 ) 
			return 0xff;
	}
	else if ( console_get_ptr()->mapping == CART_EXP ) {
		if ( (addr & 0xf0) != 0x70 ) 
			return 0xff;
	}
	else {
		if ( (addr & 0xf0) != 0xd0 ) 
			return 0xff;
	}

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		memcpy ( &memory[0xa000], &bank_image[(~addr & 0x07) * 0x2000], 0x2000*sizeof(BYTE));
	}

	return 0xff;
}

/******************************************************************************
**  Function   :  write_cart_blizzard;read_cart_blizzard
**                                     
**  Objective  :  Blizzard and Phoenix Cart Bank Switching 
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
void write_cart_blizzard ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int start = 0x80 | ((console_get_ptr()->cart->size < 0x3000) ? 0x20 : 0x00);
	struct cpu *cpu = console_get_ptr()->cpu;

	g_mem_image = 1;
	for ( i = start; i < 0xc0; ++i ) {
		debug = debug_is_page_in_debug ( i );
		cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
		cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
	}
}

void write_cart_blizzard_debug ( WORD addr, BYTE value ) {

	int i;
	int debug;
	int start = 0x80 | ((console_get_ptr()->cart->size < 0x3000) ? 0x20 : 0x00);
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	g_mem_image = 1;
	for ( i = start; i < 0xc0; ++i ) {
		debug = debug_is_page_in_debug ( i );
		cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
		cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
	}
}

BYTE read_cart_blizzard ( WORD addr ) {

	int i;
	int debug;
	int start = 0x80 | ((console_get_ptr()->cart->size < 0x3000) ? 0x20 : 0x00);
	struct cpu *cpu = console_get_ptr()->cpu;

	g_mem_image = 1;
	for ( i = start; i < 0xc0; ++i ) {
		debug = debug_is_page_in_debug ( i );
		cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
		cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
	}
	
	return 0xff;
}

BYTE read_cart_blizzard_debug ( WORD addr ) {

	int i;
	int debug;
	int start = 0x80 | ((console_get_ptr()->cart->size < 0x3000) ? 0x20 : 0x00);
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	g_mem_image = 1;
	for ( i = start; i < 0xc0; ++i ) {
		debug = debug_is_page_in_debug ( i );
		cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
		cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
	}

	return 0xff;
}

/******************************************************************************
**  Function   :  read_cart_oss ; write_cart_oss
**                                     
**  Objective  :  16k OSS Bank Switching
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_cart_oss ( WORD addr ) {

	int i,b;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		if ( (addr & 0x7) == 0x7 )
			b = 1;
		else
			b = (addr & 0x7)>>1;
		memcpy ( &memory[0xa000], &bank_image[b * 0x1000], 0x1000*sizeof(BYTE));
	}

	return 0xff;
}

BYTE read_cart_oss_debug ( WORD addr ) {

	int i,b;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		if ( (addr & 0x7) == 0x7 )
			b = 1;
		else
			b = (addr & 0x7)>>1;
		memcpy ( &memory[0xa000], &bank_image[b * 0x1000], 0x1000*sizeof(BYTE));
	}

	return 0xff;
}

void write_cart_oss ( WORD addr, BYTE value ) {

	int i,b;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		if ( (addr & 0x7) == 0x7 )
			b = 1;
		else
			b = (addr & 0x7)>>1;
		memcpy ( &memory[0xa000], &bank_image[b * 0x1000], 0x1000*sizeof(BYTE));
	}
}

void write_cart_oss_debug ( WORD addr, BYTE value ) {

	int i,b;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( addr & 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		if ( (addr & 0x7) == 0x7 )
			b = 1;
		else
			b = (addr & 0x7)>>1;
		memcpy ( &memory[0xa000], &bank_image[b * 0x1000], 0x1000*sizeof(BYTE));
	}
}

/******************************************************************************
**  Function   :  read_cart_oss2 ; write_cart_oss2
**                                     
**  Objective  :  16k OSS2 Bank Switching
**                                      
**  Parameters :  addr - address in system rom to read from        
**                                           
**  return     :  value at address          
******************************************************************************/ 
BYTE read_cart_oss2 ( WORD addr ) {

	int i,b;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( (addr & 0x09) == 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		if ( (addr & 0x9) == 0x0 )
			b = 1;
		else if ( (addr & 0x9) == 0x1 )
			b = 3;
		else
			b = 2;
		memcpy ( &memory[0xa000], &bank_image[b * 0x1000], 0x1000*sizeof(BYTE));
	}

	return 0xff;
}

BYTE read_cart_oss2_debug ( WORD addr ) {

	int i,b;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( (addr & 0x09) == 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		if ( (addr & 0x9) == 0x0 )
			b = 1;
		else if ( (addr & 0x9) == 0x1 )
			b = 3;
		else
			b = 2;
		memcpy ( &memory[0xa000], &bank_image[b * 0x1000], 0x1000*sizeof(BYTE));
	}

	return 0xff;
}

void write_cart_oss2 ( WORD addr, BYTE value ) {

	int i,b;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	if ( (addr & 0x09) == 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		if ( (addr & 0x9) == 0x0 )
			b = 1;
		else if ( (addr & 0x9) == 0x1 )
			b = 3;
		else
			b = 2;
		memcpy ( &memory[0xa000], &bank_image[b * 0x1000], 0x1000*sizeof(BYTE));
	}
}

void write_cart_oss2_debug ( WORD addr, BYTE value ) {

	int i,b;
	int debug;
	struct cpu *cpu = console_get_ptr()->cpu;

	debug_search_for_breakpoint (BK_MEM, addr );

	if ( (addr & 0x09) == 0x08 ) {
		g_mem_image = 1;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_MEM_IMG | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_MEM_IMG | debug];
		}
	}
	else {
		g_mem_image = 0;
		for ( i = 0xa0; i < 0xc0; ++i ) {
			debug = debug_is_page_in_debug ( i );
			cpu->read_mem[i] = mem_read_func[FUNC_READ_ROM | debug];
			cpu->write_mem[i] = mem_write_func[FUNC_WRITE_ROM | debug];
		}
		if ( (addr & 0x9) == 0x0 )
			b = 1;
		else if ( (addr & 0x9) == 0x1 )
			b = 3;
		else
			b = 2;
		memcpy ( &memory[0xa000], &bank_image[b * 0x1000], 0x1000*sizeof(BYTE));
	}
}
