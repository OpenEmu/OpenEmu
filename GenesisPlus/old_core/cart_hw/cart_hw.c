
/****************************************************************************
 *  Genesis Plus
 *  Cartridge Hardware support
 *
 *  Copyright (C) 2007, 2008, 2009  Eke-Eke (GCN/Wii port)
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

#define CART_CNT 26

extern int emulate_address_error;

/* Global Variables */
T_CART_HW cart_hw;
uint8 j_cart;
uint8 *default_rom;
int old_system[2] = {-1,-1};

/* Function prototypes */
void default_time_w(uint32 address, uint32 data);
void special_mapper_w(uint32 address, uint32 data);
void realtec_mapper_w(uint32 address, uint32 data);
void seganet_mapper_w(uint32 address, uint32 data);
uint32 radica_mapper_r(uint32 address);
void default_regs_w(uint32 address, uint32 data);
uint32 default_regs_r(uint32 address);
void special_regs_w(uint32 address, uint32 data);

/* Cart database entry */
typedef struct
{
  uint16 chk_1;       /* header checksum */
  uint16 chk_2;       /* real checksum */
  uint8 bank_start;   /* first mapped bank in $400000-$7fffff region */
  uint8 bank_end;     /* last mapped bank in $400000-$7fffff region */
  T_CART_HW cart_hw;  /* hardware description */
} T_CART_ENTRY;

/* Games that need extra hardware emulation:
  - copy protection device
  - custom ROM banking device
*/
static const T_CART_ENTRY rom_database[CART_CNT] =
{
/* Game no Kanzume Otokuyou */
  {0x0000,0xf9d1,0,0,{{0,0,0,0},{0,0,0,0},{0,0,0,0},0,0,0,seganet_mapper_w,0,0}},
/* RADICA (Volume 1) (not byteswapped) */
  {0x0000,0x2326,0,0,{{0,0,0,0},{0,0,0,0},{0,0,0,0},0,1,radica_mapper_r,0,0,0}},
/* RADICA (Volume 2) */
  {0x4f10,0x0836,0,0,{{0,0,0,0},{0,0,0,0},{0,0,0,0},0,1,radica_mapper_r,0,0,0}},
/* RADICA (Volume 1) */
  {0xf424,0x9f82,0,0,{{0,0,0,0},{0,0,0,0},{0,0,0,0},0,1,radica_mapper_r,0,0,0}},
/* Funny World & Balloon Boy */
  {0x0000,0x06ab,0x40,0x40,{{0,0,0,0},{0,0,0,0},{0,0,0,0},1,1,0,0,0,realtec_mapper_w}},
/* Whac-a-Critter */
  {0xffff,0xf863,0x40,0x40,{{0,0,0,0},{0,0,0,0},{0,0,0,0},1,1,0,0,0,realtec_mapper_w}},
/* Earth Defense */
  {0xffff,0x44fb,0x40,0x40,{{0,0,0,0},{0,0,0,0},{0,0,0,0},1,1,0,0,0,realtec_mapper_w}},
/* Super Mario 2 1998 */
  {0xffff,0x0474,0,0,{{0x0a,0,0,0},{0xffffff,0,0,0},{0xa13000,0,0,0},0,0,default_regs_r,0,0,0}},
/* Super Mario 2 1998 */
  {0x2020,0xb4eb,0,0,{{0x1c,0,0,0},{0xffffff,0,0,0},{0xa13000,0,0,0},0,0,default_regs_r,0,0,0}},
/* Supper Bubble Bobble */
  {0x0000,0x16cd,0x40,0x40,{{0x55,0x0f,0,0},{0xffffff,0xffffff,0,0},{0x400000,0x400002,0,0},0,0,0,0,default_regs_r,0}},
/* Mahjong Lover */
  {0x0000,0x7037,0x40,0x40,{{0x90,0xd3,0,0},{0xffffff,0xffffff,0,0},{0x400000,0x401000,0,0},0,0,0,0,default_regs_r,0}},
/* Lion King 2 */
  {0xffff,0x1d9b,0x40,0x40,{{0,0,0,0},{0xfffffd,0xfffffd,0,0},{0x400000,0x400004,0,0},0,0,0,0,default_regs_r,default_regs_w}},
/* Squirell King */
  {0x0000,0x8ec8,0x40,0x40,{{0,0,0,0},{0xfffffd,0xfffffd,0,0},{0x400000,0x400004,0,0},0,0,0,0,default_regs_r,default_regs_w}},
/* Rockman X3 */
  {0x0000,0x9d0e,0x40,0x40,{{0x0c,0x88,0,0},{0xffffff,0xffffff,0,0},{0xa13000,0x400004,0,0},0,0,default_regs_r,0,default_regs_r,0}},
/* A Bug's Life */
  {0x7f7f,0x2aad,0,0,{{0x28,0x1f,0x01,0},{0xffffff,0xffffff,0xffffff,0},{0xa13000,0xa13002,0xa1303e,0},0,0,default_regs_r,0,0,0}},
/* King of Fighter 99 */
  {0x0000,0x21e,0,0,{{0x00,0x01,0x1f,0},{0xffffff,0xffffff,0xffffff,0},{0xa13000,0xa13002,0xa1303e,0},0,0,default_regs_r,0,0,0}},
/* Pocket Monster */
  {0xd6fc,0x1eb1,0,0,{{0x00,0x01,0x1f,0},{0xffffff,0xffffff,0xffffff,0},{0xa13000,0xa13002,0xa1303e,0},0,0,default_regs_r,0,0,0}},
/* Lion King 3 */
  {0x0000,0x507c,0x60,0x7f,{{0,0,0,0},{0xf0000e,0xf0000e,0xf0000e,0},{0x600000,0x600002,0x600004,0},0,1,0,0,default_regs_r,special_regs_w}},
/* Super King Kong 99 */
  {0x0000,0x7d6e,0x60,0x7f,{{0,0,0,0},{0xf0000e,0xf0000e,0xf0000e,0},{0x600000,0x600002,0x600004,0},0,1,0,0,default_regs_r,special_regs_w}},
/* Pokemon Stadium */
  {0x0000,0x843c,0x70,0x7f,{{0,0,0,0},{0,0,0,0},{0,0,0,0},0,1,0,0,0,special_regs_w}},
/* Elf Wor */
  {0x0080,0x3dba,0x40,0x40,{{0x55,0x0f,0xc9,0x18},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,0,0,default_regs_r,0}},
/* Huan Le Tao Qi Shu - Smart Mouse */
  {0x0000,0x1a28,0x40,0x40,{{0x55,0x0f,0xaa,0xf0},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,0,0,default_regs_r,0}},
/* Ya-Se Chuanshuo */
  {0xffff,0xd472,0x40,0x40,{{0x63,0x98,0xc9,0x18},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,0,0,default_regs_r,0}},
/* Soul Blade */
  {0x0000,0x0c5b,0x40,0x40,{{0x00,0x98,0xc9,0xF0},{0xffffff,0xffffff,0xffffff,0xffffff},{0x400000,0x400002,0x400004,0x400006},0,0,0,0,default_regs_r,0}},
/* King of Fighter 98 */
  {0x0000,0xd0a0,0x48,0x4f,{{0xaa,0xa0,0xf0,0xa0},{0xfc0000,0xffffff,0xffffff,0xffffff},{0x480000,0x4c82c0,0x4cdda0,0x4f8820},0,0,0,0,default_regs_r,0}},
/* Lian Huan Pao - Barver Battle Saga */
  {0x30b9,0x1c2a,0x40,0x40,{{0,0,0,0},{0,0,0,0},{0,0,0,0},0,0,0,0,default_regs_r,0}}
};

/* temporary memory chunk */
static uint8 mem_chunk[0x10000];


/************************************************************
          Cart Hardware initialization 
*************************************************************/

/* hardware that need to be reseted on power on */
void cart_hw_reset()
{
  int i;

  /* reset bankshifting */
  if (cart_hw.bankshift)
  {
    for (i=0x00; i<0x40; i++)
      m68k_memory_map[i].base = cart_rom + (i<<16);
  }

  /* Realtec mapper */
  if (cart_hw.realtec & 1)
  {
    /* enable BOOTROM */
    for (i=0; i<0x40; i++) m68k_memory_map[i].base = mem_chunk;
    for (i=0; i<8; i++) memcpy(mem_chunk + i*0x2000, cart_rom + 0x7e000, 0x2000);
    cart_hw.realtec |= 2;
  }

  /* save default cartridge slot mapping */
  default_rom = m68k_memory_map[0].base;

  /* SVP chip */
  if (svp) svp_reset();
}

/* cart hardware detection */
void cart_hw_init()
{
  int i;

  /**********************************************
          DEFAULT CARTRIDGE MAPPING 
  ***********************************************/
  for (i=0; i<0x40; i++)
  {
    /* cartridge ROM */
    m68k_memory_map[i].base     = cart_rom + (i<<16);
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
    m68k_memory_map[i].base     = cart_rom + (i<<16);
    m68k_memory_map[i].read8    = m68k_read_bus_8;
    m68k_memory_map[i].read16   = m68k_read_bus_16;
    m68k_memory_map[i].write8   = m68k_unused_8_w;
    m68k_memory_map[i].write16  = m68k_unused_16_w;
    zbank_memory_map[i].read    = zbank_unused_r;
    zbank_memory_map[i].write   = zbank_unused_w;
  }

  /* restore previous setting */
  if (old_system[0] != -1)  input.system[0] = old_system[0];
  if (old_system[1] != -1)  input.system[1] = old_system[1];

  /**********************************************
          EXTERNAL RAM 
  ***********************************************/
  sram_init();
  eeprom_init();
  if (sram.on)
  {
    if (sram.custom)
    {
      /* serial EEPROM */
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

    m68k_memory_map[0x30].base = svp->dram;
    m68k_memory_map[0x30].read16  = NULL;
    m68k_memory_map[0x30].write16 = svp_write_dram;

    m68k_memory_map[0x31].base = svp->dram + 0x10000;
    m68k_memory_map[0x31].read16  = NULL;
    m68k_memory_map[0x31].write16 = svp_write_dram;

    m68k_memory_map[0x39].read16  = svp_read_cell_1;
    m68k_memory_map[0x3a].read16  = svp_read_cell_2;
  }


  /* default GUN settings */
  input.x_offset = 0x00;
  input.y_offset = 0x00;

  /**********************************************
          SEGA MENACER 
  ***********************************************/
  if (strstr(rominfo.international,"MENACER") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1) old_system[0] = input.system[0];
    if (old_system[1] == -1) old_system[1] = input.system[1];

    input.system[0] = NO_SYSTEM;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 0x52;
    input.y_offset = 0x00;
  }
  else if (strstr(rominfo.international,"T2 ; THE ARCADE GAME") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1) old_system[0] = input.system[0];
    if (old_system[1] == -1) old_system[1] = input.system[1];

    input.system[0] = SYSTEM_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 0x84;
    input.y_offset = 0x08;
  }
  else if (strstr(rominfo.international,"BODY COUNT") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1) old_system[0] = input.system[0];
    if (old_system[1] == -1) old_system[1] = input.system[1];

    input.system[0] = SYSTEM_MOUSE;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 0x44;
    input.y_offset = 0x18;
  }

  /**********************************************
          KONAMI JUSTIFIER 
  ***********************************************/
  else if (strstr(rominfo.international,"LETHAL ENFORCERSII") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1) old_system[0] = input.system[0];
    if (old_system[1] == -1) old_system[1] = input.system[1];

    input.system[0] = SYSTEM_GAMEPAD;
    input.system[1] = SYSTEM_JUSTIFIER;
    input.x_offset = 0x18;
    input.y_offset = 0x00;
  }
  else if (strstr(rominfo.international,"LETHAL ENFORCERS") != NULL)
  {
    /* save current setting */
    if (old_system[0] == -1) old_system[0] = input.system[0];
    if (old_system[1] == -1) old_system[1] = input.system[1];

    input.system[0] = SYSTEM_GAMEPAD;
    input.system[1] = SYSTEM_JUSTIFIER;
    input.x_offset = 0x00;
    input.y_offset = 0x00;
  }

  /**********************************************
          J-CART 
  ***********************************************/
  j_cart = 0;
  if (((strstr(rominfo.product,"00000000") != NULL) && (rominfo.checksum == 0x168b))  ||  /* Super Skidmarks, Micro Machines Military*/
    ((strstr(rominfo.product,"00000000") != NULL) && (rominfo.checksum == 0x165e))    ||  /* Pete Sampras Tennis (1991), Micro Machines 96 */
    ((strstr(rominfo.product,"00000000") != NULL) && (rominfo.checksum == 0xcee0))    ||  /* Micro Machines Military (bad) */
    ((strstr(rominfo.product,"00000000") != NULL) && (rominfo.checksum == 0x2c41))    ||  /* Micro Machines 96 (bad) */
    ((strstr(rominfo.product,"XXXXXXXX") != NULL) && (rominfo.checksum == 0xdf39))    ||  /* Sampras Tennis 96 */
    ((strstr(rominfo.product,"T-123456") != NULL) && (rominfo.checksum == 0x1eae))    ||  /* Sampras Tennis 96 */
    ((strstr(rominfo.product,"T-120066") != NULL) && (rominfo.checksum == 0x16a4))    ||  /* Pete Sampras Tennis (1994)*/
     (strstr(rominfo.product,"T-120096") != NULL))                                        /*  Micro Machines 2 */
  {
    if (genromsize <= 0x380000)  /* just to be sure (checksum might not be enough) */
    {
      j_cart = 1;
      m68k_memory_map[0x38].read16  = jcart_read;
      m68k_memory_map[0x38].write16 = jcart_write;
      m68k_memory_map[0x3f].read16  = jcart_read;
      m68k_memory_map[0x3f].write16 = jcart_write;

      /* save current setting */
      if (old_system[0] == -1) old_system[0] = input.system[0];
      if (old_system[1] == -1) old_system[1] = input.system[1];
       
      /* PORT B by default */
      input.system[0] = SYSTEM_GAMEPAD;
      input.system[1] = SYSTEM_GAMEPAD;
    }
  }

  /**********************************************
        Mappers & HW registers 
  ***********************************************/
  memset(&cart_hw, 0, sizeof(cart_hw));

  /* search for game into database */
  for (i=0; i < CART_CNT + 1; i++)
  {
    /* known cart found ! */
    if ((rominfo.checksum == rom_database[i].chk_1) &&
        (realchecksum == rom_database[i].chk_2))
    {
      /* retrieve hardware information */
      memcpy(&cart_hw, &(rom_database[i].cart_hw), sizeof(cart_hw));

      /* initialize memory handlers for $400000-$7fffff region */
      int j = rom_database[i].bank_start;
      while (j <= rom_database[i].bank_end)
      {
        if (cart_hw.regs_r)
        {
          m68k_memory_map[j].read8    = cart_hw.regs_r;
          m68k_memory_map[j].read16   = cart_hw.regs_r;
          zbank_memory_map[j].read    = cart_hw.regs_r;
        }
        if (cart_hw.regs_w)
        {
          m68k_memory_map[j].write8   = cart_hw.regs_w;
          m68k_memory_map[j].write16  = cart_hw.regs_w;
          zbank_memory_map[j].write   = cart_hw.regs_w;
        }
        j++;
      }

      /* leave loop */
      i = CART_CNT + 1;
    }
  }

#if M68K_EMULATE_ADDRESS_ERROR
  /* default behavior */
  emulate_address_error = config.addr_error; 
#endif

  /* detect ROM files larger than 4MB */
  if (genromsize > 0x800000)
  {
    /* Ultimate MK3 (hack) */
    for (i=0x40; i<0xA0; i++)
    {
      m68k_memory_map[i].base     = cart_rom + (i<<16);
      m68k_memory_map[i].read8    = NULL;
      m68k_memory_map[i].read16   = NULL;
      zbank_memory_map[i].read    = NULL;
    }

#if M68K_EMULATE_ADDRESS_ERROR
    /* this game does not work properly on real hardware */
    emulate_address_error = 0;  
#endif
  }
  else if (genromsize > 0x400000)
  {
    /* assume SSF2 mapper */
    cart_hw.bankshift = 1;
  }

  /* default write handler for !TIME signal */
  if (!cart_hw.time_w) cart_hw.time_w = default_time_w;
}

/************************************************************
          MAPPER handlers 
*************************************************************/

/* 
  "official" ROM/RAM switch
*/
static inline void sega_mapper_w(uint32 address, uint32 data)
{
  uint32 i,slot = (address >> 1) & 7;
  uint8 *src;

  switch (slot)
  {
    case 0:
      /* ROM/SRAM switch (Phantasy Star IV, Story of Thor/Beyond Oasis, Sonic 3 & Knuckles) */
      if (data & 1)
      {
        /* SRAM enabled */
        m68k_memory_map[0x20].base  = sram.sram;

        if (data & 2)
        {
          /* SRAM write disabled */
          m68k_memory_map[0x20].write8  = m68k_unused_8_w;
          m68k_memory_map[0x20].write16 = m68k_unused_16_w;
          zbank_memory_map[0x20].write  = zbank_unused_w;
        }
        else
        {
          /* SRAM write enabled */
          m68k_memory_map[0x20].write8  = NULL;
          m68k_memory_map[0x20].write16 = NULL;
          zbank_memory_map[0x20].write  = NULL;
        }
      }
      else
      {
        /* ROM enabled */
        m68k_memory_map[0x20].base  = cart_rom + 0x200000;
      }
      break;
    
    default:
      /* ROM Bankswitch (Super Street Fighter 2)
         documented by Bart Trzynadlowski (http://www.trzy.org/files/ssf2.txt) 
      */
      slot = slot << 3; /* 8 x 512k banks */
      src = cart_rom + (data << 19);
      for (i=0; i<8; i++) m68k_memory_map[slot++].base = src + (i<<16);
      break;
  }
}

/* 
  custom ROM Bankswitch used by pirate "Multi-in-1" carts
*/
static inline void multi_mapper_w(uint32 address, uint32 data)
{
  int i;

  cart_hw.bankshift = 1;

  /* 64 x 64k banks */
  for (i=0; i<64; i++)
  {
    m68k_memory_map[i].base = &cart_rom[((address++) & 0x3f) << 16];
  }
}

/* 
  Special ROM Bankswitch used for copy protection
  Used by unlicensed cartridges (Lion King III, Super King Kong 99)
*/
void special_mapper_w(uint32 address, uint32 data)
{
  /* 1 x 32k bank */
  m68k_memory_map[0].base = mem_chunk;
  memcpy(mem_chunk,&cart_rom[(data & 0x7f)  << 15],0x8000);
  memcpy(mem_chunk+0x8000,cart_rom + 0x8000,0x8000);
}

/* 
  Realtec ROM Bankswitch (Earth Defend, Balloon Boy & Funny World, Whac-A-Critter)
*/
void realtec_mapper_w(uint32 address, uint32 data)
{
  int i;
  uint32 base;

  /* 32 x 128k banks */
  switch (address)
  {
    case 0x404000:  /* three lower bits of ROM base address */
      cart_hw.regs[0] = data & 7;
      base = ((data & 7) | (cart_hw.regs[1] << 2));
      for (i=0; i<=cart_hw.regs[2]; i++) 
      {
        m68k_memory_map[i*2].base   = &cart_rom[((base + i)*2 & 0x3f) << 16];
        m68k_memory_map[i*2+1].base = &cart_rom[(((base + i)*2 + 1) & 0x3f) << 16];
      }
      return;

    case 0x400000:  /* two higher bits of ROM base address */
      cart_hw.regs[1] = data & 6;
      base = cart_hw.regs[0] | ((data & 6) << 2);
      for (i=0; i<=cart_hw.regs[2]; i++) 
      {
        m68k_memory_map[i*2].base   = &cart_rom[((base + i)*2 & 0x3f) << 16];
        m68k_memory_map[i*2+1].base = &cart_rom[(((base + i)*2 + 1) & 0x3f) << 16];
      }
      return;

    case 0x402000:  /* number of 128k blocks to map */
      cart_hw.regs[2] = data & 0x1f;
      base = cart_hw.regs[0] | (cart_hw.regs[1] << 2);
      for (i=0; i<=(data & 0x1f); i++) 
      {
        m68k_memory_map[i*2].base   = &cart_rom[((base + i)*2 & 0x3f) << 16];
        m68k_memory_map[i*2+1].base = &cart_rom[(((base + i)*2 + 1) & 0x3f) << 16];
      }
      return;

  }
}

/* Game no Kanzume Otokuyou ROM Mapper */
void seganet_mapper_w(uint32 address, uint32 data)
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
  RADICA ROM Bankswitch (use !TIME)
*/
uint32 radica_mapper_r(uint32 address)
{
  int i = 0;
  address = (address >> 1);
  
  /* 64 x 64k banks */
  for (i = 0; i < 64; i++)
  {
    m68k_memory_map[i].base = &cart_rom[((address++)& 0x3f)<< 16];
  }
  return 0xff;
}


/************************************************************
          default !TIME signal handler 
*************************************************************/

/* default ROM bankswitch */
void default_time_w(uint32 address, uint32 data)
{
  if ((address & 0xf1) == 0xf1) sega_mapper_w(address, data);
  else if (address < 0xa13040) multi_mapper_w(address, data);
}


/************************************************************
          Internal register handlers
*************************************************************/

uint32 default_regs_r(uint32 address)
{
  int i;
  for (i=0; i<4; i++)
  {
    if ((address & cart_hw.mask[i]) == cart_hw.addr[i])
      return cart_hw.regs[i];
  }

  /* unused */
  return 0xffff;
}

void default_regs_w(uint32 address, uint32 data)
{
  int i;
  for (i=0; i<4; i++)
  {
    if ((address & cart_hw.mask[i]) == cart_hw.addr[i])
    {
      cart_hw.regs[i] = data;
    }
  }
}

/* special register behaviour (Lion King III, Super Donkey Kong  99) */
void special_regs_w(uint32 address, uint32 data)
{
  /* ROM bankswitch */
  if ((address >> 16) > 0x6f)
  {
    special_mapper_w(address, data);
    return;
  }

  /* write regs */
  default_regs_w(address, data);

  /* bitswapping (documented by Haze) */
  uint32 temp = cart_hw.regs[0];
  switch (cart_hw.regs[1])
  {
    case 1:
      cart_hw.regs[2] = (temp >> 1);
      return;

    case 2:
      cart_hw.regs[2] = ((temp >> 4) | ((temp & 0x0F) << 4));
      return;

    default:
      cart_hw.regs[2] = (((temp >> 7) & 0x01) | ((temp >> 5) & 0x02) |
                         ((temp >> 3) & 0x04) | ((temp >> 1) & 0x08) |
                         ((temp << 1) & 0x10) | ((temp << 3) & 0x20) |
                         ((temp << 5) & 0x40) | ((temp << 7) & 0x80));
      return;
  }
}
