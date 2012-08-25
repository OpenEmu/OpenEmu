#include "burnint.h"
#include "burn_sound.h"
#include "k007232.h"

#define KDAC_A_PCM_MAX	(2)
#define BASE_SHIFT	(12)

typedef void (*K07232_PortWrite)(INT32 v);

static UINT32 fncode[0x200];

struct kdacApcm
{
	UINT8			vol[KDAC_A_PCM_MAX][2];
	UINT32			addr[KDAC_A_PCM_MAX];
	UINT32			start[KDAC_A_PCM_MAX];
	UINT32			step[KDAC_A_PCM_MAX];
	UINT32			bank[KDAC_A_PCM_MAX];
	INT32			play[KDAC_A_PCM_MAX];

	UINT8 			wreg[0x10];
	
	INT32			UpdateStep;
};

// stuff that doesn't need to be saved..
struct kdacPointers
{
	UINT32  		clock;
	UINT8			*pcmbuf[2];
	UINT32  		pcmlimit;
	K07232_PortWrite	K07232PortWriteHandler;
	
	double			gain[2];
	INT32			output_dir[2];
};

static struct kdacApcm Chips[2];
static struct kdacPointers Pointers[2];
static struct kdacApcm *Chip = NULL;
static struct kdacPointers *Ptr = NULL;
static INT32 *Left = NULL;
static INT32 *Right = NULL;

static INT32 nNumChips = 0;

void K007232Update(INT32 chip, INT16* pSoundBuf, INT32 nLength)
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("K007232Update called without init\n"));
	if (chip >nNumChips) bprintf(PRINT_ERROR, _T("K007232Update called with invalid chip %x\n"), chip);
#endif

	INT32 i;

	Chip = &Chips[chip];
	Ptr  = &Pointers[chip];

	memset(Left, 0, nLength * sizeof(INT32));
	memset(Right, 0, nLength * sizeof(INT32));

	for (i = 0; i < KDAC_A_PCM_MAX; i++) {
		if (Chip->play[i]) {
			INT32 volA,volB,j,out;
			UINT32 addr, old_addr;

			addr = Chip->start[i] + ((Chip->addr[i]>>BASE_SHIFT)&0x000fffff);
			volA = Chip->vol[i][0] * 2;
			volB = Chip->vol[i][1] * 2;

			for( j = 0; j < nLength; j++) {
				old_addr = addr;
				addr = Chip->start[i] + ((Chip->addr[i]>>BASE_SHIFT)&0x000fffff);
				while (old_addr <= addr) {
					if( (Ptr->pcmbuf[i][old_addr] & 0x80) || old_addr >= Ptr->pcmlimit ) {
						if( Chip->wreg[0x0d]&(1<<i) ) {
							Chip->start[i] = ((((UINT32)Chip->wreg[i*0x06 + 0x04]<<16)&0x00010000) | (((UINT32)Chip->wreg[i*0x06 + 0x03]<< 8)&0x0000ff00) | (((UINT32)Chip->wreg[i*0x06 + 0x02]    )&0x000000ff) | Chip->bank[i]);
							addr = Chip->start[i];
							Chip->addr[i] = 0;
							old_addr = addr;
						} else {
							Chip->play[i] = 0;
						}
						break;
					}

					old_addr++;
				}

				if (Chip->play[i] == 0) break;

				Chip->addr[i] += (Chip->step[i] * Chip->UpdateStep) >> 16;

				out = (Ptr->pcmbuf[i][addr] & 0x7f) - 0x40;

				Left[j] += out * volA;
				Right[j] += out * volB;
			}
		}
	}
	
	for (i = 0; i < nLength; i++) {
		INT32 nLeftSample = 0, nRightSample = 0;
		
		if ((Ptr->output_dir[BURN_SND_K007232_ROUTE_1] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(Left[i] * Ptr->gain[BURN_SND_K007232_ROUTE_1]);
		}
		if ((Ptr->output_dir[BURN_SND_K007232_ROUTE_1] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(Left[i] * Ptr->gain[BURN_SND_K007232_ROUTE_1]);
		}
		
		if ((Ptr->output_dir[BURN_SND_K007232_ROUTE_2] & BURN_SND_ROUTE_LEFT) == BURN_SND_ROUTE_LEFT) {
			nLeftSample += (INT32)(Right[i] * Ptr->gain[BURN_SND_K007232_ROUTE_2]);
		}
		if ((Ptr->output_dir[BURN_SND_K007232_ROUTE_2] & BURN_SND_ROUTE_RIGHT) == BURN_SND_ROUTE_RIGHT) {
			nRightSample += (INT32)(Right[i] * Ptr->gain[BURN_SND_K007232_ROUTE_2]);
		}
		
		nLeftSample = BURN_SND_CLIP(nLeftSample);
		nRightSample = BURN_SND_CLIP(nRightSample);
		
		pSoundBuf[0] += nLeftSample;
		pSoundBuf[1] += nRightSample;
		pSoundBuf += 2;
	}
}

UINT8 K007232ReadReg(INT32 chip, INT32 r)
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("K007232ReadReg called without init\n"));
	if (chip >nNumChips) bprintf(PRINT_ERROR, _T("K007232ReadReg called with invalid chip %x\n"), chip);
#endif

	INT32  ch = 0;

	Chip = &Chips[chip];
	Ptr  = &Pointers[chip];

	if (r == 0x0005 || r == 0x000b) {
		ch = r / 0x0006;
		r  = ch * 0x0006;

		Chip->start[ch] = ((((UINT32)Chip->wreg[r + 0x04]<<16)&0x00010000) | (((UINT32)Chip->wreg[r + 0x03]<< 8)&0x0000ff00) | (((UINT32)Chip->wreg[r + 0x02]    )&0x000000ff) | Chip->bank[ch]);

		if (Chip->start[ch] <  Ptr->pcmlimit) {
			Chip->play[ch] = 1;
			Chip->addr[ch] = 0;
		}
	}

	return 0;
}

void K007232WriteReg(INT32 chip, INT32 r, INT32 v)
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("K007232WriteReg called without init\n"));
	if (chip >nNumChips) bprintf(PRINT_ERROR, _T("K007232WriteReg called with invalid chip %x\n"), chip);
#endif

	INT32 Data;

	Chip = &Chips[chip];
	Ptr  = &Pointers[chip];

	Chip->wreg[r] = v;

	if (r == 0x0c) {
		if (Ptr->K07232PortWriteHandler) Ptr->K07232PortWriteHandler(v);
    		return;
  	}
	else if( r == 0x0d ){
    		// loopflag
    		return;
  	}
  	else {
		INT32 RegPort;

		RegPort = 0;
		if (r >= 0x06) {
			RegPort = 1;
			r -= 0x06;
		}

		switch (r) {
			case 0x00:
			case 0x01:
				Data = (((((UINT32)Chip->wreg[RegPort*0x06 + 0x01])<<8)&0x0100) | (((UINT32)Chip->wreg[RegPort*0x06 + 0x00])&0x00ff));
				Chip->step[RegPort] = fncode[Data];
				break;

			case 0x02:
			case 0x03:
			case 0x04:
				break;
    
			case 0x05:
				Chip->start[RegPort] = ((((UINT32)Chip->wreg[RegPort*0x06 + 0x04]<<16)&0x00010000) | (((UINT32)Chip->wreg[RegPort*0x06 + 0x03]<< 8)&0x0000ff00) | (((UINT32)Chip->wreg[RegPort*0x06 + 0x02]    )&0x000000ff) | Chip->bank[RegPort]);
				if (Chip->start[RegPort] < Ptr->pcmlimit ) {
					Chip->play[RegPort] = 1;
					Chip->addr[RegPort] = 0;
      				}
      			break;
    		}
  	}
}

void K007232SetPortWriteHandler(INT32 chip, void (*Handler)(INT32 v))
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("K007232SetPortWriteHandler called without init\n"));
	if (chip >nNumChips) bprintf(PRINT_ERROR, _T("K007232SetPortWriteHandler called with invalid chip %x\n"), chip);
#endif

	Chip = &Chips[chip];
	Ptr  = &Pointers[chip];

	Ptr->K07232PortWriteHandler = Handler;
}

static void KDAC_A_make_fncode()
{
	for (INT32 i = 0; i < 0x200; i++) fncode[i] = (32 << BASE_SHIFT) / (0x200 - i);
}

void K007232Init(INT32 chip, INT32 clock, UINT8 *pPCMData, INT32 PCMDataSize)
{
	DebugSnd_K007232Initted = 1;
	
	Chip = &Chips[chip];
	Ptr  = &Pointers[chip];

	memset(Chip,	0, sizeof(kdacApcm));
	memset(Ptr,	0, sizeof(kdacPointers));

	if (Left == NULL) {
		Left = (INT32*)malloc(nBurnSoundLen * sizeof(INT32));
	}

	if (Right == NULL) {
		Right = (INT32*)malloc(nBurnSoundLen * sizeof(INT32));
	}

	Ptr->pcmbuf[0] = pPCMData;
	Ptr->pcmbuf[1] = pPCMData;
	Ptr->pcmlimit  = PCMDataSize;

	Ptr->clock = clock;

	for (INT32 i = 0; i < KDAC_A_PCM_MAX; i++) {
		Chip->start[i] = 0;
		Chip->step[i] = 0;
		Chip->play[i] = 0;
		Chip->bank[i] = 0;
	}
	Chip->vol[0][0] = 255;
	Chip->vol[0][1] = 0;
	Chip->vol[1][0] = 0;
	Chip->vol[1][1] = 255;

	for (INT32 i = 0; i < 0x10; i++)  Chip->wreg[i] = 0;

	KDAC_A_make_fncode();
	
	double Rate = (double)clock / 128 / nBurnSoundRate;
	Chip->UpdateStep = (INT32)(Rate * 0x10000);
	
	Ptr->gain[BURN_SND_K007232_ROUTE_1] = 1.00;
	Ptr->gain[BURN_SND_K007232_ROUTE_2] = 1.00;
	Ptr->output_dir[BURN_SND_K007232_ROUTE_1] = BURN_SND_ROUTE_BOTH;
	Ptr->output_dir[BURN_SND_K007232_ROUTE_2] = BURN_SND_ROUTE_BOTH;
	
	nNumChips = chip;
}

void K007232SetRoute(INT32 chip, INT32 nIndex, double nVolume, INT32 nRouteDir)
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("K007232SetRoute called without init\n"));
	if (chip >nNumChips) bprintf(PRINT_ERROR, _T("K007232SetRoute called with invalid chip %x\n"), chip);
	if (nIndex < 0 || nIndex > 1) bprintf(PRINT_ERROR, _T("K007232SetRoute called with invalid index %i\n"), nIndex);
#endif

	Ptr  = &Pointers[chip];
	
	Ptr->gain[nIndex] = nVolume;
	Ptr->output_dir[nIndex] = nRouteDir;
}

void K007232Exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("K007232Exit called without init\n"));
#endif

	if (Left) {
		free(Left);
		Left = NULL;
	}	

	if (Right) {
		free(Right);
		Right = NULL;
	}
	
	DebugSnd_K007232Initted = 0;
	nNumChips = 0;
}

INT32 K007232Scan(INT32 nAction, INT32 *pnMin)
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("K007232Scan called without init\n"));
#endif

	struct BurnArea ba;
	char szName[32];

	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}
	
	if (pnMin != NULL) {
		*pnMin = 0x029693;
	}

	sprintf(szName, "K007232 Chip # %d", 0);
	ba.Data		= &Chip;
	ba.nLen		= sizeof(struct kdacApcm);
	ba.nAddress = 0;
	ba.szName	= szName;
	BurnAcb(&ba);

	return 0;
}

void K007232SetVolume(INT32 chip, INT32 channel,INT32 volumeA,INT32 volumeB)
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("K007232SetVolume called without init\n"));
	if (chip >nNumChips) bprintf(PRINT_ERROR, _T("K007232SetVolume called with invalid chip %x\n"), chip);
#endif

	Chip = &Chips[chip];

	Chip->vol[channel][0] = volumeA;
	Chip->vol[channel][1] = volumeB;
}

void k007232_set_bank(INT32 chip, INT32 chABank, INT32 chBBank )
{
#if defined FBA_DEBUG
	if (!DebugSnd_K007232Initted) bprintf(PRINT_ERROR, _T("k007232_set_bank called without init\n"));
	if (chip >nNumChips) bprintf(PRINT_ERROR, _T("k007232_set_bank called with invalid chip %x\n"), chip);
#endif

	Chip = &Chips[chip];

	Chip->bank[0] = chABank<<17;
	Chip->bank[1] = chBBank<<17;
}
