/***************************************************************************

    Philips SAA1099 Sound driver

    By Juergen Buchmueller and Manuel Abadia

    SAA1099 register layout:
    ========================

    offs | 7654 3210 | description
    -----+-----------+---------------------------
    0x00 | ---- xxxx | Amplitude channel 0 (left)
    0x00 | xxxx ---- | Amplitude channel 0 (right)
    0x01 | ---- xxxx | Amplitude channel 1 (left)
    0x01 | xxxx ---- | Amplitude channel 1 (right)
    0x02 | ---- xxxx | Amplitude channel 2 (left)
    0x02 | xxxx ---- | Amplitude channel 2 (right)
    0x03 | ---- xxxx | Amplitude channel 3 (left)
    0x03 | xxxx ---- | Amplitude channel 3 (right)
    0x04 | ---- xxxx | Amplitude channel 4 (left)
    0x04 | xxxx ---- | Amplitude channel 4 (right)
    0x05 | ---- xxxx | Amplitude channel 5 (left)
    0x05 | xxxx ---- | Amplitude channel 5 (right)
         |           |
    0x08 | xxxx xxxx | Frequency channel 0
    0x09 | xxxx xxxx | Frequency channel 1
    0x0a | xxxx xxxx | Frequency channel 2
    0x0b | xxxx xxxx | Frequency channel 3
    0x0c | xxxx xxxx | Frequency channel 4
    0x0d | xxxx xxxx | Frequency channel 5
         |           |
    0x10 | ---- -xxx | Channel 0 octave select
    0x10 | -xxx ---- | Channel 1 octave select
    0x11 | ---- -xxx | Channel 2 octave select
    0x11 | -xxx ---- | Channel 3 octave select
    0x12 | ---- -xxx | Channel 4 octave select
    0x12 | -xxx ---- | Channel 5 octave select
         |           |
    0x14 | ---- ---x | Channel 0 frequency enable (0 = off, 1 = on)
    0x14 | ---- --x- | Channel 1 frequency enable (0 = off, 1 = on)
    0x14 | ---- -x-- | Channel 2 frequency enable (0 = off, 1 = on)
    0x14 | ---- x--- | Channel 3 frequency enable (0 = off, 1 = on)
    0x14 | ---x ---- | Channel 4 frequency enable (0 = off, 1 = on)
    0x14 | --x- ---- | Channel 5 frequency enable (0 = off, 1 = on)
         |           |
    0x15 | ---- ---x | Channel 0 noise enable (0 = off, 1 = on)
    0x15 | ---- --x- | Channel 1 noise enable (0 = off, 1 = on)
    0x15 | ---- -x-- | Channel 2 noise enable (0 = off, 1 = on)
    0x15 | ---- x--- | Channel 3 noise enable (0 = off, 1 = on)
    0x15 | ---x ---- | Channel 4 noise enable (0 = off, 1 = on)
    0x15 | --x- ---- | Channel 5 noise enable (0 = off, 1 = on)
         |           |
    0x16 | ---- --xx | Noise generator parameters 0
    0x16 | --xx ---- | Noise generator parameters 1
         |           |
    0x18 | --xx xxxx | Envelope generator 0 parameters
    0x18 | x--- ---- | Envelope generator 0 control enable (0 = off, 1 = on)
    0x19 | --xx xxxx | Envelope generator 1 parameters
    0x19 | x--- ---- | Envelope generator 1 control enable (0 = off, 1 = on)
         |           |
    0x1c | ---- ---x | All channels enable (0 = off, 1 = on)
    0x1c | ---- --x- | Synch & Reset generators

***************************************************************************/

#include "burnint.h"
#include "saa1099.h"

#define LEFT	0x00
#define RIGHT	0x01

/* this structure defines a channel */
struct saa1099_channel
{
	INT32 frequency;			/* frequency (0x00..0xff) */
	INT32 freq_enable;		/* frequency enable */
	INT32 noise_enable;		/* noise enable */
	INT32 octave; 			/* octave (0x00..0x07) */
	INT32 amplitude[2];		/* amplitude (0x00..0x0f) */
	INT32 envelope[2];		/* envelope (0x00..0x0f or 0x10 == off) */

	/* vars to simulate the square wave */
	double counter;
	double freq;
	INT32 level;
};

/* this structure defines a noise channel */
struct saa1099_noise
{
	/* vars to simulate the noise generator output */
	double counter;
	double freq;
	INT32 level;					/* noise polynomal shifter */
};

/* this structure defines a SAA1099 chip */
struct saa1099_state
{
	INT32 noise_params[2];				/* noise generators parameters */
	INT32 env_enable[2];				/* envelope generators enable */
	INT32 env_reverse_right[2];			/* envelope reversed for right channel */
	INT32 env_mode[2];				/* envelope generators mode */
	INT32 env_bits[2];				/* non zero = 3 bits resolution */
	INT32 env_clock[2];				/* envelope clock mode (non-zero external) */
	INT32 env_step[2];                		/* current envelope step */
	INT32 all_ch_enable;				/* all channels enable */
	INT32 sync_state;					/* sync all channels */
	INT32 selected_reg;				/* selected register */
	struct saa1099_channel channels[6];    		/* channels */
	struct saa1099_noise noise[2];			/* noise generators */
	double sample_rate;
	INT32 bAdd;
	double gain[2];
	INT32 output_dir[2];
};

static saa1099_state chips[2];
static INT32 nNumChips = 0;

static const INT32 amplitude_lookup[16] = {
	 0*32767/16,  1*32767/16,  2*32767/16,	3*32767/16,
	 4*32767/16,  5*32767/16,  6*32767/16,	7*32767/16,
	 8*32767/16,  9*32767/16, 10*32767/16, 11*32767/16,
	12*32767/16, 13*32767/16, 14*32767/16, 15*32767/16
};

static const UINT8 envelope[8][64] = {
	/* zero amplitude */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* maximum amplitude */
    {15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
	 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
     15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15, },
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

static void saa1099_envelope(saa1099_state *saa, INT32 ch)
{
	if (saa->env_enable[ch])
	{
		INT32 step, mode, mask;
        	mode = saa->env_mode[ch];
		/* step from 0..63 and then loop in steps 32..63 */
		step = saa->env_step[ch] =
			((saa->env_step[ch] + 1) & 0x3f) | (saa->env_step[ch] & 0x20);

		mask = 15;
       		if (saa->env_bits[ch])
			mask &= ~1; 	/* 3 bit resolution, mask LSB */

		saa->channels[ch*3+0].envelope[ LEFT] =
		saa->channels[ch*3+1].envelope[ LEFT] =
		saa->channels[ch*3+2].envelope[ LEFT] = envelope[mode][step] & mask;
		if (saa->env_reverse_right[ch] & 0x01)
		{
			saa->channels[ch*3+0].envelope[RIGHT] =
			saa->channels[ch*3+1].envelope[RIGHT] =
			saa->channels[ch*3+2].envelope[RIGHT] = (15 - envelope[mode][step]) & mask;
		}
		else
		{
			saa->channels[ch*3+0].envelope[RIGHT] =
			saa->channels[ch*3+1].envelope[RIGHT] =
			saa->channels[ch*3+2].envelope[RIGHT] = envelope[mode][step] & mask;
        	}
	}
	else
	{
		/* envelope mode off, set all envelope factors to 16 */
		saa->channels[ch*3+0].envelope[ LEFT] =
		saa->channels[ch*3+1].envelope[ LEFT] =
		saa->channels[ch*3+2].envelope[ LEFT] =
		saa->channels[ch*3+0].envelope[RIGHT] =
		saa->channels[ch*3+1].envelope[RIGHT] =
		saa->channels[ch*3+2].envelope[RIGHT] = 16;
 	}
}

void saa1099Update(INT32 chip, INT16 *output, INT32 samples)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SAA1099Initted) bprintf(PRINT_ERROR, _T("saa1099Update called without init\n"));
	if (chip > nNumChips) bprintf(PRINT_ERROR, _T("saa1099Update called with invalid chip %x\n"), chip);
#endif

	saa1099_state *saa = &chips[chip];
	INT32 j, ch;

	/* if the channels are disabled we're done */
	if (!saa->all_ch_enable)
	{
		/* init output data */
		memset (output, 0, samples * sizeof(INT16) * 2);
        	return;
	}

	for (ch = 0; ch < 2; ch++)
	{
		switch (saa->noise_params[ch])
		{
			case 0: saa->noise[ch].freq = 31250.0 * 2; break;
			case 1: saa->noise[ch].freq = 15625.0 * 2; break;
			case 2: saa->noise[ch].freq =  7812.5 * 2; break;
			case 3: saa->noise[ch].freq = saa->channels[ch * 3].freq; break;
		}
	}

	/* fill all data needed */
	for( j = 0; j < samples; j++, output+=2)
	{
		INT32 output_l = 0, output_r = 0;

		/* for each channel */
		for (ch = 0; ch < 6; ch++)
		{
			if (saa->channels[ch].freq == 0.0)
 				saa->channels[ch].freq = (double)((2 * 15625) << saa->channels[ch].octave) / (511.0 - (double)saa->channels[ch].frequency);

			/* check the actual position in the square wave */
			saa->channels[ch].counter -= saa->channels[ch].freq;

			while (saa->channels[ch].counter < 0)
			{
				/* calculate new frequency now after the half wave is updated */
				saa->channels[ch].freq = (double)((2 * 15625) << saa->channels[ch].octave) /
					(511.0 - (double)saa->channels[ch].frequency);

				saa->channels[ch].counter += saa->sample_rate;
				saa->channels[ch].level ^= 1;

				/* eventually clock the envelope counters */
				if (ch == 1 && saa->env_clock[0] == 0)
					saa1099_envelope(saa, 0);
				if (ch == 4 && saa->env_clock[1] == 0)
					saa1099_envelope(saa, 1);
			}

			/* if the noise is enabled */
			if (saa->channels[ch].noise_enable)
			{
				/* if the noise level is high (noise 0: chan 0-2, noise 1: chan 3-5) */
				if (saa->noise[ch/3].level & 1)
				{
					/* subtract to avoid overflows, also use only half amplitude */
					output_l -= saa->channels[ch].amplitude[ LEFT] * saa->channels[ch].envelope[ LEFT] / 16 / 2;
					output_r -= saa->channels[ch].amplitude[RIGHT] * saa->channels[ch].envelope[RIGHT] / 16 / 2;
				}
			}

			/* if the square wave is enabled */
			if (saa->channels[ch].freq_enable)
			{
				/* if the channel level is high */
				if (saa->channels[ch].level & 1)
				{
					output_l += saa->channels[ch].amplitude[ LEFT] * saa->channels[ch].envelope[ LEFT] / 16;
					output_r += saa->channels[ch].amplitude[RIGHT] * saa->channels[ch].envelope[RIGHT] / 16;
				}
			}
		}

		for (ch = 0; ch < 2; ch++)
		{
			/* check the actual position in noise generator */
			saa->noise[ch].counter -= saa->noise[ch].freq;
			while (saa->noise[ch].counter < 0)
			{
				saa->noise[ch].counter += saa->sample_rate;
				if( ((saa->noise[ch].level & 0x4000) == 0) == ((saa->noise[ch].level & 0x0040) == 0) )
					saa->noise[ch].level = (saa->noise[ch].level << 1) | 1;
				else
					saa->noise[ch].level <<= 1;
			}
		}
		/* write sound data to the buffer */
		INT32 nLeftSample = 0, nRightSample = 0;
		
		if ((saa->output_dir[BURN_SND_SAA1099_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)((output_l / 6) * saa->gain[BURN_SND_SAA1099_ROUTE_1]);
		}
		if ((saa->output_dir[BURN_SND_SAA1099_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)((output_l / 6) * saa->gain[BURN_SND_SAA1099_ROUTE_1]);
		}
		
		if ((saa->output_dir[BURN_SND_SAA1099_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)((output_r / 6) * saa->gain[BURN_SND_SAA1099_ROUTE_2]);
		}
		if ((saa->output_dir[BURN_SND_SAA1099_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)((output_r / 6) * saa->gain[BURN_SND_SAA1099_ROUTE_2]);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
		
		output[LEFT] = nLeftSample;
		output[RIGHT] = nRightSample;
	}
}

void saa1099Reset(INT32 chip)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SAA1099Initted) bprintf(PRINT_ERROR, _T("saa1099Reset called without init\n"));
	if (chip > nNumChips) bprintf(PRINT_ERROR, _T("saa1099Reset called with invalid chip %x\n"), chip);
#endif

	saa1099_state *saa = &chips[chip];

	double sample_rate = saa->sample_rate;
	INT32 bAdd = saa->bAdd;
	double gain0 = saa->gain[BURN_SND_SAA1099_ROUTE_1];
	double gain1 = saa->gain[BURN_SND_SAA1099_ROUTE_2];
	INT32 dir0 = saa->output_dir[BURN_SND_SAA1099_ROUTE_1];
	INT32 dir1 = saa->output_dir[BURN_SND_SAA1099_ROUTE_2];

	memset (saa, 0, sizeof(saa1099_state));

	saa->sample_rate = sample_rate;
	saa->bAdd = bAdd;
	saa->gain[BURN_SND_SAA1099_ROUTE_1] = gain0;
	saa->gain[BURN_SND_SAA1099_ROUTE_2] = gain1;
	saa->output_dir[BURN_SND_SAA1099_ROUTE_1] = dir0;
	saa->output_dir[BURN_SND_SAA1099_ROUTE_2] = dir1;
}

void saa1099Init(INT32 chip, INT32 clock, INT32 bAdd)
{
	DebugSnd_SAA1099Initted = 1;
	
	saa1099_state *saa = &chips[chip];

	saa->sample_rate = clock / 256;
	saa->bAdd = bAdd;
	saa->gain[BURN_SND_SAA1099_ROUTE_1] = 1.00;
	saa->gain[BURN_SND_SAA1099_ROUTE_2] = 1.00;
	saa->output_dir[BURN_SND_SAA1099_ROUTE_1] = BURN_SND_ROUTE_BOTH;
	saa->output_dir[BURN_SND_SAA1099_ROUTE_2] = BURN_SND_ROUTE_BOTH;
	
	nNumChips = chip;
}

void saa1099SetRoute(INT32 chip, INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SAA1099Initted) bprintf(PRINT_ERROR, _T("saa1099SetRoute called without init\n"));
	if (chip > nNumChips) bprintf(PRINT_ERROR, _T("saa1099SetRoute called with invalid chip %x\n"), chip);
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("saa1099SetRoute called with invalid index %i\n"), nIndex);
#endif

	saa1099_state *saa = &chips[chip];
	
	saa->gain[nIndex] = nVolume;
	saa->output_dir[nIndex] = nRouteDir;
}

void saa1099Exit(INT32 )
{
#if defined FBA_DEBUG
	if (!DebugSnd_SAA1099Initted) bprintf(PRINT_ERROR, _T("saa1099Exit called without init\n"));
#endif

	DebugSnd_SAA1099Initted = 0;
	nNumChips = 0;
}

void saa1099ControlWrite(INT32 chip, INT32 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SAA1099Initted) bprintf(PRINT_ERROR, _T("saa1099ControlWrite called without init\n"));
	if (chip > nNumChips) bprintf(PRINT_ERROR, _T("saa1099ControlWrite called with invalid chip %x\n"), chip);
#endif

	saa1099_state *saa = &chips[chip];

	saa->selected_reg = data & 0x1f;

	if (saa->selected_reg == 0x18 || saa->selected_reg == 0x19)
	{
		/* clock the envelope channels */
		if (saa->env_clock[0])
			saa1099_envelope(saa,0);
		if (saa->env_clock[1])
			saa1099_envelope(saa,1);
	}
}

void saa1099DataWrite(INT32 chip, INT32 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SAA1099Initted) bprintf(PRINT_ERROR, _T("saa1099DataWrite called without init\n"));
	if (chip > nNumChips) bprintf(PRINT_ERROR, _T("saa1099DataWrite called with invalid chip %x\n"), chip);
#endif

	saa1099_state *saa = &chips[chip];
	INT32 reg = saa->selected_reg;
	INT32 ch;

	switch (reg)
	{
	/* channel i amplitude */
	case 0x00:	case 0x01:	case 0x02:	case 0x03:	case 0x04:	case 0x05:
		ch = reg & 7;
		saa->channels[ch].amplitude[LEFT] = amplitude_lookup[data & 0x0f];
		saa->channels[ch].amplitude[RIGHT] = amplitude_lookup[(data >> 4) & 0x0f];
		break;
	/* channel i frequency */
	case 0x08:	case 0x09:	case 0x0a:	case 0x0b:	case 0x0c:	case 0x0d:
		ch = reg & 7;
		saa->channels[ch].frequency = data & 0xff;
		break;
	/* channel i octave */
	case 0x10:	case 0x11:	case 0x12:
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
	case 0x18:	case 0x19:
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
		if (data & 0x02)
		{
			INT32 i;

			/* Synch & Reset generators */
			for (i = 0; i < 6; i++)
			{
                		saa->channels[i].level = 0;
				saa->channels[i].counter = 0.0;
			}
		}
		break;
	default: break;	/* Error! */
	}
}

void saa1099Scan(INT32 chip, INT32 nAction)
{
#if defined FBA_DEBUG
	if (!DebugSnd_SAA1099Initted) bprintf(PRINT_ERROR, _T("saa1099Scan called without init\n"));
#endif

	struct BurnArea ba;
	saa1099_state *saa = &chips[chip];

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = (UINT8*)saa;
		ba.nLen	  = sizeof(saa1099_state);
		ba.szName = "SAA data";
		BurnAcb(&ba);
	}
}
