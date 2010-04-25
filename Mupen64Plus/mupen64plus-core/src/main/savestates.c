/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - savestates.c                                            *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Olejl Tillin9                                      *
 *   Copyright (C) 2008 Richard42 Tillin9                                  *
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

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "api/config.h"

#include "savestates.h"
#include "main.h"
#include "rom.h"
#include "util.h"

#include "memory/memory.h"
#include "memory/flashram.h"
#include "r4300/macros.h"
#include "r4300/r4300.h"
#include "r4300/interupt.h"
#include "osal/preproc.h"
#include "osd/osd.h"

#include "main/zip/unzip.h"
#include "main/zip/zip.h"

const char* savestate_magic = "M64+SAVE";
const int savestate_version = 0x00010000;  /* 1.0 */
const int pj64_magic = 0x23D8A6C8;

extern unsigned int interp_addr;

int savestates_job = 0;

static unsigned int slot = 0;
static int autoinc_save_slot = 0;
static char fname[1024] = {0};

void savestates_select_slot(unsigned int s)
{
    if(s<0||s>9||s==slot)
        return;
    slot = s;
    ConfigSetParameter(g_CoreConfig, "CurrentSaveSlot", M64TYPE_INT, &s);
    StateChanged(M64CORE_SAVESTATE_SLOT, slot);

    if(rom)
        {
        char* filename = savestates_get_filename();
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "Selected state file: %s", filename);
        free(filename);
        }
    else
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "Selected state slot: %d", slot);
}

/* Returns the currently selected save slot. */
unsigned int savestates_get_slot(void)
{
    return slot;
}

/* Sets save state slot autoincrement on or off. */
void savestates_set_autoinc_slot(int b)
{
    autoinc_save_slot = b;
}

/* Returns save state slot autoincrement on or off. */
int savestates_get_autoinc_slot(void)
{
    return autoinc_save_slot != 0;
}

void savestates_inc_slot(void)
{
    if(++slot>9)
        slot = 0;
    StateChanged(M64CORE_SAVESTATE_SLOT, slot);
}

void savestates_select_filename(const char* fn)
{
   if(strlen((char*)fn)>=1024)
       return;
   strcpy(fname, fn);
}

char* savestates_get_filename()
{
    size_t length = strlen(ROM_SETTINGS.goodname)+4+1;
    char* filename = (char*)malloc(length);
    snprintf(filename, length, "%s.st%d", ROM_SETTINGS.goodname, slot);
    return filename;
}

char* savestates_get_pj64_filename()
{
    size_t length = strlen((char*)ROM_HEADER->nom)+8+1;
    char* filename = (char*)malloc(length);
    snprintf(filename, length, "%s.pj%d.zip", (char*)ROM_HEADER->nom, slot);
    return filename;
}

void savestates_save()
{
    char *filename, *file, buffer[1024];
    unsigned char outbuf[4];
    gzFile f;
    size_t length;
    int queuelength;

    if(autoinc_save_slot)
        savestates_inc_slot();

    if(fname[0]!=0)  /* A specific filename was given. */
        {
        char *handle;
        file = malloc(strlen(fname)+1);
        filename = malloc(strlen(fname)+1);
        strcpy(file, fname);
        handle = dirfrompath(file);
        strcpy(filename, fname+strlen(handle));
        free(handle);
        fname[0] = 0;
        }
    else
        {
        filename = savestates_get_filename();
        length = strlen(get_savespath())+strlen(filename)+1;
        file = malloc(length);
        snprintf(file, length, "%s%s", get_savespath(), filename);
        }

    f = gzopen(file, "wb");
    free(file);

    /* Write magic number. */
    gzwrite(f, savestate_magic, 8);

    /* Write savestate file version in big-endian. */
    outbuf[0] = (savestate_version >> 24) & 0xff;
    outbuf[1] = (savestate_version >> 16) & 0xff;
    outbuf[2] = (savestate_version >>  8) & 0xff;
    outbuf[3] = (savestate_version >>  0) & 0xff;
    gzwrite(f, outbuf, 4);

    gzwrite(f, ROM_SETTINGS.MD5, 32);

    gzwrite(f, &rdram_register, sizeof(RDRAM_register));
    gzwrite(f, &MI_register, sizeof(mips_register));
    gzwrite(f, &pi_register, sizeof(PI_register));
    gzwrite(f, &sp_register, sizeof(SP_register));
    gzwrite(f, &rsp_register, sizeof(RSP_register));
    gzwrite(f, &si_register, sizeof(SI_register));
    gzwrite(f, &vi_register, sizeof(VI_register));
    gzwrite(f, &ri_register, sizeof(RI_register));
    gzwrite(f, &ai_register, sizeof(AI_register));
    gzwrite(f, &dpc_register, sizeof(DPC_register));
    gzwrite(f, &dps_register, sizeof(DPS_register));
    gzwrite(f, rdram, 0x800000);
    gzwrite(f, SP_DMEM, 0x1000);
    gzwrite(f, SP_IMEM, 0x1000);
    gzwrite(f, PIF_RAM, 0x40);

    save_flashram_infos(buffer);
    gzwrite(f, buffer, 24);

    gzwrite(f, tlb_LUT_r, 0x100000*4);
    gzwrite(f, tlb_LUT_w, 0x100000*4);

    gzwrite(f, &llbit, 4);
    gzwrite(f, reg, 32*8);
    gzwrite(f, reg_cop0, 32*4);
    gzwrite(f, &lo, 8);
    gzwrite(f, &hi, 8);

    if ((Status & 0x04000000) == 0)
    {   // FR bit == 0 means 32-bit (MIPS I) FGR mode
        shuffle_fpr_data(0, 0x04000000);  // shuffle data into 64-bit register format for storage
        gzwrite(f, reg_cop1_fgr_64, 32*8);
        shuffle_fpr_data(0x04000000, 0);  // put it back in 32-bit mode
    }
    else
    {
        gzwrite(f, reg_cop1_fgr_64, 32*8);
    }

    gzwrite(f, &FCR0, 4);
    gzwrite(f, &FCR31, 4);
    gzwrite(f, tlb_e, 32*sizeof(tlb));
    if(r4300emu == CORE_PURE_INTERPRETER)
        gzwrite(f, &interp_addr, 4);
    else
        gzwrite(f, &PC->addr, 4);

    gzwrite(f, &next_interupt, 4);
    gzwrite(f, &next_vi, 4);
    gzwrite(f, &vi_field, 4);

    queuelength = save_eventqueue_infos(buffer);
    gzwrite(f, buffer, queuelength);

    gzclose(f);
    main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "Saved state to: %s", filename);
    free(filename);
}

void savestates_load()
{
    char *filename, *file, buffer[1024];
    unsigned char inbuf[4];
    gzFile f;
    size_t length;
    int queuelength, i;

    if(fname[0]!=0)  /* A specific filename was given. */
        {
        char *handle;
        file = malloc(strlen(fname)+1);
        filename = malloc(strlen(fname)+1);
        strcpy(file, fname);
        handle = dirfrompath(file);
        strcpy(filename, fname+strlen(handle));
        free(handle);
        }
    else
        {
        filename = savestates_get_filename();
        length = strlen(get_savespath())+strlen(filename)+1;
        file = malloc(length);
        snprintf(file, length, "%s%s", get_savespath(), filename);
        }

    f = gzopen(file, "rb");
    free(file);

    if(f==NULL)
        {
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "Could not open state file: %s", filename);
        free(filename);
        return;
        }

    /* Read and check Mupen64Plus magic number. */
    gzread(f, buffer, 8);
    if(strncmp(buffer, savestate_magic, 8)!=0)
        {
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "State file: %s is not a valid Mupen64plus savestate. Checking if in Project64 format...", filename);
        free(filename);
        gzclose(f);
        savestates_load_pj64();
        return;
        }

    fname[0] = 0;
    /* Read savestate file version in big-endian order. */
    gzread(f, inbuf, 4);
    i =            inbuf[0];
    i = (i << 8) | inbuf[1];
    i = (i << 8) | inbuf[2];
    i = (i << 8) | inbuf[3];
    if(i!=savestate_version)
        {
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "State version (%08x) doesn't match current version (%08x).", i, savestate_version);
        free(filename);
        gzclose(f);
        return;
        }

    gzread(f, buffer, 32);
    if(memcmp(buffer, ROM_SETTINGS.MD5, 32))
        {
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "State ROM MD5 does not match current ROM.");
        free(filename);
        gzclose(f);
        return;
        }

    gzread(f, &rdram_register, sizeof(RDRAM_register));
    gzread(f, &MI_register, sizeof(mips_register));
    gzread(f, &pi_register, sizeof(PI_register));
    gzread(f, &sp_register, sizeof(SP_register));
    gzread(f, &rsp_register, sizeof(RSP_register));
    gzread(f, &si_register, sizeof(SI_register));
    gzread(f, &vi_register, sizeof(VI_register));
    update_vi_status(vi_register.vi_status);
    update_vi_width(vi_register.vi_width);
    gzread(f, &ri_register, sizeof(RI_register));
    gzread(f, &ai_register, sizeof(AI_register));
    update_ai_dacrate(ai_register.ai_dacrate);
    gzread(f, &dpc_register, sizeof(DPC_register));
    gzread(f, &dps_register, sizeof(DPS_register));
    gzread(f, rdram, 0x800000);
    gzread(f, SP_DMEM, 0x1000);
    gzread(f, SP_IMEM, 0x1000);
    gzread(f, PIF_RAM, 0x40);

    gzread(f, buffer, 24);
    load_flashram_infos(buffer);

    gzread(f, tlb_LUT_r, 0x100000*4);
    gzread(f, tlb_LUT_w, 0x100000*4);

    gzread(f, &llbit, 4);
    gzread(f, reg, 32*8);
    gzread(f, reg_cop0, 32*4);
    set_fpr_pointers(Status);  // Status is reg_cop0[12]
    gzread(f, &lo, 8);
    gzread(f, &hi, 8);
    gzread(f, reg_cop1_fgr_64, 32*8);
    if ((Status & 0x04000000) == 0)  // 32-bit FPR mode requires data shuffling because 64-bit layout is always stored in savestate file
        shuffle_fpr_data(0x04000000, 0);
    gzread(f, &FCR0, 4);
    gzread(f, &FCR31, 4);
    gzread(f, tlb_e, 32*sizeof(tlb));
    if(r4300emu == CORE_PURE_INTERPRETER)
        gzread(f, &interp_addr, 4);
    else
        {
        int i;
        gzread(f, &queuelength, 4);
        for (i = 0; i < 0x100000; i++)
            invalid_code[i] = 1;
        jump_to(queuelength);
        }

    gzread(f, &next_interupt, 4);
    gzread(f, &next_vi, 4);
    gzread(f, &vi_field, 4);

    queuelength = 0;
    while(1)
        {
        gzread(f, buffer+queuelength, 4);
        if(*((unsigned int*)&buffer[queuelength])==0xFFFFFFFF)
            break;
        gzread(f, buffer+queuelength+4, 4);
        queuelength += 8;
        }
    load_eventqueue_infos(buffer);

    gzclose(f);
    if(r4300emu == CORE_PURE_INTERPRETER)
        last_addr = interp_addr;
    else
        last_addr = PC->addr;

    main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "State loaded from: %s", filename);
    free(filename);
    fname[0] = 0;
}

int savestates_save_pj64()
{
    char *file, *filename;
    unsigned int i, vi_timer, addr;
    int retval;
    size_t length;
    TLB_pj64 tlb_pj64[32];
    zipFile zipfile;
    zip_fileinfo zipfi;

    unsigned int dummy = 0;
    unsigned int SaveRDRAMSize = 0x800000;

    /* Continue only if VI / COMPARE
    Otherwise try again in a little while. */
    if (get_next_event_type() > COMPARE_INT)
       { return -1; }

    if(fname[0]!=0)  /* A specific filename was given. */
        {
        char *handle;
        file = malloc(strlen(fname)+1);
        filename = malloc(strlen(fname)+1);
        strcpy(file, fname);
        handle = dirfrompath(file);
        strcpy(filename, fname+strlen(handle));
        free(handle);
        }
    else
        {
        filename = savestates_get_pj64_filename();
        length = strlen(get_savespath())+strlen(filename)+1;
        file = malloc(length);
        snprintf(file, length, "%s%s", get_savespath(), filename);
        }

    zipfile = zipOpen(file, APPEND_STATUS_CREATE);
    free(file);

    if(zipfile==NULL)
        {
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "Could not create PJ64 state file: %s", filename);
        free(filename);
        return -2;
        }

    retval = zipOpenNewFileInZip(zipfile, filename, &zipfi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if(retval!=ZIP_OK)
        {
        zipClose(zipfile, "");
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "Zip error. Could not create state file: %s", filename);
        free(filename);
        return -3;
        }

    vi_timer = get_event(VI_INT) - reg_cop0[9]; /* Subtract current Count according to how PJ64 stores the timer. */

    if(r4300emu == CORE_PURE_INTERPRETER)
        addr = interp_addr;
    else
        addr = PC->addr;

    for (i=0; i < 32;i++)
        {
        tlb_pj64[i].BreakDownPageMask.Mask   = (unsigned int) tlb_e[i].mask;
        tlb_pj64[i].BreakDownEntryHi.VPN2    = (unsigned int) tlb_e[i].vpn2;
        tlb_pj64[i].BreakDownEntryLo0.GLOBAL = (unsigned int) tlb_e[i].g;
        tlb_pj64[i].BreakDownEntryLo1.GLOBAL = (unsigned int) tlb_e[i].g;
        tlb_pj64[i].BreakDownEntryHi.ASID    = (unsigned int) tlb_e[i].asid;
        tlb_pj64[i].BreakDownEntryLo0.PFN    = (unsigned int) tlb_e[i].pfn_even;
        tlb_pj64[i].BreakDownEntryLo0.C      = (unsigned int) tlb_e[i].c_even;
        tlb_pj64[i].BreakDownEntryLo0.D      = (unsigned int) tlb_e[i].d_even;
        tlb_pj64[i].BreakDownEntryLo0.V      = (unsigned int) tlb_e[i].v_even;
        tlb_pj64[i].BreakDownEntryLo1.PFN    = (unsigned int) tlb_e[i].pfn_odd;
        tlb_pj64[i].BreakDownEntryLo1.C      = (unsigned int) tlb_e[i].c_odd;
        tlb_pj64[i].BreakDownEntryLo1.D      = (unsigned int) tlb_e[i].d_odd;
        tlb_pj64[i].BreakDownEntryLo1.V      = (unsigned int) tlb_e[i].v_odd;
        }

    length = zipWriteInFileInZip(zipfile, &pj64_magic,                     4);
    length = zipWriteInFileInZip(zipfile, &SaveRDRAMSize,                  4);
    length = zipWriteInFileInZip(zipfile, rom,                             0x40);
    length = zipWriteInFileInZip(zipfile, &vi_timer,                       4);
    length = zipWriteInFileInZip(zipfile, &addr,                           4);
    length = zipWriteInFileInZip(zipfile, reg,                             32*8);
    length = zipWriteInFileInZip(zipfile, reg_cop1_fgr_64,                 32*8);
    length = zipWriteInFileInZip(zipfile, reg_cop0,                        32*4);
    length = zipWriteInFileInZip(zipfile, &FCR0,                           4);
    length = zipWriteInFileInZip(zipfile, &dummy,                          4*30);
    length = zipWriteInFileInZip(zipfile, &FCR31,                          4);
    length = zipWriteInFileInZip(zipfile, &hi,                             8);
    length = zipWriteInFileInZip(zipfile, &lo,                             8);
    length = zipWriteInFileInZip(zipfile, &rdram_register,                 sizeof(RDRAM_register));
    length = zipWriteInFileInZip(zipfile, &sp_register.sp_mem_addr_reg,    4);
    length = zipWriteInFileInZip(zipfile, &sp_register.sp_dram_addr_reg,   4);
    length = zipWriteInFileInZip(zipfile, &sp_register.sp_rd_len_reg,      4);
    length = zipWriteInFileInZip(zipfile, &sp_register.sp_wr_len_reg,      4);
    length = zipWriteInFileInZip(zipfile, &sp_register.sp_status_reg,      4);
    length = zipWriteInFileInZip(zipfile, &sp_register.sp_dma_full_reg,    4);
    length = zipWriteInFileInZip(zipfile, &sp_register.sp_dma_busy_reg,    4);
    length = zipWriteInFileInZip(zipfile, &sp_register.sp_semaphore_reg,   4);
    length = zipWriteInFileInZip(zipfile, &dummy,                          4); // SP_PC_REG
    length = zipWriteInFileInZip(zipfile, &dummy,                          4); // SP_IBIST_REG
    length = zipWriteInFileInZip(zipfile, &dpc_register.dpc_start,         4);
    length = zipWriteInFileInZip(zipfile, &dpc_register.dpc_end,           4);
    length = zipWriteInFileInZip(zipfile, &dpc_register.dpc_current,       4);
    length = zipWriteInFileInZip(zipfile, &dpc_register.dpc_status,        4);
    length = zipWriteInFileInZip(zipfile, &dpc_register.dpc_clock,         4);
    length = zipWriteInFileInZip(zipfile, &dpc_register.dpc_bufbusy,       4);
    length = zipWriteInFileInZip(zipfile, &dpc_register.dpc_pipebusy,      4);
    length = zipWriteInFileInZip(zipfile, &dpc_register.dpc_tmem,          4);
    length = zipWriteInFileInZip(zipfile, &dummy,                          4); // ?
    length = zipWriteInFileInZip(zipfile, &dummy,                          4); // ?
    length = zipWriteInFileInZip(zipfile, &MI_register.mi_init_mode_reg,   4); //TODO Secial handling in pj64
    length = zipWriteInFileInZip(zipfile, &MI_register.mi_version_reg,     4);
    length = zipWriteInFileInZip(zipfile, &MI_register.mi_intr_reg,        4);
    length = zipWriteInFileInZip(zipfile, &MI_register.mi_intr_mask_reg,   4);
    length = zipWriteInFileInZip(zipfile, &vi_register,                    4*14);
    length = zipWriteInFileInZip(zipfile, &ai_register,                    4*6);
    length = zipWriteInFileInZip(zipfile, &pi_register,                    sizeof(PI_register));
    length = zipWriteInFileInZip(zipfile, &ri_register,                    sizeof(RI_register));
    length = zipWriteInFileInZip(zipfile, &si_register,                    sizeof(SI_register));
    length = zipWriteInFileInZip(zipfile, tlb_pj64,                        sizeof(TLB_pj64)*32);
    length = zipWriteInFileInZip(zipfile, PIF_RAM,                         0x40);
    length = zipWriteInFileInZip(zipfile, rdram,                           0x800000);
    length = zipWriteInFileInZip(zipfile, SP_DMEM,                         0x1000);
    length = zipWriteInFileInZip(zipfile, SP_IMEM,                         0x1000);

    zipCloseFileInZip(zipfile);
    zipClose(zipfile, "");
    main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "Saved state to: %s", filename);
    free(filename);
    return 1;
}

void savestates_load_pj64()
{
    char *file, *filename, buffer[1024], RomHeader[64], szFileName[256], szExtraField[256], szComment[256];
    unsigned int magic, value, vi_timer, SaveRDRAMSize;
    unsigned char exit = 1;
    int queuelength, i;
    size_t length;
    unzFile zipstatefile;
    unz_file_info fileinfo;

    unsigned int *sp, *w_sp;
    unsigned int *dpc, *w_dpc;
    unsigned int *mi, *w_mi;
    TLB_pj64 tlb_pj64;

    if(fname[0]!=0)  /* A specific filename was given. */
        {
        char *handle;
        file = malloc(strlen(fname)+1);
        filename = malloc(strlen(fname)+1);
        strcpy(file, fname);
        handle = dirfrompath(file);
        strcpy(filename, fname+strlen(handle));
        free(handle);
        }
    else
        {
        filename = savestates_get_pj64_filename();
        length = strlen(get_savespath())+strlen(filename)+1;
        file = malloc(length);
        snprintf(file, length, "%s%s", get_savespath(), filename);
        }

    zipstatefile = unzOpen(file); /*Open the .zip file. */
    if(zipstatefile!=NULL)
        {
        if(unzGoToFirstFile(zipstatefile)==UNZ_OK)
            {
            if(unzGetCurrentFileInfo(zipstatefile, &fileinfo, szFileName, 255, szExtraField, 255, szComment, 255)==UNZ_OK)
                {
                if(unzOpenCurrentFile(zipstatefile)==UNZ_OK)
                    {
                    if(fileinfo.uncompressed_size>=4)
                        exit = 0;
                    }
                }
            }
        }

    if(exit)
        {
        unzClose(zipstatefile);
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "Zip error. Could not open state file: %s", file);
        free(file);
        return;
        }

    /* Read and check Project64 magic number. */
    unzReadCurrentFile(zipstatefile, &magic, 4);
    if (magic!=pj64_magic)
        {
        unzClose(zipstatefile);
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "State file: %s is not a valid Project64 savestate. Unrecognized file format.", file);
        free(file);
        return;
        }

    unzReadCurrentFile(zipstatefile, &SaveRDRAMSize, 4);

    unzReadCurrentFile(zipstatefile, RomHeader, 0x40);
    if(memcmp(RomHeader, rom, 0x40)!=0) 
        {
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "State ROM header does not match current ROM.");
        unzClose(zipstatefile);
        free(file);
        return;
        }

    // vi_timer
    unzReadCurrentFile(zipstatefile, &vi_timer,4);

    // Program Counter
    if(r4300emu == CORE_PURE_INTERPRETER)
        unzReadCurrentFile(zipstatefile, &interp_addr,4);
    else
        {
        unzReadCurrentFile(zipstatefile, &queuelength,4);
        for (i = 0; i < 0x100000; i++)
            { invalid_code[i] = 1; }
        jump_to(queuelength);
        }

    // GPR
    unzReadCurrentFile(zipstatefile, reg,8*32);

    // FPR
    unzReadCurrentFile(zipstatefile, reg_cop1_fgr_64,8*32);

    // CP0
    unzReadCurrentFile(zipstatefile, reg_cop0, 4*32);

    set_fpr_pointers(Status);  // Status is reg_cop0[12]

    // Initialze the interupts
    vi_timer += reg_cop0[9]; // Add current Count
    next_interupt = vi_timer;
    next_vi = vi_timer;
    vi_field = 0;
    *((unsigned int*)&buffer[0]) = VI_INT;
    *((unsigned int*)&buffer[4]) = vi_timer;
    *((unsigned int*)&buffer[8]) = 0xFFFFFFFF;

    load_eventqueue_infos(buffer);

    // FPCR
    unzReadCurrentFile(zipstatefile, &FCR0,4);
    unzReadCurrentFile(zipstatefile, &buffer,120);   // Dummy read.
    unzReadCurrentFile(zipstatefile, &FCR31,4);

    // hi / lo
    unzReadCurrentFile(zipstatefile,&hi,8);
    unzReadCurrentFile(zipstatefile,&lo,8);

    // rdram register
    unzReadCurrentFile(zipstatefile, &rdram_register, sizeof(RDRAM_register));

    // sp_register
    unzReadCurrentFile(zipstatefile, &sp_register.sp_mem_addr_reg, 4);
    unzReadCurrentFile(zipstatefile, &sp_register.sp_dram_addr_reg, 4);
    unzReadCurrentFile(zipstatefile, &sp_register.sp_rd_len_reg, 4);
    unzReadCurrentFile(zipstatefile, &sp_register.sp_wr_len_reg, 4);
    unzReadCurrentFile(zipstatefile, &sp_register.sp_status_reg, 4);
    unzReadCurrentFile(zipstatefile, &sp_register.sp_dma_full_reg, 4);
    unzReadCurrentFile(zipstatefile, &sp_register.sp_dma_busy_reg, 4);
    unzReadCurrentFile(zipstatefile, &sp_register.sp_semaphore_reg, 4);
    unzReadCurrentFile(zipstatefile, &value, 4); // SP_PC_REG -> Not part of mupen savestate. Dummy read.
    unzReadCurrentFile(zipstatefile, &value, 4); // SP_IBIST_REG -> Not part of mupen savestate. Dummy read.

    w_sp = &sp_register.w_sp_status_reg; // Only done to reduce the amount
    sp = &sp_register.sp_status_reg;     // of character in the below ifs / elses
    *w_sp = 0;
    if ((*sp & 0x0001) == 0) { *w_sp |= 0x0000001; }
    else                     { *w_sp |= 0x0000002; }
    if ((*sp & 0x0002) == 0) { *w_sp |= 0x0000004; }
    if ((*sp & 0x001c) == 0) { *w_sp |= 0x0000008; } // TODO: Unsecure if this is correct
    else                     { *w_sp |= 0x0000010; } // TODO:  --- " ---
    if ((*sp & 0x0020) == 0) { *w_sp |= 0x0000020; }
    else                     { *w_sp |= 0x0000040; }
    if ((*sp & 0x0040) == 0) { *w_sp |= 0x0000080; }
    else                     { *w_sp |= 0x0000100; }
    if ((*sp & 0x0080) == 0) { *w_sp |= 0x0000200; }
    else                     { *w_sp |= 0x0000400; }
    if ((*sp & 0x0100) == 0) { *w_sp |= 0x0000800; }
    else                     { *w_sp |= 0x0001000; }
    if ((*sp & 0x0200) == 0) { *w_sp |= 0x0002000; }
    else                     { *w_sp |= 0x0004000; }
    if ((*sp & 0x0400) == 0) { *w_sp |= 0x0008000; }
    else                     { *w_sp |= 0x0010000; }
    if ((*sp & 0x0800) == 0) { *w_sp |= 0x0020000; }
    else                     { *w_sp |= 0x0040000; }
    if ((*sp & 0x1000) == 0) { *w_sp |= 0x0080000; }
    else                     { *w_sp |= 0x0100000; }
    if ((*sp & 0x2000) == 0) { *w_sp |= 0x0200000; }
    else                     { *w_sp |= 0x0400000; }
    if ((*sp & 0x4000) == 0) { *w_sp |= 0x0800000; }
    else                     { *w_sp |= 0x1000000; }
    update_SP();

    // dpc_register
    unzReadCurrentFile(zipstatefile, &dpc_register.dpc_start, 4);
    unzReadCurrentFile(zipstatefile, &dpc_register.dpc_end, 4);
    unzReadCurrentFile(zipstatefile, &dpc_register.dpc_current, 4);
    unzReadCurrentFile(zipstatefile, &dpc_register.dpc_status, 4);
    unzReadCurrentFile(zipstatefile, &dpc_register.dpc_clock, 4);
    unzReadCurrentFile(zipstatefile, &dpc_register.dpc_bufbusy, 4);
    unzReadCurrentFile(zipstatefile, &dpc_register.dpc_pipebusy, 4);
    unzReadCurrentFile(zipstatefile, &dpc_register.dpc_tmem, 4);
    unzReadCurrentFile(zipstatefile, &value, 4); // Dummy read
    unzReadCurrentFile(zipstatefile, &value, 4); // Dummy read
    w_dpc = &dpc_register.w_dpc_status;
    dpc = &dpc_register.dpc_status;
    *w_dpc = 0;
    if ((*dpc & 0x0001) == 0) { *w_dpc |= 0x0000001; }
    else                      { *w_dpc |= 0x0000002; }
    if ((*dpc & 0x0002) == 0) { *w_dpc |= 0x0000004; }
    else                      { *w_dpc |= 0x0000008; }
    if ((*dpc & 0x0004) == 0) { *w_dpc |= 0x0000010; }
    else                      { *w_dpc |= 0x0000020; }
    update_DPC();

    // mi_register
    unzReadCurrentFile(zipstatefile, &MI_register.mi_init_mode_reg, 4);
    unzReadCurrentFile(zipstatefile, &MI_register.mi_version_reg, 4);
    unzReadCurrentFile(zipstatefile, &MI_register.mi_intr_reg, 4);
    unzReadCurrentFile(zipstatefile, &MI_register.mi_intr_mask_reg, 4);
    MI_register.w_mi_init_mode_reg = MI_register.mi_init_mode_reg & 0x7F;
    if ((MI_register.mi_init_mode_reg & 0x080) == 0)
        { MI_register.w_mi_init_mode_reg |= 0x0000080;
    } else {
        MI_register.w_mi_init_mode_reg |= 0x0000100;
    }
    if ((MI_register.mi_init_mode_reg & 0x200) == 0) {
        MI_register.w_mi_init_mode_reg |= 0x0001000;
    } else {
        MI_register.w_mi_init_mode_reg |= 0x0002000;
    }
    // TODO: Unsecure about 'clear DP interrupt' (MI_register.w_mi_init_mode_reg[11])
    update_MI_intr_mask_reg();
    w_mi = (unsigned int *) &MI_register.w_mi_intr_mask_reg;
    mi = (unsigned int *) &MI_register.mi_intr_mask_reg;
    *w_mi = 0;
    if ((*mi & 0x01) == 0) { *w_mi |= 0x0000001; }
    else                   { *w_mi |= 0x0000002; }
    if ((*mi & 0x02) == 0) { *w_mi |= 0x0000004; }
    else                   { *w_mi |= 0x0000008; }
    if ((*mi & 0x04) == 0) { *w_mi |= 0x0000010; }
    else                   { *w_mi |= 0x0000020; }
    if ((*mi & 0x08) == 0) { *w_mi |= 0x0000040; }
    else                   { *w_mi |= 0x0000080; }
    if ((*mi & 0x10) == 0) { *w_mi |= 0x0000100; }
    else                   { *w_mi |= 0x0000200; }
    if ((*mi & 0x20) == 0) { *w_mi |= 0x0000400; }
    else                   { *w_mi |= 0x0000800; }
    update_MI_init_mode_reg();

    // vi_register 
    unzReadCurrentFile(zipstatefile, &vi_register, 4*14);

    // ai_register
    unzReadCurrentFile(zipstatefile, &ai_register, 4*6);

    // TODO: Not avialable in PJ64 savestate. Leave it as is.
    // ai_register.next_delay = 0; ai_register.next_len = 0;
    // ai_register.current_delay = 0; ai_register.current_len = 0;

    // pi_register
    unzReadCurrentFile(zipstatefile, &pi_register, sizeof(PI_register));

    // ri_register
    unzReadCurrentFile(zipstatefile, &ri_register, sizeof(RI_register));

    // si_register
    unzReadCurrentFile(zipstatefile, &si_register, sizeof(SI_register));

    // tlb
    memset(tlb_LUT_r, 0, 0x400000);
    memset(tlb_LUT_w, 0, 0x400000);
    for (i=0; i < 32; i++)
        {
        unsigned int j;

        unzReadCurrentFile(zipstatefile, &tlb_pj64, sizeof(TLB_pj64));
        tlb_e[i].mask = (short) tlb_pj64.BreakDownPageMask.Mask;
        tlb_e[i].vpn2 = tlb_pj64.BreakDownEntryHi.VPN2;
        tlb_e[i].g = (char) tlb_pj64.BreakDownEntryLo0.GLOBAL & tlb_pj64.BreakDownEntryLo1.GLOBAL;
        tlb_e[i].asid = (unsigned char) tlb_pj64.BreakDownEntryHi.ASID;
        tlb_e[i].pfn_even = tlb_pj64.BreakDownEntryLo0.PFN;
        tlb_e[i].c_even = (char) tlb_pj64.BreakDownEntryLo0.C;
        tlb_e[i].d_even = (char) tlb_pj64.BreakDownEntryLo0.D;
        tlb_e[i].v_even = (char) tlb_pj64.BreakDownEntryLo0.V;
        tlb_e[i].pfn_odd = tlb_pj64.BreakDownEntryLo1.PFN;
        tlb_e[i].c_odd = (char) tlb_pj64.BreakDownEntryLo1.C;
        tlb_e[i].d_odd = (char) tlb_pj64.BreakDownEntryLo1.D;
        tlb_e[i].v_odd = (char) tlb_pj64.BreakDownEntryLo1.V;

        // This is copied from TLBWI instruction
        // tlb_e[i].r = 0;
        tlb_e[i].start_even = (unsigned int) tlb_e[i].vpn2 << 13;
        tlb_e[i].end_even = (unsigned int) tlb_e[i].start_even + (tlb_e[i].mask << 12) + 0xFFF;
        tlb_e[i].phys_even = (unsigned int) tlb_e[i].pfn_even << 12;;
        tlb_e[i].start_odd = (unsigned int) tlb_e[i].end_even + 1;
        tlb_e[i].end_odd = (unsigned int) tlb_e[i].start_odd + (tlb_e[i].mask << 12) + 0xFFF;;
        tlb_e[i].phys_odd = (unsigned int) tlb_e[i].pfn_odd << 12;

        if (tlb_e[i].v_even)
        {
            if (tlb_e[i].start_even < tlb_e[i].end_even &&
                !(tlb_e[i].start_even >= 0x80000000 && tlb_e[i].end_even < 0xC0000000) &&
                tlb_e[i].phys_even < 0x20000000)
            {
                for (j=tlb_e[i].start_even;j<tlb_e[i].end_even;j++)
                    tlb_LUT_r[j>>12] = 0x80000000 | (tlb_e[i].phys_even + 
                                                     (j - tlb_e[i].start_even));
                if (tlb_e[i].d_even)
                    for (j=tlb_e[i].start_even;j<tlb_e[i].end_even;j++)
                        tlb_LUT_w[j>>12] = 0x80000000 | (tlb_e[i].phys_even + (j - tlb_e[i].start_even));
            }
            for (j=tlb_e[i].start_even>>12; j<=tlb_e[i].end_even>>12; j++)
            {
                if(blocks[j] && blocks[j]->adler32)
                {
                    if(blocks[j]->adler32 == 
                       adler32(0,(const Bytef*)&rdram[(tlb_LUT_r[j]&0x7FF000)/4],0x1000))
                        invalid_code[j] = 0;
                }
            }
        }

        if (tlb_e[i].v_odd)
        {
            if (tlb_e[i].start_odd < tlb_e[i].end_odd &&
                !(tlb_e[i].start_odd >= 0x80000000 &&
                  tlb_e[i].end_odd < 0xC0000000) &&
                tlb_e[i].phys_odd < 0x20000000)
            {
                for (j=tlb_e[i].start_odd;j<tlb_e[i].end_odd;j++)
                    tlb_LUT_r[j>>12] = 0x80000000 | (tlb_e[i].phys_odd + (j - tlb_e[i].start_odd));
                if (tlb_e[i].d_odd)
                    for (j=tlb_e[i].start_odd;j<tlb_e[i].end_odd;j++)
                        tlb_LUT_w[j>>12] = 0x80000000 | (tlb_e[i].phys_odd + (j - tlb_e[i].start_odd));
            }
            for (j=tlb_e[i].start_odd>>12; j<=tlb_e[i].end_odd>>12; j++)
            {
                if(blocks[j] && blocks[j]->adler32)
                {
                    if(blocks[j]->adler32 == adler32(0,(const Bytef*)&rdram[(tlb_LUT_r[j]&0x7FF000)/4],0x1000))
                        invalid_code[j] = 0;
                }
            }
        }
    }

    // pif ram
    unzReadCurrentFile(zipstatefile, PIF_RAM, 0x40);

    // RDRAM
    memset(rdram, 0, 0x800000);
    unzReadCurrentFile(zipstatefile, rdram, SaveRDRAMSize);

    // DMEM
    unzReadCurrentFile(zipstatefile, SP_DMEM, 0x1000);

    // IMEM
    unzReadCurrentFile(zipstatefile, SP_IMEM, 0x1000);

    // TODO: The following is not available in PJ64 savestate. Keep the values as is.
    // rsp_register.rsp_pc = 0; rsp_register.rsp_ibist = 0; dps_register.dps_tbist = 0; dps_register.dps_test_mode = 0;
    // dps_register.dps_buftest_addr = 0; dps_register.dps_buftest_data = 0; llbit = 0;

    // No flashram info in pj64 savestate.
    init_flashram();

    unzClose(zipstatefile);
    if(r4300emu == CORE_PURE_INTERPRETER)
        last_addr = interp_addr;
    else
        last_addr = PC->addr;

    main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "State loaded from: %s", filename);
    free(filename);
    fname[0] = 0;
}

