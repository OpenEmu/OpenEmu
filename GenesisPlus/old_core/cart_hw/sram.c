/***************************************************************************************
 *  Genesis Plus
 *  Backup RAM support
 *
 *  Copyright (C) 2007, 2008, 2009  Eke-Eke (GCN/Wii port)
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
 *
 ****************************************************************************************/

#include "shared.h"
#include "zlib.h"

T_SRAM sram;

/****************************************************************************
 * A quick guide to SRAM on the Genesis
 *
 * The SRAM definition is held at offset 0x1b0 of the ROM header.
 *
 *  1B0h:   dc.b   'RA', %1x1yz000, %00100000
 *  1B4h:   dc.l   RAM start address
 *  1B8h:   dc.l   RAM end address
 *   x 1 for BACKUP and 0 If not BACKUP
 *   yz 10 if even address only 
 *      11 if odd address only 
 *      00 if both even and odd address
 *
 * Assuming max. 64k SRAM / Battery RAM throughout
 ****************************************************************************/
void sram_init()
{
  memset (&sram, 0, sizeof (T_SRAM));
  memset (&sram.sram[0], 0xFF, 0x10000);
  sram.crc = crc32 (0, &sram.sram[0], 0x10000);

  if ((READ_BYTE(cart_rom,0x1b0) == 0x52) && (READ_BYTE(cart_rom,0x1b1) == 0x41))
  {
    /* retrieve informations from headezr */
    sram.detected = 1;
    sram.start = READ_WORD_LONG(cart_rom, 0x1b4);
    sram.end   = READ_WORD_LONG(cart_rom, 0x1b8);

    /* fixe some bad header informations */
    if ((sram.start > sram.end) || ((sram.end - sram.start) >= 0x10000))
    sram.end = sram.start + 0xffff;
    sram.start &= 0xfffffffe;
    sram.end |= 1;
  }
  else 
  {
    /* default SRAM region */
    sram.start = 0x200000;
    sram.end = 0x20ffff;
  }

  /* set SRAM ON by default when ROM is not mapped */
  if (genromsize <= sram.start)
  {
    sram.on = 1;
    sram.write = 1;
  }

  /* Some games with bad header or specific configuration */
  if (strstr(rominfo.product,"T-113016") != NULL)
  {
    /* Pugsy (try accessing unmapped area for copy protection) */
    sram.on = 0;
    sram.write = 0;
  }
  else if ((strstr(rominfo.product,"T-26013") != NULL) && (rominfo.checksum == 0xa837))
  {
    /* Psy-O-Blade (bad header) */
    sram.on = 1;
    sram.write = 1;
    sram.start = 0x200001;
    sram.end = 0x203fff;
  }
  else if (((realchecksum == 0xaeaa) || (realchecksum == 0x8dba)) &&  (rominfo.checksum ==  0x8104))
  {
    /* Xin Qigai Wangzi, aka Beggar Prince (no header, use uncommon area) */
    sram.on = 1;
    sram.write = 1;
    sram.start = 0x400000;
    sram.end = 0x40ffff;
  }
}
