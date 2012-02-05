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

  /* store SRAM into cartridge area */
  if (cart.romsize > 0x500000) return;
  sram.sram = cart.rom + 0x500000;

  /* initialize SRAM */
  memset(sram.sram, 0xff, 0x10000);
  sram.crc = crc32(0, sram.sram, 0x10000);

  /* retrieve informations from header */
  if ((READ_BYTE(cart.rom,0x1b0) == 0x52) && (READ_BYTE(cart.rom,0x1b1) == 0x41))
  {
    sram.start = READ_WORD_LONG(cart.rom, 0x1b4);
    sram.end   = READ_WORD_LONG(cart.rom, 0x1b8);

    /* fixe some bad header informations */
    if ((sram.start > sram.end) || ((sram.end - sram.start) >= 0x10000))
    {
      sram.end = sram.start + 0xffff;
    }
    sram.start &= 0xfffffffe;
    sram.end |= 1;

    /* enable SRAM */
    sram.on = 1;
    sram.detected = 1;
  }
  else
  {
    /* default SRAM region */
    sram.start = 0x200000;
    sram.end = 0x20ffff;

    /* enable SRAM only if ROM < 2MB */
    if (cart.romsize <= sram.start)
      sram.on = 1;
  }

  /* autodetect some games with bad header or specific configuration */
  if (strstr(rominfo.product,"T-113016") != NULL)
  {
    /* Pugsy (try accessing unmapped area for copy protection) */
    sram.on = 0;
  }
  else if (strstr(rominfo.international,"SONIC THE HEDGEHOG 2") != NULL)
  {
    /* Sonic the Hedgehog 2 does not use SRAM */
    /* this prevents SRAM activation when using Sonic & Knuckles LOCK-ON feature */
    sram.on = 0;
  }
  else if (strstr(rominfo.international,"SONIC & KNUCKLES") != NULL)
  {
    if (cart.romsize == 0x400000)
    {
      /* Sonic 3 & Knuckles */
      /* the combined ROM has S&K header but should obviously use FRAM from Sonic the Hedgehog 3 */
      sram.on = 1;
    }
  }
  else if (strstr(rominfo.product,"T-26013") != NULL)
  {
    /* Psy-O-Blade (bad header) */
    sram.on = 1;
    sram.start = 0x200001;
    sram.end = 0x203fff;
  }
  else if (strstr(rominfo.product,"T-50086") != NULL)
  {
    /* PGA Tour Golf (no header) */
    sram.on = 1;
    sram.start = 0x200001;
    sram.end = 0x203fff;
  }
  else if (strstr(rominfo.product,"ACLD007") != NULL)
  {
    /* Winter Challenge (no header) */
    sram.on = 1;
    sram.start = 0x200001;
    sram.end = 0x200fff;
  }
  else if (strstr(rominfo.product,"T-50286") != NULL)
  {
    /* Buck Rogers - Countdown to Doomsday (no header) */
    sram.on = 1;
    sram.start = 0x200001;
    sram.end = 0x203fff;
  }
  else if (((rominfo.realchecksum == 0xaeaa) || (rominfo.realchecksum == 0x8dba)) && 
           (rominfo.checksum ==  0x8104))
  {
    /* Xin Qigai Wangzi, aka Beggar Prince (no header, use uncommon area) */
    sram.on = 1;
    sram.start = 0x400000;
    sram.end = 0x40ffff;
  }
}
