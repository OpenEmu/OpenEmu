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

#define MAXCOMPANY 64
#define MAXPERIPHERALS 14

typedef struct
{
  char companyid[6];
  char company[26];
} COMPANYINFO;

typedef struct
{
  char pID[2];
  char pName[14];
} PERIPHERALINFO;


ROMINFO rominfo;
char rom_filename[256];


/***************************************************************************
  * Genesis ROM Manufacturers
  *
  * Based on the document provided at
  * http://www.zophar.net/tech/files/Genesis_ROM_Format.txt
  **************************************************************************/
static const COMPANYINFO companyinfo[MAXCOMPANY] =
{
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
  {"60", "Victor Musical Ind."},
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
static const PERIPHERALINFO peripheralinfo[MAXPERIPHERALS] =
{
  {"J", "3B Joypad"},
  {"6", "6B Joypad"},
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
  {"M", "Mega Mouse"},
};

 /***************************************************************************
  *
  * Compute ROM real checksum.
  ***************************************************************************/
static uint16 getchecksum(uint8 *rom, int length)
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
  *
  * Pass a pointer to the ROM base address.
  ***************************************************************************/
static void getrominfo(char *romheader)
{
  uint16 offset = 0;

  /* Clear ROM info structure */
  memset (&rominfo, 0, sizeof (ROMINFO));

  /* Look for Master System ROM header */
  if (!memcmp (&cart.rom[0x1ff0], "TMR SEGA", 8))
  {
    offset = 0x1ff0;
  }
  else if (!memcmp (&cart.rom[0x3ff0], "TMR SEGA", 8))
  {
    offset = 0x3ff0;
  }
  else if (!memcmp (&cart.rom[0x7ff0], "TMR SEGA", 8))
  {
    offset = 0x7ff0;
  }

  /* If found, assume this is a SMS game */
  if (offset)
  {
    /* force SMS compatibilty mode */
    system_hw = SYSTEM_PBC;

    /* checksum */
    rominfo.checksum = cart.rom[offset + 0x0a] | (cart.rom[offset + 0x0b] << 8);

    /* product code & version */
    sprintf(&rominfo.product[0], "%02d", cart.rom[offset + 0x0e] >> 4);
    sprintf(&rominfo.product[2], "%02x", cart.rom[offset + 0x0d]);
    sprintf(&rominfo.product[4], "%02x", cart.rom[offset + 0x0c]);
    sprintf(&rominfo.product[6], "-%d", cart.rom[offset + 0x0e] & 0x0F);

    /* region code */
    switch (cart.rom[offset + 0x0f] >> 4)
    {
      case 3:
        strcpy(rominfo.country,"SMS Japan");
        break;
      case 4:
        strcpy(rominfo.country,"SMS Export");
        break;
      case 5:
        strcpy(rominfo.country,"GG Japan");
        break;
      case 6:
        strcpy(rominfo.country,"GG Export");
        break;
      case 7:
        strcpy(rominfo.country,"GG International");
        break;
      default:
        sprintf(rominfo.country,"Unknown (%d)", cart.rom[offset + 0x0f] >> 4);
        break;
    }

    /* ROM size */
    rominfo.romstart = 0;
    switch (cart.rom[offset + 0x0f] & 0x0F)
    {
      case 0x00:
        rominfo.romend = 0x3FFFF;
        break;
      case 0x01:
        rominfo.romend = 0x7FFFF;
        break;
      case 0x02:
        rominfo.romend = 0xFFFFF;
        break;
      case 0x0a:
        rominfo.romend = 0x1FFF;
        break;
      case 0x0b:
        rominfo.romend = 0x3FFF;
        break;
      case 0x0c:
        rominfo.romend = 0x7FFF;
        break;
      case 0x0d:
        rominfo.romend = 0xBFFF;
        break;
      case 0x0e:
        rominfo.romend = 0xFFFF;
        break;
      case 0x0f:
        rominfo.romend = 0x1FFFF;
        break;
    }
  }
  else
  {
    /* Some SMS games don't have any header */
    if (system_hw == SYSTEM_PBC) return;

    /* Genesis ROM header support */
    memcpy (&rominfo.consoletype, romheader + ROMCONSOLE, 16);
    memcpy (&rominfo.copyright, romheader + ROMCOPYRIGHT, 16);

    /* Domestic (japanese) name */
    rominfo.domestic[0] = romheader[ROMDOMESTIC];
    int i, j = 1;
    for (i=1; i<48; i++)
    {
      if ((rominfo.domestic[j-1] != 32) || (romheader[ROMDOMESTIC + i] != 32))
      {
        rominfo.domestic[j] = romheader[ROMDOMESTIC + i];
        j++;
      }
    }
    rominfo.domestic[j] = 0;

    /* International name */
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

    /* ROM informations */
    memcpy (&rominfo.ROMType, romheader + ROMTYPE, 2);
    memcpy (&rominfo.product, romheader + ROMPRODUCT, 12);
    memcpy (&rominfo.checksum, romheader + ROMCHECKSUM, 2);
    memcpy (&rominfo.romstart, romheader + ROMROMSTART, 4);
    memcpy (&rominfo.romend, romheader + ROMROMEND, 4);
    memcpy (&rominfo.country, romheader + ROMCOUNTRY, 16);

    /* Checksums */
#ifdef LSB_FIRST
    rominfo.checksum =  (rominfo.checksum >> 8) | ((rominfo.checksum & 0xff) << 8);
#endif
    rominfo.realchecksum = getchecksum(((uint8 *) cart.rom) + 0x200, cart.romsize - 0x200);

    /* Supported peripherals */
    rominfo.peripherals = 0;
    for (i = 0; i < 14; i++)
      for (j=0; j < 14; j++)
        if (romheader[ROMIOSUPPORT+i] == peripheralinfo[j].pID[0])
          rominfo.peripherals |= (1 << j);
  }
}

 /***************************************************************************
  * deinterleave_block
  *
  * Convert interleaved (.smd) ROM files.
  ***************************************************************************/
static void deinterleave_block(uint8 * src)
{
  int i;
  uint8 block[0x4000];
  memcpy (block, src, 0x4000);
  for (i = 0; i < 0x2000; i += 1)
  {
      src[i * 2 + 0] = block[0x2000 + (i)];
      src[i * 2 + 1] = block[0x0000 + (i)];
  }
}

 /***************************************************************************
  * load_rom
  *
  * Load a new ROM file.
  ***************************************************************************/
int load_rom(char *filename)
{
  int i, size;
 
#ifdef NGC
  size = cart.romsize;
#else
  uint8 *ptr;
  ptr = load_archive(filename, &size);
  if(!ptr) return (0);
  memcpy(cart.rom, ptr, size);
  free(ptr);
#endif

  /* Minimal ROM size */
  if (size < 0x4000)
  {
    memset(cart.rom + size, 0xFF, 0x4000 - size);
    size = 0x4000;
  }

  /* Get file extension */
  if (!strnicmp(".sms", &filename[strlen(filename) - 4], 4))
  {
    /* Force SMS compatibility mode */
    system_hw = SYSTEM_PBC;
  }
  else
  {
    /* Assume Genesis mode */
    system_hw = SYSTEM_GENESIS;
  }

  /* Take care of 512 byte header, if present */
  if (strncmp((char *)(cart.rom + 0x100),"SEGA", 4) && ((size / 512) & 1))
  {
    size -= 512;
    memcpy (cart.rom, cart.rom + 512, size);

    /* interleaved ROM format (.smd) */
    if (system_hw != SYSTEM_PBC)
    {
      for (i = 0; i < (size / 0x4000); i++)
      {
        deinterleave_block (cart.rom + (i * 0x4000));
      }
    }
  }

  /* max. 10 MBytes supported */
  if (size > MAXROMSIZE) size = MAXROMSIZE;
  cart.romsize = size;

  /* clear unused ROM space */
  memset(cart.rom + size, 0xff, MAXROMSIZE - size);

  /* get infos from ROM header */
  getrominfo((char *)cart.rom);

  /* detect console region */
  region_autodetect();

  /* Genesis ROM specific */
  if (system_hw != SYSTEM_PBC)
  {
#ifdef LSB_FIRST
    /* Byteswap ROM */
    uint8 temp;
    for(i = 0; i < size; i += 2)
    {
      temp = cart.rom[i];
      cart.rom[i] = cart.rom[i+1];
      cart.rom[i+1] = temp;
    }
#endif

    /* byteswapped RADICA dumps (from Haze) */
    if (((strstr(rominfo.product,"-K0101") != NULL) && (rominfo.checksum == 0xf424)) ||
        ((strstr(rominfo.product,"-K0109") != NULL) && (rominfo.checksum == 0x4f10)))
    {
      uint8 temp;
      for(i = 0; i < size; i += 2)
      {
        temp = cart.rom[i];
        cart.rom[i] = cart.rom[i+1];
        cart.rom[i+1] = temp;
      }
    }

    /* PICO hardware */
    if (strstr(rominfo.consoletype, "SEGA PICO") != NULL)
    {
      system_hw = SYSTEM_PICO;
    }
  }

  return(1);
}

/****************************************************************************
 * region_autodetect
 *
 * Set console region upon ROM header
 *
 ****************************************************************************/
void region_autodetect(void)
{
  if (system_hw == SYSTEM_PBC)
  {
    region_code = sms_cart_region_detect();
  }
  else
  {
    /* country codes used to differentiate region */
    /* 0001 = japan ntsc (1) */
    /* 0010 = japan  pal (2) */
    /* 0100 = usa        (4) */
    /* 1000 = europe     (8) */
    int country = 0;
    int i = 0;
    char c;

    /* from Gens */
    if (!strnicmp(rominfo.country, "eur", 3)) country |= 8;
    else if (!strnicmp(rominfo.country, "usa", 3)) country |= 4;
    else if (!strnicmp(rominfo.country, "jap", 3)) country |= 1;
    else
    {
      /* look for each characters */
      for(i = 0; i < 4; i++)
      {
        c = toupper((int)rominfo.country[i]);

        if (c == 'U') country |= 4;
        else if (c == 'J') country |= 1;
        else if (c == 'E') country |= 8;
        else if (c == 'K') country |= 1;
        else if (c < 16) country |= c;
        else if ((c >= '0') && (c <= '9')) country |= c - '0';
        else if ((c >= 'A') && (c <= 'F')) country |= c - 'A' + 10;
      }
    }

    /* set default console region (USA > JAPAN > EUROPE) */
    if (country & 4) region_code = REGION_USA;
    else if (country & 1) region_code = REGION_JAPAN_NTSC;
    else if (country & 8) region_code = REGION_EUROPE;
    else if (country & 2) region_code = REGION_JAPAN_PAL;
    else region_code = REGION_USA;

    /* some games need specific REGION setting */
    if (((strstr(rominfo.product,"T-45033") != NULL) && (rominfo.checksum == 0x0F81)) || /* Alisia Dragon (Europe) */
         (strstr(rominfo.product,"T-69046-50") != NULL) ||    /* Back to the Future III (Europe) */
         (strstr(rominfo.product,"T-120106-00") != NULL) ||   /* Brian Lara Cricket (Europe) */
         (strstr(rominfo.product,"T-70096 -00") != NULL))     /* Muhammad Ali Heavyweight Boxing (Europe) */
    {
      /* need PAL settings */
      region_code = REGION_EUROPE;
    }
    
    if ((rominfo.realchecksum == 0x532e) && (strstr(rominfo.product,"1011-00") != NULL)) 
    {
      /* On Dal Jang Goon (Korea) needs JAPAN region code */
      region_code = REGION_JAPAN_NTSC;
    }
  }

  /* forced console region */
  if (config.region_detect == 1) region_code = REGION_USA;
  else if (config.region_detect == 2) region_code = REGION_EUROPE;
  else if (config.region_detect == 3) region_code = REGION_JAPAN_NTSC;
  else if (config.region_detect == 4) region_code = REGION_JAPAN_PAL;

  /* PAL/NTSC timings */
  vdp_pal = (region_code & REGION_JAPAN_PAL) >> 6;
}


/****************************************************************************
 * get_company (Softdev - 2006)
 *
 * Try to determine which company made this rom
 *
 * Ok, for some reason there's no standard for this.
 * It seems that there can be pretty much anything you like following the
 * copyright (C) symbol!
 ****************************************************************************/
char *get_company(void)
{
  char *s;
  int i;
  char company[10];

  for (i = 3; i < 8; i++) 
  {
    company[i - 3] = rominfo.copyright[i];
  }
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
    if (company[i] == 32)
      company[i] = 0;

  if (strlen (company) == 0)
    return (char *)companyinfo[MAXCOMPANY - 1].company;

  for (i = 0; i < MAXCOMPANY - 1; i++)
  {
    if (!(strncmp (company, companyinfo[i].companyid, strlen (company))))
      return (char *)companyinfo[i].company;
  }

  return (char *)companyinfo[MAXCOMPANY - 1].company;
}

/****************************************************************************
 * get_peripheral (Softdev - 2006)
 *
 * Return peripheral name based on header code
 *
 ****************************************************************************/
char *get_peripheral(int index)
{
  if (index < MAXPERIPHERALS)
    return (char *)peripheralinfo[index].pName;
  return (char *)companyinfo[MAXCOMPANY - 1].company;
}

