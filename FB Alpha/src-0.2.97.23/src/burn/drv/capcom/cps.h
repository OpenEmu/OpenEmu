// CPS ----------------------------------
#include "burnint.h"
#include "sek.h"
#include "zet.h"

#include "msm6295.h"
#include "eeprom.h"
#include "timer.h"

// Maximum number of beam-synchronized interrupts to check
#define MAX_RASTER 10

extern UINT32 CpsMProt[4];									// Mprot changes
extern UINT32 CpsBID[3];										// Board ID changes

// cps.cpp
extern INT32 Cps;														// 1 = CPS1, 2 = CPS2, 3 = CPS CHanger
extern INT32 Cps1Qs;
extern INT32 Cps1Pic;
extern INT32 nCPS68KClockspeed;
extern INT32 nCpsCycles;												// Cycles per frame
extern INT32 nCpsZ80Cycles;
extern UINT8 *CpsGfx;  extern UINT32 nCpsGfxLen;		// All the graphics
extern UINT8 *CpsRom;  extern UINT32 nCpsRomLen;		// Program Rom (as in rom)
extern UINT8 *CpsCode; extern UINT32 nCpsCodeLen;		// Program Rom (decrypted)
extern UINT8 *CpsZRom; extern UINT32 nCpsZRomLen;		// Z80 Roms
extern          INT8 *CpsQSam; extern UINT32 nCpsQSamLen;		// QSound Sample Roms
extern UINT8 *CpsAd;   extern UINT32 nCpsAdLen;		// ADPCM Data
extern UINT32 nCpsGfxScroll[4];								// Offset to Scroll tiles
extern UINT32 nCpsGfxMask;									// Address mask
extern UINT8* CpsStar;
INT32 CpsInit();
INT32 Cps2Init();
INT32 CpsExit();
INT32 CpsLoadTiles(UINT8 *Tile,INT32 nStart);
INT32 CpsLoadTilesByte(UINT8 *Tile,INT32 nStart);
INT32 CpsLoadTilesForgottn(INT32 nStart);
INT32 CpsLoadTilesForgottnu(INT32 nStart);
INT32 CpsLoadTilesPang(UINT8 *Tile,INT32 nStart);
INT32 CpsLoadTilesHack160(UINT8 *Tile,INT32 nStart);
INT32 CpsLoadTilesBootleg(UINT8 *Tile, INT32 nStart);
INT32 CpsLoadTilesCaptcomb(UINT8 *Tile, INT32 nStart);
INT32 CpsLoadTilesPunipic2(UINT8 *Tile, INT32 nStart);
INT32 CpsLoadTilesSf2ebbl(UINT8 *Tile, INT32 nStart);
INT32 CpsLoadStars(UINT8 *pStar, INT32 nStart);
INT32 CpsLoadStarsByte(UINT8 *pStar, INT32 nStart);
INT32 CpsLoadStarsForgottnAlt(UINT8 *pStar, INT32 nStart);
INT32 Cps2LoadTiles(UINT8 *Tile,INT32 nStart);
INT32 Cps2LoadTilesSIM(UINT8 *Tile,INT32 nStart);

// cps_config.h
#define CPS_B_01		0
#define CPS_B_02		1
#define CPS_B_03		2
#define CPS_B_04		3
#define CPS_B_05		4
#define CPS_B_11		5
#define CPS_B_12		6
#define CPS_B_13		7
#define CPS_B_14		8
#define CPS_B_15		9
#define CPS_B_16		10
#define CPS_B_17		11
#define CPS_B_18		12
#define CPS_B_21_DEF		13
#define CPS_B_21_BT1		14
#define CPS_B_21_BT2		15
#define CPS_B_21_BT3		16
#define CPS_B_21_BT4		17
#define CPS_B_21_BT5		18
#define CPS_B_21_BT6		19
#define CPS_B_21_BT7		20
#define CPS_B_21_QS1		21
#define CPS_B_21_QS2		22
#define CPS_B_21_QS3		23
#define CPS_B_21_QS4		24
#define CPS_B_21_QS5		25
#define HACK_B_1		26
#define HACK_B_2		27
#define HACK_B_3		28
#define HACK_B_4		29

#define GFXTYPE_SPRITES		(1<<0)
#define GFXTYPE_SCROLL1		(1<<1)
#define GFXTYPE_SCROLL2		(1<<2)
#define GFXTYPE_SCROLL3		(1<<3)
#define GFXTYPE_STARS		(1<<4)

#define mapper_LWCHR		0
#define mapper_LW621		1
#define mapper_DM620		2
#define mapper_ST24M1		3
#define mapper_DM22A		4
#define mapper_DAM63B		5
#define mapper_ST22B		6
#define mapper_TK22B		7
#define mapper_WL24B		8
#define mapper_S224B		9
#define mapper_YI24B		10
#define mapper_AR24B		11
#define mapper_AR22B		12
#define mapper_O224B		13
#define mapper_MS24B		14
#define mapper_CK24B		15
#define mapper_NM24B		16
#define mapper_CA24B		17
#define mapper_CA22B		18
#define mapper_STF29		19
#define mapper_RT24B		20
#define mapper_RT22B		21
#define mapper_KD29B		22
#define mapper_CC63B		23
#define mapper_KR63B		24
#define mapper_S9263B		25
#define mapper_VA63B		26
#define mapper_VA22B		27
#define mapper_Q522B		28
#define mapper_TK263B		29
#define mapper_CD63B		30
#define mapper_PS63B		31
#define mapper_MB63B		32
#define mapper_QD22B		33
#define mapper_QD63B		34
#define mapper_qtono2		35
#define mapper_RCM63B		36
#define mapper_PKB10B		37
#define mapper_pang3		38
#define mapper_sfzch		39
#define mapper_cps2		40
#define mapper_frog		41
extern void SetGfxMapper(INT32 MapperId);
extern INT32 GfxRomBankMapper(INT32 Type, INT32 Code);
extern void SetCpsBId(INT32 CpsBId, INT32 bStars);

// cps_pal.cpp
extern UINT32* CpsPal;										// Hicolor version of palette
extern INT32 nCpsPalCtrlReg;
extern INT32 bCpsUpdatePalEveryFrame;
INT32 CpsPalInit();
INT32 CpsPalExit();
INT32 CpsPalUpdate(UINT8 *pNewPal);

// cps_mem.cpp
extern UINT8 *CpsRam90;
extern UINT8 *CpsZRamC0,*CpsZRamF0;
extern UINT8 *CpsSavePal;
extern UINT8 *CpsRam708,*CpsReg,*CpsFrg;
extern UINT8 *CpsSaveReg[MAX_RASTER + 1];
extern UINT8 *CpsSaveFrg[MAX_RASTER + 1];
extern UINT8 *CpsRamFF;
void CpsMapObjectBanks(INT32 nBank);
INT32 CpsMemInit();
INT32 CpsMemExit();
INT32 CpsAreaScan(INT32 nAction,INT32 *pnMin);

// cps_run.cpp
extern UINT8 CpsReset;
extern UINT8 Cpi01A, Cpi01C, Cpi01E;
extern INT32 nIrqLine50, nIrqLine52;								// The scanlines at which the interrupts are triggered
extern INT32 nCpsNumScanlines;
extern INT32 CpsDrawSpritesInReverse;
INT32 CpsRunInit();
INT32 CpsRunExit();
INT32 Cps1Frame();
INT32 Cps2Frame();

inline static UINT8* CpsFindGfxRam(INT32 nAddr,INT32 nLen)
{
  nAddr&=0xffffff; // 24-bit bus
  if (nAddr>=0x900000 && nAddr+nLen<=0x930000) return CpsRam90+nAddr-0x900000;
  return NULL;
}

inline static void GetPalette(INT32 nStart, INT32 nCount)
{
	// Update Palette (Ghouls points to the wrong place on boot up I think)
	INT32 nPal = (BURN_ENDIAN_SWAP_INT16(*((UINT16*)(CpsReg + 0x0A))) << 8) & 0xFFFC00;
	
	UINT8* Find = CpsFindGfxRam(nPal, 0xc00 << 1);
	if (Find) {
		memcpy(CpsSavePal + (nStart << 10), Find + (nStart << 10), nCount << 10);
	}
}

// cps_rw.cpp
// Treble Winner - Added INP(1FD) for sf2ue
#define CPSINPSET INP(000) INP(001) INP(006) INP(007) INP(008) INP(010) INP(011) INP(012) INP(018) INP(019) INP(020) INP(021) INP(029) INP(176) INP(177) INP(179) INP(186) INP(1fd)

// prototype for input bits
#define INP(nnn) extern UINT8 CpsInp##nnn[8];
CPSINPSET
#undef  INP

#define INP(nnn) extern UINT8 Inp##nnn;
CPSINPSET
#undef  INP

#define CPSINPEX INP(c000) INP(c001) INP(c002) INP(c003)

#define INP(nnnn) extern UINT8 CpsInp##nnnn[8];
CPSINPEX
#undef  INP

// For the Forgotten Worlds analog controls
extern UINT16 CpsInp055, CpsInp05d;
extern UINT16 CpsInpPaddle1, CpsInpPaddle2;

extern INT32 PangEEP;
extern INT32 Forgottn;
extern INT32 Cps1QsHack;
extern INT32 Kodh;
extern INT32 Cawingb;
extern INT32 Wofh;
extern INT32 Sf2thndr;
extern INT32 Pzloop2;
extern INT32 Ssf2tb;
extern INT32 Dinopic;
extern INT32 Dinohunt;
extern INT32 Port6SoundWrite;

extern UINT8* CpsEncZRom;

INT32 CpsRwInit();
INT32 CpsRwExit();
INT32 CpsRwGetInp();
UINT8 __fastcall CpsReadByte(UINT32 a);
void __fastcall CpsWriteByte(UINT32 a, UINT8 d);
UINT16 __fastcall CpsReadWord(UINT32 a);
void __fastcall CpsWriteWord(UINT32 a, UINT16 d);

// cps_draw.cpp
extern UINT8 CpsRecalcPal;				// Flag - If it is 1, recalc the whole palette
extern INT32 nCpsLcReg;							// Address of layer controller register
extern INT32 CpsLayEn[6];							// bits for layer enable
extern INT32 nStartline, nEndline;				// specify the vertical slice of the screen to render
extern INT32 nRasterline[MAX_RASTER + 2];			// The lines at which an interrupt occurs
extern INT32 MaskAddr[4];
extern INT32 CpsLayer1XOffs;
extern INT32 CpsLayer2XOffs;
extern INT32 CpsLayer3XOffs;
extern INT32 CpsLayer1YOffs;
extern INT32 CpsLayer2YOffs;
extern INT32 CpsLayer3YOffs;
void DrawFnInit();
INT32  CpsDraw();
INT32  CpsRedraw();

INT32 QsndInit();
void QsndExit();
void QsndReset();
void QsndNewFrame();
void QsndEndFrame();
void QsndSyncZ80();
INT32 QsndScan(INT32 nAction);

// qs_z.cpp
INT32 QsndZInit();
INT32 QsndZExit();
INT32 QsndZScan(INT32 nAction);

// qs_c.cpp
INT32 QscInit(INT32 nRate, INT32 nVolumeShift);
void QscReset();
void QscExit();
INT32 QscScan(INT32 nAction);
void QscNewFrame();
void QscWrite(INT32 a, INT32 d);
INT32 QscUpdate(INT32 nEnd);

// cps_tile.cpp
extern UINT32* CpstPal;
extern UINT32 nCpstType; extern INT32 nCpstX,nCpstY;
extern UINT32 nCpstTile; extern INT32 nCpstFlip;
extern short* CpstRowShift;
extern UINT32 CpstPmsk; // Pixel mask

inline static void CpstSetPal(INT32 nPal)
{
	nPal <<= 4;
	nPal &= 0x7F0;
	CpstPal= CpsPal + nPal;
}

// ctv.cpp
extern INT32 nBgHi;
extern UINT16  ZValue;
extern UINT16 *ZBuf;
extern UINT16 *pZVal;
extern UINT32    nCtvRollX,nCtvRollY;
extern UINT8  *pCtvTile;					// Pointer to tile data
extern INT32             nCtvTileAdd;					// Amount to add after each tile line
extern UINT8  *pCtvLine;					// Pointer to output bitmap
typedef INT32 (*CtvDoFn)();
typedef INT32 (*CpstOneDoFn)();
extern CtvDoFn CtvDoX[0x20];
extern CtvDoFn CtvDoXM[0x20];
extern CtvDoFn CtvDoXB[0x20];
extern CpstOneDoFn CpstOneDoX[3];
extern CpstOneDoFn CpstOneObjDoX[2];
INT32 CtvReady();

// nCpstType constants
// To get size do (nCpstType & 24) + 8
#define CTT_FLIPX ( 1)
#define CTT_CARE  ( 2)
#define CTT_ROWS  ( 4)
#define CTT_8X8   ( 0)
#define CTT_16X16 ( 8)
#define CTT_32X32 (24)

// cps_obj.cpp
extern INT32 nCpsObjectBank;

extern UINT8 *BootlegSpriteRam;

extern INT32 Sf2Hack;

INT32  CpsObjInit();
INT32  CpsObjExit();
INT32  CpsObjGet();
void CpsObjDrawInit();
INT32  Cps1ObjDraw(INT32 nLevelFrom,INT32 nLevelTo);
INT32  Cps2ObjDraw(INT32 nLevelFrom,INT32 nLevelTo);

// cps_scr.cpp
#define SCROLL_2 0
#define SCROLL_3 1
extern INT32 Ghouls;
extern INT32 Mercs;
extern INT32 Sf2jc;
extern INT32 Ssf2t;
extern INT32 Qad;
extern INT32 Xmcota;

extern INT32 Scroll1TileMask;
extern INT32 Scroll2TileMask;
extern INT32 Scroll3TileMask;
INT32 Cps1Scr1Draw(UINT8 *Base,INT32 sx,INT32 sy);
INT32 Cps1Scr3Draw(UINT8 *Base,INT32 sx,INT32 sy);
INT32 Cps2Scr1Draw(UINT8 *Base,INT32 sx,INT32 sy);
INT32 Cps2Scr3Draw(UINT8 *Base,INT32 sx,INT32 sy);

// cpsr.cpp
extern UINT8 *CpsrBase;						// Tile data base
extern INT32 nCpsrScrX,nCpsrScrY;						// Basic scroll info
extern UINT16 *CpsrRows;					// Row scroll table, 0x400 words long
extern INT32 nCpsrRowStart;							// Start of row scroll (can wrap?)

// Information needed to draw a line
struct CpsrLineInfo {
	INT32 nStart;										// 0-0x3ff - where to start drawing tiles from
	INT32 nWidth;										// 0-0x400 - width of scroll shifts
													// e.g. for no rowscroll at all, nWidth=0
	INT32 nTileStart;									// Range of tiles which are visible onscreen
	INT32 nTileEnd;									// (e.g. 0x20 -> 0x50 , wraps around to 0x10)
	INT16 Rows[16];									// 16 row scroll values for this line
	INT32 nMaxLeft, nMaxRight;						// Maximum row shifts left and right
};
extern struct CpsrLineInfo CpsrLineInfo[15];
INT32 Cps1rPrepare();
INT32 Cps2rPrepare();

// cpsrd.cpp
INT32 Cps1rRender();
INT32 Cps2rRender();

// dc_input.cpp
extern struct BurnInputInfo CpsFsi[0x1B];

// ps.cpp
extern UINT8 PsndCode, PsndFade;			// Sound code/fade sent to the z80 program
INT32 PsndInit();
INT32 PsndExit();
void PsndNewFrame();
INT32 PsndSyncZ80(INT32 nCycles);
INT32 PsndScan(INT32 nAction);

// ps_z.cpp
INT32 PsndZInit();
INT32 PsndZExit();
INT32 PsndZScan(INT32 nAction);
extern INT32 Kodb;

// ps_m.cpp
extern INT32 bPsmOkay;								// 1 if the module is okay
INT32 PsmInit();
INT32 PsmExit();
void PsmNewFrame();
INT32 PsmUpdate(INT32 nEnd);

// kabuki.cpp
void wof_decode();
void dino_decode();
void punisher_decode();
void slammast_decode();

// cps2_crypt.cpp
void cps2_decrypt_game_data();
