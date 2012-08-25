/*********************************************************

Irem GA20 PCM Sound Chip

It's not currently known whether this chip is stereo.


Revisions:

04-15-2002 Acho A. Tang
- rewrote channel mixing
- added prelimenary volume and sample rate emulation

05-30-2002 Acho A. Tang
- applied hyperbolic gain control to volume and used
  a musical-note style progression in sample rate
  calculation(still very inaccurate)

02-18-2004 R. Belmont
- sample rate calculation reverse-engineered.
  Thanks to Fujix, Yasuhiro Ogawa, the Guru, and Tormod
  for real PCB samples that made this possible.

02-03-2007 R. Belmont
- Cleaned up faux x86 assembly.

*********************************************************/

#include "burnint.h"
#include "iremga20.h"

#define MAX_GA20	1
#define MAX_VOL		256

struct IremGA20_channel_def
{
	UINT32 rate;
	UINT32 size;
	UINT32 start;
	UINT32 pos;
	UINT32 frac;
	UINT32 end;
	UINT32 volume;
	UINT32 pan;
	UINT32 effect;
	UINT32 play;
};

typedef struct _ga20_state ga20_state;
struct _ga20_state
{
	UINT8 *rom;
	INT32 rom_size;
	UINT16 regs[0x40];
	struct IremGA20_channel_def channel[4];
	INT32 frequency;
	double gain;
	INT32 output_dir;
};

static struct _ga20_state chips[MAX_GA20];
static struct _ga20_state *chip;

static UINT32 computed_steps;

// 14318180/4 -> 3579545/4 -> 894886/60 -> 14915
// 44100 / 60 -> 735

static INT32 nNumChips;

void iremga20_update(INT32 device, INT16 *buffer, INT32 length)
{
#if defined FBA_DEBUG
	if (!DebugSnd_IremGA20Initted) bprintf(PRINT_ERROR, _T("iremga20_update called without init\n"));
	if (device > nNumChips) bprintf(PRINT_ERROR, _T("iremga20_update called with invalid chip %x\n"), device);
#endif

	chip = &chips[device];
	UINT32 rate[4], pos[4], frac[4], end[4], vol[4], play[4];
	UINT8 *pSamples;
	INT32 i, sampleout;

	/* precache some values */
	for (i=0; i < 4; i++)
	{
		rate[i] = chip->channel[i].rate;
		pos[i] = chip->channel[i].pos;
		frac[i] = chip->channel[i].frac;
		end[i] = chip->channel[i].end - 0x20;
		vol[i] = chip->channel[i].volume;
		play[i] = chip->channel[i].play;
	}

	pSamples = chip->rom;

	for (i = 0; i < length; i++, buffer+=2)
	{
		sampleout = 0;

		// update the 4 channels inline
		if (play[0])
		{
			sampleout += (pSamples[pos[0]] - 0x80) * vol[0];
			frac[0] += rate[0] * computed_steps;
			pos[0] += frac[0] >> 24;
			frac[0] &= 0xffffff;
			play[0] = (pos[0] < end[0]);
		}
		if (play[1])
		{
			sampleout += (pSamples[pos[1]] - 0x80) * vol[1];
			frac[1] += rate[1] * computed_steps;
			pos[1] += frac[1] >> 24;
			frac[1] &= 0xffffff;
			play[1] = (pos[1] < end[1]);
		}
		if (play[2])
		{
			sampleout += (pSamples[pos[2]] - 0x80) * vol[2];
			frac[2] += rate[2] * computed_steps;
			pos[2] += frac[2] >> 24;
			frac[2] &= 0xffffff;
			play[2] = (pos[2] < end[2]);
		}
		if (play[3])
		{
			sampleout += (pSamples[pos[3]] - 0x80) * vol[3];
			frac[3] += rate[3] * computed_steps;
			pos[3] += frac[3] >> 24;
			frac[3] &= 0xffffff;
			play[3] = (pos[3] < end[3]);
		}

		sampleout >>= 2;

		INT32 nLeftSample = 0, nRightSample = 0;
		
		if ((chip->output_dir & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(sampleout * chip->gain);
		}
		if ((chip->output_dir & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(sampleout * chip->gain);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
		
		buffer[0] += nLeftSample;
		buffer[1] += nRightSample;
	}

	/* update the regs now */
	for (i=0; i < 4; i++)
	{
		chip->channel[i].pos = pos[i];
		chip->channel[i].frac = frac[i];
		chip->channel[i].play = play[i];
	}
}

void iremga20_write(INT32 device, INT32 offset, INT32 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_IremGA20Initted) bprintf(PRINT_ERROR, _T("iremga20_write called without init\n"));
	if (device > nNumChips) bprintf(PRINT_ERROR, _T("iremga20_write called with invalid chip %x\n"), device);
#endif

	chip = &chips[device];

	INT32 channel = offset >> 3;

	chip->regs[offset] = data;

	switch (offset & 0x7)
	{
		case 0: /* start address low */
			chip->channel[channel].start = ((chip->channel[channel].start)&0xff000) | (data<<4);
			break;

		case 1: /* start address high */
			chip->channel[channel].start = ((chip->channel[channel].start)&0x00ff0) | (data<<12);
			break;

		case 2: /* end address low */
			chip->channel[channel].end = ((chip->channel[channel].end)&0xff000) | (data<<4);
			break;

		case 3: /* end address high */
			chip->channel[channel].end = ((chip->channel[channel].end)&0x00ff0) | (data<<12);
			break;

		case 4:
			chip->channel[channel].rate = 0x1000000 / (256 - data);
			break;

		case 5: //AT: gain control
			chip->channel[channel].volume = (data * MAX_VOL) / (data + 10);
			break;

		case 6: //AT: this is always written 2(enabling both channels?)
			chip->channel[channel].play = data;
			chip->channel[channel].pos = chip->channel[channel].start;
			chip->channel[channel].frac = 0;
			break;
	}
}

UINT8 iremga20_read(INT32 device, INT32 offset)
{
#if defined FBA_DEBUG
	if (!DebugSnd_IremGA20Initted) bprintf(PRINT_ERROR, _T("iremga20_read called without init\n"));
	if (device > nNumChips) bprintf(PRINT_ERROR, _T("iremga20_read called with invalid chip %x\n"), device);
#endif

	chip = &chips[device];

	switch (offset & 0x7)
	{
		case 7:	// voice status.  bit 0 is 1 if active. (routine around 0xccc in rtypeleo)
			return chip->channel[offset >> 3].play ? 1 : 0;

		default:
			break;
	}

	return 0;
}

void iremga20_reset(INT32 device)
{
#if defined FBA_DEBUG
	if (!DebugSnd_IremGA20Initted) bprintf(PRINT_ERROR, _T("iremga20_reset called without init\n"));
	if (device > nNumChips) bprintf(PRINT_ERROR, _T("iremga20_reset called with invalid chip %x\n"), device);
#endif

	chip = &chips[device];

	for(INT32 i = 0; i < 4; i++ ) {
		chip->channel[i].rate = 0;
		chip->channel[i].size = 0;
		chip->channel[i].start = 0;
		chip->channel[i].pos = 0;
		chip->channel[i].frac = 0;
		chip->channel[i].end = 0;
		chip->channel[i].volume = 0;
		chip->channel[i].pan = 0;
		chip->channel[i].effect = 0;
		chip->channel[i].play = 0;
	}

	for ( INT32 i = 0; i < 0x40; i++ )
		chip->regs[i] = 0;
}

void iremga20_init(INT32 device, UINT8 *rom, INT32 rom_size, INT32 frequency)
{
	DebugSnd_IremGA20Initted = 1;

	chip = &chips[device];

	/* Initialize our chip structure */
	chip->rom = rom;
	chip->rom_size = rom_size;
	chip->frequency = (frequency / 4) / 60;
	
	chip->gain = 1.00;
	chip->output_dir = BURN_SND_ROUTE_BOTH;

	iremga20_reset(device);
	
	computed_steps = (UINT32)((float)(chip->frequency / (1.00000 * nBurnSoundLen)));
	
	nNumChips = device;
}

void itemga20_set_route(INT32 device, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_IremGA20Initted) bprintf(PRINT_ERROR, _T("itemga20_set_route called without init\n"));
	if (device > nNumChips) bprintf(PRINT_ERROR, _T("itemga20_set_route called with invalid chip %x\n"), device);
#endif

	chip = &chips[device];
	
	chip->gain = nVolume;
	chip->output_dir = nRouteDir;
}

void iremga20_exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_IremGA20Initted) bprintf(PRINT_ERROR, _T("iremga20_exit called without init\n"));
#endif

	DebugSnd_IremGA20Initted = 0;
	nNumChips = 0;
}

INT32 iremga20_scan(INT32 device, INT32 nAction, INT32 *pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_IremGA20Initted) bprintf(PRINT_ERROR, _T("iremga20_scan called without init\n"));
	if (device > nNumChips) bprintf(PRINT_ERROR, _T("iremga20_scan called with invalid chip %x\n"), device);
#endif

	chip = &chips[device];

	struct BurnArea ba;
	char szName[16];
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}
	
	if (pnMin != NULL) {
		*pnMin = 0x029678;
	}
	
	sprintf(szName, "DAC #%d", device);
	ba.Data		= &chip;
	ba.nLen		= sizeof(_ga20_state);
	ba.nAddress	= 0;
	ba.szName	= szName;
	BurnAcb(&ba);
	
	return 0;
}
