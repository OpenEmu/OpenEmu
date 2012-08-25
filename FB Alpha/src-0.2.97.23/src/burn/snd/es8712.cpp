/**********************************************************************************************
 *
 *  Streaming singe channel ADPCM core for the ES8712 chip
 *  Chip is branded by Excellent Systems, probably OEM'd.
 *
 *  Samples are currently looped, but whether they should and how, is unknown.
 *  Interface to the chip is also not 100% clear.
 *  Should there be any status signals signifying busy, end of sample - etc?
 *
 *  Heavily borrowed from the OKI M6295 source
 *
 *  Excellent Systems ADPCM Emulation
 *  Copyright Nicola Salmoria and the MAME Team
 *
 *  From MAME 0.139u1. Modified for use in FBA Aug 23, 2010.
 *
 **********************************************************************************************/

#include "burnint.h"
#include "math.h"
#include "es8712.h"

#define MAX_ES8712_CHIPS	1

#define MAX_SAMPLE_CHUNK	10000

/* struct describing a playing ADPCM chip */
typedef struct _es8712_state es8712_state;
struct _es8712_state
{
	UINT8 playing;			/* 1 if we're actively playing */

	UINT32 base_offset;		/* pointer to the base memory location */
	UINT32 sample;			/* current sample number */
	UINT32 count;			/* total samples to play */

	UINT32 signal;			/* current ADPCM signal */
	UINT32 step;			/* current ADPCM step */

	UINT32 start;			/* starting address for the next loop */
	UINT32 end;				/* ending address for the next loop */
	UINT8  repeat;			/* Repeat current sample when 1 */

	INT32 bank_offset;

// non volatile
	UINT8 *region_base;		/* pointer to the base of the region */

	INT32 sample_rate;		/* samples per frame */
	double volume;			/* set gain */
	INT32 output_dir;
	INT32 addSignal;			/* add signal to stream? */
};

static INT16 *tbuf[MAX_ES8712_CHIPS] = { NULL };

static _es8712_state chips[MAX_ES8712_CHIPS];
static _es8712_state *chip;

static const INT32 index_shift[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
static INT32 diff_lookup[49*16];

/**********************************************************************************************

     compute_tables -- compute the difference tables

***********************************************************************************************/

static void compute_tables()
{
	/* nibble to bit map */
	static const INT32 nbl2bit[16][4] =
	{
		{ 1, 0, 0, 0}, { 1, 0, 0, 1}, { 1, 0, 1, 0}, { 1, 0, 1, 1},
		{ 1, 1, 0, 0}, { 1, 1, 0, 1}, { 1, 1, 1, 0}, { 1, 1, 1, 1},
		{-1, 0, 0, 0}, {-1, 0, 0, 1}, {-1, 0, 1, 0}, {-1, 0, 1, 1},
		{-1, 1, 0, 0}, {-1, 1, 0, 1}, {-1, 1, 1, 0}, {-1, 1, 1, 1}
	};

	INT32 step, nib;

	/* loop over all possible steps */
	for (step = 0; step <= 48; step++)
	{
		/* compute the step value */
		INT32 stepval = (INT32)(floor(16.0 * pow(11.0 / 10.0, (double)step)));

		/* loop over all nibbles and compute the difference */
		for (nib = 0; nib < 16; nib++)
		{
			diff_lookup[step*16 + nib] = nbl2bit[nib][0] *
				(stepval   * nbl2bit[nib][1] +
				 stepval/2 * nbl2bit[nib][2] +
				 stepval/4 * nbl2bit[nib][3] +
				 stepval/8);
		}
	}
}


/**********************************************************************************************

    generate_adpcm -- general ADPCM decoding routine

***********************************************************************************************/

static void generate_adpcm(INT16 *buffer, INT32 samples)
{
	/* if this chip is active */
	if (chip->playing)
	{
		UINT8 *base = chip->region_base + chip->bank_offset + chip->base_offset;
		INT32 sample = chip->sample;
		INT32 signal = chip->signal;
		INT32 count = chip->count;
		INT32 step = chip->step;
		double volume = chip->volume;
		INT32 val;

		/* loop while we still have samples to generate */
		while (samples)
		{
			/* compute the new amplitude and update the current step */
			val = base[sample / 2] >> (((sample & 1) << 2) ^ 4);
			signal += diff_lookup[step * 16 + (val & 15)];

			/* clamp to the maximum */
			if (signal > 2047)
				signal = 2047;
			else if (signal < -2048)
				signal = -2048;

			/* adjust the step size and clamp */
			step += index_shift[val & 7];
			if (step > 48)
				step = 48;
			else if (step < 0)
				step = 0;

			/* output to the buffer */
			*buffer++ = (INT32)(signal * 16 * volume);
			samples--;

			/* next! */
			if (++sample >= count)
			{
				if (chip->repeat)
				{
					sample = 0;
					signal = -2;
					step = 0;
				}
				else
				{
					chip->playing = 0;
					break;
				}
			}
		}

		/* update the parameters */
		chip->sample = sample;
		chip->signal = signal;
		chip->step = step;
	}

	/* fill the rest with silence */
	while (samples--)
		*buffer++ = 0;
}


/**********************************************************************************************

    es8712Update -- update the sound chip so that it is in sync with CPU execution

***********************************************************************************************/

void es8712Update(INT32 device, INT16 *buffer, INT32 samples)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES8712Initted) bprintf(PRINT_ERROR, _T("es8712Update called without init\n"));
#endif

	if (device >= MAX_ES8712_CHIPS) return;

	chip = &chips[device];

	INT32 sample_num = (INT32)((float)(((samples / nBurnSoundLen) * 1.0000) * chip->sample_rate));

	float step = ((chip->sample_rate * 1.00000) / nBurnSoundLen);

	INT16 *buf = tbuf[device];

	generate_adpcm(buf, sample_num);

	float r = 0;
	for (INT32 i = 0; i < samples; i++, r += step, buffer+=2) {
		INT32 nLeftSample = 0, nRightSample = 0;
		
		if ((chip->output_dir & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(buf[(INT32)r] * chip->volume);
		}
		if ((chip->output_dir & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(buf[(INT32)r] * chip->volume);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
		
		if (chip->addSignal) {
			buffer[0] += nLeftSample;
			buffer[1] += nRightSample;
		} else {
			buffer[0] = nLeftSample;
			buffer[1] = nRightSample;
		}
	}
	
}


/**********************************************************************************************

    es8712Init -- start emulation of an ES8712 chip

***********************************************************************************************/

void es8712Init(INT32 device, UINT8 *rom, INT32 sample_rate, INT32 addSignal)
{
	DebugSnd_ES8712Initted = 1;

	if (device >= MAX_ES8712_CHIPS) return;

	chip = &chips[device];

	compute_tables();

	chip->start = 0;
	chip->end = 0;
	chip->repeat = 0;

	chip->bank_offset = 0;
	chip->region_base = (UINT8*)rom;

	/* initialize the rest of the structure */
	chip->signal = (UINT32)-2;

	chip->sample_rate = sample_rate;

	chip->volume = 1.00;
	chip->output_dir = BURN_SND_ROUTE_BOTH;
	chip->addSignal = addSignal;

	if (tbuf[device] == NULL) {
		tbuf[device] = (INT16*)malloc(sample_rate * sizeof(INT16));
	}
}

void es8712SetRoute(INT32 device, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES8712Initted) bprintf(PRINT_ERROR, _T("es8712SetRoute called without init\n"));
#endif

	if (device >= MAX_ES8712_CHIPS) return;
	
	chip = &chips[device];
	chip->volume = nVolume;
	chip->output_dir = nRouteDir;
}

/**********************************************************************************************

    es8712Exit -- stop emulation of an ES8712 chip

***********************************************************************************************/

void es8712Exit(INT32 device)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES8712Initted) bprintf(PRINT_ERROR, _T("es8712Exit called without init\n"));
#endif

	if (device >= MAX_ES8712_CHIPS) return;

	chip = &chips[device];

	memset (chip, 0, sizeof(_es8712_state));

	if (tbuf[device] != NULL) {
		free (tbuf[device]);
		tbuf[device] = NULL;
	}
	
	DebugSnd_ES8712Initted = 0;
}

/*************************************************************************************

     es8712Reset -- stop emulation of an ES8712-compatible chip

**************************************************************************************/

void es8712Reset(INT32 device)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES8712Initted) bprintf(PRINT_ERROR, _T("es8712Reset called without init\n"));
#endif

	if (device >= MAX_ES8712_CHIPS) return;

	chip = &chips[device];

	if (chip->playing)
	{
		/* update the stream, then turn it off */
		chip->playing = 0;
		chip->repeat = 0;
	}
}


/****************************************************************************

    es8712_set_bank_base -- set the base of the bank on a given chip

*****************************************************************************/

void es8712SetBankBase(INT32 device, INT32 base)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES8712Initted) bprintf(PRINT_ERROR, _T("es8712SetBankBase called without init\n"));
#endif

	if (device >= MAX_ES8712_CHIPS) return;

	chip = &chips[device];

	chip->bank_offset = base;
}


/**********************************************************************************************

    es8712Play -- Begin playing the addressed sample

***********************************************************************************************/

void es8712Play(INT32 device)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES8712Initted) bprintf(PRINT_ERROR, _T("es8712Play called without init\n"));
#endif

	if (device >= MAX_ES8712_CHIPS) return;

	chip = &chips[device];

	if (chip->start < chip->end)
	{
		if (!chip->playing)
		{
			chip->playing = 1;
			chip->base_offset = chip->start;
			chip->sample = 0;
			chip->count = 2 * (chip->end - chip->start + 1);
			chip->repeat = 0;//1;

			/* also reset the ADPCM parameters */
			chip->signal = (UINT32)-2;
			chip->step = 0;
		}
	}
	/* invalid samples go here */
	else
	{
		if (chip->playing)
		{
			/* update the stream */
			chip->playing = 0;
		}
	}
}


/**********************************************************************************************

     es8712Write -- generic data write function

***********************************************************************************************/

/**********************************************************************************************
 *
 *  offset  Start       End
 *          0hmmll  -  0HMMLL
 *    00    ----ll
 *    01    --mm--
 *    02    0h----
 *    03               ----LL
 *    04               --MM--
 *    05               0H----
 *    06           Go!
 *
 * Offsets are written in the order -> 00, 02, 01, 03, 05, 04, 06
 * Offset 06 is written with the same value as offset 04.
 *
***********************************************************************************************/

void es8712Write(INT32 device, INT32 offset, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES8712Initted) bprintf(PRINT_ERROR, _T("es8712Write called without init\n"));
#endif

	if (device >= MAX_ES8712_CHIPS) return;

	chip = &chips[device];

	switch (offset)
	{
		case 00:	chip->start &= 0x000fff00;
					chip->start |= ((data & 0xff) <<  0); break;
		case 01:	chip->start &= 0x000f00ff;
					chip->start |= ((data & 0xff) <<  8); break;
		case 02:	chip->start &= 0x0000ffff;
					chip->start |= ((data & 0x0f) << 16); break;
		case 03:	chip->end   &= 0x000fff00;
					chip->end   |= ((data & 0xff) <<  0); break;
		case 04:	chip->end   &= 0x000f00ff;
					chip->end   |= ((data & 0xff) <<  8); break;
		case 05:	chip->end   &= 0x0000ffff;
					chip->end   |= ((data & 0x0f) << 16); break;
		case 06:
				es8712Play(device);
				break;
		default:	break;
	}

	chip->start &= 0xfffff;
	chip->end &= 0xfffff;
}


/**********************************************************************************************

     es8712Scan -- save state function

***********************************************************************************************/

INT32 es8712Scan(INT32 device, INT32 nAction)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ES8712Initted) bprintf(PRINT_ERROR, _T("es8712Scan called without init\n"));
#endif

	if (device >= MAX_ES8712_CHIPS) return 1;

	if (nAction & ACB_DRIVER_DATA) {
		chip = &chips[device];

		SCAN_VAR(chip->playing);
		SCAN_VAR(chip->base_offset);
		SCAN_VAR(chip->sample);
		SCAN_VAR(chip->count);
		SCAN_VAR(chip->signal);
		SCAN_VAR(chip->step);
		SCAN_VAR(chip->start);
		SCAN_VAR(chip->end);
		SCAN_VAR(chip->repeat);
		SCAN_VAR(chip->bank_offset);
	}

	return 0;
}
