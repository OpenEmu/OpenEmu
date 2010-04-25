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

int use_flashram;

typedef enum flashram_mode
{
   NOPES_MODE = 0,
   ERASE_MODE,
   WRITE_MODE,
   READ_MODE,
   STATUS_MODE
} Flashram_mode;

static int mode;
static unsigned long long status;
static unsigned char flashram[0x20000];
static unsigned int erase_offset, write_pointer;

void save_flashram_infos(char *buf)
{
    memcpy(buf+0 , &use_flashram , 4);
    memcpy(buf+4 , &mode            , 4);
    memcpy(buf+8 , &status         , 8);
    memcpy(buf+16, &erase_offset , 4);
    memcpy(buf+20, &write_pointer, 4);
}

void load_flashram_infos(char *buf)
{
    memcpy(&use_flashram , buf+0 , 4);
    memcpy(&mode            , buf+4 , 4);
    memcpy(&status         , buf+8 , 8);
    memcpy(&erase_offset , buf+16, 4);
    memcpy(&write_pointer, buf+20, 4);
}

void init_flashram(void)
{
    mode = NOPES_MODE;
    status = 0;
}

unsigned int flashram_status(void)
{
    return (unsigned int) (status >> 32);
}

void flashram_command(unsigned int command)
{
    switch(command & 0xff000000)
    {
        case 0x4b000000:
            erase_offset = (command & 0xffff) * 128;
            break;
        case 0x78000000:
            mode = ERASE_MODE;
            status = 0x1111800800c20000LL;
            break;
        case 0xa5000000:
            erase_offset = (command & 0xffff) * 128;
            status = 0x1111800400c20000LL;
            break;
        case 0xb4000000:
            mode = WRITE_MODE;
            break;
        case 0xd2000000:  // execute
            switch (mode)
            {
                case NOPES_MODE:
                    break;
                case ERASE_MODE:
                {
                   char *filename;
                   FILE *f;
                   unsigned int i;
                   filename = (char *) malloc(strlen(get_savespath())+
                              strlen(ROM_SETTINGS.goodname)+4+1);
                   strcpy(filename, get_savespath());
                   strcat(filename, ROM_SETTINGS.goodname);
                   strcat(filename, ".fla");
                   f = fopen(filename, "rb");
                   if (f == NULL)
                   {
                       DebugMessage(M64MSG_WARNING, "couldn't open flash ram file '%s' for reading", filename);
                       memset(flashram, 0xff, 0x20000);
                   }
                   else
                   {
                       if (fread(flashram, 1, 0x20000, f) != 0x20000)
                           DebugMessage(M64MSG_WARNING, "couldn't read 128kb flash ram file '%s'", filename);
                       fclose(f);
                   }
                   for (i=erase_offset; i<(erase_offset+128); i++)
                   {
                       flashram[i^S8] = 0xff;
                   }
                   f = fopen(filename, "wb");
                   if (f == NULL)
                   {
                       DebugMessage(M64MSG_WARNING, "couldn't open flash ram file '%s' for writing", filename);
                   }
                   else
                   {
                       if (fwrite(flashram, 1, 0x20000, f) != 0x20000)
                           DebugMessage(M64MSG_WARNING, "couldn't write 128kb flash ram file '%s'", filename);
                       fclose(f);
                   }
                   free(filename);
                }
                    break;
                case WRITE_MODE:
                {
                    char *filename;
                    FILE *f;
                    int i;
                    filename = (char *) malloc(strlen(get_savespath())+
                               strlen(ROM_SETTINGS.goodname)+4+1);
                    strcpy(filename, get_savespath());
                    strcat(filename, ROM_SETTINGS.goodname);
                    strcat(filename, ".fla");
                    f = fopen(filename, "rb");
                    if (f == NULL)
                    {
                        DebugMessage(M64MSG_WARNING, "couldn't open flash ram file '%s' for reading", filename);
                        memset(flashram, 0xff, 0x20000);
                    }
                    else
                    {
                        if (fread(flashram, 1, 0x20000, f) != 0x20000)
                            DebugMessage(M64MSG_WARNING, "couldn't read 128kb flash ram file '%s'", filename);
                        fclose(f);
                    }
                    for (i=0; i<128; i++)
                    {
                        flashram[(erase_offset+i)^S8]=
                        ((unsigned char*)rdram)[(write_pointer+i)^S8];
                    }
                    f = fopen(filename, "wb");
                    if (f == NULL)
                    {
                        DebugMessage(M64MSG_WARNING, "couldn't open flashram file '%s' for writing", filename);
                    }
                    else
                    {
                        if (fwrite(flashram, 1, 0x20000, f) != 0x20000)
                            DebugMessage(M64MSG_WARNING, "couldn't write 128kb flash ram file '%s'", filename);
                        fclose(f);
                    }
                    free(filename);
                }
                    break;
                case STATUS_MODE:
                    break;
                default:
                    DebugMessage(M64MSG_WARNING, "unknown flashram command with mode:%x", (int)mode);
                    stop=1;
                    break;
            }
            mode = NOPES_MODE;
            break;
        case 0xe1000000:
            mode = STATUS_MODE;
            status = 0x1111800100c20000LL;
            break;
        case 0xf0000000:
            mode = READ_MODE;
            status = 0x11118004f0000000LL;
            break;
        default:
            DebugMessage(M64MSG_WARNING, "unknown flashram command: %x", (int)command);
            break;
      }
}

void dma_read_flashram(void)
{
    unsigned int i;
    char *filename;
    FILE *f;
    
    switch(mode)
    {
        case STATUS_MODE:
            rdram[pi_register.pi_dram_addr_reg/4] = (unsigned int)(status >> 32);
            rdram[pi_register.pi_dram_addr_reg/4+1] = (unsigned int)(status);
            break;
        case READ_MODE:
            filename = (char *) malloc(strlen(get_savespath())+
                       strlen(ROM_SETTINGS.goodname)+4+1);
            strcpy(filename, get_savespath());
            strcat(filename, ROM_SETTINGS.goodname);
            strcat(filename, ".fla");
            f = fopen(filename, "rb");
            if (f == NULL)
            {
                DebugMessage(M64MSG_WARNING, "couldn't open flash ram file '%s' for reading", filename);
                memset(flashram, 0xff, 0x20000);
            }
            else
            {
                if (fread(flashram, 1, 0x20000, f) != 0x20000)
                    DebugMessage(M64MSG_WARNING, "couldn't read 128kb flash ram file '%s'", filename);
                fclose(f);
            }
            free(filename);
            for (i=0; i<(pi_register.pi_wr_len_reg & 0x0FFFFFF)+1; i++)
            {
                ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8]=
                flashram[(((pi_register.pi_cart_addr_reg-0x08000000)&0xFFFF)*2+i)^S8];
            }
            break;
        default:  
            DebugMessage(M64MSG_WARNING, "unknown dma_read_flashram: %x", mode);
            stop=1;
            break;
      }
}

void dma_write_flashram(void)
{
    switch(mode)
    {
        case WRITE_MODE:
            write_pointer = pi_register.pi_dram_addr_reg;
            break;
        default:
            DebugMessage(M64MSG_ERROR, "unknown dma_write_flashram: %x", mode);
            stop=1;
            break;
    }
}

