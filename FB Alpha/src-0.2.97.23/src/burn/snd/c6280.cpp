/*
	HuC6280 sound chip emulator
	by Charles MacDonald
	E-mail: cgfm2@hotmail.com
	WWW: http://cgfm2.emuviews.com

	Thanks to:

	- Paul Clifford for his PSG documentation.
	- Richard Bannister for the TGEmu-specific sound updating code.
	- http://www.uspto.gov for the PSG patents.
	- All contributors to the tghack-list.

	Changes:

	(03/30/2003)
	- Removed TGEmu specific code and added support functions for MAME.
	- Modified setup code to handle multiple chips with different clock and
	  volume settings.

	Missing features / things to do:

	- Add LFO support. But do any games actually use it?

	- Add shared index for waveform playback and sample writes. Almost every
	  game will reset the index prior to playback so this isn't an issue.

	- While the noise emulation is complete, the data for the pseudo-random
	  bitstream is calculated by machine.rand() and is not a representation of what
	  the actual hardware does.

	For some background on Hudson Soft's C62 chipset:

	- http://www.hudsonsoft.net/ww/about/about.html
	- http://www.hudson.co.jp/corp/eng/coinfo/history.html

	Legal information:

	Copyright Charles MacDonald

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	Ported from MAME 0.144u6 - 18/1/12
*/

#include "burnint.h"
#include "h6280_intf.h"
#include "c6280.h"
#include "math.h"

typedef struct {
	UINT16 frequency;
	UINT8 control;
	UINT8 balance;
	UINT8 waveform[32];
	UINT8 index;
	INT16 dda;
	UINT8 noise_control;
	UINT32 noise_counter;
	UINT32 counter;
} t_channel;

typedef struct {
	UINT8 select;
	UINT8 balance;
	UINT8 lfo_frequency;
	UINT8 lfo_control;
	t_channel channel[8];
	INT16 volume_table[32];
	UINT32 noise_freq_tab[32];
	UINT32 wave_freq_tab[4096];
	INT32 bAdd;
	double gain[2];
	INT32 output_dir[2];
} c6280_t;

static INT16 *stream_buffer = NULL;
static INT32 c6280_previous_offset = 0;

static c6280_t chip[1];

INT32 c6280_sync_get_offset_end()
{
	// should we get these externally? The c6280 *should* only ever be used with
	// the h6280 and should use the same clocks.
	INT32 cycles = (INT32)((INT64)7159090 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	INT64 ret = (nBurnSoundLen * h6280TotalCycles()) / cycles;

	if (ret >= nBurnSoundLen) {
		ret = nBurnSoundLen;
	}

	return (INT32)ret;
}

void c6280_reset()
{
	c6280_t *p = &chip[0];

	p->select = 0;
	p->balance = 0;
	p->lfo_frequency = 0;
	p->lfo_control = 0;
	memset (p->channel, 0, 8 * sizeof(t_channel));

	c6280_previous_offset = 0;
}

void c6280_init(double clk, INT32 bAdd)
{
	int i;
	double step;
	c6280_t *p = &chip[0];

	/* Loudest volume level for table */
	double level = 65535.0 / 6.0 / 32.0;

	/* Clear context */
	memset(p, 0, sizeof(c6280_t));

	/* Make waveform frequency table */
	for(i = 0; i < 4096; i += 1)
	{
		step = ((clk / (nBurnSoundRate * 1.0000)) * 4096) / (i+1);
		p->wave_freq_tab[(1 + i) & 0xFFF] = (UINT32)step;
	}

	/* Make noise frequency table */
	for(i = 0; i < 32; i += 1)
	{
		step = ((clk / (nBurnSoundRate * 1.0000)) * 32) / (i+1);
		p->noise_freq_tab[i] = (UINT32)step;
	}

	/* Make volume table */
	/* PSG has 48dB volume range spread over 32 steps */
	step = 48.0 / 32.0;
	for(i = 0; i < 31; i++)
	{
		p->volume_table[i] = (UINT16)level;
		level /= pow(10.0, step / 20.0);
	}
	p->volume_table[31] = 0;

	p->bAdd = bAdd;
	p->gain[BURN_SND_C6280_ROUTE_1] = 1.00;
	p->gain[BURN_SND_C6280_ROUTE_2] = 1.00;
	p->output_dir[BURN_SND_C6280_ROUTE_1] = BURN_SND_ROUTE_LEFT;
	p->output_dir[BURN_SND_C6280_ROUTE_2] = BURN_SND_ROUTE_RIGHT;

	stream_buffer = (INT16*)BurnMalloc(nBurnSoundLen * 2 * sizeof(INT16));

	if (stream_buffer == NULL) {
#if defined FBA_DEBUG
		bprintf (0, _T("Stream buffer allocation failed!\n"));
#endif
		return;
	}
}

void c6280_set_route(INT32 nIndex, double nVolume, INT32 nRouteDir)
{
	c6280_t *p = &chip[0];
	
	p->gain[nIndex] = nVolume;
	p->output_dir[nIndex] = nRouteDir;
}

void c6280_exit()
{
	if (stream_buffer) {
		BurnFree(stream_buffer);
	}
}

static void c6280_stream_update()
{
	c6280_t *p = &chip[0];

#if 1
	INT32 end = c6280_sync_get_offset_end();
	INT32 start = c6280_previous_offset;

	INT32 samples = end - start;
	if (samples<=0) return; // don't update if length is 0

	INT16 *pBuffer = stream_buffer + start * 2;

	c6280_previous_offset = end;
	if (end >= nBurnSoundLen) {
		c6280_previous_offset = 0;
	} else {
		c6280_previous_offset = end;
	}
#endif

	static const int scale_tab[] = {
		0x00, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F,
		0x10, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F
	};
	int ch;
	int i;

	int lmal = (p->balance >> 4) & 0x0F;
	int rmal = (p->balance >> 0) & 0x0F;
	int vll, vlr;

	lmal = scale_tab[lmal];
	rmal = scale_tab[rmal];

	/* Clear buffer */
	memset (pBuffer, 0, samples * sizeof(short) * 2); // 16-bit * 2 channels

	for(ch = 0; ch < 6; ch++)
	{
		/* Only look at enabled channels */
		if(p->channel[ch].control & 0x80)
		{
			int lal = (p->channel[ch].balance >> 4) & 0x0F;
			int ral = (p->channel[ch].balance >> 0) & 0x0F;
			int al  = p->channel[ch].control & 0x1F;

			lal = scale_tab[lal];
			ral = scale_tab[ral];

			/* Calculate volume just as the patent says */
			vll = (0x1F - lal) + (0x1F - al) + (0x1F - lmal);
			if(vll > 0x1F) vll = 0x1F;

			vlr = (0x1F - ral) + (0x1F - al) + (0x1F - rmal);
			if(vlr > 0x1F) vlr = 0x1F;

			vll = p->volume_table[vll];
			vlr = p->volume_table[vlr];

			INT16 *pBuf = pBuffer;

			/* Check channel mode */
			if((ch >= 4) && (p->channel[ch].noise_control & 0x80))
			{
				/* Noise mode */
				UINT32 step = p->noise_freq_tab[(p->channel[ch].noise_control & 0x1F) ^ 0x1F];
				for(i = 0; i < samples; i++, pBuf+=2)
				{
					static int data = 0;
					p->channel[ch].noise_counter += step;
					if(p->channel[ch].noise_counter >= 0x800)
					{
						data = (rand() & 1) ? 0x1F : 0;
					}
					p->channel[ch].noise_counter &= 0x7FF;
					pBuf[0] += (INT16)(vll * (data - 16));
					pBuf[1] += (INT16)(vlr * (data - 16));
				}
			}
			else
			if(p->channel[ch].control & 0x40)
			{
				/* DDA mode */
				for(i = 0; i < samples; i++, pBuf+=2)
				{
					pBuf[0] += (INT16)(vll * (p->channel[ch].dda - 16));
					pBuf[1] += (INT16)(vlr * (p->channel[ch].dda - 16));
				}
			}
			else
			{
				/* Waveform mode */
				UINT32 step = p->wave_freq_tab[p->channel[ch].frequency];
				for(i = 0; i < samples; i++, pBuf+=2)
				{
					int offset = (p->channel[ch].counter >> 12) & 0x1F;
					p->channel[ch].counter += step;
					p->channel[ch].counter &= 0x1FFFF;
					INT16 data = p->channel[ch].waveform[offset];
					pBuf[0] += (INT16)(vll * (data - 16));
					pBuf[1] += (INT16)(vlr * (data - 16));
				}
			}
		}
	}
}

static void c6280_write_internal(int offset, int data)
{
	c6280_t *p = &chip[0];
	t_channel *q = &p->channel[p->select];

	c6280_stream_update();

	switch(offset & 0x0F)
	{
		case 0x00: /* Channel select */
			p->select = data & 0x07;
			break;

		case 0x01: /* Global balance */
			p->balance  = data;
			break;

		case 0x02: /* Channel frequency (LSB) */
			q->frequency = (q->frequency & 0x0F00) | data;
			q->frequency &= 0x0FFF;
			break;

		case 0x03: /* Channel frequency (MSB) */
			q->frequency = (q->frequency & 0x00FF) | (data << 8);
			q->frequency &= 0x0FFF;
			break;

		case 0x04: /* Channel control (key-on, DDA mode, volume) */

			/* 1-to-0 transition of DDA bit resets waveform index */
			if((q->control & 0x40) && ((data & 0x40) == 0))
			{
				q->index = 0;
			}
			q->control = data;
			break;

		case 0x05: /* Channel balance */
			q->balance = data;
			break;

		case 0x06: /* Channel waveform data */

			switch(q->control & 0xC0)
			{
				case 0x00:
					q->waveform[q->index & 0x1F] = data & 0x1F;
					q->index = (q->index + 1) & 0x1F;
					break;

				case 0x40:
					break;

				case 0x80:
					q->waveform[q->index & 0x1F] = data & 0x1F;
					q->index = (q->index + 1) & 0x1F;
					break;

				case 0xC0:
					q->dda = data & 0x1F;
					break;
			}

			break;

		case 0x07: /* Noise control (enable, frequency) */
			q->noise_control = data;
			break;

		case 0x08: /* LFO frequency */
			p->lfo_frequency = data;
			break;

		case 0x09: /* LFO control (enable, mode) */
			p->lfo_control = data;
			break;

		default:
			break;
	}
}

void c6280_update(INT16 *pBuffer, INT32 samples)
{
	c6280_t *p = &chip[0];

	c6280_stream_update();

	if (!p->bAdd) {
		memset (pBuffer, 0, samples * sizeof(INT16) * 2); // 16-bit * 2 channels
	}

	for (INT32 i = 0; i < samples; i++) {
		INT32 nLeftSample = 0, nRightSample = 0;
			
		if ((p->output_dir[BURN_SND_C6280_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(stream_buffer[(i << 1) + 0] * p->gain[BURN_SND_C6280_ROUTE_1]);
		}
		if ((p->output_dir[BURN_SND_C6280_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(stream_buffer[(i << 1) + 0] * p->gain[BURN_SND_C6280_ROUTE_1]);
		}
			
		if ((p->output_dir[BURN_SND_C6280_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(stream_buffer[(i << 1) + 1] * p->gain[BURN_SND_C6280_ROUTE_2]);
		}
		if ((p->output_dir[BURN_SND_C6280_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(stream_buffer[(i << 1) + 1] * p->gain[BURN_SND_C6280_ROUTE_2]);
		}
			
		pBuffer[(i << 1) + 0] = BURN_SND_CLIP(nLeftSample);
		pBuffer[(i << 1) + 1] = BURN_SND_CLIP(nRightSample);
	}
}

UINT8 c6280_read()
{
	return h6280io_get_buffer();
}

void c6280_write(UINT8 offset, UINT8 data)
{
	h6280io_set_buffer(data);
	c6280_write_internal(offset, data);
}

INT32 c6280_scan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin =  0x029702;
	}

	if (nAction & ACB_DRIVER_DATA) {	
		c6280_t *p = &chip[0];

		ba.Data		= p;
		ba.nLen		= sizeof(c6280_t);
		ba.nAddress	= 0;
		ba.szName	= "c6280 Chip #0";
		BurnAcb(&ba);
	}

	return 0;
}

