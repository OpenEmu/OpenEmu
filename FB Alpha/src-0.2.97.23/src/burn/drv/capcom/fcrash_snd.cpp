#include "cps.h"
#include "burn_ym2203.h"
#include "msm5205.h"

// fcrash_snd.cpp
// Sound support for games using similar sound to Final Crash
// 2 x YM2203 and 2 x MSM5205

static UINT8 *FcrashZ80Ram = NULL;
static INT32 FcrashZ80BankAddress = 0;
static INT32 FcrashSoundLatch = 0;
static INT32 FcrashMSM5205Interleave = 0;
static INT32 FcrashSampleBuffer1 = 0;
static INT32 FcrashSampleBuffer2 = 0;
static INT32 FcrashSampleSelect1 = 0;
static INT32 FcrashSampleSelect2 = 0;
static INT32 FcrashSoundPos = 0;
static INT32 FcrashCyclesPerSegment = 0;

void FcrashSoundCommand(UINT16 d)
{
	INT32 nCyclesToDo = ((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles) - ZetTotalCycles();
	INT32 nEnd = FcrashSoundPos + (INT64)FcrashMSM5205Interleave * nCyclesToDo / nCpsZ80Cycles;
		
	for (INT32 i = FcrashSoundPos; i < nEnd; i++) {
		BurnTimerUpdate((i + 1) * FcrashCyclesPerSegment);
		MSM5205Update();
		FcrashSoundPos = i;
	}
		
	FcrashSoundLatch = d & 0xff;
	ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
}

UINT8 __fastcall FcrashZ80Read(UINT16 a)
{
	switch (a) {
		case 0xd800: {
			return BurnYM2203Read(0, 0);
		}
		
		case 0xdc00: {
			return BurnYM2203Read(1, 0);
		}
		
		case 0xe400: {
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return FcrashSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall FcrashZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xd800: {
			BurnYM2203Write(0, 0, d);
			return;
		}
		
		case 0xd801: {
			BurnYM2203Write(0, 1, d);
			return;
		}
		
		case 0xdc00: {
			BurnYM2203Write(1, 0, d);
			return;
		}
		
		case 0xdc01: {
			BurnYM2203Write(1, 1, d);
			return;
		}
		
		case 0xe000: {
			MSM5205SetRoute(0, (d & 0x08) ? 0 : 0.25, BURN_SND_ROUTE_BOTH);
			MSM5205SetRoute(1, (d & 0x10) ? 0 : 0.25, BURN_SND_ROUTE_BOTH);

			FcrashZ80BankAddress = (d & 0x07) * 0x4000;
			ZetMapArea(0x8000, 0xbfff, 0, CpsZRom + FcrashZ80BankAddress);
			ZetMapArea(0x8000, 0xbfff, 2, CpsZRom + FcrashZ80BankAddress);
			return;
		}
		
		case 0xe800: {
			FcrashSampleBuffer1 = d;
			return;
		}
		
		case 0xec00: {
			FcrashSampleBuffer2 = d;
			return;
		}
		
		case 0xf002:
		case 0xf004:
		case 0xf006: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

inline static INT32 FcrashSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)((double)ZetTotalCycles() * nSoundRate / (24000000 / 6));
}

inline static double FcrashGetTime()
{
	return (double)ZetTotalCycles() / (24000000 / 6);
}

static void FcrashMSM5205Vck0()
{
	MSM5205DataWrite(0, FcrashSampleBuffer1 & 0x0f);
	FcrashSampleBuffer1 >>= 4;
	FcrashSampleSelect1 ^= 1;
	if (FcrashSampleSelect1 == 0) {
		ZetNmi();
	}
}

static void FcrashMSM5205Vck1()
{
	MSM5205DataWrite(1, FcrashSampleBuffer2 & 0x0f);
	FcrashSampleBuffer2 >>= 4;
	FcrashSampleSelect2 ^= 1;
}

INT32 FcrashSoundInit()
{
	FcrashZ80Ram = (UINT8*)BurnMalloc(0x800);
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(FcrashZ80Read);
	ZetSetWriteHandler(FcrashZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, CpsZRom + 0x00000);
	ZetMapArea(0x0000, 0x7fff, 2, CpsZRom + 0x00000);
	ZetMapArea(0x8000, 0xbfff, 0, CpsZRom + 0x08000);
	ZetMapArea(0x8000, 0xbfff, 2, CpsZRom + 0x08000);
	ZetMapArea(0xd000, 0xd7ff, 0, FcrashZ80Ram     );
	ZetMapArea(0xd000, 0xd7ff, 1, FcrashZ80Ram     );
	ZetMapArea(0xd000, 0xd7ff, 2, FcrashZ80Ram     );
	ZetMemEnd();
	ZetClose();
	
	BurnYM2203Init(2, 24000000 / 6, NULL, FcrashSynchroniseStream, FcrashGetTime, 0);
	BurnTimerAttachZet(24000000 / 6);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.70, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.07, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.07, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.07, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_YM2203_ROUTE, 0.70, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_1, 0.07, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_2, 0.07, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_3, 0.07, BURN_SND_ROUTE_BOTH);
	
	MSM5205Init(0, FcrashSynchroniseStream, 24000000 / 64, FcrashMSM5205Vck0, MSM5205_S96_4B, 1);
	MSM5205Init(1, FcrashSynchroniseStream, 24000000 / 64, FcrashMSM5205Vck1, MSM5205_S96_4B, 1);
	MSM5205SetRoute(0, 0.25, BURN_SND_ROUTE_BOTH);
	MSM5205SetRoute(1, 0.25, BURN_SND_ROUTE_BOTH);
	
	nCpsZ80Cycles = (24000000 / 6) * 100 / nBurnFPS;
	
	return 0;
}

INT32 FcrashSoundReset()
{
	ZetOpen(0);
	ZetReset();
	BurnYM2203Reset();
	MSM5205Reset();
	FcrashZ80BankAddress = 0x8000;
	ZetMapArea(0x8000, 0xbfff, 0, CpsZRom + FcrashZ80BankAddress);
	ZetMapArea(0x8000, 0xbfff, 2, CpsZRom + FcrashZ80BankAddress);
	ZetClose();
	
	FcrashSoundLatch = 0;
	FcrashSampleBuffer1 = 0;
	FcrashSampleBuffer2 = 0;
	FcrashSampleSelect1 = 0;
	FcrashSampleSelect2 = 0;
	
	return 0;
}

INT32 FcrashSoundExit()
{
	ZetExit();
	BurnYM2203Exit();
	MSM5205Exit();
	
	BurnFree(FcrashZ80Ram);
	
	FcrashZ80BankAddress = 0;
	FcrashSoundLatch = 0;
	FcrashMSM5205Interleave = 0;
	FcrashSampleBuffer1 = 0;
	FcrashSampleBuffer2 = 0;
	FcrashSampleSelect1 = 0;
	FcrashSampleSelect2 = 0;
	FcrashCyclesPerSegment = 0;
	
	nCpsZ80Cycles = 0;

	return 0;
}

void FcrashSoundFrameStart()
{
	FcrashMSM5205Interleave = MSM5205CalcInterleave(0, 24000000 / 6);
	FcrashSoundPos = 0;
	FcrashCyclesPerSegment = (INT64)nCpsZ80Cycles / FcrashMSM5205Interleave;
	
	ZetNewFrame();
	ZetOpen(0);	
}

void FcrashSoundFrameEnd()
{
	for (INT32 i = FcrashSoundPos; i < FcrashMSM5205Interleave; i++) {
		BurnTimerUpdate((i + 1) * FcrashCyclesPerSegment);
		MSM5205Update();
		FcrashSoundPos = i;
	}
	BurnTimerEndFrame(nCpsZ80Cycles);
	
	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(1, pBurnSoundOut, nBurnSoundLen);
	}
	ZetClose();
}

INT32 FcrashScanSound(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_MEMORY_RAM) {
		struct BurnArea ba;
		memset(&ba, 0, sizeof(ba));

		ba.Data = FcrashZ80Ram;
		ba.nLen = 0x00800;
		ba.szName = "FcrashZ80Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);
		
		BurnYM2203Scan(nAction, pnMin);
		MSM5205Scan(nAction, pnMin);
		
		SCAN_VAR(FcrashZ80BankAddress);
		SCAN_VAR(FcrashSoundLatch);
		SCAN_VAR(FcrashSampleBuffer1);
		SCAN_VAR(FcrashSampleBuffer2);
		SCAN_VAR(FcrashSampleSelect1);
		SCAN_VAR(FcrashSampleSelect2);
		SCAN_VAR(FcrashSoundPos);
	}

	return 0;
}
