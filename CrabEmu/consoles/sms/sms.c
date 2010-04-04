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

#include <stdio.h>
#include <string.h>
#include "sms.h"
#include "smsvdp.h"
#include "smsmem.h"
#include "smsmem-gg.h"
#include "smsz80.h"
#include "sn76489.h"
#include "sound.h"

uint16 sms_pad = 0xFFFF;
int sms_psg_enabled = 1;
int sms_initialized = 0;
sn76489_t psg;
int sms_console = CONSOLE_SMS;
int sms_region = SMS_REGION_EXPORT;
int sms_cycles_run;
int sms_cycles_to_run;

uint32 psg_samples[313];

static const float NTSC_Z80_CLOCK = 3579545.0f;
static const int PSG_DIVISOR = 16;
static const int NTSC_FPS = 60;
static const int NTSC_LINES_PER_FRAME = 262;
static const float NTSC_CLOCKS_PER_SAMPLE = 5.073051303855f;

static const float PAL_Z80_CLOCK = 3546893.0f;
static const int PAL_FPS = 50;
static const int PAL_LINES_PER_FRAME = 313;
static const float PAL_CLOCKS_PER_SAMPLE = 5.02677579365f;

extern uint8 sms_gg_regs[7];

int sms_init(int video_system, int region)  {
    int i;
    float tmp;

    if(video_system == SMS_VIDEO_NTSC)   {
        tmp = NTSC_Z80_CLOCK / PSG_DIVISOR / NTSC_FPS / NTSC_LINES_PER_FRAME /
              NTSC_CLOCKS_PER_SAMPLE;

        for(i = 0; i < NTSC_LINES_PER_FRAME; ++i)   {
            psg_samples[i] = (uint32) (tmp * (i + 1)) -
                             (uint32) (tmp * i);
        }

        /* We end up generating 734 samples per frame @ 44100 Hz, 60fps, but we
           need 735. */
        psg_samples[261] += 1;

        region |= SMS_VIDEO_NTSC;

        sn76489_init(&psg, NTSC_Z80_CLOCK, 44100.0f,
                     SN76489_NOISE_BITS_SMS, SN76489_NOISE_TAPPED_SMS);
    }
    else    {
        tmp = PAL_Z80_CLOCK / PSG_DIVISOR / PAL_FPS / PAL_LINES_PER_FRAME /
             PAL_CLOCKS_PER_SAMPLE;

        for(i = 0; i < PAL_LINES_PER_FRAME; ++i)    {
            psg_samples[i] = (uint32) (tmp * (i + 1)) -
                             (uint32) (tmp * i);            
        }

        /* We need 882 samples per frame @ 44100 Hz, 50fps. */
        region |= SMS_VIDEO_PAL;

        sn76489_init(&psg, PAL_Z80_CLOCK, 44100.0f,
                     SN76489_NOISE_BITS_SMS, SN76489_NOISE_TAPPED_SMS);
    }

    sms_region = region;

    sms_mem_init();
    sms_vdp_init(video_system);
    sms_z80_init();

    sound_init();

    sms_initialized = 1;

    return 0;
}

int sms_reset() {
    if(sms_initialized == 0)
        return 0;

    if(sms_region & SMS_VIDEO_NTSC) {
        sn76489_init(&psg, NTSC_Z80_CLOCK, 44100.0f,
                     SN76489_NOISE_BITS_SMS, SN76489_NOISE_TAPPED_SMS);
    }
    else    {
        sn76489_init(&psg, PAL_Z80_CLOCK, 44100.0f,
                     SN76489_NOISE_BITS_SMS, SN76489_NOISE_TAPPED_SMS);
    }

    sound_reset_buffer();

    sms_mem_shutdown();
    sms_mem_init();

    sms_z80_reset();
    sms_vdp_reset();

    return 0;
}

void sms_soft_reset()   {
    if(sms_initialized == 0)
        return;

    sound_reset_buffer();

    sms_mem_reset();
    sms_z80_reset();
    sms_vdp_reset();
}

int sms_shutdown()  {
    sms_mem_shutdown();
    sms_vdp_shutdown();
    sms_z80_shutdown();
    sound_shutdown();

    return 0;
}

#ifndef _arch_dreamcast
int sms_frame(int run)  {
    int16 buf[882 << 1];
    int line, total_lines;

    sms_cycles_run = run;
    sms_cycles_to_run = 0;

    if(sms_region & SMS_VIDEO_NTSC)
        total_lines = NTSC_LINES_PER_FRAME;
    else
        total_lines = PAL_LINES_PER_FRAME;

    for(line = 0; line < total_lines; ++line)   {
        sms_cycles_to_run += SMS_CYCLES_PER_LINE;
        sms_cycles_run += sms_vdp_execute(line);

        sms_cycles_run += sms_z80_run(sms_cycles_to_run - sms_cycles_run);

        if(sms_psg_enabled) {
            sn76489_execute_samples(&psg, buf, psg_samples[line]);
#ifndef TIMING_TEST
            sound_update_buffer(buf, psg_samples[line] << 2);
#endif
        }
    }

    return sms_cycles_run - sms_cycles_to_run;
}
#endif

void sms_button_pressed(int button) {
    if(button == GG_START)  {
        if(sms_console == CONSOLE_GG)   {
            sms_gg_regs[0] &= 0x7F;
        }
    }
    else    {
        sms_pad &= ~button;
    }
}

void sms_button_released(int button)    {
    if(button == GG_START)  {
        if(sms_console == CONSOLE_GG)   {
            sms_gg_regs[0] |= 0x80;
        }
    }
    else    {
        sms_pad |= button;
    }
}

void sms_set_console(int console)   {
    switch(console) {
        case CONSOLE_SMS:
            gui_set_aspect(4.0f, 3.0f);
            psg.noise_tapped = SN76489_NOISE_TAPPED_SMS;
            sms_z80_set_pread(&sms_port_read);
            sms_z80_set_pwrite(&sms_port_write);
            break;

        case CONSOLE_GG:
            gui_set_aspect(10.0f, 9.0f);
            psg.noise_tapped = SN76489_NOISE_TAPPED_SMS;
            sms_z80_set_pread(&sms_gg_port_read);
            sms_z80_set_pwrite(&sms_gg_port_write);
            break;

        case CONSOLE_SG1000:
        case CONSOLE_SC3000:
            gui_set_aspect(4.0f, 3.0f);
            psg.noise_tapped = SN76489_NOISE_TAPPED_SG1000;
            sms_z80_set_pread(&sms_port_read);
            sms_z80_set_pwrite(&sms_port_write);
            break;

        default:
            return;
    }

    sms_console = console;
}

void sms_psg_write_context(FILE *fp) {
    uint8 byte;
    int i;
    uint16 counter;

    fwrite(psg.volume, 4, 1, fp);

    for(i = 0; i < 3; ++i)  {
        byte = psg.tone[i] & 0xFF;
        fwrite(&byte, 1, 1, fp);
        byte = (psg.tone[i] >> 8) & 0xFF;
        fwrite(&byte, 1, 1, fp);
    }

    fwrite(&psg.noise, 1, 1, fp);
    fwrite(psg.tone_state, 4, 1, fp);
    fwrite(&psg.latched_reg, 1, 1, fp);

    for(i = 0; i < 4; ++i)  {
        counter = (uint16) psg.counter[i];
        byte = counter & 0xFF;
        fwrite(&byte, 1, 1, fp);
        byte = (counter >> 8) & 0xFF;
        fwrite(&byte, 1, 1, fp);
    }
}

void sms_psg_read_context(FILE *fp) {
    uint8 byte[2];
    int i;
    uint16 counter;

    fread(psg.volume, 4, 1, fp);

    for(i = 0; i < 3; ++i)  {
        fread(&byte, 2, 1, fp);
        psg.tone[i] = byte[0] | (byte[1] << 8);
    }

    fread(&psg.noise, 1, 1, fp);
    fread(psg.tone_state, 4, 1, fp);
    fread(&psg.latched_reg, 1, 1, fp);

    for(i = 0; i < 4; ++i)  {
        fread(&byte, 2, 1, fp);
        counter = byte[0] | (byte[1] << 8);
        psg.counter[i] = counter;
    }
}

int sms_save_state(const char *filename)   {
    FILE *fp;
    uint8 byte;

    if(sms_initialized == 0)    {
        /* This shouldn't happen.... */
        return -1;
    }

    fp = fopen(filename, "wb");
    if(!fp)
        return -1;

    fprintf(fp, "CrabEmu Save State");

    /* Write save state version */
    byte = 0x00;
    fwrite(&byte, 1, 1, fp);

    byte = 0x01;
    fwrite(&byte, 1, 1, fp);

    /* Write out the current Z80 context */
    sms_z80_write_context(fp);

    /* Next, write the current VDP state */
    sms_vdp_write_context(fp);

    /* Now, write the current PSG state */
    sms_psg_write_context(fp);

    /* Finally, write the current memory contents to the file */
    sms_mem_write_context(fp);

    fclose(fp);

    return 0;
}

int sms_load_state(const char *filename)   {
    char str[19];
    FILE *fp;
    char byte;

    if(sms_initialized == 0)    {
        /* This shouldn't happen.... */
        return -1;
    }

    fp = fopen(filename, "rb");
    if(!fp)
        return -1;

    fread(str, 18, 1, fp);
    str[18] = 0;
    if(strcmp("CrabEmu Save State", str))   {
        fclose(fp);
        return -2;
    }

    /* Read save state version */
    fread(&byte, 1, 1, fp);
    if(byte != 0x00)    {
        fclose(fp);
        return -2;
    }

    fread(&byte, 1, 1, fp);
    if(byte != 0x01)    {
        fclose(fp);
        return -2;
    }

    /* Read in the current Z80 context */
    sms_z80_read_context(fp);

    /* Next, read the current VDP state */
    sms_vdp_read_context(fp);

    /* Now, read the current PSG state */
    sms_psg_read_context(fp);

    /* Finally, read the current memory contents from the file */
    sms_mem_read_context(fp);

    fclose(fp);

    sound_reset_buffer();

    return 0;
}
