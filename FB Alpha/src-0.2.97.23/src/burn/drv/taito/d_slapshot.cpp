#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "taito.h"
#include "taito_ic.h"
#include "burn_ym2610.h"
#include "eeprom.h"
#include "burn_gun.h"
#include "timekpr.h"

static void SlapshotDraw();
static void Opwolf3Draw();
static TaitoF2SpriteBufferUpdate TaitoF2SpriteBufferFunction;

static bool bUseAsm68KCoreOldValue = false;

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo SlapshotInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0640FIOInputPort1 + 4, "p1 coin"        },
	{"Start 1"           , BIT_DIGITAL   , TC0640FIOInputPort3 + 4, "p1 start"       },
	{"Start 2"           , BIT_DIGITAL   , TC0640FIOInputPort3 + 5, "p2 start"       },

	{"P1 Up"             , BIT_DIGITAL   , TC0640FIOInputPort4 + 0, "p1 up"          },
	{"P1 Down"           , BIT_DIGITAL   , TC0640FIOInputPort4 + 1, "p1 down"        },
	{"P1 Left"           , BIT_DIGITAL   , TC0640FIOInputPort4 + 2, "p1 left"        },
	{"P1 Right"          , BIT_DIGITAL   , TC0640FIOInputPort4 + 3, "p1 right"       },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 0, "p1 fire 1"      },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 1, "p1 fire 2"      },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 2, "p1 fire 3"      },
	
	{"P2 Up"             , BIT_DIGITAL   , TC0640FIOInputPort4 + 4, "p2 up"          },
	{"P2 Down"           , BIT_DIGITAL   , TC0640FIOInputPort4 + 5, "p2 down"        },
	{"P2 Left"           , BIT_DIGITAL   , TC0640FIOInputPort4 + 6, "p2 left"        },
	{"P2 Right"          , BIT_DIGITAL   , TC0640FIOInputPort4 + 7, "p2 right"       },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 4, "p2 fire 1"      },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 5, "p2 fire 2"      },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 6, "p2 fire 3"      },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"          },
	{"Service"           , BIT_DIGITAL   , TC0640FIOInputPort3 + 0, "service"        },
	{"Tilt"              , BIT_DIGITAL   , TC0640FIOInputPort1 + 1, "tilt"           },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0           , "dip"            },
};

STDINPUTINFO(Slapshot)

static struct BurnInputInfo Opwolf3InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL   , TC0640FIOInputPort1 + 4, "p1 coin"        },
	{"Start 1"           , BIT_DIGITAL   , TC0640FIOInputPort2 + 2, "p1 start"       },
	{"Coin 2"            , BIT_DIGITAL   , TC0640FIOInputPort1 + 5, "p2 coin"        },
	{"Start 2"           , BIT_DIGITAL   , TC0640FIOInputPort2 + 6, "p2 start"       },
	{"Coin 3"            , BIT_DIGITAL   , TC0640FIOInputPort1 + 6, "p3 coin"        },
	{"Coin 4"            , BIT_DIGITAL   , TC0640FIOInputPort1 + 7, "p4 coin"        },

	A("P1 Gun X"         , BIT_ANALOG_REL, &TaitoAnalogPort0      , "mouse x-axis"   ),
	A("P1 Gun Y"         , BIT_ANALOG_REL, &TaitoAnalogPort1      , "mouse y-axis"   ),
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 0, "mouse button 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 1, "mouse button 2" },
	
	A("P2 Gun X"         , BIT_ANALOG_REL, &TaitoAnalogPort2      , "p2 x-axis"      ),
	A("P2 Gun Y"         , BIT_ANALOG_REL, &TaitoAnalogPort3      , "p2 y-axis"      ),	
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 4, "p2 fire 1"      },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0640FIOInputPort2 + 5, "p2 fire 2"      },
	
	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"          },
	{"Service"           , BIT_DIGITAL   , TC0640FIOInputPort3 + 0, "service"        },
	{"Tilt"              , BIT_DIGITAL   , TC0640FIOInputPort1 + 1, "tilt"           },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0           , "dip"            },
};

STDINPUTINFO(Opwolf3)

#undef A

static void TC0640FIOMakeInputs()
{
	TC0640FIOInput[0] = 0xff;
	TC0640FIOInput[1] = 0xff;
	TC0640FIOInput[2] = 0xff;
	TC0640FIOInput[3] = 0xff;
	TC0640FIOInput[4] = 0xff;
	
	for (INT32 i = 0; i < 8; i++) {
		TC0640FIOInput[0] -= (TC0640FIOInputPort0[i] & 1) << i;
		TC0640FIOInput[1] -= (TC0640FIOInputPort1[i] & 1) << i;
		TC0640FIOInput[2] -= (TC0640FIOInputPort2[i] & 1) << i;
		TC0640FIOInput[3] -= (TC0640FIOInputPort3[i] & 1) << i;
		TC0640FIOInput[4] -= (TC0640FIOInputPort4[i] & 1) << i;
	}
}

static void Opwolf3MakeInputs()
{
	TC0640FIOMakeInputs();
	
	BurnGunMakeInputs(0, (INT16)TaitoAnalogPort0, (INT16)TaitoAnalogPort1);
	BurnGunMakeInputs(1, (INT16)TaitoAnalogPort2, (INT16)TaitoAnalogPort3);
}

static struct BurnDIPInfo SlapshotDIPList[]=
{
	{0x14, 0xff, 0xff, 0x10, NULL                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x14, 0x01, 0x10, 0x10, "Off"                            },
	{0x14, 0x01, 0x10, 0x00, "On"                             },
};

STDDIPINFO(Slapshot)

static struct BurnDIPInfo Opwolf3DIPList[]=
{
	{0x11, 0xff, 0xff, 0x10, NULL                             },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x11, 0x01, 0x10, 0x10, "Off"                            },
	{0x11, 0x01, 0x10, 0x00, "On"                             },
};

STDDIPINFO(Opwolf3)

static struct BurnRomInfo SlapshotRomDesc[] = {
	{ "d71-15.3",              0x080000, 0x1470153f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d71-16.1",              0x080000, 0xf13666e0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "d71-07.77",             0x010000, 0xdd5f670c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "d71-04.79",             0x080000, 0xb727b81c, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "d71-05.80",             0x080000, 0x7b0f5d6d, BRF_GRA | TAITO_CHARS_BYTESWAP },
	
	{ "d71-06.37",             0x080000, 0xf3324188, BRF_SND | TAITO_YM2610A },
	
	// Sprites - handled manually
	{ "d71-01.23",             0x100000, 0x0b1e8c27, BRF_GRA },
	{ "d71-02.24",             0x100000, 0xccaaea2d, BRF_GRA },
	{ "d71-03.25",             0x100000, 0xdccef9ec, BRF_GRA },
};

STD_ROM_PICK(Slapshot)
STD_ROM_FN(Slapshot)

static struct BurnRomInfo Opwolf3RomDesc[] = {
	{ "d74_16.3",              0x080000, 0x198ff1f6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d74_21.1",              0x080000, 0xc61c558b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d74_18.18",             0x080000, 0xbd5d7cdb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d74_17.17",             0x080000, 0xac35a672, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "d74_22.77",             0x010000, 0x118374a6, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "d74_05.80",             0x200000, 0x85ea64cc, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "d74_06.81",             0x200000, 0x2fa1e08d, BRF_GRA | TAITO_CHARS_BYTESWAP },
	
	{ "d74_01.37",             0x200000, 0x115313e0, BRF_SND | TAITO_YM2610A },
	
	// Sprites - handled manually
	{ "d74_02.23",             0x200000, 0xaab86332, BRF_GRA },
	{ "d74_03.24",             0x200000, 0x3f398916, BRF_GRA },
	{ "d74_04.25",             0x200000, 0x2f385638, BRF_GRA },
};

STD_ROM_PICK(Opwolf3)
STD_ROM_FN(Opwolf3)

static struct BurnRomInfo Opwolf3uRomDesc[] = {
	{ "d74_16.3",              0x080000, 0x198ff1f6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d74_20.1",              0x080000, 0x960fd892, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d74_18.18",             0x080000, 0xbd5d7cdb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d74_17.17",             0x080000, 0xac35a672, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	
	{ "d74_19.77",             0x010000, 0x05d53f06, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },
	
	{ "d74_05.80",             0x200000, 0x85ea64cc, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "d74_06.81",             0x200000, 0x2fa1e08d, BRF_GRA | TAITO_CHARS_BYTESWAP },
	
	{ "d74_01.37",             0x200000, 0x115313e0, BRF_SND | TAITO_YM2610A },
	
	// Sprites - handled manually
	{ "d74_02.23",             0x200000, 0xaab86332, BRF_GRA },
	{ "d74_03.24",             0x200000, 0x3f398916, BRF_GRA },
	{ "d74_04.25",             0x200000, 0x2f385638, BRF_GRA },
};

STD_ROM_PICK(Opwolf3u)
STD_ROM_FN(Opwolf3u)

static INT32 MemIndex()
{
	UINT8 *Next; Next = TaitoMem;

	Taito68KRom1                   = Next; Next += Taito68KRom1Size;
	TaitoZ80Rom1                   = Next; Next += TaitoZ80Rom1Size;
	TaitoYM2610ARom                = Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom                = Next; Next += TaitoYM2610BRomSize;
	
	TaitoRamStart                  = Next;

	Taito68KRam1                   = Next; Next += 0x10000;
	TaitoZ80Ram1                   = Next; Next += 0x02000;
	TaitoSpriteRam                 = Next; Next += 0x10000;
	TaitoSpriteRamBuffered         = Next; Next += 0x10000;
	TaitoSpriteExtension           = Next; Next += 0x02000;
	TaitoPaletteRam                = Next; Next += 0x08000;
	
	TaitoRamEnd                    = Next;

	TaitoChars                     = Next; Next += TaitoNumChar * TaitoCharWidth * TaitoCharHeight;
	TaitoSpritesA                  = Next; Next += TaitoNumSpriteA * TaitoSpriteAWidth * TaitoSpriteAHeight;
	TaitoPalette                   = (UINT32*)Next; Next += 0x02000 * sizeof(UINT32);
	TaitoF2SpriteList              = (TaitoF2SpriteEntry*)Next; Next += 0x400 * sizeof(TaitoF2SpriteEntry);

	TaitoMemEnd                    = Next;

	return 0;
}

static INT32 SlapshotDoReset()
{
	TaitoDoReset();
	
	return 0;
}

UINT8 __fastcall Slapshot68KReadByte(UINT32 a)
{
	if (a >= 0xa00000 && a <= 0xa03fff) {
		return TimeKeeperRead((a - 0xa00000) >> 1);
	}
	
	if (a >= 0xc00000 && a <= 0xc0000f) {
		return TC0640FIORead((a - 0xc00000) >> 1);
	}
	
	if (a >= 0xc00020 && a <= 0xc0002f) {
		if (a == 0xc00027) bprintf(PRINT_IMPORTANT, _T("%x\n"), a);
		
		if (a == 0xc00026) return (TC0640FIOInput[2] & 0xef) | (TaitoDip[0] & 0x10);
		
		return TC0640FIORead((a - 0xc00020) >> 1);
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Slapshot68KWriteByte(UINT32 a, UINT8 d)
{
	if (a <= 0x0fffff) return;
	
	if (a >= 0xa00000 && a <= 0xa03fff) {
		TimeKeeperWrite((a - 0xa00000) >> 1, d);
		return;
	}
	
	TC0360PRIHalfWordWrite_Map(0xb00000)

	if (a >= 0xc00000 && a <= 0xc0000f) {
		TC0640FIOWrite((a - 0xc00000) >> 1, d);
		return;
	}
	
	if (a >= 0xc00010 && a <= 0xc0002f) {
		return;
	}
	
	switch (a) {
		case 0xd00000: {
			TC0140SYTPortWrite(d);
			return;
		}
		
		case 0xd00002: {
			TC0140SYTCommWrite(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Slapshot68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Slapshot68KWriteWord(UINT32 a, UINT16 d)
{
	TC0480SCPCtrlWordWrite_Map(0x830000)
	
	if (a >= 0xc00000 && a <= 0xc0000f) {
		TC0640FIOWrite((a - 0xc00000) >> 1, d);
		return;
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Opwolf3Gun68KReadByte(UINT32 a)
{
	switch (a) {
		case 0xe00000: {
			float Temp = (float)~BurnGunReturnX(0) / 256.0;
			Temp *= 160.0;
			return ((UINT8)Temp - 0x5b) & 0xff;
		}
		
		case 0xe00002: {
			float Temp = (float)BurnGunReturnY(0) / 256.0;
			Temp *= 112;
			return ((UINT8)Temp + 0x08) & 0xff;
		}
		
		case 0xe00004: {
			float Temp = (float)~BurnGunReturnX(1) / 256.0;
			Temp *= 160.0;
			return ((UINT8)Temp - 0x5b) & 0xff;
		}
		
		case 0xe00006: {
			float Temp = (float)BurnGunReturnY(1) / 256.0;
			Temp *= 112;
			return ((UINT8)Temp + 0x08) & 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Opwolf3Gun68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xe00000:
		case 0xe00002:
		case 0xe00004:
		case 0xe00006: {
			SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Opwolf3Gun68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Opwolf3Gun68KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall SlapshotZ80Read(UINT16 a)
{
	switch (a) {
		case 0xe000: {
			return BurnYM2610Read(0);
		}
		
		case 0xe201: {
			return TC0140SYTSlaveCommRead();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall SlapshotZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xe000: {
			BurnYM2610Write(0, d);
			return;
		}
		
		case 0xe001: {
			BurnYM2610Write(1, d);
			return;
		}
		
		case 0xe002: {
			BurnYM2610Write(2, d);
			return;
		}
		
		case 0xe003: {
			BurnYM2610Write(3, d);
			return;
		}
		
		case 0xe200: {
			TC0140SYTSlavePortWrite(d);
			return;
		}
		
		case 0xe201: {
			TC0140SYTSlaveCommWrite(d);
			return;
		}
		
		case 0xf200: {
			TaitoZ80Bank = (d - 1) & 7;
			ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void SlapshotFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 SlapshotSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static double SlapshotGetTime()
{
	return (double)ZetTotalCycles() / 4000000;
}

static INT32 CharPlaneOffsets[4]          = { 0, 1, 2, 3 };
static INT32 CharXOffsets[16]             = { 4, 0, 20, 16, 12, 8, 28, 24, 36, 32, 52, 48, 44, 40, 60, 56 };
static INT32 CharYOffsets[16]             = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
static INT32 SpritePlaneOffsets[6]        = { 0x1000000, 0x1000001, 0, 1, 2, 3 };
static INT32 SpriteXOffsets[16]           = { 4, 0, 12, 8, 20, 16, 28, 24, 36, 32, 44, 40, 52, 48, 60, 56 };
static INT32 SpriteYOffsets[16]           = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
static INT32 Opwolf3SpritePlaneOffsets[6] = { 0x2000000, 0x2000001, 0, 1, 2, 3 };

static void SwitchToMusashi()
{
	if (bBurnUseASMCPUEmulation) {
#if 1 && defined FBA_DEBUG
		bprintf(PRINT_NORMAL, _T("Switching to Musashi 68000 core\n"));
#endif
		bUseAsm68KCoreOldValue = bBurnUseASMCPUEmulation;
		bBurnUseASMCPUEmulation = false;
	}
}

static INT32 MachineInit()
{
	INT32 nLen;
	
	TaitoNum68Ks = 1;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;
	
	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();
	
	GenericTilesInit();
	
	TC0480SCPInit(TaitoNumChar, 3, 30, 9, -1, 1, -2);
	TC0480SCPSetColourBase(256);
	TC0140SYTInit();
	TC0360PRIInit();
	TC0640FIOInit();
	
	if (TaitoLoadRoms(1)) return 1;
	
	SwitchToMusashi();
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1            , 0x000000, Taito68KRom1Size - 1, SM_ROM);
	SekMapMemory(Taito68KRam1            , 0x500000, 0x50ffff, SM_RAM);
	SekMapMemory(TaitoSpriteRam          , 0x600000, 0x60ffff, SM_RAM);
	SekMapMemory(TaitoSpriteExtension    , 0x700000, 0x701fff, SM_RAM);
	SekMapMemory(TC0480SCPRam            , 0x800000, 0x80ffff, SM_RAM);
	SekMapMemory(TaitoPaletteRam         , 0x900000, 0x907fff, SM_RAM);
	SekSetReadWordHandler(0, Slapshot68KReadWord);
	SekSetWriteWordHandler(0, Slapshot68KWriteWord);
	SekSetReadByteHandler(0, Slapshot68KReadByte);
	SekSetWriteByteHandler(0, Slapshot68KWriteByte);
	SekClose();	
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(SlapshotZ80Read);
	ZetSetWriteHandler(SlapshotZ80Write);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1               );
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1               );
	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000      );
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1               );
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1               );
	ZetMemEnd();
	ZetClose();	
	
	BurnYM2610Init(8000000, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &SlapshotFMIRQHandler, SlapshotSynchroniseStream, SlapshotGetTime, 0);
	BurnTimerAttachZet(4000000);
	
	TaitoMakeInputsFunction = TC0640FIOMakeInputs;
	TaitoDrawFunction = SlapshotDraw;

	nTaitoCyclesTotal[0] = 14364000 / 60;
	nTaitoCyclesTotal[1] = 4000000 / 60;
	
	TimeKeeperInit(TIMEKEEPER_MK48T08, NULL);
	
	TaitoF2SpritesDisabled = 1;
	TaitoF2SpritesActiveArea = 0;
	TaitoF2SpriteType = 2;
	TaitoXOffset = 3;
	
	TaitoF2SpriteBufferFunction = TaitoF2NoBuffer;
	
	for (INT32 i = 0; i < 8; i++) {
		TaitoF2SpriteBankBuffered[i] = 0x400 * i;
		TaitoF2SpriteBank[i] = TaitoF2SpriteBankBuffered[i];
	}
	
	return 0;
}

static INT32 SlapshotInit()
{
	TaitoCharModulo = 0x400;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 16;
	TaitoCharHeight = 16;
	TaitoCharPlaneOffsets = CharPlaneOffsets;
	TaitoCharXOffsets = CharXOffsets;
	TaitoCharYOffsets = CharYOffsets;
	TaitoNumChar = 0x2000;
	
	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 6;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x4000;
	
	if (MachineInit()) return 1;
	
	INT32 nRet;
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x400000);
	memset(TempRom, 0, 0x400000);
	nRet = BurnLoadRom(TempRom + 0x000000, 6, 2); if (nRet) return 1;
	nRet = BurnLoadRom(TempRom + 0x000001, 7, 2); if (nRet) return 1;
	nRet = BurnLoadRom(TempRom + 0x300000, 8, 1); if (nRet) return 1;
	INT32 Data;
	INT32 Offset = 0x400000 / 2;
	for (INT32 i = 0x400000 / 2 + 0x400000 / 4; i < 0x400000; i++) {
		INT32 d1, d2, d3, d4;

		Data = TempRom[i];
		d1 = (Data >> 0) & 3;
		d2 = (Data >> 2) & 3;
		d3 = (Data >> 4) & 3;
		d4 = (Data >> 6) & 3;

		TempRom[Offset] = (d1 << 2) | (d2 << 6);
		Offset++;

		TempRom[Offset] = (d3 << 2) | (d4 << 6);
		Offset++;
	}
	GfxDecode(TaitoNumSpriteA, TaitoSpriteANumPlanes, TaitoSpriteAWidth, TaitoSpriteAHeight, TaitoSpriteAPlaneOffsets, TaitoSpriteAXOffsets, TaitoSpriteAYOffsets, TaitoSpriteAModulo, TempRom, TaitoSpritesA);
	BurnFree(TempRom);
	
	SlapshotDoReset();

	return 0;
}

static INT32 Opwolf3Init()
{
	TaitoCharModulo = 0x400;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 16;
	TaitoCharHeight = 16;
	TaitoCharPlaneOffsets = CharPlaneOffsets;
	TaitoCharXOffsets = CharXOffsets;
	TaitoCharYOffsets = CharYOffsets;
	TaitoNumChar = 0x8000;
	
	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 6;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = Opwolf3SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x8000;
	
	if (MachineInit()) return 1;
	
	INT32 nRet;
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x800000);
	memset(TempRom, 0, 0x400000);
	nRet = BurnLoadRom(TempRom + 0x000000,  8, 2); if (nRet) return 1;
	nRet = BurnLoadRom(TempRom + 0x000001,  9, 2); if (nRet) return 1;
	nRet = BurnLoadRom(TempRom + 0x600000, 10, 1); if (nRet) return 1;
	INT32 Data;
	INT32 Offset = 0x800000 / 2;
	for (INT32 i = 0x800000 / 2 + 0x800000 / 4; i < 0x800000; i++) {
		INT32 d1, d2, d3, d4;

		Data = TempRom[i];
		d1 = (Data >> 0) & 3;
		d2 = (Data >> 2) & 3;
		d3 = (Data >> 4) & 3;
		d4 = (Data >> 6) & 3;

		TempRom[Offset] = (d1 << 2) | (d2 << 6);
		Offset++;

		TempRom[Offset] = (d3 << 2) | (d4 << 6);
		Offset++;
	}
	GfxDecode(TaitoNumSpriteA, TaitoSpriteANumPlanes, TaitoSpriteAWidth, TaitoSpriteAHeight, TaitoSpriteAPlaneOffsets, TaitoSpriteAXOffsets, TaitoSpriteAYOffsets, TaitoSpriteAModulo, TempRom, TaitoSpritesA);
	BurnFree(TempRom);
	
	SekOpen(0);
	SekMapHandler(1, 0xe00000, 0xe00007, SM_RAM);
	SekSetReadWordHandler(1, Opwolf3Gun68KReadWord);
	SekSetWriteWordHandler(1, Opwolf3Gun68KWriteWord);
	SekSetReadByteHandler(1, Opwolf3Gun68KReadByte);
	SekSetWriteByteHandler(1, Opwolf3Gun68KWriteByte);
	SekClose();
	
	TaitoMakeInputsFunction = Opwolf3MakeInputs;
	TaitoDrawFunction = Opwolf3Draw;
	
	BurnGunInit(2, true);

	SlapshotDoReset();

	return 0;
}

static INT32 SlapshotExit()
{
	TaitoExit();
	
	// Switch back CPU core if needed
	if (bUseAsm68KCoreOldValue) {
#if 1 && defined FBA_DEBUG
		bprintf(PRINT_NORMAL, _T("Switching back to A68K core\n"));
#endif
		bUseAsm68KCoreOldValue = false;
		bBurnUseASMCPUEmulation = true;
	}
	
	TimeKeeperExit();
	
	return 0;
}

inline static INT32 CalcCol(INT32 nColour)
{
	INT32 r, g, b;

	r = (BURN_ENDIAN_SWAP_INT32(nColour) & 0x000000ff) >> 0;
	g = (BURN_ENDIAN_SWAP_INT32(nColour) & 0xff000000) >> 24;
	b = (BURN_ENDIAN_SWAP_INT32(nColour) & 0x00ff0000) >> 16;

	return BurnHighCol(r, g, b, 0);
}

static void SlapshotCalcPalette()
{
	INT32 i;
	UINT32* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT32*)TaitoPaletteRam, pd = TaitoPalette; i < 0x2000; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
}

static void SlapshotDraw()
{
	UINT8 Layer[4];
	UINT16 Priority = TC0480SCPGetBgPriority();
	
	Layer[0] = (Priority & 0xf000) >> 12;
	Layer[1] = (Priority & 0x0f00) >>  8;
	Layer[2] = (Priority & 0x00f0) >>  4;
	Layer[3] = (Priority & 0x000f) >>  0;
	
	TaitoF2TilePriority[Layer[0]] = TC0360PRIRegs[4] & 0x0f;
	TaitoF2TilePriority[Layer[1]] = TC0360PRIRegs[4] >> 4;
	TaitoF2TilePriority[Layer[2]] = TC0360PRIRegs[5] & 0x0f;
	TaitoF2TilePriority[Layer[3]] = TC0360PRIRegs[5] >> 4;
	
	TaitoF2SpritePriority[0] = TC0360PRIRegs[6] & 0x0f;
	TaitoF2SpritePriority[1] = TC0360PRIRegs[6] >> 4;
	TaitoF2SpritePriority[2] = TC0360PRIRegs[7] & 0x0f;
	TaitoF2SpritePriority[3] = TC0360PRIRegs[7] >> 4;
	
	SlapshotCalcPalette();
	BurnTransferClear();	
	
	TaitoF2MakeSpriteList();
	
	for (INT32 i = 0; i < 16; i++) {
		if (TaitoF2TilePriority[0] == i) TC0480SCPTilemapRender(Layer[0], 0, TaitoChars);
		if (TaitoF2TilePriority[1] == i) TC0480SCPTilemapRender(Layer[1], 0, TaitoChars);
		if (TaitoF2TilePriority[2] == i) TC0480SCPTilemapRender(Layer[2], 0, TaitoChars);
		if (TaitoF2TilePriority[3] == i) TC0480SCPTilemapRender(Layer[3], 0, TaitoChars);
		if (TaitoF2SpritePriority[3] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[3]);
		if (TaitoF2SpritePriority[2] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[2]);
		if (TaitoF2SpritePriority[1] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[1]);
		if (TaitoF2SpritePriority[0] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[0]);
	}
	
	TC0480SCPRenderCharLayer();
	
	BurnTransferCopy(TaitoPalette);
}

static void Opwolf3Draw()
{
	SlapshotDraw();
	
	for (INT32 i = 0; i < nBurnGunNumPlayers; i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}
}

static INT32 SlapshotFrame()
{
	INT32 nInterleave = 100;

	if (TaitoReset) SlapshotDoReset();

	TaitoMakeInputsFunction();
	
	nTaitoCyclesDone[0] = nTaitoCyclesDone[1] = 0;

	SekNewFrame();
	ZetNewFrame();
	
	if ((GetCurrentFrame() % 60) == 0) TimeKeeperTick();

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
		if (i == 10) { SekSetIRQLine(6, SEK_IRQSTATUS_AUTO); nTaitoCyclesDone[0] += SekRun(200000 - 500); }
		if (i == (nInterleave - 1)) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		ZetOpen(0);
		BurnTimerUpdate(i * (nTaitoCyclesTotal[1] / nInterleave));
		ZetClose();
	}
	
	ZetOpen(0);
	BurnTimerEndFrame(nTaitoCyclesTotal[1]);
	BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();
	
	TaitoF2HandleSpriteBuffering();
	
	if (pBurnDraw) TaitoDrawFunction();
	
	TaitoF2SpriteBufferFunction();

	return 0;
}

static INT32 SlapshotScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029709;
	}
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TaitoRamStart;
		ba.nLen	  = TaitoRamEnd-TaitoRamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	TaitoICScan(nAction);
	
	TimeKeeperScan(nAction);
	
	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2610Scan(nAction, pnMin);
		
		SCAN_VAR(TC0640FIOInput);
		SCAN_VAR(TaitoZ80Bank);
		SCAN_VAR(nTaitoCyclesDone);
		SCAN_VAR(nTaitoCyclesSegment);
		SCAN_VAR(TaitoF2SpriteBank);
		SCAN_VAR(TaitoF2SpriteBankBuffered);
	}
	
	if (nAction & ACB_WRITE) {
		if (TaitoZ80Bank) {
			ZetOpen(0);
			ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000));
			ZetClose();
		}		
	}
	
	return 0;
}

static INT32 Opwolf3Scan(INT32 nAction, INT32 *pnMin)
{
	INT32 nRet = SlapshotScan(nAction, pnMin);
	
	if (nAction & ACB_DRIVER_DATA) {
		BurnGunScan();
	}
	
	return nRet;
}

struct BurnDriver BurnDrvSlapshot = {
	"slapshot", NULL, NULL, NULL, "1994",
	"Slap Shot (Japan)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_SPORTSMISC, 0,
	NULL, SlapshotRomInfo, SlapshotRomName, NULL, NULL, SlapshotInputInfo, SlapshotDIPInfo,
	SlapshotInit, SlapshotExit, SlapshotFrame, NULL, SlapshotScan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvOpwolf3 = {
	"opwolf3", NULL, NULL, NULL, "1994",
	"Operation Wolf 3 (World)\0", NULL, "Taito Corporation Japan", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_SHOOT, 0,
	NULL, Opwolf3RomInfo, Opwolf3RomName, NULL, NULL, Opwolf3InputInfo, Opwolf3DIPInfo,
	Opwolf3Init, SlapshotExit, SlapshotFrame, NULL, Opwolf3Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvOpwolf3u = {
	"opwolf3u", "opwolf3", NULL, NULL, "1994",
	"Operation Wolf 3 (US)\0", NULL, "Taito America Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_SHOOT, 0,
	NULL, Opwolf3uRomInfo, Opwolf3uRomName, NULL, NULL, Opwolf3InputInfo, Opwolf3DIPInfo,
	Opwolf3Init, SlapshotExit, SlapshotFrame, NULL, Opwolf3Scan,
	NULL, 0x2000, 320, 224, 4, 3
};
