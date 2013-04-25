/*
 * votraxsnd.c - Atari800 interface to votrax.c
 *
 * Copyright (C) 2007-2010 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include "atari.h"
#include "util.h"
#include "votraxsnd.h"
#ifdef PBI_XLD
#include "pbi_xld.h"
#endif
#ifdef VOICEBOX
#include "voicebox.h"
#endif
#include "pokey.h"
#include "log.h"

/* Votrax */

#define VTRX_RATE 24500

static double ratio;
static int bit16;
#define VTRX_BLOCK_SIZE 1024
SWORD *temp_votrax_buffer = NULL;
SWORD *votrax_buffer = NULL;
int VOTRAXSND_busy = FALSE;
static int votrax_sync_samples;
static int dsprate;
static int num_pokeys;
static int samples_per_frame;
/*if SYNCHRONIZED_SOUND is not used and the sound generation runs in a
 * separate thread, then these variables are accessed in two different
 * threads: */
static int votrax_written = FALSE;
static int votrax_written_byte = 0x3f;

void VOTRAXSND_PutByte(UBYTE byte)
{
	/* put byte to voice box */
	votrax_sync_samples = (int)((1.0/ratio)*(double)Votrax_Samples((votrax_written_byte&0x3f), (byte&0x3f), votrax_sync_samples));
	votrax_written = TRUE;
	votrax_written_byte = byte;
	if (!VOTRAXSND_busy) {
		VOTRAXSND_busy = TRUE;
#ifdef PBI_XLD
		if (PBI_XLD_enabled) {
			PBI_XLD_votrax_busy_callback(TRUE); /* idle -> busy */
		}
#endif
	}
}

static void VOTRAXSND_busy_callback_async(int busy_status)
{
	return;
	/* do nothing */
}

static int votraxsnd_enabled(void)
{
	if (
#ifdef VOICEBOX
		!VOICEBOX_enabled &&
#endif
#ifdef PBI_XLD
		!PBI_XLD_v_enabled &&

#endif
		1) return FALSE;
	else return TRUE;
}

/* called from POKEYSND_Init */
void VOTRAXSND_Init(int playback_freq, int n_pokeys, int b16)
{
	static struct Votrax_interface vi;
	int temp_votrax_buffer_size;
	bit16 = b16;
	dsprate = playback_freq;
	num_pokeys = n_pokeys;
	if (!votraxsnd_enabled()) return;
	if (num_pokeys != 1 && num_pokeys != 2) {
		Log_print("VOTRAXSND_Init: cannot handle num_pokeys=%d", num_pokeys);
#ifdef PBI_XLD
		PBI_XLD_v_enabled = FALSE;
#endif
#ifdef VOICEBOX
		VOICEBOX_enabled = FALSE;
#endif
		return;
	}
	vi.num = 1;
	vi.BusyCallback = VOTRAXSND_busy_callback_async;
	Votrax_Stop();
	Votrax_Start((void *)&vi);
	samples_per_frame = dsprate/(Atari800_tv_mode == Atari800_TV_PAL ? 50 : 60);
	ratio = (double)VTRX_RATE/(double)dsprate;
#ifdef VOICEBOX
	temp_votrax_buffer_size = (int)(VTRX_BLOCK_SIZE*ratio*(VOICEBOX_BASEAUDF+1) + 10); /* +10 .. little extra? */
#else
	temp_votrax_buffer_size = (int)(VTRX_BLOCK_SIZE*ratio + 10); /* +10 .. little extra? */
#endif
	free(temp_votrax_buffer);
	temp_votrax_buffer = (SWORD *)Util_malloc(temp_votrax_buffer_size*sizeof(SWORD));
	free(votrax_buffer);
	votrax_buffer = (SWORD *)Util_malloc(VTRX_BLOCK_SIZE*sizeof(SWORD));

	VOTRAXSND_busy = FALSE;
	votrax_sync_samples = 0;
}

void VOTRAXSND_Reinit(void)
{
	if (dsprate) VOTRAXSND_Init(dsprate, num_pokeys, bit16);
}

/* process votrax and interpolate samples */
static void votrax_process(SWORD *v_buffer, int len, SWORD *temp_v_buffer)
{
	static SWORD last_sample;
	static SWORD last_sample2;
	static double startpos;
	static int have;
	int max_left_sample_index = (int)(startpos + (double)(len - 1)*ratio);
	int pos = 0;
	double fraction = 0;
	int i;
	int floor_next_pos;

	if (have == 2) {
	    temp_v_buffer[0] = last_sample;
		temp_v_buffer[1] = last_sample2;
		Votrax_Update(0, temp_v_buffer + 2, (max_left_sample_index + 1 + 1) - 2);
	}
	else if (have == 1) {
	    temp_v_buffer[0] = last_sample;
		Votrax_Update(0, temp_v_buffer + 1, (max_left_sample_index + 1 + 1) - 1);
	}
	else if (have == 0) {
		Votrax_Update(0, temp_v_buffer, max_left_sample_index + 1 + 1);
	}
	else if (have < 0) {
		Votrax_Update(0, temp_v_buffer, -have);
		Votrax_Update(0, temp_v_buffer, max_left_sample_index + 1 + 1);
	}

	for (i = 0; i < len; i++) {
		SWORD left_sample;
		SWORD right_sample;
		SWORD interp_sample;
		pos = (int)(startpos + (double)i*ratio);
		fraction = startpos + (double)i*ratio - (double)pos;
		left_sample = temp_v_buffer[pos];
		right_sample = temp_v_buffer[pos+1];
		interp_sample = (int)(left_sample + fraction*(double)(right_sample-left_sample));
		v_buffer[i] = interp_sample;
	}
	floor_next_pos = (int)(startpos + (double)len*ratio);
	startpos = (startpos + (double)len*ratio) - (double)floor_next_pos;
	if (floor_next_pos == max_left_sample_index)
	{
		have = 2;
		last_sample = temp_v_buffer[floor_next_pos];
		last_sample2 = temp_v_buffer[floor_next_pos+1];
	}
	else if (floor_next_pos == max_left_sample_index + 1) {
		have = 1;
		last_sample = temp_v_buffer[floor_next_pos];
	}
	else {
		have = (floor_next_pos - (max_left_sample_index + 2));
	}
}

/* 16 bit mixing */
static void mix(SWORD *dst, SWORD *src, int sndn, int volume)
{
	SWORD s1, s2;
	int val;

	while (sndn--) {
		s1 = *src;
		s1 = s1*volume/128;
		s2 = *dst;
		src++;
		val = s1 + s2;
		if (val > 32767) val = 32767;
		if (val < -32768) val = -32768;
		*dst++ = val;
		if (num_pokeys == 2) {
			dst++;
		}
	}
}

/* 8 bit mixing */
static void mix8(UBYTE *dst, SWORD *src, int sndn, int volume)
{
	SWORD s1, s2;
	int val;

	while (sndn--) {
		s1 = *src;
		s1 = s1*volume/128;
		s2 = ((int)(*dst) - 0x80)*256;
		src++;
		val = s1 + s2;
		if (val > 32767) val = 32767;
		if (val < -32768) val = -32768;
		*dst++ = (UBYTE)((val/256) + 0x80);
		if (num_pokeys == 2) {
			dst++;
		}
	}
}

void VOTRAXSND_Frame(void)
{
	if (!votraxsnd_enabled()) return;
#ifdef VOICEBOX
	if (VOICEBOX_enabled && VOICEBOX_ii) {
		double factor = (VOICEBOX_BASEAUDF+1.0)/(POKEY_AUDF[3]+1.0);
		ratio = (double)VTRX_RATE/(double)dsprate * factor;
		samples_per_frame = ((double)dsprate/(double)(Atari800_tv_mode == Atari800_TV_PAL ? 50 : 60)) / factor;
	}
#endif
	votrax_sync_samples -= samples_per_frame;
	if (votrax_sync_samples <= 0 ) {
		votrax_sync_samples = 0;
		VOTRAXSND_busy = FALSE;
#ifdef PBI_XLD
		PBI_XLD_votrax_busy_callback(FALSE); /* busy -> idle */
#endif
	}
}

void VOTRAXSND_Process(void *sndbuffer, int sndn)
{
	if (!votraxsnd_enabled()) return;

	if(votrax_written) {
		votrax_written = FALSE;
		Votrax_PutByte(votrax_written_byte);
	}
	sndn /= num_pokeys;
	while (sndn > 0) {
		int amount = ((sndn > VTRX_BLOCK_SIZE) ? VTRX_BLOCK_SIZE : sndn);
		votrax_process(votrax_buffer, amount, temp_votrax_buffer);
		if (bit16) mix((SWORD *)sndbuffer, votrax_buffer, amount, 128/4);
		else mix8((UBYTE *)sndbuffer, votrax_buffer, amount, 128/4);
		sndbuffer = (char *) sndbuffer + VTRX_BLOCK_SIZE*(bit16 ? 2 : 1)*((num_pokeys == 2) ? 2: 1);
		sndn -= VTRX_BLOCK_SIZE;
	}
}

