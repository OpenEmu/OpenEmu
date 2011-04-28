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

#include "shared.h"
#include "header.h"
#include "../general.h"
#include <ctype.h>

namespace MDFN_IEN_MD
{

typedef struct
{
 uint8 ascii;
 uint8 numeric;
} region_lut_t;

static const region_lut_t region_lut[] =
{
 { 0xFF, 0x1 },
 { 0x00, 0x1 },
 { 0xd4, 0x1 },

 { 4, 0x4   }, // Silly Toy Story

 { ' ', 0x1 }, // Assume Japanese
 { 'J',	0x1 },
 { 'U', 0x4 },
 { 'E', 0x8 },

 { '0', 0x0 },
 { '1', 0x1 },
 { '2', 0x2 },
 { '3', 0x3 },
 { '4', 0x4 },
 { '5', 0x5 },
 { '6', 0x6 },
 { '7', 0x7 },
 { '8', 0x8 },
 { '9', 0x9 },
 { 'A', 0xa },
 { 'B', 0xb },
 { 'C', 0xc },
 { 'D', 0xd },
 { 'E', 0xe },
 { 'F', 0xf }
};

static uint8 get_region_numeric(uint8 code)
{
 code = toupper(code);

 for(int i = 0; i < sizeof(region_lut) / sizeof(region_lut_t); i++)
 {
  if(region_lut[i].ascii == code)
  {
   return(region_lut[i].numeric);
  }
 }

 printf("Unknown region code: %c %02x\n", code, code);
 //exit(1);

 return(0x0);
}

void MD_ReadSegaHeader(const uint8 *header, md_game_info *ginfo)
{
 ginfo->rom_size = 0; //fp->size;

 memcpy(ginfo->copyright, header + 0x10, 16);
 ginfo->copyright[16] = 0;
 MDFN_RemoveControlChars(ginfo->copyright);

 memcpy(ginfo->domestic_name, header + 0x20, 0x30);
 ginfo->domestic_name[0x30] = 0;
 MDFN_RemoveControlChars(ginfo->domestic_name);

 memcpy(ginfo->overseas_name, header + 0x50, 0x30);
 ginfo->overseas_name[0x30] = 0;
 MDFN_RemoveControlChars(ginfo->overseas_name);

 memcpy(ginfo->product_code, header + 0x82, 0x0C);
 ginfo->product_code[0xC] = 0;
 MDFN_RemoveControlChars(ginfo->product_code);
 MDFN_trim(ginfo->product_code);

 ginfo->checksum = (header[0x8E] << 8) | (header[0x8F] << 0);

 for(int i = 0; i < 16; i++)
 {
  for(unsigned int iot = 0; iot < sizeof(IO_types) / sizeof(IO_type_t); iot++)
  {
   if(IO_types[iot].code_char == header[0x90 + i])
   {
    ginfo->io_support |= 1 << IO_types[iot].id;
    break;
   }
  }
 }

 ginfo->region_support = get_region_numeric(header[0xF0]);
}


}
