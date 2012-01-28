
/****************************************************************************
 *  Genesis Plus
 *  Mega Drive cartridge hardware support
 *
 *  Copyright (C) 2007-2011  Eke-Eke (GCN/Wii port)
 *
 *  Most cartridge protections documented by Haze
 *  (http://haze.mameworld.info/)
 *
 *  Realtec mapper documented by TascoDeluxe
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#include "shared.h"
#include "gamepad.h"

#define CART_CNT (44)

/* Cart database entry */
typedef struct
{
  uint16 chk_1;       /* header checksum */
  uint16 chk_2;       /* real checksum */
  uint8 bank_start;   /* first mapped bank in $400000-$7fffff region */
  uint8 bank_end;     /* last mapped bank in $400000-$7fffff region */
  T_CART_HW cart_hw;  /* hardware description */
} T_CART_ENTRY;

extern int emulate_address_error;

/* Cartridge Hardware structure */
T_CART cart;

/* Function prototypes */
static void mapper_sega_w(uint32 data);
static void mapper_ssf2_w(uint32 address, uint32 data);
static void mapper_realtec_w(uint32 address, uint32 data);
static void mapper_seganet_w(uint32 address, uint32 data);
static void mapper_32k_w(uint32 data);
static void mapper_64k_w(uint32 data);
static void mapper_64k_multi_w(uint32 address);
static uint32 mapper_radica_r(uint32 address);
static void default_time_w(uint32 address, uint32 data);
static void default_regs_w(uint32 address, uint32 data);
static uint32 default_regs_r(uint32 address);
static uint32 default_regs_r_16(uint32 address);
static void custom_regs_w(uint32 address, uint32 data);
static void custom_alt_regs_w(uint32 address, uint32 data);
static uint32 topshooter_read(uint32 address);
static void topshooter_write(uint32 address, uint32 data);

/* Games that need extra hardware emulation:
  - copy protection device
  - custom ROM banking device
*/
static const T_CART_ENTRY rom_database[CART_CNT] =
{
/* Funny World & Balloon Boy */
  {0x0000,0x06ab,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},1,0,NULL,NULL,NULL,mapper_realtec_w}},
/* Whac-a-Critter */
  {0xffff,0xf863,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},1,0,NULL,NULL,NULL,mapper_realtec_w}},
/* Earth Defense */
  {0xffff,0x44fb,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},1,0,NULL,NULL,NULL,mapper_realtec_w}},
/* RADICA (Volume 1) */
  {0x0000,0x2326,0x00,0x00,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},0,0,mapper_radica_r,NULL,NULL,NULL}},
/* RADICA (Volume 2) */
  {0x4f10,0x0836,0x00,0x00,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},0,0,mapper_radica_r,NULL,NULL,NULL}},
/* RADICA (Volume 1) (byteswapped version) */
  {0xf424,0x9f82,0x00,0x00,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},0,0,mapper_radica_r,NULL,NULL,NULL}},
/* Chinese Fighters III */
  {0x9490,0x8180,0x40,0x6f,{{0x00,0x00,0x00,0x00},{0xf0000c,0xf0000c,0xf0000c,0xf0000c},{0x400000,0x400004,0x400008,0x40000c},0,1,NULL,NULL,default_regs_r,custom_alt_regs_w}},
/* Top Fighter */
  {0x4eb9,0x5d8b,0x60,0x7f,{{0x00,0x00,0x00,0x00},{0xf00007,0xf00007,0xf00007,0xffffff},{0x600001,0x600003,0x600005,0x000000},0,1,NULL,NULL,default_regs_r,custom_regs_w}},
/* Mulan */
  {0x0404,0x1b40,0x60,0x7f,{{0x00,0x00,0x00,0x00},{0xf00007,0xf00007,0xf00007,0xffffff},{0x600001,0x600003,0x600005,0x000000},0,1,NULL,NULL,default_regs_r,custom_regs_w}},
/* Pocket Monsters II */
  {0x47f9,0x17e5,0x60,0x7f,{{0x00,0x00,0x00,0x00},{0xf00007,0xf00007,0xf00007,0xffffff},{0x600001,0x600003,0x600005,0x000000},0,1,NULL,NULL,default_regs_r,custom_regs_w}},
/* Lion King 3 */
  {0x0000,0x507c,0x60,0x7f,{{0x00,0x00,0x00,0x00},{0xf00007,0xf00007,0xf00007,0xffffff},{0x600001,0x600003,0x600005,0x000000},0,1,NULL,NULL,default_regs_r,custom_regs_w}},
/* Super King Kong 99 */
  {0x0000,0x7d6e,0x60,0x7f,{{0x00,0x00,0x00,0x00},{0xf00007,0xf00007,0xf00007,0xffffff},{0x600001,0x600003,0x600005,0x000000},0,1,NULL,NULL,default_regs_r,custom_regs_w}},
/* Pokemon Stadium */
  {0x0000,0x843c,0x70,0x7f,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},0,1,NULL,NULL,NULL,custom_regs_w}},
/* Lion King 2 */
  {0xffff,0x1d9b,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xfffffd,0xfffffd,0xffffff,0xffffff},{0x400000,0x400004,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,default_regs_w}},
/* Squirell King */
  {0x0000,0x8ec8,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xfffffd,0xfffffd,0xffffff,0xffffff},{0x400000,0x400004,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,default_regs_w}},
/* Lian Huan Pao - Barver Battle Saga (registers accessed by Z80, related to sound engine ?) */
  {0x30b9,0x1c2a,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xfffffd,0xfffffd,0xffffff,0xffffff},{0x400000,0x400004,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,default_regs_w}},
/* Shui Hu Zhuan (registers accessed by Z80, related to sound engine ?) */
  {0x6001,0x0211,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xfffffd,0xfffffd,0xffffff,0xffffff},{0x400000,0x400004,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,default_regs_w}},
/* Feng Shen Ying Jie Chuan (registers accessed by Z80, related to sound engine ?) */
  {0xffff,0x5d98,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xfffffd,0xfffffd,0xffffff,0xffffff},{0x400000,0x400004,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,default_regs_w}},
/* (*) Shui Hu - Feng Yun Zhuan (patched ROM, unused registers) */
  {0x3332,0x872b,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xfffffd,0xfffffd,0xffffff,0xffffff},{0x400000,0x400004,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,default_regs_w}},
/* (*) Chao Ji Da Fu Weng (patched ROM, various words witten to register, long word also read from $7E0000, unknown banking hardware ?) */
  {0xa697,0xa697,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x000000,0x000000,0x000000},0,0,NULL,NULL,NULL,default_regs_w}},
/* (*) Aq Renkan Awa (patched ROM, ON/OFF bit sequence is written to register, unknown banking hardware ?) */
  {0x8104,0x0517,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400001,0x000000,0x000000,0x000000},0,0,NULL,NULL,NULL,default_regs_w}},
/* (*) Jiu Ji Ma Jiang II - Ye Yan Bian (patched ROM, using expected register value - $0f - crashes the game) (uses 16-bits reads) */
  {0x0c44,0xba81,0x40,0x40,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x400006},0,0,NULL,NULL,default_regs_r_16,NULL}},
/* (*) Tun Shi Tian Di III (patched ROM, unused register) */
  {0x0000,0x9c5e,0x40,0x40,{{0xab,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400046,0x000000,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,NULL}},
/* Ma Jiang Qing Ren - Ji Ma Jiang Zhi */
  {0x0000,0x7037,0x40,0x40,{{0x90,0xd3,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x401000,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,NULL}},
/* 16 Zhang Ma Jiang (uses 16-bits reads) */
  {0xfb40,0x4bed,0x40,0x40,{{0x00,0xaa,0x00,0xf0},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x400002,0x000000,0x400006},0,0,NULL,NULL,default_regs_r_16,NULL}},
/* King of Fighter 98 */
  {0x0000,0xd0a0,0x48,0x4f,{{0x00,0x00,0xaa,0xf0},{0xffffff,0xffffff,0xfc0000,0xfc0000},{0x000000,0x000000,0x480000,0x4c0000},0,0,NULL,NULL,default_regs_r,NULL}},
/* Supper Bubble Bobble */
  {0x0000,0x16cd,0x40,0x40,{{0x55,0x0f,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x000000,0x000000},0,0,NULL,NULL,default_regs_r,NULL}},
/* Huan Le Tao Qi Shu - Smart Mouse */
  {0x0000,0x1a28,0x40,0x40,{{0x55,0x0f,0xaa,0xf0},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,NULL,NULL,default_regs_r,NULL}},
/* (*) Hei Tao 2 - Super Big 2 (patched ROM, unused registers) */
  {0x0000,0x5843,0x40,0x40,{{0x55,0x0f,0xaa,0xf0},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,NULL,NULL,default_regs_r,NULL}},
/* Elf Wor */
  {0x0080,0x3dba,0x40,0x40,{{0x55,0x0f,0xc9,0x18},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,NULL,NULL,default_regs_r,NULL}},
/* Ya-Se Chuanshuo */
  {0xffff,0xd472,0x40,0x40,{{0x63,0x98,0xc9,0x18},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,NULL,NULL,default_regs_r,NULL}},
/* 777 Casino (For first one, 0x55 works as well. Other values are never used so they are guessed from on other unlicensed games using similar mapper) */
  {0x0000,0xf8d9,0x40,0x40,{{0x63,0x98,0xc9,0x18},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,NULL,NULL,default_regs_r,NULL}},
/* Soul Blade */
  {0x0000,0x0c5b,0x40,0x40,{{0x63,0x98,0xc9,0xf0},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,NULL,NULL,default_regs_r,NULL}},
/* Rockman X3 (half-patched ROM, two last register values are not used, 0xaa/0x18 works too) */
  {0x0000,0x9d0e,0x40,0x40,{{0x0c,0x00,0xc9,0xf0},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0x000000,0x400004,0x400006},0,0,default_regs_r,NULL,default_regs_r,NULL}},
/* (*) Tekken 3 Special (patched ROM, register value not used, unknown writes to $400000-$40000E, read from $400002) */
  {0x0000,0x8c6e,0x00,0x00,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0x000000,0x000000,0x000000},0,0,default_regs_r,NULL,NULL,NULL}},
/* (*) Dragon Ball Final Bout  (patched ROM, in original code, different switches occurs depending on returned value $00-$0f) */
  {0xc65a,0xc65a,0x00,0x00,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0x000000,0x000000,0x000000},0,0,default_regs_r,NULL,NULL,NULL}},
/* (*) Yang Jia Jiang - Yang Warrior Family (patched ROM, register value unused) */
  {0x0000,0x96b0,0x00,0x00,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0x000000,0x000000,0x000000},0,0,default_regs_r,NULL,NULL,NULL}},
/* Super Mario 2 1998 */
  {0xffff,0x0474,0x00,0x00,{{0x0a,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0x000000,0x000000,0x000000},0,0,default_regs_r,NULL,NULL,NULL}},
/* Super Mario World */
  {0x2020,0xb4eb,0x00,0x00,{{0x1c,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0x000000,0x000000,0x000000},0,0,default_regs_r,NULL,NULL,NULL}},
/* A Bug's Life */
  {0x7f7f,0x2aad,0x00,0x00,{{0x28,0x1f,0x01,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0xa13002,0xa1303e,0x000000},0,0,default_regs_r,NULL,NULL,NULL}},
/* King of Fighter 99 */
  {0x0000,0x021e,0x00,0x00,{{0x00,0x01,0x1f,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0xa13002,0xa1303e,0x000000},0,0,default_regs_r,NULL,NULL,NULL}},
/* Pocket Monster */
  {0xd6fc,0x1eb1,0x00,0x00,{{0x00,0x01,0x1f,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0xa13000,0xa13002,0xa1303e,0x000000},0,0,default_regs_r,NULL,NULL,NULL}},
/* Game no Kanzume Otokuyou */
  {0x0000,0xf9d1,0x00,0x00,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},0,0,NULL,mapper_seganet_w,NULL,NULL}},
/* Top Shooter (arcade hardware) */
  {0xffff,0x3632,0x20,0x20,{{0x00,0x00,0x00,0x00},{0xffffff,0xffffff,0xffffff,0xffffff},{0x000000,0x000000,0x000000,0x000000},0,0,NULL,NULL,topshooter_read,topshooter_write}}
};


/************************************************************
          Cart Hardware initialization 
*************************************************************/

/* cart hardware detection */
void md_cart_init(void)
{
  int i;

  /***************************************************************************************************************
                CARTRIDGE ROM MIRRORING                                                                                   
   ***************************************************************************************************************
  
    Cartridge area is mapped to $000000-$3fffff:

      -> when accessing ROM, 68k address lines A1 to A21 are used by the internal cartridge hardware to decode the
         full 4MB address range.
      -> depending on the ROM total size, some address lines might be ignored, resulting in ROM mirroring.


    Cartridges can use either 8-bits (x2) or 16-bits (x1, x2) Mask ROM chips, each chip size is a factor of 2 bytes:

      -> two 8-bits chips are equivalent to one 16-bits chip, no specific address decoding is required, needed
         address lines are simply connected to each chip, upper address lines are ignored and data lines are
         connected appropriately to each chip (D0-D7 to one chip, D8-D15 to the other one).
         ROM is mirrored each N bytes where N=2^(k+1) is the total ROM size (ROM1+ROM2,ROM1+ROM2,...).

      -> one single 16-bits chip do not need specific address decoding, address lines are simply connected
         depending on the ROM size, upper address lines being ignored.
         ROM is mirrored each N bytes where N=2^k is the size of the ROM chip (ROM1,ROM1,ROM1,...).

      -> two 16-bits chips of the same size are equivalent to one chip of double size, address decoding generally
         is the same except that specific hardware is used (one address line is generally used for chip selection,
         lower ones being used to address the chips and upper ones being ignored).
         ROM is mirrored continuously each N bytes where N=2^(k+1) is the total ROM size (ROM1,ROM2,ROM1,ROM2,...).

      -> two 16-bits chips with different size are mapped differently. Address decoding is done the same way as
         above (one address line used for chip selection) but the ignored & required address lines differ from
         one chip to another, which makes ROM mirroring different.
         ROM2 size is generally half of ROM1 size and ROM are mirrored like that : ROM1,ROM2,ROM2,ROM1,ROM2,ROM2,...

      From the emulator point of view, we only need to distinguish 3 cases:

      1/ total ROM size is a factor of 2: ROM is mirrored each 2^k bytes.

      2/ total ROM size is not a factor of 2 and cartridge uses one or two chips of the same size (Type A):
         ROM is padded up to 2^k and mirrored each 2^k bytes.

      3/ total ROM size is not a factor of 2 and cartridge uses two chips of different sizes (Type B):
         ROM is not padded and the first 2^(k-1) bytes are mirrored each 2^k bytes while the next 2^(k-2) bytes are
         mirrored in the last 2^(k-2) bytes.

  ******************************************************************************************************************/
  
  /* calculate nearest size with factor of 2 */
  unsigned int size = 0x10000;
  while (cart.romsize > size)
    size <<= 1;

  /* total ROM size is not a factor of 2  */
  /* TODO: handle more possible ROM configurations (using cartridge database ???) */
  if ((size < MAXROMSIZE) && (cart.romsize < size))
  {
    /* two chips with different size */
    if (config.romtype)
    {
      /* third ROM section is mirrored in the last section */
      memcpy(cart.rom + cart.romsize, cart.rom + 2*cart.romsize - size, size - cart.romsize);
    }
    else
    {
      /* ROM is padded up to 2^k bytes */
      memset(cart.rom + cart.romsize, 0xff, size - cart.romsize);
    }
  }

  /* special case: Sonic & Knuckles */
  /* $200000-$3fffff is mapped to external cartridge */
  if (strstr(rominfo.international,"SONIC & KNUCKLES") != NULL)
  {
    /* disable ROM mirroring */
    size = 0x400000;
  }

  /* ROM is mirrored each 2^k bytes */
  cart.mask = size - 1;

  /**********************************************
          DEFAULT CARTRIDGE MAPPING 
  ***********************************************/
  for (i=0; i<0x40; i++)
  {
    /* cartridge ROM */
    m68k_memory_map[i].base     = cart.rom + ((i<<16) & cart.mask);
    m68k_memory_map[i].read8    = NULL;
    m68k_memory_map[i].read16   = NULL;
    m68k_memory_map[i].write8   = m68k_unused_8_w;
    m68k_memory_map[i].write16  = m68k_unused_16_w;
    zbank_memory_map[i].read    = NULL;
    zbank_memory_map[i].write   = zbank_unused_w;
  }

  for (i=0x40; i<0x80; i++)
  {
    /* unused area */
    m68k_memory_map[i].base     = cart.rom + (i<<16);
    m68k_memory_map[i].read8    = m68k_read_bus_8;
    m68k_memory_map[i].read16   = m68k_read_bus_16;
    m68k_memory_map[i].write8   = m68k_unused_8_w;
    m68k_memory_map[i].write16  = m68k_unused_16_w;
    zbank_memory_map[i].read    = zbank_unused_r;
    zbank_memory_map[i].write   = zbank_unused_w;
  }

  /**********************************************
          BACKUP MEMORY 
  ***********************************************/
  sram_init();
  eeprom_init();
  if (sram.on)
  {
    if (sram.custom)
    {
      /* Serial EEPROM */
      m68k_memory_map[eeprom.type.sda_out_adr >> 16].read8  = eeprom_read_byte;
      m68k_memory_map[eeprom.type.sda_out_adr >> 16].read16 = eeprom_read_word;
      m68k_memory_map[eeprom.type.sda_in_adr >> 16].read8   = eeprom_read_byte;
      m68k_memory_map[eeprom.type.sda_in_adr >> 16].read16  = eeprom_read_word;
      m68k_memory_map[eeprom.type.scl_adr >> 16].write8     = eeprom_write_byte;
      m68k_memory_map[eeprom.type.scl_adr >> 16].write16    = eeprom_write_word;
      zbank_memory_map[eeprom.type.sda_out_adr >> 16].read  = eeprom_read_byte;
      zbank_memory_map[eeprom.type.sda_in_adr >> 16].read   = eeprom_read_byte;
      zbank_memory_map[eeprom.type.scl_adr >> 16].write     = eeprom_write_byte;
    }
    else
    {
      /* Static RAM (64k max.) */
      m68k_memory_map[sram.start >> 16].base    = sram.sram;
      m68k_memory_map[sram.start >> 16].read8   = NULL;
      m68k_memory_map[sram.start >> 16].read16  = NULL;
      m68k_memory_map[sram.start >> 16].write8  = NULL;
      m68k_memory_map[sram.start >> 16].write16 = NULL;
      zbank_memory_map[sram.start >> 16].read   = NULL;
      zbank_memory_map[sram.start >> 16].write  = NULL;
    }
  }

  /**********************************************
          SVP CHIP 
  ***********************************************/
  svp = NULL;
  if (strstr(rominfo.international,"Virtua Racing") != NULL)
  {
    svp_init();

    m68k_memory_map[0x30].base    = svp->dram;
    m68k_memory_map[0x30].read16  = NULL;
    m68k_memory_map[0x30].write16 = svp_write_dram;

    m68k_memory_map[0x31].base    = svp->dram + 0x10000;
    m68k_memory_map[0x31].read16  = NULL;
    m68k_memory_map[0x31].write16 = svp_write_dram;

    m68k_memory_map[0x39].read16  = svp_read_cell_1;
    m68k_memory_map[0x3a].read16  = svp_read_cell_2;
  }

  /**********************************************
          SPECIFIC PERIPHERAL SUPPORT 
  ***********************************************/

  /* restore previous input settings */
  if (old_system[0] != -1)
  {
    input.system[0] = old_system[0];
  }
  if (old_system[1] != -1)
  {
    input.system[1] = old_system[1];
  }

  /* default GUN settings */
  input.x_offset = 0x00;
  input.y_offset = 0x00;

  /* SEGA Menacer */
  if (strstr(rominfo.international,"MENACER") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1)
    {
      old_system[0] = input.system[0];
    }
    if (old_system[1] == -1)
    {
      old_system[1] = input.system[1];
    }

    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 0x52;
    input.y_offset = 0x00;
  }
  else if (strstr(rominfo.international,"T2 ; THE ARCADE GAME") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1)
    {
      old_system[0] = input.system[0];
    }
    if (old_system[1] == -1)
    {
      old_system[1] = input.system[1];
    }

    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 0x84;
    input.y_offset = 0x08;
  }
  else if (strstr(rominfo.international,"BODY COUNT") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1)
    {
      old_system[0] = input.system[0];
    }
    if (old_system[1] == -1)
    {
      old_system[1] = input.system[1];
    }

    input.system[0] = SYSTEM_MOUSE;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 0x44;
    input.y_offset = 0x18;
  }

  /* KONAMI Justifiers */
  else if (strstr(rominfo.international,"LETHAL ENFORCERSII") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1)
    {
      old_system[0] = input.system[0];
    }
    if (old_system[1] == -1)
    {
      old_system[1] = input.system[1];
    }

    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_JUSTIFIER;
    input.x_offset = 0x18;
    input.y_offset = 0x00;
  }
  else if (strstr(rominfo.international,"LETHAL ENFORCERS") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1)
    {
      old_system[0] = input.system[0];
    }
    if (old_system[1] == -1)
    {
      old_system[1] = input.system[1];
    }

    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_JUSTIFIER;
    input.x_offset = 0x00;
    input.y_offset = 0x00;
  }

  cart.special = 0;

  /**********************************************
          J-CART 
  ***********************************************/
  if (((strstr(rominfo.product,"00000000")  != NULL) && (rominfo.checksum == 0x168b)) ||  /* Super Skidmarks, Micro Machines Military*/
      ((strstr(rominfo.product,"00000000")  != NULL) && (rominfo.checksum == 0x165e)) ||  /* Pete Sampras Tennis (1991), Micro Machines 96 */
      ((strstr(rominfo.product,"00000000")  != NULL) && (rominfo.checksum == 0xcee0)) ||  /* Micro Machines Military (bad) */
      ((strstr(rominfo.product,"00000000")  != NULL) && (rominfo.checksum == 0x2c41)) ||  /* Micro Machines 96 (bad) */
      ((strstr(rominfo.product,"XXXXXXXX")  != NULL) && (rominfo.checksum == 0xdf39)) ||  /* Sampras Tennis 96 */
      ((strstr(rominfo.product,"T-123456")  != NULL) && (rominfo.checksum == 0x1eae)) ||  /* Sampras Tennis 96 */
      ((strstr(rominfo.product,"T-120066")  != NULL) && (rominfo.checksum == 0x16a4)) ||  /* Pete Sampras Tennis (1994)*/
      (strstr(rominfo.product,"T-120096")    != NULL))                                    /* Micro Machines 2 */
  {
    if (cart.romsize <= 0x380000)  /* just to be sure (checksum might not be enough) */
    {
      cart.special |= HW_J_CART;

      /* set default port 1 setting */
      if (input.system[1] != SYSTEM_WAYPLAY)
      {
        old_system[1] = input.system[1];
        input.system[1] = SYSTEM_MD_GAMEPAD;
      }

      /* extra connectors mapped at $38xxxx or $3Fxxxx */
      m68k_memory_map[0x38].read16  = jcart_read;
      m68k_memory_map[0x38].write16 = jcart_write;
      m68k_memory_map[0x3f].read16  = jcart_read;
      m68k_memory_map[0x3f].write16 = jcart_write;
    }
  }

  /**********************************************
          LOCK-ON 
  ***********************************************/
  
  /* clear existing patches */
  ggenie_shutdown();
  areplay_shutdown();

  /* initialize extra hardware */
  switch (config.lock_on)
  {
    case TYPE_GG:
    {
      ggenie_init();
      break;
    }

    case TYPE_AR:
    {
      areplay_init();
      break;
    }

    case TYPE_SK:
    {
      /* store S&K ROM above cartridge ROM + SRAM */
      if (cart.romsize > 0x600000) break;

       /* load Sonic & Knuckles ROM (2 MBytes) */
      FILE *f = fopen(SK_ROM,"r+b");
      if (!f) break;
      int done = 0;
      while (done < 0x200000)
      {
        fread(cart.rom + 0x600000 + done, 2048, 1, f);
        done += 2048;
      }
      fclose(f);

      /* load Sonic 2 UPMEM ROM (256 KBytes) */
      f = fopen(SK_UPMEM,"r+b");
      if (!f) break;
      done = 0;
      while (done < 0x40000)
      {
        fread(cart.rom + 0x800000 + done, 2048, 1, f);
        done += 2048;
      }
      fclose(f);
          
#ifdef LSB_FIRST
      /* Byteswap ROM */
      int i;
      uint8 temp;
      for(i = 0; i < 0x240000; i += 2)
      {
        temp = cart.rom[i + 0x600000];
        cart.rom[i + 0x600000] = cart.rom[i + 0x600000 + 1];
        cart.rom[i + 0x600000 + 1] = temp;
      }
#endif

      /*$000000-$1FFFFF is mapped to S&K ROM */
      for (i=0x00; i<0x20; i++)
      {
        m68k_memory_map[i].base = (cart.rom + 0x600000) + (i<<16);
      }

      cart.special |= HW_LOCK_ON;
      break;
    }

    default:
    {
      break;
    }
  }

  /**********************************************
        Cartridge Extra Hardware
  ***********************************************/
  memset(&cart.hw, 0, sizeof(T_CART_HW));

  /* search for game into database */
  for (i=0; i < CART_CNT + 1; i++)
  {
    /* known cart found ! */
    if ((rominfo.checksum == rom_database[i].chk_1) &&
        (rominfo.realchecksum == rom_database[i].chk_2))
    {
      /* retrieve hardware information */
      memcpy(&cart.hw, &(rom_database[i].cart_hw), sizeof(T_CART_HW));

      /* initialize memory handlers for $400000-$7FFFFF region */
      int j = rom_database[i].bank_start;
      while (j <= rom_database[i].bank_end)
      {
        if (cart.hw.regs_r)
        {
          m68k_memory_map[j].read8    = cart.hw.regs_r;
          m68k_memory_map[j].read16   = cart.hw.regs_r;
          zbank_memory_map[j].read    = cart.hw.regs_r;
        }
        if (cart.hw.regs_w)
        {
          m68k_memory_map[j].write8   = cart.hw.regs_w;
          m68k_memory_map[j].write16  = cart.hw.regs_w;
          zbank_memory_map[j].write   = cart.hw.regs_w;
        }
        j++;
      }

      /* leave loop */
      i = CART_CNT + 1;
    }
  }

  /* Realtec mapper */
  if (cart.hw.realtec)
  {
    /* 8k BOOT ROM */
    for (i=0; i<8; i++)
    {
      memcpy(cart.rom + 0x900000 + i*0x2000, cart.rom + 0x7e000, 0x2000);
    }

    /* BOOT ROM is mapped to $000000-$3FFFFF */
    for (i=0x00; i<0x40; i++)
    {
      m68k_memory_map[i].base = cart.rom + 0x900000;
    }
  }

#if M68K_EMULATE_ADDRESS_ERROR
  /* default behavior */
  emulate_address_error = config.addr_error; 
#endif

  /* detect special cartridges */
  if (cart.romsize > 0x800000)
  {
    /* Ultimate MK3 (hack) */
    for (i=0x40; i<0xA0; i++)
    {
      m68k_memory_map[i].base     = cart.rom + (i<<16);
      m68k_memory_map[i].read8    = NULL;
      m68k_memory_map[i].read16   = NULL;
      zbank_memory_map[i].read    = NULL;
    }

#if M68K_EMULATE_ADDRESS_ERROR
    /* this game does not work properly on real hardware */
    emulate_address_error = 0;  
#endif
  }
  else if (cart.romsize > 0x400000)
  {
    /* assume SSF2 mapper */
    cart.hw.bankshift = 1;
    cart.hw.time_w = mapper_ssf2_w;
  }

  /* default write handler for !TIME range ($A130xx)*/
  if (!cart.hw.time_w)
  {
    cart.hw.time_w = default_time_w;
  }
}

/* hardware that need to be reseted on power on */
void md_cart_reset(int hard_reset)
{
  int i;

  /* reset cartridge mapping */
  if (cart.hw.bankshift)
  {
    for (i=0x00; i<0x40; i++)
    {
      m68k_memory_map[i].base = cart.rom + ((i<<16) & cart.mask);
    }
  }
  
  /* SVP chip */
  if (svp) svp_reset();

  /* Lock-ON */
  switch (config.lock_on)
  {
    case TYPE_GG:
    {
      ggenie_reset(hard_reset);
      break;
    }

    case TYPE_AR:
    {
      areplay_reset(hard_reset);
      break;
    }

    case TYPE_SK:
    {
      if (cart.special & HW_LOCK_ON)
      {
        /* disable UPMEM chip at $300000-$3fffff */
        for (i=0x30; i<0x40; i++)
        {
          m68k_memory_map[i].base = cart.rom + ((i<<16) & cart.mask);
        }
      }
      break;
    }

    default:
    {
      break;
    }
  }

  /* save default cartridge slot mapping */
  cart.base = m68k_memory_map[0].base;
}

int md_cart_context_save(uint8 *state)
{
  int i;
  int bufferptr = 0;
  uint8 *base;

  /* cartridge mapping */
  for (i=0; i<0x40; i++)
  {
    /* get base address */
    base = m68k_memory_map[i].base;
      
    if (base == sram.sram)
    {
      /* SRAM */
      state[bufferptr++] = 0xff;
    }
    else
    {
      /* ROM */
      state[bufferptr++] = ((base - cart.rom) >> 16) & 0xff;
    }
  }

  /* hardware registers */
  save_param(cart.hw.regs, sizeof(cart.hw.regs));

  /* SVP */
  if (svp)
  {
    save_param(svp->iram_rom, 0x800);
    save_param(svp->dram,sizeof(svp->dram));
    save_param(&svp->ssp1601,sizeof(ssp1601_t));
  }

  return bufferptr;
}

int md_cart_context_load(uint8 *state, char *version)
{
  int i;
  int bufferptr = 0;
  uint8 offset;

  /* extended state (from 1.4.1 and above) */
  if ((version[11] > 0x31) || (version[13] > 0x34) || (version[15] > 0x30))
  {
    /* cartridge mapping */
    for (i=0; i<0x40; i++)
    {
      /* get offset */
      offset = state[bufferptr++];

      if (offset == 0xff)
      {
        /* SRAM */
        m68k_memory_map[i].base = sram.sram;
      }
      else
      {
        /* ROM */
        m68k_memory_map[i].base = cart.rom + (offset << 16);
      }
    }

    /* hardware registers */
    load_param(cart.hw.regs, sizeof(cart.hw.regs));

    /* SVP */
    if (svp)
    {
      load_param(svp->iram_rom, 0x800);
      load_param(svp->dram,sizeof(svp->dram));
      load_param(&svp->ssp1601,sizeof(ssp1601_t));
    }
  }

  return bufferptr;
}

/************************************************************
          MAPPER handlers 
*************************************************************/

/* 
  ROM/SRAM Bankswitch (Phantasy Star IV, Story of Thor/Beyond Oasis, Sonic 3 & Knuckles)
*/
static void mapper_sega_w(uint32 data)
{
  int i;

  if (data & 1)
  {
    /* Only if SRAM is detected */
    if (sram.on)
    {
      /* $200000-$3fffff is mapped to SRAM */
      for (i=0x20; i<0x40; i++)
      {
        m68k_memory_map[i].base  = sram.sram;
      }

      if (data & 2)
      {
        /* SRAM write disabled */
        for (i=0x20; i<0x40; i++)
        {
          m68k_memory_map[i].write8  = m68k_unused_8_w;
          m68k_memory_map[i].write16 = m68k_unused_16_w;
          zbank_memory_map[i].write  = zbank_unused_w;
        }
      }
      else
      {
        /* SRAM write enabled */
        for (i=0x20; i<0x40; i++)
        {
          m68k_memory_map[i].write8  = NULL;
          m68k_memory_map[i].write16 = NULL;
          zbank_memory_map[i].write  = NULL;
        }
      }
    }

    /* S&K lock-on chip */
    if ((cart.special & HW_LOCK_ON) && (config.lock_on == TYPE_SK))
    {
      /* $300000-$3fffff is mapped to S2K upmem chip */
      for (i=0x30; i<0x40; i++)
      {
        m68k_memory_map[i].base = (cart.rom + 0x800000) + ((i & 3)<<16);
      }
    }
  }
  else
  {
    /* $200000-$3fffff is mapped to ROM */
    for (i=0x20; i<0x40; i++)
    {
      m68k_memory_map[i].base = cart.rom + ((i<<16) & cart.mask);
      m68k_memory_map[i].write8 = m68k_unused_8_w;
      m68k_memory_map[i].write16 = m68k_unused_16_w;
      zbank_memory_map[i].write = zbank_unused_w;
    }
  }
}

/*
   Super Street Fighter 2 ROM Bankswitch
   documented by Bart Trzynadlowski (http://www.trzy.org/files/ssf2.txt) 
*/
static void mapper_ssf2_w(uint32 address, uint32 data)
{
  /* 8 x 512k banks */
  uint32 dst = (address << 2) & 0x38;
  
  /* bank 0 remains unchanged */
  if (dst)
  {
    uint32 i;
    uint8 *src = cart.rom + (data << 19);

    for (i=0; i<8; i++)
    {
      m68k_memory_map[dst++].base = src + (i<<16);
    }
  }
}

/* 
  Realtec ROM Bankswitch (Earth Defend, Balloon Boy & Funny World, Whac-A-Critter)
  (Note: register usage is inverted in TascoDlx documentation)
*/
static void mapper_realtec_w(uint32 address, uint32 data)
{
  switch (address)
  {
    case 0x402000:  
    {
      /* number of mapped 64k blocks (the written value is a number of 128k blocks) */
      cart.hw.regs[2] = data << 1;
      return;
    }

    case 0x404000:
    {
      /* 00000xxx */
      cart.hw.regs[0] = data & 7;
      return;
    }

    case 0x400000:  
    {
      /* 00000yy1 */
      cart.hw.regs[1] = data & 6;

      /* mapped start address is 00yy xxx0 0000 0000 0000 0000 */
      uint32 base = (cart.hw.regs[0] << 1) | (cart.hw.regs[1] << 3);

      /* ensure mapped size is not null */
      if (cart.hw.regs[2])
      {
        /* selected blocks are mirrored into the whole cartridge area */
        int i;
        for (i=0x00; i<0x40; i++)
          m68k_memory_map[i].base = &cart.rom[(base + (i % cart.hw.regs[2])) << 16];
      }
      return;
    }
  }
}

/* Game no Kanzume Otokuyou ROM Mapper */
static void mapper_seganet_w(uint32 address, uint32 data)
{
  if ((address & 0xff) == 0xf1)
  {
    int i;
    if (data & 1)
    {
      /* ROM Write protected */
      for (i=0; i<0x40; i++)
      {
        m68k_memory_map[i].write8   = m68k_unused_8_w;
        m68k_memory_map[i].write16  = m68k_unused_16_w;
        zbank_memory_map[i].write   = zbank_unused_w;
      }
    }
    else
    {
      /* ROM Write enabled */
      for (i=0; i<0x40; i++)
      {
        m68k_memory_map[i].write8   = NULL;
        m68k_memory_map[i].write16  = NULL;
        zbank_memory_map[i].write   = NULL;
      }
    }
  }
}

/* 
  Custom ROM Bankswitch used in Top Fighter, Mulan, Pocket Monsters II, Lion King 3, Super King Kong 99, Pokemon Stadium
*/
static void mapper_32k_w(uint32 data)
{
  int i;
  
  /* 64 x 32k banks */
  if (data)
  {
    /* unverified (Top Fighter writes $2A instead $2E) */
    if (data >> 2) data |= 4;

    /* bank is mapped at $000000-$0FFFFF */
    for (i=0; i<16; i++)
    {
      memcpy(cart.rom + 0x900000 + (i<<16), cart.rom + ((data & 0x3f) << 15), 0x8000);
      memcpy(cart.rom + 0x908000 + (i<<16), cart.rom + ((data & 0x3f) << 15), 0x8000);
      m68k_memory_map[i].base = cart.rom + 0x900000 + (i<<16);
    }
  }
  else
  {
    /* reset default $000000-$0FFFFF mapping */
    for (i=0; i<16; i++)
    {
      m68k_memory_map[i].base = &cart.rom[i << 16];
    }
  }
}

/* 
  Custom ROM Bankswitch used in Chinese Fighter III
*/
static void mapper_64k_w(uint32 data)
{
  int i;

  /* 16 x 64k banks */
  if (data)
  {
    /* bank is mapped at $000000-$0FFFFF */
    for (i=0; i<16; i++)
    {
      m68k_memory_map[i].base = &cart.rom[(data & 0xf) << 16];
    }
  }
  else
  {
    /* reset default $000000-$0FFFFF mapping */
    for (i=0; i<16; i++)
    {
      m68k_memory_map[i].base = &cart.rom[(i & 0xf) << 16];
    }
  }
}

/* 
  Custom ROM Bankswitch used in pirate "Multi-in-1" cartridges, A Bug's Life, King of Fighter 99, Pocket Monster, Rockman X3
 */
static void mapper_64k_multi_w(uint32 address)
{
  int i;

  /* 64 x 64k banks */
  for (i=0; i<64; i++)
  {
    m68k_memory_map[i].base = &cart.rom[((address++) & 0x3f) << 16];
  }
}

/*
  Custom ROM Bankswitch used in RADICA cartridges
*/
static uint32 mapper_radica_r(uint32 address)
{
  int i = 0;
  address = (address >> 1);
  
  /* 64 x 64k banks */
  for (i = 0; i < 64; i++)
  {
    m68k_memory_map[i].base = &cart.rom[((address++)& 0x3f)<< 16];
  }

  return 0xffff;
}


/************************************************************
          default !TIME signal handler 
*************************************************************/

static void default_time_w(uint32 address, uint32 data)
{
  if (address < 0xa13040)
  {
    /* unlicensed cartridges mapper (default) */
    mapper_64k_multi_w(address);
    return;
  }

  /* official cartridges mapper (default) */
  mapper_sega_w(data);
}


/************************************************************
          Internal register handlers
*************************************************************/

static uint32 default_regs_r(uint32 address)
{
  int i;
  for (i=0; i<4; i++)
  {
    if ((address & cart.hw.mask[i]) == cart.hw.addr[i])
    {
      return cart.hw.regs[i];
    }
  }
  return m68k_read_bus_8(address);
}

static uint32 default_regs_r_16(uint32 address)
{
  int i;
  for (i=0; i<4; i++)
  {
    if ((address & cart.hw.mask[i]) == cart.hw.addr[i])
    {
      return (cart.hw.regs[i] << 8);
    }
  }
  return m68k_read_bus_16(address);
}

static void default_regs_w(uint32 address, uint32 data)
{
  int i;
  for (i=0; i<4; i++)
  {
    if ((address & cart.hw.mask[i]) == cart.hw.addr[i])
    {
      cart.hw.regs[i] = data;
      return;
    }
  }
  m68k_unused_8_w(address, data);
}

/* custom register hardware (Top Fighter, Lion King III, Super Donkey Kong  99, Mulan, Pocket Monsters II, Pokemon Stadium) */
static void custom_regs_w(uint32 address, uint32 data)
{
  /* ROM bankswitch */
  if ((address >> 16) > 0x6f)
  {
    mapper_32k_w(data);
    return;
  }

  /* write register */
  default_regs_w(address, data);

  /* bitswapping */
  uint32 temp = cart.hw.regs[0];
  switch (cart.hw.regs[1] & 3)
  {
    case 0:
      cart.hw.regs[2] = (temp << 1);
      break;

    case 1:
      cart.hw.regs[2] = (temp >> 1);
      return;

    case 2:
      cart.hw.regs[2] = ((temp >> 4) | ((temp & 0x0F) << 4));
      return;

    default:
      cart.hw.regs[2] = (((temp >> 7) & 0x01) | ((temp >> 5) & 0x02) |
                         ((temp >> 3) & 0x04) | ((temp >> 1) & 0x08) |
                         ((temp << 1) & 0x10) | ((temp << 3) & 0x20) |
                         ((temp << 5) & 0x40) | ((temp << 7) & 0x80));
      return;
  }
}

/* alternate custom register hardware (Chinese Fighters III) */
static void custom_alt_regs_w(uint32 address, uint32 data)
{
  /* ROM bankswitch */
  if ((address >> 16) > 0x5f)
  {
    mapper_64k_w(data);
    return;
  }

  /* write regs */
  default_regs_w(address, data);
}

static uint32 topshooter_read(uint32 address)
{
  if (address < 0x202000)
  {
    uint8 temp = 0xff;

    switch (address & 0xff)
    {
      case 0x43:
      {
        if (input.pad[0] & INPUT_A)     temp &= ~0x80; /* Shoot */
        if (input.pad[0] & INPUT_B)     temp &= ~0x10; /* Bet */
        if (input.pad[0] & INPUT_START) temp &= ~0x20; /* Start */
        break;
      }

      case 0x45:  /* ??? (DOWN) & Service Mode (UP) */
      {
        if (input.pad[0] & INPUT_UP)    temp &= ~0x08; /* Service Mode */
        if (input.pad[0] & INPUT_DOWN)  temp &= ~0x10; /* ???, used in service menu to select next option */
        break;
      }

      case 0x47:
      {
        if (input.pad[0] & INPUT_RIGHT) temp &= ~0x03; /* Insert 10 coins */
        break;
      }

      case 0x49:
      {
        if (input.pad[0] & INPUT_LEFT)  temp &= ~0x03; /* Clear coins */
        if (input.pad[0] & INPUT_C)     temp &= ~0x01; /* Insert XXX coins */
        break;
      }

      case 0x51:
      {
        temp = 0xA5;
        break;
      }

      default:
      {
        temp = m68k_read_bus_8(address);
        break;
      }
    }
    return temp;
  }

  return READ_BYTE(sram.sram , address & 0xffff);
}

static void topshooter_write(uint32 address, uint32 data)
{
  if (address >= 0x202000)
  {
    WRITE_BYTE(sram.sram , address & 0xffff, data);
    return;
  }
  m68k_unused_8_w(address, data);
}
