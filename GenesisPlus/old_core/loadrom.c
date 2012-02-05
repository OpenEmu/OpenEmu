/***************************************************************************************
 *  Genesis Plus
 *  ROM Loading Support
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007,2008,2009), additional code & fixes for the GCN/Wii port
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

#include <ctype.h>
#include "shared.h"
#include "fileio.h"

/*** ROM Information ***/
#define ROMCONSOLE    256
#define ROMCOPYRIGHT  272
#define ROMDOMESTIC   288
#define ROMWORLD      336
#define ROMTYPE       384
#define ROMPRODUCT    386
#define ROMCHECKSUM   398
#define ROMIOSUPPORT  400
#define ROMROMSTART   416
#define ROMROMEND     420
#define ROMRAMINFO    424
#define ROMRAMSTART   436
#define ROMRAMEND     440
#define ROMMODEMINFO  444
#define ROMMEMO       456
#define ROMCOUNTRY    496

#define P3BUTTONS   1
#define P6BUTTONS   2
#define PKEYBOARD   4
#define PPRINTER    8
#define PBALL       16
#define PFLOPPY     32
#define PACTIVATOR  64
#define PTEAMPLAYER 128
#define PMSYSTEMPAD 256
#define PSERIAL     512
#define PTABLET     1024
#define PPADDLE     2048
#define PCDROM      4096
#define PMOUSE      8192

uint16 peripherals;
uint16 realchecksum;
ROMINFO rominfo;

/***************************************************************************
  * Genesis ROM Manufacturers
  *
  * Based on the document provided at
  * http://www.zophar.net/tech/files/Genesis_ROM_Format.txt
  **************************************************************************/
COMPANYINFO companyinfo[MAXCOMPANY] = {
  {"ACLD", "Ballistic"},
  {"RSI", "Razorsoft"},
  {"SEGA", "SEGA"},
  {"TREC", "Treco"},
  {"VRGN", "Virgin Games"},
  {"WSTN", "Westone"},
  {"10", "Takara"},
  {"11", "Taito or Accolade"},
  {"12", "Capcom"},
  {"13", "Data East"},
  {"14", "Namco or Tengen"},
  {"15", "Sunsoft"},
  {"16", "Bandai"},
  {"17", "Dempa"},
  {"18", "Technosoft"},
  {"19", "Technosoft"},
  {"20", "Asmik"},
  {"22", "Micronet"},
  {"23", "Vic Tokai"},
  {"24", "American Sammy"},
  {"29", "Kyugo"},
  {"32", "Wolfteam"},
  {"33", "Kaneko"},
  {"35", "Toaplan"},
  {"36", "Tecmo"},
  {"40", "Toaplan"},
  {"42", "UFL Company Limited"},
  {"43", "Human"},
  {"45", "Game Arts"},
  {"47", "Sage's Creation"},
  {"48", "Tengen"},
  {"49", "Renovation or Telenet"},
  {"50", "Electronic Arts"},
  {"56", "Razorsoft"},
  {"58", "Mentrix"},
  {"60", "Victor Musical Industries"},
  {"69", "Arena"},
  {"70", "Virgin"},
  {"73", "Soft Vision"},
  {"74", "Palsoft"},
  {"76", "Koei"},
  {"79", "U.S. Gold"},
  {"81", "Acclaim/Flying Edge"},
  {"83", "Gametek"},
  {"86", "Absolute"},
  {"87", "Mindscape"},
  {"93", "Sony"},
  {"95", "Konami"},
  {"97", "Tradewest"},
  {"100", "T*HQ Software"},
  {"101", "Tecmagik"},
  {"112", "Designer Software"},
  {"113", "Psygnosis"},
  {"119", "Accolade"},
  {"120", "Code Masters"},
  {"125", "Interplay"},
  {"130", "Activision"},
  {"132", "Shiny & Playmates"},
  {"144", "Atlus"},
  {"151", "Infogrames"},
  {"161", "Fox Interactive"},
  {"177", "Ubisoft"},
  {"239", "Disney Interactive"},
  {"---", "Unknown"}
};

 /***************************************************************************
  * Genesis Peripheral Information
  *
  * Based on the document provided at
  * http://www.zophar.net/tech/files/Genesis_ROM_Format.txt
  ***************************************************************************/
PERIPHERALINFO peripheralinfo[14] = {
  {"J", "3-Button Joypad"},
  {"6", "6-button Joypad"},
  {"K", "Keyboard"},
  {"P", "Printer"},
  {"B", "Control Ball"},
  {"F", "Floppy Drive"},
  {"L", "Activator"},
  {"4", "Team Player"},
  {"0", "MS Joypad"},
  {"R", "RS232C Serial"},
  {"T", "Tablet"},
  {"V", "Paddle"},
  {"C", "CD-ROM"},
  {"M", "Mega Mouse"}
};

/*
 * softdev - New Checksum Calculation
   eke-eke: fixed 
 */
static uint16 GetRealChecksum (uint8 *rom, int length)
{
  int i;
  uint16 checksum = 0;

  for (i = 0; i < length; i += 2)
  {
    checksum += ((rom[i] << 8) + rom[i + 1]);
  }

  return checksum;
}

 /***************************************************************************
  * getrominfo
  *
  * Pass a pointer to the ROM base address.
  ***************************************************************************/
static void getrominfo (char *romheader)
{
  int i,j;

  memset (&rominfo, 0, sizeof (ROMINFO));

  memcpy (&rominfo.consoletype, romheader + ROMCONSOLE, 16);
  memcpy (&rominfo.copyright, romheader + ROMCOPYRIGHT, 16);

  rominfo.domestic[0] = romheader[ROMDOMESTIC];
  j=1;
  for (i=1; i<48; i++)
  {
    if ((rominfo.domestic[j-1] != 32) || (romheader[ROMDOMESTIC + i] != 32))
    {
      rominfo.domestic[j] = romheader[ROMDOMESTIC + i];
      j++;
    }
  }
  rominfo.domestic[j] = 0;

  rominfo.international[0] = romheader[ROMWORLD];
  j=1;
  for (i=1; i<48; i++)
  {
    if ((rominfo.international[j-1] != 32) || (romheader[ROMWORLD + i] != 32))
    {
      rominfo.international[j] = romheader[ROMWORLD + i];
      j++;
    }
  }
  rominfo.international[j] = 0;

  memcpy (&rominfo.ROMType, romheader + ROMTYPE, 2);
  memcpy (&rominfo.product, romheader + ROMPRODUCT, 12);
  memcpy (&rominfo.checksum, romheader + ROMCHECKSUM, 2);
  memcpy (&rominfo.io_support, romheader + ROMIOSUPPORT, 16);
  memcpy (&rominfo.romstart, romheader + ROMROMSTART, 4);
  memcpy (&rominfo.romend, romheader + ROMROMEND, 4);
  memcpy (&rominfo.RAMInfo, romheader + ROMRAMINFO, 12);
  memcpy (&rominfo.ramstart, romheader + ROMRAMSTART, 4);
  memcpy (&rominfo.ramend, romheader + ROMRAMEND, 4);
  memcpy (&rominfo.modem, romheader + ROMMODEMINFO, 12);
  memcpy (&rominfo.memo, romheader + ROMMEMO, 40);
  memcpy (&rominfo.country, romheader + ROMCOUNTRY, 16);

  realchecksum = GetRealChecksum (((uint8 *) cart_rom) + 0x200, genromsize - 0x200);
#ifdef LSB_FIRST
  rominfo.checksum =  (rominfo.checksum >> 8) | ((rominfo.checksum & 0xff) << 8);
#endif

  peripherals = 0;

  for (i = 0; i < 14; i++)
  for (j=0; j < 14; j++)
  if (rominfo.io_support[i] == peripheralinfo[j].pID[0]) peripherals |= (1 << j);
}

/* SMD (interleaved) rom support */
static uint8 block[0x4000];

static void deinterleave_block (uint8 * src)
{
  int i;
  memcpy (block, src, 0x4000);
  for (i = 0; i < 0x2000; i += 1)
  {
      src[i * 2 + 0] = block[0x2000 + (i)];
      src[i * 2 + 1] = block[0x0000 + (i)];
  }
}

int load_rom(char *filename)
{
  int i, size, offset = 0;
 
#ifdef NGC
  size = genromsize;
  sprintf(rom_filename,"%s",filename);
  rom_filename[strlen(rom_filename) - 4] = 0;
#else
  uint8 *ptr;
  ptr = load_archive(filename, &size);
  if(!ptr) return (0);
  memcpy(cart_rom, ptr + offset, size);
  free(ptr);
#endif

  /* detect interleaved roms (.smd format) */
  if (strncmp((char *)(cart_rom + 0x100),"SEGA", 4) && ((size / 512) & 1))
  {
    size -= 512;
    offset += 512;

    for (i = 0; i < (size / 0x4000); i += 1)
    {
      deinterleave_block (cart_rom + offset + (i * 0x4000));
    }

      memcpy(cart_rom, cart_rom + offset, size);
  }

  /* max. 10 MBytes supported */
  if (size > MAXROMSIZE) size = MAXROMSIZE;
  genromsize = size;
  
  /* clear unused ROM space */
  if (size < MAXROMSIZE) memset (cart_rom + size, 0x00, MAXROMSIZE - size);

  getrominfo((char *)cart_rom);  /* get infos from ROM header */
  set_region();      /* set game region (PAL/NTSC, JAP/USA/EUR) */
   
#ifdef LSB_FIRST
  /* Byteswap ROM */
  uint8 temp;
  for(i = 0; i < genromsize; i += 2)
  {
    temp = cart_rom[i];
    cart_rom[i] = cart_rom[i+1];
    cart_rom[i+1] = temp;
  }
#endif

  /* byteswapped RADICA dumps (from Haze) */
  if (((strstr(rominfo.product,"-K0101") != NULL) && (rominfo.checksum == 0xf424)) ||
      ((strstr(rominfo.product,"-K0109") != NULL) && (rominfo.checksum == 0x4f10)))
  {
    uint8 temp;
    for(i = 0; i < genromsize; i += 2)
    {
      temp = cart_rom[i];
      cart_rom[i] = cart_rom[i+1];
      cart_rom[i+1] = temp;
    }
  }

  /* console hardware */
  if (strstr(rominfo.consoletype, "SEGA PICO") != NULL) system_hw = SYSTEM_PICO;
  else if (strstr(rominfo.consoletype, "SEGA MEGADRIVE") != NULL) system_hw = SYSTEM_MEGADRIVE;
  else system_hw = SYSTEM_GENESIS;

  return(1);
}

/* 05/05/2006: new region detection routine (taken from GENS sourcecode) */
void set_region ()
{
  /* country codes used to differentiate region */
  /* 0001 = japan ntsc (1) */
  /* 0010 = japan  pal (2) */
  /* 0100 = usa        (4) */
  /* 1000 = europe     (8) */

  int country = 0;
  int i = 0;
  char c;

  /* reading header to find the country */
  if (!strncasecmp(rominfo.country, "eur", 3)) country |= 8;
  else if (!strncasecmp(rominfo.country, "usa", 3)) country |= 4;
  else if (!strncasecmp(rominfo.country, "jap", 3)) country |= 1;

  else for(i = 0; i < 4; i++)
  {
    c = toupper((int)rominfo.country[i]);
    if (c == 'U') country |= 4;
    else if (c == 'J') country |= 1;
    else if (c == 'E') country |= 8;
    else if (c < 16) country |= c;
    else if ((c >= '0') && (c <= '9')) country |= c - '0';
    else if ((c >= 'A') && (c <= 'F')) country |= c - 'A' + 10;
  }

  /* automatic detection */
  /* setting region */
  /* this is used by IO register */
  if (country & 4) region_code = REGION_USA;
  else if (country & 1) region_code = REGION_JAPAN_NTSC;
  else if (country & 8) region_code = REGION_EUROPE;
  else if (country & 2) region_code = REGION_JAPAN_PAL;
  else region_code = REGION_USA;

  /* some games need specific REGION setting */
  if (((strstr(rominfo.product,"T-45033") != NULL) && (rominfo.checksum == 0x0F81)) || /* Alisia Dragon (E) */
       (strstr(rominfo.product,"T-69046-50") != NULL)) /* On Dal Jang Goon (Korea) */
  {
    /* need PAL settings */
    region_code = REGION_EUROPE;
  }
  else if ((realchecksum == 0x532e) && (strstr(rominfo.product,"1011-00") != NULL)) 
  {
    /* On Dal Jang Goon (Korea) needs JAP region code */
    region_code = REGION_JAPAN_NTSC;
  }

  /* Force region setting */
  if (config.region_detect == 1) region_code = REGION_USA;
  else if (config.region_detect == 2) region_code = REGION_EUROPE;
  else if (config.region_detect == 3) region_code = REGION_JAPAN_NTSC;
  else if (config.region_detect == 4) region_code = REGION_JAPAN_PAL;

  /* set cpu/vdp speed: PAL or NTSC */
  if ((region_code == REGION_EUROPE) || (region_code == REGION_JAPAN_PAL)) vdp_pal = 1;
  else vdp_pal = 0;
}

/****************************************************************************
 * getcompany
 *
 * Try to determine which company made this rom
 *
 * Ok, for some reason there's no standard for this.
 * It seems that there can be pretty much anything you like following the
 * copyright (C) symbol!
 ****************************************************************************/
int getcompany ()
{
  char *s;
  int i;
  char company[10];

  for (i = 3; i < 8; i++) company[i - 3] = rominfo.copyright[i];
  company[5] = 0;

  /** OK, first look for a hyphen
   *  Capcom use T-12 for example
   */
  s = strstr (company, "-");
  if (s != NULL)
  {
    s++;
    strcpy (company, s);
  }

  /** Strip any trailing spaces **/
  for (i = strlen (company) - 1; i >= 0; i--)
  if (company[i] == 32) company[i] = 0;

  if (strlen (company) == 0) return MAXCOMPANY - 1;

  for (i = 0; i < MAXCOMPANY - 1; i++)
  {
    if (!(strncmp (company, companyinfo[i].companyid, strlen (company)))) return i;
  }

  return MAXCOMPANY - 1;
}
