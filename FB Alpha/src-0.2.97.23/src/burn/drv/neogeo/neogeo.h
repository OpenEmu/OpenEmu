#include "burnint.h"
#include "m68000_intf.h"
#include "z80_intf.h"

// Uncomment the following line to make the display the full 320 pixels wide
#define NEO_DISPLAY_OVERSCAN

#if defined NEO_DISPLAY_OVERSCAN
 extern INT32 nNeoScreenWidth;// = 320;
#else
 extern INT32 nNeoScreenWidth;// = 304;
#endif

#define MAX_SLOT		(8)

#define NEO_SYS_CD		(1 << 4)
#define NEO_SYS_PCB		(1 << 3)
#define NEO_SYS_CART	(1 << 2)
#define NEO_SYS_AES		(1 << 1)
#define NEO_SYS_MVS		(1 << 0)

struct NeoGameInfo {
	INT32 nCodeOffset; INT32 nCodeNum;
	INT32 nTextOffset;
	INT32 nSpriteOffset; INT32 nSpriteNum;
	INT32 nSoundOffset;
	INT32 nADPCMOffset; INT32 nADPCMANum; INT32 nADPCMBNum;
	INT32 nNeoSRAMProtection;
};

struct NEO_CALLBACK {
	void (*pInitialise)();
	void (*pInstallHandlers)();
	void (*pRemoveHandlers)();
	void (*pBankswitch)();
	INT32 (*pScan)(INT32, INT32*);
};

extern NEO_CALLBACK  NeoCallback[MAX_SLOT];
extern NEO_CALLBACK* NeoCallbackActive;

// neogeo.cpp
void NeoClearScreen();
INT32 NeoLoadCode(INT32 nOffset, INT32 nNum, UINT8* pDest);
INT32 NeoLoadSprites(INT32 nOffset, INT32 nNum, UINT8* pDest, UINT32 nSpriteSize);
INT32 NeoLoadADPCM(INT32 nOffset, INT32 nNum, UINT8* pDest);

void NeoDecodeSprites(UINT8* pDest, INT32 nSize);
void NeoDecodeSpritesCD(UINT8* pData, UINT8* pDest, INT32 nSize);

// neo_run.cpp
extern UINT8* NeoGraphicsRAM;

extern UINT8 nNeoNumSlots;
extern UINT32 nNeoActiveSlot;

extern UINT8 NeoButton1[];
extern UINT8 NeoButton2[];
extern UINT8 NeoButton3[];
extern UINT8 NeoButton4[];
extern UINT8 NeoJoy1[];
extern UINT8 NeoJoy2[];
extern UINT8 NeoJoy3[];
extern UINT8 NeoJoy4[];
extern UINT16 NeoAxis[];
extern UINT8 NeoInput[];
extern UINT8 NeoDiag[];
extern UINT8 NeoDebugDip[];
extern UINT8 NeoReset, NeoSystem;

extern UINT8* Neo68KROMActive;
extern UINT8* NeoVectorActive;
extern UINT8* NeoZ80ROMActive;

extern UINT8* YM2610ADPCMAROM[MAX_SLOT];
extern UINT8* Neo68KFix[MAX_SLOT];

extern UINT32 nNeo68KROMBank;

extern UINT8 *NeoSpriteRAM, *NeoTextRAM;

extern bool bNeoEnableGraphics;

extern bool bDisableNeoWatchdog;

INT32 NeoInit();
INT32 NeoCDInit();
INT32 NeoExit();
INT32 NeoScan(INT32 nAction, INT32* pnMin);
INT32 NeoRender();
INT32 NeoFrame();

void NeoMapBank();
void NeoMap68KFix();
void NeoUpdateVector();

// neo_palette.cpp
extern UINT8* NeoPalSrc[2];
extern INT32 nNeoPaletteBank;
extern UINT32* NeoPalette;

extern UINT8 NeoRecalcPalette;

INT32 NeoInitPalette();
void NeoExitPalette();
INT32 NeoUpdatePalette();
void NeoSetPalette();

void __fastcall NeoPalWriteByte(UINT32 nAddress, UINT8 byteValue);
void __fastcall NeoPalWriteWord(UINT32 nAddress, UINT16 wordValue);

// neo_text.cpp
extern UINT8* NeoTextROMBIOS;
extern UINT8* NeoTextROM[MAX_SLOT];
extern INT32 nNeoTextROMSize[MAX_SLOT];
extern bool bBIOSTextROMEnabled;

INT32 NeoInitText(INT32 nSlot);
void NeoSetTextSlot(INT32 nSlot);
void NeoExitText(INT32 nSlot);
INT32 NeoRenderText();

void NeoDecodeTextBIOS(INT32 nOffset, const INT32 nSize, UINT8* pData);
void NeoDecodeText(INT32 nOffset, const INT32 nSize, UINT8* pData, UINT8* pDest);
void NeoUpdateTextOne(INT32 nOffset, const UINT8 byteValue);
void NeoUpdateText(INT32 nOffset, const INT32 nSize, UINT8* pData, UINT8* pDest);

// neo_sprite.cpp
extern UINT8* NeoSpriteROM[MAX_SLOT];
extern UINT8* NeoZoomROM;

extern INT32 nNeoSpriteFrame;
extern UINT32 nNeoTileMask[MAX_SLOT];
extern INT32 nNeoMaxTile[MAX_SLOT];

extern INT32 nSliceStart, nSliceEnd, nSliceSize;

void NeoUpdateSprites(INT32 nOffset, INT32 nSize);
void NeoSetSpriteSlot(INT32 nSlot);
INT32 NeoInitSprites(INT32 nSlot);
void NeoExitSprites(INT32 nSlot);
INT32 NeoRenderSprites();

// neo_decrypt.cpp
extern UINT8 nNeoProtectionXor;

void NeoCMC42Init();
void NeoCMC50Init();
void NeoCMCDecrypt(INT32 extra_xor, UINT8* rom, UINT8* buf, INT32 offset, INT32 block_size, INT32 rom_size);
void NeoCMCExtractSData(UINT8* rom, UINT8* sdata, INT32 rom_size, INT32 sdata_size);

void neogeo_cmc50_m1_decrypt();

// neo_upd4990a.cpp
void uPD4990AExit();
void uPD499ASetTicks(UINT32 nTicksPerSecond);
INT32 uPD4990AInit(UINT32 nTicksPerSecond);
void uPD4990AScan(INT32 nAction, INT32* pnMin);
void uPD4990AUpdate(UINT32 nTicks);
void uPD4990AWrite(UINT8 CLK, UINT8 STB, UINT8 DATA);
UINT8 uPD4990ARead(UINT32 nTicks);

// d_neogeo.cpp
void kf2k3pcb_bios_decode();
