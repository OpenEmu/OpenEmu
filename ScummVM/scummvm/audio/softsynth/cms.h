/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef AUDIO_SOFTSYNTH_CMS_H
#define AUDIO_SOFTSYNTH_CMS_H

#include "common/scummsys.h"

/* this structure defines a channel */
struct saa1099_channel {
	int frequency;				/* frequency (0x00..0xff) */
	int freq_enable;			/* frequency enable */
	int noise_enable;			/* noise enable */
	int octave;				/* octave (0x00..0x07) */
	int amplitude[2];			/* amplitude (0x00..0x0f) */
	int envelope[2];			/* envelope (0x00..0x0f or 0x10 == off) */

	/* vars to simulate the square wave */
	double counter;
	double freq;
	int level;
};

/* this structure defines a noise channel */
struct saa1099_noise {
	/* vars to simulate the noise generator output */
	double counter;
	double freq;
	int level;				/* noise polynomal shifter */
};

/* this structure defines a SAA1099 chip */
struct SAA1099 {
	int stream;				/* our stream */
	int noise_params[2];			/* noise generators parameters */
	int env_enable[2];			/* envelope generators enable */
	int env_reverse_right[2];		/* envelope reversed for right channel */
	int env_mode[2];			/* envelope generators mode */
	int env_bits[2];			/* non zero = 3 bits resolution */
	int env_clock[2];			/* envelope clock mode (non-zero external) */
	int env_step[2];			/* current envelope step */
	int all_ch_enable;			/* all channels enable */
	int sync_state;				/* sync all channels */
	int selected_reg;			/* selected register */
	struct saa1099_channel channels[6];	/* channels */
	struct saa1099_noise noise[2];		/* noise generators */
};

class CMSEmulator {
public:
	CMSEmulator(uint32 sampleRate) {
		_sampleRate = sampleRate;
		memset(_saa1099, 0, sizeof(SAA1099)*2);
	}

	~CMSEmulator() { }

	void portWrite(int port, int val);
	void readBuffer(int16 *buffer, const int numSamples);
private:
	uint32 _sampleRate;

	SAA1099 _saa1099[2];

	void envelope(int chip, int ch);
	void update(int chip, int16 *buffer, int length);
	void portWriteIntern(int chip, int offset, int data);
};


#endif
