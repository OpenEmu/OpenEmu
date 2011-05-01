/* Mednafen - Multi-system Emulator
 * 
 * Copyright notice for this file:
 *  Copyright (C) 2002,2003 Xodnizel
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

#include "../nes.h"
#include <math.h>
#include "palette.h"

static const MDFNPalStruct rp2c04_0001[64] = {
 #include "palettes/rp2c04-0001.h"
};

static const MDFNPalStruct rp2c04_0002[64] = {
 #include "palettes/rp2c04-0002.h"
};

static const MDFNPalStruct rp2c04_0003[64] = {
 #include "palettes/rp2c04-0003.h"
};

static const MDFNPalStruct rp2c04_0004[64] = {
 #include "palettes/rp2c04-0004.h"
};

static const MDFNPalStruct rp2c0x[64] = {
 #include "palettes/rp2c0x.h"
};

/* Default palette */
static const MDFNPalStruct default_palette[64] = {
 #include "palettes/default.h"
};


MDFNPalStruct ActiveNESPalette[0x200];

static const char *pal_altnames[6] = { NULL, "rp2c04-0001", "rp2c04-0002", "rp2c04-0003", "rp2c04-0004", "rp2c0x" };
static const MDFNPalStruct *Palettes[6] =
{
     default_palette,
     rp2c04_0001,
     rp2c04_0002,
     rp2c04_0003,
     rp2c04_0004,
     rp2c0x
};

static bool LoadCPalette(MDFNPalStruct *palette, const char *sys)
{
 uint8 ptmp[192];
 FILE *fp;

 std::string cpalette_fn = MDFN_MakeFName(MDFNMKF_PALETTE, 0, sys).c_str();

 MDFN_printf(_("Loading custom palette from \"%s\"...\n"),  cpalette_fn.c_str());
 MDFN_indent(1);

 if(!(fp = fopen(cpalette_fn.c_str(), "rb")))
 {
  ErrnoHolder ene(errno);

  MDFN_printf(_("Error opening file: %s\n"), ene.StrError());
  MDFN_indent(-1);
  return(false);
 }

 if(fread(ptmp, 1, 192, fp) != 192)
 {
  ErrnoHolder ene(errno);

  MDFN_PrintError(_("Error reading file: %s\n"), feof(fp) ? _("File length is too short") : ene.StrError());
  fclose(fp);
  return(false);
 }

 fclose(fp);

 for(int x = 0; x < 64; x++)
 {
  palette[x].r = ptmp[x * 3 + 0];
  palette[x].g = ptmp[x * 3 + 1];
  palette[x].b = ptmp[x * 3 + 2];
 }

 return(true);
}

void MDFN_InitPalette(const unsigned int which)
{
 static const double rtmul[8] = { 1, 1.239,  .794, 1.019,  .905, 1.023, .741, .75 };
 static const double gtmul[8] = { 1,  .915, 1.086,  .98,  1.026,  .908, .987, .75 };
 static const double btmul[8] = { 1,  .743,  .882,  .653, 1.277,  .979, .101, .75 };
 MDFNPalStruct custom_palette[64];
 const MDFNPalStruct *palette = Palettes[which];

 if(LoadCPalette(custom_palette, pal_altnames[which]))
  palette = custom_palette;

 for(int x = 0; x < 0x200; x++)
 {
  int emp = which ? 0 : (x >> 6);
  int r = (int)(rtmul[emp] * palette[x & 0x3F].r);
  int g = (int)(gtmul[emp] * palette[x & 0x3F].g);
  int b = (int)(btmul[emp] * palette[x & 0x3F].b);

  if(r > 255) r = 255;
  if(g > 255) g = 255;
  if(b > 255) b = 255;

  ActiveNESPalette[x].r = r;
  ActiveNESPalette[x].g = g;
  ActiveNESPalette[x].b = b;
 }
}

