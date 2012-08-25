#include "sys16.h"
#include "dac.h"
#include "i8039.h"
#include "mc8123.h"
#include "upd7759.h"
#include "segapcm.h"

UINT8  System16InputPort0[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8  System16InputPort1[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8  System16InputPort2[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8  System16InputPort3[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8  System16InputPort4[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8  System16InputPort5[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8  System16InputPort6[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8  System16Gear           = 0;
INT32  System16AnalogPort0    = 0;
INT32  System16AnalogPort1    = 0;
INT32  System16AnalogPort2    = 0;
INT32  System16AnalogPort3    = 0;
INT32  System16AnalogPort4    = 0;
INT32  System16AnalogPort5    = 0;
INT32  System16AnalogSelect   = 0;
UINT8  System16Dip[3]         = {0, 0, 0};
UINT8  System16Input[7]       = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
UINT8  System16Reset          = 0;

UINT8  *Mem                   = NULL;
UINT8  *MemEnd                = NULL;
UINT8  *RamStart              = NULL;
UINT8  *RamEnd                = NULL;
UINT8  *System16Rom           = NULL;
UINT8  *System16Code          = NULL;
UINT8  *System16Rom2          = NULL;
UINT8  *System16Rom3          = NULL;
UINT8  *System16Z80Rom        = NULL;
UINT8  *System16Z80Code       = NULL;
UINT8  *System16Z80Rom2       = NULL;
UINT8  *System16Z80Rom3       = NULL;
UINT8  *System16Z80Rom4       = NULL;
UINT8  *System167751Prog      = NULL;
UINT8  *System167751Data      = NULL;
UINT8  *System16UPD7759Data   = NULL;
UINT8  *System16PCMData       = NULL;
UINT8  *System16PCM2Data      = NULL;
UINT8  *System16RF5C68Data    = NULL;
UINT8  *System16Prom          = NULL;
UINT8  *System16Key           = NULL;
UINT8  *System16Ram           = NULL;
UINT8  *System16ExtraRam      = NULL;
UINT8  *System16ExtraRam2     = NULL;
UINT8  *System16ExtraRam3     = NULL;
UINT8  *System16BackupRam     = NULL;
UINT8  *System16BackupRam2    = NULL;
UINT8  *System16Z80Ram        = NULL;
UINT8  *System16Z80Ram2       = NULL;
UINT8  *System16TileRam       = NULL;
UINT8  *System16TextRam       = NULL;
UINT8  *System16SpriteRam     = NULL;
UINT8  *System16SpriteRamBuff = NULL;
UINT8  *System16SpriteRam2    = NULL;
UINT8  *System16RotateRam     = NULL;
UINT8  *System16RotateRamBuff = NULL;
UINT8  *System16PaletteRam    = NULL;
UINT8  *System16RoadRam       = NULL;
UINT8  *System16RoadRamBuff   = NULL;
UINT8  *System16Tiles         = NULL;
UINT8  *System16Sprites       = NULL;
UINT8  *System16Sprites2      = NULL;
UINT8  *System16Roads         = NULL;
UINT32   *System16Palette       = NULL;
UINT8  *System16TempGfx       = NULL;

UINT32 System16RomSize = 0;
UINT32 System16RomNum = 0;
UINT32 System16Rom2Size = 0;
UINT32 System16Rom2Num = 0;
UINT32 System16Rom3Size = 0;
UINT32 System16Rom3Num = 0;
UINT32 System16TileRomSize = 0;
UINT32 System16TileRomNum = 0;
UINT32 System16NumTiles = 0;
UINT32 System16SpriteRomSize = 0;
UINT32 System16SpriteRomNum = 0;
UINT32 System16Sprite2RomSize = 0;
UINT32 System16Sprite2RomNum = 0;
UINT32 System16RoadRomSize = 0;
UINT32 System16RoadRomNum = 0;
UINT32 System16Z80RomSize = 0;
UINT32 System16Z80RomNum = 0;
UINT32 System16Z80Rom2Size = 0;
UINT32 System16Z80Rom2Num = 0;
UINT32 System16Z80Rom3Size = 0;
UINT32 System16Z80Rom3Num = 0;
UINT32 System16Z80Rom4Size = 0;
UINT32 System16Z80Rom4Num = 0;
UINT32 System167751ProgSize = 0;
UINT32 System167751ProgNum = 0;
UINT32 System167751DataSize = 0;
UINT32 System167751DataNum = 0;
UINT32 System16UPD7759DataSize = 0;
UINT32 System16UPD7759DataNum = 0;
UINT32 System16PCMDataSize = 0;
UINT32 System16PCMDataNum = 0;
UINT32 System16PCMDataSizePreAllocate = 0;
UINT32 System16PCM2DataSize = 0;
UINT32 System16PCM2DataNum = 0;
UINT32 System16RF5C68DataSize = 0;
UINT32 System16RF5C68DataNum = 0;
UINT32 System16PromSize = 0;
UINT32 System16PromNum = 0;
UINT32 System16KeySize = 0;
UINT32 System16RamSize = 0;
UINT32 System16ExtraRamSize = 0;
UINT32 System16ExtraRam2Size = 0;
UINT32 System16ExtraRam3Size = 0;
UINT32 System16SpriteRamSize = 0;
UINT32 System16SpriteRam2Size = 0;
UINT32 System16RotateRamSize = 0;
UINT32 System16BackupRamSize = 0;
UINT32 System16BackupRam2Size = 0;

static INT32 System16LastGear;
bool System16HasGears = false;

UINT8 System16VideoControl;
INT32 System16SoundLatch;
bool System16BTileAlt = false;
bool Shangon = false;
bool Hangon = false;
bool System16Z80Enable = true;

INT32 nSystem16CyclesDone[4];
static INT32 nCyclesTotal[4];
static INT32 nCyclesSegment;
UINT32 System16ClockSpeed = 0;

INT32 System16YM2413IRQInterval;

static bool bUseAsm68KCoreOldValue = false;

static UINT8 N7751Command;
static UINT32 N7751RomAddress;
static UINT32 UPD7759BankAddress;
static UINT32 RF5C68PCMBankAddress;

Sim8751 Simulate8751;
System16Map68K System16Map68KDo;
System16MapZ80 System16MapZ80Do;
System16CustomLoadRom System16CustomLoadRomDo;
System16CustomDecryptOpCode System16CustomDecryptOpCodeDo;
System16ProcessAnalogControls System16ProcessAnalogControlsDo;
System16MakeAnalogInputs System16MakeAnalogInputsDo;

/*====================================================
Inputs
====================================================*/

inline static void System16ClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x30) == 0x30) {
		*nJoystickInputs &= ~0x30;
	}
	if ((*nJoystickInputs & 0xc0) == 0xc0) {
		*nJoystickInputs &= ~0xc0;
	}
}

inline static void System16MakeInputs()
{
	// Reset Inputs
	System16Input[0] = System16Input[1] = System16Input[2] = System16Input[3] = System16Input[4] = System16Input[5] = System16Input[6] = 0;

	// Compile Digital Inputs
	for (int i = 0; i < 8; i++) {
		System16Input[0] |= (System16InputPort0[i] & 1) << i;
		System16Input[1] |= (System16InputPort1[i] & 1) << i;
		System16Input[2] |= (System16InputPort2[i] & 1) << i;
		System16Input[3] |= (System16InputPort3[i] & 1) << i;
		System16Input[4] |= (System16InputPort4[i] & 1) << i;
		System16Input[5] |= (System16InputPort5[i] & 1) << i;
		System16Input[6] |= (System16InputPort6[i] & 1) << i;
	}
	
	if (System16MakeAnalogInputsDo) System16MakeAnalogInputsDo();
}

inline static void OutrunMakeInputs()
{
	// Reset Inputs
	System16Input[0] = 0;
	
	if (System16Gear && System16LastGear == 0) System16InputPort0[4] ^= 1;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		System16Input[0] |= (System16InputPort0[i] & 1) << i;
	}
	
	System16LastGear = System16Gear;
}

inline static void PdriftMakeInputs()
{
	// Reset Inputs
	System16Input[0] = 0;
	
	if (System16Gear && System16LastGear == 0) System16InputPort0[5] ^= 1;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		System16Input[0] |= (System16InputPort0[i] & 1) << i;
	}
	
	System16LastGear = System16Gear;
}

static void System16GunMakeInputs()
{
	if (nBurnGunNumPlayers) BurnGunMakeInputs(0, (INT16)System16AnalogPort0, (INT16)System16AnalogPort1);
	if (nBurnGunNumPlayers >= 2) BurnGunMakeInputs(1, (INT16)System16AnalogPort2, (INT16)System16AnalogPort3);
	if (nBurnGunNumPlayers >= 3) BurnGunMakeInputs(2, (INT16)System16AnalogPort4, (INT16)System16AnalogPort5);
}

/*====================================================
Reset Driver
====================================================*/

static INT32 System16DoReset()
{
	INT32 i;
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC_CPU2)) {
		fd1094_machine_init();
	}
	
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_ISGSM) {
		SekOpen(0);
		SekMapMemory(System16Rom, 0x000000, 0x0fffff, SM_ROM);
		SekClose();
	}
	
	SekOpen(0);
	SekReset();
	SekClose();
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMX || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMY) {
		SekOpen(1);
		SekReset();
		SekClose();
		
		System16LastGear = 0;
		System16RoadControl = 0;
		System16AnalogSelect = 0;
		
		if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN) {
			// Start in low gear
			if (System16HasGears) System16InputPort0[4] = 1;
		}
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMY) {
		SekOpen(2);
		SekReset();
		SekClose();
		
		// Start in low gear
		if (System16HasGears) System16InputPort0[5] = 1;
	}
	
	if (System16Z80RomNum || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_ISGSM)) {
		ZetOpen(0);
		ZetReset();
		ZetClose();
	}
	
	if (System16Z80Rom2Num) {
		ZetOpen(1);
		ZetReset();
		ZetClose();
	}
	
	if (System167751ProgSize) {
		N7751Reset();
		DACReset();
		N7751Command = 0;
		N7751RomAddress = 0;
	}
	
	if (System16UPD7759DataSize) {
		UPD7759Reset();
		UPD7759BankAddress = 0;
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18) {
		BurnYM3438Reset();
		RF5C68PCMReset();
	} else {
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2203) {
			BurnYM2203Reset();
		} else {
			if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2413) {
				BurnYM2413Reset();
			} else {
				BurnYM2151Reset();
			}
		}
	}
	
	// Reset Variables
	for (i = 0; i < 4; i++) {
		System16Page[i] = 0;
		System16OldPage[i] = 0;
		System16ScrollX[i] = 0;
		System16ScrollY[i] = 0;
		BootlegBgPage[i] = 0;
		BootlegFgPage[i] = 0;
	}
	
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_5358) {
		System16SpriteBanks[ 0] = 0;
		System16SpriteBanks[ 1] = 255;
		System16SpriteBanks[ 2] = 255;
		System16SpriteBanks[ 3] = 255;
		System16SpriteBanks[ 4] = 255;
		System16SpriteBanks[ 5] = 255;
		System16SpriteBanks[ 6] = 255;
		System16SpriteBanks[ 7] = 3;
		System16SpriteBanks[ 8] = 255;
		System16SpriteBanks[ 9] = 255;
		System16SpriteBanks[10] = 255;
		System16SpriteBanks[11] = 2;
		System16SpriteBanks[12] = 255;
		System16SpriteBanks[13] = 1;
		System16SpriteBanks[14] = 0;
		System16SpriteBanks[15] = 255;
	} else {
		for (i = 0; i < 16; i++) {
			System16SpriteBanks[i] = i;
		}
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16B || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18 || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMX) {
		for (i = 0; i < 8; i++) {
			System16TileBanks[i] = i;
			System16OldTileBanks[i] = i;
		}
	}
	
	System16VideoEnable = 0;
	System18VdpEnable = 0;
	System18VdpMixing = 0;
	System16VideoControl = 0;
	System16ScreenFlip = 0;
	System16SoundLatch = 0;
	System16ColScroll = 0;
	System16RowScroll = 0;;
	
	return 0;
}

INT32 __fastcall OutrunResetCallback()
{
	INT32 nLastCPU = nSekActive;
	SekClose();
	
	SekOpen(1);
	SekReset();
	SekClose();
	
	SekOpen(nLastCPU);

	return 0;
}

/*====================================================
Z80 Memory Handlers
====================================================*/

UINT8 __fastcall System16Z80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x01: {
			return BurnYM2151ReadStatus();
		}
		
		case 0x40:
		case 0xc0: {
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return System16SoundLatch;
		}
		
		case 0x80: {
			if (System16UPD7759DataSize) {
				return UPD7759BusyRead(0) << 7; 
			}
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read Port -> %02X\n"), a);
#endif

	return 0;
}

UINT8 __fastcall System16PPIZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x01: {
			return BurnYM2151ReadStatus();
		}
		
		case 0x40:
		case 0xc0: {
			ppi8255_set_portC(0, 0x00);
			return System16SoundLatch;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read Port -> %02X\n"), a);
#endif

	return 0;
}

UINT8 __fastcall SystemXZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x01: {
			return BurnYM2151ReadStatus();
		}
		
		case 0x40:
		case 0xc0: {
			return System16SoundLatch;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read Port -> %02X\n"), a);
#endif

	return 0;
}

UINT8 __fastcall System16Z80PortRead2(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x40:
		case 0xc0: {
			return System16SoundLatch;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read Port -> %02X\n"), a);
#endif

	return 0;
}

void __fastcall System16Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	d &= 0xff;
	
	switch (a) {
		case 0x00: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0x01: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0x40: {
			if (System16UPD7759DataSize) {
				UPD7759StartWrite(0,d & 0x80);
				UPD7759ResetWrite(0,d & 0x40);
				
				if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_5358) {
					if (!(d & 0x04)) UPD7759BankAddress = 0x00000;
					if (!(d & 0x08)) UPD7759BankAddress = 0x10000;
					if (!(d & 0x10)) UPD7759BankAddress = 0x20000;
					if (!(d & 0x20)) UPD7759BankAddress = 0x30000;
					UPD7759BankAddress += (d & 0x03) * 0x4000;
					
				}
				
				if ((BurnDrvGetHardwareCode() & HARDWARE_SEGA_5521) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_5704_PS2)) {
					UPD7759BankAddress = ((d & 0x08) >> 3) * 0x20000;
					UPD7759BankAddress += (d & 0x07) * 0x4000;
				}
				
				if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_5797) {
					UPD7759BankAddress = ((d & 0x08) >> 3) * 0x40000;
					UPD7759BankAddress += ((d & 0x10) >> 4) * 0x20000;
					UPD7759BankAddress += (d & 0x07) * 0x04000;
				}
				
				UPD7759BankAddress %= System16UPD7759DataSize;
				
				ZetMapArea(0x8000, 0xdfff, 0, System16UPD7759Data + UPD7759BankAddress);
				ZetMapArea(0x8000, 0xdfff, 2, System16UPD7759Data + UPD7759BankAddress);
				return;
			}
		}
		
		case 0x80: {
			if (System167751ProgSize) {
				N7751RomAddress &= 0x3fff;
				N7751RomAddress |= (d & 0x01) << 14;
				if (!(d & 0x02) && System167751DataNum >= 1) N7751RomAddress |= 0x00000;
				if (!(d & 0x04) && System167751DataNum >= 2) N7751RomAddress |= 0x08000;
				if (!(d & 0x08) && System167751DataNum >= 3) N7751RomAddress |= 0x10000;
				if (!(d & 0x10) && System167751DataNum >= 4) N7751RomAddress |= 0x18000;
				N7751Command = d >> 5;			
				return;
			}
			
			if (System16UPD7759DataSize) {
				UPD7759PortWrite(0,d);
				return;
			}
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Write Port -> %02X, %02X\n"), a, d);
#endif
}

#if 0 && defined FBA_DEBUG
UINT8 __fastcall System16Z80Read(UINT16 a)
{
	bprintf(PRINT_NORMAL, _T("Z80 Read -> %04X\n"), a);

	return 0;
}
#endif

#if 0 && defined FBA_DEBUG
void __fastcall System16Z80Write(UINT16 a, UINT8 d)
{
	bprintf(PRINT_NORMAL, _T("Z80 Write -> %04X, %02X\n"), a, d);
}
#endif

UINT8 __fastcall System16Z80PCMRead(UINT16 a)
{
	if (a >= 0xf000 && a <= 0xf0ff) {
		return SegaPCMRead(0, a - 0xf000);
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read -> %04X\n"), a);
#endif

	return 0;
}

void __fastcall System16Z80PCMWrite(UINT16 a, UINT8 d)
{
	if (a >= 0xf000 && a <= 0xf0ff) {
		SegaPCMWrite(0, a - 0xf000, d);
		return;
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Write -> %04X, %02X\n"), a, d);
#endif
}

UINT8 __fastcall System16Z802203PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x40: {
			ppi8255_set_portC(0, 0x00);
			return System16SoundLatch;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read Port -> %02X\n"), a);
#endif

	return 0;
}

UINT8 __fastcall System16Z802203Read(UINT16 a)
{
	if (a >= 0xe000 && a <= 0xe0ff) {
		return SegaPCMRead(0, a - 0xe000);
	}
	
	switch (a) {
		case 0xd000: {
			return BurnYM2203Read(0, 0);
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read -> %04X\n"), a);
#endif

	return 0;
}

void __fastcall System16Z802203Write(UINT16 a, UINT8 d)
{
	if (a >= 0xe000 && a <= 0xe0ff) {
		SegaPCMWrite(0, a - 0xe000, d);
		return;
	}
	
	switch (a) {
		case 0xd000: {
			BurnYM2203Write(0, 0, d);
			return;
		}
		
		case 0xd001: {
			BurnYM2203Write(0, 1, d);
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Write -> %04X, %02X\n"), a, d);
#endif
}

UINT8 __fastcall System16Z80Read2(UINT16 a)
{
	if (a >= 0xf000 && a <= 0xf0ff) {
		return SegaPCMRead(1, a - 0xf000);
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read -> %04X\n"), a);
#endif

	return 0;
}

void __fastcall System16Z80Write2(UINT16 a, UINT8 d)
{
	if (a >= 0xf000 && a <= 0xf0ff) {
		SegaPCMWrite(1, a - 0xf000, d);
		return;
	}
	
#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Write -> %04X, %02X\n"), a, d);
#endif
}

UINT8 __fastcall System18Z80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x80: {
			return BurnYM3438Read(0, 0);
		}
		
		case 0xc0: {
			return System16SoundLatch;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read Port -> %02X\n"), a);
#endif

	return 0;
}

void __fastcall System18Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x80: {
			BurnYM3438Write(0, 0, d);
			return;
		}
		
		case 0x81: {
			BurnYM3438Write(0, 1, d);
			return;
		}
		
		case 0x82: {
			BurnYM3438Write(0, 2, d);
			return;
		}
		
		case 0x83: {
			BurnYM3438Write(0, 3, d);
			return;			
		}
		
		case 0x90: {
			BurnYM3438Write(1, 0, d);
			return;
		}
		
		case 0x91: {
			BurnYM3438Write(1, 1, d);
			return;
		}
		
		case 0x92: {
			BurnYM3438Write(1, 2, d);
			return;
		}
		
		case 0x93: {
			BurnYM3438Write(1, 3, d);
			return;
		}
		
		case 0xa0: {
			RF5C68PCMBankAddress = d * 0x2000;
			ZetMapArea(0xa000, 0xbfff, 0, System16Z80Rom + 0x10000 + RF5C68PCMBankAddress);
			ZetMapArea(0xa000, 0xbfff, 2, System16Z80Rom + 0x10000 + RF5C68PCMBankAddress);
			return;
		}
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Write Port -> %02X, %02X\n"), a, d);
#endif
}

UINT8 __fastcall System18Z80Read(UINT16 a)
{
	if (a >= 0xd000 && a <= 0xdfff) {
		return RF5C68PCMRead(a - 0xd000);
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Read -> %04X\n"), a);
#endif

	return 0;
}

void __fastcall System18Z80Write(UINT16 a, UINT8 d)
{
	if (a >= 0xc000 && a <= 0xc00f) {
		RF5C68PCMRegWrite(a - 0xc000, d);
		return;
	}
	
	if (a >= 0xd000 && a <= 0xdfff) {
		RF5C68PCMWrite(a - 0xd000, d);
		return;
	}

#if 0 && defined FBA_DEBUG
	bprintf(PRINT_NORMAL, _T("Z80 Write -> %04X, %02X\n"), a, d);
#endif
}

void System16N7751ControlWrite(UINT32 Port, UINT32 Data)
{
	if (!(Data & 0x01)) {
		N7751Reset();
		return;
	}
	
	if (!(Data & 0x02)) {
		N7751SetIrqState(1);
		return;
	}
	
	if (Data & 0x02) {
		N7751SetIrqState(0);
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("YM2151 Write Port %x, %02X\n"), Port, Data);
}

UINT8 __fastcall N7751ReadIo(UINT32 Port)
{
	switch (Port) {
		case 0x102: {
			return 0x80 | ((N7751Command & 0x07) << 4);
		}
		
		case 0x120: {
			return System167751Data[N7751RomAddress];
		}
	}
	
	return 0;
}

void __fastcall N7751WriteIo(UINT32 Port, UINT8 Value)
{
	switch (Port) {
		case 0x101: {
			DACWrite(0, Value);
			return;
		}
		
		case 0x102: {
			return;
		}
		
		case 0x104:
		case 0x105:
		case 0x106:
		case 0x107: {
			INT32 Offset = Port - 0x104;
			INT32 Mask = (0xf << (4 * Offset)) & 0x3fff;
			INT32 NewData = (Value << (4 * Offset)) & Mask;
			N7751RomAddress = (N7751RomAddress & ~Mask) | NewData;
			return;
		}
	}
}

UINT8 __fastcall N7751Read(UINT32 Address)
{
	if (Address < 0x400) return System167751Prog[Address];
	return 0;
}

/*====================================================
Allocate Memory
====================================================*/

static INT32 System16MemIndex()
{
	UINT8 *Next; Next = Mem;
	
	System16PaletteEntries = 0x800;
	System16RamSize = 0x4000;
	System16ExtraRamSize = 0;
	System16ExtraRam2Size = 0;
	System16ExtraRam3Size = 0;
	System16SpriteRamSize = 0x800;
	System16SpriteRam2Size = 0;
	System16RotateRamSize = 0;
	System16BackupRamSize = 0;
	System16BackupRam2Size = 0;
	bool SpriteBuffer = false;
	bool HasRoad = false;
	bool UseTempDraw = false;
	INT32 Z80RomSize = 0x10000;
	INT32 Z80RamSize = 0x00800;
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18) {
		Z80RomSize = 0x210000;
		Z80RamSize = 0x2000;
		UseTempDraw = true;
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON) {
		System16ExtraRamSize = 0x4000;
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "hangon") || !strcmp(BurnDrvGetTextA(DRV_NAME), "shangupb")) {
			System16RamSize = 0x10000;
		} else {
			System16SpriteRamSize = 0x1000;
		}
		HasRoad = true;
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN) {
		System16PaletteEntries = 0x1000;
		System16RamSize = 0x8000;
		System16ExtraRamSize = 0x8000;
		System16SpriteRamSize = 0x1000;
		SpriteBuffer = true;
		HasRoad = true;
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMX) {
		System16PaletteEntries = 0x2000;
		System16RamSize = 0x8000;
		System16SpriteRamSize = 0x1000;
		System16BackupRamSize = 0x4000;
		System16BackupRam2Size = 0x4000;
		SpriteBuffer = true;
		HasRoad = true;
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMY) {
		System16PaletteEntries = 0x2000;
		System16RamSize = 0x10000;
		System16ExtraRamSize = 0x10000;
		System16ExtraRam2Size = 0x10000;
		System16ExtraRam3Size = 0x10000;
		System16SpriteRamSize = 0x1000;
		System16BackupRamSize = 0x4000;
		System16SpriteRam2Size = 0x10000;
		System16RotateRamSize = 0x800;
		UseTempDraw = true;
	}
	
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_ISGSM) {
		System16ExtraRamSize = 0x40000;
		Z80RomSize = 0x40000;
	}
	
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_5704_PS2) {
		System16RamSize = 0x40000;
	}
	
	System16Rom          = Next; Next += (System16RomSize > 0x100000) ? System16RomSize : 0x100000;
	System16Code         = Next; Next += (System16RomSize > 0x100000) ? System16RomSize : 0x100000;
	System16Rom2         = Next; (System16Rom2Size) ? Next += 0x080000 : Next += 0;
	System16Rom3         = Next; (System16Rom3Size) ? Next += 0x080000 : Next += 0;
	System16Z80Rom       = Next; Next += Z80RomSize;
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MC8123_ENC) {
		System16Z80Code      = Next; Next += Z80RomSize;
	}	
	System16Z80Rom2      = Next; Next += System16Z80Rom2Size;
	System16Z80Rom3      = Next; Next += System16Z80Rom3Size;
	System16Z80Rom4      = Next; Next += System16Z80Rom4Size;
	System167751Prog     = Next; Next += System167751ProgSize;
	System167751Data     = Next; Next += System167751DataSize;
	System16UPD7759Data  = Next; Next += System16UPD7759DataSize;
	System16PCMData      = Next; Next += System16PCMDataSize;
	System16PCM2Data     = Next; Next += System16PCM2DataSize;
	System16RF5C68Data   = Next; Next += System16RF5C68DataSize;
	System16Key          = Next; Next += System16KeySize;
	System16Prom         = Next; Next += System16PromSize;
	
	RamStart = Next;

	System16Ram          = Next; Next += System16RamSize;
	System16ExtraRam     = Next; Next += System16ExtraRamSize;
	System16ExtraRam2    = Next; Next += System16ExtraRam2Size;
	System16ExtraRam3    = Next; Next += System16ExtraRam3Size;
	System16TileRam      = Next; (System16TileRomSize) ? Next += 0x10000 : Next += 0;
	System16TextRam      = Next; (System16TileRomSize) ? Next += 0x01000 : Next += 0;
	System16SpriteRam    = Next; Next += System16SpriteRamSize;
	System16SpriteRam2   = Next; Next += System16SpriteRam2Size;
	System16RotateRam    = Next; Next += System16RotateRamSize;
	System16RotateRamBuff= Next; Next += System16RotateRamSize;
	if (SpriteBuffer) System16SpriteRamBuff = Next; Next += System16SpriteRamSize;
	System16PaletteRam   = Next; Next += System16PaletteEntries * 2;

	if (HasRoad) {
		System16RoadRam       = Next; Next += 0x01000;
		System16RoadRamBuff   = Next; Next += 0x01000;
	}
	
	System16BackupRam    = Next; Next += System16BackupRamSize;
	System16BackupRam2   = Next; Next += System16BackupRam2Size;
	
	System16Z80Ram       = Next; Next += Z80RamSize;
	
	if (System16Z80Rom2Num) {
		System16Z80Ram2  = Next; Next += 0x800;
	}

	RamEnd = Next;

	System16Tiles        = Next; Next += (System16NumTiles * 8 * 8);
	System16Sprites      = Next; Next += System16SpriteRomSize;
	System16Sprites2     = Next; Next += System16Sprite2RomSize;
	
	if (HasRoad) {
		System16Roads        = Next; Next += 0x40000;
	}
	
	System16Palette      = (UINT32*)Next; Next += System16PaletteEntries * 3 * sizeof(UINT32) + (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18) ? (0x40 * sizeof(UINT32)) : 0);
	
	if (UseTempDraw) pTempDraw = (UINT16*)Next; Next += (512 * 512 * sizeof(UINT16));
	
	MemEnd = Next;

	return 0;
}

/*====================================================
Rom Loading
====================================================*/

INT32 System16LoadRoms(bool bLoad)
{
	struct BurnRomInfo ri;
	ri.nType = 0;
	ri.nLen = 0;
	INT32 nOffset = -1;
	UINT32 i;
	INT32 nRet = 0;
	
	if (!bLoad) {
		do {
			ri.nLen = 0;
			ri.nType = 0;
			BurnDrvGetRomInfo(&ri, ++nOffset);
			if ((ri.nType & 0xff) == SYS16_ROM_PROG) {
				System16RomSize += ri.nLen;
				System16RomNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_PROG2) {
				System16Rom2Size += ri.nLen;
				System16Rom2Num++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_PROG3) {
				System16Rom3Size += ri.nLen;
				System16Rom3Num++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_TILES) {
				System16TileRomSize += ri.nLen;
				System16TileRomNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_SPRITES) {
				System16SpriteRomSize += ri.nLen;
				System16SpriteRomNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_SPRITES2) {
				System16Sprite2RomSize += ri.nLen;
				System16Sprite2RomNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_ROAD) {
				System16RoadRomSize += ri.nLen;
				System16RoadRomNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_Z80PROG) {
				System16Z80RomSize += ri.nLen;
				System16Z80RomNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_Z80PROG2) {
				System16Z80Rom2Size += ri.nLen;
				System16Z80Rom2Num++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_Z80PROG3) {
				System16Z80Rom3Size += ri.nLen;
				System16Z80Rom3Num++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_Z80PROG4) {
				System16Z80Rom4Size += ri.nLen;
				System16Z80Rom4Num++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_7751PROG) {
				System167751ProgSize += ri.nLen;
				System167751ProgNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_7751DATA) {
				System167751DataSize += ri.nLen;
				System167751DataNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_UPD7759DATA) {
				if (ri.nLen < 0x10000) {
					System16UPD7759DataSize += 0x10000;
				} else {
					System16UPD7759DataSize += ri.nLen;
				}
				System16UPD7759DataNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_PCMDATA) {
				if (System16PCMDataSizePreAllocate) {
					System16PCMDataSize = System16PCMDataSizePreAllocate;
				} else {
					System16PCMDataSize += ri.nLen;
				}
				
				System16PCMDataNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_PCM2DATA) {
				System16PCM2DataSize += ri.nLen;
				System16PCM2DataNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_RF5C68DATA) {
				System16RF5C68DataNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_PROM) {
				System16PromSize += ri.nLen;
				System16PromNum++;
			}
			if ((ri.nType & 0xff) == SYS16_ROM_KEY) {
				System16KeySize += ri.nLen;
			}
		} while (ri.nLen);
		
		System16NumTiles = System16TileRomSize / 24;
		if (System16RF5C68DataNum) {
			System16Z80RomNum += System16RF5C68DataNum;
			System16Z80RomSize = 0x210000;
		}
		
#if 1 && defined FBA_DEBUG	
		bprintf(PRINT_NORMAL, _T("68K Rom Size: 0x%X (%i roms)\n"), System16RomSize, System16RomNum);
		if (System16Rom2Size) bprintf(PRINT_NORMAL, _T("68K #2 Rom Size: 0x%X (%i roms)\n"), System16Rom2Size, System16Rom2Num);
		if (System16Rom3Size) bprintf(PRINT_NORMAL, _T("68K #3 Rom Size: 0x%X (%i roms)\n"), System16Rom3Size, System16Rom3Num);
		bprintf(PRINT_NORMAL, _T("Tile Rom Size: 0x%X (%i roms, 0x%X Tiles)\n"), System16TileRomSize, System16TileRomNum, System16NumTiles);
		bprintf(PRINT_NORMAL, _T("Sprite Rom Size: 0x%X (%i roms)\n"), System16SpriteRomSize, System16SpriteRomNum);
		if (System16Sprite2RomSize) bprintf(PRINT_NORMAL, _T("Sprite Rom 2 Size: 0x%X (%i roms)\n"), System16Sprite2RomSize, System16Sprite2RomNum);
		bprintf(PRINT_NORMAL, _T("Z80 Rom Size: 0x%X (%i roms)\n"), System16Z80RomSize, System16Z80RomNum);
		if (System16Z80Rom2Size) bprintf(PRINT_NORMAL, _T("Z80 #2 Rom Size: 0x%X (%i roms)\n"), System16Z80Rom2Size, System16Z80Rom2Num);
		if (System16Z80Rom3Size) bprintf(PRINT_NORMAL, _T("Z80 #3 Rom Size: 0x%X (%i roms)\n"), System16Z80Rom3Size, System16Z80Rom3Num);
		if (System16Z80Rom4Size) bprintf(PRINT_NORMAL, _T("Z80 #4 Rom Size: 0x%X (%i roms)\n"), System16Z80Rom4Size, System16Z80Rom4Num);
		if (System16RoadRomSize) bprintf(PRINT_NORMAL, _T("Road Rom Size: 0x%X (%i roms)\n"), System16RoadRomSize, System16RoadRomNum);
		if (System167751ProgSize) bprintf(PRINT_NORMAL, _T("7751 Prog Size: 0x%X (%i roms)\n"), System167751ProgSize, System167751ProgNum);
		if (System167751DataSize) bprintf(PRINT_NORMAL, _T("7751 Data Size: 0x%X (%i roms)\n"), System167751DataSize, System167751DataNum);
		if (System16UPD7759DataSize) bprintf(PRINT_NORMAL, _T("UPD7759 Data Size: 0x%X (%i roms)\n"), System16UPD7759DataSize, System16UPD7759DataNum);
		if (System16PCMDataSize) bprintf(PRINT_NORMAL, _T("PCM Data Size: 0x%X (%i roms)\n"), System16PCMDataSize, System16PCMDataNum);
		if (System16PCM2DataSize) bprintf(PRINT_NORMAL, _T("PCM Data #2 Size: 0x%X (%i roms)\n"), System16PCM2DataSize, System16PCM2DataNum);
		if (System16PromSize) bprintf(PRINT_NORMAL, _T("PROM Rom Size: 0x%X (%i roms)\n"), System16PromSize, System16PromNum);
		if (System16KeySize) bprintf(PRINT_NORMAL, _T("Encryption Key Size: 0x%X\n"), System16KeySize);
#endif
	}
	
	if (bLoad) {
		INT32 Offset;
		
		// 68000 Program Roms
		Offset = 0;
		for (i = 0; i < System16RomNum; i += 2) {
			nRet = BurnLoadRom(System16Rom + Offset + 1, i + 0, 2); if (nRet) return 1;
			nRet = BurnLoadRom(System16Rom + Offset + 0, i + 1, 2); if (nRet) return 1;
			
			BurnDrvGetRomInfo(&ri, i + 0);
			Offset += ri.nLen;
			BurnDrvGetRomInfo(&ri, i + 1);
			Offset += ri.nLen;
		}
		
		// 68000 #2 Program Roms
		if (System16Rom2Size) {
			Offset = 0;
			for (i = System16RomNum; i < System16RomNum + System16Rom2Num; i += 2) {
				nRet = BurnLoadRom(System16Rom2 + Offset + 1, i + 0, 2); if (nRet) return 1;
				nRet = BurnLoadRom(System16Rom2 + Offset + 0, i + 1, 2); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 1);
				Offset += ri.nLen;
			}
		}
		
		// 68000 #3 Program Roms
		if (System16Rom3Size) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num; i < System16RomNum + System16Rom2Num + System16Rom3Num; i += 2) {
				nRet = BurnLoadRom(System16Rom3 + Offset + 1, i + 0, 2); if (nRet) return 1;
				nRet = BurnLoadRom(System16Rom3 + Offset + 0, i + 1, 2); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 1);
				Offset += ri.nLen;
			}
		}

		// Tile Roms
		Offset = 0;
		System16TempGfx = (UINT8*)BurnMalloc(System16TileRomSize);
		for (i = System16RomNum + System16Rom2Num + System16Rom3Num; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum; i++) {
			nRet = BurnLoadRom(System16TempGfx + Offset, i, 1); if (nRet) return 1;
			
			BurnDrvGetRomInfo(&ri, i + 0);
			Offset += ri.nLen;
		}
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_INVERT_TILES) {
			for (i = 0; i < System16TileRomSize; i++) {
				System16TempGfx[i] ^= 0xff;
			}	
		}
		System16Decode8x8Tiles(System16Tiles, System16NumTiles, System16TileRomSize * 2 / 3, System16TileRomSize * 1 / 3, 0);
		BurnFree(System16TempGfx);
		
		// Sprite Roms
		Offset = 0;
		
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_SPRITE_LOAD32) {
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum; i += 4) {
				nRet = BurnLoadRom(System16Sprites + Offset + 0, i + 0, 4); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites + Offset + 1, i + 1, 4); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites + Offset + 2, i + 2, 4); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites + Offset + 3, i + 3, 4); if (nRet) return 1;
				
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 1);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 2);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 3);
				Offset += ri.nLen;
			}
		} else {
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum; i += 2) {
				nRet = BurnLoadRom(System16Sprites + Offset + 0, i + 0, 2); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites + Offset + 1, i + 1, 2); if (nRet) return 1;
				
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 1);
				Offset += ri.nLen;
			}
		}
		
		// Sprite 2 Roms
		if (System16Sprite2RomSize) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum; i += 8) {
				nRet = BurnLoadRom(System16Sprites2 + Offset + 7, i + 0, 8); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites2 + Offset + 6, i + 1, 8); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites2 + Offset + 5, i + 2, 8); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites2 + Offset + 4, i + 3, 8); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites2 + Offset + 3, i + 4, 8); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites2 + Offset + 2, i + 5, 8); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites2 + Offset + 1, i + 6, 8); if (nRet) return 1;
				nRet = BurnLoadRom(System16Sprites2 + Offset + 0, i + 7, 8); if (nRet) return 1;
				
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 1);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 2);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 3);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 4);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 5);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 6);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 7);
				Offset += ri.nLen;
			}
		}
		
		// Road Roms
		if (System16RoadRomSize) {
			Offset = 0;
			System16TempGfx = (UINT8*)BurnMalloc(System16RoadRomSize);
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum; i++) {
				nRet = BurnLoadRom(System16TempGfx + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
			if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMX) OutrunDecodeRoad();
			if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON) HangonDecodeRoad();
			BurnFree(System16TempGfx);
		}
		
		// Z80 Program Roms
		Offset = 0;
		if (System16RF5C68DataNum) Offset = 0x10000;
		for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum; i++) {
			nRet = BurnLoadRom(System16Z80Rom + Offset, i, 1);
			if (nRet) {
				// Fill with 0xff
				memset(System16Z80Rom, 0xff, System16Z80RomSize);
				
				nRet = 0;
			}
			
			BurnDrvGetRomInfo(&ri, i + 0);
			if (System16RF5C68DataNum) {
				Offset += 0x80000;
			} else {
				Offset += ri.nLen;
			}
		}		
		if (System16RF5C68DataNum) {
			memcpy(System16Z80Rom, System16Z80Rom + 0x10000, 0x10000);
			System16RF5C68DataNum = 0;
		}
		
		// Z80 #2 Program Roms
		if (System16Z80Rom2Size) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum+ System16Z80RomNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num; i++) {
				nRet = BurnLoadRom(System16Z80Rom2 + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// Z80 #3 Program Roms
		if (System16Z80Rom3Size) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num; i++) {
				nRet = BurnLoadRom(System16Z80Rom3 + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// Z80 #4 Program Roms
		if (System16Z80Rom4Size) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num; i++) {
				nRet = BurnLoadRom(System16Z80Rom4 + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// 7751 Program Roms
		if (System167751ProgSize) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum; i++) {
				nRet = BurnLoadRom(System167751Prog + Offset, i, 1); if (nRet) return 1;
				
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// 7751 Data Roms
		if (System167751DataSize) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum; i++) {
				nRet = BurnLoadRom(System167751Data + Offset, i, 1); if (nRet) return 1;
				
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// UPD7759 Data Roms
		if (System16UPD7759DataSize) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum + System16UPD7759DataNum; i++) {
				nRet = BurnLoadRom(System16UPD7759Data + Offset, i, 1);
				
				if (nRet) {
					memset(System16UPD7759Data, 0xff, System16UPD7759DataSize); 
					
					nRet = 0;
				}
				
				BurnDrvGetRomInfo(&ri, i + 0);
				if (ri.nLen < 0x10000) {
					Offset += 0x10000;
				} else {
					Offset += ri.nLen;
				}
			}
		}
		
		// PCM Data Roms
		if (System16PCMDataSize) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum + System16UPD7759DataNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum + System16UPD7759DataNum + System16PCMDataNum; i++) {
				nRet = BurnLoadRom(System16PCMData + Offset, i, 1); if (nRet) return 1;
				
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// PCM2 Data Roms
		if (System16PCM2DataSize) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum + System16UPD7759DataNum + System16PCMDataNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum + System16UPD7759DataNum + System16PCMDataNum + System16PCM2DataNum; i++) {
				nRet = BurnLoadRom(System16PCM2Data + Offset, i, 1); if (nRet) return 1;
				
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// PROMs
		if (System16PromSize) {
			Offset = 0;
			for (i = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum + System16UPD7759DataNum + System16PCMDataNum + System16PCM2DataNum + System16RF5C68DataNum; i < System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum + System16UPD7759DataNum + System16PCMDataNum + System16PCM2DataNum + System16RF5C68DataNum + System16PromNum; i++) {
				nRet = BurnLoadRom(System16Prom + Offset, i, 1); if (nRet) return 1;
				
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// Encryption Key
		if (System16KeySize) {
			Offset = System16RomNum + System16Rom2Num + System16Rom3Num + System16TileRomNum + System16SpriteRomNum + System16Sprite2RomNum + System16RoadRomNum + System16Z80RomNum + System16Z80Rom2Num + System16Z80Rom3Num + System16Z80Rom4Num + System167751ProgNum + System167751DataNum + System16UPD7759DataNum + System16PCMDataNum + System16PCM2DataNum + System16RF5C68DataNum + System16PromNum;
			nRet = BurnLoadRom(System16Key, Offset, 1); if (nRet) return 1;
		}
	}
	
	return nRet;
}

INT32 CustomLoadRom20000()
{
	INT32 nRet = 1;
	UINT8 *pTemp = (UINT8*)BurnMalloc(0xc0000);
	
	if (pTemp) {
		memcpy(pTemp, System16Rom, 0xc0000);
		memset(System16Rom, 0, 0xc0000);
		memcpy(System16Rom + 0x00000, pTemp + 0x00000, 0x20000);
		memcpy(System16Rom + 0x80000, pTemp + 0x20000, 0x40000);
		BurnFree(pTemp);
		nRet = 0;
	}
		
	return nRet;
}

INT32 CustomLoadRom40000()
{
	INT32 nRet = 1;
	UINT8 *pTemp = (UINT8*)BurnMalloc(0xc0000);
	
	if (pTemp) {
		memcpy(pTemp, System16Rom, 0xc0000);
		memset(System16Rom, 0, 0xc0000);
		memcpy(System16Rom + 0x00000, pTemp + 0x00000, 0x40000);
		memcpy(System16Rom + 0x80000, pTemp + 0x40000, 0x40000);
		BurnFree(pTemp);
		nRet = 0;
	}
		
	return nRet;
}

/*====================================================
Sound Support Functions
====================================================*/

inline static INT32 System16SndGetBank(INT32 Reg86)
{
	return (Reg86>>4)&7;
}

inline static INT32 PdriftSndGetBank(INT32 Reg86)
{
	return (Reg86>>3)&0x1f;
}

inline void System16YM2151IRQHandler(INT32 Irq)
{
	if (Irq) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

inline static void System16YM2203IRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

inline static INT32 System16SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

inline static double System16GetTime()
{
	return (double)ZetTotalCycles() / 4000000;
}

inline static INT32 System18SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 8000000;
}

inline static double System18GetTime()
{
	return (double)ZetTotalCycles() / 8000000;
}

static void System16UPD7759DrqCallback(INT32 state)
{
	if (state) ZetNmi();
}

static INT32 System16ASyncDAC()
{
	return (INT32)(float)(nBurnSoundLen * (I8039TotalCycles() / ((6000000.0000 / 15) / (nBurnFPS / 100.0000))));
}

/*====================================================
Multiply Protection Chip Emulation
====================================================*/

struct multiply_chip
{
	UINT16	regs[4];
};

static struct multiply_chip multiply[3];

UINT16 System16MultiplyChipRead(INT32 which, INT32 offset)
{
	offset &= 3;
	switch (offset) {
		case 0:	{
			return multiply[which].regs[0];
		}
		
		case 1:	{
			return multiply[which].regs[1];
		}
		
		case 2:	{
			return ((INT16)multiply[which].regs[0] * (INT16)multiply[which].regs[1]) >> 16;
		}
		
		case 3:	{
			return ((INT16)multiply[which].regs[0] * (INT16)multiply[which].regs[1]) & 0xffff;
		}
	}
	
	return 0xffff;
}

void System16MultiplyChipWrite(INT32 which, INT32 offset, UINT16 data)
{
	offset &= 3;
	switch (offset) {
		case 0: {
			multiply[which].regs[0] = data;
			return;
		}
		
		case 1: {
			multiply[which].regs[1] = data;
			return;
		}
		
		case 2: {
			multiply[which].regs[0] = data;
			return;
		}
		
		case 3: {
			multiply[which].regs[1] = data;
			return;
		}
	}
}

/*====================================================
Divide Protection Chip Emulation
====================================================*/

struct divide_chip
{
	UINT16 	regs[8];
};

static struct divide_chip divide[3];

static void update_divide(INT32 which, INT32 mode)
{
	/* clear the flags by default */
	divide[which].regs[6] = 0;

	/* if mode 0, store quotient/remainder */
	if (mode == 0)
	{
		INT32 dividend = (INT32)((divide[which].regs[0] << 16) | divide[which].regs[1]);
		INT32 divisor = (INT16)divide[which].regs[2];
		INT32 quotient, remainder;

		/* perform signed divide */
		if (divisor == 0)
		{
			quotient = dividend;//((INT32)(dividend ^ divisor) < 0) ? 0x8000 : 0x7fff;
			divide[which].regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;
		remainder = dividend - quotient * divisor;

		/* clamp to 16-bit signed */
		if (quotient < -32768)
		{
			quotient = -32768;
			divide[which].regs[6] |= 0x8000;
		}
		else if (quotient > 32767)
		{
			quotient = 32767;
			divide[which].regs[6] |= 0x8000;
		}

		/* store quotient and remainder */
		divide[which].regs[4] = quotient;
		divide[which].regs[5] = remainder;
	}

	/* if mode 1, store 32-bit quotient */
	else
	{
		UINT32 dividend = (UINT32)((divide[which].regs[0] << 16) | divide[which].regs[1]);
		UINT32 divisor = (UINT16)divide[which].regs[2];
		UINT32 quotient;

		/* perform unsigned divide */
		if (divisor == 0)
		{
			quotient = dividend;//0x7fffffff;
			divide[which].regs[6] |= 0x4000;
		}
		else
			quotient = dividend / divisor;

		/* store 32-bit quotient */
		divide[which].regs[4] = quotient >> 16;
		divide[which].regs[5] = quotient & 0xffff;
	}
}

UINT16 System16DivideChipRead(INT32 which, INT32 offset)
{
	offset &= 7;
	switch (offset) {
		case 0:	{
			return divide[which].regs[0];
		}
		
		case 1:	{
			return divide[which].regs[1];
		}
		
		case 2:	{
			return divide[which].regs[2];
		}
		
		case 4:	{
			return divide[which].regs[4];
		}
		
		case 5:	{
			return divide[which].regs[5];
		}
		
		case 6:	{
			return divide[which].regs[6];
		}
	}
	
	return 0xffff;
}

void System16DivideChipWrite(INT32 which, INT32 offset, UINT16 data)
{
	INT32 a4 = offset & 8;
	INT32 a3 = offset & 4;
	
	offset &= 3;
	switch (offset) {
		case 0: {
			divide[which].regs[0] = data;
			break;
		}
		
		case 1: {
			divide[which].regs[1] = data;
			break;
		}
		
		case 2: {
			divide[which].regs[2] = data;
			break;
		}
		
		case 3: {
			break;
		}
	}
	
	if (a4) update_divide(which, a3);
}

/*====================================================
Compare Timer Protection Chip Emulation
====================================================*/

struct compare_timer_chip
{
	UINT16	regs[16];
	UINT16	counter;
	UINT8	bit;
};

static struct compare_timer_chip compare_timer[2];

inline static INT32 segaic16_compare_timer_clock(INT32 which)
{
	INT32 old_counter = compare_timer[which].counter;
	INT32 result = 0;

	/* if we're enabled, clock the upcounter */
	if (compare_timer[which].regs[10] & 1)
		compare_timer[which].counter++;

	/* regardless of the enable, a value of 0xfff will generate the IRQ */
	if (old_counter == 0xfff)
	{
		result = 1;
		compare_timer[which].counter = compare_timer[which].regs[8] & 0xfff;
	}
	return result;
}

inline static void update_compare(INT32 which, INT32 update_history)
{
	INT32 bound1 = (INT16)compare_timer[which].regs[0];
	INT32 bound2 = (INT16)compare_timer[which].regs[1];
	INT32 value = (INT16)compare_timer[which].regs[2];
	INT32 min = (bound1 < bound2) ? bound1 : bound2;
	INT32 max = (bound1 > bound2) ? bound1 : bound2;

	if (value < min)
	{
		compare_timer[which].regs[7] = min;
		compare_timer[which].regs[3] = 0x8000;
	}
	else if (value > max)
	{
		compare_timer[which].regs[7] = max;
		compare_timer[which].regs[3] = 0x4000;
	}
	else
	{
		compare_timer[which].regs[7] = value;
		compare_timer[which].regs[3] = 0x0000;
	}

	if (update_history)
		compare_timer[which].regs[4] |= (compare_timer[which].regs[3] == 0) << compare_timer[which].bit++;
}

UINT16 System16CompareTimerChipRead(INT32 which, INT32 offset)
{
	offset &= 0xf;
	switch (offset) {
		case 0x0:	return compare_timer[which].regs[0];
		case 0x1:	return compare_timer[which].regs[1];
		case 0x2:	return compare_timer[which].regs[2];
		case 0x3:	return compare_timer[which].regs[3];
		case 0x4:	return compare_timer[which].regs[4];
		case 0x5:	return compare_timer[which].regs[1];
		case 0x6:	return compare_timer[which].regs[2];
		case 0x7:	return compare_timer[which].regs[7];
	}
	
	return 0xffff;
}

void System16CompareTimerChipWrite(INT32 which, INT32 offset, UINT16 data)
{
	offset &= 0xf;
	switch (offset) {
		case 0x0:	compare_timer[which].regs[0] = data; update_compare(which, 0); break;
		case 0x1:	compare_timer[which].regs[1] = data; update_compare(which, 0); break;
		case 0x2:	compare_timer[which].regs[2] = data; update_compare(which, 1); break;
		case 0x4:	compare_timer[which].regs[4] = 0; compare_timer[which].bit = 0; break;
		case 0x6:	compare_timer[which].regs[2] = data; update_compare(which, 0); break;
		case 0x8:
		case 0xc:	compare_timer[which].regs[8] = data; break;
		case 0xa:
		case 0xe:	compare_timer[which].regs[10] = data; break;
		case 0xb:
		case 0xf:
			compare_timer[which].regs[11] = data;
			break;
	}
}

/*====================================================
Main Driver Init function
====================================================*/

INT32 System16Init()
{
	INT32 nRet = 0, nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	System16LoadRoms(0); // Get required rom sizes
	System16MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	System16MemIndex();
	
	// Load Roms
	if (!(BurnDrvGetHardwareCode() & HARDWARE_SEGA_5704_PS2)) {
		nRet = System16LoadRoms(1); if (nRet) return 1;
	}
	if (System16CustomLoadRomDo) { nRet = System16CustomLoadRomDo(); if (nRet) return 1; }
	
	// Copy the first 68000 rom to code (FETCH)
	memcpy(System16Code, System16Rom, System16RomSize);
	
	// Handle any op-code decryption
	if (System16CustomDecryptOpCodeDo) { nRet = System16CustomDecryptOpCodeDo(); if (nRet) return 1; }
	
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1089A_ENC || BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1089B_ENC) {
		FD1089Decrypt();
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC_CPU2)) {
		// Make sure we use Musashi
		if (bBurnUseASMCPUEmulation) {
#if 1 && defined FBA_DEBUG
			bprintf(PRINT_NORMAL, _T("Switching to Musashi 68000 core\n"));
#endif
			bUseAsm68KCoreOldValue = bBurnUseASMCPUEmulation;
			bBurnUseASMCPUEmulation = false;
		}
		
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC) fd1094_driver_init(0);
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC_CPU2) fd1094_driver_init(1);
	}	
	
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MC8123_ENC) {
		mc8123_decrypt_rom(0, 0, System16Z80Rom, System16Z80Code, System16Key);
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16A) {
		if (System16Map68KDo) {
			System16Map68KDo();
		} else {
			SekInit(0, 0x68000);
			SekOpen(0);
			SekMapMemory(System16Rom           , 0x000000, 0x0fffff, SM_READ);
			SekMapMemory(System16Code          , 0x000000, 0x0fffff, SM_FETCH);
			SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
			SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
			SekMapMemory(System16SpriteRam     , 0x440000, 0x4407ff, SM_RAM);
			SekMapMemory(System16PaletteRam    , 0x840000, 0x840fff, SM_RAM);
			SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
		
			SekSetReadWordHandler(0, System16AReadWord);
			SekSetWriteWordHandler(0, System16AWriteWord);
			SekSetReadByteHandler(0, System16AReadByte);
			SekSetWriteByteHandler(0, System16AWriteByte);
			SekClose();
		}
		
		if (System16MapZ80Do) {
			ZetInit(0);
			ZetOpen(0);
			System16MapZ80Do();
			ZetClose();
		} else {
			ZetInit(0);
			ZetOpen(0);
			ZetMapArea(0x0000, 0xdfff, 0, System16Z80Rom);
			ZetMapArea(0x0000, 0xdfff, 2, System16Z80Rom);
	
			ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram);
			ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram);
			ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram);
			ZetMemEnd();
		
#if 0 && defined FBA_DEBUG
			ZetSetReadHandler(System16Z80Read);
			ZetSetWriteHandler(System16Z80Write);
#endif	
			ZetSetInHandler(System16PPIZ80PortRead);
			ZetSetOutHandler(System16Z80PortWrite);
			ZetClose();
		}
		
		if (PPI0PortWriteA == NULL) PPI0PortWriteA = System16APPI0WritePortA;
		if (PPI0PortWriteB == NULL) PPI0PortWriteB = System16APPI0WritePortB;
		if (PPI0PortWriteC == NULL) PPI0PortWriteC = System16APPI0WritePortC;
		ppi8255_init(1);
		
		BurnYM2151Init(4000000);
		BurnYM2151SetAllRoutes(1.00, BURN_SND_ROUTE_BOTH);
		
		if (System167751ProgSize) {
			N7751Init(NULL);
		
			N7751SetIOReadHandler(N7751ReadIo);
			N7751SetIOWriteHandler(N7751WriteIo);
			N7751SetProgramReadHandler(N7751Read);
			N7751SetCPUOpReadHandler(N7751Read);
			N7751SetCPUOpReadArgHandler(N7751Read);
			
			YM2151SetPortWriteHandler(0, &System16N7751ControlWrite);
			BurnYM2151SetAllRoutes(0.43, BURN_SND_ROUTE_BOTH);
			DACInit(0, 0, 1, System16ASyncDAC);
			DACSetRoute(0, 0.80, BURN_SND_ROUTE_BOTH);
		}
		
		System16TileBankSize = 0x1000;
		System16CreateOpaqueTileMaps = 1;
		System16ATileMapsInit(1);
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16B) {
		if (System16Map68KDo) {
			System16Map68KDo();
		} else {
			SekInit(0, 0x68000);
			SekOpen(0);
			SekMapMemory(System16Rom           , 0x000000, 0x0fffff, SM_READ);
			SekMapMemory(System16Code          , 0x000000, 0x0fffff, SM_FETCH);
			SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
			SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
			SekMapMemory(System16SpriteRam     , 0x440000, 0x4407ff, SM_RAM);
			SekMapMemory(System16PaletteRam    , 0x840000, 0x840fff, SM_RAM);
			SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
		
			SekSetReadByteHandler(0, System16BReadByte);
			SekSetWriteByteHandler(0, System16BWriteByte);
			SekSetWriteWordHandler(0, System16BWriteWord);
			SekClose();
		}
		
		if (System16Z80RomNum || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_ISGSM)) {
			if (System16MapZ80Do) {
				ZetInit(0);
				ZetOpen(0);
				System16MapZ80Do();
				ZetClose();
			} else {
				ZetInit(0);
				ZetOpen(0);
				if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_MC8123_ENC) {
					ZetMapArea(0x0000, 0xdfff, 0, System16Z80Rom);
					ZetMapArea(0x0000, 0xdfff, 2, System16Z80Code, System16Z80Rom);
				} else {
					ZetMapArea(0x0000, 0xdfff, 0, System16Z80Rom);
					ZetMapArea(0x0000, 0xdfff, 2, System16Z80Rom);
				}			
	
				ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram);
				ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram);
				ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram);
				ZetMemEnd();
		
#if 0 && defined FBA_DEBUG
				ZetSetReadHandler(System16Z80Read);
				ZetSetWriteHandler(System16Z80Write);
#endif	
				ZetSetInHandler(System16Z80PortRead);
				ZetSetOutHandler(System16Z80PortWrite);
				ZetClose();
			}
		}
		
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2413) {
			BurnYM2413Init(5000000);
			BurnYM2413SetAllRoutes(1.00, BURN_SND_ROUTE_BOTH);
		} else {
			BurnYM2151Init(4000000);
			BurnYM2151SetAllRoutes(0.43, BURN_SND_ROUTE_BOTH);
		}
		
		if (System16UPD7759DataSize) {
			UPD7759Init(0,UPD7759_STANDARD_CLOCK, NULL);
			UPD7759SetDrqCallback(0,System16UPD7759DrqCallback);
			UPD7759SetRoute(0, 0.48, BURN_SND_ROUTE_BOTH);
		}
		
		System16TileBankSize = 0x1000;
		System16CreateOpaqueTileMaps = 1;
		System16BTileMapsInit(1);
		System16ClockSpeed = 10000000;
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18) {
		if (System16Map68KDo) {
			System16Map68KDo();
		} else {
			SekInit(0, 0x68000);
			SekOpen(0);
			SekMapMemory(System16Rom           , 0x000000, 0x0fffff, SM_READ);
			SekMapMemory(System16Code          , 0x000000, 0x0fffff, SM_FETCH);
			SekMapMemory(System16TileRam       , 0x400000, 0x40ffff, SM_READ);
			SekMapMemory(System16TextRam       , 0x410000, 0x410fff, SM_RAM);
			SekMapMemory(System16SpriteRam     , 0x440000, 0x4407ff, SM_RAM);
			SekMapMemory(System16PaletteRam    , 0x840000, 0x840fff, SM_RAM);
			SekMapMemory(System16Ram           , 0xffc000, 0xffffff, SM_RAM);
			
			SekSetReadWordHandler(0, System18ReadWord);
			SekSetWriteWordHandler(0, System18WriteWord);
			SekSetReadByteHandler(0, System18ReadByte);
			SekSetWriteByteHandler(0, System18WriteByte);
			SekClose();
		}
		
		if (System16MapZ80Do) {
			ZetInit(0);
			ZetOpen(0);
			System16MapZ80Do();
			ZetClose();
		} else {
			ZetInit(0);
			ZetOpen(0);
			ZetMapArea(0x0000, 0x9fff, 0, System16Z80Rom);
			ZetMapArea(0x0000, 0x9fff, 2, System16Z80Rom);
			
			ZetMapArea(0xa000, 0xbfff, 0, System16Z80Rom + 0xa000);
			ZetMapArea(0xa000, 0xbfff, 2, System16Z80Rom + 0xa000);
			
			ZetMapArea(0xe000, 0xffff, 0, System16Z80Ram);
			ZetMapArea(0xe000, 0xffff, 1, System16Z80Ram);
			ZetMapArea(0xe000, 0xffff, 2, System16Z80Ram);
			ZetMemEnd();
		
			ZetSetReadHandler(System18Z80Read);
			ZetSetWriteHandler(System18Z80Write);
			ZetSetInHandler(System18Z80PortRead);
			ZetSetOutHandler(System18Z80PortWrite);
			ZetClose();
		}
		
		BurnYM3438Init(2, 8000000, NULL, System18SynchroniseStream, System18GetTime, 1);
		BurnTimerAttachZet(8000000);
		BurnYM3438SetAllRoutes(0, 0.40, BURN_SND_ROUTE_BOTH);
		BurnYM3438SetAllRoutes(1, 0.40, BURN_SND_ROUTE_BOTH);
		
		RF5C68PCMInit(10000000);
		RF5C68PCMSetAllRoutes(1.00, BURN_SND_ROUTE_BOTH);
		
		System16TileBankSize = 0x400;
		System16CreateOpaqueTileMaps = 1;
		System16BTileMapsInit(1);
		
		StartGenesisVDP(0, System16Palette);
		GenesisPaletteBase = 0x1800;
		GenesisBgPalLookup[0] = GenesisSpPalLookup[0] = 0x1800;
		GenesisBgPalLookup[1] = GenesisSpPalLookup[1] = 0x1810;
		GenesisBgPalLookup[2] = GenesisSpPalLookup[2] = 0x1820;
		GenesisBgPalLookup[3] = GenesisSpPalLookup[3] = 0x1830;
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON) {
		if (System16Map68KDo) {
			System16Map68KDo();
		} else {
			SekInit(0, 0x68000);
			SekOpen(0);
			SekMapMemory(System16Rom             , 0x000000, 0x03ffff, SM_READ);
			SekMapMemory(System16Code            , 0x000000, 0x03ffff, SM_FETCH);
			SekMapMemory(System16Ram             , 0x200000, 0x20ffff, SM_RAM);
			SekMapMemory(System16TileRam         , 0x400000, 0x403fff, SM_READ);
			SekMapMemory(System16TextRam         , 0x410000, 0x410fff, SM_RAM);
			SekMapMemory(System16SpriteRam       , 0x600000, 0x607fff, SM_RAM);
			SekMapMemory(System16PaletteRam      , 0xa00000, 0xa00fff, SM_RAM);
			SekMapMemory(System16Rom2            , 0xc00000, 0xc3ffff, SM_READ);
			SekMapMemory(System16RoadRam         , 0xc68000, 0xc68fff, SM_RAM);
			SekMapMemory(System16ExtraRam        , 0xc7c000, 0xc7ffff, SM_RAM);
			
			SekSetReadWordHandler(0, HangonReadWord);
			SekSetReadByteHandler(0, HangonReadByte);
			SekSetWriteByteHandler(0, HangonWriteByte);
			SekSetWriteWordHandler(0, HangonWriteWord);
			SekClose();
		}
		
		SekInit(1, 0x68000);
		SekOpen(1);
		SekMapMemory(System16Rom2            , 0x000000, 0x03ffff, SM_READ);
		SekMapMemory(System16Rom2            , 0x000000, 0x03ffff, SM_FETCH);
		SekMapMemory(System16RoadRam         , 0xc68000, 0xc68fff, SM_RAM);
		SekMapMemory(System16ExtraRam        , 0xc7c000, 0xc7ffff, SM_RAM);
		SekClose();
		
		if (System16MapZ80Do) {
			ZetInit(0);
			ZetOpen(0);
			System16MapZ80Do();
			ZetClose();
		} else {
			if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2203) {
				ZetInit(0);
				ZetOpen(0);
				ZetMapArea(0x0000, 0x7fff, 0, System16Z80Rom);
				ZetMapArea(0x0000, 0x7fff, 2, System16Z80Rom);
	
				ZetMapArea(0xc000, 0xc7ff, 0, System16Z80Ram);
				ZetMapArea(0xc000, 0xc7ff, 1, System16Z80Ram);
				ZetMapArea(0xc000, 0xc7ff, 2, System16Z80Ram);
				ZetMemEnd();
		
				ZetSetReadHandler(System16Z802203Read);
				ZetSetWriteHandler(System16Z802203Write);
				ZetSetInHandler(System16Z802203PortRead);
				ZetClose();
			} else {
				ZetInit(0);
				ZetOpen(0);
				ZetMapArea(0x0000, 0xdfff, 0, System16Z80Rom);
				ZetMapArea(0x0000, 0xdfff, 2, System16Z80Rom);
	
				ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram);
				ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram);
				ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram);
				ZetMemEnd();
		
				ZetSetReadHandler(System16Z80PCMRead);
				ZetSetWriteHandler(System16Z80PCMWrite);
				ZetSetInHandler(System16PPIZ80PortRead);
				ZetSetOutHandler(System16Z80PortWrite);
				ZetClose();
			}
		}
		
		if (PPI0PortWriteA == NULL) PPI0PortWriteA = HangonPPI0WritePortA;
		if (PPI0PortWriteB == NULL) PPI0PortWriteB = HangonPPI0WritePortB;
		if (PPI0PortWriteC == NULL) PPI0PortWriteC = HangonPPI0WritePortC;
		if (PPI1PortReadC == NULL) PPI1PortReadC = HangonPPI1ReadPortC;
		if (PPI1PortWriteA == NULL) PPI1PortWriteA = HangonPPI1WritePortA;
		ppi8255_init(2);
		
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2203) {
			BurnYM2203Init(1, 4000000, &System16YM2203IRQHandler, System16SynchroniseStream, System16GetTime, 0);
			BurnTimerAttachZet(4000000);
			BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.37, BURN_SND_ROUTE_BOTH);
			BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.13, BURN_SND_ROUTE_BOTH);
			BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.13, BURN_SND_ROUTE_BOTH);
			BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.13, BURN_SND_ROUTE_BOTH);
		} else {
			BurnYM2151Init(4000000);
			BurnYM2151SetIrqHandler(&System16YM2151IRQHandler);
			BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.43, BURN_SND_ROUTE_LEFT);
			BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.43, BURN_SND_ROUTE_RIGHT);
		}
		
		if (System16PCMDataSize) {
			if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2203) {
				SegaPCMInit(0, 8000000, BANK_512, System16PCMData, System16PCMDataSize);
			} else {
				SegaPCMInit(0, 4000000, BANK_512, System16PCMData, System16PCMDataSize);
			}
			SegaPCMSetRoute(0, BURN_SND_SEGAPCM_ROUTE_1, 1.0, BURN_SND_ROUTE_LEFT);
			SegaPCMSetRoute(0, BURN_SND_SEGAPCM_ROUTE_2, 1.0, BURN_SND_ROUTE_RIGHT);
		}
		
		System16TileBankSize = 0x1000;
		System16ATileMapsInit(0);
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN) {
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(System16Rom           , 0x000000, 0x05ffff, SM_READ);
		SekMapMemory(System16Code          , 0x000000, 0x05ffff, SM_FETCH);
		SekMapMemory(System16ExtraRam      , 0x060000, 0x067fff, SM_RAM);
		SekMapMemory(System16TileRam       , 0x100000, 0x10ffff, SM_READ);
		SekMapMemory(System16TextRam       , 0x110000, 0x110fff, SM_RAM);
		SekMapMemory(System16PaletteRam    , 0x120000, 0x121fff, SM_RAM);
		SekMapMemory(System16SpriteRam     , 0x130000, 0x130fff, SM_RAM);
		SekMapMemory(System16Rom2          , 0x200000, 0x23ffff, SM_READ);
		SekMapMemory(System16Ram           , 0x260000, 0x267fff, SM_RAM);
		SekMapMemory(System16RoadRam       , 0x280000, 0x280fff, SM_RAM);
		SekSetResetCallback(OutrunResetCallback);
		SekSetReadWordHandler(0, OutrunReadWord);
		SekSetWriteWordHandler(0, OutrunWriteWord);
		SekSetReadByteHandler(0, OutrunReadByte);
		SekSetWriteByteHandler(0, OutrunWriteByte);
		SekClose();
		
		SekInit(1, 0x68000);
		SekOpen(1);
		SekMapMemory(System16Rom2          , 0x000000, 0x03ffff, SM_READ);
		SekMapMemory(System16Rom2          , 0x000000, 0x03ffff, SM_FETCH);
		SekMapMemory(System16Ram           , 0x060000, 0x067fff, SM_RAM);
		SekMapMemory(System16RoadRam       , 0x080000, 0x080fff, SM_RAM);
		SekSetWriteWordHandler(0, Outrun2WriteWord);
		SekSetReadByteHandler(0, Outrun2ReadByte);
		SekSetWriteByteHandler(0, Outrun2WriteByte);
		SekClose();
		
		if (System16MapZ80Do) {
			ZetInit(0);
			ZetOpen(0);
			System16MapZ80Do();
			ZetClose();
		} else {
			ZetInit(0);
			ZetOpen(0);
			ZetMapArea(0x0000, 0xdfff, 0, System16Z80Rom);
			ZetMapArea(0x0000, 0xdfff, 2, System16Z80Rom);
	
			ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram);
			ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram);
			ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram);
			ZetMemEnd();
		
			ZetSetReadHandler(System16Z80PCMRead);
			ZetSetWriteHandler(System16Z80PCMWrite);
			ZetSetInHandler(System16PPIZ80PortRead);
			ZetSetOutHandler(System16Z80PortWrite);
			ZetClose();
		}
		
		if (PPI0PortWriteC == NULL) PPI0PortWriteC = OutrunPPI0WritePortC;
		ppi8255_init(1);
		
		BurnYM2151Init(4000000);
		BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.43, BURN_SND_ROUTE_LEFT);
		BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.43, BURN_SND_ROUTE_RIGHT);
		
		if (System16PCMDataSize) {
			SegaPCMInit(0, 4000000, BANK_512, System16PCMData, System16PCMDataSize);
			SegaPCMSetRoute(0, BURN_SND_SEGAPCM_ROUTE_1, 1.0, BURN_SND_ROUTE_LEFT);
			SegaPCMSetRoute(0, BURN_SND_SEGAPCM_ROUTE_2, 1.0, BURN_SND_ROUTE_RIGHT);
		}
		
		System16RoadColorOffset1 = 0x400;
		System16RoadColorOffset2 = 0x420;
		System16RoadColorOffset3 = 0x780;	
		
		System16TileBankSize = 0x1000;
		System16BTileMapsInit(0);
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMX) {
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(System16Rom           , 0x000000, 0x07ffff, SM_READ);
		SekMapMemory(System16Code          , 0x000000, 0x07ffff, SM_FETCH);
		SekMapMemory(System16TileRam       , 0x0c0000, 0x0cffff, SM_READ);
		SekMapMemory(System16TextRam       , 0x0d0000, 0x0d0fff, SM_RAM);
		SekMapMemory(System16SpriteRam     , 0x100000, 0x100fff, SM_RAM);
		SekMapMemory(System16SpriteRam     , 0x101000, 0x101fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x102000, 0x102fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x103000, 0x103fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x104000, 0x104fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x105000, 0x105fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x106000, 0x106fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x107000, 0x107fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x108000, 0x108fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x109000, 0x109fff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x10a000, 0x10afff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x10b000, 0x10bfff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x10c000, 0x10cfff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x10d000, 0x10dfff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x10e000, 0x10efff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16SpriteRam     , 0x10f000, 0x10ffff, SM_RAM); // Tests past Sprite RAM in mem tests (mirror?)
		SekMapMemory(System16PaletteRam    , 0x120000, 0x123fff, SM_RAM);
		SekMapMemory(System16Rom2          , 0x200000, 0x27ffff, SM_READ);
		SekMapMemory(System16Ram           , 0x29c000, 0x2a3fff, SM_RAM);
		SekMapMemory(System16RoadRam       , 0x2ec000, 0x2ecfff, SM_RAM);
		SekMapMemory(System16RoadRam       , 0x2ed000, 0x2edfff, SM_RAM); // Tests past Road RAM in mem tests (mirror?)
		SekMapMemory(System16BackupRam2    , 0xff4000, 0xff7fff, SM_RAM);
		SekMapMemory(System16BackupRam     , 0xff8000, 0xffffff, SM_RAM);
		SekMapMemory(System16BackupRam2    , 0xffc000, 0xffffff, SM_RAM);
		SekSetResetCallback(OutrunResetCallback);
		SekSetReadWordHandler(0, XBoardReadWord);
		SekSetWriteWordHandler(0, XBoardWriteWord);
		SekSetReadByteHandler(0, XBoardReadByte);
		SekSetWriteByteHandler(0, XBoardWriteByte);
		SekClose();
		
		SekInit(1, 0x68000);
		SekOpen(1);
		SekMapMemory(System16Rom2          , 0x000000, 0x07ffff, SM_ROM);
		SekMapMemory(System16Ram           , 0x09c000, 0x0a3fff, SM_RAM);
		SekMapMemory(System16RoadRam       , 0x0ec000, 0x0ecfff, SM_RAM);
		SekMapMemory(System16Rom2          , 0x200000, 0x27ffff, SM_ROM);
		SekMapMemory(System16Ram           , 0x29c000, 0x2a3fff, SM_RAM);
		SekMapMemory(System16RoadRam       , 0x2ec000, 0x2ecfff, SM_RAM);
		SekSetReadWordHandler(0, XBoard2ReadWord);
		SekSetWriteWordHandler(0, XBoard2WriteWord);
		SekSetReadByteHandler(0, XBoard2ReadByte);
		SekSetWriteByteHandler(0, XBoard2WriteByte);
		SekClose();
		
		if (System16MapZ80Do) {
			ZetInit(0);
			ZetOpen(0);
			System16MapZ80Do();
			ZetClose();
		} else {
			ZetInit(0);
			ZetOpen(0);
			ZetMapArea(0x0000, 0xdfff, 0, System16Z80Rom);
			ZetMapArea(0x0000, 0xdfff, 2, System16Z80Rom);
	
			ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram);
			ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram);
			ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram);
			ZetMemEnd();
		
			ZetSetReadHandler(System16Z80PCMRead);
			ZetSetWriteHandler(System16Z80PCMWrite);
			ZetSetInHandler(SystemXZ80PortRead);
			ZetSetOutHandler(System16Z80PortWrite);
			ZetClose();
		}
		
		if (System16Z80Rom2Num) {
			ZetInit(1);
			ZetOpen(1);
			ZetMapArea(0x0000, 0xefff, 0, System16Z80Rom2);
			ZetMapArea(0x0000, 0xefff, 2, System16Z80Rom2);
	
			ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram2);
			ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram2);
			ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram2);
			ZetMemEnd();
		
			ZetSetReadHandler(System16Z80Read2);
			ZetSetWriteHandler(System16Z80Write2);
			ZetSetInHandler(System16Z80PortRead2);
			ZetClose();
		}
		
		BurnYM2151Init(4000000);
		BurnYM2151SetIrqHandler(&System16YM2151IRQHandler);
		BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.43, BURN_SND_ROUTE_LEFT);
		BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.43, BURN_SND_ROUTE_RIGHT);
		
		if (System16PCMDataSize) {
			SegaPCMInit(0, 4000000, BANK_512, System16PCMData, System16PCMDataSize);
			SegaPCMSetRoute(0, BURN_SND_SEGAPCM_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
			SegaPCMSetRoute(0, BURN_SND_SEGAPCM_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);
		}		
		
		if (System16PCM2DataSize) {
			SegaPCMInit(1, 4000000, BANK_512, System16PCM2Data, System16PCM2DataSize);
			SegaPCMSetRoute(1, BURN_SND_SEGAPCM_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
			SegaPCMSetRoute(1, BURN_SND_SEGAPCM_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);
		}		
		
		System16RoadColorOffset1 = 0x1700;
		System16RoadColorOffset2 = 0x1720;
		System16RoadColorOffset3 = 0x1780;
		System16RoadXOffset = -166;
		System16RoadPriority = 1;
		System16TilemapColorOffset = 0x1c00;
		
		System16TileBankSize = 0x1000;
		System16BTileMapsInit(0);
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMY) {
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(System16Rom           , 0x000000, 0x07ffff, SM_READ);
		SekMapMemory(System16Code          , 0x000000, 0x07ffff, SM_FETCH);
		SekMapMemory(System16Ram           , 0x0c0000, 0x0cffff, SM_RAM);
		SekMapMemory(System16ExtraRam      , 0xff0000, 0xffffff, SM_RAM);
		SekSetReadWordHandler(0, YBoardReadWord);
		SekSetWriteWordHandler(0, YBoardWriteWord);
		SekSetReadByteHandler(0, YBoardReadByte);
		SekSetWriteByteHandler(0, YBoardWriteByte);
		SekClose();
		
		SekInit(1, 0x68000);
		SekOpen(1);
		SekMapMemory(System16Rom2          , 0x000000, 0x03ffff, SM_ROM);
		SekMapMemory(System16Ram           , 0x0c0000, 0x0cffff, SM_RAM);
		SekMapMemory(System16SpriteRam2    , 0x180000, 0x18ffff, SM_RAM);
		SekMapMemory(System16ExtraRam2     , 0xff0000, 0xffbfff, SM_RAM);
		SekMapMemory(System16BackupRam     , 0xffc000, 0xffffff, SM_RAM);
		SekSetReadWordHandler(0, YBoard2ReadWord);
		SekSetWriteWordHandler(0, YBoard2WriteWord);
		SekClose();
		
		SekInit(2, 0x68000);
		SekOpen(2);
		SekMapMemory(System16Rom3          , 0x000000, 0x03ffff, SM_ROM);
		SekMapMemory(System16Ram           , 0x0c0000, 0x0cffff, SM_RAM);
		SekMapMemory(System16RotateRam     , 0x180000, 0x1807ff, SM_RAM);
		SekMapMemory(System16SpriteRam     , 0x188000, 0x188fff, SM_RAM);
		SekMapMemory(System16PaletteRam    , 0x190000, 0x193fff, SM_RAM);
		SekMapMemory(System16PaletteRam    , 0x194000, 0x197fff, SM_RAM);
		SekMapMemory(System16ExtraRam3     , 0xff0000, 0xffffff, SM_RAM);
		SekSetReadWordHandler(0, YBoard3ReadWord);
		SekSetWriteWordHandler(0, YBoard3WriteWord);
		SekSetReadByteHandler(0, YBoard3ReadByte);
		SekClose();
		
		if (System16MapZ80Do) {
			ZetInit(0);
			ZetOpen(0);
			System16MapZ80Do();
			ZetClose();
		} else {
			ZetInit(0);
			ZetOpen(0);
			ZetMapArea(0x0000, 0xdfff, 0, System16Z80Rom);
			ZetMapArea(0x0000, 0xdfff, 2, System16Z80Rom);
	
			ZetMapArea(0xf800, 0xffff, 0, System16Z80Ram);
			ZetMapArea(0xf800, 0xffff, 1, System16Z80Ram);
			ZetMapArea(0xf800, 0xffff, 2, System16Z80Ram);
			ZetMemEnd();
		
			ZetSetReadHandler(System16Z80PCMRead);
			ZetSetWriteHandler(System16Z80PCMWrite);
			ZetSetInHandler(SystemXZ80PortRead);
			ZetSetOutHandler(System16Z80PortWrite);
			ZetClose();
		}
		
		BurnYM2151Init(32215900 / 8);
		BurnYM2151SetIrqHandler(&System16YM2151IRQHandler);
		BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.43, BURN_SND_ROUTE_LEFT);
		BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.43, BURN_SND_ROUTE_RIGHT);
		
		if (System16PCMDataSize) {
			SegaPCMInit(0, 32215900 / 8, BANK_12M | BANK_MASKF8, System16PCMData, System16PCMDataSize);
			SegaPCMSetRoute(0, BURN_SND_SEGAPCM_ROUTE_1, 1.0, BURN_SND_ROUTE_LEFT);
			SegaPCMSetRoute(0, BURN_SND_SEGAPCM_ROUTE_2, 1.0, BURN_SND_ROUTE_RIGHT);
		}
	}
	
	GenericTilesInit();
	bSystem16BootlegRender = false;
	
	// Reset the driver
	System16DoReset();
	
	return 0;
}

/*====================================================
Exit Function
====================================================*/

INT32 System16Exit()
{
	INT32 i;
	
	SekExit();
	if (System16Z80RomNum || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_ISGSM)) ZetExit();
	if (System167751ProgSize) {
		N7751Exit();
		DACExit();
	}
	
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18) {
		BurnYM3438Exit();
		RF5C68PCMExit();
	} else {
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2203) {
			BurnYM2203Exit();
		} else {
			if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2413) {
				BurnYM2413Exit();
			} else {
				BurnYM2151Exit();
			}
		}
	}
			
	if (System16PCMDataSize) SegaPCMExit();
	if (System16UPD7759DataSize) UPD7759Exit();
	
	if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16A) || ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON) || ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN)) {
		ppi8255_exit();
	}
	
	if (nBurnGunNumPlayers) BurnGunExit();
	
	GenericTilesExit();
	System16TileMapsExit();

	BurnFree(Mem);
	
	// Reset Variables
	for (i = 0; i < 4; i++) {
		System16Page[i] = 0;
		System16OldPage[i] = 0;
		System16ScrollX[i] = 0;
		System16ScrollY[i] = 0;
		BootlegBgPage[i] = 0;
		BootlegFgPage[i] = 0;
	}
	
	for (i = 0; i < 16; i++) {
		System16SpriteBanks[i] = 0;
	}
	
	for (i = 0; i < 8; i++) {
		System16TileBanks[i] = 0;
		System16OldTileBanks[i] = 0;
	}
	
	System16VideoEnable = 0;
	System18VdpEnable = 0;
	System18VdpMixing = 0;
	System16ScreenFlip = 0;
	System16SpriteShadow = 0;
	System16SpriteXOffset = 0;
	System16VideoControl = 0;
	System16SoundLatch = 0;
	System16ColScroll = 0;
	System16RowScroll = 0;
	System16IgnoreVideoEnable = 0;
	
	System16LastGear = 0;
	System16HasGears = false;

 	System16RoadControl = 0;
 	System16RoadColorOffset1 = 0;
 	System16RoadColorOffset2 = 0;
 	System16RoadColorOffset3 = 0;
 	System16RoadXOffset = 0;
 	System16RoadPriority = 0;
 	System16AnalogSelect = 0;
 	
 	System16ClockSpeed = 0;
 	
 	System16PaletteEntries = 0;
 	System16RamSize = 0;
 	System16ExtraRamSize = 0;
 	System16ExtraRam2Size = 0;
 	System16ExtraRam3Size = 0;
 	System16SpriteRamSize = 0;
 	System16SpriteRam2Size = 0;
 	System16RotateRamSize = 0;
 	System16TilemapColorOffset = 0;
	System16TileBankSize = 0;
	System16RecalcBgTileMap = 0;
	System16RecalcBgAltTileMap = 0;
	System16RecalcFgTileMap = 0;
	System16RecalcFgAltTileMap = 0;
	System16CreateOpaqueTileMaps = 0;
	
	System16BTileAlt = false;
	Shangon = false;
	Hangon = false;
	bSystem16BootlegRender = false;
 	
 	System16YM2413IRQInterval = 0;
 	
 	UPD7759BankAddress = 0;
 	N7751Command = 0;
 	N7751RomAddress = 0;
	
	System16RomSize = 0;
	System16RomNum = 0;
	System16Rom2Size = 0;
	System16Rom2Num = 0;
	System16Rom3Size = 0;
	System16Rom3Num = 0;
	System16TileRomSize = 0;
 	System16TileRomNum = 0;
 	System16NumTiles = 0;
 	System16SpriteRomSize = 0;
 	System16SpriteRomNum = 0;
 	System16Sprite2RomSize = 0;
 	System16Sprite2RomNum = 0;
 	System16RoadRomSize = 0;
 	System16RoadRomNum = 0;
 	System16Z80RomSize = 0;
 	System16Z80RomNum = 0;
 	System16Z80Rom2Size = 0;
 	System16Z80Rom2Num = 0;
 	System16Z80Rom3Size = 0;
 	System16Z80Rom3Num = 0;
 	System16Z80Rom4Size = 0;
 	System16Z80Rom4Num = 0;
 	System167751ProgSize = 0;
 	System167751ProgNum = 0;
 	System167751DataSize = 0;
 	System167751DataNum = 0;
 	System16UPD7759DataSize = 0;
 	System16UPD7759DataNum = 0;
 	System16PCMDataSizePreAllocate = 0;
 	System16PCMDataSize = 0;
 	System16PCMDataNum = 0;
 	System16PCM2DataSize = 0;
 	System16PCM2DataNum = 0;
 	System16RF5C68DataSize = 0;
	System16RF5C68DataNum = 0;
 	System16PromSize = 0;
	System16PromNum = 0;
 	System16KeySize = 0;
 	
 	Simulate8751 = NULL;
 	System16Map68KDo = NULL;
 	System16MapZ80Do = NULL;
 	System16CustomLoadRomDo = NULL;
 	System16CustomDecryptOpCodeDo = NULL;
 	System16ProcessAnalogControlsDo = NULL;
 	System16MakeAnalogInputsDo = NULL;
 	
 	memset(multiply, 0, sizeof(multiply));
 	memset(divide, 0, sizeof(divide));
 	memset(compare_timer, 0, sizeof(compare_timer));
 	
 	if ((BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC_CPU2)) {
		fd1094_exit();
		
		// Switch back CPU core if needed
		if (bUseAsm68KCoreOldValue) {
#if 1 && defined FBA_DEBUG
			bprintf(PRINT_NORMAL, _T("Switching back to A68K core\n"));
#endif
			bUseAsm68KCoreOldValue = false;
			bBurnUseASMCPUEmulation = true;
		}
	}
	
	return 0;
}

/*====================================================
Frame Functions
====================================================*/

INT32 System16AFrame()
{
	INT32 nInterleave = 100; // alien syndrome needs high interleave for the DAC sounds to be processed
	
	if (System16Reset) System16DoReset();

	System16MakeInputs();
	
	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = 4000000 / 60;
	nCyclesTotal[2] = (6000000 / 15) / 60;
	nSystem16CyclesDone[0] = nSystem16CyclesDone[1] = nSystem16CyclesDone[2] = 0;

	INT32 nSoundBufferPos = 0;

	SekNewFrame();
	ZetNewFrame();
	I8039NewFrame();

	SekOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nSystem16CyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);

		// Run Z80
		nCurrentCPU = 1;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();
		
		if (System167751ProgSize) {
			nCurrentCPU = 2;
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
			nCyclesSegment = N7751Run(nCyclesSegment);
			nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		}

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
			ZetClose();
		}
	}

	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekClose();
	
	if (Simulate8751) Simulate8751();

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
		}
		
		if (System167751ProgSize) DACUpdate(pBurnSoundOut, nBurnSoundLen);
	}
	
	if (pBurnDraw) System16ARender();

	return 0;
}

INT32 System16BFrame()
{
	INT32 nInterleave = (nBurnSoundRate <= 44100) ? 183 : 200;	// For the UPD7759
	
	if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2413) nInterleave = System16YM2413IRQInterval;
	
	if (System16Reset) System16DoReset();
	
	System16MakeInputs();
	
	nCyclesTotal[0] = (INT32)((INT64)System16ClockSpeed * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = 5000000 / 60;
	nSystem16CyclesDone[0] = nSystem16CyclesDone[1] = 0;

	INT32 nSoundBufferPos = 0;
	
	SekNewFrame();
	if (System16Z80RomNum || ((BurnDrvGetHardwareCode() & HARDWARE_SEGA_ISGSM) && System16Z80Enable)) ZetNewFrame();
	
	SekOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nSystem16CyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		
		if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2413) {
			SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		}

		// Run Z80
		if (System16Z80RomNum || ((BurnDrvGetHardwareCode() & HARDWARE_SEGA_ISGSM) && System16Z80Enable)) {
			nCurrentCPU = 1;
			ZetOpen(0);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
			ZetClose();
		}

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			
			if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2413) {
				BurnYM2413Render(pSoundBuf, nSegmentLength);
			} else {
				ZetOpen(0);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				if (System16UPD7759DataSize) UPD7759Update(0,pSoundBuf, nSegmentLength);
				ZetClose();
			}
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2413) {
				BurnYM2413Render(pSoundBuf, nSegmentLength);
			} else {
				ZetOpen(0);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				if (System16UPD7759DataSize) UPD7759Update(0,pSoundBuf, nSegmentLength);
				ZetClose();
			}
		}
	}
	
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekClose();
	
	if (Simulate8751) Simulate8751();
	
	if (pBurnDraw) {
		if (System16BTileAlt) {
			System16BAltRender();
		} else {
			if (bSystem16BootlegRender) {
				System16BootlegRender();
			} else {
				System16BRender();
			}
		}
	}

	return 0;
}

INT32 System18Frame()
{
	INT32 nInterleave = nBurnSoundLen;

	if (System16Reset) System16DoReset();

	System16MakeInputs();
	
	if (nBurnGunNumPlayers) System16GunMakeInputs();
	
	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = 8000000 / 60;
	nSystem16CyclesDone[0] = nSystem16CyclesDone[1] = 0;
	
	INT32 nSoundBufferPos = 0;

	SekNewFrame();
	ZetNewFrame();
	
	SekOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nSystem16CyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		
		nCurrentCPU = 1;
		ZetOpen(0);
		BurnTimerUpdate(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		ZetClose();
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			RF5C68PCMUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekClose();
	
	ZetOpen(0);
	BurnTimerEndFrame(nCyclesTotal[1]);
	ZetClose();
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			RF5C68PCMUpdate(pSoundBuf, nSegmentLength);
		}
	}
	
	ZetOpen(0);
	if (pBurnSoundOut) BurnYM3438Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();

	if (pBurnDraw) {
		System18Render();
	}
	
	return 0;
}

INT32 HangonFrame()
{
	INT32 nInterleave = 10, i;

	if (System16Reset) System16DoReset();

	System16MakeInputs();
	
	nCyclesTotal[0] = (INT32)((INT64)System16ClockSpeed * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)System16ClockSpeed * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[2] = 4000000 / 60;
	nSystem16CyclesDone[0] = nSystem16CyclesDone[1] = nSystem16CyclesDone[2] = 0;

	INT32 nSoundBufferPos = 0;
	
	SekNewFrame();
	ZetNewFrame();

	for (i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nSystem16CyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		SekClose();
		
		// Run 68000 #2
		nCurrentCPU = 1;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = SekRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		SekClose();

		// Run Z80
		nCurrentCPU = 2;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			SegaPCMUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			SegaPCMUpdate(pSoundBuf, nSegmentLength);
		}
	}

	SekOpen(0);
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekClose();
	
	if (Simulate8751) Simulate8751();	

	if (pBurnDraw) {
		HangonRender();
	}

	return 0;
}

INT32 HangonYM2203Frame()
{
	INT32 nInterleave = 100, i;

	if (System16Reset) System16DoReset();

	System16MakeInputs();
	
	nCyclesTotal[0] = (INT32)((INT64)System16ClockSpeed * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)System16ClockSpeed * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[2] = 4000000 / 60;
	nSystem16CyclesDone[0] = nSystem16CyclesDone[1] = nSystem16CyclesDone[2] = 0;
	
	SekNewFrame();
	ZetNewFrame();

	for (i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nSystem16CyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		SekClose();
		
		// Run 68000 #2
		nCurrentCPU = 1;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = SekRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		SekClose();
		
		ZetOpen(0);
		BurnTimerUpdate(i * (nCyclesTotal[2] / nInterleave));
		ZetClose();
	}
	
	SekOpen(0);
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekClose();
	
	ZetOpen(0);
	BurnTimerEndFrame(nCyclesTotal[2]);
	ZetClose();
	
	if (pBurnSoundOut) {
		ZetOpen(0);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		SegaPCMUpdate(pBurnSoundOut, nBurnSoundLen);
		ZetClose();
	}
	
	if (Simulate8751) Simulate8751();	

	if (pBurnDraw) {
		if (Hangon) {
			HangonAltRender();
		} else {
			HangonRender();
		}
	}

	return 0;
}

INT32 OutrunFrame()
{
	INT32 nInterleave = 10, i;

	if (System16Reset) System16DoReset();

	if (System16HasGears) {
		OutrunMakeInputs();
	} else {
		System16MakeInputs();
	}
	
	nCyclesTotal[0] = (INT32)((INT64)(40000000 / 4) * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)(40000000 / 4) * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[2] = (16000000 / 4) / 60;
	nSystem16CyclesDone[0] = nSystem16CyclesDone[1] = nSystem16CyclesDone[2] = 0;

	INT32 nSoundBufferPos = 0;
	
	SekNewFrame();
	ZetNewFrame();

	for (i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nSystem16CyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 2 || i == 6 || i == 8) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		// Run 68000 #2
		nCurrentCPU = 1;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = SekRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		SekClose();

		// Run Z80
		nCurrentCPU = 2;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			SegaPCMUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			SegaPCMUpdate(pSoundBuf, nSegmentLength);
		}
	}

	for (i = 0; i < 2; i++) {
		SekOpen(i);
		SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		SekClose();
	}	
	
	if (pBurnDraw) {
		if (!Shangon) {
			OutrunRender();
		} else {
			ShangonRender();
		}
	}

	return 0;
}

INT32 XBoardFrame()
{
	INT32 nInterleave = 100, i;

	if (System16Reset) System16DoReset();

	System16MakeInputs();
	
	if (nBurnGunNumPlayers) System16GunMakeInputs();
	
	nCyclesTotal[0] = (INT32)((INT64)(50000000 / 4) * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)(50000000 / 4) * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[2] = (16000000 / 4) / 60;
	nCyclesTotal[3] = (16000000 / 4) / 60;
	nSystem16CyclesDone[0] = nSystem16CyclesDone[1] = nSystem16CyclesDone[2] = nSystem16CyclesDone[3] = 0;

	INT32 nSoundBufferPos = 0;
	
	SekNewFrame();
	ZetNewFrame();
	
	for (i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nSystem16CyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 20 || i == 40 || i == 60 || i == 80) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		if (i == 99) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		// Run 68000 #2
		nCurrentCPU = 1;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = SekRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == 99) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		SekClose();

		// Run Z80
		nCurrentCPU = 2;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();
		
		// Run Z80 #2
		if (System16Z80Rom2Num) {
			nCurrentCPU = 3;
			ZetOpen(1);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
			ZetClose();
		}

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			if (System16PCMDataSize) SegaPCMUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			if (System16PCMDataSize) SegaPCMUpdate(pSoundBuf, nSegmentLength);
		}
	}

	if (pBurnDraw) {
		XBoardRender();
	}

	return 0;
}

INT32 YBoardFrame()
{
	INT32 nInterleave = 262, i;

	if (System16Reset) System16DoReset();
	
	if (System16HasGears) {
		PdriftMakeInputs();
	} else {
		System16MakeInputs();
	}
	
	if (nBurnGunNumPlayers) System16GunMakeInputs();

	nCyclesTotal[0] = (INT32)((INT64)(50000000 / 4) * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)(50000000 / 4) * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[2] = (INT32)((INT64)(50000000 / 4) * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[3] = (32215900 / 8) / 60;
	nSystem16CyclesDone[0] = nSystem16CyclesDone[1] = nSystem16CyclesDone[2] = nSystem16CyclesDone[3] = 0;

	INT32 nSoundBufferPos = 0;
	
	SekNewFrame();
	ZetNewFrame();

	for (i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;
		
		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nSystem16CyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 170) SekSetIRQLine(2, SEK_IRQSTATUS_ACK);
		if (i == 171) SekSetIRQLine(2, SEK_IRQSTATUS_NONE);
		if (i == 223) SekSetIRQLine(4, SEK_IRQSTATUS_ACK);
		if (i == 224) SekSetIRQLine(4, SEK_IRQSTATUS_NONE);
		SekClose();
		
		// Run 68000 #2
		nCurrentCPU = 1;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = SekRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == 170) SekSetIRQLine(2, SEK_IRQSTATUS_ACK);
		if (i == 171) SekSetIRQLine(2, SEK_IRQSTATUS_NONE);
		if (i == 223) SekSetIRQLine(4, SEK_IRQSTATUS_ACK);
		if (i == 224) SekSetIRQLine(4, SEK_IRQSTATUS_NONE);
		SekClose();
		
		// Run 68000 #3
		nCurrentCPU = 2;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = SekRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == 170) SekSetIRQLine(2, SEK_IRQSTATUS_ACK);
		if (i == 171) SekSetIRQLine(2, SEK_IRQSTATUS_NONE);
		if (i == 223) SekSetIRQLine(4, SEK_IRQSTATUS_ACK);
		if (i == 224) SekSetIRQLine(4, SEK_IRQSTATUS_NONE);
		SekClose();

		// Run Z80
		nCurrentCPU = 3;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nSystem16CyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nSystem16CyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			SegaPCMUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			SegaPCMUpdate(pSoundBuf, nSegmentLength);
		}
	}

	if (pBurnDraw) {
		YBoardRender();
	}

	return 0;
}

/*====================================================
Driver Scan
====================================================*/

INT32 System16Scan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {
		*pnMin =  0x029719;
	}

	if (nAction & ACB_NVRAM) {
		if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMX || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMY) {
			if (System16BackupRamSize) {
				memset(&ba, 0, sizeof(ba));
				ba.Data = System16BackupRam;
				ba.nLen = System16BackupRamSize;
				ba.szName = "Backup Ram 1";
				BurnAcb(&ba);
			}
		
			if (System16BackupRam2Size) {
				memset(&ba, 0, sizeof(ba));
				ba.Data = System16BackupRam2;
				ba.nLen = System16BackupRam2Size;
				ba.szName = "Backup Ram 2";
				BurnAcb(&ba);
			}
		}
		
		if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16B || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16A || (BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18) {
			memset(&ba, 0, sizeof(ba));
			ba.Data = System16Ram;
			ba.nLen = System16RamSize;
			ba.szName = "Work Ram";
			BurnAcb(&ba);
		}
	}
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
    	ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		if (System16Z80RomNum || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_ISGSM)) {
			ZetScan(nAction);
		}
		
		if ((BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC) || (BurnDrvGetHardwareCode() & HARDWARE_SEGA_FD1094_ENC_CPU2)) {
			fd1094_scan(nAction);
		}

		if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16A) || ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON) || ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN)) {
			ppi8255_scan();
		}
		
		if (nBurnGunNumPlayers) BurnGunScan();
		
		SCAN_VAR(System16SoundLatch);
		SCAN_VAR(System16Input);
		SCAN_VAR(System16Dip);
		SCAN_VAR(System16VideoEnable);
		SCAN_VAR(System16ScreenFlip);
		SCAN_VAR(System16ScrollX);
		SCAN_VAR(System16ScrollY);
		SCAN_VAR(System16ColScroll);
		SCAN_VAR(System16RowScroll);
		SCAN_VAR(System16SpriteBanks);
		SCAN_VAR(System16TileBanks);
		SCAN_VAR(System16Page);
		SCAN_VAR(BootlegBgPage);
		SCAN_VAR(BootlegFgPage);
		SCAN_VAR(System16AnalogSelect);
		SCAN_VAR(System16LastGear);
		SCAN_VAR(nSystem16CyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(System16RoadControl);
		SCAN_VAR(multiply);
		SCAN_VAR(divide);
		SCAN_VAR(compare_timer);
		SCAN_VAR(N7751Command);
		SCAN_VAR(N7751RomAddress);
		SCAN_VAR(UPD7759BankAddress);
		SCAN_VAR(System18VdpMixing);
		SCAN_VAR(System18VdpEnable);
		SCAN_VAR(RF5C68PCMBankAddress);
		SCAN_VAR(System16Z80Enable);
		
		if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18) {
			BurnYM3438Scan(nAction, pnMin);
			RF5C68PCMScan(nAction);
			
			if (nAction & ACB_WRITE) {
				ZetOpen(0);
				ZetMapArea(0xa000, 0xbfff, 0, System16Z80Rom + 0x10000 + RF5C68PCMBankAddress);
				ZetMapArea(0xa000, 0xbfff, 2, System16Z80Rom + 0x10000 + RF5C68PCMBankAddress);
				ZetClose();
			}
		} else {
			if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2203) {
				BurnYM2203Scan(nAction, pnMin);
			} else {
				if (BurnDrvGetHardwareCode() & HARDWARE_SEGA_YM2413) {
					BurnYM2413Scan(nAction);
				} else {
					BurnYM2151Scan(nAction);
				}
			}
		}
		
		if (System16UPD7759DataSize) {
			UPD7759Scan(0,nAction, pnMin);
			
			if (nAction & ACB_WRITE) {
				ZetOpen(0);
				ZetMapArea(0x8000, 0xdfff, 0, System16UPD7759Data + UPD7759BankAddress);
				ZetMapArea(0x8000, 0xdfff, 2, System16UPD7759Data + UPD7759BankAddress);
				ZetClose();
			}
		}
		
		if (System167751ProgSize) {
			N7751Scan(nAction, pnMin);
			DACScan(nAction, pnMin);
		}
		
		if (System16PCMDataSize) {
			SegaPCMScan(nAction, pnMin);
		}
		
		System16GfxScan(nAction);
	}

	return 0;
}
