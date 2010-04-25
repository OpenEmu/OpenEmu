/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - pif.c                                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "pif.h"
#include "pif2.h"

#include "r4300/r4300.h"
#include "r4300/interupt.h"

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "api/debugger.h"
#include "main/main.h"
#include "main/rom.h"
#include "plugin/plugin.h"

static unsigned char eeprom[0x800];
static unsigned char mempack[4][0x8000];

//#define DEBUG_PIF
#ifdef DEBUG_PIF
void print_pif(void)
{
   int i;
   for (i=0; i<(64/8); i++)
     DebugMessage(M64MSG_INFO, "%x %x %x %x | %x %x %x %x",
        PIF_RAMb[i*8+0], PIF_RAMb[i*8+1],PIF_RAMb[i*8+2], PIF_RAMb[i*8+3],
        PIF_RAMb[i*8+4], PIF_RAMb[i*8+5],PIF_RAMb[i*8+6], PIF_RAMb[i*8+7]);
   getchar();
}
#endif

static void EepromCommand(unsigned char *Command)
{
    static int EepromFileWarningSent = 0;

   switch (Command[2])
     {
      case 0: // check
    if (Command[1] != 3)
      {
         Command[1] |= 0x40;
         if ((Command[1] & 3) > 0)
           Command[3] = 0;
         if ((Command[1] & 3) > 1)
           Command[4] = ROM_SETTINGS.eeprom_16kb == 0 ? 0x80 : 0xc0;
         if ((Command[1] & 3) > 2)
           Command[5] = 0;
      }
    else
      {
         Command[3] = 0;
         Command[4] = ROM_SETTINGS.eeprom_16kb == 0 ? 0x80 : 0xc0;
         Command[5] = 0;
      }
    break;
      case 4: // read
      {
         char *filename;
         FILE *f;
         filename = (char *) malloc(strlen(get_savespath()) + strlen(ROM_SETTINGS.goodname)+4+1);
         strcpy(filename, get_savespath());
         strcat(filename, ROM_SETTINGS.goodname);
         strcat(filename, ".eep");
         f = fopen(filename, "rb");
         if (f == NULL)
         {
             if (!EepromFileWarningSent)
                 DebugMessage(M64MSG_INFO, "couldn't open eeprom file '%s' for reading", filename);
             EepromFileWarningSent = 1; /* this is to avoid spamming the console the first time a game is run */
             memset(eeprom, 0, 0x800);
         }
         else
         {
             if (fread(eeprom, 1, 0x800, f) != 0x800)
                 DebugMessage(M64MSG_WARNING, "fread() failed for 2kb eeprom file '%s'", filename);
             fclose(f);
         }
         free(filename);
         memcpy(&Command[4], eeprom + Command[3]*8, 8);
      }
    break;
      case 5: // write
      {
         char *filename;
         FILE *f;
         filename = (char *) malloc(strlen(get_savespath()) + strlen(ROM_SETTINGS.goodname)+4+1);
         strcpy(filename, get_savespath());
         strcat(filename, ROM_SETTINGS.goodname);
         strcat(filename, ".eep");
         f = fopen(filename, "rb");
         if (f == NULL)
         {
             if (!EepromFileWarningSent)
                 DebugMessage(M64MSG_INFO, "couldn't open eeprom file '%s' for reading", filename);
             EepromFileWarningSent = 1; /* this is to avoid spamming the console the first time a game is run */
             memset(eeprom, 0, 0x800);
         }
         else
         {
             if (fread(eeprom, 1, 0x800, f) != 0x800)
                 DebugMessage(M64MSG_WARNING, "fread() failed for 2kb eeprom file '%s'", filename);
             fclose(f);
         }
         memcpy(eeprom + Command[3]*8, &Command[4], 8);
         f = fopen(filename, "wb");
         if (f == NULL)
         {
             DebugMessage(M64MSG_WARNING, "couldn't open eeprom file '%s' for writing", filename);
         }
         else
         {
             if (fwrite(eeprom, 1, 0x800, f) != 0x800)
                 DebugMessage(M64MSG_WARNING, "fread() failed for 2kb eeprom file '%s'", filename);
             fclose(f);
         }
         free(filename);
      }
    break;
      default:
    DebugMessage(M64MSG_ERROR, "unknown command in EepromCommand(): %x", Command[2]);
     }
}

static void format_mempacks(void)
{
   unsigned char init[] =
     {
    0x81,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x0c,0x0d,0x0e,0x0f,
    0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b, 0x1c,0x1d,0x1e,0x1f,
    0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
    0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0xff, 0x05,0x1a,0x5f,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff, 0xff,0xff,0x01,0xff, 0x66,0x25,0x99,0xcd,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
    0x00,0x71,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03
     };
   int i,j;
   for (i=0; i<4; i++)
     {
    for (j=0; j<0x8000; j+=2)
      {
         mempack[i][j] = 0;
         mempack[i][j+1] = 0x03;
      }
    memcpy(mempack[i], init, 272);
     }
}

static unsigned char mempack_crc(unsigned char *data)
{
   int i;
   unsigned char CRC = 0;
   for (i=0; i<=0x20; i++)
     {
    int mask;
    for (mask = 0x80; mask >= 1; mask >>= 1)
      {
         int xor_tap = (CRC & 0x80) ? 0x85 : 0x00;
         CRC <<= 1;
         if (i != 0x20 && (data[i] & mask)) CRC |= 1;
         CRC ^= xor_tap;
      }
     }
   return CRC;
}

static void internal_ReadController(int Control, unsigned char *Command)
{
   switch (Command[2])
     {
      case 1:
    if (Controls[Control].Present)
      {
         BUTTONS Keys;
         getKeys(Control, &Keys);
         *((unsigned int *)(Command + 3)) = Keys.Value;
#ifdef COMPARE_CORE
         CoreCompareDataSync(4, Command+3);
#endif
      }
    break;
      case 2: // read controller pack
    if (Controls[Control].Present)
      {
         if (Controls[Control].Plugin == PLUGIN_RAW)
           if (controllerCommand) readController(Control, Command);
      }
    break;
      case 3: // write controller pack
    if (Controls[Control].Present)
      {
         if (Controls[Control].Plugin == PLUGIN_RAW)
           if (controllerCommand) readController(Control, Command);
      }
    break;
     }
}

static void internal_ControllerCommand(int Control, unsigned char *Command)
{
    int MpkFileWarningSent = 0;

   switch (Command[2])
     {
      case 0x00: // check
      case 0xFF:
    if ((Command[1] & 0x80))
      break;
    if (Controls[Control].Present)
      {
         Command[3] = 0x05;
         Command[4] = 0x00;
         switch(Controls[Control].Plugin)
           {
        case PLUGIN_MEMPAK:
          Command[5] = 1;
          break;
        case PLUGIN_RAW:
          Command[5] = 1;
          break;
        default:
          Command[5] = 0;
          break;
           }
      }
    else
      Command[1] |= 0x80;
    break;
      case 0x01:
    if (!Controls[Control].Present)
      Command[1] |= 0x80;
    break;
      case 0x02: // read controller pack
    if (Controls[Control].Present)
      {
         switch(Controls[Control].Plugin)
           {
        case PLUGIN_MEMPAK:
            {
               int address = (Command[3] << 8) | Command[4];
               if (address == 0x8001)
             {
                memset(&Command[5], 0, 0x20);
                Command[0x25] = mempack_crc(&Command[5]);
             }
               else
             {
                address &= 0xFFE0;
                if (address <= 0x7FE0)
                  {
                 char *filename;
                 FILE *f;
                 filename = (char *) malloc(strlen(get_savespath()) + strlen(ROM_SETTINGS.goodname)+4+1);
                 strcpy(filename, get_savespath());
                 strcat(filename, ROM_SETTINGS.goodname);
                 strcat(filename, ".mpk");
                 f = fopen(filename, "rb");
                 if (f == NULL)
                 {
                     if (!MpkFileWarningSent)
                         DebugMessage(M64MSG_INFO, "couldn't open memory pack file '%s' for reading", filename);
                     MpkFileWarningSent = 1;
                     format_mempacks();
                 }
                 else
                 {
                     if (fread(mempack[0], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fread() failed for 1st 32kb mempack in file '%s'", filename);
                     else if (fread(mempack[1], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fread() failed for 2nd 32kb mempack in file '%s'", filename);
                     else if (fread(mempack[2], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fread() failed for 3rd 32kb mempack in file '%s'", filename);
                     else if (fread(mempack[3], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fread() failed for 4th 32kb mempack in file '%s'", filename);
                     fclose(f);
                 }
                 free(filename);
                 memcpy(&Command[5], &mempack[Control][address], 0x20);
                  }
                else
                  {
                 memset(&Command[5], 0, 0x20);
                  }
                Command[0x25] = mempack_crc(&Command[5]);
             }
            }
          break;
        case PLUGIN_RAW:
          if (controllerCommand) controllerCommand(Control, Command);
          break;
        default:
          memset(&Command[5], 0, 0x20);
          Command[0x25] = 0;
           }
      }
    else
      Command[1] |= 0x80;
    break;
      case 0x03: // write controller pack
    if (Controls[Control].Present)
      {
         switch(Controls[Control].Plugin)
           {
        case PLUGIN_MEMPAK:
            {
               int address = (Command[3] << 8) | Command[4];
               if (address == 0x8001) 
             Command[0x25] = mempack_crc(&Command[5]);
               else
             {
                address &= 0xFFE0;
                if (address <= 0x7FE0)
                  {
                 char *filename;
                 FILE *f;
                 filename = (char *) malloc(strlen(get_savespath()) + strlen(ROM_SETTINGS.goodname)+4+1);
                 strcpy(filename, get_savespath());
                 strcat(filename, ROM_SETTINGS.goodname);
                 strcat(filename, ".mpk");
                 f = fopen(filename, "rb");
                 if (f == NULL)
                 {
                     if (!MpkFileWarningSent)
                         DebugMessage(M64MSG_INFO, "couldn't open memory pack file '%s' for reading", filename);
                     MpkFileWarningSent = 1;
                     format_mempacks();
                 }
                 else
                 {
                     if (fread(mempack[0], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fread() failed for 1st 32kb mempack in file '%s'", filename);
                     else if (fread(mempack[1], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fread() failed for 2nd 32kb mempack in file '%s'", filename);
                     else if (fread(mempack[2], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fread() failed for 3rd 32kb mempack in file '%s'", filename);
                     else if (fread(mempack[3], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fread() failed for 4th 32kb mempack in file '%s'", filename);
                     fclose(f);
                 }
                 memcpy(&mempack[Control][address], &Command[5], 0x20);
                 f = fopen(filename, "wb");
                 if (f == NULL)
                 {
                     DebugMessage(M64MSG_WARNING, "couldn't open memory pack file '%s' for writing", filename);
                 }
                 else
                 {
                     if (fwrite(mempack[0], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fwrite() failed for 1st 32kb memory pack in file '%s'", filename);
                     else if (fwrite(mempack[1], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fwrite() failed for 2nd 32kb memory pack in file '%s'", filename);
                     else if (fwrite(mempack[2], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fwrite() failed for 3rd 32kb memory pack in file '%s'", filename);
                     else if (fwrite(mempack[3], 1, 0x8000, f) != 0x8000)
                         DebugMessage(M64MSG_WARNING, "fwrite() failed for 4th 32kb memory pack in file '%s'", filename);
                     fclose(f);
                 }
                 free(filename);
                  }
                Command[0x25] = mempack_crc(&Command[5]);
             }
            }
          break;
        case PLUGIN_RAW:
          if (controllerCommand) controllerCommand(Control, Command);
          break;
        default:
          Command[0x25] = mempack_crc(&Command[5]);
           }
      }
    else
      Command[1] |= 0x80;
    break;
     }
}

void update_pif_write(void)
{
   int i=0, channel=0;
#ifdef DEBUG_PIF
   DebugMessage(M64MSG_INFO, "update_pif_write()");
   print_pif();
#endif
   if (PIF_RAMb[0x3F] > 1)
     {
    switch (PIF_RAMb[0x3F])
      {
       case 0x02:
         for (i=0; i<sizeof(pif2_lut)/32; i++)
           {
          if (!memcmp(PIF_RAMb + 64-2*8, pif2_lut[i][0], 16))
            {
               memcpy(PIF_RAMb + 64-2*8, pif2_lut[i][1], 16);
               return;
            }
           }
         DebugMessage(M64MSG_ERROR, "update_pif_write(): unknown pif2 code:");
         for (i=(64-2*8)/8; i<(64/8); i++)
           DebugMessage(M64MSG_ERROR, "%x %x %x %x | %x %x %x %x",
              PIF_RAMb[i*8+0], PIF_RAMb[i*8+1],PIF_RAMb[i*8+2], PIF_RAMb[i*8+3],
              PIF_RAMb[i*8+4], PIF_RAMb[i*8+5],PIF_RAMb[i*8+6], PIF_RAMb[i*8+7]);
         break;
       case 0x08:
         PIF_RAMb[0x3F] = 0;
         break;
       default:
         DebugMessage(M64MSG_ERROR, "error in update_pif_write(): %x", PIF_RAMb[0x3F]);
      }
    return;
     }
   while (i<0x40)
     {
    switch(PIF_RAMb[i])
      {
       case 0x00:
         channel++;
         if (channel > 6) i=0x40;
         break;
       case 0xFF:
         break;
       default:
         if (!(PIF_RAMb[i] & 0xC0))
           {
          if (channel < 4)
            {
               if (Controls[channel].Present && 
               Controls[channel].RawData)
             controllerCommand(channel, &PIF_RAMb[i]);
               else
             internal_ControllerCommand(channel, &PIF_RAMb[i]);
            }
          else if (channel == 4)
            EepromCommand(&PIF_RAMb[i]);
          else
            DebugMessage(M64MSG_ERROR, "channel >= 4 in update_pif_write");
          i += PIF_RAMb[i] + (PIF_RAMb[(i+1)] & 0x3F) + 1;
          channel++;
           }
         else
           i=0x40;
      }
    i++;
     }
   //PIF_RAMb[0x3F] = 0;
   controllerCommand(-1, NULL);
#ifdef DEBUG_PIF
   print_pif();
#endif
}

void update_pif_read(void)
{
   int i=0, channel=0;
#ifdef DEBUG_PIF
   DebugMessage(M64MSG_INFO, "update_pif_read()");
   print_pif();
#endif
   while (i<0x40)
     {
    switch(PIF_RAMb[i])
      {
       case 0x00:
         channel++;
         if (channel > 6) i=0x40;
         break;
       case 0xFE:
         i = 0x40;
         break;
       case 0xFF:
         break;
       case 0xB4:
       case 0x56:
       case 0xB8:
         break;
       default:
         if (!(PIF_RAMb[i] & 0xC0))
           {
          if (channel < 4)
            {
               if (Controls[channel].Present && 
               Controls[channel].RawData)
             readController(channel, &PIF_RAMb[i]);
               else
             internal_ReadController(channel, &PIF_RAMb[i]);
            }
          i += PIF_RAMb[i] + (PIF_RAMb[(i+1)] & 0x3F) + 1;
          channel++;
           }
         else
           i=0x40;
      }
    i++;
     }
   readController(-1, NULL);
#ifdef DEBUG_PIF
   print_pif();
#endif
}

