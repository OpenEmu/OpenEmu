#include "tiles_generic.h"
#include "sh2.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[1]        = {0};
static UINT8 DrvInput[3]      = {0x00, 0x00, 0x00};
static UINT8 DrvReset         = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *DrvBiosRom          = NULL;
static UINT8 *DrvPrgRom           = NULL;
static UINT8 *DrvSpriteRom        = NULL;
static UINT8 *DrvSampleRom        = NULL;
static UINT8 *DrvPrgRam           = NULL;
static UINT8 *DrvBackupRam        = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvSpriteRegs       = NULL;
static UINT8 *DrvTileARam         = NULL;
static UINT8 *DrvTileBRam         = NULL;
static UINT8 *DrvTileBTilesRam    = NULL;
static UINT8 *DrvTileRegs         = NULL;
static UINT8 *DrvTileLineRam      = NULL;
static UINT8 *DrvCacheRam         = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvPaletteRegs      = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT8 *DrvTilesA8Bpp       = NULL;
static UINT8 *DrvTilesB8Bpp       = NULL;
static UINT8 *DrvTilesA4Bpp       = NULL;
static UINT8 *DrvTilesB4Bpp       = NULL;
static UINT32 *DrvPalette          = NULL;

static INT32 nCyclesDone[1], nCyclesTotal[1];
static INT32 nCyclesSegment;

static struct BurnInputInfo CyvernInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 6, "service"   },
	{"Tilt"              , BIT_DIGITAL  , DrvInputPort2 + 5, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
};

STDINPUTINFO(Cyvern)

static inline void DrvClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = DrvInput[2] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] |= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] |= (DrvInputPort2[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[0]);
	DrvClearOpposites(&DrvInput[1]);
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x13, 0x01, 0x02, 0x02, "Off"                    },
	{0x13, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Use backup RAM"         },
	{0x13, 0x01, 0x40, 0x00, "No"                     },
	{0x13, 0x01, 0x40, 0x40, "Yes"                    },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Drv)

static struct BurnRomInfo CyvernRomDesc[] = {
	{ "sknsj1.u10",    0x080000, 0x7e2b836c, BRF_ESS | BRF_PRG }, //  0	BIOS
	
	{ "cvj-even.u10",  0x100000, 0x802fadb4, BRF_ESS | BRF_PRG }, //  1	SH-2 Program
	{ "cvj-odd.u8",    0x100000, 0xf8a0fbdd, BRF_ESS | BRF_PRG }, //  2
	
	{ "cv100-00.u24",  0x400000, 0xcd4ae88a, BRF_GRA },	     //   3	Sprites
	{ "cv101-00.u20",  0x400000, 0xa6cb3f0b, BRF_GRA },	     //   4
	
	{ "cv200-00.u16",  0x400000, 0xddc8c67e, BRF_GRA },	     //   5	Tiles Plane A
	{ "cv201-00.u13",  0x400000, 0x65863321, BRF_GRA },	     //   6
	
	{ "cv210-00.u18",  0x400000, 0x7486bf3a, BRF_GRA },	     //   7	Tiles Plane B

	{ "cv300-00.u4",   0x400000, 0xfbeda465, BRF_SND },	     //   8	Samples
};

STD_ROM_PICK(Cyvern)
STD_ROM_FN(Cyvern)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvBiosRom             = Next; Next += 0x080000;
	DrvPrgRom              = Next; Next += 0x200000;
	DrvSpriteRom           = Next; Next += 0x800000;
	DrvSampleRom           = Next; Next += 0x400000;

	RamStart               = Next;

	DrvPrgRam              = Next; Next += 0x100000;
	DrvBackupRam           = Next; Next += 0x002000;
	DrvSpriteRam           = Next; Next += 0x004000;
	DrvSpriteRegs          = Next; Next += 0x000040;
	DrvTileARam            = Next; Next += 0x004000;
	DrvTileBRam            = Next; Next += 0x004000;
	DrvTileBTilesRam       = Next; Next += 0x040000;
	DrvTileRegs            = Next; Next += 0x000080;
	DrvTileLineRam         = Next; Next += 0x008000;
	DrvPaletteRam          = Next; Next += 0x020000;
	DrvPaletteRegs         = Next; Next += 0x000020;
	DrvCacheRam            = Next; Next += 0x001000;

	RamEnd                 = Next;

	DrvTilesA8Bpp          = Next; Next += 0x08000 * 16 * 16;
	DrvTilesB8Bpp          = Next; Next += 0x08000 * 16 * 16;
	DrvTilesA4Bpp          = Next; Next += 0x10000 * 16 * 16;
	DrvTilesB4Bpp          = Next; Next += 0x10000 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x08000 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static struct {
	UINT16 x1p, y1p, z1p, x1s, y1s, z1s;
	UINT16 x2p, y2p, z2p, x2s, y2s, z2s;
	UINT16 org;

	UINT16 x1_p1, x1_p2, y1_p1, y1_p2, z1_p1, z1_p2;
	UINT16 x2_p1, x2_p2, y2_p1, y2_p2, z2_p1, z2_p2;
	UINT16 x1tox2, y1toy2, z1toz2;
	INT16 x_in, y_in, z_in;
	UINT16 flag;

	UINT8 disconnect;
} hit;

static void hit_calc_orig(UINT16 p, UINT16 s, UINT16 org, UINT16 *l, UINT16 *r)
{
	switch(org & 3) {
	case 0:
		*l = p;
		*r = p+s;
	break;
	case 1:
		*l = p-s/2;
		*r = *l+s;
	break;
	case 2:
		*l = p-s;
		*r = p;
	break;
	case 3:
		*l = p-s;
		*r = p+s;
	break;
	}
}

static void hit_calc_axis(UINT16 x1p, UINT16 x1s, UINT16 x2p, UINT16 x2s, UINT16 org,
			  UINT16 *x1_p1, UINT16 *x1_p2, UINT16 *x2_p1, UINT16 *x2_p2,
			  INT16 *x_in, UINT16 *x1tox2)
{
	UINT16 x1l=0, x1r=0, x2l=0, x2r=0;
	hit_calc_orig(x1p, x1s, org,      &x1l, &x1r);
	hit_calc_orig(x2p, x2s, org >> 8, &x2l, &x2r);

	*x1tox2 = x2p-x1p;
	*x1_p1 = x1p;
	*x2_p1 = x2p;
	*x1_p2 = x1r;
	*x2_p2 = x2l;
	*x_in = x1r-x2l;
}

static void hit_recalc(void)
{
	hit_calc_axis(hit.x1p, hit.x1s, hit.x2p, hit.x2s, hit.org,
		&hit.x1_p1, &hit.x1_p2, &hit.x2_p1, &hit.x2_p2,
		&hit.x_in, &hit.x1tox2);
	hit_calc_axis(hit.y1p, hit.y1s, hit.y2p, hit.y2s, hit.org,
		&hit.y1_p1, &hit.y1_p2, &hit.y2_p1, &hit.y2_p2,
		&hit.y_in, &hit.y1toy2);
	hit_calc_axis(hit.z1p, hit.z1s, hit.z2p, hit.z2s, hit.org,
		&hit.z1_p1, &hit.z1_p2, &hit.z2_p1, &hit.z2_p2,
		&hit.z_in, &hit.z1toz2);

	hit.flag = 0;
	hit.flag |= hit.y2p > hit.y1p ? 0x8000 : hit.y2p == hit.y1p ? 0x4000 : 0x2000;
	hit.flag |= hit.y_in >= 0 ? 0 : 0x1000;
	hit.flag |= hit.x2p > hit.x1p ? 0x0800 : hit.x2p == hit.x1p ? 0x0400 : 0x0200;
	hit.flag |= hit.x_in >= 0 ? 0 : 0x0100;
	hit.flag |= hit.z2p > hit.z1p ? 0x0080 : hit.z2p == hit.z1p ? 0x0040 : 0x0020;
	hit.flag |= hit.z_in >= 0 ? 0 : 0x0010;
	hit.flag |= hit.x_in >= 0 && hit.y_in >= 0 && hit.z_in >= 0 ? 8 : 0;
	hit.flag |= hit.z_in >= 0 && hit.x_in >= 0                  ? 4 : 0;
	hit.flag |= hit.y_in >= 0 && hit.z_in >= 0                  ? 2 : 0;
	hit.flag |= hit.x_in >= 0 && hit.y_in >= 0                  ? 1 : 0;
}

static void SknsHitWrite(UINT32 offset, UINT32 data)
{
	INT32 adr = offset * 4;

	switch(adr) {
	case 0x00:
	case 0x28:
		hit.x1p = data;
	break;
	case 0x08:
	case 0x30:
		hit.y1p = data;
	break;
	case 0x38:
	case 0x50:
		hit.z1p = data;
	break;
	case 0x04:
	case 0x2c:
		hit.x1s = data;
	break;
	case 0x0c:
	case 0x34:
		hit.y1s = data;
	break;
	case 0x3c:
	case 0x54:
		hit.z1s = data;
	break;
	case 0x10:
	case 0x58:
		hit.x2p = data;
	break;
	case 0x18:
	case 0x60:
		hit.y2p = data;
	break;
	case 0x20:
	case 0x68:
		hit.z2p = data;
	break;
	case 0x14:
	case 0x5c:
		hit.x2s = data;
	break;
	case 0x1c:
	case 0x64:
		hit.y2s = data;
	break;
	case 0x24:
	case 0x6c:
		hit.z2s = data;
	break;
	case 0x70:
		hit.org = data;
	break;
	default:
//      log_write("HIT", adr, data, type);
	break;
	}
	hit_recalc();
}

static void SknsHit2Write(UINT32 offset, UINT32 data)
{
//  log_event("HIT", "Set disconnect to %02x", data);
	hit.disconnect = data;
}

UINT32 __fastcall SknsHitRead(UINT32 offset)
{
	INT32 adr = offset *4;

//  log_read("HIT", adr, type);

	if(hit.disconnect)
		return 0x0000;
	switch(adr) {
	case 0x28:
	case 0x2a:
		return rand() & 0xffff;
	case 0x00:
	case 0x10:
		return (UINT16)hit.x_in;
	case 0x04:
	case 0x14:
		return (UINT16)hit.y_in;
	case 0x18:
		return (UINT16)hit.z_in;
	case 0x08:
	case 0x1c:
		return hit.flag;
	case 0x40:
		return hit.x1p;
	case 0x48:
		return hit.y1p;
	case 0x50:
		return hit.z1p;
	case 0x44:
		return hit.x1s;
	case 0x4c:
		return hit.y1s;
	case 0x54:
		return hit.z1s;
	case 0x58:
		return hit.x2p;
	case 0x60:
		return hit.y2p;
	case 0x68:
		return hit.z2p;
	case 0x5c:
		return hit.x2s;
	case 0x64:
		return hit.y2s;
	case 0x6c:
		return hit.z2s;
	case 0x70:
		return hit.org;
	case 0x80:
		return hit.x1tox2;
	case 0x84:
		return hit.y1toy2;
	case 0x88:
		return hit.z1toz2;
	case 0x90:
		return hit.x1_p1;
	case 0xa0:
		return hit.y1_p1;
	case 0xb0:
		return hit.z1_p1;
	case 0x98:
		return hit.x1_p2;
	case 0xa8:
		return hit.y1_p2;
	case 0xb8:
		return hit.z1_p2;
	case 0x94:
		return hit.x2_p1;
	case 0xa4:
		return hit.y2_p1;
	case 0xb4:
		return hit.z2_p1;
	case 0x9c:
		return hit.x2_p2;
	case 0xac:
		return hit.y2_p2;
	case 0xbc:
		return hit.z2_p2;
	default:
//      log_read("HIT", adr, type);
	return 0;
	}
}

static INT32 DrvDoReset()
{
	Sh2Open(0);
	Sh2Reset(*(UINT32 *)(DrvBiosRom + 0), *(UINT32 *)(DrvBiosRom + 4));
	Sh2Close();
	
	return 0;
}

UINT8 __fastcall CyvernReadByte(UINT32 a)
{
	switch (a) {
		case 0x20400007: {
			//???
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Read byte => %08X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall CyvernWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x0040000e: {
			// unknown i/o write;
			return;
		}
		
		case 0x00c00000:
		case 0x00c00001: {
			//sound write
			return;
		}
		
		case 0x01800000: {
			SknsHit2Write(0, d);
			return;
		}
		
		case 0x2040000e: {
			//???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write byte => %08X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall CyvernReadWord(UINT32 a)
{
	if (a >= 0x06000000 && a <= 0x06ffffff) {
		if (a >= 0x06000028 && a <= 0x0600002b) bprintf(PRINT_NORMAL, _T("Read Word Bios Skip %x, %x\n"), a, Sh2GetPC(0));
		UINT32 Offset = (a - 0x06000000) / 2;
		UINT16 *Ram = (UINT16*)DrvPrgRam;
		
		return Ram[Offset];
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Read word => %08X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall CyvernWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x05000000: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write word => %08X, %04X\n"), a, d);
		}
	}
}

UINT32 __fastcall CyvernReadLong(UINT32 a)
{
	if (a >= 0x02f00000 && a <= 0x02f000ff) {
		UINT32 Offset = (a - 0x02f00000) / 4;
		
		return SknsHitRead(Offset);
	}
	
	if (a >= 0x06000000 && a <= 0x06ffffff) {
		if (a >= 0x06000028 && a <= 0x0600002b) bprintf(PRINT_NORMAL, _T("Read Long Bios Skip %x, %x\n"), a, Sh2GetPC(0) / 4);
		UINT32 Offset = (a - 0x06000000) / 4;
		UINT32 *Ram = (UINT32*)DrvPrgRam;
		
		return Ram[Offset];
	}

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Read long => %08X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall CyvernWriteLong(UINT32 a, UINT32 d)
{
	if (a >= 0x02f00000 && a <= 0x02f000ff) {
		UINT32 Offset = (a - 0x02f00000) / 4;
		
		SknsHitWrite(Offset, d);
		return;
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Write long => %08X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall BiosSkipReadByte(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Read Bios Skip byte => %08X\n"), a);
		}
	}
	
	return 0;
}

UINT16 __fastcall BiosSkipReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Read Bios Skip word => %08X\n"), a);
		}
	}
	
	return 0;
}

UINT32 __fastcall BiosSkipReadLong(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Read Bios Skip long => %08X\n"), a);
		}
	}
	
	return 0;
}

static void be_to_le(UINT8 * p, INT32 size)
{
	UINT8 c;
	for(INT32 i=0; i<size; i+=4, p+=4) {
		c = *(p+0);	*(p+0) = *(p+3);	*(p+3) = c;
		c = *(p+1);	*(p+1) = *(p+2);	*(p+2) = c;
	}
}

static INT32 CyvernInit()
{
	INT32 nRet = 0, nLen;
	
	BurnSetRefreshRate(59.5971);

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)malloc(0x800000);

	// Load BIOS Rom
	nRet = BurnLoadRom(DrvBiosRom + 0x000000, 0, 1); if (nRet != 0) return 1;
	be_to_le(DrvBiosRom, 0x00080000);
	
	// Load Program Rom
	nRet = BurnLoadRom(DrvPrgRom  + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPrgRom  + 0x000001, 2, 2); if (nRet != 0) return 1;
	be_to_le(DrvPrgRom, 0x00200000);
	
	if (DrvTempRom) {
		free(DrvTempRom);
		DrvTempRom = NULL;
	}
	
	// Setup the 68000 emulation
	Sh2Init(1);
	Sh2Open(0);
	Sh2MapMemory(DrvBiosRom           , 0x00000000, 0x0007ffff, SM_ROM);
	Sh2MapMemory(DrvSpriteRam         , 0x02000000, 0x02003fff, SM_RAM);
	Sh2MapMemory(DrvSpriteRegs        , 0x02100000, 0x0210003f, SM_RAM);
	Sh2MapMemory(DrvTileRegs          , 0x02400000, 0x0240007f, SM_RAM);
	Sh2MapMemory(DrvTileARam          , 0x02500000, 0x02503fff, SM_RAM);
	Sh2MapMemory(DrvTileBRam          , 0x02504000, 0x02507fff, SM_RAM);
	Sh2MapMemory(DrvTileLineRam       , 0x02600000, 0x02607fff, SM_RAM);
	Sh2MapMemory(DrvPaletteRegs       , 0x02a00000, 0x02a0001f, SM_RAM);
	Sh2MapMemory(DrvPaletteRam        , 0x02a40000, 0x02a5ffff, SM_RAM);
	Sh2MapMemory(DrvPrgRom            , 0x04000000, 0x041fffff, SM_ROM);
	Sh2MapMemory(DrvTileBTilesRam     , 0x04800000, 0x0483ffff, SM_RAM);
	Sh2MapMemory(DrvPrgRam            , 0x06000000, 0x06ffffff, SM_RAM);
//	Sh2MapMemory(DrvPrgRam            , 0x06000000, 0x06ffffff, SM_WRITE);
	Sh2MapMemory(DrvCacheRam          , 0xc0000000, 0xc0000fff, SM_RAM);
//	Sh2MapHandler(1                   , 0x06000028, 0x0600002b, SM_READ);
	Sh2SetReadByteHandler (0, CyvernReadByte);
	Sh2SetReadWordHandler (0, CyvernReadWord);
	Sh2SetReadLongHandler (0, CyvernReadLong);
	Sh2SetWriteByteHandler(0, CyvernWriteByte);
	Sh2SetWriteWordHandler(0, CyvernWriteWord);
	Sh2SetWriteLongHandler(0, CyvernWriteLong);
//	Sh2SetReadByteHandler (1, BiosSkipReadByte);
//	Sh2SetReadWordHandler (1, BiosSkipReadWord);
//	Sh2SetReadLongHandler (1, BiosSkipReadLong);
	Sh2Close();
	
	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	Sh2Exit();
	
	GenericTilesExit();
	
	BurnFree(Mem);

	return 0;
}

static void DrvCalcPalette()
{
	UINT32 *PaletteRam = (UINT32*)DrvPaletteRam;
	
	for (INT32 Offset = 0; Offset <= 32768; Offset++) {
		INT32 r = (PaletteRam[Offset] >>  0) & 0x1f;
		INT32 g = (PaletteRam[Offset] >>  5) & 0x1f;
		INT32 b = (PaletteRam[Offset] >> 10) & 0x1f;
		
		r <<= 3;
		g <<= 3;
		b <<= 3;
		
		DrvPalette[Offset] = BurnHighCol(r, g, b, 0);
	}
}

static void DrvRenderTileALayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;
	
	UINT32 *VideoRam = (UINT32*)DrvTileARam;
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = VideoRam[TileIndex] & 0x001fffff;
			Colour = (VideoRam[TileIndex] & 0x3f000000) >> 24;
			
			if (Code) bprintf(PRINT_NORMAL, _T("%x, %x\n"), Code, Colour);
			
			x = 16 * mx;
			y = 16 * my;
			
//			x -= DrvFgScrollX;
//			y -= DrvFgScrollY;
			if (x < -16) x += 1024;
			if (y < -16) y += 1024;
			
			if (x > 16 && x < 304 && y > 16 && y < 224) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 8, 0, 0, DrvTilesA8Bpp);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 8, 0, 0, DrvTilesA8Bpp);
			}
			
			TileIndex++;
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	
	DrvRenderTileALayer();
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;

	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = 28638000 / 60;
	nCyclesDone[0] = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		Sh2Open(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += Sh2Run(nCyclesSegment);
		if (i == 5) Sh2SetIRQLine(1, SH2_IRQSTATUS_AUTO);
		if (i == 9) Sh2SetIRQLine(5, SH2_IRQSTATUS_AUTO);
		Sh2Close();
	}
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029693;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	return 0;
}

struct BurnDriver BurnDrvCyvern = {
	"cyvern", NULL, NULL, NULL, "1998",
	"Cyvern (Japan)\0", NULL, "Kaneko", "Super Kaneko Nova System",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_KANEKO_MISC, GBF_VERSHOOT, 0,
	NULL, CyvernRomInfo, CyvernRomName, NULL, NULL, CyvernInputInfo, DrvDIPInfo,
	CyvernInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x8000, 240, 320, 3, 4
};
