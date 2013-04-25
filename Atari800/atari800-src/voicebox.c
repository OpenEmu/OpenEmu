/*
 * voicebox.c - Emulation of the Alien Group Voice Box I and II
 *
 * Copyright (C) 2010 Atari800 development team (see DOC/CREDITS)
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
#include <string.h>
#include "voicebox.h"
#include "atari.h"
#include "votraxsnd.h"
#include "pokey.h"
#include "log.h"
#include "pia.h"

int VOICEBOX_enabled = FALSE;
int VOICEBOX_ii = FALSE;

int VOICEBOX_Initialise(int *argc, char *argv[])
{
	int i, j;
	for (i = j = 1; i < *argc; i++) {
		if (strcmp(argv[i], "-voicebox") == 0) {
			VOICEBOX_enabled = TRUE;
			VOICEBOX_ii = FALSE;
		}
		else if (strcmp(argv[i], "-voiceboxii") == 0){
			VOICEBOX_enabled = TRUE;
			VOICEBOX_ii = TRUE;
		}
		else {
		 	if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-voicebox        Emulate the Alien Group Voice Box I");
				Log_print("\t-voiceboxii      Emulate the Alien Group Voice Box II");
			}

			argv[j++] = argv[i];
		}

	}
	*argc = j;

	return TRUE;
}

/* For Voice Box I */
void VOICEBOX_SKCTLPutByte(int byte)
{
	static int prev_byte;
	static int prev_prev_byte;
	static int voice_box_byte;
	static int voice_box_bit;
	if (!VOICEBOX_enabled || VOICEBOX_ii) return;
	if (PIA_PACTL&0x08) return; /* Cassette motor line must be on */
#ifdef DEBUG_VOICEBOX
	printf ("SKCTL: %2x\n",byte);
#endif
	if (prev_prev_byte == 0xf7 && prev_byte == 0x03 && (byte == 0xff || byte == 0x03)) {
		if (byte != 0x03) {
			voice_box_byte += (1<< (7-voice_box_bit));
		}
		voice_box_bit++;
		if (voice_box_bit > 7 ) {
			voice_box_bit = 0;
#ifdef DEBUG_VOICEBOX
			printf("VOICE BOX BYTE: %02X\n",voice_box_byte);
#endif
			VOTRAXSND_PutByte(voice_box_byte);
			voice_box_byte = 0;
		}
	}
	prev_prev_byte = prev_byte;
	prev_byte = byte;
}

/* For Voice Box II */
void VOICEBOX_SEROUTPutByte(int byte)
{
	if (!VOICEBOX_enabled || !VOICEBOX_ii) return;
	if (PIA_PACTL&0x08) return; /* Cassette motor line must be on */
	if ((POKEY_SKCTL & 0x70) == 0x60 ) {
		int byte_swapped = 0;
		int i;
		for (i=0;i<8;i++) {
			byte_swapped += ((!!(byte&(1<<i)))<<(7-i));
		}
#ifdef DEBUG_VOICEBOX
		printf("VOICEBOX: %02x audf:%02X %02X %02X %02X, AUDCTL:%02X\n",byte_swapped,POKEY_AUDF[0],POKEY_AUDF[1],POKEY_AUDF[2],POKEY_AUDF[3],POKEY_AUDCTL[0]);
#endif
		VOTRAXSND_PutByte(byte_swapped);
	}
}
