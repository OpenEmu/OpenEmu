#include "cps.h"
#include "burn_ym2151.h"
#include "msm5205.h"

// sf2mdt_snd.cpp
// Sound support for games using similar sound to Street Fighter 2: Magic Delta Turbo
// 1 x YM2151 and 2 x MSM5205

static UINT8 *Sf2mdtZ80Ram = NULL;
static INT32 Sf2mdtZ80BankAddress = 0;
static INT32 Sf2mdtSoundLatch = 0;
static INT32 Sf2mdtMSM5205Interleave = 0;
static INT32 Sf2mdtSampleBuffer1 = 0;
static INT32 Sf2mdtSampleBuffer2 = 0;
static INT32 Sf2mdtSampleSelect1 = 0;
static INT32 Sf2mdtSampleSelect2 = 0;
static INT32 Sf2mdtSoundPos = 0;
static INT32 Sf2mdtCyclesPerSegment = 0;
static INT32 Sf2mdtNumZ80Banks = 0;

void Sf2mdtSoundCommand(UINT16 d)
{
	INT32 nCyclesToDo = ((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles) - ZetTotalCycles();
	INT32 nEnd = Sf2mdtSoundPos + (INT64)Sf2mdtMSM5205Interleave * nCyclesToDo / nCpsZ80Cycles;
	
	for (INT32 i = Sf2mdtSoundPos; i < nEnd; i++) {
		ZetRun(Sf2mdtCyclesPerSegment);
		MSM5205Update();
		Sf2mdtSoundPos = i;
	}
	
	Sf2mdtSoundLatch = d & 0xff;
	ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
}

UINT8 __fastcall Sf2mdtZ80Read(UINT16 a)
{
	switch (a) {
		case 0xd801: {
			return BurnYM2151ReadStatus();
		}
		
		case 0xdc00: {
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return Sf2mdtSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Sf2mdtZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xd800: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0xd801: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0xe000: {
			MSM5205SetVolume(0, (d & 0x20) ? 0 : 20);
			MSM5205SetVolume(1, (d & 0x10) ? 0 : 20);
			
			Sf2mdtZ80BankAddress = (d & Sf2mdtNumZ80Banks) * 0x4000;
			ZetMapArea(0x8000, 0xbfff, 0, CpsZRom + Sf2mdtZ80BankAddress);
			ZetMapArea(0x8000, 0xbfff, 2, CpsZRom + Sf2mdtZ80BankAddress);
			return;
		}
		
		case 0xe400: {
			Sf2mdtSampleBuffer1 = d;
			return;
		}
		
		case 0xe800: {
			Sf2mdtSampleBuffer2 = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

inline static INT32 Sf2mdtSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(ZetTotalCycles() * nSoundRate / 3579540);
}

static void Sf2mdtMSM5205Vck0()
{
	MSM5205DataWrite(0, Sf2mdtSampleBuffer1 & 0x0f);
	Sf2mdtSampleBuffer1 >>= 4;
	Sf2mdtSampleSelect1 ^= 1;
	if (Sf2mdtSampleSelect1 == 0) {
		ZetNmi();
	}
}

static void Sf2mdtMSM5205Vck1()
{
	MSM5205DataWrite(1, Sf2mdtSampleBuffer2 & 0x0f);
	Sf2mdtSampleBuffer2 >>= 4;
	Sf2mdtSampleSelect2 ^= 1;
}

INT32 Sf2mdtSoundInit()
{
	Sf2mdtZ80Ram = (UINT8*)BurnMalloc(0x800);
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Sf2mdtZ80Read);
	ZetSetWriteHandler(Sf2mdtZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, CpsZRom + 0x00000);
	ZetMapArea(0x0000, 0x7fff, 2, CpsZRom + 0x00000);
	ZetMapArea(0x8000, 0xbfff, 0, CpsZRom + 0x08000);
	ZetMapArea(0x8000, 0xbfff, 2, CpsZRom + 0x08000);
	ZetMapArea(0xd000, 0xd7ff, 0, Sf2mdtZ80Ram     );
	ZetMapArea(0xd000, 0xd7ff, 1, Sf2mdtZ80Ram     );
	ZetMapArea(0xd000, 0xd7ff, 2, Sf2mdtZ80Ram     );
	ZetMemEnd();
	ZetClose();
	
	BurnYM2151Init(3579540, 50.0);
	
	MSM5205Init(0, Sf2mdtSynchroniseStream, 24000000 / 64, Sf2mdtMSM5205Vck0, MSM5205_S96_4B, 20, 1);
	MSM5205Init(1, Sf2mdtSynchroniseStream, 24000000 / 64, Sf2mdtMSM5205Vck1, MSM5205_S96_4B, 20, 1);
	
	nCpsZ80Cycles = 3579540 * 100 / nBurnFPS;
	
	Sf2mdtNumZ80Banks = (nCpsZRomLen / 0x4000) - 1;
	
	return 0;
}

INT32 Sf2mdtSoundReset()
{
	ZetOpen(0);
	ZetReset();
	BurnYM2151Reset();
	MSM5205Reset();
	Sf2mdtZ80BankAddress = 0x8000;
	ZetMapArea(0x8000, 0xbfff, 0, CpsZRom + Sf2mdtZ80BankAddress);
	ZetMapArea(0x8000, 0xbfff, 2, CpsZRom + Sf2mdtZ80BankAddress);
	ZetClose();
	
	Sf2mdtSoundLatch = 0;
	Sf2mdtSampleBuffer1 = 0;
	Sf2mdtSampleBuffer2 = 0;
	Sf2mdtSampleSelect1 = 0;
	Sf2mdtSampleSelect2 = 0;
	
	return 0;
}

INT32 Sf2mdtSoundExit()
{
	ZetExit();
	BurnYM2151Exit();
	MSM5205Exit();
	
	BurnFree(Sf2mdtZ80Ram);
	
	Sf2mdtZ80BankAddress = 0;
	Sf2mdtSoundLatch = 0;
	Sf2mdtMSM5205Interleave = 0;
	Sf2mdtSampleBuffer1 = 0;
	Sf2mdtSampleBuffer2 = 0;
	Sf2mdtSampleSelect1 = 0;
	Sf2mdtSampleSelect2 = 0;
	Sf2mdtCyclesPerSegment = 0;
	Sf2mdtNumZ80Banks = 0;
	
	nCpsZ80Cycles = 0;

	return 0;
}

void Sf2mdtSoundFrameStart()
{
	Sf2mdtMSM5205Interleave = MSM5205CalcInterleave(0, 3579540);
	Sf2mdtSoundPos = 0;
	Sf2mdtCyclesPerSegment = (INT64)nCpsZ80Cycles / Sf2mdtMSM5205Interleave;
	
	ZetNewFrame();
	ZetOpen(0);	
}

void Sf2mdtSoundFrameEnd()
{
	for (INT32 i = Sf2mdtSoundPos; i < Sf2mdtMSM5205Interleave; i++) {
		ZetRun(Sf2mdtCyclesPerSegment);
		MSM5205Update();
		Sf2mdtSoundPos = i;
	}
	ZetRun(nCpsZ80Cycles - ZetTotalCycles());
	
	if (pBurnSoundOut) {
		BurnYM2151Render(pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(1, pBurnSoundOut, nBurnSoundLen);
	}
	ZetClose();
}

INT32 Sf2mdtScanSound(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_MEMORY_RAM) {
		struct BurnArea ba;
		memset(&ba, 0, sizeof(ba));

		ba.Data = Sf2mdtZ80Ram;
		ba.nLen = 0x00800;
		ba.szName = "Sf2mdtZ80Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);
		
		BurnYM2151Scan(nAction);
		MSM5205Scan(nAction, pnMin);
		
		SCAN_VAR(Sf2mdtZ80BankAddress);
		SCAN_VAR(Sf2mdtSoundLatch);
		SCAN_VAR(Sf2mdtSampleBuffer1);
		SCAN_VAR(Sf2mdtSampleBuffer2);
		SCAN_VAR(Sf2mdtSampleSelect1);
		SCAN_VAR(Sf2mdtSampleSelect2);
		SCAN_VAR(Sf2mdtSoundPos);
	}

	return 0;
}
