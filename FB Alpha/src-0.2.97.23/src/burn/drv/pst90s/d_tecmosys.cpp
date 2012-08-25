// FB Alpha Tecmo System driver module
// Based on MAME driver by Farfetch, David Haywood, Tomasz Slanina, and nuapete

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "msm6295.h"
#include "eeprom.h"
#include "ymz280b.h"
// ymf262

//#define ENABLE_SOUND_HARDWARE // no sound without ymf262 core anyway...

#ifdef ENABLE_SOUND_HARDWARE
#include "z80_intf.h"
#endif

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvSprROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;

static UINT8 *Drv68KRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvTxtRAM;
static UINT8 *DrvBgRAM0;
static UINT8 *DrvBgRAM1;
static UINT8 *DrvBgRAM2;
static UINT8 *DrvBgScrRAM0;
static UINT8 *DrvBgScrRAM1;
static UINT8 *DrvBgScrRAM2;
static UINT8 *spritelist_select;
static UINT8 *Drv88Regs;
static UINT8 *DrvA8Regs;
static UINT8 *DrvB0Regs;
static UINT8 *DrvC0Regs;
static UINT8 *DrvC8Regs;

#ifdef ENABLE_SOUND_HARDWARE
static UINT8 *DrvZ80ROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvOkiBank;
static UINT8 *DrvZ80Bank;
static UINT8 *soundlatch;
static UINT8 *soundlatch2;
#endif

static UINT8 protection_read_pointer;
static UINT8 protection_status;
static UINT8 protection_value;

static UINT16 *DrvTmpSprites;

static UINT32 *DrvPalette;
static UINT32 *DrvPalette24;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static INT32 vblank;
static INT32 watchdog;
static INT32 deroon;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 10,	"p1 fire 4"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 10,	"p2 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service 1",		BIT_DIGITAL,	DrvJoy1 + 9,	"service"	},
	{"Service 2",		BIT_DIGITAL,	DrvJoy2 + 9,	"service"	},
};

STDINPUTINFO(Drv)

struct prot_data
{
	UINT8 passwd_len;
	const UINT8* passwd;
	const UINT8* code;
	UINT8 checksums[4];
};

static const struct prot_data *protection_data;

static const UINT8 deroon_passwd[] = { 'L', 'U', 'N', 'A', 0 };
static const UINT8 deroon_upload[] = { 0x02, 0x4e, 0x75, 0x00 };
static const struct prot_data deroon_data  = { 0x05, deroon_passwd, deroon_upload, { 0xa6, 0x29, 0x4b, 0x3f } };

static const UINT8 tkdensho_passwd[] = { 'A','G','E','P','R','O','T','E','C','T',' ','S','T','A','R','T', 0 };
static const UINT8 tkdensho_upload[] = { 0x06, 0x4e, 0xf9, 0x00, 0x00, 0x22, 0xc4, 0x00 };
static const struct prot_data tkdensho_data  = { 0x11, tkdensho_passwd, tkdensho_upload, { 0xbf, 0xfa, 0xda, 0xda } };
static const struct prot_data tkdenshoa_data = { 0x11, tkdensho_passwd, tkdensho_upload, { 0xbf, 0xfa, 0x21, 0x5d } };

static void protection_reset()
{
	protection_read_pointer = 0;
	protection_status = 0; // idle
	protection_value = 0xff;
}

static void tecmosys_prot_data_write(INT32 data)
{
	static const UINT8 ranges[] = { 
		0x10,0x11,0x12,0x13,0x24,0x25,0x26,0x27,0x38,0x39,0x3a,0x3b,0x4c,0x4d,0x4e,0x4f, 0x00
	};

	switch (protection_status)
	{
		case 0: // idle
			if (data == 0x13)
			{
				protection_status = 1; // login
				protection_value = protection_data->passwd_len;
				protection_read_pointer = 0;
				break;
			}
			break;

		case 1: // login
			if (protection_read_pointer >= protection_data->passwd_len)
			{
				protection_status = 2; // send code
				protection_value = protection_data->code[0];
				protection_read_pointer = 1;
			}
			else
				protection_value = protection_data->passwd[protection_read_pointer++] == data ? 0 : 0xff;
			break;

		case 2: // send code
			if (protection_read_pointer >= protection_data->code[0]+2)
			{
				protection_status = 3; // send address
				protection_value = ranges[0];
				protection_read_pointer = 1;
			}
			else
				protection_value = data == protection_data->code[protection_read_pointer-1] ? protection_data->code[protection_read_pointer++] : 0xff;
			break;

		case 3: // send address
			if (protection_read_pointer >= 17)
			{
				protection_status = 4; // send checksum
				protection_value = 0;
				protection_read_pointer = 0;
			}
			else
			{
				protection_value = data == ranges[protection_read_pointer-1] ? ranges[protection_read_pointer++] : 0xff;
			}
			break;

		case 4: // send checksum
			if (protection_read_pointer >= 5)
			{
				protection_status = 5; // done
				protection_value = 0;
			}
			else
				protection_value = data == protection_data->checksums[protection_read_pointer] ? protection_data->checksums[protection_read_pointer++] : 0xff;
			break;

		case 5: // done
			break;
	}
}

void __fastcall tecmosys_main_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0x880000:
		case 0x880002:
			*((UINT16*)(Drv88Regs + (address & 0x02))) = data;
		return;

		case 0x880008:
			*spritelist_select = data & 0x03;
		return;

		case 0x880022:
			watchdog = 0;
		return;

		case 0xa00000:
			EEPROMWrite((data & 0x0400), (data & 0x0200), (data & 0x0800));
		return;

		case 0xa80000:
		case 0xa80002:
		case 0xa80004:
			*((UINT16*)(DrvA8Regs + (address & 0x06))) = data;
		return;

		case 0xb00000:
		case 0xb00002:
		case 0xb00004:
			*((UINT16*)(DrvB0Regs + (address & 0x06))) = data;
		return;

		case 0xb80000: // protection status - does nothing
		return;

		case 0xc00000:
		case 0xc00002:
		case 0xc00004:
			*((UINT16*)(DrvC0Regs + (address & 0x06))) = data;
		return;

		case 0xc80000:
		case 0xc80002:
		case 0xc80004:
			*((UINT16*)(DrvC8Regs + (address & 0x06))) = data;
		return;

		case 0xe00000:
#ifdef ENABLE_SOUND_HARDWARE
			ZetRun((SekTotalCycles() / 2) - ZetTotalCycles());
			*soundlatch = data & 0xff;
			ZetNmi();
#endif
		return;

		case 0xe80000:
			tecmosys_prot_data_write(data >> 8);
		return;
	}
}

void __fastcall tecmosys_main_write_byte(UINT32 /*address*/, UINT8 /*data*/)
{

}

UINT16 __fastcall tecmosys_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x880000:
			return vblank;

		case 0xd00000:
			return DrvInputs[0];

		case 0xd00002:
			return DrvInputs[1];

		case 0xd80000:
			return (EEPROMRead() & 1) << 11;

		case 0xf00000:
#ifdef ENABLE_SOUND_HARDWARE
			ZetRun((SekTotalCycles() / 2) - ZetTotalCycles());
			return *soundlatch2;
#else
			return 0;
#endif

		case 0xf80000:
			INT32 ret = protection_value;
			protection_value = 0xff;
			return ret << 8;
	}

	return 0;
}

UINT8 __fastcall tecmosys_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xb80000:
			return 0x00; // protection status
	}

	return 0;
}

static inline void palette_update(INT32 pal)
{
	UINT16 p = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvPalRAM + pal * 2)));

	INT32 r = (p >>  5) & 0x1f;
	INT32 g = (p >> 10) & 0x1f;
	INT32 b = (p >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[pal] = BurnHighCol(r, g, b, 0);
	DrvPalette24[pal] = (r << 16) + (g << 8) + b;
}

void __fastcall tecmosys_palette_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xff8000) == 0x900000) {
		*((UINT16 *)(DrvPalRAM + 0x0000 + (address & 0x7ffe))) = BURN_ENDIAN_SWAP_INT16(data);
		palette_update((0x0000 + (address & 0x7ffe)) / 2);
		return;
	}

	if ((address & 0xfff000) == 0x980000) {
		*((UINT16 *)(DrvPalRAM + 0x8000 + (address & 0x0ffe))) = BURN_ENDIAN_SWAP_INT16(data);
		palette_update((0x8000 + (address & 0x0ffe)) / 2);
		return;
	}
}

void __fastcall tecmosys_palette_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xff8000) == 0x900000) {
		DrvPalRAM[(0x0000 + (address & 0x7fff)) ^ 1] = data;
		palette_update((0x0000 + (address & 0x7ffe)) / 2);
		return;
	}

	if ((address & 0xfff000) == 0x980000) {
		DrvPalRAM[(0x8000 + (address & 0x0fff)) ^ 1] = data;
		palette_update((0x8000 + (address & 0x0ffe)) / 2);
		return;
	}
}

#ifdef ENABLE_SOUND_HARDWARE

static void bankswitch(INT32 data)
{
	if ((data & 0x0f) != *DrvZ80Bank) {
		ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + (data & 0x0f) * 0x4000);
		ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + (data & 0x0f) * 0x4000);
	}

	*DrvZ80Bank = data & 0x0f;
}

static void oki_bankswitch(INT32 data)
{
	if ((data & 0x33) != *DrvOkiBank) {

		INT32 upperbank = (data & 0x30) >> 4;
		INT32 lowerbank = (data & 0x03) >> 0;
	
		if (lowerbank != ((*DrvOkiBank & 0x0f) >> 0)) {
			memcpy (DrvSndROM0 + 0x00000, DrvSndROM0 + 0x80000 + lowerbank * 0x20000, 0x20000);
		}

		if (upperbank != ((*DrvOkiBank & 0xf0) >> 4)) {
			memcpy (DrvSndROM0  +0x20000, DrvSndROM0 + 0x80000 + upperbank * 0x20000, 0x20000);
		}
	}

	*DrvOkiBank = data & 0x33;
}

void __fastcall tecmosys_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			// ymf262_w
		return;

		case 0x10:
			MSM6295Command(0, data);
		return;

		case 0x20:
			oki_bankswitch(data);
		return;

		case 0x30:
			bankswitch(data);
		return;

		case 0x50:
			*soundlatch2 = data;
		return;

		case 0x60:
			YMZ280BSelectRegister(data);
		return;

		case 0x61:
			YMZ280BWriteRegister(data);
		return;
	}
}

UINT8 __fastcall tecmosys_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			return 0; // ymf262_r

		case 0x10:
			return MSM6295ReadStatus(0);

		case 0x40:
			return *soundlatch;

		case 0x60:
		case 0x61:
			return YMZ280BReadStatus();
	}

	return 0;
}

/*
static void DrvIrqHandler(INT32 irq) // for ymf262
{
	if (irq) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}
*/

#endif

static INT32 DrvDoReset(INT32 full_reset)
{
	if (full_reset) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	SekOpen(0);
	SekReset();
	SekClose();

	EEPROMReset();

	protection_reset();

	watchdog = 0;

#ifdef ENABLE_SOUND_HARDWARE
	ZetOpen(0);
	ZetReset();
	ZetClose();

	// ymf262
	YMZ280BReset();
	MSM6295Reset(0);

	*DrvOkiBank = *DrvZ80Bank = ~0;
#endif

	return 0;
}

#ifdef ENABLE_SOUND_HARDWARE
static INT32 MemIndex(INT32 sndlen)
#else
static INT32 MemIndex(INT32 /*sndlen*/)
#endif
{

	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x100000;

	DrvGfxROM0		= Next; Next += 0x200000;
	DrvGfxROM1		= Next; Next += 0x200000;
	DrvGfxROM2		= Next; Next += 0x200000;
	DrvGfxROM3		= Next; Next += 0x200000;

#ifdef ENABLE_SOUND_HARDWARE
	DrvZ80ROM		= Next; Next += 0x040000;

	MSM6295ROM		= Next;
	DrvSndROM0		= Next; Next += 0x100000;

	YMZ280BROM		= Next;
	DrvSndROM1		= Next; Next += sndlen;
#endif

	DrvPalette		= (UINT32*)Next; Next += 0x4800 * sizeof(UINT32);
	DrvPalette24		= (UINT32*)Next; Next += 0x4800 * sizeof(UINT32);

	DrvTmpSprites		= (UINT16*)Next; Next += 320 * 240 * sizeof(UINT16);

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x010000;
	DrvSprRAM		= Next; Next += 0x010000;
	DrvPalRAM		= Next; Next += 0x009000;

	DrvTxtRAM		= Next; Next += 0x004000;

	DrvBgRAM0		= Next; Next += 0x001000;
	DrvBgScrRAM0		= Next; Next += 0x000400;
	DrvBgRAM1		= Next; Next += 0x001000;
	DrvBgScrRAM1		= Next; Next += 0x000400;
	DrvBgRAM2		= Next; Next += 0x001000;
	DrvBgScrRAM2		= Next; Next += 0x000400;


#ifdef ENABLE_SOUND_HARDWARE
	DrvOkiBank 		= Next; Next += 0x000001;
	DrvZ80Bank 		= Next; Next += 0x000001;

	DrvZ80RAM		= Next; Next += 0x001800;

	soundlatch 		= Next; Next += 0x000001;
	soundlatch2 		= Next; Next += 0x000001;
#endif
	spritelist_select	= Next; Next += 0x000001;

	Drv88Regs		= Next; Next += 0x000004;
	DrvA8Regs		= Next; Next += 0x000006;
	DrvB0Regs		= Next; Next += 0x000006;
	DrvC0Regs		= Next; Next += 0x000006;
	DrvC8Regs		= Next; Next += 0x000006;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static void descramble_sprites(INT32 len)
{
	UINT8 *src = DrvSprROM;
	UINT8 tmp[4];

	for (INT32 i=0; i < len; i+=4)
	{
		tmp[0] = ((src[i+2]&0xf0)>>0) | ((src[i+3]&0xf0)>>4);
		tmp[1] = ((src[i+2]&0x0f)<<4) | ((src[i+3]&0x0f)>>0);
		tmp[2] = ((src[i+0]&0xf0)>>0) | ((src[i+1]&0xf0)>>4);
		tmp[3] = ((src[i+0]&0x0f)<<4) | ((src[i+1]&0x0f)<<0);

		memcpy (src + i, tmp, 4);
	}
}

static void expand_characters()
{
	for (INT32 i = 0x100000 - 1; i >= 0; i--) {
		DrvGfxROM0[i * 2 + 0] = DrvGfxROM0[i] >> 4;
		DrvGfxROM0[i * 2 + 1] = DrvGfxROM0[i] & 0x0f;
	}
}

static void expand_tiles(UINT8 *rom, INT32 len)
{
	INT32 Planes[4] = { 0, 1, 2, 3 };
	INT32 XOffs[16] = { 0x000, 0x004, 0x008, 0x00c, 0x010, 0x014, 0x018, 0x01c, 0x100, 0x104, 0x108, 0x10c, 0x110, 0x114, 0x118, 0x11c };
	INT32 YOffs[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0, 0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	UINT8 *tmp = (UINT8*)BurnMalloc(len);

	memcpy (tmp, rom, len);

	GfxDecode((len * 2) / (16 * 16), 4, 16, 16, Planes, XOffs, YOffs, 128*8, tmp, rom);

	BurnFree (tmp);
}

static INT32 CommonInit(INT32 (*pRomLoadCallback)(), INT32 spritelen, INT32 sndlen, const struct prot_data *dev_data_pointer, INT32 game)
{
	AllMem = NULL;
	MemIndex(sndlen);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex(sndlen);

	DrvSprROM = (UINT8*)BurnMalloc(spritelen);
	if (DrvSprROM == NULL) return 1;

	if (pRomLoadCallback) {
		if (pRomLoadCallback()) return 1;
	}

	descramble_sprites(spritelen);
	expand_characters();
	expand_tiles(DrvGfxROM1, 0x100000);
	expand_tiles(DrvGfxROM2, 0x100000);
	expand_tiles(DrvGfxROM3, 0x100000);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvBgRAM0,		0x300000, 0x300fff, SM_RAM);
	SekMapMemory(DrvBgScrRAM0,	0x301000, 0x3013ff, SM_RAM);
	SekMapMemory(DrvBgRAM1,		0x400000, 0x400fff, SM_RAM);
	SekMapMemory(DrvBgScrRAM1,	0x401000, 0x4013ff, SM_RAM);
	SekMapMemory(DrvBgRAM2,		0x500000, 0x500fff, SM_RAM);
	SekMapMemory(DrvBgScrRAM2,	0x501000, 0x5013ff, SM_RAM);
	SekMapMemory(DrvTxtRAM,		0x700000, 0x703fff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x800000, 0x80ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x900000, 0x907fff, SM_ROM);
	SekMapMemory(DrvPalRAM + 0x8000,0x980000, 0x980fff, SM_ROM);
	SekSetWriteWordHandler(0,	tecmosys_main_write_word);
	SekSetWriteByteHandler(0,	tecmosys_main_write_byte);
	SekSetReadWordHandler(0,	tecmosys_main_read_word);
	SekSetReadByteHandler(0,	tecmosys_main_read_byte);

	SekMapHandler(1,		0x900000, 0x980fff, SM_WRITE);
	SekSetWriteWordHandler(1,	tecmosys_palette_write_word);
	SekSetWriteByteHandler(1,	tecmosys_palette_write_byte);
	SekClose();

	deroon = game;
	protection_data = dev_data_pointer;

	EEPROMInit(&eeprom_interface_93C46);

	BurnSetRefreshRate(57.4458);

#ifdef ENABLE_SOUND_HARDWARE
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xe000, 0xf7ff, 0, DrvZ80RAM);
	ZetMapArea(0xe000, 0xf7ff, 1, DrvZ80RAM);
	ZetMapArea(0xe000, 0xf7ff, 2, DrvZ80RAM);
	ZetSetOutHandler(tecmosys_sound_out);
	ZetSetInHandler(tecmosys_sound_in);
	ZetMemEnd();
	ZetClose();

	// ymf262

	YMZ280BInit(16900000, NULL);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_1, 0.30, BURN_SND_ROUTE_LEFT);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_2, 0.30, BURN_SND_ROUTE_RIGHT);

	MSM6295Init(0, 2000000 / 132, 1);
	MSM6295SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);
#endif

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	EEPROMExit();

	SekExit();

#ifdef ENABLE_SOUND_HARDWARE
	ZetExit();

	// ymf262

	MSM6295Exit(0);
	MSM6295ROM = NULL;

	YMZ280BExit();
	YMZ280BROM = NULL;
#endif

	BurnFree (DrvSprROM);
	BurnFree (AllMem);

	return 0;
}

static void draw_character_layer()
{
	UINT16 *vram = (UINT16*)DrvTxtRAM;

	for (INT32 offs = 0; offs < 64 * 64; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 0]);
		INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 1]) & 0x7fff;
		INT32 color = attr & 0x003f;
		INT32 flipy = attr & 0x0080;
		INT32 flipx = attr & 0x0040;

		if (code == 0) continue; // Should save some cycles

		if (sx >= 0 && sx <= (nScreenWidth - 8) && sy >= 0 && sy <= (nScreenHeight - 8)) {
			if (flipy) {
				if (flipx) {
					Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, 0xc400, DrvGfxROM0);
				} else {
					Render8x8Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 4, 0, 0xc400, DrvGfxROM0);	
				}
			} else {
				if (flipx) {
					Render8x8Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 0, 0xc400, DrvGfxROM0);
				} else {
					Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0xc400, DrvGfxROM0);	
				}
			}
		} else {
			if (flipy) {
				if (flipx) {
					Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0xc400, DrvGfxROM0);
				} else {
					Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0xc400, DrvGfxROM0);	
				}
			} else {
				if (flipx) {
					Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0xc400, DrvGfxROM0);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0xc400, DrvGfxROM0);	
				}
			}
		}
	}
}

static void draw_background_layer(UINT8 *ram, UINT8 *gfx, UINT8 *regs, INT32 yoff, INT32 xoff, INT32 priority)
{
	UINT16 *vram = (UINT16*)ram;

	INT32 scrollx = (*((UINT16*)(regs + 0)) + xoff) & 0x1ff;
	INT32 scrolly = (*((UINT16*)(regs + 2)) + yoff) & 0x1ff;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx -= scrollx;
		if (sx < -15) sx += 512;
		sy -= scrolly;
		if (sy < -15) sy += 512;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 0]);
		INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 1]) & 0x1fff;
		INT32 color = attr & 0x003f;
		INT32 flipy = attr & 0x0080;
		INT32 flipx = attr & 0x0040;

		if (!code) continue; // Should save some cycles

		if (sx >= 0 && sx <= (nScreenWidth - 16) && sy >= 0 && sy <= (nScreenHeight - 16)) {
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, priority, gfx);
				} else {
					Render16x16Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 4, 0, priority, gfx);	
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 0, priority, gfx);
				} else {
					Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, priority, gfx);	
				}
			}
		} else {
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, priority, gfx);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, priority, gfx);	
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, priority, gfx);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, priority, gfx);	
				}
			}
		}
	}
}

static void draw_sprite_zoomed(INT32 addr, INT32 color, INT32 x, INT32 y, INT32 flipx, INT32 flipy, INT32 xsize, INT32 ysize, INT32 zoomx, INT32 zoomy)
{
	if (y >= nScreenHeight || x >= nScreenWidth) return;

	INT32 drawx, drawy;
	UINT8 *rom = DrvSprROM + addr;

	for (INT32 ycnt = 0; ycnt < ysize; ycnt++, rom += xsize)
	{
		if (flipy)
			drawy = y + (((ysize * zoomy) / 256) - 1) - ((ycnt * zoomy) / 256);
		else
			drawy = y + ((ycnt * zoomy) / 256);

		if (drawy < 0 || drawy >= 240) continue;

		UINT16 *dstptr = DrvTmpSprites + drawy * nScreenWidth;

		for (INT32 xcnt = 0; xcnt < xsize; xcnt++)
		{
			if (flipx)
				drawx = x + (((xsize * zoomx) / 256) - 1) - ((xcnt * zoomx) / 256);
			else
				drawx = x + ((xcnt * zoomx) / 256);

			if (drawx >= 0 && drawx < 320)
			{
				INT32 data = rom[xcnt];

				if (data) dstptr[drawx] = data + color;
			}
		}
	}
}

static void draw_sprite_nozoom(INT32 addr, INT32 color, INT32 x, INT32 y, INT32 flipx, INT32 flipy, INT32 xsize, INT32 ysize)
{
	if (y >= nScreenHeight || x >= nScreenWidth) return;

	INT32 drawx, drawy;
	UINT8 *rom = DrvSprROM + addr;

	for (INT32 ycnt = 0; ycnt < ysize; ycnt++, rom += xsize)
	{
		if (flipy)
			drawy = y + (ysize - 1) - ycnt;
		else
			drawy = y + ycnt;

		if (drawy < 0 || drawy >= 240) continue;

		UINT16 *dstptr = DrvTmpSprites + drawy * nScreenWidth;

		if (x >= 0 && (x + xsize) < nScreenWidth) {
			if (flipx) {
				drawx = x + (xsize - 1);

				for (INT32 xcnt = 0; xcnt < xsize; xcnt++)
				{
					INT32 data = rom[xcnt];
		
					if (data) dstptr[drawx - xcnt] = data + color;
				}
			} else {
				dstptr += x;

				for (INT32 xcnt = 0; xcnt < xsize; xcnt++)
				{
					INT32 data = rom[xcnt];
		
					if (data) dstptr[xcnt] = data + color;
				}
			}
		} else {
			for (INT32 xcnt = 0; xcnt < xsize; xcnt++)
			{
				if (flipx)
					drawx = x + (xsize - 1) - xcnt;
				else
					drawx = x + xcnt;
	
				if (drawx >= 0 && drawx < 320)
				{
					INT32 data = rom[xcnt];
	
					if (data) dstptr[drawx] = data + color;
				}
			}
		}
	}
}

static void draw_sprites()
{
	INT32 extrax =  (*((UINT16*)(Drv88Regs + 0)));
	INT32 extray =  (*((UINT16*)(Drv88Regs + 2)));

	memset (DrvTmpSprites, 0, 320 * 240 * sizeof(INT16));

	UINT16 *spriteram = (UINT16*)(DrvSprRAM + (*spritelist_select * 0x4000));

	for (INT32 i = 0; i < 0x4000/2; i+=8)
	{
		INT32 x   = ((BURN_ENDIAN_SWAP_INT16(spriteram[i+0]) + 386) - extrax) & 0x3ff;
		INT32 y   = ((BURN_ENDIAN_SWAP_INT16(spriteram[i+1]) +   1) - extray) & 0x1ff;
		INT32 zoomx = BURN_ENDIAN_SWAP_INT16(spriteram[i+2]) & 0x0fff;
		INT32 zoomy = BURN_ENDIAN_SWAP_INT16(spriteram[i+3]) & 0x0fff;
		INT32 prio  =(BURN_ENDIAN_SWAP_INT16(spriteram[i+4]) & 0x0030) << 10;
		INT32 flipx = BURN_ENDIAN_SWAP_INT16(spriteram[i+4]) & 0x0040;
		INT32 flipy = BURN_ENDIAN_SWAP_INT16(spriteram[i+4]) & 0x0080;
		INT32 color = BURN_ENDIAN_SWAP_INT16(spriteram[i+4]) & 0x3f00;
		INT32 addr  =(BURN_ENDIAN_SWAP_INT16(spriteram[i+5]) | ((BURN_ENDIAN_SWAP_INT16(spriteram[i+4]) & 0x000f) << 16)) << 8;
		INT32 ysize =(BURN_ENDIAN_SWAP_INT16(spriteram[i+6]) & 0x00ff) << 4;
		INT32 xsize =(BURN_ENDIAN_SWAP_INT16(spriteram[i+6]) & 0xff00) >> 4;

		if ((BURN_ENDIAN_SWAP_INT16(spriteram[i+4]) & 0x8000) || !zoomx || !zoomy) continue;

		if (x & 0x200) x -= 0x400; // positions are signed
		if (y & 0x100) y -= 0x200;

		if (zoomx == 0x100 && zoomy == 0x100) {
			draw_sprite_nozoom(addr, color + prio, x, y, flipx, flipy, xsize, ysize);
		} else {
			draw_sprite_zoomed(addr, color + prio, x, y, flipx, flipy, xsize, ysize, zoomx, zoomy);
		}
	}
}

static void blend_sprites_and_transfer()
{
	UINT16 *srcptr  = pTransDraw;
	UINT16 *srcptr2 = DrvTmpSprites;
	UINT8  *dstptr  = pBurnDraw;
	UINT16 *palram  = (UINT16*)DrvPalRAM;

	for (INT32 z = 0; z < 320 * 240; z++)
	{
		if (!(srcptr2[z] & 0x3fff))	// tiles only, copy
		{
			PutPix(dstptr + z * nBurnBpp, DrvPalette[(srcptr[z] & 0x7ff) + 0x4000]);
			continue;
		}

		// check for blend/priority

		INT32 pxl  =(srcptr [z] & 0x07ff) + 0x4000;
		INT32 pxl2 =(srcptr2[z] & 0x3fff);

		if ((BURN_ENDIAN_SWAP_INT16(palram[pxl]) & 0x8000) && (BURN_ENDIAN_SWAP_INT16(palram[pxl2]) & 0x8000)) // blend
		{
			INT32 colour  = DrvPalette24[pxl];
			INT32 colour2 = DrvPalette24[pxl2];

			INT32 b = ((colour & 0x000000ff) + (colour2 & 0x000000ff)) >>  1;
			INT32 g = ((colour & 0x0000ff00) + (colour2 & 0x0000ff00)) >>  9;
			INT32 r = ((colour & 0x00ff0000) + (colour2 & 0x00ff0000)) >> 17;

			PutPix(dstptr + z * nBurnBpp, BurnHighCol(r, g, b, 0));
		}
		else if ((srcptr2[z] & 0xc000) >= (srcptr[z] & 0xc000))
		{
			PutPix(dstptr + z * nBurnBpp, DrvPalette[pxl2]);
		}
		else
		{
			PutPix(dstptr + z * nBurnBpp, DrvPalette[pxl]);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x9000/2; i++) {
			palette_update(i);
		}
		DrvRecalc = 0;
	}

	BurnTransferClear();

	if (!deroon) { // deroon does not have tiles for this layer, so don't bother drawing it...
		draw_background_layer(DrvBgRAM0, DrvGfxROM1, DrvC8Regs, 16, 104, 0x0000);
	}
	draw_background_layer(DrvBgRAM1, DrvGfxROM2, DrvA8Regs, 17, 106, 0x4000);
	draw_background_layer(DrvBgRAM2, DrvGfxROM3, DrvB0Regs, 17, 106, 0x8000);
	draw_character_layer();

	blend_sprites_and_transfer();

	draw_sprites(); // draw next frame's sprites

	return 0;
}

static INT32 DrvFrame()
{
	SekNewFrame();
#ifdef ENABLE_SOUND_HARDWARE
	ZetNewFrame();
#endif

	watchdog++;
	if (watchdog >= 400) { //?
		DrvDoReset(0);
	}

	if (DrvReset) {
		DrvDoReset(1);
	}

	{
		memset (DrvInputs, 0xff, 2 * sizeof(UINT16));

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		// clear opposites
		if ((DrvInputs[0] & 0x03) == 0x00) DrvInputs[0] |= 0x03;
		if ((DrvInputs[0] & 0x0c) == 0x00) DrvInputs[0] |= 0x0c;
		if ((DrvInputs[1] & 0x03) == 0x00) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0x00) DrvInputs[1] |= 0x0c;
	}

	INT32 nSegment;
	INT32 nInterleave = 256;
#ifdef ENABLE_SOUND_HARDWARE
	INT32 nSoundBufferPos = 0;
#endif
	INT32 nCyclesTotal[2] = { 1600000000 / 5745, 800000000 / 5745 }; // 57.4458hz
	INT32 nCyclesDone[2] = { 0, 0 };

	nCyclesTotal[0] = (INT32)((INT64)nCyclesTotal[0] * nBurnCPUSpeedAdjust / 0x0100);
	nCyclesTotal[1] = (INT32)((INT64)nCyclesTotal[1] * nBurnCPUSpeedAdjust / 0x0100);

	SekOpen(0);
#ifdef ENABLE_SOUND_HARDWARE
	ZetOpen(0);
#endif

	vblank = 1;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 240) {
			vblank = 0;
			SekSetIRQLine(1, SEK_IRQSTATUS_ACK);
		}

		nSegment = nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += SekRun(nSegment);

#ifdef ENABLE_SOUND_HARDWARE
		nCyclesDone[1] += ZetRun((SekTotalCycles() / 2) -  ZetTotalCycles());

		if (pBurnSoundOut) {
			nSegment = nBurnSoundLen / nInterleave;
			YMZ280BRender(pBurnSoundOut + nSoundBufferPos, nSegment);
			MSM6295Render(0, pBurnSoundOut + nSoundBufferPos, nSegment);

			nSoundBufferPos += nSegment << 1;
		}
#endif
	}

	SekSetIRQLine(1, SEK_IRQSTATUS_NONE);

#ifdef ENABLE_SOUND_HARDWARE
	if (pBurnSoundOut) {
		nSegment = nBurnSoundLen - nSoundBufferPos;
		if (nSegment > 0) {
			YMZ280BRender(pBurnSoundOut + nSoundBufferPos, nSegment);
			MSM6295Render(0, pBurnSoundOut + nSoundBufferPos, nSegment);
		}
	}

	ZetClose();
#endif
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin =  0x029702;
	}

	DrvRecalc = 1;

	if (nAction & ACB_MEMORY_ROM) {	
		ba.Data		= Drv68KROM;
		ba.nLen		= 0x100000;
		ba.nAddress	= 0;
		ba.szName	= "68K ROM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_MEMORY_RAM) {	
		ba.Data		= Drv68KRAM;
		ba.nLen		= 0x010000;
		ba.nAddress	= 0x200000;
		ba.szName	= "68K RAM";
		BurnAcb(&ba);

		ba.Data		= DrvBgRAM0;
		ba.nLen		= 0x0001000;
		ba.nAddress	= 0x300000;
		ba.szName	= "Background RAM";
		BurnAcb(&ba);

		ba.Data		= DrvBgScrRAM0;
		ba.nLen		= 0x000400;
		ba.nAddress	= 0x301000;
		ba.szName	= "Background Scroll RAM";
		BurnAcb(&ba);

		ba.Data		= DrvBgRAM1;
		ba.nLen		= 0x0001000;
		ba.nAddress	= 0x400000;
		ba.szName	= "Midground RAM";
		BurnAcb(&ba);

		ba.Data		= DrvBgScrRAM1;
		ba.nLen		= 0x000400;
		ba.nAddress	= 0x401000;
		ba.szName	= "Midground Scroll RAM";
		BurnAcb(&ba);

		ba.Data		= DrvBgRAM2;
		ba.nLen		= 0x0001000;
		ba.nAddress	= 0x500000;
		ba.szName	= "Foreground RAM";
		BurnAcb(&ba);

		ba.Data		= DrvBgScrRAM2;
		ba.nLen		= 0x000400;
		ba.nAddress	= 0x501000;
		ba.szName	= "Foreground Scroll RAM";
		BurnAcb(&ba);

		ba.Data		= DrvTxtRAM;
		ba.nLen		= 0x004000;
		ba.nAddress	= 0x700000;
		ba.szName	= "Text RAM";
		BurnAcb(&ba);

		ba.Data		= DrvSprRAM;
		ba.nLen		= 0x001000;
		ba.nAddress	= 0x800000;
		ba.szName	= "Sprite RAM";
		BurnAcb(&ba);

		ba.Data		= Drv88Regs;
		ba.nLen		= 0x000004;
		ba.nAddress	= 0x880000;
		ba.szName	= "880000 Registers";
		BurnAcb(&ba);

		ba.Data		= DrvPalRAM;
		ba.nLen		= 0x008000;
		ba.nAddress	= 0x900000;
		ba.szName	= "Sprite Palette RAM";
		BurnAcb(&ba);

		ba.Data		= DrvPalRAM;
		ba.nLen		= 0x001000;
		ba.nAddress	= 0x980000;
		ba.szName	= "Layer Palette RAM";
		BurnAcb(&ba);

		ba.Data		= DrvA8Regs;
		ba.nLen		= 0x000006;
		ba.nAddress	= 0xa80000;
		ba.szName	= "A80000 Registers";
		BurnAcb(&ba);

		ba.Data		= DrvB0Regs;
		ba.nLen		= 0x000006;
		ba.nAddress	= 0xb00000;
		ba.szName	= "B00000 Registers";
		BurnAcb(&ba);

		ba.Data		= DrvC0Regs;
		ba.nLen		= 0x000006;
		ba.nAddress	= 0xc00000;
		ba.szName	= "C00000 Registers";
		BurnAcb(&ba);

		ba.Data		= DrvC8Regs;
		ba.nLen		= 0x000006;
		ba.nAddress	= 0xc80000;
		ba.szName	= "C80000 Registers";
		BurnAcb(&ba);

#ifdef ENABLE_SOUND_HARDWARE
		ba.Data		= DrvZ80RAM;
		ba.nLen		= 0x001800;
		ba.nAddress	= 0xff0000;
		ba.szName	= "Z80 RAM (Not accessible)";
		BurnAcb(&ba);
#endif
	}

	if (nAction & ACB_DRIVER_DATA) {
	
		SekScan(nAction);
#ifdef ENABLE_SOUND_HARDWARE
		ZetScan(nAction);

		// ymf262
		YMZ280BScan();
		MSM6295Scan(0, nAction);
#endif

		EEPROMScan(nAction, pnMin);

		SCAN_VAR(protection_read_pointer);
		SCAN_VAR(protection_status);
		SCAN_VAR(protection_value);

	}

	if (nAction & ACB_WRITE) {
#ifdef ENABLE_SOUND_HARDWARE
		INT32 bank;

		ZetOpen(0);
		bank = *DrvZ80Bank;
		*DrvZ80Bank = ~0;
		bankswitch(bank);
		ZetClose();

		bank = *DrvOkiBank;
		*DrvOkiBank = ~0;
		oki_bankswitch(*DrvOkiBank);
#endif
	}

 	return 0;
}


// Deroon DeroDero

static struct BurnRomInfo deroonRomDesc[] = {
	{ "t001.upau1",			0x080000, 0x14b92c18, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "t002.upal1",			0x080000, 0x0fb05c68, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "t003.uz1",			0x040000, 0x8bdfafa0, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "t101.uah1",			0x200000, 0x74baf845, 3 | BRF_GRA },           //  3 Sprites
	{ "t102.ual1",			0x200000, 0x1a02c4a3, 3 | BRF_GRA },           //  4
	{ "t103.ubl1",			0x400000, 0x84e7da88, 3 | BRF_GRA },           //  5
	{ "t104.ucl1",			0x200000, 0x66eb611a, 3 | BRF_GRA },           //  6

	{ "t301.ubd1",			0x100000, 0x8b026177, 4 | BRF_GRA },           //  7 Character Tiles

	{ "t201.ubb1",			0x100000, 0xd5a087ac, 6 | BRF_GRA },           //  8 Midground Layer

	{ "t202.ubc1",			0x100000, 0xf051dae1, 7 | BRF_GRA },           //  9 Foreground Layer

	{ "t401.uya1",			0x200000, 0x92111992, 8 | BRF_SND },           // 10 YMZ280B Samples

	{ "t501.uad1",			0x080000, 0x2fbcfe27, 9 | BRF_SND },           // 11 OKI6295 Samples

	{ "deroon_68hc11a8.rom",	0x002000, 0x00000000, 0 | BRF_NODUMP },        // 12 68HC11A8 Code
	{ "deroon_68hc11a8.eeprom",	0x000200, 0x00000000, 0 | BRF_NODUMP },        // 13 68HC11A8 EEPROM
};

STD_ROM_PICK(deroon)
STD_ROM_FN(deroon)

static INT32 DeroonRomCallback()
{
	if (BurnLoadRom(Drv68KROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x0000000,  1, 2)) return 1;

#ifdef ENABLE_SOUND_HARDWARE
	if (BurnLoadRom(DrvZ80ROM  + 0x0000000,  2, 1)) return 1;
#endif

	if (BurnLoadRom(DrvSprROM + 0x0000000,   3, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x0000001,   4, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x0800001,   5, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x1000001,   6, 2)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x0000000,  7, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x0000000,  8, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x0000000,  9, 1)) return 1;

#ifdef ENABLE_SOUND_HARDWARE
	if (BurnLoadRom(DrvSndROM1 + 0x0000000, 10, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x0080000, 11, 1)) return 1;
#endif

	return 0;
}

static INT32 DeroonInit()
{
	return CommonInit(DeroonRomCallback, 0x2000000, 0x200000, &deroon_data, 1);
}

struct BurnDriver BurnDrvDeroon = {
	"deroon", NULL, NULL, NULL, "1995",
	"Deroon DeroDero\0", "No sound", "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, deroonRomInfo, deroonRomName, NULL, NULL, DrvInputInfo, NULL,
	DeroonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x4800,
	320, 240, 4, 3
};


// Toukidenshou - Angel Eyes (VER. 960614)

static struct BurnRomInfo tkdenshoRomDesc[] = {
	{ "aeprge-2.pal",		0x080000, 0x25e453d6, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "aeprgo-2.pau",		0x080000, 0x22d59510, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "aesprg-2.z1",		0x020000, 0x43550ab6, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "ae100h.ah1",			0x400000, 0x06be252b, 3 | BRF_GRA },           //  3 Sprites
	{ "ae100.al1",			0x400000, 0x009cdff4, 3 | BRF_GRA },           //  4
	{ "ae101h.bh1",			0x400000, 0xf2469eff, 3 | BRF_GRA },           //  5
	{ "ae101.bl1",			0x400000, 0xdb7791bb, 3 | BRF_GRA },           //  6
	{ "ae102h.ch1",			0x200000, 0xf9d2a343, 3 | BRF_GRA },           //  7
	{ "ae102.cl1",			0x200000, 0x681be889, 3 | BRF_GRA },           //  8
	{ "ae104.el1",			0x400000, 0xe431b798, 3 | BRF_GRA },           //  9
	{ "ae105.fl1",			0x400000, 0xb7f9ebc1, 3 | BRF_GRA },           // 10
	{ "ae106.gl1",			0x200000, 0x7c50374b, 3 | BRF_GRA },           // 11

	{ "ae300w36.bd1",		0x080000, 0xe829f29e, 4 | BRF_GRA },           // 12 Character Tiles

	{ "ae200w74.ba1",		0x100000, 0xc1645041, 5 | BRF_GRA },           // 13 Background Tiles

	{ "ae201w75.bb1",		0x100000, 0x3f63bdff, 6 | BRF_GRA },           // 14 Midground Tiles

	{ "ae202w76.bc1",		0x100000, 0x5cc857ca, 7 | BRF_GRA },           // 15 Foreground Tiles

	{ "ae400t23.ya1",		0x200000, 0xc6ffb043, 8 | BRF_SND },           // 16 YMZ280B Samples
	{ "ae401t24.yb1",		0x200000, 0xd83f1a73, 8 | BRF_SND },           // 17

	{ "ae500w07.ad1",		0x080000, 0x3734f92c, 9 | BRF_SND },           // 18 OKI6295 Samples

	{ "tkdensho_68hc11a8.rom",	0x002000, 0x00000000, 0 | BRF_NODUMP },        // 19 68HC11A8 Code
	{ "tkdensho_68hc11a8.eeprom",	0x000200, 0x00000000, 0 | BRF_NODUMP },        // 20 68HC11A8 EEPROM
};

STD_ROM_PICK(tkdensho)
STD_ROM_FN(tkdensho)

static INT32 TkdenshoRomCallback()
{
	if (BurnLoadRom(Drv68KROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x0000000,  1, 2)) return 1;

#ifdef ENABLE_SOUND_HARDWARE
	if (BurnLoadRom(DrvZ80ROM  + 0x0000000,  2, 1)) return 1;
	memcpy (DrvZ80ROM + 0x20000, DrvZ80ROM, 0x20000);
#endif

	if (BurnLoadRom(DrvSprROM + 0x0000000,   3, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x0000001,   4, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x0800000,   5, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x0800001,   6, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x1000000,   7, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x1000001,   8, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x2000001,   9, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x2800001,  10, 2)) return 1;
	if (BurnLoadRom(DrvSprROM + 0x3000001,  11, 2)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x0000000, 12, 1)) return 1;
	memcpy (DrvGfxROM0 + 0x80000, DrvGfxROM0, 0x80000); // double here rather than mask later

	if (BurnLoadRom(DrvGfxROM1 + 0x0000000, 13, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x0000000, 14, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM3 + 0x0000000, 15, 1)) return 1;

#ifdef ENABLE_SOUND_HARDWARE
	if (BurnLoadRom(DrvSndROM1 + 0x0000000, 16, 1)) return 1;
	if (BurnLoadRom(DrvSndROM1 + 0x0200000, 17, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x0080000, 18, 1)) return 1;
#endif

	return 0;
}

static INT32 TkdenshoInit()
{
	return CommonInit(TkdenshoRomCallback, 0x4000000, 0x400000, &tkdensho_data, 0);
}

struct BurnDriver BurnDrvTkdensho = {
	"tkdensho", NULL, NULL, NULL, "1996",
	"Toukidenshou - Angel Eyes (VER. 960614)\0", "No sound", "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, tkdenshoRomInfo, tkdenshoRomName, NULL, NULL, DrvInputInfo, NULL,
	TkdenshoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x4800,
	320, 240, 4, 3
};


// Toukidenshou - Angel Eyes (VER. 960427)

static struct BurnRomInfo tkdenshoaRomDesc[] = {
	{ "aeprge.pal",			0x080000, 0x17a209ff, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "aeprgo.pau",			0x080000, 0xd265e6a1, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "aesprg-2.z1",		0x020000, 0x43550ab6, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "ae100h.ah1",			0x400000, 0x06be252b, 3 | BRF_GRA },           //  3 Sprites
	{ "ae100.al1",			0x400000, 0x009cdff4, 3 | BRF_GRA },           //  4
	{ "ae101h.bh1",			0x400000, 0xf2469eff, 3 | BRF_GRA },           //  5
	{ "ae101.bl1",			0x400000, 0xdb7791bb, 3 | BRF_GRA },           //  6
	{ "ae102h.ch1",			0x200000, 0xf9d2a343, 3 | BRF_GRA },           //  7
	{ "ae102.cl1",			0x200000, 0x681be889, 3 | BRF_GRA },           //  8
	{ "ae104.el1",			0x400000, 0xe431b798, 3 | BRF_GRA },           //  9
	{ "ae105.fl1",			0x400000, 0xb7f9ebc1, 3 | BRF_GRA },           // 10
	{ "ae106.gl1",			0x200000, 0x7c50374b, 3 | BRF_GRA },           // 11

	{ "ae300w36.bd1",		0x080000, 0xe829f29e, 4 | BRF_GRA },           // 12 Character Tiles

	{ "ae200w74.ba1",		0x100000, 0xc1645041, 5 | BRF_GRA },           // 13 Background Tiles

	{ "ae201w75.bb1",		0x100000, 0x3f63bdff, 6 | BRF_GRA },           // 14 Midground Tiles

	{ "ae202w76.bc1",		0x100000, 0x5cc857ca, 7 | BRF_GRA },           // 15 Foreground Tiles

	{ "ae400t23.ya1",		0x200000, 0xc6ffb043, 8 | BRF_SND },           // 16 YMZ280B Samples
	{ "ae401t24.yb1",		0x200000, 0xd83f1a73, 8 | BRF_SND },           // 17

	{ "ae500w07.ad1",		0x080000, 0x3734f92c, 9 | BRF_SND },           // 18 OKI6295 Samples

	{ "tkdensho_68hc11a8.rom",	0x002000, 0x00000000, 0 | BRF_NODUMP },        // 19 68HC11A8 Code
	{ "tkdensho_68hc11a8.eeprom",	0x000200, 0x00000000, 0 | BRF_NODUMP },        // 20 68HC11A8 EEPROM
};

STD_ROM_PICK(tkdenshoa)
STD_ROM_FN(tkdenshoa)

static INT32 TkdenshoaInit()
{
	return CommonInit(TkdenshoRomCallback, 0x4000000, 0x400000, &tkdenshoa_data, 0);
}

struct BurnDriver BurnDrvTkdenshoa = {
	"tkdenshoa", "tkdensho", NULL, NULL, "1996",
	"Toukidenshou - Angel Eyes (VER. 960427)\0", "No sound", "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, tkdenshoaRomInfo, tkdenshoaRomName, NULL, NULL, DrvInputInfo, NULL,
	TkdenshoaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x4800,
	320, 240, 4, 3
};
