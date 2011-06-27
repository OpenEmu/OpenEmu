/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../shared.h"
#include "cart.h"

#include "map_rom.h"
#include "map_sram.h"
#include "map_eeprom.h"

#include "map_realtec.h"
#include "map_ssf2.h"

#include "map_ff.h"
#include "map_rmx3.h"
#include "map_sbb.h"
#include "map_yase.h"

#include "../header.h"
#include "../../md5.h"
#include "../../general.h"
#include <ctype.h>

static MD_Cart_Type *cart_hardware = NULL;
static uint32 Cart_ROM_Size;

void MDCart_Write8(uint32 A, uint8 V)
{
 cart_hardware->Write8(A, V);
}

void MDCart_Write16(uint32 A, uint16 V)
{
 cart_hardware->Write16(A, V);
}

uint8 MDCart_Read8(uint32 A)
{
 return(cart_hardware->Read8(A));
}

uint16 MDCart_Read16(uint32 A)
{
 return(cart_hardware->Read16(A));
}

void MDCart_Reset(void)
{
 cart_hardware->Reset();
}

// MD_Cart_Type* (*MapperMake)(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size);
// MD_Make_Cart_Type_REALTEC
// MD_Make_Cart_Type_SSF2
// Final Fantasy
// MD_Make_Cart_Type_FF
// MD_Make_Cart_Type_RMX3
// MD_Make_Cart_Type_SBB
// MD_Make_Cart_Type_YaSe

typedef struct
{
 // Set any field to 0(or -1 for signed fields) to ignore it
 const char *id;

 const uint64 md5;
 const uint64 header_md5;

 const char *mapper;

 // Overrides for bad headers
 const uint32 sram_type;
 const int32 sram_start;
 const int32 sram_end;
} game_db_t;

static game_db_t GamesDB[] =
{
 // Balloon Boy
 { NULL, 0, 0xa9509f505d00db6eULL, "REALTEC", 0, 0, 0 },

 // Earth Defend
 { NULL, 0, 0xcf6afbf45299a800ULL, "REALTEC", 0, 0, 0 },

 // Whac a Critter
 { NULL, 0, 0x5499d14fcef32f60ULL, "REALTEC", 0, 0, 0 },

 // Super Street Fighter II
 { "T-12056 -00", 0, 0, "SSF2", 0, 0, 0 },
 { "T-12043 -00", 0, 0, "SSF2", 0, 0, 0 },

// "Conquering the world III 0x3fff"
// "Xin Qi Gai Wing Zi", "
//	0xFFFF, 0x400000, 0x40ffff
// "Rings of Power",      0x200000  0x203fff

 // Final Fantasy
 { NULL, 0, 0x7c0e11c426d65105ULL, "FF", 0, 0, 0 },
 { NULL, 0, 0xe144baf931c8b61eULL, "FF", 0, 0, 0 },

 // Rockman X3
 { NULL, 0, 0x1d1add6f2a64fb99ULL, "RMX3", 0, 0, 0 },

 // Super Bubble Bobble
 { NULL, 0, 0x8eff5373b653111eULL, "SBB", 0, 0, 0 },

 // Ya-Se Chuan Shuo
 { NULL, 0, 0x2786df4902ef8856ULL, "YaSe", 0, 0, 0 },

 // Chaoji Dafuweng
 { NULL, 0, 0x13639e87230c85aaULL, NULL,  0x5241f820, 0x200001, 0x200fff },

 // Psy-O-Blade
 { "T-26013 -00", 0, 0, NULL, 0x5241f820, 0x200000, 0x203fff },

 // Sonic and Knuckles and Sonic 3
 { NULL, 0xde0ac4e17a844d10ULL, 0, NULL, 0x5241f820, 0x200000, 0x2003ff },

 // Starflight
 { "T-50216 -00", 0, 0, NULL, 0x5241f820, 0x200000, 0x203fff },
 { "T-50216 -01", 0, 0, NULL, 0x5241f820, 0x200000, 0x203fff },


 // Taiwan Tycoon(TODO)

 // Top Shooter
 { NULL, 0, 0x31fea3093b231863ULL, NULL,  0x5241f820, 0x200001, 0x203fff },

 // World Pro Baseball 94
 { NULL, 0, 0xe7bb31787f189ebeULL, NULL, 0x5241f820, 0x200001, 0x20ffff },

 //
 // EEPROM Carts
 //
 //	Acclaim
 // NBA Jam (UE)
 {"T-081326", 0, 0, "Acclaim_24C02_Old", 0, 0, 0  },

 // NBA Jam (J)   
 {"T-81033", 0, 0, "Acclaim_24C02_Old", 0, 0, 0  },

 // NBA Jam TE
 {"T-81406", 0, 0, "Acclaim_24C02", 0, 0, 0  },

 // NFL Quarterback Club
 {"T-081276", 0, 0, "Acclaim_24C02", 0, 0, 0  },

 // NFL Quarterback Club '96
 {"T-081586", 0, 0, "Acclaim_24C16", 0, 0, 0  },

 // College Slam
 {"T-81576", 0, 0, "Acclaim_24C65", 0, 0, 0  },

 // Frank Thomas Big Hurt Baseball
 {"T-81476", 0, 0, "Acclaim_24C65", 0, 0, 0  },
        
 //	EA
 // NHLPA Hockey 93 (UE)
 {"T-50396", 0, 0, "EA_24C01", 0, 0, 0  },

 // Rings of Power
 {"T-50176", 0, 0, "EA_24C01", 0, 0, 0  },
        
 //	Sega
 // Megaman - The Wily Wars
 {"T-12046", 0, 0, "Sega_24C01", 0, 0, 0  },

 // Rockman Mega World (J) [A]
 {"T-12053", 0, 0, "Sega_24C01", 0, 0, 0  },

 // Evander 'Real Deal' Holyfield's Boxing
 {"MK-1215", 0, 0, "Sega_24C01", 0, 0, 0  },

 // Greatest Heavyweights of the Ring (U)
 {"MK-1228", 0, 0, "Sega_24C01", 0, 0, 0  },

 // Greatest Heavyweights of the Ring (J)
 {"G-5538", 0, 0, "Sega_24C01", 0, 0, 0  },

 // Greatest Heavyweights of the Ring (E)
 {"PR-1993", 0, 0, "Sega_24C01", 0, 0, 0  },

 // Wonderboy in Monster World
 {"G-4060", 0, 0, "Sega_24C01", 0, 0, 0  },

 // Sports Talk Baseball
 {"00001211-00", 0, 0, "Sega_24C01_Alt", 0, 0, 0  },
        
 //	Codemasters
 // Brian Lara Cricket
 {"T-120106", 0, 0, "Codemasters_24C01", 0, 0, 0  },

 // Micro Machines 2 - Turbo Tournament (E)
 {"T-120096-50", 0, 0, "Codemasters_24C08", 0, 0, 0  },

 // Micro Machines Military
 { NULL, 0, 0x34253755ee0eed41ULL, "Codemasters_24C08", 0, 0, 0  },

 // Micro Machines Military (bad)
 { NULL, 0, 0x3241b7da6ce42fecULL, "Codemasters_24C08", 0, 0, 0  },

 // Micro Machines Turbo Tournament 96
 {NULL, 0, 0xe672e84fed6ce270ULL, "Codemasters_24C16", 0, 0, 0  },

 // Micro Machines Turbo Tournament 96 (bad)
 {NULL, 0, 0x290afe3cd27be26cULL, "Codemasters_24C16", 0, 0, 0  },

 // Brian Lara Cricket 96, Shane Warne Cricket
 {"T-120146-50", 0, 0, "Codemasters_24C65", 0, 0, 0  },
 //
 // End EEPROM carts
 //
};

typedef struct
{
 const char *boardname;
 MD_Cart_Type *(*MapperMake)(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, 
	const uint32 iparam, const char *sparam);
 const uint32 iparam;
 const char *sparam;
} BoardHandler_t;

static BoardHandler_t BoardHandlers[] =
{
 { "ROM", MD_Make_Cart_Type_ROM, 0, NULL },
 { "SRAM", MD_Make_Cart_Type_SRAM, 0, NULL },

 { "REALTEC", MD_Make_Cart_Type_REALTEC, 0, NULL },
 { "SSF2", MD_Make_Cart_Type_SSF2, 0, NULL },
 { "FF", MD_Make_Cart_Type_FF, 0, NULL },
 { "SBB", MD_Make_Cart_Type_SBB, 0, NULL },
 { "YaSe", MD_Make_Cart_Type_YaSe, 0, NULL },

 { "Acclaim_24C02_Old", MD_Make_Cart_Type_EEPROM, EEP_ACCLAIM_24C02_OLD, NULL },
 { "Acclaim_24C02", MD_Make_Cart_Type_EEPROM, EEP_ACCLAIM_24C02, NULL }, 
 { "Acclaim_24C16", MD_Make_Cart_Type_EEPROM, EEP_ACCLAIM_24C16, NULL },
 { "Acclaim_24C65", MD_Make_Cart_Type_EEPROM, EEP_ACCLAIM_24C65, NULL },

 { "EA_24C01", MD_Make_Cart_Type_EEPROM, EEP_EA_24C01, NULL },

 { "Sega_24C01", MD_Make_Cart_Type_EEPROM, EEP_SEGA_24C01, NULL },
 { "Sega_24C01_Alt", MD_Make_Cart_Type_EEPROM, EEP_SEGA_24C01_ALT, NULL },

 { "Codemasters_24C01", MD_Make_Cart_Type_EEPROM, EEP_CM_24C01, NULL },
 { "Codemasters_24C08", MD_Make_Cart_Type_EEPROM, EEP_CM_24C08, NULL },
 { "Codemasters_24C16", MD_Make_Cart_Type_EEPROM, EEP_CM_24C16, NULL },
 { "Codemasters_24C65", MD_Make_Cart_Type_EEPROM, EEP_CM_24C65, NULL },
 { NULL, NULL, 0, NULL },
};

bool MDCart_TestMagic(const char *name, MDFNFILE *fp)
{
 if(fp->size < 512)
  return(FALSE);

 //if((fp->size & 512) && fp->data[0x01] == 0x03 && fp->data[0x08] == 0xAA && fp->data[0x09] == 0xBB && fp->data[0x0A] == 0x06)
 // return(TRUE);

 if(!memcmp(fp->data + 0x100, "SEGA MEGA DRIVE", 15) || !memcmp(fp->data + 0x100, "SEGA GENESIS", 12) || !memcmp(fp->data + 0x100, "SEGA 32X", 8))
  return(TRUE);

 if((!memcmp(fp->data + 0x100, "SEGA", 4) || !memcmp(fp->data + 0x100, " SEGA", 5)) && (!strcmp(fp->ext, "bin") || 
	!strcmp(fp->ext, "gen") || !strcmp(fp->ext, "md")))
  return(TRUE);

 if(!strcmp(fp->ext, "gen"))
 {
  puts("Warning");
  return(TRUE);
 }

 return(FALSE);
}

int MDCart_Load(md_game_info *ginfo, const char *name, MDFNFILE *fp)
{
 const char *mapper = NULL;

 md5_context md5;

 if(fp->size & 512)
 {
  if(memcmp(fp->data + 0x100, "SEGA", 4))
  {
   puts("Unhandled!");
   return(0);
  }
 }

 MD_ReadSegaHeader(fp->data + 0x100, ginfo);
 Cart_ROM_Size = fp->size;

 cart_rom = (uint8 *)MDFN_calloc(1, Cart_ROM_Size, _("Cart ROM"));
 memcpy(cart_rom, fp->data, fp->size);

 ginfo->rom_size = Cart_ROM_Size = fp->size;

 md5.starts();
 md5.update(fp->data, fp->size);
 md5.finish(ginfo->md5);

 md5.starts();
 md5.update(fp->data + 0x100, 0x100);
 md5.finish(ginfo->info_header_md5);

 ginfo->checksum_real = 0;
 for(uint32 i = 0x200; i < fp->size; i += 2)
 {
  ginfo->checksum_real += cart_rom[i + 0] << 8;
  ginfo->checksum_real += cart_rom[i + 1] << 0;
 }

 // Rockman MegaWorld: 5241e840
 // Sonic 3: 5241f820

 uint32 sram_type = READ_32_MSB(cart_rom, 0x1B0);
 uint32 sram_start = READ_32_MSB(cart_rom, 0x1B4);
 uint32 sram_end = READ_32_MSB(cart_rom, 0x1B8);

 {
  uint64 hmd5_partial = 0;
  uint64 md5_partial = 0;

  for(int i = 0; i < 8; i++)
  {
   hmd5_partial |= (uint64)ginfo->info_header_md5[15 - i] << (8 * i);
   md5_partial |= (uint64)ginfo->md5[15 - i] << (8 * i);
  }
  printf("Real: 0x%016llxULL    Header: 0x%016llxULL\n", (unsigned long long)md5_partial, (unsigned long long)hmd5_partial);

  for(int i = 0; i < sizeof(GamesDB) / sizeof(game_db_t); i++)
  {
   bool found = FALSE;

   if(GamesDB[i].header_md5 && GamesDB[i].header_md5 == hmd5_partial)
    found = TRUE;
   else if(GamesDB[i].md5 && GamesDB[i].md5 == md5_partial)
    found = TRUE;
   else if(GamesDB[i].id && !strcmp(GamesDB[i].id, ginfo->product_code))
    found = TRUE;

   if(found)
   {
    mapper = GamesDB[i].mapper;

    sram_type = GamesDB[i].sram_type;
    if(GamesDB[i].sram_start > 0)
     sram_start = GamesDB[i].sram_start;
    if(GamesDB[i].sram_end > 0)
     sram_end = GamesDB[i].sram_end;
    break;
   }
  }
 }

 printf("%08x, %08x, %08x\n", sram_type, sram_start, sram_end);
 
 if(sram_type == 0x5241f820 && sram_start == 0x20202020)
  sram_type = 0x20202020;

 ginfo->sram_type = sram_type;
 ginfo->sram_start = sram_start;
 ginfo->sram_end = sram_end;

 if(!mapper)
 {
  if(sram_type == 0x5241f820)
   mapper = "SRAM";
  else if(sram_type == 0x5241e840)
   mapper = "Sega_24C01";
  else
   mapper = "ROM";
 }

 {
  const BoardHandler_t *bh = BoardHandlers;
  bool BoardFound = FALSE;

  printf("Mapper: %s\n", mapper);
  while(bh->boardname)
  {
   if(!strcasecmp(bh->boardname, mapper))
   {
    cart_hardware = bh->MapperMake(ginfo, cart_rom, Cart_ROM_Size, bh->iparam, bh->sparam);
    BoardFound = TRUE;
    break;
   }
   bh++;
  }
  if(!BoardFound)
  {
   MDFN_printf(_("Handler for mapper/board \"%s\" not found!\n"), mapper);
   return(0);
  }
 }

 //MD_Cart_Type* (*MapperMake)(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size) = NULL;
 //cart_hardware = MapperMake(ginfo, cart_rom, Cart_ROM_Size);

 return(1);
}

bool MDCart_LoadNV(void)
{
 // Load any saved RAM/EEPROM now!
 if(cart_hardware->GetNVMemorySize())
 {
  uint8 buf[cart_hardware->GetNVMemorySize()];

  memset(buf, 0, sizeof(buf));
  gzFile sp;

  if((sp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb")))
  {
   if(gzread(sp, buf, sizeof(buf)) == sizeof(buf))
   {
    cart_hardware->WriteNVMemory(buf);
   }
   gzclose(sp);
  }
 }
 return(TRUE);
}

bool MDCart_Close(void)
{
 if(cart_hardware)
 {
  if(cart_hardware->GetNVMemorySize())
  {
   uint8 buf[cart_hardware->GetNVMemorySize()];

   cart_hardware->ReadNVMemory(buf);

   MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, buf, sizeof(buf));
  }

  delete cart_hardware;

  cart_hardware = NULL;
 }
 return(TRUE);
}

int MDCart_StateAction(StateMem *sm, int load, int data_only)
{
 return(cart_hardware->StateAction(sm, load, data_only, "CARTBOARD"));
}

