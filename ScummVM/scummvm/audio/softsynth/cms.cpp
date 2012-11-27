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

#include "audio/softsynth/cms.h"
#include "audio/null.h"

#include "common/textconsole.h"
#include "common/translation.h"
#include "common/debug.h"

// CMS/Gameblaster Emulation taken from DosBox

#define LEFT	0x00
#define RIGHT	0x01

static const byte envelope[8][64] = {
	/* zero amplitude */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* maximum amplitude */
	{15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 },
	/* single decay */
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive decay */
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
	/* single triangular */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
	 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive triangular */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
	 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
	 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
	/* single attack */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive attack */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
	  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
	  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
	  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 }
};

static const int amplitude_lookup[16] = {
	 0*32767/16,  1*32767/16,  2*32767/16,	3*32767/16,
	 4*32767/16,  5*32767/16,  6*32767/16,	7*32767/16,
	 8*32767/16,  9*32767/16, 10*32767/16, 11*32767/16,
	12*32767/16, 13*32767/16, 14*32767/16, 15*32767/16
};

void CMSEmulator::portWrite(int port, int val) {
	switch (port) {
	case 0x220:
		portWriteIntern(0, 1, val);
		break;

	case 0x221:
		_saa1099[0].selected_reg = val & 0x1f;
		if (_saa1099[0].selected_reg == 0x18 || _saa1099[0].selected_reg == 0x19) {
			/* clock the envelope channels */
			if (_saa1099[0].env_clock[0])
				envelope(0, 0);
			if (_saa1099[0].env_clock[1])
				envelope(0, 1);
		}
		break;

	case 0x222:
		portWriteIntern(1, 1, val);
		break;

	case 0x223:
		_saa1099[1].selected_reg = val & 0x1f;
		if (_saa1099[1].selected_reg == 0x18 || _saa1099[1].selected_reg == 0x19) {
			/* clock the envelope channels */
			if (_saa1099[1].env_clock[0])
				envelope(1, 0);
			if (_saa1099[1].env_clock[1])
				envelope(1, 1);
		}
		break;

	default:
		warning("CMSEmulator got port: 0x%X", port);
		break;
	}
}

void CMSEmulator::readBuffer(int16 *buffer, const int numSamples) {
	update(0, &buffer[0], numSamples);
	update(1, &buffer[0], numSamples);
}

void CMSEmulator::envelope(int chip, int ch) {
	SAA1099 *saa = &_saa1099[chip];
	if (saa->env_enable[ch]) {
		int step, mode, mask;
		mode = saa->env_mode[ch];
		/* step from 0..63 and then loop in steps 32..63 */
		step = saa->env_step[ch] = ((saa->env_step[ch] + 1) & 0x3f) | (saa->env_step[ch] & 0x20);

		mask = 15;
		if (saa->env_bits[ch])
			mask &= ~1; 	/* 3 bit resolution, mask LSB */

		saa->channels[ch*3+0].envelope[ LEFT] =
		saa->channels[ch*3+1].envelope[ LEFT] =
		saa->channels[ch*3+2].envelope[ LEFT] = ::envelope[mode][step] & mask;
		if (saa->env_reverse_right[ch] & 0x01) {
			saa->channels[ch*3+0].envelope[RIGHT] =
			saa->channels[ch*3+1].envelope[RIGHT] =
			saa->channels[ch*3+2].envelope[RIGHT] = (15 - ::envelope[mode][step]) & mask;
		} else {
			saa->channels[ch*3+0].envelope[RIGHT] =
			saa->channels[ch*3+1].envelope[RIGHT] =
			saa->channels[ch*3+2].envelope[RIGHT] = ::envelope[mode][step] & mask;
		}
	} else {
		/* envelope mode off, set all envelope factors to 16 */
		saa->channels[ch*3+0].envelope[ LEFT] =
		saa->channels[ch*3+1].envelope[ LEFT] =
		saa->channels[ch*3+2].envelope[ LEFT] =
		saa->channels[ch*3+0].envelope[RIGHT] =
		saa->channels[ch*3+1].envelope[RIGHT] =
		saa->channels[ch*3+2].envelope[RIGHT] = 16;
	}
}

void CMSEmulator::update(int chip, int16 *buffer, int length) {
	struct SAA1099 *saa = &_saa1099[chip];
	int j, ch;

	if (chip == 0) {
		memset(buffer, 0, sizeof(int16)*length*2);
	}

	/* if the channels are disabled we're done */
	if (!saa->all_ch_enable) {
		return;
	}

	for (ch = 0; ch < 2; ch++) {
		switch (saa->noise_params[ch]) {
		case 0: saa->noise[ch].freq = 31250.0 * 2; break;
		case 1: saa->noise[ch].freq = 15625.0 * 2; break;
		case 2: saa->noise[ch].freq =  7812.5 * 2; break;
		case 3: saa->noise[ch].freq = saa->channels[ch * 3].freq; break;
		}
	}

	/* fill all data needed */
	for (j = 0; j < length; ++j) {
		int output_l = 0, output_r = 0;

		/* for each channel */
		for (ch = 0; ch < 6; ch++) {
			if (saa->channels[ch].freq == 0.0)
				saa->channels[ch].freq = (double)((2 * 15625) << saa->channels[ch].octave) /
				(511.0 - (double)saa->channels[ch].frequency);

			/* check the actual position in the square wave */
			saa->channels[ch].counter -= saa->channels[ch].freq;
			while (saa->channels[ch].counter < 0) {
				/* calculate new frequency now after the half wave is updated */
				saa->channels[ch].freq = (double)((2 * 15625) << saa->channels[ch].octave) /
					(511.0 - (double)saa->channels[ch].frequency);

				saa->channels[ch].counter += _sampleRate;
				saa->channels[ch].level ^= 1;

				/* eventually clock the envelope counters */
				if (ch == 1 && saa->env_clock[0] == 0)
					envelope(chip, 0);
				if (ch == 4 && saa->env_clock[1] == 0)
					envelope(chip, 1);
			}

			/* if the noise is enabled */
			if (saa->channels[ch].noise_enable) {
				/* if the noise level is high (noise 0: chan 0-2, noise 1: chan 3-5) */
				if (saa->noise[ch/3].level & 1) {
					/* subtract to avoid overflows, also use only half amplitude */
					output_l -= saa->channels[ch].amplitude[ LEFT] * saa->channels[ch].envelope[ LEFT] / 16 / 2;
					output_r -= saa->channels[ch].amplitude[RIGHT] * saa->channels[ch].envelope[RIGHT] / 16 / 2;
				}
			}

			/* if the square wave is enabled */
			if (saa->channels[ch].freq_enable) {
				/* if the channel level is high */
				if (saa->channels[ch].level & 1) {
					output_l += saa->channels[ch].amplitude[ LEFT] * saa->channels[ch].envelope[ LEFT] / 16;
					output_r += saa->channels[ch].amplitude[RIGHT] * saa->channels[ch].envelope[RIGHT] / 16;
				}
			}
		}

		for (ch = 0; ch < 2; ch++) {
			/* check the actual position in noise generator */
			saa->noise[ch].counter -= saa->noise[ch].freq;
			while (saa->noise[ch].counter < 0) {
				saa->noise[ch].counter += _sampleRate;
				if (((saa->noise[ch].level & 0x4000) == 0) == ((saa->noise[ch].level & 0x0040) == 0) )
					saa->noise[ch].level = (saa->noise[ch].level << 1) | 1;
				else
					saa->noise[ch].level <<= 1;
			}
		}
		/* write sound data to the buffer */
		buffer[j*2+0] = CLIP<int>(buffer[j*2+0] + output_l / 6, -32768, 32767);
		buffer[j*2+1] = CLIP<int>(buffer[j*2+1] + output_r / 6, -32768, 32767);
	}
}

void CMSEmulator::portWriteIntern(int chip, int offset, int data) {
	SAA1099 *saa = &_saa1099[chip];
	int reg = saa->selected_reg;
	int ch;

	switch (reg) {
	/* channel i amplitude */
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
		ch = reg & 7;
		saa->channels[ch].amplitude[LEFT] = amplitude_lookup[data & 0x0f];
		saa->channels[ch].amplitude[RIGHT] = amplitude_lookup[(data >> 4) & 0x0f];
		break;

	/* channel i frequency */
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
		ch = reg & 7;
		saa->channels[ch].frequency = data & 0xff;
		break;

	/* channel i octave */
	case 0x10:
	case 0x11:
	case 0x12:
		ch = (reg - 0x10) << 1;
		saa->channels[ch + 0].octave = data & 0x07;
		saa->channels[ch + 1].octave = (data >> 4) & 0x07;
		break;

	/* channel i frequency enable */
	case 0x14:
		saa->channels[0].freq_enable = data & 0x01;
		saa->channels[1].freq_enable = data & 0x02;
		saa->channels[2].freq_enable = data & 0x04;
		saa->channels[3].freq_enable = data & 0x08;
		saa->channels[4].freq_enable = data & 0x10;
		saa->channels[5].freq_enable = data & 0x20;
		break;

	/* channel i noise enable */
	case 0x15:
		saa->channels[0].noise_enable = data & 0x01;
		saa->channels[1].noise_enable = data & 0x02;
		saa->channels[2].noise_enable = data & 0x04;
		saa->channels[3].noise_enable = data & 0x08;
		saa->channels[4].noise_enable = data & 0x10;
		saa->channels[5].noise_enable = data & 0x20;
		break;

	/* noise generators parameters */
	case 0x16:
		saa->noise_params[0] = data & 0x03;
		saa->noise_params[1] = (data >> 4) & 0x03;
		break;

	/* envelope generators parameters */
	case 0x18:
	case 0x19:
		ch = reg - 0x18;
		saa->env_reverse_right[ch] = data & 0x01;
		saa->env_mode[ch] = (data >> 1) & 0x07;
		saa->env_bits[ch] = data & 0x10;
		saa->env_clock[ch] = data & 0x20;
		saa->env_enable[ch] = data & 0x80;
		/* reset the envelope */
		saa->env_step[ch] = 0;
		break;

	/* channels enable & reset generators */
	case 0x1c:
		saa->all_ch_enable = data & 0x01;
		saa->sync_state = data & 0x02;
		if (data & 0x02) {
			int i;
			/* Synch & Reset generators */
			for (i = 0; i < 6; i++) {
				saa->channels[i].level = 0;
				saa->channels[i].counter = 0.0;
			}
		}
		break;

	default:
		// The CMS allows all registers to be written, so we just output some debug
		// message here
		debug(5, "CMS Unknown write to reg %x with %x",reg, data);
	}
}

class CMSMusicPlugin : public NullMusicPlugin {
public:
	const char *getName() const {
		return _s("Creative Music System Emulator");
	}

	const char *getId() const {
		return "cms";
	}

	MusicDevices getDevices() const;
};

MusicDevices CMSMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_CMS));
	return devices;
}

//#if PLUGIN_ENABLED_DYNAMIC(CMS)
	//REGISTER_PLUGIN_DYNAMIC(CMS, PLUGIN_TYPE_MUSIC, CMSMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(CMS, PLUGIN_TYPE_MUSIC, CMSMusicPlugin);
//#endif
