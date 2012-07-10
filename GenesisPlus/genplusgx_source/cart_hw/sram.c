/***************************************************************************************
 *  Genesis Plus
 *  Backup RAM support
 *
 *  Copyright (C) 2007-2011  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
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

  /* SRAM data is stored above cartridge ROM area, at $500000-$50FFFF (max. 64K) */
  if (cart.romsize > 0x500000) return;
  sram.sram = cart.rom + 0x500000;

  /* initialize SRAM */
  memset(sram.sram, 0xFF, 0x10000);
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
    /* by default, enable SRAM only for ROM <= 2MB */
    if (cart.romsize <= 0x200000)
    {
      /* SRAM mapped to $200000-$20ffff */
      sram.start = 0x200000;
      sram.end = 0x20ffff;
      sram.on = 1;
    }
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
