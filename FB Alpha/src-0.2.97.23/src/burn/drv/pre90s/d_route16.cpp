// FB Alpha Route 16 driver module
// Based on code by Zsolt Vasvari

#include "burnint.h"
#include "driver.h"
#include "dac.h"
extern "C" {
#include "ay8910.h"
}

//------------------------------------------------------------------------------------------------

static INT32 draw_type;
static UINT8 *Mem = NULL;
static UINT8 *Rom0, *Rom1, *Prom;

static UINT8 flipscreen, palette_1, palette_2, ttmahjng_port_select;
static INT32 speakres_vrx;

static INT16 *pAY8910Buffer[3];
static INT16 *pFMBuffer = NULL;

static UINT8  DrvJoy1[24], DrvJoy2[8], Dips, DrvReset;

//------------------------------------------------------------------------------------------------

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"    ,   BIT_DIGITAL  , DrvJoy1 + 0,	"p1 coin"  },
	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 1,	"p1 start" },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 2, 	"p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 3, 	"p1 left"  },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy1 + 4,   "p1 up"    },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy1 + 5,   "p1 down", },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 fire 1"},

	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy2 + 0,	"p2 start" },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 1, 	"p2 right" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 2, 	"p2 left"  },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy2 + 3,   "p2 up"    },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy2 + 4,   "p2 down", },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p2 fire 1"},

	{"Service Mode",  BIT_DIGITAL,   DrvJoy1 + 7,   "diag"     },
	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"     ,    BIT_DIPSWITCH, &Dips  ,	"dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo mahjongInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 +  0,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 +  1,	"p1 start"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 +  2,	"p2 start"},
	{"A",			BIT_DIGITAL,	DrvJoy1 +  3,	"mah a"},
	{"E",			BIT_DIGITAL,	DrvJoy1 +  4,	"mah e"},
	{"I",			BIT_DIGITAL,	DrvJoy1 +  5,	"mah i"},
	{"M",			BIT_DIGITAL,	DrvJoy1 +  6,	"mah m"},
	{"Kan",			BIT_DIGITAL,	DrvJoy1 +  7,	"mah kan"},
	{"B",			BIT_DIGITAL,	DrvJoy1 +  8,	"mah b"},
	{"F",			BIT_DIGITAL,	DrvJoy1 +  9,	"mah f"},
	{"J",			BIT_DIGITAL,	DrvJoy1 + 10,	"mah j"},
	{"N",			BIT_DIGITAL,	DrvJoy1 + 11,	"mah n"},
	{"Reach",		BIT_DIGITAL,	DrvJoy1 + 12,	"mah reach"},
	{"C",			BIT_DIGITAL,	DrvJoy1 + 13,	"mah c"},
	{"G",			BIT_DIGITAL,	DrvJoy1 + 14,	"mah g"},
	{"K",			BIT_DIGITAL,	DrvJoy1 + 15,	"mah k"},
	{"Chi",			BIT_DIGITAL,	DrvJoy1 + 16,	"mah chi"},
	{"Ron",			BIT_DIGITAL,	DrvJoy1 + 17,	"mah ron"},
	{"D",			BIT_DIGITAL,	DrvJoy1 + 18,	"mah d"},
	{"H",			BIT_DIGITAL,	DrvJoy1 + 19,	"mah h"},
	{"L",			BIT_DIGITAL,	DrvJoy1 + 20,	"mah l"},
	{"Pon",			BIT_DIGITAL,	DrvJoy1 + 21,	"mah pon"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
};

STDINPUTINFO(mahjong)



static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x20, NULL                     },

	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0f, 0x01, 0x01, 0x00, "3"       		  },
	{0x0f, 0x01, 0x01, 0x01, "5"       		  },

	{0   , 0xfe, 0   , 3   , "Coinage"                },
	{0x0f, 0x01, 0x18, 0x08, "2C_1C"     		  },
	{0x0f, 0x01, 0x18, 0x00, "1C_1C"    		  },
	{0x0f, 0x01, 0x18, 0x18, "1C_2C"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x20, 0x20, "Upright"     		  },
	{0x0f, 0x01, 0x20, 0x00, "Cocktail"		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0f, 0x01, 0x40, 0x00, "Off"     		  },
	{0x0f, 0x01, 0x40, 0x40, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x80, 0x00, "Off"     		  },
	{0x0f, 0x01, 0x80, 0x80, "On"    		  },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo stratvoxDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x20, NULL                     },

	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x0f, 0x01, 0x01, 0x00, "3"       		  },
	{0x0f, 0x01, 0x01, 0x01, "5"       		  },

	{0   , 0xfe, 0   , 2   , "Replenish Astronauts"   },
	{0x0f, 0x01, 0x02, 0x00, "No"       		  },
	{0x0f, 0x01, 0x02, 0x02, "Yes"       		  },

	{0   , 0xfe, 0   , 4   , "2 Attackers At Wave"    },
	{0x0f, 0x01, 0x0c, 0x00, "2"       		  },
	{0x0f, 0x01, 0x0c, 0x04, "3"       		  },
	{0x0f, 0x01, 0x0c, 0x08, "4"       		  },
	{0x0f, 0x01, 0x0c, 0x0c, "5"       		  },

	{0   , 0xfe, 0   , 3   , "Astronauts Kidnapped"   },
	{0x0f, 0x01, 0x10, 0x00, "Less Often"     	  },
	{0x0f, 0x01, 0x10, 0x10, "More Often"    	  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x20, 0x20, "Upright"     		  },
	{0x0f, 0x01, 0x20, 0x00, "Cocktail"		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0f, 0x01, 0x40, 0x00, "Off"     		  },
	{0x0f, 0x01, 0x40, 0x40, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x80, 0x00, "Off"     		  },
	{0x0f, 0x01, 0x80, 0x80, "On"    		  },
};

STDDIPINFO(stratvox)

static struct BurnDIPInfo speakresDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0x20, NULL                     },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0f, 0x01, 0x03, 0x00, "3"       		  },
	{0x0f, 0x01, 0x03, 0x01, "4"       		  },
	{0x0f, 0x01, 0x03, 0x02, "5"       		  },
	{0x0f, 0x01, 0x03, 0x03, "6"       		  },

	{0   , 0xfe, 0   , 4   , "2 Attackers At Wave"    },
	{0x0f, 0x01, 0x0c, 0x00, "2"       		  },
	{0x0f, 0x01, 0x0c, 0x04, "3"       		  },
	{0x0f, 0x01, 0x0c, 0x08, "4"       		  },
	{0x0f, 0x01, 0x0c, 0x0c, "5"       		  },

	{0   , 0xfe, 0   , 3   , "Bonus Life"  		  },
	{0x0f, 0x01, 0x10, 0x00, "5000"     		  },
	{0x0f, 0x01, 0x10, 0x10, "8000"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x20, 0x20, "Upright"     		  },
	{0x0f, 0x01, 0x20, 0x00, "Cocktail"		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0f, 0x01, 0x40, 0x00, "Off"     		  },
	{0x0f, 0x01, 0x40, 0x40, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x80, 0x00, "Off"     		  },
	{0x0f, 0x01, 0x80, 0x80, "On"    		  },
};

STDDIPINFO(speakres)


//------------------------------------------------------------------------------------------------

static UINT8 ttmahjng_input_port_matrix_r()
{
	UINT8 ret = 0;

	switch (ttmahjng_port_select)
	{
		case 1: {
			for (INT32 i = 0; i < 5; i++) ret |= DrvJoy1[ 3 + i] << i;
			ret |= DrvJoy1[1] << 5;
		}
		break;
		case 2:	{
			for (INT32 i = 0; i < 5; i++) ret |= DrvJoy1[ 8 + i] << i;
			ret |= DrvJoy1[2] << 5;
		}
		break;
		case 4:	{
			for (INT32 i = 0; i < 5; i++) ret |= DrvJoy1[13 + i] << i;
		}
		break;
		case 8:	{
			for (INT32 i = 0; i < 4; i++) ret |= DrvJoy1[18 + i] << i;
		}
		break;
		default: break;
	}

	return ~ret;
}


UINT8 __fastcall route16_cpu0_read(UINT16 offset)
{
	UINT8 nRet = 0;

	switch (offset)
	{
		case 0x4800:
			return Dips;

		case 0x5000:
		{
			if (draw_type == 1) {	// ttmahjng
				nRet = (DrvJoy1[0] << 7) ^ 0xff;
			} else {
				if (DrvJoy1[2]) nRet |= 0x01;
				if (DrvJoy1[3]) nRet |= 0x02;
				if (DrvJoy1[4]) nRet |= 0x04;
				if (DrvJoy1[5]) nRet |= 0x08;
				if (DrvJoy1[6]) nRet |= 0x10;
				if (DrvJoy1[7]) nRet |= 0x40; // service mode
				if (DrvJoy1[0]) nRet |= 0x80; // coin
			}
			return nRet;	
		}


		case 0x5800:
		{
			if (draw_type == 1) {	// ttmahjng
				nRet = ttmahjng_input_port_matrix_r();
			} else {
				if (DrvJoy2[1]) nRet |= 0x01;
				if (DrvJoy2[2]) nRet |= 0x02;
				if (DrvJoy2[3]) nRet |= 0x04;
				if (DrvJoy2[4]) nRet |= 0x08;
				if (DrvJoy2[5]) nRet |= 0x10;
				if (DrvJoy2[0]) nRet |= 0x40; // start 2
				if (DrvJoy1[1]) nRet |= 0x80; // start 1
			}

			return nRet;
		}

		case 0x6000: // 	speakres
		{
			INT32 bit2=4, bit1=2, bit0=1;

			// just using a counter, the constants are the number of reads
			// before going low, each read is 40 cycles apart. the constants
			// were chosen based on the startup tests and for vr0=vr2
			speakres_vrx++;
			if(speakres_vrx>0x300) bit0=0;		/* VR0 100k ohm - speech */
			if(speakres_vrx>0x200) bit1=0;		/* VR1  50k ohm - main volume */
			if(speakres_vrx>0x300) bit2=0;		/* VR2 100k ohm - explosion */

			return 0xf8|bit2|bit1|bit0;
		}

		case 0x6400:		// routex
		{
			if (ZetPc(-1) == 0x2f)
				return 0xfb;
			else
				return 0;
		}
	}

	return 0;
}


void __fastcall route16_cpu0_write(UINT16 offset, UINT8 data)
{
	if (offset >= 0x4000 && offset < 0x4400) {
		Rom0[offset] = data;

		// 4313-4319 are used in Route 16 as triggers to wake the other CPU
		if (offset >= 0x4313 && offset <= 0x4319 && data == 0xff)
		{
			// Let the other CPU run
		//	cpu_yield();
			ZetRunEnd();	// Correct?
		}

		return;
	}

	switch (offset)
	{
		case 0x2800:			// stratvox
			DACWrite(0, data);
		break;

		case 0x4800:
			palette_1 = data & 0x1f;
		break;

		case 0x5000:
			palette_2 = data & 0x1f;
			flipscreen = (data >> 5) & 0x01;
		break;

		case 0x5800:			// speakres, ttmahjng
			ttmahjng_port_select = data;
			speakres_vrx = 0;
		break;

		case 0x6800:			// ttmahjng
			AY8910Write(0, 1, data);
		break;

		case 0x6900:			// ttmahjng
			AY8910Write(0, 0, data);
		break;
	}
}

void __fastcall route16_cpu0_out(UINT16 offset, UINT8 data)
{
	switch (offset & 0x1ff)
	{
		case 0x000:
			AY8910Write(0, 1, data);
		break;

		case 0x100:
			AY8910Write(0, 0, data);
		break;
	}
}


static INT32 GetRoms()
{
	char* pRomName;
	struct BurnRomInfo ri;
	UINT8 *Rom0Load = Rom0;
	UINT8 *Rom1Load = Rom1;
	UINT8 *PromLoad = Prom;

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & 7) == 1) {
			if (BurnLoadRom(Rom0Load, i, 1)) return 1;
			Rom0Load += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 2) {
			if (BurnLoadRom(Rom1Load, i, 1)) return 1;
			Rom1Load += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 3) {
			if (BurnLoadRom(PromLoad, i, 1)) return 1;
			PromLoad += ri.nLen;
			continue;
		}
	}

	return 0;
}


static INT32 DrvDoReset()
{
	flipscreen = palette_1 = palette_2 = ttmahjng_port_select = 0;
	speakres_vrx = 0;

	DrvReset = 0;

	memset (Rom0 + 0x4000, 0, 0xc000); // Shared & video ram
	memset (Rom1 + 0x8000, 0, 0x4000);

	ZetOpen(0);
	ZetReset();
	ZetClose();
	ZetOpen(1);
	ZetReset();
	ZetClose();

	AY8910Reset(0);

	return 0;
}

void stratvox_sn76477_write(UINT32, UINT32)
{

}

static INT32 DrvInit()
{
	Mem = (UINT8*)BurnMalloc(0x10000 + 0x10000 + 0x200);
	if (Mem == NULL) {
		return 1;
	}

	pFMBuffer = (INT16 *)BurnMalloc (nBurnSoundLen * 3 * sizeof(INT16));
	if (pFMBuffer == NULL) {
		return 1;
	}

	Rom0 = Mem + 0x00000;
	Rom1 = Mem + 0x10000;
	Prom = Mem + 0x20000;

	// Load Roms
	if (GetRoms()) return 1;

	ZetInit(0);
	ZetOpen(0);
	ZetSetOutHandler(route16_cpu0_out);
	ZetSetReadHandler(route16_cpu0_read);
	ZetSetWriteHandler(route16_cpu0_write);
	ZetMapArea (0x0000, 0x3fff, 0, Rom0 + 0x0000); // ROM
	ZetMapArea (0x0000, 0x3fff, 2, Rom0 + 0x0000);
	ZetMapArea (0x4000, 0x43ff, 0, Rom0 + 0x4000); // Read Shared RAM
	ZetMapArea (0x8000, 0xbfff, 0, Rom0 + 0x8000); // Video RAM
	ZetMapArea (0x8000, 0xbfff, 1, Rom0 + 0x8000);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetSetWriteHandler(route16_cpu0_write);
	ZetMapArea (0x0000, 0x1fff, 0, Rom1 + 0x0000); // ROM
	ZetMapArea (0x0000, 0x1fff, 2, Rom1 + 0x0000);
	ZetMapArea (0x4000, 0x43ff, 0, Rom0 + 0x4000); // Read Shared RAM
	ZetMapArea (0x8000, 0xbfff, 0, Rom1 + 0x8000); // Video RAM
	ZetMapArea (0x8000, 0xbfff, 1, Rom1 + 0x8000);
	ZetMemEnd();
	ZetClose();

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;

	AY8910Init(0, 1250000, nBurnSoundRate, NULL, NULL, &stratvox_sn76477_write, NULL);
	
	DACInit(0, 0, 1);
	DACSetVolShift(0, 2);

	DrvDoReset();

	return 0;
}


static INT32 DrvExit()
{
	DACExit();
	ZetExit();
	AY8910Exit(0);

	BurnFree (Mem);
	BurnFree (pFMBuffer);

	draw_type = 0;

	pFMBuffer = NULL;
	Mem = NULL;
	Rom0 = Rom1 = Prom = NULL;
	pAY8910Buffer[0] = pAY8910Buffer[1] = pAY8910Buffer[2] = NULL;

	return 0;
}


//------------------------------------------------------------------------------------------------
// Drawing functions

static inline UINT32 route16_make_pen(UINT8 color)
{
	UINT32 ret = 0;
	if (color & 1) ret |= 0x00ff0000;
	if (color & 2) ret |= 0x0000ff00;
	if (color & 4) ret |= 0x000000ff;
	return ret;
}


static inline UINT32 ttmajng_make_pen(UINT8 color)
{
	UINT32 ret = 0;
	if (color & 4) ret |= 0x00ff0000;
	if (color & 2) ret |= 0x0000ff00;
	if (color & 1) ret |= 0x000000ff;
	return ret;
}


static INT32 DrvDraw()
{
	UINT8 *prom1 = Prom + 0x0000;
	UINT8 *prom2 = Prom + 0x0100;

	for (INT32 offs = 0; offs < 0x4000; offs++)
	{
		UINT8 y = offs >> 6;
		UINT8 x = offs << 2;

		UINT8 d1 = Rom0[0x8000 + offs];
		UINT8 d2 = Rom1[0x8000 + offs];

		for (INT32 i = 0; i < 4; i++)
		{
			UINT8 color1, color2;

			// stratvox & ttmahjng
			if (draw_type) {
				color1 = prom1[(palette_1 << 2) | ((d1 >> 3) & 2) | (d1 & 1)];
				color2 = prom2[(((d1 << 3) & 0x80) | ((d1 << 7) & 0x80)) | (palette_2 << 2) | ((d2 >> 3) & 2) | (d2 & 1)];
			} else {
				color1 = prom1[((palette_1 << 6) & 0x80) | (palette_1 << 2) | ((d1 >> 3) & 0x02) | ((d1 >> 0) & 0x01)];
				color2 = prom2[((palette_2 << 6) & 0x80) | (((color1 << 6) & 0x80) | ((color1 << 7) & 0x80)) | (palette_2 << 2) | ((d2 >> 3) & 2) | (d2 & 1)];
			}

			UINT8 final_color = color1 | color2;

			UINT32 pen;
			if (draw_type == 1) {
				pen = ttmajng_make_pen(final_color);
			} else {
				pen = route16_make_pen(final_color);
			}

			if (flipscreen)
				PutPix(pBurnDraw + ((x << 8) | (y ^ 0xff)) * nBurnBpp, BurnHighCol(pen>>16, pen>>8, pen, 0));
			else
				PutPix(pBurnDraw + (((x ^ 0xff) << 8) | y) * nBurnBpp, BurnHighCol(pen>>16, pen>>8, pen, 0));

			x += 1;
			d1 >>= 1;
			d2 >>= 1;
		}
	}

	return 0;
}


static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = nBurnSoundLen;
	
	INT32 DACIRQFireSlice[48];
	for (INT32 i = 0; i < 48; i++) {
		DACIRQFireSlice[i] = (INT32)((double)((nInterleave * (i + 1)) / 49));
	}

	INT32 nCyclesSegment;
	INT32 nCyclesDone[2], nCyclesTotal[2];

	nCyclesTotal[0] = 2500000 / 60;
	nCyclesTotal[1] = 2500000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run Z80 #0
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i+1 == nInterleave) ZetRaiseIrq(1);
		ZetClose();

		// Run Z80 #1
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (draw_type == 3) { // space echo
			for (INT32 j = 0; j < 48; j++) {
				if (i == DACIRQFireSlice[j]) {
					ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
				}
			}
		}
		ZetClose();

		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSample;
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample /= 4;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
			}

			DACUpdate(pSoundBuf, nSegmentLength);

			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSample;
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample /= 4;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
			}
		}

		DACUpdate(pSoundBuf, nSegmentLength);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}


//------------------------------------------------------------------------------------------------
// Savestates

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {					// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {			// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = Rom0 + 0x4000;
		ba.nLen	  = 0x0400;
		ba.szName = "Shared RAM";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  = Rom0 + 0x8000;
		ba.nLen	  = 0x4000;
		ba.szName = "Cpu0 Video RAM";
		BurnAcb(&ba);
	
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = Rom1 + 0x8000;
		ba.nLen	  = 0x4000;
		ba.szName = "Cpu1 Video RAM";
		BurnAcb(&ba);

		ZetScan(nAction);			// Scan Z80
		AY8910Scan(nAction, pnMin);		// Scan AY8910

		// Scan critical driver variables
		SCAN_VAR(flipscreen);
		SCAN_VAR(palette_1);
		SCAN_VAR(palette_2);		
		SCAN_VAR(ttmahjng_port_select);
		SCAN_VAR(speakres_vrx);
	}

	return 0;
}


//------------------------------------------------------------------------------------------------
// Drivers


// Route 16

static struct BurnRomInfo route16RomDesc[] = {
	{ "route16.a0",   0x0800, 0x8f9101bd, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "route16.a1",   0x0800, 0x389bc077, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "route16.a2",   0x0800, 0x1065a468, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "route16.a3",   0x0800, 0x0b1987f3, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "route16.a4",   0x0800, 0xf67d853a, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "route16.a5",   0x0800, 0xd85cf758, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "route16.b0",   0x0800, 0x0f9588a7, 2 | BRF_ESS | BRF_PRG }, //  6 Z80 #1 Code
	{ "route16.b1",   0x0800, 0x2b326cf9, 2 | BRF_ESS | BRF_PRG }, //  7
	{ "route16.b2",   0x0800, 0x529cad13, 2 | BRF_ESS | BRF_PRG }, //  8
	{ "route16.b3",   0x0800, 0x3bd8b899, 2 | BRF_ESS | BRF_PRG }, //  9

	{ "im5623.f10",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 10 Graphics
	{ "im5623.f12",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 11
};

STD_ROM_PICK(route16)
STD_ROM_FN(route16)

static INT32 route16Init()
{
	INT32 nRet;

	draw_type = 0;

	nRet = DrvInit();

	if (nRet == 0)
	{
		// Patch protection
		Rom0[0x00e9] = 0x3a;
		Rom0[0x0754] = 0xc3;
		Rom0[0x0755] = 0x63;
		Rom0[0x0756] = 0x07;
	}

	return nRet;
}

struct BurnDriver BurnDrvroute16 = {
	"route16", NULL, NULL, NULL, "1981",
	"Route 16\0", NULL, "Tehkan/Sun (Centuri license)", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, route16RomInfo, route16RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	route16Init, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 3, 4
};


// Route 16 (set 2)

static struct BurnRomInfo route16aRomDesc[] = {
	{ "vg-54",        0x0800, 0x0c966319, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "vg-55",        0x0800, 0xa6a8c212, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "vg-56",        0x0800, 0x5c74406a, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "vg-57",        0x0800, 0x313e68ab, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "vg-58",        0x0800, 0x40824e3c, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "vg-59",        0x0800, 0x9313d2c2, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "route16.b0",   0x0800, 0x0f9588a7, 2 | BRF_ESS | BRF_PRG }, //  6 Z80 #1 Code
	{ "vg-61",        0x0800, 0xb216c88c, 2 | BRF_ESS | BRF_PRG }, //  7
	{ "route16.b2",   0x0800, 0x529cad13, 2 | BRF_ESS | BRF_PRG }, //  8
	{ "route16.b3",   0x0800, 0x3bd8b899, 2 | BRF_ESS | BRF_PRG }, //  9

	{ "im5623.f10",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 10 Graphics
	{ "im5623.f12",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 11
};

STD_ROM_PICK(route16a)
STD_ROM_FN(route16a)

static INT32 route16aInit()
{
	INT32 nRet;

	draw_type = 0;

	nRet = DrvInit();

	if (nRet == 0)
	{
		// Patch protection
		Rom0[0x00e9] = 0x3a;

		Rom0[0x0105] = 0x00; // jp nz,$4109 (nirvana) - NOP's in route16
		Rom0[0x0106] = 0x00;
		Rom0[0x0107] = 0x00;

		Rom0[0x0731] = 0x00; // jp nz,$4238 (nirvana)
		Rom0[0x0732] = 0x00;
		Rom0[0x0733] = 0x00;

		Rom0[0x0747] = 0xc3;
		Rom0[0x0748] = 0x56;
		Rom0[0x0749] = 0x07;
	}

	return nRet;
}

struct BurnDriver BurnDrvroute16a = {
	"route16a", "route16", NULL, NULL, "1981",
	"Route 16 (set 2)\0", NULL, "Tehkan/Sun (Centuri license)", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, route16aRomInfo, route16aRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	route16aInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 3, 4
};


// Route 16 (bootleg)

static struct BurnRomInfo route16bRomDesc[] = {
	{ "rt16.0",       0x0800, 0xb1f0f636, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "rt16.1",       0x0800, 0x3ec52fe5, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "rt16.2",       0x0800, 0xa8e92871, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "rt16.3",       0x0800, 0xa0fc9fc5, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "rt16.4",       0x0800, 0x6dcaf8c4, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "rt16.5",       0x0800, 0x63d7b05b, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "rt16.6",       0x0800, 0xfef605f3, 2 | BRF_ESS | BRF_PRG }, //  6 Z80 #1 Code
	{ "rt16.7",       0x0800, 0xd0d6c189, 2 | BRF_ESS | BRF_PRG }, //  7
	{ "rt16.8",       0x0800, 0xdefc5797, 2 | BRF_ESS | BRF_PRG }, //  8
	{ "rt16.9",       0x0800, 0x88d94a66, 2 | BRF_ESS | BRF_PRG }, //  9

	{ "im5623.f10",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 10 Graphics
	{ "im5623.f12",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 11
};

STD_ROM_PICK(route16b)
STD_ROM_FN(route16b)

static INT32 route16bInit()
{
	draw_type = 0;

	return DrvInit();
}

struct BurnDriver BurnDrvroute16b = {
	"route16b", "route16", NULL, NULL, "1981",
	"Route 16 (bootleg)\0", NULL, "bootleg", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, route16bRomInfo, route16bRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	route16Init, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 3, 4
};


// Route X (bootleg)

static struct BurnRomInfo routexRomDesc[] = {
	{ "routex01.a0",  0x0800, 0x99b500e7, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "rt16.1",       0x0800, 0x3ec52fe5, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "rt16.2",       0x0800, 0xa8e92871, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "rt16.3",       0x0800, 0xa0fc9fc5, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "routex05.a4",  0x0800, 0x2fef7653, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "routex06.a5",  0x0800, 0xa39ef648, 1 | BRF_ESS | BRF_PRG }, //  5
	{ "routex07.a6",  0x0800, 0x89f80c1c, 1 | BRF_ESS | BRF_PRG }, //  6

	{ "routex11.b0",  0x0800, 0xb51edd1d, 2 | BRF_ESS | BRF_PRG }, //  7 Z80 #1 Code
	{ "rt16.7",       0x0800, 0xd0d6c189, 2 | BRF_ESS | BRF_PRG }, //  8
	{ "rt16.8",       0x0800, 0xdefc5797, 2 | BRF_ESS | BRF_PRG }, //  9
	{ "rt16.9",       0x0800, 0x88d94a66, 2 | BRF_ESS | BRF_PRG }, // 10

	{ "im5623.f10",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 11 Graphics
	{ "im5623.f12",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 12
};

STD_ROM_PICK(routex)
STD_ROM_FN(routex)

struct BurnDriver BurnDrvroutex = {
	"routex", "route16", NULL, NULL, "1981",
	"Route X (bootleg)\0", NULL, "bootleg", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, routexRomInfo, routexRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	route16bInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 3, 4
};


// Speak & Rescue

static struct BurnRomInfo speakresRomDesc[] = {
	{ "speakres.1",   0x0800, 0x6026e4ea, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "speakres.2",   0x0800, 0x93f0d4da, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "speakres.3",   0x0800, 0xa3874304, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "speakres.4",   0x0800, 0xf484be3a, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "speakres.5",   0x0800, 0x61b12a67, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "speakres.6",   0x0800, 0x220e0ab2, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "speakres.7",   0x0800, 0xd417be13, 2 | BRF_ESS | BRF_PRG }, //  6 Z80 #1 Code
	{ "speakres.8",   0x0800, 0x52485d60, 2 | BRF_ESS | BRF_PRG }, //  7

	{ "im5623.f10",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       //  8 Graphics
	{ "im5623.f12",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       //  9
};

STD_ROM_PICK(speakres)
STD_ROM_FN(speakres)

static INT32 speakresInit()
{
	draw_type = 2;

	return DrvInit();
}

struct BurnDriver BurnDrvspeakres = {
	"speakres", NULL, NULL, NULL, "1980",
	"Speak & Rescue\0", NULL, "Sun Electronics", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, speakresRomInfo, speakresRomName, NULL, NULL, DrvInputInfo, speakresDIPInfo,
	speakresInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 3, 4
};


// Stratovox

static struct BurnRomInfo stratvoxRomDesc[] = {
	{ "ls01.bin",     0x0800, 0xbf4d582e, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "ls02.bin",     0x0800, 0x16739dd4, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "ls03.bin",     0x0800, 0x083c28de, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "ls04.bin",     0x0800, 0xb0927e3b, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "ls05.bin",     0x0800, 0xccd25c4e, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "ls06.bin",     0x0800, 0x07a907a7, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "ls07.bin",     0x0800, 0x4d333985, 2 | BRF_ESS | BRF_PRG }, //  6 Z80 #1 Code
	{ "ls08.bin",     0x0800, 0x35b753fc, 2 | BRF_ESS | BRF_PRG }, //  7

	{ "im5623.f10",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       //  8 Graphics
	{ "im5623.f12",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       //  9
};

STD_ROM_PICK(stratvox)
STD_ROM_FN(stratvox)

struct BurnDriver BurnDrvstratvox = {
	"stratvox", "speakres", NULL, NULL, "1980",
	"Stratovox\0", NULL, "[Sun Electronics] (Taito license)", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, stratvoxRomInfo, stratvoxRomName, NULL, NULL, DrvInputInfo, stratvoxDIPInfo,
	speakresInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 3, 4
};


// Stratovox (bootleg)

static struct BurnRomInfo stratvobRomDesc[] = {
	{ "j0-1",         0x0800, 0x93c78274, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "j0-2",         0x0800, 0x93b2b02d, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "j0-3",         0x0800, 0x655facb5, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "j0-4",         0x0800, 0xb0927e3b, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "j0-5",         0x0800, 0x9d2178d9, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "j0-6",         0x0800, 0x79118ffc, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "b0-a",         0x0800, 0x4d333985, 2 | BRF_ESS | BRF_PRG }, //  6 Z80 #1 Code
	{ "j0-a",         0x0800, 0x3416a830, 2 | BRF_ESS | BRF_PRG }, //  7

	{ "im5623.f10",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       //  8 Graphics
	{ "im5623.f12",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       //  9
};

STD_ROM_PICK(stratvob)
STD_ROM_FN(stratvob)

struct BurnDriver BurnDrvstratvob = {
	"stratvoxb", "speakres", NULL, NULL, "1980",
	"Stratovox (bootleg)\0", NULL, "bootleg", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, stratvobRomInfo, stratvobRomName, NULL, NULL, DrvInputInfo, stratvoxDIPInfo,
	speakresInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 3, 4
};


// Space Echo (bootleg)

static struct BurnRomInfo spacechoRomDesc[] = {
	{ "rom.a0",       0x0800, 0x40d74dce, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "rom.a1",       0x0800, 0xa5f0a34f, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "rom.a2",       0x0800, 0xcbbb3acb, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "rom.a3",       0x0800, 0x311050ca, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "rom.a4",       0x0800, 0x28943803, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "rom.a5",       0x0800, 0x851c9f28, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "rom.b0",       0x0800, 0xdb45689d, 2 | BRF_ESS | BRF_PRG }, //  6 Z80 #1 Code
	{ "rom.b2",       0x0800, 0x1e074157, 2 | BRF_ESS | BRF_PRG }, //  7
	{ "rom.b3",       0x0800, 0xd50a8b20, 2 | BRF_ESS | BRF_PRG }, //  8

	{ "im5623.f10",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       //  9 Graphics
	{ "im5623.f12",   0x0100, 0x08793ef7, 3 | BRF_GRA },	       // 10
};

STD_ROM_PICK(spacecho)
STD_ROM_FN(spacecho)

static INT32 spacechoInit()
{
	INT32 nRet;

	draw_type = 3;

	nRet = DrvInit();

	memcpy (Rom1 + 0x1000, Rom1 + 0x800, 0x1000);

	return nRet;
}

struct BurnDriver BurnDrvspacecho = {
	"spacecho", "speakres", NULL, NULL, "1980",
	"Space Echo (bootleg)\0", NULL, "bootleg", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, spacechoRomInfo, spacechoRomName, NULL, NULL, DrvInputInfo, stratvoxDIPInfo,
	spacechoInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 3, 4
};


// Mahjong

static struct BurnRomInfo ttmahjngRomDesc[] = {
	{ "ju04",         0x1000, 0xfe7c693a, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 #0 Code
	{ "ju05",         0x1000, 0x985723d3, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "ju06",         0x1000, 0x2cd69bc8, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "ju07",         0x1000, 0x30e8ec63, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "ju01",         0x0800, 0x0f05ca3c, 2 | BRF_ESS | BRF_PRG }, //  4 Z80 #1 Code
	{ "ju02",         0x0800, 0xc1ffeceb, 2 | BRF_ESS | BRF_PRG }, //  5
	{ "ju08",         0x0800, 0x2dcc76b5, 2 | BRF_ESS | BRF_PRG }, //  6

	{ "ju03",         0x0100, 0x27d47624, 3 | BRF_GRA },	       //  7 Graphics
	{ "ju09",         0x0100, 0x27d47624, 3 | BRF_GRA },	       //  8
};

STD_ROM_PICK(ttmahjng)
STD_ROM_FN(ttmahjng)

static INT32 ttmahjngInit()
{
	draw_type = 1;

	return DrvInit();
}

struct BurnDriver BurnDrvttmahjng = {
	"ttmahjng", NULL, NULL, NULL, "1980",
	"Mahjong\0", NULL, "Taito", "Route 16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_MAHJONG, 0,
	NULL, ttmahjngRomInfo, ttmahjngRomName, NULL, NULL, mahjongInputInfo, NULL,
	ttmahjngInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0,
	256, 256, 4, 3
};


