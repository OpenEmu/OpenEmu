// konamiic.cpp
//---------------------------------------------------------------------------------------------------------------
extern UINT32 KonamiIC_K051960InUse;
extern UINT32 KonamiIC_K052109InUse;
extern UINT32 KonamiIC_K051316InUse;
extern UINT32 KonamiIC_K053245InUse;
extern UINT32 KonamiIC_K053247InUse;
extern UINT32 KonamiIC_K053936InUse;

extern UINT16 *konami_temp_screen;
void KonamiBlendCopy(UINT32 *palette /* 32-bit color */, UINT32 *drvpalette /* n-bit color */);

extern INT32 K05324xZRejection;
void K05324xSetZRejection(INT32 z);

void KonamiICReset();
void KonamiICExit();
void KonamiICScan(INT32 nAction);

void konami_rom_deinterleave_2(UINT8 *src, INT32 len);
void konami_rom_deinterleave_4(UINT8 *src, INT32 len);

void KonamiRecalcPal(UINT8 *src, UINT32 *dst, INT32 len);


void K052109_051960_w(INT32 offset, INT32 data);
UINT8 K052109_051960_r(INT32 offset);

// k051960.cpp
//---------------------------------------------------------------------------------------------------------------
extern INT32 K051960ReadRoms;
extern INT32 K052109_irq_enabled;
void K051960SpritesRender(UINT8 *pSrc, INT32 priority);
UINT8 K0519060FetchRomData(UINT32 Offset);
UINT8 K051960Read(UINT32 Offset);
void K051960Write(UINT32 Offset, UINT8 Data);
void K051960SetCallback(void (*Callback)(INT32 *Code, INT32 *Colour, INT32 *Priority, INT32 *Shadow));
void K051960SetSpriteOffset(INT32 x, INT32 y);
void K051960Reset();
void K051960Init(UINT8* pRomSrc, UINT32 RomMask);
void K051960Exit();
void K051960Scan(INT32 nAction);
void K051937Write(UINT32 Offset, UINT8 Data);
UINT8 K051937Read(UINT32 Offset);

// k052109.cpp
//---------------------------------------------------------------------------------------------------------------
extern INT32 K052109RMRDLine;
extern INT32 K051960_irq_enabled;
extern INT32 K051960_nmi_enabled;
extern INT32 K051960_spriteflip;

void K052109UpdateScroll();
void K052109AdjustScroll(INT32 x, INT32 y);
void K052109RenderLayer(INT32 nLayer, INT32 Opaque, UINT8 *pSrc);
UINT8 K052109Read(UINT32 Offset);
void K052109Write(UINT32 Offset, UINT8 Data);
void K052109SetCallback(void (*Callback)(INT32 Layer, INT32 Bank, INT32 *Code, INT32 *Colour, INT32 *xFlip, INT32 *priority));
void K052109Reset();
void K052109Init(UINT8 *pRomSrc, UINT32 RomMask);
void K052109Exit();
void K052109Scan(INT32 nAction);

#define K051960ByteRead(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	return K051960Read(a - nStartAddress);					\
}

#define K051960ByteWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	K051960Write((a - nStartAddress), d);					\
	return;									\
}

#define K051960WordWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 0x3ff) {				\
	if (a & 1) {								\
		K051960Write((a - nStartAddress) + 1, d & 0xff);		\
	} else {								\
		K051960Write((a - nStartAddress) + 0, (d >> 8) & 0xff);		\
	}									\
	return;									\
}

#define K051937ByteRead(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 7) {				\
	INT32 Offset = (a - nStartAddress);					\
										\
	if (Offset == 0) {							\
		static INT32 Counter;						\
		return (Counter++) & 1;						\
	}									\
										\
	if (K051960ReadRoms && (Offset >= 0x04 && Offset <= 0x07)) {		\
		return K0519060FetchRomData(Offset & 3);			\
	}									\
										\
	return 0;								\
}

#define K015937ByteWrite(nStartAddress)						\
if (a >= nStartAddress && a <= nStartAddress + 7) {				\
	K051937Write((a - nStartAddress), d);					\
	return;									\
}

#define K052109WordNoA12Read(nStartAddress)					\
if (a >= nStartAddress && a <= nStartAddress + 0x7fff) {			\
	INT32 Offset = (a - nStartAddress) >> 1;					\
	Offset = ((Offset & 0x3000) >> 1) | (Offset & 0x07ff);			\
										\
	if (a & 1) {								\
		return K052109Read(Offset + 0x2000);				\
	} else {								\
		return K052109Read(Offset + 0x0000);				\
	}									\
}


#define K052109WordNoA12Write(nStartAddress)					\
if (a >= nStartAddress && a <= nStartAddress + 0x7fff) {			\
	INT32 Offset = (a - nStartAddress) >> 1;					\
	Offset = ((Offset & 0x3000) >> 1) | (Offset & 0x07ff);			\
										\
	if (a & 1) {								\
		K052109Write(Offset + 0x2000, d);				\
	} else {								\
		K052109Write(Offset + 0x0000, d);				\
	}									\
	return;									\
}

// K051316.cpp
//---------------------------------------------------------------------------------------------------------------
void K051316Init(INT32 chip, UINT8 *gfx, UINT8 *gfxexp, INT32 mask, void (*callback)(INT32 *code,INT32 *color,INT32 *flags), INT32 bpp, INT32 transp);
void K051316Reset();
void K051316Exit();

void K051316RedrawTiles(INT32 chip);

UINT8 K051316ReadRom(INT32 chip, INT32 offset);
UINT8 K051316Read(INT32 chip, INT32 offset);
void K051316Write(INT32 chip, INT32 offset, INT32 data);

void K051316WriteCtrl(INT32 chip, INT32 offset, INT32 data);
void K051316WrapEnable(INT32 chip, INT32 status);
void K051316SetOffset(INT32 chip, INT32 xoffs, INT32 yoffs);
void K051316_zoom_draw(INT32 chip, INT32 flags);
void K051316Scan(INT32 nAction);

// K053245.cpp
//---------------------------------------------------------------------------------------------------------------
INT32 K053245Reset();
void K053245GfxDecode(UINT8 *src, UINT8 *dst, INT32 len);
void K053245Init(INT32 chip, UINT8 *gfx, INT32 mask, void (*callback)(INT32 *code,INT32 *color,INT32 *priority));
void K053245Exit();

void K053245SpritesRender(INT32 chip, UINT8 *gfxdata, INT32 priority);

void K053245SetSpriteOffset(INT32 chip,INT32 offsx, INT32 offsy);
void K053245ClearBuffer(INT32 chip);
void K053245UpdateBuffer(INT32 chip);
void K053244BankSelect(INT32 chip, INT32 bank);

UINT16 K053245ReadWord(INT32 chip, INT32 offset);
void K053245WriteWord(INT32 chip, INT32 offset, INT32 data);

UINT8 K053245Read(INT32 chip, INT32 offset);
void K053245Write(INT32 chip, INT32 offset, INT32 data);
UINT8 K053244Read(INT32 chip, INT32 offset);
void K053244Write(INT32 chip, INT32 offset, INT32 data);

void K053245Scan(INT32 nAction);

// K053251.cpp
//---------------------------------------------------------------------------------------------------------------
void K053251Reset();

void K053251Write(INT32 offset, INT32 data);

INT32 K053251GetPriority(INT32 idx);
INT32 K053251GetPaletteIndex(INT32 idx);

void K053251Write(INT32 offset, INT32 data);
INT32 K053251GetPriority(INT32 idx);
INT32 K053251GetPaletteIndex(INT32 idx);

void K053251Scan(INT32 nAction);

// K053247.cpp
//---------------------------------------------------------------------------------------------------------------
void K053247Reset();
void K053247Init(UINT8 *gfxrom, INT32 gfxlen, void (*Callback)(INT32 *code, INT32 *color, INT32 *priority), INT32 flags);
void K053247Exit();
void K053247Scan(INT32 nAction);

void K053247Export(UINT8 **ram, UINT8 **gfx, void (**callback)(INT32 *, INT32 *, INT32 *), INT32 *dx, INT32 *dy);
void K053247GfxDecode(UINT8 *src, UINT8 *dst, INT32 len); // 16x16
void K053247SetSpriteOffset(INT32 offsx, INT32 offsy);
void K053247WrapEnable(INT32 status);

void K053246_set_OBJCHA_line(INT32 state); // 1 assert, 0 clear
INT32 K053246_is_IRQ_enabled();

UINT8 K053247Read(INT32 offset);
void K053247Write(INT32 offset, INT32 data);
UINT8 K053246Read(INT32 offset);
void K053246Write(INT32 offset, INT32 data);

void K053247SpritesRender(UINT8 *gfxbase, INT32 priority);

// k054000.cpp
//------------------------------------------------------------------------------------------
void K054000Reset();
void K054000Write(INT32 offset, INT32 data);
UINT8 K054000Read(INT32 address);
void K054000Scan(INT32 nAction);

// K051733.cpp
//------------------------------------------------------------------------------------------
void K051733Reset();
void K051733Write(INT32 offset, INT32 data);
UINT8 K051733Read(INT32 offset);
void K051733Scan(INT32 nAction);

// K053936.cpp
//------------------------------------------------------------------------------------------
void K053936Init(INT32 chip, UINT8 *ram, INT32 len, INT32 w, INT32 h, void (*pCallback)(INT32 offset, UINT16 *ram, INT32 *code, INT32 *color, INT32 *sx, INT32 *sy, INT32 *fx, INT32 *fy));
void K053936Reset();
void K053936Exit();
void K053936Scan(INT32 nAction);

void K053936EnableWrap(INT32 chip, INT32 status);
void K053936SetOffset(INT32 chip, INT32 xoffs, INT32 yoffs);

void K053936PredrawTiles(INT32 chip, UINT8 *gfx, INT32 transparent, INT32 tcol /*transparent color*/);
void K053936Draw(INT32 chip, UINT16 *ctrl, UINT16 *linectrl, INT32 transp);

