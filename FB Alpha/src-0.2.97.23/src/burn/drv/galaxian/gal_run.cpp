#include "gal.h"

UINT8 GalInputPort0[8]       = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8 GalInputPort1[8]       = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8 GalInputPort2[8]       = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8 GalInputPort3[8]       = {0, 0, 0, 0, 0, 0, 0, 0};
UINT8 GalDip[7]              = {0, 0, 0, 0, 0, 0, 0};
UINT8 GalInput[4]            = {0x00, 0x00, 0x00, 0x00};
UINT8 GalReset               = 0;
UINT8 GalFakeDip             = 0;
INT32 GalAnalogPort0         = 0;
INT32 GalAnalogPort1         = 0;

UINT8 *GalMem                = NULL;
UINT8 *GalMemEnd             = NULL;
UINT8 *GalRamStart           = NULL;
UINT8 *GalRamEnd             = NULL;
UINT8 *GalZ80Rom1            = NULL;
UINT8 *GalZ80Rom1Op          = NULL;
UINT8 *GalZ80Rom2            = NULL;
UINT8 *GalZ80Rom3            = NULL;
UINT8 *GalS2650Rom1          = NULL;
UINT8 *GalZ80Ram1            = NULL;
UINT8 *GalZ80Ram2            = NULL;
UINT8 *GalZ80Ram3            = NULL;
UINT8 *GalVideoRam           = NULL;
UINT8 *GalVideoRam2          = NULL;
UINT8 *GalSpriteRam          = NULL;
UINT8 *GalScrollVals         = NULL;
UINT8 *GalProm               = NULL;
UINT8 *GalChars              = NULL;
UINT8 *GalSprites            = NULL;
UINT8 *GalTempRom            = NULL;
UINT32  *GalPalette            = NULL;

UINT32   GalZ80Rom1Size        = 0;
UINT32   GalZ80Rom1Num         = 0;
UINT32   GalZ80Rom2Size        = 0;
UINT32   GalZ80Rom2Num         = 0;
UINT32   GalZ80Rom3Size        = 0;
UINT32   GalZ80Rom3Num         = 0;
UINT32   GalS2650Rom1Size      = 0;
UINT32   GalS2650Rom1Num       = 0;
UINT32   GalTilesSharedRomSize = 0;
UINT32   GalTilesSharedRomNum  = 0;
UINT32   GalTilesCharRomSize   = 0;
UINT32   GalTilesCharRomNum    = 0;
UINT32   GalNumChars           = 0;
UINT32   GalTilesSpriteRomSize = 0;
UINT32   GalTilesSpriteRomNum  = 0;
UINT32   GalNumSprites         = 0;
UINT32   GalPromRomSize        = 0;
UINT32   GalPromRomNum         = 0;

GalPostLoadCallback GalPostLoadCallbackFunction;

// CPU Control
UINT8 GalIrqType;
UINT8 GalIrqFire;
INT32 nGalCyclesDone[3], nGalCyclesTotal[3];
static INT32 nGalCyclesSegment;

// Misc variables
UINT8 ZigzagAYLatch;
UINT8 GalSoundLatch;
UINT8 GalSoundLatch2;
UINT8 KingballSound;
UINT8 KingballSpeechDip;
UINT8 KonamiSoundControl;
UINT8 SfxSampleControl;
UINT16 ScrambleProtectionState;
UINT8 ScrambleProtectionResult;
UINT8 MoonwarPortSelect;
UINT8 MshuttleAY8910CS;
UINT8 GmgalaxSelectedGame;
UINT8 Fourin1Bank;
UINT8 GameIsGmgalax;
UINT8 CavelonBankSwitch;
UINT8 GalVBlank;

static inline void GalMakeInputs()
{
	// Reset Inputs
	GalInput[0] = GalInput[1] = GalInput[2] = GalInput[3] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		GalInput[0] |= (GalInputPort0[i] & 1) << i;
		GalInput[1] |= (GalInputPort1[i] & 1) << i;
		GalInput[2] |= (GalInputPort2[i] & 1) << i;
		GalInput[3] |= (GalInputPort3[i] & 1) << i;
	}
}

static INT32 GalMemIndex()
{
	UINT8 *Next; Next = GalMem;
	
	GalZ80Rom1             = Next; Next += GalZ80Rom1Size;
	GalZ80Rom2             = Next; Next += GalZ80Rom2Size;
	GalZ80Rom3             = Next; Next += GalZ80Rom3Size;
	GalS2650Rom1           = Next; Next += GalS2650Rom1Size;
	GalProm                = Next; Next += GalPromRomSize;

	GalRamStart            = Next;

	GalZ80Ram1             = Next; Next += 0x01000;
	GalVideoRam            = Next; Next += 0x00400;
	GalSpriteRam           = Next; Next += 0x00100;
	GalScrollVals          = Next; Next += 0x00020;
	GalGfxBank             = Next; Next += 0x0001f;
	
	if (GalZ80Rom2Size) {
		GalZ80Ram2     = Next; Next += 0x00400;
	}
	
	if (GalZ80Rom3Size) {
		GalZ80Ram3     = Next; Next += 0x00400;
	}
	
	GalRamEnd              = Next;

	GalChars               = Next; Next += GalNumChars * 8 * 8;
	GalSprites             = Next; Next += GalNumSprites * 16 * 16;
	GalPalette             = (UINT32*)Next; Next += (GAL_PALETTE_NUM_COLOURS_PROM + GAL_PALETTE_NUM_COLOURS_STARS + GAL_PALETTE_NUM_COLOURS_BULLETS + GAL_PALETTE_NUM_COLOURS_BACKGROUND) * sizeof(UINT32);
	
	GalMemEnd              = Next;

	return 0;
}

static INT32 GalDoReset()
{
	if (GalS2650Rom1Size) {
		s2650Open(0);
		s2650Reset();
		s2650Close();
	}
	
	if (GalZ80Rom1Size) {
		ZetOpen(0);
		ZetReset();
		ZetClose();
	}

	if (GalZ80Rom2Size) {
		ZetOpen(1);
		ZetReset();
		ZetClose();
	}
		
	if (GalZ80Rom3Size) {
		ZetOpen(2);
		ZetReset();
		ZetClose();
	}
	
	GalSoundReset();
	HiscoreReset();
	
	GalIrqFire = 0;
	GalFlipScreenX = 0;
	GalFlipScreenY = 0;
	ZigzagAYLatch = 0;
	GalSoundLatch = 0;
	GalSoundLatch2 = 0;
	KingballSpeechDip = 0;
	KingballSound = 0;
	GalStarsEnable = 0;
	GalStarsScrollPos = 0;
	GalBackgroundRed = 0;
	GalBackgroundGreen = 0;
	GalBackgroundBlue = 0;
	GalBackgroundEnable = 0;
	ScrambleProtectionState = 0;
	ScrambleProtectionResult = 0;
	MoonwarPortSelect = 0;
	MshuttleAY8910CS = 0;
	Fourin1Bank = 0;
	CavelonBankSwitch = 0;
	GalVBlank = 0;

	return 0;
}

static INT32 GalLoadRoms(bool bLoad)
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
			if ((ri.nType & 0xff) == GAL_ROM_Z80_PROG1) {
				GalZ80Rom1Size += ri.nLen;
				GalZ80Rom1Num++;
			}
			if ((ri.nType & 0xff) == GAL_ROM_Z80_PROG2) {
				GalZ80Rom2Size += ri.nLen;
				GalZ80Rom2Num++;
			}
			if ((ri.nType & 0xff) == GAL_ROM_Z80_PROG3) {
				GalZ80Rom3Size += ri.nLen;
				GalZ80Rom3Num++;
			}
			if ((ri.nType & 0xff) == GAL_ROM_TILES_SHARED) {
				GalTilesSharedRomSize += ri.nLen;
				GalTilesSharedRomNum++;
			}
			if ((ri.nType & 0xff) == GAL_ROM_TILES_CHARS) {
				GalTilesCharRomSize += ri.nLen;
				GalTilesCharRomNum++;
			}
			if ((ri.nType & 0xff) == GAL_ROM_TILES_SPRITES) {
				GalTilesSpriteRomSize += ri.nLen;
				GalTilesSpriteRomNum++;
			}
			if ((ri.nType & 0xff) == GAL_ROM_PROM) {
				GalPromRomSize += ri.nLen;
				GalPromRomNum++;
			}
			if ((ri.nType & 0xff) == GAL_ROM_S2650_PROG1) {
				GalS2650Rom1Size += ri.nLen;
				GalS2650Rom1Num++;
			}
		} while (ri.nLen);
		
		if (GalTilesSharedRomSize) {
			GalNumChars = GalTilesSharedRomSize / 16;
			GalNumSprites = GalTilesSharedRomSize / 64;
			CharPlaneOffsets[1] = GalTilesSharedRomSize * 4;
			SpritePlaneOffsets[1] = GalTilesSharedRomSize * 4;
		}
		
		if (GalTilesCharRomSize) {
			GalNumChars = GalTilesCharRomSize / 16;
			CharPlaneOffsets[1] = GalTilesCharRomSize * 4;
		}
		
		if (GalTilesSpriteRomSize) {
			GalNumSprites = GalTilesSpriteRomSize / 64;
			SpritePlaneOffsets[1] = GalTilesSpriteRomSize * 4;
		}
		
#if 1 && defined FBA_DEBUG	
		if (GalZ80Rom1Size) bprintf(PRINT_NORMAL, _T("Z80 #1 Rom Size: 0x%X (%i roms)\n"), GalZ80Rom1Size, GalZ80Rom1Num);
		if (GalZ80Rom2Size) bprintf(PRINT_NORMAL, _T("Z80 #2 Rom Size: 0x%X (%i roms)\n"), GalZ80Rom2Size, GalZ80Rom2Num);
		if (GalZ80Rom3Size) bprintf(PRINT_NORMAL, _T("Z80 #3 Rom Size: 0x%X (%i roms)\n"), GalZ80Rom3Size, GalZ80Rom3Num);
		if (GalS2650Rom1Size) bprintf(PRINT_NORMAL, _T("S2650 #1 Rom Size: 0x%X (%i roms)\n"), GalS2650Rom1Size, GalS2650Rom1Num);
		if (GalTilesSharedRomSize) bprintf(PRINT_NORMAL, _T("Shared Tile Rom Size: 0x%X (%i roms, 0x%X Chars, 0x%X Sprites)\n"), GalTilesSharedRomSize, GalTilesSharedRomNum, GalNumChars, GalNumSprites);
		if (GalTilesCharRomSize) bprintf(PRINT_NORMAL, _T("Char Rom Size: 0x%X (%i roms, 0x%X Chars)\n"), GalTilesCharRomSize, GalTilesCharRomNum, GalNumChars);
		if (GalTilesSpriteRomSize) bprintf(PRINT_NORMAL, _T("Sprite Rom Size: 0x%X (%i roms, 0x%X Sprites)\n"), GalTilesSpriteRomSize, GalTilesSpriteRomNum, GalNumSprites);
		if (GalPromRomSize) bprintf(PRINT_NORMAL, _T("PROM Rom Size: 0x%X (%i roms)\n"), GalPromRomSize, GalPromRomNum);
#endif
	}

	if (bLoad) {
		INT32 Offset;
		
		// Z80 #1 Program Roms
		Offset = 0;
		for (i = 0; i < GalZ80Rom1Num; i++) {
			nRet = BurnLoadRom(GalZ80Rom1 + Offset, i, 1); if (nRet) return 1;
			
			BurnDrvGetRomInfo(&ri, i + 0);
			Offset += ri.nLen;
		}
		
		// Z80 #2 Program Roms
		if (GalZ80Rom2Size) {
			Offset = 0;
			for (i = GAL_ROM_OFFSET_Z80_PROG2; i < GAL_ROM_OFFSET_Z80_PROG2 + GalZ80Rom2Num; i++) {
				nRet = BurnLoadRom(GalZ80Rom2 + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// Z80 #3 Program Roms
		if (GalZ80Rom3Size) {
			Offset = 0;
			for (i = GAL_ROM_OFFSET_Z80_PROG3; i < GAL_ROM_OFFSET_Z80_PROG3 + GalZ80Rom3Num; i++) {
				nRet = BurnLoadRom(GalZ80Rom3 + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// Shared Tile Roms
		if (GalTilesSharedRomSize) {
			Offset = 0;
			GalTempRom = (UINT8*)BurnMalloc(GalTilesSharedRomSize);
			for (i = GAL_ROM_OFFSET_TILES_SHARED; i < GAL_ROM_OFFSET_TILES_SHARED + GalTilesSharedRomNum; i++) {
				nRet = BurnLoadRom(GalTempRom + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
			GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
			GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);		
			BurnFree(GalTempRom);
		}
		
		// Char Tile Roms
		if (GalTilesCharRomSize) {
			Offset = 0;
			GalTempRom = (UINT8*)BurnMalloc(GalTilesCharRomSize);
			for (i = GAL_ROM_OFFSET_TILES_CHARS; i < GAL_ROM_OFFSET_TILES_CHARS + GalTilesCharRomNum; i++) {
				nRet = BurnLoadRom(GalTempRom + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
			GfxDecode(GalNumChars, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, GalTempRom, GalChars);
			BurnFree(GalTempRom);
		}
		
		// Sprite Tile Roms
		if (GalTilesSpriteRomSize) {
			Offset = 0;
			GalTempRom = (UINT8*)BurnMalloc(GalTilesSpriteRomSize);
			for (i = GAL_ROM_OFFSET_TILES_SPRITES; i < GAL_ROM_OFFSET_TILES_SPRITES + GalTilesSpriteRomNum; i++) {
				nRet = BurnLoadRom(GalTempRom + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
			GfxDecode(GalNumSprites, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, GalTempRom, GalSprites);		
			BurnFree(GalTempRom);
		}
		
		// Prom
		if (GalPromRomSize) {
			Offset = 0;
			for (i = GAL_ROM_OFFSET_PROM; i < GAL_ROM_OFFSET_PROM + GalPromRomNum; i++) {
				nRet = BurnLoadRom(GalProm + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
		
		// S2650 #1 Program Roms
		if (GalS2650Rom1Size) {
			Offset = 0;
			for (i = GAL_ROM_OFFSET_S2650_PROG1; i < GAL_ROM_OFFSET_S2650_PROG1 + GalS2650Rom1Num; i++) {
				nRet = BurnLoadRom(GalS2650Rom1 + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			}
		}
	}

	return nRet;
}

// Konami PPI Handlers
UINT8 KonamiPPIReadIN0()
{
	return 0xff - GalInput[0] - GalDip[0];
}

UINT8 KonamiPPIReadIN1()
{
	return 0xff - GalInput[1] - GalDip[1];
}

UINT8 KonamiPPIReadIN2()
{
	return 0xff - GalInput[2] - GalDip[2];
}

UINT8 KonamiPPIReadIN3()
{
	return 0xff - GalInput[3] - GalDip[3];
}

void KonamiPPIInit()
{
	ppi8255_init(2);
	PPI0PortReadA = KonamiPPIReadIN0;
	PPI0PortReadB = KonamiPPIReadIN1;
	PPI0PortReadC = KonamiPPIReadIN2;
	PPI1PortReadC = KonamiPPIReadIN3;
	PPI1PortWriteA = KonamiSoundLatchWrite;
	PPI1PortWriteB = KonamiSoundControlWrite;
}

// Galaxian Memory Map
UINT8 __fastcall GalaxianZ80Read(UINT16 a)
{
	switch (a) {
		case 0x6000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x6800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x7000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0x7800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall GalaxianZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5800 && a <= 0x58ff) {
		int Offset = a - 0x5800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x6000:
		case 0x6001: {
			// start_lamp_w
			return;
		}
		
		case 0x6002: {
			// coin_lock_w
			return;
		}
		
		case 0x6003: {
			// coin_count_0_w
			return;
		}
		
		case 0x6004:
		case 0x6005:
		case 0x6006:
		case 0x6007: {
			GalaxianLfoFreqWrite(a - 0x6004, d);
			return;
		}
		
		case 0x6800:
		case 0x6801:
		case 0x6802:
		case 0x6803:
		case 0x6804:
		case 0x6805:
		case 0x6806:
		case 0x6807: {
			GalaxianSoundWrite(a - 0x6800, d);
			return;
		}
		
		case 0x7001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x7006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x7007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall GalaxianZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall GalaxianZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

INT32 GalInit()
{
	INT32 nLen;
	
	GalLoadRoms(0);
	
	if (!GalSoundType) GalSoundType = GAL_SOUND_HARDWARE_TYPE_GALAXIAN;

	// Allocate and Blank all required memory
	GalMem = NULL;
	GalMemIndex();
	nLen = GalMemEnd - (UINT8 *)0;
	if ((GalMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(GalMem, 0, nLen);
	GalMemIndex();

	if (GalLoadRoms(1)) return 1;
	
	// Setup the Z80 emulation
	if (GalSoundType != GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910) {
		if (GalZ80Rom3Size) {
			ZetInit(0);
			ZetInit(1);
			ZetInit(2);
		} else {
			if (GalZ80Rom2Size) {
				ZetInit(0);
				ZetInit(1);
			} else {
				if (GalZ80Rom1Size) {
					ZetInit(0);
				}
			}
		}
		if (GalZ80Rom1Size) {
			ZetOpen(0);
			ZetSetReadHandler(GalaxianZ80Read);
			ZetSetWriteHandler(GalaxianZ80Write);
			ZetSetInHandler(GalaxianZ80PortRead);
			ZetSetOutHandler(GalaxianZ80PortWrite);
			ZetMapArea(0x0000, (GalZ80Rom1Size > 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
			ZetMapArea(0x0000, (GalZ80Rom1Size > 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
			ZetMapArea(0x4000, 0x43ff, 0, GalZ80Ram1);
			ZetMapArea(0x4000, 0x43ff, 1, GalZ80Ram1);
			ZetMapArea(0x4000, 0x43ff, 2, GalZ80Ram1);
			ZetMapArea(0x4400, 0x47ff, 0, GalZ80Ram1);
			ZetMapArea(0x4400, 0x47ff, 1, GalZ80Ram1);
			ZetMapArea(0x4400, 0x47ff, 2, GalZ80Ram1);
			ZetMapArea(0x5000, 0x53ff, 0, GalVideoRam);
			ZetMapArea(0x5000, 0x53ff, 1, GalVideoRam);
			ZetMapArea(0x5000, 0x53ff, 2, GalVideoRam);
			ZetMapArea(0x5800, 0x58ff, 0, GalSpriteRam);
			ZetMapArea(0x5800, 0x58ff, 2, GalSpriteRam);
			ZetMemEnd();
			ZetClose();
		}
	}
	
	nGalCyclesTotal[0] = (18432000 / 3 / 2) / 60;
	
	if (GalPostLoadCallbackFunction) GalPostLoadCallbackFunction();
	
	GalCalcPaletteFunction = GalaxianCalcPalette;
	GalRenderBackgroundFunction = GalaxianDrawBackground;
	GalDrawBulletsFunction = GalaxianDrawBullets;
	
	GalIrqType = GAL_IRQ_TYPE_NMI;
	
	GalSpriteClipStart = 16;
	GalSpriteClipEnd = 255;
	
	GalSoundInit();
	
	GalInitStars();
	
	GenericTilesInit();
	
	GalColourDepth = 2;
	
	// Reset the driver
	GalDoReset();

	return 0;
}

// Moon Cresta Memory Map
UINT8 __fastcall MooncrstZ80Read(UINT16 a)
{
	switch (a) {
		case 0xa000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0xa800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0xb000: {
			return GalInput[2] | GalDip[2];
		}
		
		case 0xb800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall MooncrstZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		int Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0xa000:
		case 0xa001:
		case 0xa002: {
			GalGfxBank[a - 0xa000] = d;
			return;
		}
		
		case 0xa003: {
			// coin_count_0_w
			return;
		}
		
		case 0xa004:
		case 0xa005:
		case 0xa006:
		case 0xa007: {
			GalaxianLfoFreqWrite(a - 0xa004, d);
			return;
		}
		
		case 0xa800:
		case 0xa801:
		case 0xa802:
		case 0xa803:
		case 0xa804:
		case 0xa805:
		case 0xa806:
		case 0xa807: {
			GalaxianSoundWrite(a - 0xa800, d);
			return;
		}
		
		case 0xb000: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xb006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb800: {
			GalPitch = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall MooncrstZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall MooncrstZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void MapMooncrst()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(MooncrstZ80Read);
	ZetSetWriteHandler(MooncrstZ80Write);
	ZetSetInHandler(MooncrstZ80PortRead);
	ZetSetOutHandler(MooncrstZ80PortWrite);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size >= 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x83ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x83ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x83ff, 2, GalZ80Ram1);
	ZetMapArea(0x9000, 0x93ff, 0, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 1, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 2, GalVideoRam);
	ZetMapArea(0x9800, 0x98ff, 0, GalSpriteRam);
	ZetMapArea(0x9800, 0x98ff, 2, GalSpriteRam);
	ZetMemEnd();
	ZetClose();
}

// Jumpbug Memory Map
UINT8 __fastcall JumpbugZ80Read(UINT16 a)
{
	if (a >= 0xb000 && a <= 0xbfff) {
		INT32 Offset = a - 0xb000;
		
		switch (Offset) {
			case 0x114: return 0x4f;
			case 0x118: return 0xd3;
			case 0x214: return 0xcf;
			case 0x235: return 0x02;
			case 0x311: return 0xff;
		}
	}
	
	switch (a) {
		case 0x6000: {
			return GalInput[0] | GalDip[0];
		}
		
		case 0x6800: {
			return GalInput[1] | GalDip[1];
		}
		
		case 0x7000: {
			return GalInput[2] | GalDip[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall JumpbugZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x5800: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0x5900: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x6002:
		case 0x6003:
		case 0x6004:
		case 0x6005:
		case 0x6006: {
			GalGfxBank[a - 0x6002] = d;
			return;
		}
		
		case 0x7001: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x7002: {
			// coin_count_0_w
			return;
		}
		
		case 0x7004: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x7006: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x7007: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0x7800: {
			// watchdog write
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall JumpbugZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall JumpbugZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void MapJumpbug()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(JumpbugZ80Read);
	ZetSetWriteHandler(JumpbugZ80Write);
	ZetSetInHandler(JumpbugZ80PortRead);
	ZetSetOutHandler(JumpbugZ80PortWrite);
	ZetMapArea(0x0000, 0x3fff, 0, GalZ80Rom1);
	ZetMapArea(0x0000, 0x3fff, 2, GalZ80Rom1);
	ZetMapArea(0x4000, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 2, GalZ80Ram1);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 0, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 1, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 2, GalVideoRam);
	ZetMapArea(0x5000, 0x50ff, 0, GalSpriteRam);
	ZetMapArea(0x5000, 0x50ff, 2, GalSpriteRam);
	ZetMapArea(0x8000, GalZ80Rom1Size + 0x4000 - 1, 0, GalZ80Rom1 + 0x4000);
	ZetMapArea(0x8000, GalZ80Rom1Size + 0x4000 - 1, 2, GalZ80Rom1 + 0x4000);
	ZetMemEnd();
	ZetClose();
}

// Frogger Memory Map
UINT8 __fastcall FroggerZ80Read(UINT16 a)
{
	if (a >= 0xc000) {
		UINT32 Offset = a - 0xc000;
		UINT8 Result = 0xff;
		if (Offset & 0x1000) Result &= ppi8255_r(1, (Offset >> 1) & 3);
		if (Offset & 0x2000) Result &= ppi8255_r(0, (Offset >> 1) & 3);
		return Result;
	}
	
	switch (a) {
		case 0x8800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall FroggerZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0xb000 && a <= 0xb0ff) {
		INT32 Offset = a - 0xb000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				d = (d >> 4) | (d << 4);
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0xc000) {
		INT32 Offset = a - 0xc000;
		if (Offset & 0x1000) ppi8255_w(1, (Offset >> 1) & 3, d);
		if (Offset & 0x2000) ppi8255_w(0, (Offset >> 1) & 3, d);
		return;
	}
	
	switch (a) {
		case 0xb808: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xb80c: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xb810: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xb818: {
			// coin_count_0_w
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall FroggerZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall FroggerZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void MapFrogger()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(FroggerZ80Read);
	ZetSetWriteHandler(FroggerZ80Write);
	ZetSetInHandler(FroggerZ80PortRead);
	ZetSetOutHandler(FroggerZ80PortWrite);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetMapArea(0xa800, 0xabff, 0, GalVideoRam);
	ZetMapArea(0xa800, 0xabff, 1, GalVideoRam);
	ZetMapArea(0xa800, 0xabff, 2, GalVideoRam);
	ZetMapArea(0xb000, 0xb0ff, 0, GalSpriteRam);
	ZetMapArea(0xb000, 0xb0ff, 2, GalSpriteRam);
	ZetMemEnd();
	ZetClose();
}

// The End Memory Map
UINT8 __fastcall TheendZ80Read(UINT16 a)
{
	if (a >= 0x8000) {
		UINT32 Offset = a - 0x8000;
		UINT8 Result = 0xff;
		if (Offset & 0x0100) Result &= ppi8255_r(0, Offset & 3);
		if (Offset & 0x0200) Result &= ppi8255_r(1, Offset & 3);
		return Result;
	}
	
	switch (a) {
		case 0x7000: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall TheendZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x5000 && a <= 0x50ff) {
		INT32 Offset = a - 0x5000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0x8000) {
		INT32 Offset = a - 0x8000;
		if (Offset & 0x0100) ppi8255_w(0, Offset & 3, d);
		if (Offset & 0x0200) ppi8255_w(1, Offset & 3, d);
		return;
	}
	
	switch (a) {
		case 0x6801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0x6802: {
			// coin_count_0_w
			return;
		}
		
		case 0x6803: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0x6804: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0x6806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0x6807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall TheendZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall TheendZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void MapTheend()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(TheendZ80Read);
	ZetSetWriteHandler(TheendZ80Write);
	ZetSetInHandler(TheendZ80PortRead);
	ZetSetOutHandler(TheendZ80PortWrite);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x4000) ? 0x3fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x4000, 0x47ff, 0, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 1, GalZ80Ram1);
	ZetMapArea(0x4000, 0x47ff, 2, GalZ80Ram1);
	ZetMapArea(0x4800, 0x4bff, 0, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 1, GalVideoRam);
	ZetMapArea(0x4800, 0x4bff, 2, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 0, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 1, GalVideoRam);
	ZetMapArea(0x4c00, 0x4fff, 2, GalVideoRam);
	ZetMapArea(0x5000, 0x50ff, 0, GalSpriteRam);
	ZetMapArea(0x5000, 0x50ff, 2, GalSpriteRam);
	ZetMemEnd();
	ZetClose();
}

// Turtles Memory Map
UINT8 __fastcall TurtlesZ80Read(UINT16 a)
{
	if (a >= 0xb000 && a <= 0xb03f) {
		UINT32 Offset = a - 0xb000;
		return ppi8255_r(0, (Offset >> 4) & 3);
	}
	
	if (a >= 0xb800 && a <= 0xb83f) {
		UINT32 Offset = a - 0xb800;
		return ppi8255_r(1, (Offset >> 4) & 3);
	}
	
	switch (a) {
		case 0xa800: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall TurtlesZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9800 && a <= 0x98ff) {
		INT32 Offset = a - 0x9800;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	if (a >= 0xb000 && a <= 0xb03f) {
		UINT32 Offset = a - 0xb000;
		ppi8255_w(0, (Offset >> 4) & 3, d);
		return;
	}
	
	if (a >= 0xb800 && a <= 0xb83f) {
		UINT32 Offset = a - 0xb800;
		ppi8255_w(1, (Offset >> 4) & 3, d);
		return;
	}
	
	switch (a) {
		case 0xa000: {
			GalBackgroundRed = d & 1;
			return;
		}
		
		case 0xa008: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa010: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		case 0xa018: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xa020: {
			GalBackgroundGreen = d & 1;
			return;
		}
		
		case 0xa028: {
			GalBackgroundBlue = d & 1;
			return;
		}
		
		case 0xa030: {
			// coin_count_0_w
			return;
		}
		
		case 0xa038: {
			// coin_count_1_w
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall TurtlesZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall TurtlesZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void MapTurtles()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(TurtlesZ80Read);
	ZetSetWriteHandler(TurtlesZ80Write);
	ZetSetInHandler(TurtlesZ80PortRead);
	ZetSetOutHandler(TurtlesZ80PortWrite);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x8000) ? 0x7fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x8000) ? 0x7fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetMapArea(0x9000, 0x93ff, 0, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 1, GalVideoRam);
	ZetMapArea(0x9000, 0x93ff, 2, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 0, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 1, GalVideoRam);
	ZetMapArea(0x9400, 0x97ff, 2, GalVideoRam);
	ZetMapArea(0x9800, 0x98ff, 0, GalSpriteRam);
	ZetMapArea(0x9800, 0x98ff, 2, GalSpriteRam);
	ZetMemEnd();
	ZetClose();
}

// Super Cobra Memory Map
UINT8 __fastcall ScobraZ80Read(UINT16 a)
{
	switch (a) {
		case 0x9800:
		case 0x9801:
		case 0x9802:
		case 0x9803: {
			return ppi8255_r(0, a - 0x9800);
		}
		
		case 0xa000:
		case 0xa001:
		case 0xa002:
		case 0xa003: {
			return ppi8255_r(1, a - 0xa000);
		}
		
		case 0xb000: {
			// watchdog read
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall ScobraZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0x9000 && a <= 0x90ff) {
		INT32 Offset = a - 0x9000;
		
		GalSpriteRam[Offset] = d;
		
		if (Offset < 0x40) {
			if ((Offset & 0x01) == 0) {
				GalScrollVals[Offset >> 1] = d;
			}
		}
		
		return;
	}
	
	switch (a) {
		case 0x9800:
		case 0x9801:
		case 0x9802:
		case 0x9803: {
			return ppi8255_w(0, a - 0x9800, d);
		}
		
		case 0xa000:
		case 0xa001:
		case 0xa002:
		case 0xa003: {
			return ppi8255_w(1, a - 0xa000, d);
		}
		
		case 0xa801: {
			GalIrqFire = d & 1;
			return;
		}
		
		case 0xa802: {
			// coin_count_0_w
			return;
		}
		
		case 0xa803: {
			GalBackgroundEnable = d & 1;
			return;
		}
		
		case 0xa804: {
			GalStarsEnable = d & 0x01;
			if (!GalStarsEnable) GalStarsScrollPos = -1;
			return;
		}
		
		case 0xa806: {
			GalFlipScreenX = d & 1;
			return;
		}
		
		case 0xa807: {
			GalFlipScreenY = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall ScobraZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}
	
	return 0xff;
}

void __fastcall ScobraZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void MapScobra()
{
	ZetOpen(0);
	ZetMemCallback(0x0000, 0xffff, 0);
	ZetMemCallback(0x0000, 0xffff, 1);
	ZetMemCallback(0x0000, 0xffff, 2);
	ZetSetReadHandler(ScobraZ80Read);
	ZetSetWriteHandler(ScobraZ80Write);
	ZetSetInHandler(ScobraZ80PortRead);
	ZetSetOutHandler(ScobraZ80PortWrite);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x8000) ? 0x7fff : GalZ80Rom1Size - 1, 0, GalZ80Rom1);
	ZetMapArea(0x0000, (GalZ80Rom1Size > 0x8000) ? 0x7fff : GalZ80Rom1Size - 1, 2, GalZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 1, GalZ80Ram1);
	ZetMapArea(0x8000, 0x87ff, 2, GalZ80Ram1);
	ZetMapArea(0x8800, 0x8bff, 0, GalVideoRam);
	ZetMapArea(0x8800, 0x8bff, 1, GalVideoRam);
	ZetMapArea(0x8800, 0x8bff, 2, GalVideoRam);
	ZetMapArea(0x8c00, 0x8fff, 0, GalVideoRam);
	ZetMapArea(0x8c00, 0x8fff, 1, GalVideoRam);
	ZetMapArea(0x8c00, 0x8fff, 2, GalVideoRam);
	ZetMapArea(0x9000, 0x90ff, 0, GalSpriteRam);
	ZetMapArea(0x9000, 0x90ff, 2, GalSpriteRam);
	ZetMemEnd();
	ZetClose();
}

INT32 GalExit()
{
	if (GalZ80Rom1Size) ZetExit();
	if (GalS2650Rom1Size) s2650Exit();
	
	GenericTilesExit();
	
	BurnFree(GalMem);
	BurnFree(GalZ80Rom1Op);
	BurnFree(GalVideoRam2);
	BurnFree(RockclimTiles);
	
	GalSoundExit();
	
	GalIrqFire = 0;
	GalIrqType = 0;
	GalSoundType = 0;
	GalSoundVolumeShift = 0;
	GalFlipScreenX = 0;
	GalFlipScreenY = 0;
	ZigzagAYLatch = 0;
	GalSoundLatch = 0;
	GalSoundLatch2 = 0;
	KingballSpeechDip = 0;
	KingballSound = 0;
	KonamiSoundControl = 0;
	SfxSampleControl = 0;
	GmgalaxSelectedGame = 0;
	GalPaletteBank = 0;
	GalSpriteClipStart = 0;
	GalSpriteClipEnd = 0;
	FroggerAdjust = 0;
	GalBackgroundRed = 0;
	GalBackgroundGreen = 0;
	GalBackgroundBlue = 0;
	GalBackgroundEnable = 0;
	ScrambleProtectionState = 0;
	ScrambleProtectionResult = 0;
	
	GalZ80Rom1Size = 0;
	GalZ80Rom1Num = 0;
	GalZ80Rom2Size = 0;
	GalZ80Rom2Num = 0;
	GalZ80Rom3Size = 0;
	GalZ80Rom3Num = 0;
	GalS2650Rom1Size = 0;
	GalS2650Rom1Num = 0;
	GalTilesSharedRomSize = 0;
	GalTilesSharedRomNum = 0;
	GalTilesCharRomSize = 0;
	GalTilesCharRomNum = 0;
	GalNumChars = 0;
	GalTilesSpriteRomSize = 0;
	GalTilesSpriteRomNum = 0;
	GalNumSprites = 0;
	GalPromRomSize = 0;
	GalPromRomNum = 0;
	
	GalPostLoadCallbackFunction = NULL;
	GalRenderBackgroundFunction = NULL;
	GalCalcPaletteFunction = NULL;
	GalDrawBulletsFunction = NULL;
	GalExtendTileInfoFunction = NULL;
	GalExtendSpriteInfoFunction = NULL;
	GalRenderFrameFunction = NULL;
	
	SfxTilemap = 0;
	GalOrientationFlipX = 0;
	MoonwarPortSelect = 0;
	MshuttleAY8910CS = 0;
	Fourin1Bank = 0;
	GameIsGmgalax = 0;
	CavelonBankSwitch = 0;
	DarkplntBulletColour = 0;
	DambustrBgColour1 = 0;
	DambustrBgColour2 = 0;
	DambustrBgPriority = 0;
	DambustrBgSplitLine = 0;
	RockclimScrollX = 0;
	RockclimScrollY = 0;
	GalVBlank = 0;
	
	GalColourDepth = 0;

	return 0;
}

INT32 KonamiExit()
{
	ppi8255_exit();
	
	return GalExit();
}

INT32 GalFrame()
{
	INT32 nInterleave = 8;
	INT32 nSoundBufferPos = 0;
	
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_GALAXIAN || GalSoundType == GAL_SOUND_HARDWARE_TYPE_KINGBALLDAC) {
		nInterleave = nBurnSoundLen;
		GalaxianSoundUpdateTimers();
	}
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910) nInterleave = nBurnSoundLen / 4;
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC) nInterleave = 32;
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_CHECKMAJAY8910) nInterleave = 32;
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910) nInterleave = 32;
	
	INT32 nIrqInterleaveFire = nInterleave / 4;	

	if (GalReset) GalDoReset();
	
	if (GameIsGmgalax && (GmgalaxSelectedGame != GalFakeDip)) {
		GmgalaxSelectedGame = GalFakeDip;
		INT32 nAddress = 0x0000;
		if (GmgalaxSelectedGame == 1) nAddress = 0x4000;
		ZetOpen(0);
		ZetMapArea(0x0000, 0x3fff, 0, GalZ80Rom1 + nAddress);
		ZetMapArea(0x0000, 0x3fff, 2 ,GalZ80Rom1 + nAddress);
		ZetClose();
		
		GalGfxBank[0] = 0;
		if (GmgalaxSelectedGame == 1) GalGfxBank[0] = 1;
		GalPaletteBank = 0;
		if (GmgalaxSelectedGame == 1) GalPaletteBank = 1;
		
		GalDoReset();
	}

	GalMakeInputs();
	GalVBlank = 0;
	
	if (GalZ80Rom1Size) ZetNewFrame();

	nGalCyclesDone[0] = nGalCyclesDone[1] = nGalCyclesDone[2] = 0;
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

			// Run Z80 #1
		if (GalZ80Rom1Size && GalSoundType != GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910) {
			nCurrentCPU = 0;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nGalCyclesTotal[nCurrentCPU] / nInterleave;
			nGalCyclesSegment = nNext - nGalCyclesDone[nCurrentCPU];
			nGalCyclesDone[nCurrentCPU] += ZetRun(nGalCyclesSegment);
			if (i == nIrqInterleaveFire && GalIrqFire) {
				if (GalIrqType == GAL_IRQ_TYPE_NMI) ZetNmi();
				if (GalIrqType == GAL_IRQ_TYPE_IRQ0) ZetRaiseIrq(0);
				GalIrqFire = 0;
			}
			ZetClose();
		}
			
		if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_KINGBALLDAC || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC || GalSoundType == GAL_SOUND_HARDWARE_TYPE_AD2083AY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_CHECKMANAY8910) {
			// Run Z80 #2
			nCurrentCPU = 1;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nGalCyclesTotal[nCurrentCPU] / nInterleave;
			nGalCyclesSegment = nNext - nGalCyclesDone[nCurrentCPU];
			nGalCyclesDone[nCurrentCPU] += ZetRun(nGalCyclesSegment);
			if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_CHECKMANAY8910) ZetRaiseIrq(0);
			ZetClose();
		}
			
		if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_CHECKMAJAY8910) {
			// Run Z80 #2
			nCurrentCPU = 1;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nGalCyclesTotal[nCurrentCPU] / nInterleave;
			nGalCyclesSegment = nNext - nGalCyclesDone[nCurrentCPU];
			nGalCyclesSegment = ZetRun(nGalCyclesSegment);
			nGalCyclesDone[nCurrentCPU] += nGalCyclesSegment;
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			nGalCyclesDone[nCurrentCPU] += ZetRun(300);
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			nGalCyclesDone[nCurrentCPU] += ZetRun(300);
			ZetClose();
		}
			
		if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC) {
			// Run Z80 #3
			nCurrentCPU = 2;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nGalCyclesTotal[nCurrentCPU] / nInterleave;
			nGalCyclesSegment = nNext - nGalCyclesDone[nCurrentCPU];
			nGalCyclesDone[nCurrentCPU] += ZetRun(nGalCyclesSegment);
			ZetClose();
		}
			
		// Run S2650 #1
		if (GalS2650Rom1Size) {
			nCurrentCPU = 0;
			s2650Open(nCurrentCPU);
			nNext = (i + 1) * nGalCyclesTotal[nCurrentCPU] / nInterleave;
			nGalCyclesSegment = nNext - nGalCyclesDone[nCurrentCPU];
			nGalCyclesDone[nCurrentCPU] += s2650Run(nGalCyclesSegment);
			if (i == nIrqInterleaveFire) {
				GalVBlank = 1;
				s2650_set_irq_line(0, 1);
				s2650Run(0);
				s2650_set_irq_line(0, 0);
				s2650Run(0);
			}
			s2650Close();
		}
			
		if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910) {
			// Run Z80 #1
			nCurrentCPU = 1;
			ZetOpen(0);
			nNext = (i + 1) * nGalCyclesTotal[nCurrentCPU] / nInterleave;
			nGalCyclesSegment = nNext - nGalCyclesDone[nCurrentCPU];
			nGalCyclesDone[nCurrentCPU] += ZetRun(nGalCyclesSegment);
			if (HunchbksSoundIrqFire) ZetRaiseIrq(0);
			ZetClose();
		}
		
		if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_GALAXIAN) {
			if (pBurnSoundOut) {
				INT32 nSegmentLength = nBurnSoundLen / nInterleave;
				INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				GalRenderSoundSamples(pSoundBuf, nSegmentLength);
				nSoundBufferPos += nSegmentLength;
			}
		}
		
		if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_ZIGZAGAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_JUMPBUGAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_CHECKMANAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_CHECKMAJAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC || GalSoundType == GAL_SOUND_HARDWARE_TYPE_MSHUTTLEAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_BONGOAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_AD2083AY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910) {
			if (pBurnSoundOut) {
				INT32 nSample;
				INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
				INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
				if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC || GalSoundType == GAL_SOUND_HARDWARE_TYPE_AD2083AY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910) {
					AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
				}				
				if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910) {
					AY8910Update(2, &pAY8910Buffer[6], nSegmentLength);
				}
				for (INT32 n = 0; n < nSegmentLength; n++) {
					nSample  = pAY8910Buffer[0][n] >> GalSoundVolumeShift;
					nSample += pAY8910Buffer[1][n] >> GalSoundVolumeShift;
					nSample += pAY8910Buffer[2][n] >> GalSoundVolumeShift;
					if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC || GalSoundType == GAL_SOUND_HARDWARE_TYPE_AD2083AY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910) {
						nSample += pAY8910Buffer[3][n] >> GalSoundVolumeShift;
						nSample += pAY8910Buffer[4][n] >> GalSoundVolumeShift;
						nSample += pAY8910Buffer[5][n] >> GalSoundVolumeShift;
					}
					if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910) {
						nSample += pAY8910Buffer[6][n] >> GalSoundVolumeShift;
						nSample += pAY8910Buffer[7][n] >> GalSoundVolumeShift;
						nSample += pAY8910Buffer[8][n] >> GalSoundVolumeShift;
					}

					nSample = BURN_SND_CLIP(nSample);

					pSoundBuf[(n << 1) + 0] = nSample;
					pSoundBuf[(n << 1) + 1] = nSample;
    			}
				nSoundBufferPos += nSegmentLength;
			}
		}
		
		if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_KINGBALLDAC) {
			if (pBurnSoundOut) {
				INT32 nSegmentLength = nBurnSoundLen / nInterleave;
				INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

				GalRenderSoundSamples(pSoundBuf, nSegmentLength);
				DACUpdate(pSoundBuf, nSegmentLength);
				nSoundBufferPos += nSegmentLength;
			}
		}
		
		if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_RACKNROLSN76496 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HEXPOOLASN76496) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			SN76496Update(0, pSoundBuf, nSegmentLength);
			if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_RACKNROLSN76496) {
				SN76496Update(1, pSoundBuf, nSegmentLength);
				SN76496Update(2, pSoundBuf, nSegmentLength);
			}
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	// Make sure the buffer is entirely filled.
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_GALAXIAN) {
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			if (nSegmentLength) {
				GalRenderSoundSamples(pSoundBuf, nSegmentLength);
			}
		}
	}
	
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_ZIGZAGAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_JUMPBUGAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_CHECKMANAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_CHECKMAJAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FROGGERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC || GalSoundType == GAL_SOUND_HARDWARE_TYPE_MSHUTTLEAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_BONGOAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_AD2083AY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910) {
		if (pBurnSoundOut) {
			INT32 nSample;
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (nSegmentLength) {
				AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
				if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC || GalSoundType == GAL_SOUND_HARDWARE_TYPE_AD2083AY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910) {
					AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
				}
				if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910) {
					AY8910Update(2, &pAY8910Buffer[6], nSegmentLength);
				}
				for (INT32 n = 0; n < nSegmentLength; n++) {
					nSample  = pAY8910Buffer[0][n] >> GalSoundVolumeShift;
					nSample += pAY8910Buffer[1][n] >> GalSoundVolumeShift;
					nSample += pAY8910Buffer[2][n] >> GalSoundVolumeShift;
					if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_KONAMIAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_EXPLORERAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC || GalSoundType == GAL_SOUND_HARDWARE_TYPE_AD2083AY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HUNCHBACKAY8910 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_FANTASTCAY8910) {
						nSample += pAY8910Buffer[3][n] >> GalSoundVolumeShift;
						nSample += pAY8910Buffer[4][n] >> GalSoundVolumeShift;
						nSample += pAY8910Buffer[5][n] >> GalSoundVolumeShift;
					}
					if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_SCORPIONAY8910) {
						nSample += pAY8910Buffer[6][n] >> GalSoundVolumeShift;
						nSample += pAY8910Buffer[7][n] >> GalSoundVolumeShift;
						nSample += pAY8910Buffer[8][n] >> GalSoundVolumeShift;
					}

					nSample = BURN_SND_CLIP(nSample);

					pSoundBuf[(n << 1) + 0] = nSample;
					pSoundBuf[(n << 1) + 1] = nSample;
 				}
 			}
		}
	}
	
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_KINGBALLDAC) {
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			if (nSegmentLength) {
				GalRenderSoundSamples(pSoundBuf, nSegmentLength);
				DACUpdate(pSoundBuf, nSegmentLength);
			}
		}
	}
	
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_SFXAY8910DAC) {
		if (pBurnSoundOut) DACUpdate(pBurnSoundOut, nBurnSoundLen);
	}
	
	if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_RACKNROLSN76496 || GalSoundType == GAL_SOUND_HARDWARE_TYPE_HEXPOOLASN76496) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			SN76496Update(0, pSoundBuf, nSegmentLength);
			if (GalSoundType == GAL_SOUND_HARDWARE_TYPE_RACKNROLSN76496) {
				SN76496Update(1, pSoundBuf, nSegmentLength);
				SN76496Update(2, pSoundBuf, nSegmentLength);
			}
		}
	}
	
	if (pBurnDraw) GalDraw();

	return 0;
}

INT32 GalScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029703;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = GalRamStart;
		ba.nLen	  = GalRamEnd - GalRamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		if (GalZ80Rom1Size) ZetScan(nAction);
		ppi8255_scan();
		if (GalS2650Rom1Size) s2650Scan(nAction, pnMin);
		
		GalSoundScan(nAction, pnMin);
		
		// Scan critical driver variables
		SCAN_VAR(GalInput);
		SCAN_VAR(GalAnalogPort0);
		SCAN_VAR(GalAnalogPort1);
		SCAN_VAR(GalIrqFire);
		SCAN_VAR(nGalCyclesDone);
		SCAN_VAR(nGalCyclesSegment);
		SCAN_VAR(ZigzagAYLatch);
		SCAN_VAR(GalSoundLatch);
		SCAN_VAR(GalSoundLatch2);
		SCAN_VAR(KingballSound);
		SCAN_VAR(KingballSpeechDip);
		SCAN_VAR(KonamiSoundControl);
		SCAN_VAR(SfxSampleControl);
		SCAN_VAR(ScrambleProtectionResult);
		SCAN_VAR(MoonwarPortSelect);
		SCAN_VAR(MshuttleAY8910CS);
		SCAN_VAR(GmgalaxSelectedGame);
		SCAN_VAR(Fourin1Bank);
		SCAN_VAR(CavelonBankSwitch);
		SCAN_VAR(GalFlipScreenX);
		SCAN_VAR(GalFlipScreenY);
		SCAN_VAR(GalPaletteBank);
		SCAN_VAR(GalBackgroundRed);
		SCAN_VAR(GalBackgroundGreen);
		SCAN_VAR(GalBackgroundBlue);
		SCAN_VAR(GalBackgroundEnable);
		SCAN_VAR(DarkplntBulletColour);
		SCAN_VAR(DambustrBgColour1);
		SCAN_VAR(DambustrBgColour2);
		SCAN_VAR(DambustrBgPriority);
		SCAN_VAR(DambustrBgSplitLine);
		SCAN_VAR(RockclimScrollX);
		SCAN_VAR(RockclimScrollY);
		SCAN_VAR(GalStarsEnable);
		SCAN_VAR(GalStarsScrollPos);
		SCAN_VAR(GalStarsBlinkState);
		SCAN_VAR(GalVBlank);
	}
	
	return 0;
}
