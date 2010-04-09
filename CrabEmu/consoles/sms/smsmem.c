/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2007, 2008, 2009 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sms.h"
#include "smsmem.h"
#include "smsmem-gg.h"
#include "smsvdp.h"
#include "sn76489.h"
#include "93c46.h"
#include "smsz80.h"
#include "mappers.h"
#include "terebi.h"
#include "mapper-93c46.h"
#include "mapper-korean.h"
#include "mapper-codemasters.h"
#include "mapper-sega.h"
#include "mapper-sg1000.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _arch_dreamcast
#include <zlib/zlib.h>
#include <dc/vmu_pkg.h>
#include "icon.h"
#endif

static uint8 ram[8 * 1024];

static uint8 *sms_bios_rom = NULL;
static uint32 sms_bios_len;
static char *savefn = NULL;
static int cartram_enabled = 0;

static int mapper;

extern uint16 sms_pad;
extern int sms_region;
extern sn76489_t psg;
extern int sms_console;
extern eeprom93c46_t e93c46;

uint8 sms_paging_regs[4];
uint8 *sms_rom_page0;
uint8 *sms_rom_page1;
uint8 *sms_rom_page2;
uint8 *sms_cart_rom = NULL;
uint32 sms_cart_len;
uint8 *gg_bios_rom = NULL;
uint32 gg_bios_len;
uint8 sms_memctl = 0;
uint16 sms_ioctl_input_mask = 0xFFFF;
uint16 sms_ioctl_output_mask = 0x0000;
uint16 sms_ioctl_output_bits = 0x0000;
uint8 sms_gg_regs[7];
uint8 *sms_read_map[256];
uint8 *sms_write_map[256];
uint8 sms_dummy_arear[256];
uint8 sms_dummy_areaw[256];
uint8 sms_cart_ram[0x8000];

typedef void (*remap_page_func)();
remap_page_func sms_mem_remap_page[4];

static void remap_page0_unmapped()  {
    int i;

    for(i = 0; i < 0x40; ++i)   {
        sms_read_map[i] = sms_dummy_arear;
        sms_write_map[i] = sms_dummy_areaw;
    }
}

static void remap_page1_unmapped()  {
    int i;

    for(i = 0x40; i < 0x80; ++i)    {
        sms_read_map[i] = sms_dummy_arear;
        sms_write_map[i] = sms_dummy_areaw;
    }
}

static void remap_page2_unmapped()  {
    int i;

    for(i = 0x80; i < 0xC0; ++i)    {
        sms_read_map[i] = sms_dummy_arear;
        sms_write_map[i] = sms_dummy_areaw;
    }
}

static void remap_page0()   {
    int i;

    if(sms_paging_regs[1] && sms_cart_len > 0x4000) {
        sms_rom_page0 = &sms_cart_rom[0x4000 * (sms_paging_regs[1] %
                                                (sms_cart_len / 0x4000))];
    }
    else    {
        sms_rom_page0 = sms_cart_rom;
    }

    if(mapper != SMS_MAPPER_SEGA)   {
        i = 0x00;
    }
    else    {
        i = 0x04;
        sms_read_map[0] = sms_cart_rom;
        sms_read_map[1] = sms_cart_rom + 0x100;
        sms_read_map[2] = sms_cart_rom + 0x200;
        sms_read_map[3] = sms_cart_rom + 0x300;
        sms_write_map[0] = sms_dummy_areaw;
        sms_write_map[1] = sms_dummy_areaw;
        sms_write_map[2] = sms_dummy_areaw;
        sms_write_map[3] = sms_dummy_areaw;
    }

    for(; i < 0x40; ++i)    {
        sms_read_map[i] = sms_rom_page0 + (i << 8);
        sms_write_map[i] = sms_dummy_areaw;
    }

    sms_z80_set_readmap(sms_read_map);
}

static void remap_page0_sms_bios()  {
    int i;

    if(sms_paging_regs[1] && sms_bios_len > 0x4000) {
        sms_rom_page0 = &sms_bios_rom[0x4000 * (sms_paging_regs[1] %
                                                (sms_bios_len / 0x4000))];
    }
    else    {
        sms_rom_page0 = sms_bios_rom;
    }

    sms_read_map[0] = sms_bios_rom;
    sms_read_map[1] = sms_bios_rom + 0x100;
    sms_read_map[2] = sms_bios_rom + 0x200;
    sms_read_map[3] = sms_bios_rom + 0x300;
    sms_write_map[0] = sms_dummy_areaw;
    sms_write_map[1] = sms_dummy_areaw;
    sms_write_map[2] = sms_dummy_areaw;
    sms_write_map[3] = sms_dummy_areaw;

    for(i = 0x04; i < 0x40; ++i)    {
        sms_read_map[i] = sms_rom_page0 + (i << 8);
        sms_write_map[i] = sms_dummy_areaw;
    }

    sms_z80_set_readmap(sms_read_map);
}

static void remap_page1()   {
    int i;

    if(sms_paging_regs[2] && sms_cart_len > 0x4000) {
        sms_rom_page1 = &sms_cart_rom[0x4000 * (sms_paging_regs[2] %
                                                (sms_cart_len / 0x4000))];
    }
    else    {
        sms_rom_page1 = sms_cart_rom;
    }

    for(i = 0x40; i < 0x80; ++i)    {
        sms_read_map[i] = sms_rom_page1 + ((i & 0x3F) << 8);
        sms_write_map[i] = sms_dummy_areaw;
    }

    sms_z80_set_readmap(sms_read_map);
}

static void remap_page1_sms_bios()  {
    int i;

    if(sms_paging_regs[2] && sms_bios_len > 0x4000) {
        sms_rom_page1 = &sms_bios_rom[0x4000 * (sms_paging_regs[2] %
                                                (sms_bios_len / 0x4000))];
    }
    else    {
        sms_rom_page1 = sms_bios_rom;
    }

    for(i = 0x40; i < 0x80; ++i)    {
        sms_read_map[i] = sms_rom_page1 + ((i & 0x3F) << 8);
        sms_write_map[i] = sms_dummy_areaw;
    }

    sms_z80_set_readmap(sms_read_map);
}

static void remap_page2()   {
    int i, isram = 0;

    if(sms_paging_regs[0] & 0x08 && mapper == SMS_MAPPER_SEGA)  {
        /* Cartridge RAM enable */
        if(sms_paging_regs[0] & 0x04)   {
            sms_rom_page2 = &sms_cart_ram[0x4000];
        }
        else    {
            sms_rom_page2 = &sms_cart_ram[0];
        }

        isram = 1;
        cartram_enabled = 1;
    }
    else if(sms_paging_regs[3] && sms_cart_len > 0x4000)    {
        sms_rom_page2 = &sms_cart_rom[0x4000 * (sms_paging_regs[3] %
                                                (sms_cart_len / 0x4000))];
    }
    else    {
        sms_rom_page2 = sms_cart_rom;
    }

    if(isram)   {
        for(i = 0x80; i < 0xC0; ++i)    {
            sms_read_map[i] = sms_rom_page2 + ((i & 0x3F) << 8);
            sms_write_map[i] = sms_rom_page2 + ((i & 0x3F) << 8);
        }
    }
    else    {
        for(i = 0x80; i < 0xC0; ++i)    {
            sms_read_map[i] = sms_rom_page2 + ((i & 0x3F) << 8);
            sms_write_map[i] = sms_dummy_areaw;
        }
    }

    sms_z80_set_readmap(sms_read_map);
}

static void remap_page2_sms_bios()  {
    int i;

    /* Assume no SRAM for BIOS. */
    if(sms_paging_regs[3] && sms_bios_len > 0x4000) {
        sms_rom_page2 = &sms_bios_rom[0x4000 * (sms_paging_regs[3] %
                                                (sms_bios_len / 0x4000))];
    }
    else    {
        sms_rom_page2 = sms_bios_rom;
    }

    for(i = 0x80; i < 0xC0; ++i)    {
        sms_read_map[i] = sms_rom_page2 + ((i & 0x3F) << 8);
        sms_write_map[i] = sms_dummy_areaw;
    }

    sms_z80_set_readmap(sms_read_map);
}

static void reorganize_pages()  {
    sms_mem_remap_page[1]();
    sms_mem_remap_page[2]();
    sms_mem_remap_page[3]();
}

/* This function is based on code from MEKA, the duty of this function is to
   try to autodetect what mapper type this cartridge should be using. */
static void setup_mapper()  {
    int codemasters, sega, korean;
    uint8 *ptr;

    if(sms_cart_len < 0x8000)   {
        /* The cartridge contains less than 32KB of data, so there probably
           isn't even a mapper in it, but just to be safe, in case it has
           SRAM for some reason, we'll just assume the Sega Mapper. */
        mapper = SMS_MAPPER_SEGA;
        return;
    }

    sega = 0;
    codemasters = 0;
    korean = 0;
    ptr = sms_cart_rom;

    while(ptr < sms_cart_rom + 0x8000)  {
        if((*ptr++) == 0x32)  {
            uint16 value = (*ptr) | ((*(ptr + 1)) << 8);

            ptr += 2;

            if(value == 0xFFFF)
                ++sega;
            else if(value == 0x8000 || value == 0x4000)
                ++codemasters;
            else if(value == 0xA000)
                ++korean;
        }
    }

    if(codemasters > sega && codemasters > korean)
        mapper = SMS_MAPPER_CODEMASTERS;
    else if(korean > codemasters && korean > sega)
        mapper = SMS_MAPPER_KOREAN;
    else
        mapper = SMS_MAPPER_SEGA;
}

void sms_mem_handle_memctl(uint8 data)  {
    if(sms_memctl == data)
        return;

    if(sms_console != CONSOLE_SMS && sms_console != CONSOLE_GG)
        return;

    if(!(data & SMS_MEMCTL_BIOS) && sms_console == CONSOLE_SMS &&
       sms_bios_rom != NULL)    {
        sms_mem_remap_page[0] = &remap_page2_sms_bios;
        sms_mem_remap_page[1] = &remap_page0_sms_bios;
        sms_mem_remap_page[2] = &remap_page1_sms_bios;
        sms_mem_remap_page[3] = &remap_page2_sms_bios;
        sms_z80_set_mread(&sms_mem_sega_mread);
        sms_z80_set_mread16(&sms_mem_sega_mread16);
        sms_z80_set_mwrite(&sms_mem_sega_mwrite);
        sms_z80_set_mwrite16(&sms_mem_sega_mwrite16);
    }
    else if(!(data & SMS_MEMCTL_CART) || sms_bios_rom == NULL)  {
        switch(mapper)  {
            case SMS_MAPPER_SEGA:
                sms_z80_set_mread(&sms_mem_sega_mread);
                sms_z80_set_mread16(&sms_mem_sega_mread16);
                sms_z80_set_mwrite(&sms_mem_sega_mwrite);
                sms_z80_set_mwrite16(&sms_mem_sega_mwrite16);
                break;

            case SMS_MAPPER_CODEMASTERS:
                sms_z80_set_mread(&sms_mem_codemasters_mread);
                sms_z80_set_mwrite(&sms_mem_codemasters_mwrite);
                sms_z80_set_mread16(&sms_mem_codemasters_mread16);
                sms_z80_set_mwrite16(&sms_mem_codemasters_mwrite16);
                sms_mem_remap_page[0] = &sms_mem_remap_page2_codemasters;
                sms_mem_remap_page[1] = &remap_page0;
                sms_mem_remap_page[2] = &sms_mem_remap_page1_codemasters;
                sms_mem_remap_page[3] = &sms_mem_remap_page2_codemasters;
                break;

            case SMS_MAPPER_KOREAN:
                sms_z80_set_mread(&sms_mem_korean_mread);
                sms_z80_set_mwrite(&sms_mem_korean_mwrite);
                sms_z80_set_mread16(&sms_mem_korean_mread16);
                sms_z80_set_mwrite16(&sms_mem_korean_mwrite16);
                break;

            case SMS_MAPPER_93C46:
                sms_z80_set_mread(&sms_mem_93c46_mread);
                sms_z80_set_mwrite(&sms_mem_93c46_mwrite);
                sms_z80_set_mread16(&sms_mem_93c46_mread16);
                sms_z80_set_mwrite16(&sms_mem_93c46_mwrite16);
                break;
        }

        if(mapper != SMS_MAPPER_CODEMASTERS)    {
            sms_mem_remap_page[0] = &remap_page2;
            sms_mem_remap_page[1] = &remap_page0;
            sms_mem_remap_page[2] = &remap_page1;
            sms_mem_remap_page[3] = &remap_page2;
        }
    }
    else    {
        sms_mem_remap_page[0] = &remap_page2_unmapped;
        sms_mem_remap_page[1] = &remap_page0_unmapped;
        sms_mem_remap_page[2] = &remap_page1_unmapped;
        sms_mem_remap_page[3] = &remap_page2_unmapped;
    }

    reorganize_pages();

    sms_memctl = data;
}

void sms_mem_handle_ioctl(uint8 data)   {
    int old, new;

    /* Make sure we're emulating an export SMS, the Japanese SMS (and earlier
       hardware) did not have the I/O Control Register functionality. */
    if((sms_console == CONSOLE_SMS && (sms_region & SMS_REGION_DOMESTIC)) ||
       sms_console == CONSOLE_SG1000)
        return;

    old = ((sms_pad & sms_ioctl_input_mask) |
           (sms_ioctl_output_bits & sms_ioctl_output_mask)) ^ SMS_TH_MASK;

    /* Reset the input/output masks. */
    sms_ioctl_input_mask = 0x37DF;

    sms_ioctl_input_mask |= ((data & SMS_IOCTL_TR_A_DIRECTION) << 5) |
                        ((data & SMS_IOCTL_TH_A_DIRECTION) << 13) |
                        ((data & SMS_IOCTL_TR_B_DIRECTION) << 9) |
                        ((data & SMS_IOCTL_TH_B_DIRECTION) << 12);
   sms_ioctl_output_mask = sms_ioctl_input_mask ^ 0xFFFF;

    /* And grab the output levels of the bits specified. */
   sms_ioctl_output_bits = ((data & SMS_IOCTL_TR_A_LEVEL) << 1) |
                        ((data & SMS_IOCTL_TH_A_LEVEL) << 9) |
                        ((data & SMS_IOCTL_TR_B_LEVEL) << 5) |
                        ((data & SMS_IOCTL_TH_B_LEVEL) << 8);

   new = ((sms_pad & sms_ioctl_input_mask) |
          (sms_ioctl_output_bits & sms_ioctl_output_mask)) & SMS_TH_MASK;

   if(old & new)    {
       sms_vdp_hcnt_latch();
   }
}

void sms_port_write(uint16 port, uint8 data)    {
    port &= 0xFF;

    if(port < 0x40) {
        if(port & 0x01) {
            /* I/O Control register */
            sms_mem_handle_ioctl(data);
        }
        else    {
            /* Memory Control register */
            sms_mem_handle_memctl(data);
        }
    }
    else if(port < 0x80)    {
        /* SN76489 PSG */
        sn76489_write(&psg, data);
    }
    else if(port < 0xC0)    {
        if(port & 0x01) {
            /* VDP Control port */
            sms_vdp_ctl_write(data);
        }
        else    {
            /* VDP Data port */
            sms_vdp_data_write(data);
        }
    }
    else    {
    }
}

uint8 sms_port_read(uint16 port)    {
    port &= 0xFF;

    if(port < 0x40) {
        return 0xFF;
    }
    else if(port < 0x80)    {
        if(port & 0x01) {
            return sms_vdp_hcnt_read();
        }
        else    {
            return sms_vdp_vcnt_read();
        }
    }
    else if(port < 0xC0)    {
        if(port & 0x01) {
            return sms_vdp_status_read();
        }
        else    {
            return sms_vdp_data_read();
        }
    }
    else    {
        if(!(sms_memctl & SMS_MEMCTL_IO))   {
            if(port & 0x01) {
                /* I/O port B/misc register */
                return (((sms_pad & sms_ioctl_input_mask) |
                         (sms_ioctl_output_bits &
                          sms_ioctl_output_mask)) >> 8) & 0xFF;
            }
            else    {
                /* I/O port A/B register */
                return ((sms_pad & sms_ioctl_input_mask) |
                        (sms_ioctl_output_bits &
                         sms_ioctl_output_mask)) & 0xFF;
            }
        }
        else    {
            return 0xFF;
        }
    }
}

#ifndef _arch_dreamcast

int sms_write_cartram_to_file(void) {
    FILE *fp;

    if(savefn == NULL)
        return -1;

    if(mapper == SMS_MAPPER_93C46)  {
        fp = fopen(savefn, "wb");

        if(!fp)
            return -1;

        fwrite(e93c46.data, 64, 2, fp);
        fclose(fp);
    }
    else    {
        if(!cartram_enabled)
            return 0;

        fp = fopen(savefn, "wb");
        if(!fp)
            return -1;

        fwrite(sms_cart_ram, 0x8000, 1, fp);
        fclose(fp);
    }

    return 0;
}

int sms_read_cartram_from_file(void)    {
    FILE * fp;

    fp = fopen(savefn, "rb");
    
    if(fp != NULL)  {
        if(mapper == SMS_MAPPER_93C46)  {
            eeprom93c46_init();
            fread(e93c46.data, 64, 2, fp);
        }
        else    {
            fread(sms_cart_ram, 0x8000, 1, fp);
            cartram_enabled = 1;
        }
        fclose(fp);

        return 0;
    }

    return -1;
}

#else

int sms_write_cartram_to_file(void) {
    vmu_pkg_t pkg;
    uint8 *pkg_out;
    uint8 *comp;
    int pkg_size;
    uint32 len;
    FILE *fp;
    char prodcode[7];
    char savename[15];
    int err;

    if(savefn == NULL)  {
        return -1;
    }

    if(!cartram_enabled && mapper != SMS_MAPPER_93C46)  {
        return 0;
    }

    sprintf(prodcode, "%d%d%d%d%d", (sms_cart_rom[0x7FFE] & 0xF0) >> 4,
            (sms_cart_rom[0x7FFD] & 0xF0) >> 4, (sms_cart_rom[0x7FFD] & 0x0F),
            (sms_cart_rom[0x7FFC] & 0xF0) >> 4, (sms_cart_rom[0x7FFC] & 0x0F));

    sprintf(savename, "/vmu/a1/%s", prodcode);

    comp = (uint8 *)malloc(0x10000);
    len = 0x10000;

    if(mapper == SMS_MAPPER_93C46)  {
        err = compress2(comp, &len, (uint8 *)e93c46.data, 128, 9);
    }
    else    {
        err = compress2(comp, &len, sms_cart_ram, 0x8000, 9);
    }

    strcpy(pkg.desc_short, "CrabEmu Save");
    strcpy(pkg.desc_long, prodcode);
    strcpy(pkg.app_id, "CrabEmu");
    pkg.icon_cnt = 1;
    pkg.icon_anim_speed = 0;
    memcpy(pkg.icon_pal, icon_pal, 32);
    pkg.icon_data = icon_img;
    pkg.eyecatch_type = VMUPKG_EC_NONE;
    pkg.data_len = len;
    pkg.data = comp;

    vmu_pkg_build(&pkg, &pkg_out, &pkg_size);

    fp = fopen(savename, "wb");

    if(!fp) {
        free(pkg_out);
        free(comp);
        return -1;
    }

    fwrite(pkg_out, pkg_size, 1, fp);
    fclose(fp);

    free(pkg_out);
    free(comp);

    return 0;
}

int sms_read_cartram_from_file(void)    {
    vmu_pkg_t pkg;
    uint8 *pkg_out;
    int pkg_size;
    char prodcode[7];
    FILE *fp;
    char savename[15];
    uint32 real_size = mapper == SMS_MAPPER_93C46 ? 128 : 0x8000;

    if(savefn == NULL)
        return -1;

    sprintf(prodcode, "%d%d%d%d%d", (sms_cart_rom[0x7FFE] & 0xF0) >> 4,
            (sms_cart_rom[0x7FFD] & 0xF0) >> 4, (sms_cart_rom[0x7FFD] & 0x0F),
            (sms_cart_rom[0x7FFC] & 0xF0) >> 4, (sms_cart_rom[0x7FFC] & 0x0F));
    
    sprintf(savename, "/vmu/a1/%s", prodcode);

    fp = fopen(savename, "rb");

    if(!fp) {
        return -1;
    }

    fseek(fp, 0, SEEK_SET);
    fseek(fp, 0, SEEK_END);
    pkg_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    pkg_out = (uint8 *)malloc(pkg_size);
    fread(pkg_out, pkg_size, 1, fp);
    fclose(fp);

    vmu_pkg_parse(pkg_out, &pkg);

    if(mapper == SMS_MAPPER_93C46)  {
        eeprom93c46_init();
        uncompress((uint8 *)e93c46.data, &real_size, pkg.data, pkg.data_len);
    }
    else    {
        uncompress(sms_cart_ram, &real_size, pkg.data, pkg.data_len);
    }

    free(pkg_out);

    cartram_enabled = 1;

    return 0;
}

#endif /* _arch_dreamcast */

int sms_mem_load_bios(const char *fn)   {
    FILE *fp;
    int size;
    char *ext;
    uint8 **ptr;
    uint32 *len;

    fp = fopen(fn, "rb");

    if(!fp) {
#ifdef DEBUG
        fprintf(stderr, "sms_mem_load_bios: Could not open ROM: %s!\n", fn);
#endif
        return -1;
    }

    /* First, determine which type of BIOS ROM it is */
    ext = strrchr(fn, '.');

    if(!strcmp(ext, ".gg")) {
        /* Game Gear BIOS */
        ptr = &gg_bios_rom;
        len = &gg_bios_len;
    }
    else    {
        /* Master System BIOS, hopefully */
        ptr = &sms_bios_rom;
        len = &sms_bios_len;
    }

    if(*ptr != NULL)    {
        free(*ptr);
        *ptr = NULL;
        *len = 0;
    }

    /* Determine the size of the file */
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* Assume there's no footers or headers. */

    *ptr = (uint8 *)malloc(size);
    fread(*ptr, size, 1, fp);
    fclose(fp);

    *len = size;

    /* Assume the mapper is the Sega mapper, if any is even needed. */

    return 0;
}

int sms_mem_load_rom(const char *fn)    {
    FILE *fp;
    int size, i, console;
    char *ext, *name;

    memset(sms_cart_ram, 0, 0x8000);
    cartram_enabled = 0;

    fp = fopen(fn, "rb");

    if(!fp) {
#ifdef DEBUG
        fprintf(stderr, "sms_mem_load_rom: Could not open ROM: %s!\n", fn);
#endif
        return -1;
    }

    /* First, determine which type of ROM it is */
    ext = strrchr(fn, '.');

    if(!strcmp(ext, ".gg")) {
        /* GameGear rom */
        console = CONSOLE_GG;
    }
    else if(!strcmp(ext, ".sc"))    {
        /* SC-3000 rom */
        console = CONSOLE_SC3000;
    }
    else if(!strcmp(ext, ".sg"))    {
        /* SG-1000 rom */
        console = CONSOLE_SG1000;
    }
    else    {
        /* Master System rom, hopefully */
        console = CONSOLE_SMS;
    }

    sms_set_console(console);

    /* Determine the size of the file */
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* Determine if header/footer info is present */
    if((size % 0x4000) == 512)  {
        /* SMD header present, skip it */
        fseek(fp, 512, SEEK_SET);
        size -= 512;
    }
    else if((size % 0x4000) == 64)  {
        /* Footer present, truncate it */
        size -= 64;
    }

    sms_cart_rom = (uint8 *)malloc(size);
    fread(sms_cart_rom, size, 1, fp);
    fclose(fp);

    if(savefn)  {
        free(savefn);
    }

    sms_cart_len = size;

    /* Determine which mapper should be used, and set up the paging
       accordingly. First check the mappers database */
    if((mapper = sms_find_mapper(sms_cart_rom, sms_cart_len)) == -1)    {
        mapper = SMS_MAPPER_SEGA;

        if(sms_console != CONSOLE_SG1000)   {
            setup_mapper();

            if(mapper == SMS_MAPPER_SEGA)   {
                /* Check if this game uses the 93c46 serial eeprom, and if so
                   enable it. Most of these should be detected by the
                   sms_find_mapper function, but just in case, detect them
                   here as well. */
                const uint16 prodcodes[5] = {
                    0x3432, 0x2537, 0x2439, 0x3407, 0x2418
                };
                const uint16 popbreaker = 0x2017;
                uint16 prodcode = sms_cart_rom[0x7FFC] | (sms_cart_rom[0x7FFD] << 8) |
                    ((sms_cart_rom[0x7FFE] & 0xF0) << 12);

                for(i = 0; i < 5; ++i)  {
                    if(prodcode == prodcodes[i])    {
                        mapper = SMS_MAPPER_93C46;
                        break;
                    }
                }

                /* While we're at it, check if its Pop Breaker, which apparently
                   doesn't like running on a non-japanese Game Gear */
                if(prodcode == popbreaker)  {
                    sms_gg_regs[0] = 0x80;
                }
            }
        }
    }

    if(sms_console == CONSOLE_SG1000)   {
        for(i = 0; i < 0x08; ++i)   {
            sms_read_map[i + 0xC8] = ram + (i << 8);
            sms_read_map[i + 0xD0] = ram + (i << 8);
            sms_read_map[i + 0xD8] = ram + (i << 8);
            sms_read_map[i + 0xE8] = ram + (i << 8);
            sms_read_map[i + 0xF0] = ram + (i << 8);
            sms_read_map[i + 0xF8] = ram + (i << 8);

            sms_write_map[i + 0xC8] = ram + (i << 8);
            sms_write_map[i + 0xD0] = ram + (i << 8);
            sms_write_map[i + 0xD8] = ram + (i << 8);
            sms_write_map[i + 0xE8] = ram + (i << 8);
            sms_write_map[i + 0xF0] = ram + (i << 8);
            sms_write_map[i + 0xF8] = ram + (i << 8);
        }
    }

    if((sms_console != CONSOLE_SMS || sms_bios_rom == NULL) &&
       (sms_console != CONSOLE_GG || gg_bios_rom == NULL))  {
        switch(mapper)  {
            case SMS_MAPPER_SEGA:
                if(sms_console != CONSOLE_SG1000)   {
                    sms_z80_set_mread(&sms_mem_sega_mread);
                    sms_z80_set_mread16(&sms_mem_sega_mread16);
                    sms_z80_set_mwrite(&sms_mem_sega_mwrite);
                    sms_z80_set_mwrite16(&sms_mem_sega_mwrite16);
                }
                else    {
                    sms_z80_set_mread(&sms_mem_sg_mread);
                    sms_z80_set_mread16(&sms_mem_sg_mread16);
                    sms_z80_set_mwrite(&sms_mem_sg_mwrite);
                    sms_z80_set_mwrite16(&sms_mem_sg_mwrite16);
                }

                break;

            case SMS_MAPPER_CODEMASTERS:
                sms_z80_set_mread(&sms_mem_codemasters_mread);
                sms_z80_set_mwrite(&sms_mem_codemasters_mwrite);
                sms_z80_set_mread16(&sms_mem_codemasters_mread16);
                sms_z80_set_mwrite16(&sms_mem_codemasters_mwrite16);
                sms_mem_remap_page[0] = &sms_mem_remap_page2_codemasters;
                sms_mem_remap_page[1] = &remap_page0;
                sms_mem_remap_page[2] = &sms_mem_remap_page1_codemasters;
                sms_mem_remap_page[3] = &sms_mem_remap_page2_codemasters;
                break;

            case SMS_MAPPER_KOREAN:
                sms_z80_set_mread(&sms_mem_korean_mread);
                sms_z80_set_mwrite(&sms_mem_korean_mwrite);
                sms_z80_set_mread16(&sms_mem_korean_mread16);
                sms_z80_set_mwrite16(&sms_mem_korean_mwrite16);
                break;

            case SMS_MAPPER_93C46:
                eeprom93c46_init();
                sms_z80_set_mread(&sms_mem_93c46_mread);
                sms_z80_set_mwrite(&sms_mem_93c46_mwrite);
                sms_z80_set_mread16(&sms_mem_93c46_mread16);
                sms_z80_set_mwrite16(&sms_mem_93c46_mwrite16);
                break;

            case SMS_MAPPER_CASTLE:
                sms_z80_set_mread(&sms_mem_sg_mread);
                sms_z80_set_mwrite(&sms_mem_sg_mwrite);
                sms_z80_set_mread16(&sms_mem_sg_mread16);
                sms_z80_set_mwrite16(&sms_mem_sg_mwrite16);
                sms_mem_remap_page[3] = &sms_mem_remap_page2_castle;
                break;

            case SMS_MAPPER_TEREBI_OEKAKI:
                sms_z80_set_mread(&terebi_mread);
                sms_z80_set_mwrite(&terebi_mwrite);
                sms_z80_set_mread16(&terebi_mread16);
                sms_z80_set_mwrite16(&terebi_mwrite16);
                break;
        }
    }
    else if(sms_console == CONSOLE_SMS) {
        sms_mem_remap_page[0] = &remap_page2_sms_bios;
        sms_mem_remap_page[1] = &remap_page0_sms_bios;
        sms_mem_remap_page[2] = &remap_page1_sms_bios;
        sms_mem_remap_page[3] = &remap_page2_sms_bios;
        sms_z80_set_mread(&sms_mem_sega_mread);
        sms_z80_set_mread16(&sms_mem_sega_mread16);
        sms_z80_set_mwrite(&sms_mem_sega_mwrite);
        sms_z80_set_mwrite16(&sms_mem_sega_mwrite16);
    }
    else if(sms_console == CONSOLE_GG)  {
        sms_mem_remap_page[0] = &remap_page2;
        sms_mem_remap_page[1] = &sms_mem_remap_page0_gg_bios;
        sms_mem_remap_page[2] = &remap_page1;
        sms_mem_remap_page[3] = &remap_page2;
        sms_z80_set_mread(&sms_mem_sega_mread);
        sms_z80_set_mread16(&sms_mem_sega_mread16);
        sms_z80_set_mwrite(&sms_mem_sega_mwrite);
        sms_z80_set_mwrite16(&sms_mem_sega_mwrite16);
    }
        
    reorganize_pages();

    savefn = (char *) malloc(strlen(fn) + 5);
    sprintf(savefn, "%s.ces", fn);

    sms_read_cartram_from_file();

    name = strrchr(fn, '/');

    if(name != NULL)    {
        gui_set_title(name + 1);
    }
    else    {
        gui_set_title("CrabEmu");
    }

    return 0;
}

void sms_mem_write_context(FILE *fp)    {
    fwrite(sms_paging_regs, 4, 1, fp);
    fwrite(sms_gg_regs, 7, 1, fp);
    fwrite(ram, 0x2000, 1, fp);
    fwrite(sms_cart_ram, 0x8000, 1, fp);
}

void sms_mem_read_context(FILE *fp) {
    fread(sms_paging_regs, 4, 1, fp);
    fread(sms_gg_regs, 7, 1, fp);
    fread(ram, 0x2000, 1, fp);
    fread(sms_cart_ram, 0x8000, 1, fp);
    reorganize_pages();
}

int sms_mem_init(void)  {
    int i;

    sms_cart_rom = NULL;
    sms_rom_page0 = NULL;
    sms_rom_page1 = NULL;
    sms_rom_page2 = NULL;

    sms_mem_remap_page[0] = &remap_page2;
    sms_mem_remap_page[1] = &remap_page0;
    sms_mem_remap_page[2] = &remap_page1;
    sms_mem_remap_page[3] = &remap_page2;

    sms_paging_regs[0] = 0;
    sms_paging_regs[1] = 0;
    sms_paging_regs[2] = 1;
    sms_paging_regs[3] = 0;

    sms_gg_regs[0] = 0xC0;
    sms_gg_regs[1] = 0x7F;
    sms_gg_regs[2] = 0xFF;
    sms_gg_regs[3] = 0x00;
    sms_gg_regs[4] = 0xFF;
    sms_gg_regs[5] = 0x00;
    sms_gg_regs[6] = 0xFF;

    for(i = 0x00; i < 0xC0; ++i)    {
        sms_write_map[i] = sms_dummy_areaw;
    }

    for(i = 0xC0; i < 0xE0; ++i)    {
        sms_read_map[i] = ram + ((i - 0xC0) << 8);
        sms_write_map[i] = ram + ((i - 0xC0) << 8);
    }

    for(i = 0xE0; i < 0x100; ++i)   {
        sms_read_map[i] = ram + ((i - 0xE0) << 8);
        sms_write_map[i] = ram + ((i - 0xE0) << 8);
    }

    for(i = 0; i < 256; ++i)    {
        sms_dummy_arear[i] = 0xFF;
    }

    sms_ioctl_input_mask = 0xFFFF;
    sms_ioctl_output_mask = 0x0000;
    sms_ioctl_output_bits = 0x0000;

    sms_memctl = (SMS_MEMCTL_IO | SMS_MEMCTL_CART | SMS_MEMCTL_RAM) ^ 0xFF;

    return 0;
}

int sms_mem_shutdown(void)  {
    if(sms_cart_rom != NULL)
        free(sms_cart_rom);

    if(sms_bios_rom != NULL)
        free(sms_bios_rom);

    if(gg_bios_rom != NULL)
        free(gg_bios_rom);

    sms_bios_rom = NULL;
    gg_bios_rom = NULL;
    sms_cart_rom = NULL;

    return 0;
}

void sms_mem_reset(void)    {
    sms_paging_regs[0] = 0;
    sms_paging_regs[1] = 0;
    sms_paging_regs[2] = 1;
    sms_paging_regs[3] = 0;

    sms_gg_regs[0] |= 0x80;
    sms_gg_regs[1] = 0x7F;
    sms_gg_regs[2] = 0xFF;
    sms_gg_regs[3] = 0x00;
    sms_gg_regs[4] = 0xFF;
    sms_gg_regs[5] = 0x00;
    sms_gg_regs[6] = 0xFF;

    if(sms_console == CONSOLE_SMS && sms_bios_rom != NULL)  {
        sms_mem_remap_page[0] = &remap_page2_sms_bios;
        sms_mem_remap_page[1] = &remap_page0_sms_bios;
        sms_mem_remap_page[2] = &remap_page1_sms_bios;
        sms_mem_remap_page[3] = &remap_page2_sms_bios;
        sms_memctl = (SMS_MEMCTL_IO | SMS_MEMCTL_BIOS | SMS_MEMCTL_RAM) ^ 0xFF;
    }
    else if(sms_console == CONSOLE_GG && gg_bios_rom != NULL)   {
        sms_mem_remap_page[0] = &remap_page2;
        sms_mem_remap_page[1] = &sms_mem_remap_page0_gg_bios;
        sms_mem_remap_page[2] = &remap_page1;
        sms_mem_remap_page[3] = &remap_page2;
        sms_memctl = (SMS_MEMCTL_IO | SMS_MEMCTL_BIOS | SMS_MEMCTL_RAM) ^ 0xFF;
    }
    else    {
        sms_memctl = (SMS_MEMCTL_IO | SMS_MEMCTL_CART | SMS_MEMCTL_RAM) ^ 0xFF;
    }

    sms_ioctl_input_mask = 0xFFFF;
    sms_ioctl_output_mask = 0x0000;
    sms_ioctl_output_bits = 0x0000;

    reorganize_pages();
}
