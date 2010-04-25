/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - savestate_convert.c                                     *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Richard Goedeken                                   *
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

#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* savestate file header: magic number and version number */
const char *savestate_magic = "M64+SAVE";
const int savestate_newest_version = 0x00010000;  // 1.0

/* Data field lengths */

#define SIZE_REG_RDRAM       40
#define SIZE_REG_MIPS        36
#define SIZE_REG_PI          52
#define SIZE_REG_SP          52
#define SIZE_REG_RSP         8
#define SIZE_REG_SI          16
#define SIZE_REG_VI          60
#define SIZE_REG_RI          32
#define SIZE_REG_AI          40
#define SIZE_REG_DPC         48
#define SIZE_REG_DPS         16

#define SIZE_FLASHRAM_INFO   24
#define SIZE_TLB_ENTRY       52

#define SIZE_MAX_EVENTQUEUE  1024

/* Arrays and pointers for savestate data */

char rom_md5[32];

char rdram_register[SIZE_REG_RDRAM];
char mips_register[SIZE_REG_MIPS];
char pi_register[SIZE_REG_PI];
char sp_register[SIZE_REG_SP];
char rsp_register[SIZE_REG_RSP];
char si_register[SIZE_REG_SI];
char vi_register[SIZE_REG_VI];
char ri_register[SIZE_REG_RI];
char ai_register[SIZE_REG_AI];
char dpc_register[SIZE_REG_DPC];
char dps_register[SIZE_REG_DPS];

char *rdram;       /* 0x800000 bytes */
char SP_DMEM[0x1000];
char SP_IMEM[0x1000];
char PIF_RAM[0x40];

char flashram[SIZE_FLASHRAM_INFO];

char *tlb_LUT_r;   /* 0x400000 bytes */
char *tlb_LUT_w;   /* 0x400000 bytes */

char llbit[4];
char reg[32][8];
char reg_cop0[32][4];
char lo[8];
char hi[8];
char reg_cop1_fgr_64[32][8];
char FCR0[4];
char FCR31[4];
char tlb_e[32][SIZE_TLB_ENTRY];
char PCaddr[4];

char next_interupt[4];
char next_vi[4];
char vi_field[4];

char eventqueue[SIZE_MAX_EVENTQUEUE];

/* savestate data parameters calculated from file contents */
int queuelength = 0;

/* Forward declarations for functions */
void printhelp(const char *progname);

int allocate_memory(void);
void free_memory(void);

int load_original_mupen64(const char *filename);
int save_newest(const char *filename);

/* Main Function - parse arguments, check version, load state file, overwrite state file with new one */
int main(int argc, char *argv[])
{
    FILE *pfTest;
    gzFile f;
    char *filename;
    char magictag[8];
    unsigned char inbuf[4];
    int (*load_function)(const char *) = NULL;
    int iVersion;

    /* start by parsing the command-line arguments */
    if (argc != 2 || strncmp(argv[1], "-h", 2) == 0 || strncmp(argv[1], "--help", 6) == 0)
    {
        printhelp(argv[0]);
        return 1;
    }
    filename = argv[1];
    pfTest = fopen(filename, "rb");
    if (pfTest == NULL)
    {
        printf("Error: cannot open savestate file '%s' for reading.\n", filename);
        return 2;
    }
    fclose(pfTest);

    /* try to determine the version of this savestate file */
    f = gzopen(filename, "rb");
    if (f == NULL)
    {
        printf("Error: state file '%s' is corrupt\n", filename);
        return 3;
    }
    if (gzread(f, magictag, 8) != 8 || gzread(f, inbuf, 4) != 4)
    {
        printf("Error: state file '%s' is corrupt: end of savestate file while reading header.\n", filename);
        gzclose(f);
        return 4;
    }
    gzclose(f);
    iVersion =                   inbuf[0];
    iVersion = (iVersion << 8) | inbuf[1];
    iVersion = (iVersion << 8) | inbuf[2];
    iVersion = (iVersion << 8) | inbuf[3];

    /* determine which type of savestate file to load, based on savestate version */
    if (strncmp(magictag, savestate_magic, 8) != 0)
    {
        printf("Warning: old savestate file format.  This is presumed to be from the original Mupen64 or Mupen64Plus version 1.4 or earlier.\n");
        load_function = load_original_mupen64;
    }
    else if (iVersion == savestate_newest_version)
    {
        printf("This savestate file is already up to date (version %08x)\n", savestate_newest_version);
        return 0;
    }
    else
    {
        printf("This savestate file uses an unknown version (%08x)\n", iVersion);
        return 5;
    }

    /* allocate memory for savestate data */
    if (allocate_memory() != 0)
    {
        printf("Error: couldn't allocate memory for savestate data storage.\n");
        return 6;
    }

    /* load the savestate file */
    if (load_function(filename) != 0)
    {
        free_memory();
        return 7;
    }

    /* write new updated savestate file */
    if (save_newest(filename) != 0)
    {
        free_memory();
        return 8;
    }

    /* free the memory and return */
    printf("Savestate file '%s' successfully converted to latest version (%08x).\n", filename, savestate_newest_version);
    free_memory();
    return 0;
}

void printhelp(const char *progname)
{
    printf("%s - convert older Mupen64Plus savestate files to most recent version.\n\n", progname);
    printf("Usage: %s [-h] [--help] <savestatepath>\n\n", progname);
    printf("       -h, --help: display this message\n");
    printf("       <savestatepath>: full path to savestate file which will be overwritten with latest version.\n");
}

int allocate_memory(void)
{
    rdram = malloc(0x800000);
    if (rdram == NULL)
        return 1;

    tlb_LUT_r = malloc(0x400000);
    if (tlb_LUT_r == NULL)
    {
        free_memory();
        return 2;
    }

    tlb_LUT_w = malloc(0x400000);
    if (tlb_LUT_w == NULL)
    {
        free_memory();
        return 3;
    }

    return 0;
}

void free_memory(void)
{
    if (rdram != NULL)
    {
        free(rdram);
        rdram = NULL;
    }

    if (tlb_LUT_r != NULL)
    {
        free(tlb_LUT_r);
        tlb_LUT_r = NULL;
    }

    if (tlb_LUT_w != NULL)
    {
        free(tlb_LUT_w);
        tlb_LUT_w = NULL;
    }
}

/* State Loading Functions */
int load_original_mupen64(const char *filename)
{
    char buffer[4];
    int i;
    gzFile f;

    f = gzopen(filename, "rb");

    if (f == NULL)
    {
        printf("Error: savestate file '%s' is corrupt.\n", filename);
        return 1;
    }

    gzread(f, rom_md5, 32);

    gzread(f, rdram_register, SIZE_REG_RDRAM);
    gzread(f, mips_register, SIZE_REG_MIPS);
    gzread(f, pi_register, SIZE_REG_PI);
    gzread(f, sp_register, SIZE_REG_SP);
    gzread(f, rsp_register, SIZE_REG_RSP);
    gzread(f, si_register, SIZE_REG_SI);
    gzread(f, vi_register, SIZE_REG_VI);
    gzread(f, ri_register, SIZE_REG_RI);
    gzread(f, ai_register, SIZE_REG_AI);
    gzread(f, dpc_register, SIZE_REG_DPC);
    gzread(f, dps_register, SIZE_REG_DPS);

    gzread(f, rdram, 0x800000);
    gzread(f, SP_DMEM, 0x1000);
    gzread(f, SP_IMEM, 0x1000);
    gzread(f, PIF_RAM, 0x40);

    gzread(f, flashram, SIZE_FLASHRAM_INFO);

    memset(tlb_LUT_r, 0, 0x400000);
    memset(tlb_LUT_w, 0, 0x400000);
    gzread(f, tlb_LUT_r, 0x100000);
    gzread(f, tlb_LUT_w, 0x100000);

    gzread(f, llbit, 4);
    gzread(f, reg, 32*8);
    for (i = 0; i < 32; i++)
    {
        gzread(f, reg_cop0[i], 4);
        gzread(f, buffer, 4); /* for compatibility with older versions. */
    }
    gzread(f, lo, 8);
    gzread(f, hi, 8);
    gzread(f, reg_cop1_fgr_64[0], 32 * 8);
    gzread(f, FCR0, 4);
    gzread(f, FCR31, 4);
    gzread(f, tlb_e[0], 32 * SIZE_TLB_ENTRY);
    gzread(f, PCaddr, 4);

    gzread(f, next_interupt, 4);
    gzread(f, next_vi, 4);
    gzread(f, vi_field, 4);

    queuelength = 0;
    while(queuelength < SIZE_MAX_EVENTQUEUE)
    {
        if (gzread(f, eventqueue + queuelength, 4) != 4)
        {
            printf("Error: savestate file '%s' is corrupt.\n", filename);
            return 2;
        }
        if (*((unsigned int*) &eventqueue[queuelength]) == 0xFFFFFFFF)
        {
            queuelength += 4;
            break;
        }
        gzread(f, eventqueue + queuelength + 4, 4);
        queuelength += 8;
    }

    if (queuelength >= SIZE_MAX_EVENTQUEUE)
    {
        printf("Error: savestate file '%s' has event queue larger than %i bytes.\n", filename, SIZE_MAX_EVENTQUEUE);
        return 3;
    }

    gzclose(f);
    return 0;
}

/* State Saving Functions */

int save_newest(const char *filename)
{
    unsigned char outbuf[4];
    gzFile f;

    f = gzopen(filename, "wb");

    /* write magic number */
    gzwrite(f, savestate_magic, 8);

    /* write savestate file version in big-endian */
    outbuf[0] = (savestate_newest_version >> 24) & 0xff;
    outbuf[1] = (savestate_newest_version >> 16) & 0xff;
    outbuf[2] = (savestate_newest_version >>  8) & 0xff;
    outbuf[3] = (savestate_newest_version >>  0) & 0xff;
    gzwrite(f, outbuf, 4);

    gzwrite(f, rom_md5, 32);

    gzwrite(f, rdram_register, SIZE_REG_RDRAM);
    gzwrite(f, mips_register, SIZE_REG_MIPS);
    gzwrite(f, pi_register, SIZE_REG_PI);
    gzwrite(f, sp_register, SIZE_REG_SP);
    gzwrite(f, rsp_register, SIZE_REG_RSP);
    gzwrite(f, si_register, SIZE_REG_SI);
    gzwrite(f, vi_register, SIZE_REG_VI);
    gzwrite(f, ri_register, SIZE_REG_RI);
    gzwrite(f, ai_register, SIZE_REG_AI);
    gzwrite(f, dpc_register, SIZE_REG_DPC);
    gzwrite(f, dps_register, SIZE_REG_DPS);

    gzwrite(f, rdram, 0x800000);
    gzwrite(f, SP_DMEM, 0x1000);
    gzwrite(f, SP_IMEM, 0x1000);
    gzwrite(f, PIF_RAM, 0x40);

    gzwrite(f, flashram, SIZE_FLASHRAM_INFO);

    gzwrite(f, tlb_LUT_r, 0x400000);
    gzwrite(f, tlb_LUT_w, 0x400000);

    gzwrite(f, llbit, 4);
    gzwrite(f, reg[0], 32*8);
    gzwrite(f, reg_cop0[0], 32*4);
    gzwrite(f, lo, 8);
    gzwrite(f, hi, 8);
    gzwrite(f, reg_cop1_fgr_64[0], 32*8);
    gzwrite(f, FCR0, 4);
    gzwrite(f, FCR31, 4);
    gzwrite(f, tlb_e[0], 32 * SIZE_TLB_ENTRY);
    gzwrite(f, PCaddr, 4);

    gzwrite(f, next_interupt, 4);
    gzwrite(f, next_vi, 4);
    gzwrite(f, vi_field, 4);

    gzwrite(f, eventqueue, queuelength);

    gzclose(f);
    return 0;
}

