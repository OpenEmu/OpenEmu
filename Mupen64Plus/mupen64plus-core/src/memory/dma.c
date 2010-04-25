/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dma.c                                                   *
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "api/m64p_types.h"

#include "dma.h"
#include "memory.h"
#include "pif.h"
#include "flashram.h"

#include "r4300/r4300.h"
#include "r4300/interupt.h"
#include "r4300/macros.h"
#include "r4300/ops.h"

#include "api/config.h"
#include "api/callbacks.h"
#include "main/main.h"
#include "main/rom.h"

static unsigned char sram[0x8000];

void dma_pi_read(void)
{
    unsigned int i;
   
    if (pi_register.pi_cart_addr_reg >= 0x08000000
    && pi_register.pi_cart_addr_reg < 0x08010000)
    {
        if (use_flashram != 1)
        {
            char *filename;
            FILE *f;
            filename = (char *) malloc(strlen(get_savespath())+
            strlen(ROM_SETTINGS.goodname)+4+1);
            strcpy(filename, get_savespath());
            strcat(filename, ROM_SETTINGS.goodname);
            strcat(filename, ".sra");
            f = fopen(filename, "rb");
            if (f == NULL)
            {
                DebugMessage(M64MSG_WARNING, "couldn't open sram file '%s' for reading", filename);
                memset(sram, 0, 0x8000);
            }
            else
            {
                if (fread(sram, 1, 0x8000, f) != 0x8000)
                {
                    DebugMessage(M64MSG_WARNING, "fread() failed on 32kb read from sram file '%s'", filename);
                    memset(sram, 0, 0x8000);
                }
                fclose(f);
            }
            for (i=0; i < (pi_register.pi_rd_len_reg & 0xFFFFFF)+1; i++)
            {
                sram[((pi_register.pi_cart_addr_reg-0x08000000)+i)^S8] =
                ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8];
            }
            f = fopen(filename, "wb");
            if (f == NULL)
            {
                 DebugMessage(M64MSG_WARNING, "couldn't open sram file '%s' for writing.", filename);
            }
            else
            {
                if (fwrite(sram, 1, 0x8000, f) != 0x8000)
                    DebugMessage(M64MSG_WARNING, "frwite() failed on 32kb write to sram file '%s'", filename);
                fclose(f);
            }
            free(filename);
            use_flashram = -1;
        }
        else
        {
            dma_write_flashram();
        }
    }
    else
    {
        DebugMessage(M64MSG_WARNING, "Unknown dma read in dma_pi_read()");
    }
    
    pi_register.read_pi_status_reg |= 1;
    update_count();
    add_interupt_event(PI_INT, 0x1000/*pi_register.pi_rd_len_reg*/);
}

void dma_pi_write(void)
{
    unsigned int longueur;
    int i;
    
    if (pi_register.pi_cart_addr_reg < 0x10000000)
    {
        if (pi_register.pi_cart_addr_reg >= 0x08000000 
        && pi_register.pi_cart_addr_reg < 0x08010000)
        {
            if (use_flashram != 1)
            {
                char *filename;
                FILE *f;
                int i;
                
                filename = (char *) malloc(strlen(get_savespath())+
                strlen(ROM_SETTINGS.goodname)+4+1);
                strcpy(filename, get_savespath());
                strcat(filename, ROM_SETTINGS.goodname);
                strcat(filename, ".sra");
                f = fopen(filename, "rb");
                if (f == NULL)
                {
                    DebugMessage(M64MSG_WARNING, "couldn't open sram file '%s' for reading", filename);
                    memset(sram, 0, 0x8000);
                }
                else
                {
                    if (fread(sram, 1, 0x8000, f) != 0x8000)
                        DebugMessage(M64MSG_WARNING, "fread() failed on 32kb read to sram file '%s'", filename);
                    fclose(f);
                }
                
                free(filename);
                
                for (i=0; i<(int)(pi_register.pi_wr_len_reg & 0xFFFFFF)+1; i++)
                {
                    ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8]=
                    sram[(((pi_register.pi_cart_addr_reg-0x08000000)&0xFFFF)+i)^S8];
                }
                
                use_flashram = -1;
            }
            else
            {
                dma_read_flashram();
            }
        }
        else if (pi_register.pi_cart_addr_reg >= 0x06000000
               && pi_register.pi_cart_addr_reg < 0x08000000)
        {
        }
        else
        {
            DebugMessage(M64MSG_WARNING, "Unknown dma write 0x%x in dma_pi_write()", (int)pi_register.pi_cart_addr_reg);
        }
        
        pi_register.read_pi_status_reg |= 1;
        update_count();
        add_interupt_event(PI_INT, /*pi_register.pi_wr_len_reg*/0x1000);
        
        return;
    }

    if (pi_register.pi_cart_addr_reg >= 0x1fc00000) // for paper mario
    {
        pi_register.read_pi_status_reg |= 1;
        update_count();
        add_interupt_event(PI_INT, 0x1000);
        
        return;
    }

    longueur = (pi_register.pi_wr_len_reg & 0xFFFFFF)+1;
    i = (pi_register.pi_cart_addr_reg-0x10000000)&0x3FFFFFF;
    longueur = (i + (int) longueur) > rom_size ?
    (rom_size - i) : longueur;
    longueur = (pi_register.pi_dram_addr_reg + longueur) > 0x7FFFFF ?
    (0x7FFFFF - pi_register.pi_dram_addr_reg) : longueur;

    if(i>rom_size || pi_register.pi_dram_addr_reg > 0x7FFFFF)
    {
        pi_register.read_pi_status_reg |= 3;
        update_count();
        add_interupt_event(PI_INT, longueur/8);
        
        return;
    }
    
    if(r4300emu != CORE_PURE_INTERPRETER)
    {
        for (i=0; i<(int)longueur; i++)
        {
            unsigned long rdram_address1 = pi_register.pi_dram_addr_reg+i+0x80000000;
            unsigned long rdram_address2 = pi_register.pi_dram_addr_reg+i+0xa0000000;
            ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8]=
            rom[(((pi_register.pi_cart_addr_reg-0x10000000)&0x3FFFFFF)+i)^S8];

            if(!invalid_code[rdram_address1>>12])
            {
                if(blocks[rdram_address1>>12]->block[(rdram_address1&0xFFF)/4].ops != NOTCOMPILED)
                {
                    invalid_code[rdram_address1>>12] = 1;
                }
            }
            if(!invalid_code[rdram_address2>>12])
            {
                if(blocks[rdram_address2>>12]->block[(rdram_address2&0xFFF)/4].ops != NOTCOMPILED)
                {
                    invalid_code[rdram_address2>>12] = 1;
                }
            }
        }
    }
    else
    {
        for (i=0; i<(int)longueur; i++)
        {
            ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8]=
            rom[(((pi_register.pi_cart_addr_reg-0x10000000)&0x3FFFFFF)+i)^S8];
        }
    }

    if ((debug_count+Count) < 0x100000)
    {
        
        switch(CIC_Chip)
        {
            case 1:
            case 2:
            case 3:
            case 6:
            {
                if (ConfigGetParamInt(g_CoreConfig, "DisableExtraMem"))
                {
                    rdram[0x318/4] = 0x400000;
                }
                else
                {
                    rdram[0x318/4] = 0x800000;
                }
                break;
            }
            case 5:
            {
                if (ConfigGetParamInt(g_CoreConfig, "DisableExtraMem"))
                {
                    rdram[0x3F0/4] = 0x400000;
                }
                else
                {
                    rdram[0x3F0/4] = 0x800000;
                }
                break;
            }
        }
    }

    pi_register.read_pi_status_reg |= 3;
    update_count();
    add_interupt_event(PI_INT, longueur/8);
    
    return;
}
    
void dma_sp_write(void)
{
    unsigned int i;
    
    if ((sp_register.sp_mem_addr_reg & 0x1000) > 0)
    {
        for (i=0; i<((sp_register.sp_rd_len_reg & 0xFFF)+1); i++)
        {
            ((unsigned char *)(SP_IMEM))[((sp_register.sp_mem_addr_reg & 0xFFF)+i)^S8]=
            ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF)+i)^S8];
        }
    }
    else
    {
        for (i=0; i<((sp_register.sp_rd_len_reg & 0xFFF)+1); i++)
        {
            ((unsigned char *)(SP_DMEM))[((sp_register.sp_mem_addr_reg & 0xFFF)+i)^S8]=
            ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF)+i)^S8];
        }
    }
}

void dma_sp_read(void)
{
    unsigned int i;
    
    if ((sp_register.sp_mem_addr_reg & 0x1000) > 0)
    {
        for (i=0; i<((sp_register.sp_wr_len_reg & 0xFFF)+1); i++)
        {
            ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF)+i)^S8]=
            ((unsigned char *)(SP_IMEM))[((sp_register.sp_mem_addr_reg & 0xFFF)+i)^S8];
        }
    }
    else
    {
        for (i=0; i<((sp_register.sp_wr_len_reg & 0xFFF)+1); i++)
        {
            ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF)+i)^S8]=
            ((unsigned char *)(SP_DMEM))[((sp_register.sp_mem_addr_reg & 0xFFF)+i)^S8];
        }
    }
}

void dma_si_write(void)
{
    int i;
    
    if (si_register.si_pif_addr_wr64b != 0x1FC007C0)
    {
        DebugMessage(M64MSG_ERROR, "dma_si_write(): unknown SI use");
        stop=1;
    }
    
    for (i=0; i<(64/4); i++)
    {
        PIF_RAM[i] = sl(rdram[si_register.si_dram_addr/4+i]);
    }
    
    update_pif_write();
    update_count();
    add_interupt_event(SI_INT, /*0x100*/0x900);
}

void dma_si_read(void)
{
    int i;
    
    if (si_register.si_pif_addr_rd64b != 0x1FC007C0)
    {
        DebugMessage(M64MSG_ERROR, "dma_si_read(): unknown SI use");
        stop=1;
    }
    
    update_pif_read();
    
    for (i=0; i<(64/4); i++)
    {
        rdram[si_register.si_dram_addr/4+i] = sl(PIF_RAM[i]);
    }
    
    update_count();
    add_interupt_event(SI_INT, /*0x100*/0x900);
}

