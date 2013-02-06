/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - flashram.c                                              *
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
#include "flashram.h"

#include "r4300/r4300.h"

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "main/main.h"
#include "main/rom.h"
#include "main/util.h"

Flashram_info flashram_info;

typedef enum flashram_mode
{
    NOPES_MODE = 0,
    ERASE_MODE,
    WRITE_MODE,
    READ_MODE,
    STATUS_MODE
} Flashram_mode;

static unsigned char flashram[0x20000];

static char *get_flashram_path(void)
{
    return formatstr("%s%s.fla", get_savesrampath(), ROM_SETTINGS.goodname);
}

static void flashram_format(void)
{
    memset(flashram, 0xff, sizeof(flashram));
}

static void flashram_read_file(void)
{
    char *filename = get_flashram_path();

    flashram_format();
    switch (read_from_file(filename, flashram, sizeof(flashram)))
    {
        case file_open_error:
            DebugMessage(M64MSG_WARNING, "couldn't open flash ram file '%s' for reading", filename);
            flashram_format();
            break;
        case file_read_error:
            DebugMessage(M64MSG_WARNING, "couldn't read 128kb flash ram file '%s'", filename);
            break;
        default: break;
    }

    free(filename);
}

static void flashram_write_file(void)
{
    char *filename = get_flashram_path();

    switch (write_to_file(filename, flashram, sizeof(flashram)))
    {
        case file_open_error:
            DebugMessage(M64MSG_WARNING, "couldn't open flash ram file '%s' for writing", filename);
            break;
        case file_write_error:
            DebugMessage(M64MSG_WARNING, "couldn't write 128kb flash ram file '%s'", filename);
            break;
        default: break;
    }

    free(filename);
}

void init_flashram(void)
{
    flashram_info.mode = NOPES_MODE;
    flashram_info.status = 0;
}

unsigned int flashram_status(void)
{
    return (unsigned int) (flashram_info.status >> 32);
}

void flashram_command(unsigned int command)
{
    switch (command & 0xff000000)
    {
    case 0x4b000000:
        flashram_info.erase_offset = (command & 0xffff) * 128;
        break;
    case 0x78000000:
        flashram_info.mode = ERASE_MODE;
        flashram_info.status = 0x1111800800c20000LL;
        break;
    case 0xa5000000:
        flashram_info.erase_offset = (command & 0xffff) * 128;
        flashram_info.status = 0x1111800400c20000LL;
        break;
    case 0xb4000000:
        flashram_info.mode = WRITE_MODE;
        break;
    case 0xd2000000:  // execute
        switch (flashram_info.mode)
        {
        case NOPES_MODE:
            break;
        case ERASE_MODE:
        {
            unsigned int i;
            flashram_read_file();
            for (i=flashram_info.erase_offset; i<(flashram_info.erase_offset+128); i++)
            {
                flashram[i^S8] = 0xff;
            }
            flashram_write_file();
        }
        break;
        case WRITE_MODE:
        {
            int i;
            flashram_read_file();
            for (i=0; i<128; i++)
            {
                flashram[(flashram_info.erase_offset+i)^S8]=
                    ((unsigned char*)rdram)[(flashram_info.write_pointer+i)^S8];
            }
            flashram_write_file();
        }
        break;
        case STATUS_MODE:
            break;
        default:
            DebugMessage(M64MSG_WARNING, "unknown flashram command with mode:%x", (int)flashram_info.mode);
            stop=1;
            break;
        }
        flashram_info.mode = NOPES_MODE;
        break;
    case 0xe1000000:
        flashram_info.mode = STATUS_MODE;
        flashram_info.status = 0x1111800100c20000LL;
        break;
    case 0xf0000000:
        flashram_info.mode = READ_MODE;
        flashram_info.status = 0x11118004f0000000LL;
        break;
    default:
        DebugMessage(M64MSG_WARNING, "unknown flashram command: %x", (int)command);
        break;
    }
}

void dma_read_flashram(void)
{
    unsigned int i;

    switch (flashram_info.mode)
    {
    case STATUS_MODE:
        rdram[pi_register.pi_dram_addr_reg/4] = (unsigned int)(flashram_info.status >> 32);
        rdram[pi_register.pi_dram_addr_reg/4+1] = (unsigned int)(flashram_info.status);
        break;
    case READ_MODE:
        flashram_read_file();
        for (i=0; i<(pi_register.pi_wr_len_reg & 0x0FFFFFF)+1; i++)
        {
            ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8]=
                flashram[(((pi_register.pi_cart_addr_reg-0x08000000)&0xFFFF)*2+i)^S8];
        }
        break;
    default:
        DebugMessage(M64MSG_WARNING, "unknown dma_read_flashram: %x", flashram_info.mode);
        stop=1;
        break;
    }
}

void dma_write_flashram(void)
{
    switch (flashram_info.mode)
    {
    case WRITE_MODE:
        flashram_info.write_pointer = pi_register.pi_dram_addr_reg;
        break;
    default:
        DebugMessage(M64MSG_ERROR, "unknown dma_write_flashram: %x", flashram_info.mode);
        stop=1;
        break;
    }
}

