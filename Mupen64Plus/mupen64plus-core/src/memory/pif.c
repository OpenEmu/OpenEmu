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
#include <time.h>

#include "memory.h"
#include "pif.h"
#include "n64_cic_nus_6105.h"

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
}
#endif

static unsigned char byte2bcd(int n)
{
    n %= 100;
    return ((n / 10) << 4) | (n % 10);
}

static void EepromCommand(unsigned char *Command)
{
    time_t curtime_time;
    struct tm curtime;

    switch (Command[2])
    {
    case 0: // check
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "EepromCommand() check size");
#endif
        if (Command[1] != 3)
        {
            Command[1] |= 0x40;
            if ((Command[1] & 3) > 0)
                Command[3] = 0;
            if ((Command[1] & 3) > 1)
                Command[4] = (ROM_SETTINGS.savetype != EEPROM_16KB) ? 0x80 : 0xc0;
            if ((Command[1] & 3) > 2)
                Command[5] = 0;
        }
        else
        {
            Command[3] = 0;
            Command[4] = (ROM_SETTINGS.savetype != EEPROM_16KB) ? 0x80 : 0xc0;
            Command[5] = 0;
        }
        break;
    case 4: // read
    {
        char *filename;
        FILE *f;
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "EepromCommand() read 8-byte block %i", Command[3]);
#endif
        filename = (char *) malloc(strlen(get_savesrampath()) + strlen(ROM_SETTINGS.goodname)+4+1);
        strcpy(filename, get_savesrampath());
        strcat(filename, ROM_SETTINGS.goodname);
        strcat(filename, ".eep");
        f = fopen(filename, "rb");
        if (f == NULL)
        {
            DebugMessage(M64MSG_VERBOSE, "couldn't open eeprom file '%s' for reading", filename);
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
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "EepromCommand() write 8-byte block %i", Command[3]);
#endif
        filename = (char *) malloc(strlen(get_savesrampath()) + strlen(ROM_SETTINGS.goodname)+4+1);
        strcpy(filename, get_savesrampath());
        strcat(filename, ROM_SETTINGS.goodname);
        strcat(filename, ".eep");
        f = fopen(filename, "rb");
        if (f == NULL)
        {
            DebugMessage(M64MSG_VERBOSE, "couldn't open eeprom file '%s' for reading", filename);
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
    case 6:
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "EepromCommand() RTC status query");
#endif
        // RTCstatus query
        Command[3] = 0x00;
        Command[4] = 0x10;
        Command[5] = 0x00;
        break;
    case 7:
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "EepromCommand() read RTC block %i", Command[3]);
#endif
        // read RTC block
        switch (Command[3]) {	// block number
        case 0:
            Command[4] = 0x00;
            Command[5] = 0x02;
            Command[12] = 0x00;
            break;
        case 1:
            DebugMessage(M64MSG_ERROR, "RTC command in EepromCommand(): read block %d", Command[2]);
            break;
        case 2:
            time(&curtime_time);
#if defined(WIN32)
            localtime_s(&curtime, &curtime_time);
#else
            localtime_r(&curtime_time, &curtime);
#endif
            Command[4] = byte2bcd(curtime.tm_sec);
            Command[5] = byte2bcd(curtime.tm_min);
            Command[6] = 0x80 + byte2bcd(curtime.tm_hour);
            Command[7] = byte2bcd(curtime.tm_mday);
            Command[8] = byte2bcd(curtime.tm_wday);
            Command[9] = byte2bcd(curtime.tm_mon + 1);
            Command[10] = byte2bcd(curtime.tm_year);
            Command[11] = byte2bcd(curtime.tm_year / 100);
            Command[12] = 0x00;	// status
            break;
        }
        break;
    case 8:
        // write RTC block
        DebugMessage(M64MSG_ERROR, "RTC write in EepromCommand(): %d not yet implemented", Command[2]);
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
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "internal_ReadController() Channel %i Command 1 read buttons", Control);
#endif
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
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "internal_ReadController() Channel %i Command 2 read controller pack (in Input plugin)", Control);
#endif
        if (Controls[Control].Present)
        {
            if (Controls[Control].Plugin == PLUGIN_RAW)
                if (controllerCommand) readController(Control, Command);
        }
        break;
    case 3: // write controller pack
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "internal_ReadController() Channel %i Command 3 write controller pack (in Input plugin)", Control);
#endif
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

    switch (Command[2])
    {
    case 0x00: // read status
    case 0xFF: // reset
        if ((Command[1] & 0x80))
            break;
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "internal_ControllerCommand() Channel %i Command %02x check pack present", Control, Command[2]);
#endif
        if (Controls[Control].Present)
        {
            Command[3] = 0x05;
            Command[4] = 0x00;
            switch (Controls[Control].Plugin)
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
#ifdef DEBUG_PIF
        DebugMessage(M64MSG_INFO, "internal_ControllerCommand() Channel %i Command 1 check controller present", Control);
#endif
        if (!Controls[Control].Present)
            Command[1] |= 0x80;
        break;
    case 0x02: // read controller pack
        if (Controls[Control].Present)
        {
            switch (Controls[Control].Plugin)
            {
            case PLUGIN_MEMPAK:
            {
                int address = (Command[3] << 8) | Command[4];
#ifdef DEBUG_PIF
                DebugMessage(M64MSG_INFO, "internal_ControllerCommand() Channel %i Command 2 read mempack address %04x", Control, address);
#endif
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
                        filename = (char *) malloc(strlen(get_savesrampath()) + strlen(ROM_SETTINGS.goodname)+4+1);
                        strcpy(filename, get_savesrampath());
                        strcat(filename, ROM_SETTINGS.goodname);
                        strcat(filename, ".mpk");
                        f = fopen(filename, "rb");
                        if (f == NULL)
                        {
                            DebugMessage(M64MSG_VERBOSE, "couldn't open memory pack file '%s' for reading", filename);
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
#ifdef DEBUG_PIF
                DebugMessage(M64MSG_INFO, "internal_ControllerCommand() Channel %i Command 2 controllerCommand (in Input plugin)", Control);
#endif
                if (controllerCommand) controllerCommand(Control, Command);
                break;
            default:
#ifdef DEBUG_PIF
                DebugMessage(M64MSG_INFO, "internal_ControllerCommand() Channel %i Command 2 (no pack plugged in)", Control);
#endif
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
            switch (Controls[Control].Plugin)
            {
            case PLUGIN_MEMPAK:
            {
                int address = (Command[3] << 8) | Command[4];
#ifdef DEBUG_PIF
                DebugMessage(M64MSG_INFO, "internal_ControllerCommand() Channel %i Command 3 write mempack address %04x", Control, address);
#endif
                if (address == 0x8001)
                    Command[0x25] = mempack_crc(&Command[5]);
                else
                {
                    address &= 0xFFE0;
                    if (address <= 0x7FE0)
                    {
                        char *filename;
                        FILE *f;
                        filename = (char *) malloc(strlen(get_savesrampath()) + strlen(ROM_SETTINGS.goodname)+4+1);
                        strcpy(filename, get_savesrampath());
                        strcat(filename, ROM_SETTINGS.goodname);
                        strcat(filename, ".mpk");
                        f = fopen(filename, "rb");
                        if (f == NULL)
                        {
                            DebugMessage(M64MSG_VERBOSE, "couldn't open memory pack file '%s' for reading", filename);
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
#ifdef DEBUG_PIF
                DebugMessage(M64MSG_INFO, "internal_ControllerCommand() Channel %i Command 3 controllerCommand (in Input plugin)", Control);
#endif
                if (controllerCommand) controllerCommand(Control, Command);
                break;
            default:
#ifdef DEBUG_PIF
                DebugMessage(M64MSG_INFO, "internal_ControllerCommand() Channel %i Command 3 (no pack plugged in)", Control);
#endif
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
    char challenge[30], response[30];
    int i=0, channel=0;
    if (PIF_RAMb[0x3F] > 1)
    {
        switch (PIF_RAMb[0x3F])
        {
        case 0x02:
#ifdef DEBUG_PIF
            DebugMessage(M64MSG_INFO, "update_pif_write() pif_ram[0x3f] = 2 - CIC challenge");
#endif
            // format the 'challenge' message into 30 nibbles for X-Scale's CIC code
            for (i = 0; i < 15; i++)
            {
                challenge[i*2] =   (PIF_RAMb[48+i] >> 4) & 0x0f;
                challenge[i*2+1] =  PIF_RAMb[48+i]       & 0x0f;
            }
            // calculate the proper response for the given challenge (X-Scale's algorithm)
            n64_cic_nus_6105(challenge, response, CHL_LEN - 2);
            // re-format the 'response' into a byte stream
            for (i = 0; i < 15; i++)
            {
                PIF_RAMb[48+i] = (response[i*2] << 4) + response[i*2+1];
            }
            // the last byte (2 nibbles) is always 0
            PIF_RAMb[63] = 0;
            break;
        case 0x08:
#ifdef DEBUG_PIF
            DebugMessage(M64MSG_INFO, "update_pif_write() pif_ram[0x3f] = 8");
#endif
            PIF_RAMb[0x3F] = 0;
            break;
        default:
            DebugMessage(M64MSG_ERROR, "error in update_pif_write(): %x", PIF_RAMb[0x3F]);
        }
        return;
    }
    while (i<0x40)
    {
        switch (PIF_RAMb[i])
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
}

void update_pif_read(void)
{
    int i=0, channel=0;
    while (i<0x40)
    {
        switch (PIF_RAMb[i])
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
}

