#include "burnint.h"
#include "rf5c68.h"
#include "burn_sound.h"

#define NUM_CHANNELS	(8)

static UINT32 nUpdateStep;

struct pcm_channel
{
	UINT8	enable;
	UINT8	env;
	UINT8	pan;
	UINT8	start;
	UINT32	addr;
	UINT16	step;
	UINT16	loopst;
};

struct rf5c68pcm
{
	struct pcm_channel	chan[NUM_CHANNELS];
	UINT8		cbank;
	UINT8		wbank;
	UINT8		enable;
	UINT8		data[0x10000];
};

static struct rf5c68pcm *chip = NULL;

static INT32 *left = NULL;
static INT32 *right = NULL;

void RF5C68PCMUpdate(INT16* pSoundBuf, INT32 length)
{
#if defined FBA_DEBUG
	if (!DebugSnd_RF5C68Initted) bprintf(PRINT_ERROR, _T("RF5C68PCMUpdate called without init\n"));
#endif

	if (!chip->enable) return;
	
	INT32 i, j;
	
	memset(left, 0, length * sizeof(INT32));
	memset(right, 0, length * sizeof(INT32));
	
	for (i = 0; i < NUM_CHANNELS; i++) {
		pcm_channel *chan = &chip->chan[i];
		
		if (chan->enable) {
			INT32 lv = (chan->pan & 0xf) * chan->env;
			INT32 rv = ((chan->pan >> 4) & 0xf) * chan->env;
		
			for (j = 0; j < length; j++) {
				INT32 sample;
				
				sample = chip->data[(chan->addr >> 11) & 0xffff];
				if (sample == 0xff) {
					chan->addr = chan->loopst << 11;
					sample = chip->data[(chan->addr >> 11) & 0xffff];
					if (sample == 0xff) break;
				}
				
				chan->addr += (chan->step * nUpdateStep) >> 15;//(chan->step * 1181) / 1000;
				
				if (sample & 0x80) {
					sample &= 0x7f;
					left[j] += (sample * lv) >> 5;
					right[j] += (sample * rv) >> 5;
				} else {
					left[j] -= (sample * lv) >> 5;
					right[j] -= (sample * rv) >> 5;
				}
			}
		}
	}
	
	for (i = 0; i < length; i++) {
		if (left[i] > 32767) left[i] = 32767;
		if (left[i] < -32768) left[i] = -32768;
		left[i] = left[i] & ~0x3f;
		if (right[i] > 32767) right[i] = 32767;
		if (right[i] < -32768) right[i] = -32768;
		right[i] = right[i] & ~0x3f;
		
		pSoundBuf[i + 0] = left[i];
		pSoundBuf[i + 1] = right[i];
	}
}

void RF5C68PCMReset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_RF5C68Initted) bprintf(PRINT_ERROR, _T("RF5C68PCMReset called without init\n"));
#endif

	memset(chip, 0, sizeof(*chip));
	memset(chip->data, 0xff, sizeof(chip->data));
}

void RF5C68PCMInit(INT32 clock)
{
	chip = (struct rf5c68pcm*)malloc(sizeof(struct rf5c68pcm));
	
	INT32 Rate = clock / 384;
	
	nUpdateStep = (INT32)(((float)Rate / nBurnSoundRate) * 32768);
	
	left = (INT32*)malloc(nBurnSoundLen * sizeof(INT32));
	right = (INT32*)malloc(nBurnSoundLen * sizeof(INT32));
	
	DebugSnd_RF5C68Initted = 1;
}

void RF5C68PCMExit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_RF5C68Initted) bprintf(PRINT_ERROR, _T("RF5C68PCMExit called without init\n"));
#endif

	if (left) {
		free(left);
		left = NULL;
	}
	if (right) {
		free(right);
		right = NULL;
	}

	DebugSnd_RF5C68Initted = 0;
}

void RF5C68PCMScan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_DRIVER_DATA) {
		memset(&ba, 0, sizeof(ba));
		ba.Data = chip->data;
		ba.nLen = 0x10000;
		ba.szName = "RF5C68PCMData";
		BurnAcb(&ba);

		SCAN_VAR(chip->cbank);
		SCAN_VAR(chip->wbank);
		SCAN_VAR(chip->enable);
		
		for (INT32 i = 0; i < NUM_CHANNELS; i++) {
			pcm_channel *Chan = &chip->chan[i];
			
			SCAN_VAR(Chan->enable);
			SCAN_VAR(Chan->env);
			SCAN_VAR(Chan->pan);
			SCAN_VAR(Chan->start);
			SCAN_VAR(Chan->addr);
			SCAN_VAR(Chan->step);
			SCAN_VAR(Chan->loopst);
		}		
	}
}

void RF5C68PCMRegWrite(UINT8 offset, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_RF5C68Initted) bprintf(PRINT_ERROR, _T("RF5C68PCMReqWrite called without init\n"));
#endif

	struct pcm_channel *chan = &chip->chan[chip->cbank];
	INT32 i;
	
	/* force the stream to update first */
//	stream_update(chip->stream);

	switch (offset) {
		case 0x00: {
			chan->env = data;
			break;
		}
		
		case 0x01: {
			chan->pan = data;
			break;
		}
		
		case 0x02: {
			chan->step = (chan->step & 0xff00) | (data & 0xff);
			break;
		}
		
		case 0x03: {
			chan->step = (chan->step & 0xff) | ((data << 8) & 0xff00);
			break;
		}
		
		case 0x04: {
			chan->loopst = (chan->loopst & 0xff00) | (data & 0xff);
			break;
		}
		
		case 0x05: {
			chan->loopst = (chan->loopst & 0xff) | ((data << 8) & 0xff00);
			break;
		}
		
		case 0x06: {
			chan->start = data;
			if (!chan->enable) chan->addr = chan->start << (8 + 11);
			break;
		}
		
		case 0x07: {
			chip->enable = (data >> 7) & 1;
			if (data & 0x40) {
				chip->cbank = data & 7;
			} else {
				chip->wbank = data & 15;
			}
			break;
		}
		
		case 0x08: {
			for (i = 0; i < 8; i++) {
				chip->chan[i].enable = (~data >> i) & 1;
				if (!chip->chan[i].enable) chip->chan[i].addr = chip->chan[i].start << (8 + 11);
			}
			break;
		}
	}
}

UINT8 RF5C68PCMRead(UINT16 offset)
{
#if defined FBA_DEBUG
	if (!DebugSnd_RF5C68Initted) bprintf(PRINT_ERROR, _T("RF5C68PCMRead called without init\n"));
#endif

	return chip->data[chip->wbank * 0x1000 + offset];
}

void RF5C68PCMWrite(UINT16 offset, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_RF5C68Initted) bprintf(PRINT_ERROR, _T("RF5C68PCMWrite called without init\n"));
#endif

	chip->data[(chip->wbank * 0x1000) + offset] = data;
}

#undef NUM_CHANNELS
