/***************************************************************************************
 *  Genesis Plus
 *  ROM Loading Support
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Copyright (C) 2007-2012  Eke-Eke (Genesis Plus GX)
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
#define MAXPERIPHERALS 15

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
uint8 romtype;

static uint8 rom_region;

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
  {"G", "Menacer"},
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
 *
 * Pass a pointer to the ROM base address.
 ***************************************************************************/
void getrominfo(char *romheader)
{
  /* Clear ROM info structure */
  memset (&rominfo, 0, sizeof (ROMINFO));

  /* Genesis ROM header support */
  if (system_hw & SYSTEM_MD)
  {
    int i,j;

    memcpy (&rominfo.consoletype, romheader + ROMCONSOLE, 16);
    memcpy (&rominfo.copyright, romheader + ROMCOPYRIGHT, 16);

    /* Domestic (japanese) name */
    rominfo.domestic[0] = romheader[ROMDOMESTIC];
    j = 1;
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
  else
  {
    uint16 offset = 0;

    /* detect Master System ROM header */
    if (!memcmp (&romheader[0x1ff0], "TMR SEGA", 8))
    {
      offset = 0x1ff0;
    }
    else if (!memcmp (&romheader[0x3ff0], "TMR SEGA", 8))
    {
      offset = 0x3ff0;
    }
    else if (!memcmp (&romheader[0x7ff0], "TMR SEGA", 8))
    {
      offset = 0x7ff0;
    }

    /* if found, get infos from header */
    if (offset)
    {
      /* checksum */
      rominfo.checksum = romheader[offset + 0x0a] | (romheader[offset + 0x0b] << 8);

      /* product code & version */
      sprintf(&rominfo.product[0], "%02d", romheader[offset + 0x0e] >> 4);
      sprintf(&rominfo.product[2], "%02x", romheader[offset + 0x0d]);
      sprintf(&rominfo.product[4], "%02x", romheader[offset + 0x0c]);
      sprintf(&rominfo.product[6], "-%d", romheader[offset + 0x0e] & 0x0F);

      /* region code */
      switch (romheader[offset + 0x0f] >> 4)
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
          sprintf(rominfo.country,"Unknown (%d)", romheader[offset + 0x0f] >> 4);
          break;
      }

      /* ROM size */
      rominfo.romstart = 0;
      switch (romheader[offset + 0x0f] & 0x0F)
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
  }
}

/***************************************************************************
 * load_bios
 *
 * Load current system BIOS file.
 *
 * Return loaded size (-1 if already loaded)
 *
 ***************************************************************************/
int load_bios(void)
{
  int size = 0;

  switch (system_hw)
  {
    case SYSTEM_MCD:
    {
      /* check if CD BOOTROM is already loaded */
      if (!(system_bios & 0x10) || ((system_bios & 0x0c) != (region_code >> 4)))
      {
        /* load CD BOOTROM (fixed 128KB size) */
        switch (region_code)
        {
          case REGION_USA:
            size = load_archive(CD_BIOS_US, scd.bootrom, sizeof(scd.bootrom), 0);
            break;
          case REGION_EUROPE:
            size = load_archive(CD_BIOS_EU, scd.bootrom, sizeof(scd.bootrom), 0);
            break;
          default:
            size = load_archive(CD_BIOS_JP, scd.bootrom, sizeof(scd.bootrom), 0);
            break;
        }

        /* CD BOOTROM loaded ? */
        if (size > 0)
        {
#ifdef LSB_FIRST
          /* Byteswap ROM to optimize 16-bit access */
          int i;
          for (i = 0; i < size; i += 2)
          {
            uint8 temp = scd.bootrom[i];
            scd.bootrom[i] = scd.bootrom[i+1];
            scd.bootrom[i+1] = temp;
          }
#endif
          /* mark CD BIOS as being loaded */
          system_bios = system_bios | 0x10;

          /* loaded BIOS region */
          system_bios = (system_bios & 0xf0) | (region_code >> 4);
        }

        return size;
      }
      
      return -1;
    }

    case SYSTEM_GG:
    case SYSTEM_GGMS:
    {
      /* check if Game Gear BOOTROM is already loaded */
      if (!(system_bios & SYSTEM_GG))
      {      
        /* mark both Master System & Game Gear BOOTROM as unloaded */
        system_bios &= ~(SYSTEM_SMS | SYSTEM_GG);

        /* BOOTROM is stored above cartridge ROM area (max. 4MB) */
        if (cart.romsize <= 0x400000)
        {
          /* load Game Gear BOOTROM file */
          size = load_archive(GG_BIOS, cart.rom + 0x400000, 0x100000, 0);

          if (size > 0)
          {
            /* mark Game Gear BOOTROM as loaded */
            system_bios |= SYSTEM_GG;
          }
        }

        return size;
      }
      
      return -1;
    }

    case SYSTEM_SMS:
    case SYSTEM_SMS2:
    {
      /* check if Master System BOOTROM is already loaded */
      if (!(system_bios & SYSTEM_SMS) || ((system_bios & 0x0c) != (region_code >> 4)))
      {      
        /* mark both Master System & Game Gear BOOTROM as unloaded */
        system_bios &= ~(SYSTEM_SMS | SYSTEM_GG);

        /* BOOTROM is stored above cartridge ROM area (max. 4MB) */
        if (cart.romsize <= 0x400000)
        {
          /* load Master System BOOTROM file */
          switch (region_code)
          {
            case REGION_USA:
              size = load_archive(MS_BIOS_US, cart.rom + 0x400000, 0x400000, 0);
              break;
            case REGION_EUROPE:
              size = load_archive(MS_BIOS_EU, cart.rom + 0x400000, 0x400000, 0);
              break;
            default:
              size = load_archive(MS_BIOS_JP, cart.rom + 0x400000, 0x400000, 0);
              break;
          }

          if (size > 0)
          {
            /* mark Master System BOOTROM as loaded */
            system_bios |= SYSTEM_SMS;

            /* loaded BOOTROM region */
            system_bios = (system_bios & 0xf0) | (region_code >> 4);
          }
        }

        return size;
      }
      
      return -1;
    }

    default:
    {
      return 0;
    }
  }
}

/***************************************************************************
 * load_rom
 *
 * Load a new ROM file.
 *
 * Return 0 on error, 1 on success
 *
 ***************************************************************************/
int load_rom(char *filename)
{
  int i, size;

  /* clear any existing patches */
  ggenie_shutdown();
  areplay_shutdown();

  /* check previous loaded ROM size */
  if (cart.romsize > 0x800000)
  {
    /* assume no CD is currently loaded */
    cdd.loaded = 0;
  }

  /* auto-detect CD image files */
  size = cdd_load(filename, (char *)(cart.rom));
  if (size < 0)
  {
    /* error opening file */
    return 0;
  }

  if (size > 0)
  {
    /* CD image file loaded */
    system_hw = SYSTEM_MCD;
  }
  else
  {
    /* load file into ROM buffer */
    char extension[4];
    size = load_archive(filename, cart.rom, sizeof(cart.rom), extension);
    if (!size)
    {
      /* mark all BOOTROM as unloaded since they could have been overwritten */
      system_bios &= ~(0x10 | SYSTEM_SMS | SYSTEM_GG);
      return 0;
    }

    /* mark BOOTROM as unloaded if they have been overwritten by cartridge ROM */
    if (size > 0x800000)
    {
      /* CD BIOS ROM are loaded at the start of CD area */
      system_bios &= ~0x10;
    }
    else if (size > 0x400000)
    {
      /* Master System or Game Gear BIOS ROM are loaded within $400000-$4FFFFF area */
      system_bios &= ~(SYSTEM_SMS | SYSTEM_GG);
    }

    /* convert lower case to upper case */
    *(uint32 *)(extension) &= 0xdfdfdfdf;

    /* auto-detect system hardware from ROM file extension */
    if (!memcmp("SMS", &extension[0], 3))
    {
      /* Master System II hardware */
      system_hw = SYSTEM_SMS2;
    }
    else if (!memcmp("GG", &extension[1], 2))
    {
      /* Game Gear hardware (GG mode) */
      system_hw = SYSTEM_GG;
    }
    else if (!memcmp("SG", &extension[1], 2))
    {
      /* SG-1000 hardware */
      system_hw = SYSTEM_SG;
    }
    else
    {
      /* Mega Drive hardware (Genesis mode) */
      system_hw = SYSTEM_MD;

      /* Decode .MDX format */
      if (!memcmp("MDX", &extension[0], 3))
      {
        for (i = 4; i < size - 1; i++)
        {
          cart.rom[i-4] = cart.rom[i] ^ 0x40;
        }
        size = size - 5;
      }
    }

    /* auto-detect 512 byte extra header */
    if (memcmp((char *)(cart.rom + 0x100), "SEGA", 4) && ((size / 512) & 1) && !(size % 512))
    {
      /* remove header */
      size -= 512;
      memcpy (cart.rom, cart.rom + 512, size);

      /* assume interleaved Genesis ROM format (.smd) */
      if (system_hw == SYSTEM_MD)
      {
        for (i = 0; i < (size / 0x4000); i++)
        {
          deinterleave_block (cart.rom + (i * 0x4000));
        }
      }
    }
  }
    
  /* initialize ROM size */
  cart.romsize = size;

  /* get infos from ROM header */
  getrominfo((char *)(cart.rom));

  /* set console region */
  get_region((char *)(cart.rom));

  /* CD image file */
  if (system_hw == SYSTEM_MCD)
  {   
    /* load CD BOOT ROM */
    if (!load_bios())
    {
      /* unmount CD image */
      cdd_unload();

      /* error loading CD BOOT ROM */
      return (0);
    }

    /* boot from CD */
    scd.cartridge.boot = 0x00;
  }

  /* 16-bit ROM specific */
  else if (system_hw == SYSTEM_MD)
  {
#ifdef LSB_FIRST
    /* Byteswap ROM to optimize 16-bit access */
    for (i = 0; i < cart.romsize; i += 2)
    {
      uint8 temp = cart.rom[i];
      cart.rom[i] = cart.rom[i+1];
      cart.rom[i+1] = temp;
    }
#endif

    /* byteswapped RADICA dumps (from Haze) */
    if (((strstr(rominfo.product,"-K0101") != NULL) && (rominfo.checksum == 0xf424)) ||
        ((strstr(rominfo.product,"-K0109") != NULL) && (rominfo.checksum == 0x4f10)))
    {
      for(i = 0; i < cart.romsize; i += 2)
      {
        uint8 temp = cart.rom[i];
        cart.rom[i] = cart.rom[i+1];
        cart.rom[i+1] = temp;
      }
    }
  }

  /* Save auto-detected system hardware  */
  romtype = system_hw;

  /* PICO ROM */
  if (strstr(rominfo.consoletype, "SEGA PICO") != NULL)
  {
    /* PICO hardware */
    system_hw = SYSTEM_PICO;
  }

  /* CD BOOTROM */
  else if (strstr(rominfo.ROMType, "BR") != NULL)
  {
    /* enable CD hardware */
    system_hw = SYSTEM_MCD;

    /* boot from CD */
    scd.cartridge.boot = 0x00;

    /* copy ROM to BOOTROM area */
    memcpy(scd.bootrom, cart.rom, sizeof(scd.bootrom));

    /* mark CD BIOS as being loaded */
    system_bios = system_bios | 0x10;

    /* loaded CD BIOS region */
    system_bios = (system_bios & 0xf0) | (region_code >> 4);
  }

  /* ROM cartridges with CD support */
  else if ((strstr(rominfo.domestic,"FLUX") != NULL) ||
           (strstr(rominfo.domestic,"WONDER LIBRARY") != NULL) ||
           (strstr(rominfo.product,"T-5740") != NULL))
  {
    /* check if console hardware is set to AUTO */
    if (config.system == 0x00)
    {
      /* auto-enable CD hardware */
      system_hw = SYSTEM_MCD;

      /* try to load CD BOOTROM */
      if (load_bios())
      {
        /* boot from cartridge */
        scd.cartridge.boot = 0x40;

        /* automatically load associated .iso image */
        strncpy(&filename[strlen(filename) - 4], ".iso", 4);
        cdd_load(filename, (char *)cdc.ram);
      }
      else
      {
        /* if not found, disable CD hardware */
        system_hw = SYSTEM_MD;
      }
    }
  }

  /* Force system hardware if requested */
  if (config.system == SYSTEM_MD)
  {
    if (!(system_hw & SYSTEM_MD))
    {
      /* Mega Drive in MS compatibility mode  */
      system_hw = SYSTEM_PBC;
    }
  }
  else if (config.system == SYSTEM_GG)
  {
    if (system_hw != SYSTEM_GG)
    {
      /* Game Gear in MS compatibility mode  */
      system_hw = SYSTEM_GGMS;
    }
  }
  else if (config.system)
  {
    system_hw = config.system;
  }

  /* restore previous input settings */
  if (old_system[0] != -1)
  {
    input.system[0] = old_system[0];
  }
  if (old_system[1] != -1)
  {
    input.system[1] = old_system[1];
  }

  /* default gun settings */
  input.x_offset = (input.system[1] == SYSTEM_MENACER) ? 64 : 0;
  input.y_offset = 0;

  /* autodetect gun support */
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

    /* force MENACER configuration */
    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 82;
    input.y_offset = 0;
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

    /* force MENACER configuration */
    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 133;
    input.y_offset = -8;
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

    /* force MOUSE+MENACER configuration */
    input.system[0] = SYSTEM_MOUSE;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 68;
    input.y_offset = -24;
  }
  else if (strstr(rominfo.international,"CORPSE KILLER") != NULL)
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

    /* force MENACER configuration */
    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 64;
    input.y_offset = -8;
  }
  else if (strstr(rominfo.international,"CRIME PATROL") != NULL)
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

    /* force MENACER configuration */
    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 61;
    input.y_offset = 0;
  }
  else if (strstr(rominfo.international,"MAD DOG II THE LOST GOLD") != NULL)
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

    /* force MENACER configuration */
    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 70;
    input.y_offset = 18;
  }
  else if (strstr(rominfo.international,"MAD DOG MCCREE") != NULL)
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

    /* force MENACER configuration */
    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 49;
    input.y_offset = 0;
  }
  else if (strstr(rominfo.international,"WHO SHOT JOHNNY ROCK?") != NULL)
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

    /* force MENACER configuration */
    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_MENACER;
    input.x_offset = 60;
    input.y_offset = 30;
  }
  else if ((strstr(rominfo.international,"LETHAL ENFORCERS") != NULL) ||
           (strstr(rominfo.international,"SNATCHER") != NULL))
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

    /* force JUSTIFIER configuration */
    input.system[0] = SYSTEM_MD_GAMEPAD;
    input.system[1] = SYSTEM_JUSTIFIER;
    input.x_offset = (strstr(rominfo.international,"GUN FIGHTERS") != NULL) ? 24 : 0;
    input.y_offset = 0;
  }

  return(1);
}

/****************************************************************************
 * get_region
 *
 * Set console region from ROM header passed as parameter or 
 * from previous auto-detection (if NULL) 
 *
 ****************************************************************************/
void get_region(char *romheader)
{
  /* region auto-detection ? */
  if (romheader)
  {
    /* Mega CD image */
    if (system_hw == SYSTEM_MCD)
    {
      /* security code */
      switch (romheader[0x20b])
      {
        case 0x7a:
          region_code = REGION_USA;
          break;

        case 0x64:
          region_code = REGION_EUROPE;
          break;
   
        default:
          region_code = REGION_JAPAN_NTSC;
          break;
      }
    }

    /* 16-bit cartridge */
    else if (system_hw & SYSTEM_MD)
    {
      /* country codes used to differentiate region */
      /* 0001 = japan ntsc (1) */
      /* 0010 = japan pal  (2) -> does not exist ? */
      /* 0100 = usa        (4) */
      /* 1000 = europe     (8) */
      int country = 0;

      /* from Gens */
      if (!memcmp(rominfo.country, "eur", 3)) country |= 8;
      else if (!memcmp(rominfo.country, "EUR", 3)) country |= 8;
      else if (!memcmp(rominfo.country, "jap", 3)) country |= 1;
      else if (!memcmp(rominfo.country, "JAP", 3)) country |= 1;
      else if (!memcmp(rominfo.country, "usa", 3)) country |= 4;
      else if (!memcmp(rominfo.country, "USA", 3)) country |= 4;
      else
      {
        int i;
        char c;

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

      /* some games need specific region settings but have wrong header*/
      if (((strstr(rominfo.product,"T-45033") != NULL) && (rominfo.checksum == 0x0F81)) || /* Alisia Dragon (Europe) */
           (strstr(rominfo.product,"T-69046-50") != NULL) ||    /* Back to the Future III (Europe) */
           (strstr(rominfo.product,"T-120106-00") != NULL) ||   /* Brian Lara Cricket (Europe) */
           (strstr(rominfo.product,"T-70096 -00") != NULL))     /* Muhammad Ali Heavyweight Boxing (Europe) */
      {
        /* need PAL settings */
        region_code = REGION_EUROPE;
      }
      else if ((rominfo.realchecksum == 0x532e) && (strstr(rominfo.product,"1011-00") != NULL)) 
      {
        /* On Dal Jang Goon (Korea) needs JAPAN region code */
        region_code = REGION_JAPAN_NTSC;
      }
    }

    /* 8-bit cartridge */
    else
    {
      region_code = sms_cart_region_detect();
    }

    /* save auto-detected region */
    rom_region = region_code;
  }
  else
  {
    /* restore auto-detected region */
    region_code = rom_region;
  }
  
  /* force console region if requested */
  if (config.region_detect == 1) region_code = REGION_USA;
  else if (config.region_detect == 2) region_code = REGION_EUROPE;
  else if (config.region_detect == 3) region_code = REGION_JAPAN_NTSC;
  else if (config.region_detect == 4) region_code = REGION_JAPAN_PAL;

  /* autodetect PAL/NTSC timings */
  vdp_pal = (region_code >> 6) & 0x01;

  /* autodetect PAL/NTSC master clock */
  system_clock = vdp_pal ? MCLOCK_PAL : MCLOCK_NTSC;

  /* force PAL/NTSC timings if requested */
  if (config.vdp_mode == 1) vdp_pal = 0;
  else if (config.vdp_mode == 2) vdp_pal = 1;

  /* force PAL/NTSC master clock if requested */
  if (config.master_clock == 1) system_clock = MCLOCK_NTSC;
  else if (config.master_clock == 2) system_clock = MCLOCK_PAL;
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

