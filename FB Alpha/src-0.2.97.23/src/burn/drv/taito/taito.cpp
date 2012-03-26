#include "tiles_generic.h"
#include "taito.h"
#include "taito_ic.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym2151.h"
#include "burn_ym2610.h"
#include "burn_ym2203.h"
#include "msm5205.h"
#include "msm6295.h"
#include "eeprom.h"
#include "burn_gun.h"

UINT8 TaitoInputPort0[8]       = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TaitoInputPort1[8]       = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TaitoInputPort2[8]       = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TaitoInputPort3[8]       = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TaitoInputPort4[8]       = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TaitoInputPort5[8]       = { 0, 0, 0, 0, 0, 0, 0, 0 };
UINT8 TaitoDip[2]              = { 0, 0 };
UINT8 TaitoInput[6]            = { 0, 0, 0, 0, 0, 0 };
UINT8 TaitoReset               = 0;

UINT8 TaitoCoinLockout[4]      = { 0, 0, 0, 0 };

INT32 TaitoAnalogPort0         = 0;
INT32 TaitoAnalogPort1         = 0;
INT32 TaitoAnalogPort2         = 0;
INT32 TaitoAnalogPort3         = 0;

UINT8 *TaitoMem                = NULL;
UINT8 *TaitoMemEnd             = NULL;
UINT8 *TaitoRamStart           = NULL;
UINT8 *TaitoRamEnd             = NULL;
UINT8 *Taito68KRam1            = NULL;
UINT8 *Taito68KRam2            = NULL;
UINT8 *Taito68KRam3            = NULL;
UINT8 *TaitoSharedRam          = NULL;
UINT8 *TaitoZ80Ram1            = NULL;
UINT8 *TaitoZ80Ram2            = NULL;
UINT8 *TaitoPaletteRam         = NULL;
UINT8 *TaitoSpriteRam          = NULL;
UINT8 *TaitoSpriteRam2         = NULL;
UINT8 *TaitoSpriteRamBuffered  = NULL;
UINT8 *TaitoSpriteRamDelayed   = NULL;
UINT8 *TaitoSpriteExtension    = NULL;
UINT8 *TaitoVideoRam           = NULL;
UINT32 *TaitoPalette           = NULL;
UINT8 *TaitoPriorityMap        = NULL;

UINT8 TaitoZ80Bank = 0;
UINT8 TaitoSoundLatch = 0;
UINT16 TaitoCpuACtrl = 0;
INT32 TaitoRoadPalBank = 0;

INT32 TaitoXOffset = 0;
INT32 TaitoYOffset = 0;
INT32 TaitoIrqLine = 0;
INT32 TaitoFrameInterleave = 0;
INT32 TaitoFlipScreenX = 0;

INT32 TaitoNum68Ks = 0;
INT32 TaitoNumZ80s = 0;
INT32 TaitoNumYM2610 = 0;
INT32 TaitoNumYM2151 = 0;
INT32 TaitoNumYM2203 = 0;
INT32 TaitoNumMSM5205 = 0;
INT32 TaitoNumMSM6295 = 0;
INT32 TaitoNumES5505 = 0;
INT32 TaitoNumEEPROM = 0;

INT32 nTaitoCyclesDone[4], nTaitoCyclesTotal[4];
INT32 nTaitoCyclesSegment;

TaitoRender TaitoDrawFunction;
TaitoMakeInputs TaitoMakeInputsFunction;
TaitoResetFunc TaitoResetFunction;

INT32 TaitoDoReset()
{
	INT32 i;
	
	for (i = 0; i < TaitoNum68Ks; i++) {
		SekOpen(i);
		SekReset();
		SekClose();
	}
	
	for (i = 0; i < TaitoNumZ80s; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}
	
	if (TaitoNumYM2610) BurnYM2610Reset();
	if (TaitoNumYM2151) BurnYM2151Reset();
	if (TaitoNumYM2203) BurnYM2203Reset();
	
	if (TaitoNumMSM5205) MSM5205Reset();
	
	for (i = 0; i < TaitoNumMSM6295; i++) {
		MSM6295Reset(i);
	}
	
	if (TaitoNumEEPROM) EEPROMReset();
	
	TaitoICReset();
	
	TaitoZ80Bank = 0;
	TaitoSoundLatch = 0;
	TaitoRoadPalBank = 0;
	TaitoCpuACtrl = 0xff;
	
	memset (TaitoCoinLockout, 0, 4);

	return 0;
}

UINT8 *Taito68KRom1    = NULL;
UINT8 *Taito68KRom2    = NULL;
UINT8 *Taito68KRom3    = NULL;
UINT8 *TaitoZ80Rom1    = NULL;
UINT8 *TaitoZ80Rom2    = NULL;
UINT8 *TaitoChars      = NULL;
UINT8 *TaitoCharsB     = NULL;
UINT8 *TaitoCharsPivot = NULL;
UINT8 *TaitoSpritesA   = NULL;
UINT8 *TaitoSpritesB   = NULL;
UINT8 *TaitoSpriteMapRom = NULL;
UINT8 *TaitoYM2610ARom = NULL;
UINT8 *TaitoYM2610BRom = NULL;
UINT8 *TaitoMSM5205Rom = NULL;
UINT8 *TaitoMSM6295Rom = NULL;
UINT8 *TaitoES5505Rom  = NULL;
UINT8 *TaitoDefaultEEProm = NULL;

UINT32 Taito68KRom1Num = 0;
UINT32 Taito68KRom2Num = 0;
UINT32 Taito68KRom3Num = 0;
UINT32 TaitoZ80Rom1Num = 0;
UINT32 TaitoZ80Rom2Num = 0;
UINT32 TaitoCharRomNum = 0;
UINT32 TaitoCharBRomNum = 0;
UINT32 TaitoCharPivotRomNum = 0;
UINT32 TaitoSpriteARomNum = 0;
UINT32 TaitoSpriteBRomNum = 0;
UINT32 TaitoRoadRomNum       = 0;
UINT32 TaitoSpriteMapRomNum  = 0;
UINT32 TaitoYM2610ARomNum = 0;
UINT32 TaitoYM2610BRomNum = 0;
UINT32 TaitoMSM5205RomNum = 0;
UINT32 TaitoMSM6295RomNum = 0;
UINT32 TaitoES5505RomNum = 0;
UINT32 TaitoDefaultEEPromNum = 0;

UINT32 Taito68KRom1Size = 0;
UINT32 Taito68KRom2Size = 0;
UINT32 Taito68KRom3Size = 0;
UINT32 TaitoZ80Rom1Size = 0;
UINT32 TaitoZ80Rom2Size = 0;
UINT32 TaitoCharRomSize = 0;
UINT32 TaitoCharBRomSize = 0;
UINT32 TaitoCharPivotRomSize = 0;
UINT32 TaitoSpriteARomSize = 0;
UINT32 TaitoSpriteBRomSize = 0;
UINT32 TaitoRoadRomSize = 0;
UINT32 TaitoSpriteMapRomSize = 0;
UINT32 TaitoYM2610ARomSize = 0;
UINT32 TaitoYM2610BRomSize = 0;
UINT32 TaitoMSM5205RomSize = 0;
UINT32 TaitoMSM6295RomSize = 0;
UINT32 TaitoES5505RomSize = 0;
UINT32 TaitoDefaultEEPromSize = 0;

UINT32 TaitoCharModulo = 0;
UINT32 TaitoCharNumPlanes = 0;
UINT32 TaitoCharWidth = 0;
UINT32 TaitoCharHeight = 0;
UINT32 TaitoNumChar = 0;
INT32 *TaitoCharPlaneOffsets = NULL;
INT32 *TaitoCharXOffsets = NULL;
INT32 *TaitoCharYOffsets = NULL;

UINT32 TaitoCharBModulo = 0;
UINT32 TaitoCharBNumPlanes = 0;
UINT32 TaitoCharBWidth = 0;
UINT32 TaitoCharBHeight = 0;
UINT32 TaitoNumCharB = 0;
INT32 *TaitoCharBPlaneOffsets = NULL;
INT32 *TaitoCharBXOffsets = NULL;
INT32 *TaitoCharBYOffsets = NULL;

UINT32 TaitoCharPivotModulo = 0;
UINT32 TaitoCharPivotNumPlanes = 0;
UINT32 TaitoCharPivotWidth = 0;
UINT32 TaitoCharPivotHeight = 0;
UINT32 TaitoNumCharPivot = 0;
INT32 *TaitoCharPivotPlaneOffsets = NULL;
INT32 *TaitoCharPivotXOffsets = NULL;
INT32 *TaitoCharPivotYOffsets = NULL;

UINT32 TaitoSpriteAModulo = 0;
UINT32 TaitoSpriteANumPlanes = 0;
UINT32 TaitoSpriteAWidth = 0;
UINT32 TaitoSpriteAHeight = 0;
UINT32 TaitoNumSpriteA = 0;
INT32 *TaitoSpriteAPlaneOffsets = NULL;
INT32 *TaitoSpriteAXOffsets = NULL;
INT32 *TaitoSpriteAYOffsets = NULL;
INT32 TaitoSpriteAInvertRom = 0;

UINT32 TaitoSpriteBModulo = 0;
UINT32 TaitoSpriteBNumPlanes = 0;
UINT32 TaitoSpriteBWidth = 0;
UINT32 TaitoSpriteBHeight = 0;
UINT32 TaitoNumSpriteB = 0;
INT32 *TaitoSpriteBPlaneOffsets = NULL;
INT32 *TaitoSpriteBXOffsets = NULL;
INT32 *TaitoSpriteBYOffsets = NULL;

INT32 TaitoLoadRoms(INT32 bLoad)
{
	struct BurnRomInfo ri;
	ri.nType = 0;
	ri.nLen = 0;
	INT32 nOffset = -1;
	UINT32 i = 0;
	INT32 nRet = 0;
	
	if (!bLoad) {
		do {
			ri.nLen = 0;
			ri.nType = 0;
			BurnDrvGetRomInfo(&ri, ++nOffset);
			if ((ri.nType & 0xff) == TAITO_68KROM1 || (ri.nType & 0xff) == TAITO_68KROM1_BYTESWAP || (ri.nType & 0xff) == TAITO_68KROM1_BYTESWAP_JUMPING || (ri.nType & 0xff) == TAITO_68KROM1_BYTESWAP32) {
				Taito68KRom1Size += ri.nLen;
				Taito68KRom1Num++;
			}
			if ((ri.nType & 0xff) == TAITO_68KROM2 || (ri.nType & 0xff) == TAITO_68KROM2_BYTESWAP) {
				Taito68KRom2Size += ri.nLen;
				Taito68KRom2Num++;
			}
			if ((ri.nType & 0xff) == TAITO_68KROM3 || (ri.nType & 0xff) == TAITO_68KROM3_BYTESWAP) {
				Taito68KRom3Size += ri.nLen;
				Taito68KRom3Num++;
			}
			if ((ri.nType & 0xff) == TAITO_Z80ROM1) {
				TaitoZ80Rom1Size += ri.nLen;
				TaitoZ80Rom1Num++;
			}
			if ((ri.nType & 0xff) == TAITO_Z80ROM2) {
				TaitoZ80Rom2Size += ri.nLen;
				TaitoZ80Rom2Num++;
			}
			if ((ri.nType & 0xff) == TAITO_CHARS || (ri.nType & 0xff) == TAITO_CHARS_BYTESWAP) {
				TaitoCharRomSize += ri.nLen;
				TaitoCharRomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_CHARSB || (ri.nType & 0xff) == TAITO_CHARSB_BYTESWAP) {
				TaitoCharBRomSize += ri.nLen;
				TaitoCharBRomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_SPRITESA || (ri.nType & 0xff) == TAITO_SPRITESA_BYTESWAP || (ri.nType & 0xff) == TAITO_SPRITESA_BYTESWAP32 || (ri.nType & 0xff) == TAITO_SPRITESA_TOPSPEED) {
				TaitoSpriteARomSize += ri.nLen;
				TaitoSpriteARomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_SPRITESB || (ri.nType & 0xff) == TAITO_SPRITESB_BYTESWAP || (ri.nType & 0xff) == TAITO_SPRITESB_BYTESWAP32) {
				TaitoSpriteBRomSize += ri.nLen;
				TaitoSpriteBRomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_ROAD) {
				TaitoRoadRomSize += ri.nLen;
				TaitoRoadRomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_SPRITEMAP) {
				TaitoSpriteMapRomSize += ri.nLen;
				TaitoSpriteMapRomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_YM2610A) {
				TaitoYM2610ARomSize += ri.nLen;
				TaitoYM2610ARomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_YM2610B) {
				TaitoYM2610BRomSize += ri.nLen;
				TaitoYM2610BRomNum++;
			}			
			if ((ri.nType & 0xff) == TAITO_MSM5205 || (ri.nType & 0xff) == TAITO_MSM5205_BYTESWAP) {
				TaitoMSM5205RomSize += ri.nLen;
				TaitoMSM5205RomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_CHARS_PIVOT) {
				TaitoCharPivotRomSize += ri.nLen;
				TaitoCharPivotRomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_MSM6295) {
				TaitoMSM6295RomSize += ri.nLen;
				TaitoMSM6295RomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_ES5505 || (ri.nType & 0xff) == TAITO_ES5505_BYTESWAP) {
				TaitoES5505RomSize += ri.nLen;
				TaitoES5505RomNum++;
			}
			if ((ri.nType & 0xff) == TAITO_DEFAULT_EEPROM) {
				TaitoDefaultEEPromSize += ri.nLen;
				TaitoDefaultEEPromNum++;
			}
		} while (ri.nLen);
		
#if 1 && defined FBA_DEBUG
		if (Taito68KRom1Size) bprintf(PRINT_IMPORTANT, _T("68K #1 Rom Length %06X, (%i roms)\n"), Taito68KRom1Size, Taito68KRom1Num);
		if (Taito68KRom2Size) bprintf(PRINT_IMPORTANT, _T("68K #2 Rom Length %06X, (%i roms)\n"), Taito68KRom2Size, Taito68KRom2Num);
		if (Taito68KRom3Size) bprintf(PRINT_IMPORTANT, _T("68K #3 Rom Length %06X, (%i roms)\n"), Taito68KRom3Size, Taito68KRom3Num);
		if (TaitoZ80Rom1Size) bprintf(PRINT_IMPORTANT, _T("Z80 #1 Rom Length %06X, (%i roms)\n"), TaitoZ80Rom1Size, TaitoZ80Rom1Num);
		if (TaitoZ80Rom2Size) bprintf(PRINT_IMPORTANT, _T("Z80 #2 Rom Length %06X, (%i roms)\n"), TaitoZ80Rom2Size, TaitoZ80Rom2Num);
		if (TaitoCharRomSize) bprintf(PRINT_IMPORTANT, _T("Char Rom Length %08X, (%i roms, 0x%06X tiles)\n"), TaitoCharRomSize, TaitoCharRomNum, TaitoNumChar);
		if (TaitoCharBRomSize) bprintf(PRINT_IMPORTANT, _T("Char B Rom Length %08X, (%i roms, 0x%06X tiles)\n"), TaitoCharBRomSize, TaitoCharBRomNum, TaitoNumCharB);
		if (TaitoSpriteARomSize) bprintf(PRINT_IMPORTANT, _T("Sprite A Rom Length %08X, (%i roms, 0x%06X tiles)\n"), TaitoSpriteARomSize, TaitoSpriteARomNum, TaitoNumSpriteA);
		if (TaitoSpriteBRomSize) bprintf(PRINT_IMPORTANT, _T("Sprite B Rom Length %08X, (%i roms, 0x%06X tiles)\n"), TaitoSpriteBRomSize, TaitoSpriteBRomNum, TaitoNumSpriteB);
		if (TaitoRoadRomSize) bprintf(PRINT_IMPORTANT, _T("Road Rom Length %08X, (%i roms)\n"), TaitoRoadRomSize, TaitoRoadRomNum);
		if (TaitoSpriteMapRomSize) bprintf(PRINT_IMPORTANT, _T("Sprite Map Rom Length %08X, (%i roms)\n"), TaitoSpriteMapRomSize, TaitoSpriteMapRomNum);
		if (TaitoYM2610ARomSize) bprintf(PRINT_IMPORTANT, _T("YM2610 Samples Rom Length %08X, (%i roms)\n"), TaitoYM2610ARomSize, TaitoYM2610ARomNum);
		if (TaitoYM2610BRomSize) bprintf(PRINT_IMPORTANT, _T("YM2610 Delta-T Rom Length %08X, (%i roms)\n"), TaitoYM2610BRomSize, TaitoYM2610BRomNum);
		if (TaitoMSM5205RomSize) bprintf(PRINT_IMPORTANT, _T("MSM5205 Rom Length %08X, (%i roms)\n"), TaitoMSM5205RomSize, TaitoMSM5205RomNum);
		if (TaitoCharPivotRomSize) bprintf(PRINT_IMPORTANT, _T("Pivot Char Rom Length %08X, (%i roms, 0x%06X tiles)\n"), TaitoCharPivotRomSize, TaitoCharPivotRomNum, TaitoNumCharPivot);
		if (TaitoMSM6295RomSize) bprintf(PRINT_IMPORTANT, _T("MSM6295 Rom Length %08X, (%i roms)\n"), TaitoMSM6295RomSize, TaitoMSM6295RomNum);
		if (TaitoES5505RomSize) bprintf(PRINT_IMPORTANT, _T("ES5505 Rom Length %08X, (%i roms)\n"), TaitoES5505RomSize, TaitoES5505RomNum);
		if (TaitoDefaultEEPromSize) bprintf(PRINT_IMPORTANT, _T("Default EEPROM Length %08X, (%i roms)\n"), TaitoDefaultEEPromSize, TaitoDefaultEEPromNum);
#endif
	}
	
	if (bLoad) {
		INT32 Offset = 0;
		
		i = 0;
		while (i < Taito68KRom1Num) {
			BurnDrvGetRomInfo(&ri, i + 0);
			
			if ((ri.nType & 0xff) == TAITO_68KROM1_BYTESWAP) {
				nRet = BurnLoadRom(Taito68KRom1 + Offset + 1, i + 0, 2); if (nRet) return 1;
				nRet = BurnLoadRom(Taito68KRom1 + Offset + 0, i + 1, 2); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 1);
				Offset += ri.nLen;
				
				i += 2;
			}
			
			if ((ri.nType & 0xff) == TAITO_68KROM1_BYTESWAP32) {
				nRet = BurnLoadRom(Taito68KRom1 + Offset + 1, i + 0, 4); if (nRet) return 1;
				nRet = BurnLoadRom(Taito68KRom1 + Offset + 0, i + 1, 4); if (nRet) return 1;
				nRet = BurnLoadRom(Taito68KRom1 + Offset + 3, i + 2, 4); if (nRet) return 1;
				nRet = BurnLoadRom(Taito68KRom1 + Offset + 2, i + 3, 4); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 1);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 2);
				Offset += ri.nLen;
				BurnDrvGetRomInfo(&ri, i + 3);
				Offset += ri.nLen;
				
				i += 4;
			}
			
			if ((ri.nType & 0xff) == TAITO_68KROM1_BYTESWAP_JUMPING) {
				nRet = BurnLoadRom(Taito68KRom1 + Offset + 0, i + 0, 2); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i + 0);
				Offset += ri.nLen;
			
				i++;
			}
			
			if ((ri.nType & 0xff) == TAITO_68KROM1) {
				nRet = BurnLoadRom(Taito68KRom1 + Offset, i, 1); if (nRet) return 1;
			
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
				
				i++;
			}
		}
		
		if (Taito68KRom2Size) {
			Offset = 0;
			i = Taito68KRom1Num;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_68KROM2_BYTESWAP) {
					nRet = BurnLoadRom(Taito68KRom2 + Offset + 1, i + 0, 2); if (nRet) return 1;
					nRet = BurnLoadRom(Taito68KRom2 + Offset + 0, i + 1, 2); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
				
					i += 2;
				}
				
				if ((ri.nType & 0xff) == TAITO_68KROM2) {
					nRet = BurnLoadRom(Taito68KRom2 + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
			}
		}
		
		if (Taito68KRom3Size) {
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_68KROM3_BYTESWAP) {
					nRet = BurnLoadRom(Taito68KRom3 + Offset + 1, i + 0, 2); if (nRet) return 1;
					nRet = BurnLoadRom(Taito68KRom3 + Offset + 0, i + 1, 2); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
				
					i += 2;
				}
				
				if ((ri.nType & 0xff) == TAITO_68KROM3) {
					nRet = BurnLoadRom(Taito68KRom3 + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
			}
		}
		
		if (TaitoZ80Rom1Size) {
			Offset = 0;
			for (i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num; i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num; i++) {
				BurnLoadRom(TaitoZ80Rom1 + Offset, i, 1);
				
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
		
		if (TaitoZ80Rom2Size) {
			Offset = 0;
			for (i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num; i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num; i++) {
				BurnLoadRom(TaitoZ80Rom2 + Offset, i, 1);
				
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
		
		if (TaitoCharRomSize) {
			UINT8 *TempRom = (UINT8*)BurnMalloc(TaitoCharRomSize);
			memset(TempRom, 0, TaitoCharRomSize);
			
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_CHARS) {
					nRet = BurnLoadRom(TempRom + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
				
				if ((ri.nType & 0xff) == TAITO_CHARS_BYTESWAP) {
					nRet = BurnLoadRom(TempRom + Offset + 0, i + 0, 2); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 1, i + 1, 2); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
				
					i += 2;
				}
			}
			
			if (TaitoNumChar) {
				GfxDecode(TaitoNumChar, TaitoCharNumPlanes, TaitoCharWidth, TaitoCharHeight, TaitoCharPlaneOffsets, TaitoCharXOffsets, TaitoCharYOffsets, TaitoCharModulo, TempRom, TaitoChars);
			} else {
				memcpy (TaitoChars, TempRom, Offset);
			}

			BurnFree(TempRom);
		}
		
		if (TaitoCharBRomSize) {
			UINT8 *TempRom = (UINT8*)BurnMalloc(TaitoCharBRomSize);
			memset(TempRom, 0, TaitoCharBRomSize);
			
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_CHARSB) {
					nRet = BurnLoadRom(TempRom + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
				
				if ((ri.nType & 0xff) == TAITO_CHARSB_BYTESWAP) {
					nRet = BurnLoadRom(TempRom + Offset + 0, i + 0, 2); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 1, i + 1, 2); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
				
					i += 2;
				}
			}
			
			GfxDecode(TaitoNumCharB, TaitoCharBNumPlanes, TaitoCharBWidth, TaitoCharBHeight, TaitoCharBPlaneOffsets, TaitoCharBXOffsets, TaitoCharBYOffsets, TaitoCharBModulo, TempRom, TaitoCharsB);
			
			BurnFree(TempRom);
		}
		
		if (TaitoSpriteARomSize) {
			UINT8 *TempRom = (UINT8*)BurnMalloc(TaitoSpriteARomSize);
			memset(TempRom, 0, TaitoSpriteARomSize);
			
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_SPRITESA) {
					nRet = BurnLoadRom(TempRom + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
				
				if ((ri.nType & 0xff) == TAITO_SPRITESA_BYTESWAP) {
					nRet = BurnLoadRom(TempRom + Offset + 0, i + 0, 2); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 1, i + 1, 2); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
				
					i += 2;
				}
				
				if ((ri.nType & 0xff) == TAITO_SPRITESA_BYTESWAP32) {
					nRet = BurnLoadRom(TempRom + Offset + 0, i + 0, 4); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 1, i + 1, 4); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 2, i + 2, 4); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 3, i + 3, 4); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 2);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 3);
					Offset += ri.nLen;
				
					i += 4;
				}
				
				if ((ri.nType & 0xff) == TAITO_SPRITESA_TOPSPEED) {
					nRet = BurnLoadRom(TempRom + 0x000003, i +  0, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x100003, i +  1, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x000007, i +  2, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x100007, i +  3, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x000002, i +  4, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x100002, i +  5, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x000006, i +  6, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x100006, i +  7, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x000001, i +  8, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x100001, i +  9, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x000005, i + 10, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x100005, i + 11, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x000000, i + 12, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x100000, i + 13, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x000004, i + 14, 8); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + 0x100004, i + 15, 8); if (nRet) return 1;
				
					i += 16;
				}
			}
			
			if (TaitoSpriteAInvertRom) {
				for (UINT32 j = 0; j < TaitoSpriteARomSize; j++) {
					TempRom[j] ^= 0xff;
				}
			}

			if (TaitoNumSpriteA) {
				GfxDecode(TaitoNumSpriteA, TaitoSpriteANumPlanes, TaitoSpriteAWidth, TaitoSpriteAHeight, TaitoSpriteAPlaneOffsets, TaitoSpriteAXOffsets, TaitoSpriteAYOffsets, TaitoSpriteAModulo, TempRom, TaitoSpritesA);
			} else {
				memcpy (TaitoSpritesA, TempRom, Offset);
			}

			BurnFree(TempRom);
		}
		
		if (TaitoSpriteBRomSize) {
			UINT8 *TempRom = (UINT8*)BurnMalloc(TaitoSpriteBRomSize);
			memset(TempRom, 0, TaitoSpriteBRomSize);
			
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_SPRITESB) {
					nRet = BurnLoadRom(TempRom + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
				
				if ((ri.nType & 0xff) == TAITO_SPRITESB_BYTESWAP) {
					nRet = BurnLoadRom(TempRom + Offset + 0, i + 0, 2); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 1, i + 1, 2); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
				
					i += 2;
				}
				
				if ((ri.nType & 0xff) == TAITO_SPRITESB_BYTESWAP32) {
					nRet = BurnLoadRom(TempRom + Offset + 0, i + 0, 4); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 1, i + 1, 4); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 2, i + 2, 4); if (nRet) return 1;
					nRet = BurnLoadRom(TempRom + Offset + 3, i + 3, 4); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 2);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 3);
					Offset += ri.nLen;
				
					i += 4;
				}
			}
			
			GfxDecode(TaitoNumSpriteB, TaitoSpriteBNumPlanes, TaitoSpriteBWidth, TaitoSpriteBHeight, TaitoSpriteBPlaneOffsets, TaitoSpriteBXOffsets, TaitoSpriteBYOffsets, TaitoSpriteBModulo, TempRom, TaitoSpritesB);
			
			BurnFree(TempRom);
		}
		
		if (TaitoRoadRomSize) {
			Offset = 0;
			for (i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum; i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum; i++) {
				BurnLoadRom(TC0150RODRom + Offset, i, 1);
				
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
		
		if (TaitoSpriteMapRomSize) {
			Offset = 0;
			for (i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum; i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum; i++) {
				BurnLoadRom(TaitoSpriteMapRom + Offset, i, 1);
				
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
		
		if (TaitoYM2610ARomSize) {
			Offset = 0;
			for (i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum; i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum; i++) {
				BurnLoadRom(TaitoYM2610ARom + Offset, i, 1);
				
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
		
		if (TaitoYM2610BRomSize) {
			Offset = 0;
			for (i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum; i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum; i++) {
				BurnLoadRom(TaitoYM2610BRom + Offset, i, 1);
				
				BurnDrvGetRomInfo(&ri, i);
				Offset += ri.nLen;
			}
		}
		
		if (TaitoMSM5205RomSize) {
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_MSM5205) {
					nRet = BurnLoadRom(TaitoMSM5205Rom + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
				
				if ((ri.nType & 0xff) == TAITO_MSM5205_BYTESWAP) {
					nRet = BurnLoadRom(TaitoMSM5205Rom + Offset + 0, i + 0, 2); if (nRet) return 1;
					nRet = BurnLoadRom(TaitoMSM5205Rom + Offset + 1, i + 1, 2); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i + 0);
					Offset += ri.nLen;
					BurnDrvGetRomInfo(&ri, i + 1);
					Offset += ri.nLen;
				
					i += 2;
				}
			}
		}
		
		if (TaitoCharPivotRomSize) {
			UINT8 *TempRom = (UINT8*)BurnMalloc(TaitoCharPivotRomSize);
			memset(TempRom, 0, TaitoCharPivotRomSize);
			
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum + TaitoCharPivotRomNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_CHARS_PIVOT) {
					nRet = BurnLoadRom(TempRom + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
			}
			
			GfxDecode(TaitoNumCharPivot, TaitoCharPivotNumPlanes, TaitoCharPivotWidth, TaitoCharPivotHeight, TaitoCharPivotPlaneOffsets, TaitoCharPivotXOffsets, TaitoCharPivotYOffsets, TaitoCharPivotModulo, TempRom, TaitoCharsPivot);
			
			BurnFree(TempRom);
		}
		
		if (TaitoMSM6295RomSize) {
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum + TaitoCharPivotRomNum;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum + TaitoCharPivotRomNum + TaitoMSM6295RomNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_MSM6295) {
					nRet = BurnLoadRom(TaitoMSM6295Rom + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
			}
		}
		
		if (TaitoES5505RomSize) {
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum + TaitoCharPivotRomNum + TaitoMSM6295RomNum;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum + TaitoCharPivotRomNum + TaitoMSM6295RomNum + TaitoES5505RomNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_ES5505) {
					nRet = BurnLoadRom(TaitoES5505Rom + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
				
				if ((ri.nType & 0xff) == TAITO_ES5505_BYTESWAP) {
					nRet = BurnLoadRom(TaitoES5505Rom + (Offset * 2), i, 2); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
			}
		}
		
		if (TaitoDefaultEEPromSize) {
			Offset = 0;
			i = Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum + TaitoCharPivotRomNum + TaitoMSM6295RomNum + TaitoES5505RomNum;
			
			while (i < Taito68KRom1Num + Taito68KRom2Num + Taito68KRom3Num + TaitoZ80Rom1Num + TaitoZ80Rom2Num + TaitoCharRomNum + TaitoCharBRomNum + TaitoSpriteARomNum + TaitoSpriteBRomNum + TaitoRoadRomNum + TaitoSpriteMapRomNum + TaitoYM2610ARomNum + TaitoYM2610BRomNum + TaitoMSM5205RomNum + TaitoCharPivotRomNum + TaitoMSM6295RomNum + TaitoES5505RomNum + TaitoDefaultEEPromNum) {
				BurnDrvGetRomInfo(&ri, i + 0);
				
				if ((ri.nType & 0xff) == TAITO_DEFAULT_EEPROM) {
					nRet = BurnLoadRom(TaitoDefaultEEProm + Offset, i, 1); if (nRet) return 1;
				
					BurnDrvGetRomInfo(&ri, i);
					Offset += ri.nLen;
				
					i++;
				}
			}
		}
	}
	
	return 0;
}

INT32 TaitoExit()
{
	INT32 i;

	if (TaitoNum68Ks) SekExit();
	if (TaitoNumZ80s) ZetExit();
	if (TaitoNumYM2610) BurnYM2610Exit();
	if (TaitoNumYM2151) BurnYM2151Exit();
	if (TaitoNumYM2203) BurnYM2203Exit();
	if (TaitoNumMSM5205) MSM5205Exit();
	for (i = 0; i < TaitoNumMSM6295; i++) {
		MSM6295Exit(i);
	}
	if (TaitoNumEEPROM) EEPROMExit();
	
	TaitoICExit();
	
	GenericTilesExit();
	if (nBurnGunNumPlayers) BurnGunExit();
	
	BurnFree(TaitoMem);

	Taito68KRom1Num = 0;
	Taito68KRom2Num = 0;
	Taito68KRom3Num = 0;
	TaitoZ80Rom1Num = 0;
	TaitoZ80Rom2Num = 0;
	TaitoCharRomNum = 0;
	TaitoCharBRomNum = 0;
	TaitoCharPivotRomNum = 0;
	TaitoSpriteARomNum = 0;
	TaitoSpriteBRomNum = 0;
	TaitoRoadRomNum = 0;
	TaitoSpriteMapRomNum = 0;
	TaitoYM2610ARomNum = 0;
	TaitoYM2610BRomNum = 0;
	TaitoMSM5205RomNum = 0;
	TaitoMSM6295RomNum = 0;
	TaitoES5505RomNum = 0;
	TaitoDefaultEEPromNum = 0;

	Taito68KRom1Size = 0;
	Taito68KRom2Size = 0;
	Taito68KRom3Size = 0;
	TaitoZ80Rom1Size = 0;
	TaitoZ80Rom2Size = 0;
	TaitoCharRomSize = 0;
	TaitoCharBRomSize = 0;
	TaitoCharPivotRomSize = 0;
	TaitoSpriteARomSize = 0;
	TaitoSpriteBRomSize = 0;
	TaitoRoadRomSize = 0;
	TaitoSpriteMapRomSize = 0;
	TaitoYM2610ARomSize = 0;
	TaitoYM2610BRomSize = 0;
	TaitoMSM5205RomSize = 0;
	TaitoMSM6295RomSize = 0;
	TaitoES5505RomSize = 0;
	TaitoDefaultEEPromSize = 0;
	
	TaitoCharModulo = 0;
	TaitoCharNumPlanes = 0;
	TaitoCharWidth = 0;
	TaitoCharHeight = 0;
	TaitoNumChar = 0;
	TaitoCharPlaneOffsets = NULL;
	TaitoCharXOffsets = NULL;
	TaitoCharYOffsets = NULL;
	
	TaitoCharBModulo = 0;
	TaitoCharBNumPlanes = 0;
	TaitoCharBWidth = 0;
	TaitoCharBHeight = 0;
	TaitoNumCharB = 0;
	TaitoCharBPlaneOffsets = NULL;
	TaitoCharBXOffsets = NULL;
	TaitoCharBYOffsets = NULL;
	
	TaitoCharPivotModulo = 0;
	TaitoCharPivotNumPlanes = 0;
	TaitoCharPivotWidth = 0;
	TaitoCharPivotHeight = 0;
	TaitoNumCharPivot = 0;
	TaitoCharPivotPlaneOffsets = NULL;
	TaitoCharPivotXOffsets = NULL;
	TaitoCharPivotYOffsets = NULL;

	TaitoSpriteAModulo = 0;
	TaitoSpriteANumPlanes = 0;
	TaitoSpriteAWidth = 0;
	TaitoSpriteAHeight = 0;
	TaitoNumSpriteA = 0;
	TaitoSpriteAPlaneOffsets = NULL;
	TaitoSpriteAXOffsets = NULL;
	TaitoSpriteAYOffsets = NULL;
	TaitoSpriteAInvertRom = 0;
	
	TaitoSpriteBModulo = 0;
	TaitoSpriteBNumPlanes = 0;
	TaitoSpriteBWidth = 0;
	TaitoSpriteBHeight = 0;
	TaitoNumSpriteB = 0;
	TaitoSpriteBPlaneOffsets = NULL;
	TaitoSpriteBXOffsets = NULL;
	TaitoSpriteBYOffsets = NULL;
	
	TaitoZ80Bank = 0;
	TaitoSoundLatch = 0;
	TaitoRoadPalBank = 0;
	TaitoCpuACtrl = 0;
	
	TaitoXOffset = 0;
	TaitoYOffset = 0;
	TaitoIrqLine = 0;
	TaitoFrameInterleave = 0;
	TaitoFlipScreenX = 0;
	
	TaitoNum68Ks = 0;
	TaitoNumZ80s = 0;
	TaitoNumYM2610 = 0;
	TaitoNumYM2151 = 0;
	TaitoNumYM2203 = 0;
	TaitoNumMSM5205 = 0;
	TaitoNumMSM6295 = 0;
	TaitoNumES5505 = 0;
	TaitoNumEEPROM = 0;
	
	TaitoDrawFunction = NULL;
	TaitoMakeInputsFunction = NULL;
	TaitoResetFunction = NULL;
	
	return 0;
}
