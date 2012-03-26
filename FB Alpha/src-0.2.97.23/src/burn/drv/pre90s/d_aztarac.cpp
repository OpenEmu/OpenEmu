// FB Alpha Aztarac driver module
// Based on MAME driver by Mathis Rosenhauer

#include "burnint.h"
#include "sek.h"
#include "zet.h"
#include "vector.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvVecRAM;
static UINT8 *DrvNVRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static INT32 sound_irq_timer;
static INT32 sound_status;

static INT32 sound_initialized = 0;
static INT16 *pFMBuffer[12];

static INT32 xcenter;
static INT32 ycenter;

static INT32 watchdog;

static UINT8 DrvJoy1[8];
static UINT8 DrvInputs[1];
static UINT8 DrvReset;

static INT16 DrvAnalogPort0;
static INT16 DrvAnalogPort1;

static UINT8 xAxis = 0;
static UINT8 yAxis = 0;
static UINT8 Dial = 0;
static UINT8 DialInputs[2];

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo AztaracInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 4,		"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 2,		"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 1,		"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 0,		"p1 fire 2"	},
	
	A("Left/Right", 	BIT_ANALOG_REL, &DrvAnalogPort0,	"p1 x-axis"  ),
	A("Up/Down",		BIT_ANALOG_REL, &DrvAnalogPort1,	"p1 y-axis"  ),
	{"Aim Left",		BIT_DIGITAL,	DialInputs + 0,		"p1 fire 3"	},
	{"Aim Right",		BIT_DIGITAL,	DialInputs + 1,		"p1 fire 4"	},
	

	{"Reset",			BIT_DIGITAL,	&DrvReset,			"reset"		},
	{"Service",			BIT_DIGITAL,    DrvJoy1 + 7,  		"service"	},
};

STDINPUTINFO(Aztarac)

#undef A

static inline void read_vectorram (INT32 addr, INT32 *x, INT32 *y, INT32 *c)
{
	*c = SekReadWord(0xff8000 + addr);
	*x = SekReadWord(0xff9000 + addr) & 0x03ff;
	*y = SekReadWord(0xffa000 + addr) & 0x03ff;
	if (*x & 0x200) *x |= 0xfffffc00; // signed
	if (*y & 0x200) *y |= 0xfffffc00; // signed
}

static void aztarac_process_vector_list()
{
	INT32 x, y, c, intensity, xoffset, yoffset, color;
	INT32 defaddr, objaddr, ndefs;

	vector_reset();

	for (objaddr = 0; objaddr < 0x800; objaddr++)
	{
		read_vectorram (objaddr * 2, &xoffset, &yoffset, &c);

		if (c & 0x4000)	break;

		if ((c & 0x2000) == 0)
		{
			defaddr = (c >> 1) & 0x7ff;

			vector_add_point((xcenter + (xoffset << 16)), (ycenter - (yoffset << 16)), 0, 0);

			read_vectorram (defaddr * 2, &x, &ndefs, &c);
			ndefs++;

			if (c & 0xff00)
			{
				intensity = (c >> 8);
				color = c & 0x3f;

				while (ndefs--)
				{
					defaddr++;
					read_vectorram (defaddr * 2, &x, &y, &c);

					if ((c & 0xff00) == 0)
						vector_add_point((xcenter + ((x + xoffset) << 16)), (ycenter - ((y + yoffset) << 16)), 0, 0);
					else
						vector_add_point((xcenter + ((x + xoffset) << 16)), (ycenter - ((y + yoffset) << 16)), color, intensity);
				}
			}
			else
			{
				while (ndefs--)
				{
					defaddr++;
					read_vectorram (defaddr * 2, &x, &y, &c);

					color = c & 0x3f;
					vector_add_point((xcenter + ((x + xoffset) << 16)), (ycenter - ((y + yoffset) << 16)), color, c >> 8);
				}
			}
		}
	}
}

static inline void sync_cpu()
{
	INT32 cycles = (SekTotalCycles() / 4) - ZetTotalCycles();

	if (cycles > 0)	ZetRun(cycles);
}

void __fastcall aztarac_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff00) == 0x022000) {
		*((UINT16*)(DrvNVRAM + (address & 0xfe))) = data | 0xfff0;
		return;
	}

	if (address == 0xffb000) {
		if (data) aztarac_process_vector_list(); // used once?
		return;
	}
}

void __fastcall aztarac_write_byte(UINT32 address, UINT8 data)
{
	if (address == 0x027009) {
		sync_cpu();
		*soundlatch = data;
		sound_status ^= 0x21;

		if (sound_status & 0x20) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);

		return;
	}

	if (address == 0xffb001) {
		if (data) aztarac_process_vector_list();
		return;
	}
}

UINT16 __fastcall aztarac_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x027004:
			return 0xff00 | DrvInputs[0];

		case 0x02700e:
			watchdog = 0;
			return 0;
	}

	return 0;
}

UINT8 __fastcall aztarac_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x027000:
			return xAxis - 0x0f;
			
		case 0x027001:
			return yAxis - 0x0f;

		case 0x027005:
			return DrvInputs[0]; // inputs

		case 0x027009:
			sync_cpu();
			return sound_status & 0x01;

		case 0x02700d:
			return Dial;
	}

	return 0;
}

void __fastcall aztarac_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x8c00:
		case 0x8c01:
		case 0x8c02:
		case 0x8c03:
		case 0x8c04:
		case 0x8c05:
		case 0x8c06:
		case 0x8c07:
			AY8910Write((address & 6) / 2, ~address & 1, data);
		return;

		case 0x9000:
			sound_status &= ~0x10;
		return;
	}
}

UINT8 __fastcall aztarac_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x8800:
			sound_status = (sound_status | 0x01) & ~0x20;
			return *soundlatch;

		case 0x8c00:
		case 0x8c01:
		case 0x8c02:
		case 0x8c03:
		case 0x8c04:
		case 0x8c05:
		case 0x8c06:
		case 0x8c07:
			return AY8910Read((address & 6) / 2);

		case 0x9000:
			return sound_status & ~0x01;
	}

	return 0;
}

static INT32 __fastcall aztarac_irq_callback(INT32)
{
	return 0x0c;
}

static INT32 DrvDoReset(INT32 reset_ram)
{
	if (reset_ram) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	AY8910Reset(0);
	AY8910Reset(1);
	AY8910Reset(2);
	AY8910Reset(3);

	sound_status = 0;
	sound_irq_timer = 0;

	watchdog = 0;

	vector_reset();

	return 0;
}

static void sound_init() // Changed refresh rate causes crashes
{
	for (INT32 i = 0; i < 12; i++) {
		pFMBuffer[i] = (INT16*)BurnMalloc(nBurnSoundLen * sizeof(INT16));
	}

	sound_initialized = 1;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x010000;
	DrvZ80ROM		= Next; Next += 0x002000;

	DrvNVRAM		= Next; Next += 0x000400;

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x002000;
	DrvZ80RAM		= Next; Next += 0x000800;
	DrvVecRAM		= Next; Next += 0x003000;

	soundlatch		= Next; Next += 0x000001;	

	RamEnd			= Next;

	DrvPalette		= (UINT32*)Next; Next += 0x0040 * 256 * sizeof(UINT32);

	MemEnd			= Next;

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x40; i++) // color
	{
		for (INT32 j = 0; j < 256; j++) // intensity
		{
			INT32 r = (i >> 4) & 3;
			r = (r << 6) | (r << 4) | (r << 2) | (r << 0);
	
			INT32 g = (i >> 2) & 3;
			g = (g << 6) | (g << 4) | (g << 2) | (g << 0);
	
			INT32 b = (i >> 0) & 3;
			b = (b << 6) | (b << 4) | (b << 2) | (b << 0);

			r = (r * j) / 255;
			g = (g * j) / 255;
			b = (b * j) / 255;
	
			DrvPalette[i * 256 + j] = (r << 16) | (g << 8) | b;
		}
	}
}

static INT32 DrvInit()
{
	BurnSetRefreshRate(40.0);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x00000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x02001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x02000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x04001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x04000,  5, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x06001,  6, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x06000,  7, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x08001,  8, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x08000,  9, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x0a001, 10, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x0a000, 11, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM + 0x00000, 12, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x01000, 13, 1)) return 1;
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekSetIrqCallback(aztarac_irq_callback);
	SekMapMemory(Drv68KROM,		0x000000, 0x00bfff, SM_ROM);
	SekMapMemory(DrvNVRAM,		0x022000, 0x0223ff, SM_ROM);
	SekMapMemory(DrvVecRAM,		0xff8000, 0xffafff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0xffe000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0,	aztarac_write_word);
	SekSetWriteByteHandler(0,	aztarac_write_byte);
	SekSetReadWordHandler(0,	aztarac_read_word);
	SekSetReadByteHandler(0,	aztarac_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x1fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x1fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(aztarac_sound_write);
	ZetSetReadHandler(aztarac_sound_read);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(2, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(3, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	DrvPaletteInit();

	vector_init();

	xcenter = (1024 / 2) << 16;
	ycenter = ( 768 / 2) << 16;

	memset (DrvNVRAM, 0xff, 0x100);

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	vector_exit();

	SekExit();
	ZetExit();

	AY8910Exit(0);
	AY8910Exit(1);
	AY8910Exit(2);
	AY8910Exit(3);

	BurnFree (AllMem);

	sound_initialized = 0;
	for (INT32 i = 0; i < 12; i++) {
		BurnFree (pFMBuffer[i]);
	}

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	draw_vector(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (sound_initialized == 0) {
		if (pBurnSoundOut) {
			sound_init();
		}
	}

	if (DrvReset) {
		DrvDoReset(1);
	}

	watchdog++;
	if (watchdog == 180) { // 3 seconds?
		DrvDoReset(0);
	}

	SekNewFrame();
	ZetNewFrame();

	{
		DrvInputs[0] = 0xff;
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		}
		
		xAxis = (DrvAnalogPort0 >> 7) + 0x0f;
		if (xAxis > 0x80) xAxis = 0;
		
		yAxis = (~DrvAnalogPort1 >> 7) + 0x10;
		if (yAxis > 0x1d) yAxis = 0x1d;
		
		if (DialInputs[0]) {
			Dial += 0x04;
		} else {
			if (DialInputs[1]) {
				Dial -= 0x04;
			}
		}
	}

	INT32 nInterleave = 100;
	INT32 nCyclesTotal[2] = { 8000000 / 40, 2000000 / 40 };
	INT32 nCyclesDone[2]  = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++, sound_irq_timer++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		if (i == (nInterleave - 1)) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);

		//nCyclesDone[1] += ZetRun((SekTotalCycles() / 4) - ZetTotalCycles());
		sync_cpu();

		if ((sound_irq_timer % 40) == 39) {	// every 20000 cycles, 50000 / frame
			sound_status ^= 0x10;
			if (sound_status & 0x10) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		}
	}

	SekClose();
	ZetClose();

	if (pBurnSoundOut) {
		INT32 nSample;

		AY8910Update(0, &pFMBuffer[0], nBurnSoundLen);
		AY8910Update(1, &pFMBuffer[3], nBurnSoundLen);
		AY8910Update(2, &pFMBuffer[6], nBurnSoundLen);
		AY8910Update(3, &pFMBuffer[9], nBurnSoundLen);
		for (INT32 n = 0; n < nBurnSoundLen; n++) {
			nSample  = pFMBuffer[ 0][n] / 2;
			nSample += pFMBuffer[ 1][n] / 2;
			nSample += pFMBuffer[ 2][n] / 2;
			nSample += pFMBuffer[ 3][n] / 2;
			nSample += pFMBuffer[ 4][n] / 2;
			nSample += pFMBuffer[ 5][n] / 2;
			nSample += pFMBuffer[ 6][n] / 2;
			nSample += pFMBuffer[ 7][n] / 2;
			nSample += pFMBuffer[ 8][n] / 2;
			nSample += pFMBuffer[ 9][n] / 2;
			nSample += pFMBuffer[10][n] / 2;
			nSample += pFMBuffer[11][n] / 2;

			nSample = BURN_SND_CLIP(nSample);

			pBurnSoundOut[(n << 1) + 0] = nSample;
			pBurnSoundOut[(n << 1) + 1] = nSample;
		}
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029698;
	}

	if (nAction & ACB_MEMORY_ROM) {	
		ba.Data		= Drv68KROM;
		ba.nLen		= 0x000c000;
		ba.nAddress	= 0x0000000;
		ba.szName	= "68K ROM";
		BurnAcb(&ba);

		ba.Data		= DrvZ80ROM;
		ba.nLen		= 0x0002000;
		ba.nAddress	= 0x0000000;
		ba.szName	= "Z80 ROM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= Drv68KRAM;
		ba.nLen		= 0x0002000;
		ba.nAddress	= 0x0ffe000;
		ba.szName	= "68K RAM";
		BurnAcb(&ba);

		ba.Data		= DrvVecRAM;
		ba.nLen		= 0x0003000;
		ba.nAddress	= 0x0ff8000;
		ba.szName	= "Vector RAM";
		BurnAcb(&ba);

 		ba.Data		= DrvZ80RAM;
		ba.nLen		= 0x0000800;
		ba.nAddress	= 0x0000000;
		ba.szName	= "Z80 RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_NVRAM) {
		ba.Data		= DrvNVRAM;
		ba.nLen		= 0x000100;
		ba.nAddress	= 0x022000;
		ba.szName	= "NV RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		AY8910Scan(nAction, pnMin);

		SCAN_VAR(*soundlatch);
		SCAN_VAR(sound_irq_timer);
		SCAN_VAR(sound_status);
	}

	vector_scan(nAction);

	return 0;
}


// Aztarac

static struct BurnRomInfo aztaracRomDesc[] = {
	{ "l8_6.bin",	0x1000, 0x25f8da18, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "n8_0.bin",	0x1000, 0x04e20626, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "l7_7.bin",	0x1000, 0x230e244c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "n7_1.bin",	0x1000, 0x37b12697, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "l6_8.bin",	0x1000, 0x1293fb9d, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "n6_2.bin",	0x1000, 0x712c206a, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "l5_9.bin",	0x1000, 0x743a6501, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "n5_3.bin",	0x1000, 0xa65cbf99, 1 | BRF_PRG | BRF_ESS }, //  7
	{ "l4_a.bin",	0x1000, 0x9cf1b0a1, 1 | BRF_PRG | BRF_ESS }, //  8
	{ "n4_4.bin",	0x1000, 0x5f0080d5, 1 | BRF_PRG | BRF_ESS }, //  9
	{ "l3_b.bin",	0x1000, 0x8cc7f7fa, 1 | BRF_PRG | BRF_ESS }, // 10
	{ "n3_5.bin",	0x1000, 0x40452376, 1 | BRF_PRG | BRF_ESS }, // 11

	{ "j4_c.bin",	0x1000, 0xe897dfcd, 2 | BRF_PRG | BRF_ESS }, // 12 Z80 Code
	{ "j3_d.bin",	0x1000, 0x4016de77, 2 | BRF_PRG | BRF_ESS }, // 13
};

STD_ROM_PICK(aztarac)
STD_ROM_FN(aztarac)

struct BurnDriver BurnDrvAztarac = {
	"aztarac", NULL, NULL, NULL, "1983",
	"Aztarac\0", "Vector graphics", "Centuri", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, aztaracRomInfo, aztaracRomName, NULL, NULL, AztaracInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x40 * 256,
	1024, 768, 4, 3
};
