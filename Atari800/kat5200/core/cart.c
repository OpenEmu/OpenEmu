/******************************************************************************
*
* FILENAME: cart.c
*
* DESCRIPTION:  This contains functions for handling carts for each system
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/05/07  bberlin      Creation
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "cart.h"
#include "memory.h"
#include "console.h"

e_cart_type cart_find_16k_mapping ( unsigned long crc );

t_cart cart;
t_cart cart_right;

/******************************************************************************
**  Function   :  cart_open
**                            
**  Objective  :  This function will load the cart rom into memory
**
**  Parameters :  file   - file name of file containing image 
**                buffer - contains the rom contents
**                size   - size of the buffer
**                crc    - checksum of the rom
**                mapping- rom to memory mapping type
**                launch - whether we are booting the image or just attaching
**                                                
**  return     :  0 if success                   
**                1 if failure                  
******************************************************************************/ 
int cart_open ( char *file, unsigned char *buffer, int size, unsigned long crc, 
                                                   e_cart_type mapping, int launch  ) {

	strcpy ( cart.filename, file );
	if ( launch )
		console_set_media_type ( CART_CART );

	/*
	 * First check for 5200 Bounty Bob
	 */
	if ( crc == 0x74e6a611 )      {  /* $4000 */
		memory_write_bank_bytes ( 0x0000, buffer, 0x4000 );
		memory_write_rom_bytes ( 0x4000, buffer, 0x1000 );
		cart.mapping = CART_40K;
	}
	else if ( crc == 0xefa4915d ) {  /* $5000 */
		memory_write_bank_bytes ( 0x4000, buffer, 0x4000 );
		memory_write_rom_bytes ( 0x5000, buffer, 0x1000 );
		cart.mapping = CART_40K;
	}
	else if ( crc == 0x02931055  ) { /* $A000 */
		memory_write_rom_bytes ( 0x8000, buffer, 0x2000 );
		memory_write_rom_bytes ( 0xa000, buffer, 0x2000 );
		cart.mapping = CART_40K;
	}

	/*
	 * Now check for 4k and 8k roms
	 */
	else if ( size < 5000 ) {
		memory_write_rom_bytes ( 0x8000, buffer, 0x1000 );
		memory_write_rom_bytes ( 0x9000, buffer, 0x1000 );
		memory_write_rom_bytes ( 0xa000, buffer, 0x1000 );
		memory_write_rom_bytes ( 0xb000, buffer, 0x1000 );
		cart.mapping = CART_4K;
	}
	else if ( size < 10000 ) {
		if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
			memory_write_rom_bytes ( 0x8000, buffer, 0x2000 );
			memory_write_rom_bytes ( 0xa000, buffer, 0x2000 );
			cart.mapping = CART_8K;
		}
		else if ( mapping == CART_PC_RIGHT ) {
			memory_write_rom_bytes ( 0x8000, buffer, 0x2000 );
			cart.mapping = CART_PC_RIGHT;
		}
		else {
			memory_write_rom_bytes ( 0xa000, buffer, 0x2000 );
			if ( mapping == CART_PHOENIX )
				cart.mapping = CART_PHOENIX;
			else
				cart.mapping = CART_PC_8K;
		}
	}

	/*
	 * Now check for 16k roms
	 */
	else if ( size < 20000 ) {
		if ( !mapping )
			mapping = cart_find_16k_mapping ( crc );

		if ( mapping == CART_16K_ONE_CHIP ) {
			memory_write_rom_bytes ( 0x4000, buffer, 0x4000 );
			memory_write_rom_bytes ( 0x8000, buffer, 0x4000 );
			cart.mapping = CART_16K_ONE_CHIP;
		}
		else {
			memory_write_rom_bytes ( 0x4000, buffer, 0x2000 );
			memory_write_rom_bytes ( 0x6000, buffer, 0x2000 );
			memory_write_rom_bytes ( 0x8000, buffer+0x2000, 0x2000 );
			memory_write_rom_bytes ( 0xa000, buffer+0x2000, 0x2000 );
			cart.mapping = CART_16K_TWO_CHIP;
		}

		if ( console_get_machine_type() != MACHINE_TYPE_5200 ) {
			if ( mapping == CART_OSS ) {
				memory_write_rom_bytes ( 0xa000, buffer, 0x1000 );
				memory_write_rom_bytes ( 0xb000, buffer+0x3000, 0x1000 );
				cart.mapping = CART_OSS;
			}
			else if ( mapping == CART_OSS2 ) {
				memory_write_rom_bytes ( 0xa000, buffer+0x1000, 0x1000 );
				memory_write_rom_bytes ( 0xb000, buffer, 0x1000 );
				cart.mapping = CART_OSS2;
			}
			else {
				memory_write_rom_bytes ( 0x8000, buffer, 0x4000 );
				if ( mapping == CART_BLIZZARD )
					cart.mapping = CART_BLIZZARD;
				else
					cart.mapping = CART_PC_16K;
			}
		}
	}

	/*
	 * Now check for 5200 Bounty Bob 40k
	 */
	else if ( size > 40000  && size < 42000 ) {
		if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
			memory_write_bank_bytes ( 0x0000, buffer, 0x8000 );

			memory_write_rom_bytes ( 0x4000, buffer, 0x1000 );
			memory_write_rom_bytes ( 0x5000, buffer+0x4000, 0x1000 );

			memory_write_rom_bytes ( 0x8000, buffer+0x8000, 0x2000 );
			memory_write_rom_bytes ( 0xa000, buffer+0x8000, 0x2000 );
			cart.mapping = CART_40K;
		}
		else {
			memory_write_bank_bytes ( 0x0000, buffer, 0x8000 );

			memory_write_rom_bytes ( 0x8000, buffer, 0x1000 );
			memory_write_rom_bytes ( 0x9000, buffer+0x4000, 0x1000 );

			memory_write_rom_bytes ( 0xa000, buffer+0x8000, 0x2000 );
			cart.mapping = CART_PC_40K;
		}
	}

	/*
	 * Now check for 64k
	 */
	else if ( size > 62000  && size < 68000 ) {
		memory_write_bank_bytes ( 0x0000, buffer, 0x10000 );
		if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
			memory_write_rom_bytes ( 0x4000, buffer+0x8000, 0x2000 );
			cart.mapping = CART_64K;
		}
		else {
			if ( mapping == CART_MEGA ) {
				memory_write_rom_bytes ( 0x8000, buffer, 0x4000 );
				cart.mapping = CART_MEGA;
			}
			if ( mapping == CART_WILL || mapping == CART_SDX ||
			     mapping == CART_EXP || mapping == CART_DIAMOND ) {
				memory_write_rom_bytes ( 0xa000, buffer, 0x2000 );
				cart.mapping = CART_WILL;
			}
			else {
				memory_write_rom_bytes ( 0x8000, buffer, 0x2000 );
				memory_write_rom_bytes ( 0xa000, buffer + (size-0x2000), 0x2000 );
				if ( mapping == CART_XEGS )
					cart.mapping = CART_XEGS;
				else
					cart.mapping = CART_SWXEGS;
			}
		}
	}

	/*
	 * Now check for 128k
	 */
	else if ( size > 130000  && size < 132000 ) {
		memory_write_bank_bytes ( 0x0000, buffer, 0x20000 );
		if ( mapping == CART_MEGA ) {
			memory_write_rom_bytes ( 0x8000, buffer, 0x4000 );
			cart.mapping = CART_MEGA;
		}
		else if ( mapping == CART_ATMAX ) {
			memory_write_rom_bytes ( 0xa000, buffer, 0x2000 );
			cart.mapping = CART_ATMAX;
		}
		else if ( mapping == CART_ATRAX ) {
			memory_write_rom_bytes ( 0xa000, buffer, 0x2000 );
			cart.mapping = CART_ATRAX;
		}
		else {
			memory_write_rom_bytes ( 0x8000, buffer, 0x2000 );
			memory_write_rom_bytes ( 0xa000, buffer + (size-0x2000), 0x2000 );
			if ( mapping == CART_XEGS )
				cart.mapping = CART_XEGS;
			else
				cart.mapping = CART_SWXEGS;
		}
	}

	/*
	 * Now check for 1024K
	 */
	else if ( size > 1040000  && size < 1050000 ) {
		memory_write_bank_bytes ( 0x0000, buffer, 0x100000 );
		if ( mapping == CART_ATMAX ) {
			memory_write_rom_bytes ( 0xa000, buffer + (size-0x2000), 0x2000 );
			cart.mapping = CART_ATMAX;
		}
	}

	/*
	 * Must be a standard 32k
	 */
	else if ( size > 31000 && size < 34000 ) {
		if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
			memory_write_rom_bytes ( 0x4000, buffer, 0x8000 );
			cart.mapping = CART_32K;
		}
		else if ( mapping == CART_MEGA ) {
			memory_write_bank_bytes ( 0x0000, buffer, 0x8000 );
			memory_write_rom_bytes ( 0x8000, buffer, 0x4000 );
			cart.mapping = CART_MEGA;
		}
		else if ( mapping == CART_WILL ) {
			memory_write_rom_bytes ( 0xa000, buffer, 0x2000 );
			cart.mapping = CART_WILL;
		}
		else if ( mapping == CART_XEGS ) {
			memory_write_bank_bytes ( 0x0000, buffer, 0x8000 );
			memory_write_rom_bytes ( 0x8000, buffer, 0x2000 );
			memory_write_rom_bytes ( 0xa000, buffer+0x6000, 0x2000 );
			cart.mapping = CART_XEGS;
		}
		else {
			memory_write_bank_bytes ( 0x0000, buffer, 0x8000 );
			memory_write_rom_bytes ( 0x8000, buffer, 0x2000 );
			memory_write_rom_bytes ( 0xa000, buffer+0x6000, 0x2000 );
			cart.mapping = CART_PC_32K;
		}
	}
	else {
		return -1;
	}

	return 0;

} /* end cart_open */

/******************************************************************************
**  Function   :  cart_find_16k_mapping
**                            
**  Objective  :  This function determine which mapping (one chip or two chip)
**                to use if none is specified (for the 5200).
**
**  Parameters :  crc - rom checksum to check
**                                                
**  return     :  One chip or Two chip mapping
******************************************************************************/ 
e_cart_type cart_find_16k_mapping ( unsigned long crc ) {

	if ( crc == 0xc8f9c094 ||     /* Blaster           */
	     crc == 0x35484751 ||     /* AE                */
	     crc == 0x9bae58dc ||     /* Beamrider         */
	     crc == 0xbe3cd348 ||     /* Berzerk           */
	     crc == 0x0624E6E7 ||     /* BluePrint         */
	     crc == 0x9ad53bbc ||     /* ChopLifter        */
	     crc == 0xf43e7cd0 ||     /* Decathlon         */
	     crc == 0xd3bd3221 ||     /* Final Legacy      */
	     crc == 0x18a73af3 ||     /* H.E.R.O           */
	     crc == 0x83517703 ||     /* Last StarFigtr    */
	     crc == 0xab8e035b ||     /* Meteorites        */
	     crc == 0x969cfe1a ||     /* Millipede         */
	     crc == 0x7df1adfb ||     /* Miner 2049er      */
	     crc == 0xb8b6a2fd ||     /* Missle Command+   */
	     crc == 0xd0b2f285 ||     /* Moon Patrol       */
	     crc == 0xe8b130c4 ||     /* PAM Diags2.0      */
	     crc == 0x4b910461 ||     /* Pitfall II        */
	     crc == 0xb5f3402b ||     /* Quest Quintana    */
	     crc == 0x4252abd9 ||     /* Robotron 2084     */
	     crc == 0x387365dc ||     /* Space Shuttle     */
	     crc == 0x82E2981F ||     /* Super Pacman      */
	     crc == 0x1187342f ||     /* Tempest           */
	     crc == 0xe80dbb2  ||     /* Time Runner (Conv) */
	     crc == 0x0ba22ece ||     /* Track and Field   */
	     crc == 0x47dc1314 ||     /* Preppie (Conv)    */
	     crc == 0xF1E21530 ||     /* Preppie (Conv)    */
	     crc == 0xd6f7ddfd ||     /* Wizard of Wor     */
	     crc == 0x2959d827 ||     /* Zone Ranger       */
	     crc == 0x38F4A6A4 ||     /* Xmas (Demo)       */
	     crc == 0xa4ddb1e     ) { /* Super Pacman      */
		return CART_16K_ONE_CHIP;
	}
	else
		return CART_16K_TWO_CHIP;

} /* cart_find_16k_mapping */

/******************************************************************************
**  Function   :  cart_get_mapping_string
**                            
**  Objective  :  This function looks up the string associated with the input
**                mapping.
**
**  Parameters :  type - input to look up
**                desc - string containing result
**                                                
**  return     :  pointer to input string
******************************************************************************/ 
char * cart_get_mapping_string ( e_cart_type type, char *desc ) {

	switch ( type ) {
		case CART_MIN: strcpy ( desc, "Auto" ); break;
		case CART_32K: strcpy ( desc, "32K Standard" ); break;
		case CART_16K_TWO_CHIP: strcpy ( desc, "16K Two Chip" ); break;
		case CART_16K_ONE_CHIP: strcpy ( desc, "16K One Chip" ); break;
		case CART_40K: strcpy ( desc, "40K Bounty Bob" ); break;
		case CART_64K: strcpy ( desc, "64K Bank Switch" ); break;
		case CART_8K: strcpy ( desc, "8K Standard" ); break;
		case CART_4K: strcpy ( desc, "4K Standard" ); break;
		case CART_PC_8K: strcpy ( desc, "8K Standard" ); break;
		case CART_PC_16K: strcpy ( desc, "16K Standard" ); break;
		case CART_PC_32K: strcpy ( desc, "32K DB" ); break;
		case CART_PC_40K: strcpy ( desc, "40K Bounty Bob" ); break;
		case CART_PC_RIGHT: strcpy ( desc, "8K Right Slot" ); break;
		case CART_XEGS: strcpy ( desc, "XEGS Bank Switch" ); break;
		case CART_SWXEGS: strcpy ( desc, "SWXEGS Bank Switch" ); break;
		case CART_MEGA: strcpy ( desc, "MegaCart" ); break;
		case CART_OSS: strcpy ( desc, "OSS SuperCart" ); break;
		case CART_OSS2: strcpy ( desc, "OSS2 SuperCart" ); break;
		case CART_ATMAX: strcpy ( desc, "ATMax Cart" ); break;
		case CART_ATRAX: strcpy ( desc, "ATRax Cart" ); break;
		case CART_WILL: strcpy ( desc, "WILL Cart" ); break;
		case CART_SDX: strcpy ( desc, "SDX Cart" ); break;
		case CART_EXP: strcpy ( desc, "EXP Cart" ); break;
		case CART_DIAMOND: strcpy ( desc, "Diamond Cart" ); break;
		case CART_PHOENIX: strcpy ( desc, "Phoenix Cart" ); break;
		case CART_BLIZZARD: strcpy ( desc, "Blizzard Cart" ); break;
		case CART_FLOPPY: strcpy ( desc, "Floppy" ); break;
		case CART_CASSETTE: strcpy ( desc, "Cassette" ); break;
		case CART_EXE: strcpy ( desc, "Executable" ); break;
		default: strcpy ( desc, "Unknown" ); break;
	}

	return desc;

} /* cart_get_mapping_string */
