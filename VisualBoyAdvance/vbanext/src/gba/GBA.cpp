#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __LIBSNES__
#include <stddef.h>
#endif

#ifdef __CELLOS_LV2__
#include <ppu_intrinsics.h>
#endif

static const int table [0x40] =
{
		0xFF10,     0,0xFF11,0xFF12,0xFF13,0xFF14,     0,     0,
		0xFF16,0xFF17,     0,     0,0xFF18,0xFF19,     0,     0,
		0xFF1A,     0,0xFF1B,0xFF1C,0xFF1D,0xFF1E,     0,     0,
		0xFF20,0xFF21,     0,     0,0xFF22,0xFF23,     0,     0,
		0xFF24,0xFF25,     0,     0,0xFF26,     0,     0,     0,
		     0,     0,     0,     0,     0,     0,     0,     0,
		0xFF30,0xFF31,0xFF32,0xFF33,0xFF34,0xFF35,0xFF36,0xFF37,
		0xFF38,0xFF39,0xFF3A,0xFF3B,0xFF3C,0xFF3D,0xFF3E,0xFF3F,
};

#include "GBA.h"
#include "GBAcpu.h"
#include "GBAinline.h"
#include "Globals.h"
#include "EEprom.h"
#include "Flash.h"
#include "Sound.h"
#include "Sram.h"
#include "bios_.h"
#include "../NLS.h"
#ifdef USE_CHEATS
#include "Cheats.h"
#endif
#ifdef ELF
#include "elf.h"
#endif
#include "../Util.h"
#include "../common/Port.h"
#include "../System.h"

bus_t bus;
graphics_t graphics;

static int clockTicks;

#define CPU_UPDATE_CPSR() \
{ \
	uint32_t CPSR; \
	CPSR = bus.reg[16].I & 0x40; \
	if(N_FLAG) \
		CPSR |= 0x80000000; \
	if(Z_FLAG) \
		CPSR |= 0x40000000; \
	if(C_FLAG) \
		CPSR |= 0x20000000; \
	if(V_FLAG) \
		CPSR |= 0x10000000; \
	if(!armState) \
		CPSR |= 0x00000020; \
	if(!armIrqEnable) \
		CPSR |= 0x80; \
	CPSR |= (armMode & 0x1F); \
	bus.reg[16].I = CPSR; \
}

bool N_FLAG = 0;
bool C_FLAG = 0;
bool Z_FLAG = 0;
bool V_FLAG = 0;

#include "GBA-arm_.h"
#include "GBA-thumb_.h"

static int romSize = 0x2000000;
uint32_t line[6][240];
bool gfxInWin[2][240];
int lineOBJpixleft[128];
uint64_t joy = 0;

int gfxBG2Changed = 0;
int gfxBG3Changed = 0;

int gfxBG2X = 0;
int gfxBG2Y = 0;
int gfxBG3X = 0;
int gfxBG3Y = 0;

//int gfxLastVCOUNT = 0;

#include "GBAGfx.h"

int coeff[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

// GLOBALS.CPP 


memoryMap map[256];
bool ioReadable[0x400];
bool armState = true;
bool armIrqEnable = true;
int armMode = 0x1f;
uint32_t stop = 0x08000568;
int saveType = 0;
bool useBios = false;
bool skipBios = false;
#ifdef USE_FRAMESKIP
int frameSkip = 1;
bool speedup = false;
#endif
bool cpuIsMultiBoot = false;
bool speedHack = true;
int cpuSaveType = 0;
#ifdef USE_CHEATS
bool cheatsEnabled = true;
#else
bool cheatsEnabled = false;
#endif
bool enableRtc = false;
bool mirroringEnable = false;
bool skipSaveGameBattery = false;
bool skipSaveGameCheats = false;

uint8_t *bios = 0;
uint8_t *rom = 0;
uint8_t *internalRAM = 0;
uint8_t *workRAM = 0;
uint8_t *vram = 0;
uint8_t *pix = 0;
uint8_t *oam = 0;
uint8_t *ioMem = 0;

uint16_t VCOUNT   = 0x0000;
uint16_t BG0CNT   = 0x0000;
uint16_t BG1CNT   = 0x0000;
uint16_t BG2CNT   = 0x0000;
uint16_t BG3CNT   = 0x0000;
uint16_t BG0HOFS  = 0x0000;
uint16_t BG0VOFS  = 0x0000;
uint16_t BG1HOFS  = 0x0000;
uint16_t BG1VOFS  = 0x0000;
uint16_t BG2HOFS  = 0x0000;
uint16_t BG2VOFS  = 0x0000;
uint16_t BG3HOFS  = 0x0000;
uint16_t BG3VOFS  = 0x0000;
uint16_t BG2PA    = 0x0100;
uint16_t BG2PB    = 0x0000;
uint16_t BG2PC    = 0x0000;
uint16_t BG2PD    = 0x0100;
uint16_t BG2X_L   = 0x0000;
uint16_t BG2X_H   = 0x0000;
uint16_t BG2Y_L   = 0x0000;
uint16_t BG2Y_H   = 0x0000;
uint16_t BG3PA    = 0x0100;
uint16_t BG3PB    = 0x0000;
uint16_t BG3PC    = 0x0000;
uint16_t BG3PD    = 0x0100;
uint16_t BG3X_L   = 0x0000;
uint16_t BG3X_H   = 0x0000;
uint16_t BG3Y_L   = 0x0000;
uint16_t BG3Y_H   = 0x0000;
uint16_t WIN0H    = 0x0000;
uint16_t WIN1H    = 0x0000;
uint16_t WIN0V    = 0x0000;
uint16_t WIN1V    = 0x0000;
uint16_t WININ    = 0x0000;
uint16_t WINOUT   = 0x0000;
uint16_t MOSAIC   = 0x0000;
uint16_t BLDMOD   = 0x0000;
uint16_t COLEV    = 0x0000;
uint16_t COLY     = 0x0000;
uint16_t DM0SAD_L = 0x0000;
uint16_t DM0SAD_H = 0x0000;
uint16_t DM0DAD_L = 0x0000;
uint16_t DM0DAD_H = 0x0000;
uint16_t DM0CNT_L = 0x0000;
uint16_t DM0CNT_H = 0x0000;
uint16_t DM1SAD_L = 0x0000;
uint16_t DM1SAD_H = 0x0000;
uint16_t DM1DAD_L = 0x0000;
uint16_t DM1DAD_H = 0x0000;
uint16_t DM1CNT_L = 0x0000;
uint16_t DM1CNT_H = 0x0000;
uint16_t DM2SAD_L = 0x0000;
uint16_t DM2SAD_H = 0x0000;
uint16_t DM2DAD_L = 0x0000;
uint16_t DM2DAD_H = 0x0000;
uint16_t DM2CNT_L = 0x0000;
uint16_t DM2CNT_H = 0x0000;
uint16_t DM3SAD_L = 0x0000;
uint16_t DM3SAD_H = 0x0000;
uint16_t DM3DAD_L = 0x0000;
uint16_t DM3DAD_H = 0x0000;
uint16_t DM3CNT_L = 0x0000;
uint16_t DM3CNT_H = 0x0000;
uint16_t TM0D     = 0x0000;
uint16_t TM0CNT   = 0x0000;
uint16_t TM1D     = 0x0000;
uint16_t TM1CNT   = 0x0000;
uint16_t TM2D     = 0x0000;
uint16_t TM2CNT   = 0x0000;
uint16_t TM3D     = 0x0000;
uint16_t TM3CNT   = 0x0000;
uint16_t P1       = 0xFFFF;
uint16_t IE       = 0x0000;
uint16_t IF       = 0x0000;
uint16_t IME      = 0x0000;

//END OF GLOBALS.CPP

#ifdef USE_SWITICKS
int SWITicks = 0;
#endif
int IRQTicks = 0;

uint32_t mastercode = 0;
int cpuDmaTicksToUpdate = 0;
int cpuDmaCount = 0;

int cpuNextEvent = 0;

int gbaSaveType = 0; // used to remember the save type on reset
bool intState = false;
bool stopState = false;
bool holdState = false;
int holdType = 0;
bool cpuSramEnabled = true;
bool cpuFlashEnabled = true;
bool cpuEEPROMEnabled = true;
bool cpuEEPROMSensorEnabled = false;

uint32_t cpuPrefetch[2];

int cpuTotalTicks = 0;

uint8_t timerOnOffDelay = 0;
uint16_t timer0Value = 0;
bool timer0On = false;
int timer0Ticks = 0;
int timer0Reload = 0;
int timer0ClockReload  = 0;
uint16_t timer1Value = 0;
bool timer1On = false;
int timer1Ticks = 0;
int timer1Reload = 0;
int timer1ClockReload  = 0;
uint16_t timer2Value = 0;
bool timer2On = false;
int timer2Ticks = 0;
int timer2Reload = 0;
int timer2ClockReload  = 0;
uint16_t timer3Value = 0;
bool timer3On = false;
int timer3Ticks = 0;
int timer3Reload = 0;
int timer3ClockReload  = 0;
uint32_t dma0Source = 0;
uint32_t dma0Dest = 0;
uint32_t dma1Source = 0;
uint32_t dma1Dest = 0;
uint32_t dma2Source = 0;
uint32_t dma2Dest = 0;
uint32_t dma3Source = 0;
uint32_t dma3Dest = 0;
void (*cpuSaveGameFunc)(uint32_t,uint8_t) = flashSaveDecide;
bool fxOn = false;
bool windowOn = false;
#ifdef USE_FRAMESKIP
int frameCount = 0;
#endif
#ifdef USE_FRAMESKIP
uint32_t lastTime = 0;
#endif
int count = 0;

const uint32_t TIMER_TICKS[4] = {0, 6, 8, 10};

const uint32_t  objTilesAddress [3] = {0x010000, 0x014000, 0x014000};
const uint8_t gamepakRamWaitState[4] = { 4, 3, 2, 8 };
const uint8_t gamepakWaitState[4] =  { 4, 3, 2, 8 };
const uint8_t gamepakWaitState0[2] = { 2, 1 };
const uint8_t gamepakWaitState1[2] = { 4, 1 };
const uint8_t gamepakWaitState2[2] = { 8, 1 };
const bool isInRom [16]=
  { false, false, false, false, false, false, false, false,
    true, true, true, true, true, true, false, false };

uint8_t memoryWait[16] =
  { 0, 0, 2, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 4, 0 };
uint8_t memoryWait32[16] =
  { 0, 0, 5, 0, 0, 1, 1, 0, 7, 7, 9, 9, 13, 13, 4, 0 };
uint8_t memoryWaitSeq[16] =
  { 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 4, 4, 8, 8, 4, 0 };
uint8_t memoryWaitSeq32[16] =
  { 0, 0, 5, 0, 0, 1, 1, 0, 5, 5, 9, 9, 17, 17, 4, 0 };
uint8_t biosProtected[4];

#ifndef LSB_FIRST
bool cpuBiosSwapped = false;
#endif

uint32_t myROM[] = {
0xEA000006,
0xEA000093,
0xEA000006,
0x00000000,
0x00000000,
0x00000000,
0xEA000088,
0x00000000,
0xE3A00302,
0xE1A0F000,
0xE92D5800,
0xE55EC002,
0xE28FB03C,
0xE79BC10C,
0xE14FB000,
0xE92D0800,
0xE20BB080,
0xE38BB01F,
0xE129F00B,
0xE92D4004,
0xE1A0E00F,
0xE12FFF1C,
0xE8BD4004,
0xE3A0C0D3,
0xE129F00C,
0xE8BD0800,
0xE169F00B,
0xE8BD5800,
0xE1B0F00E,
0x0000009C,
0x0000009C,
0x0000009C,
0x0000009C,
0x000001F8,
0x000001F0,
0x000000AC,
0x000000A0,
0x000000FC,
0x00000168,
0xE12FFF1E,
0xE1A03000,
0xE1A00001,
0xE1A01003,
0xE2113102,
0x42611000,
0xE033C040,
0x22600000,
0xE1B02001,
0xE15200A0,
0x91A02082,
0x3AFFFFFC,
0xE1500002,
0xE0A33003,
0x20400002,
0xE1320001,
0x11A020A2,
0x1AFFFFF9,
0xE1A01000,
0xE1A00003,
0xE1B0C08C,
0x22600000,
0x42611000,
0xE12FFF1E,
0xE92D0010,
0xE1A0C000,
0xE3A01001,
0xE1500001,
0x81A000A0,
0x81A01081,
0x8AFFFFFB,
0xE1A0000C,
0xE1A04001,
0xE3A03000,
0xE1A02001,
0xE15200A0,
0x91A02082,
0x3AFFFFFC,
0xE1500002,
0xE0A33003,
0x20400002,
0xE1320001,
0x11A020A2,
0x1AFFFFF9,
0xE0811003,
0xE1B010A1,
0xE1510004,
0x3AFFFFEE,
0xE1A00004,
0xE8BD0010,
0xE12FFF1E,
0xE0010090,
0xE1A01741,
0xE2611000,
0xE3A030A9,
0xE0030391,
0xE1A03743,
0xE2833E39,
0xE0030391,
0xE1A03743,
0xE2833C09,
0xE283301C,
0xE0030391,
0xE1A03743,
0xE2833C0F,
0xE28330B6,
0xE0030391,
0xE1A03743,
0xE2833C16,
0xE28330AA,
0xE0030391,
0xE1A03743,
0xE2833A02,
0xE2833081,
0xE0030391,
0xE1A03743,
0xE2833C36,
0xE2833051,
0xE0030391,
0xE1A03743,
0xE2833CA2,
0xE28330F9,
0xE0000093,
0xE1A00840,
0xE12FFF1E,
0xE3A00001,
0xE3A01001,
0xE92D4010,
0xE3A03000,
0xE3A04001,
0xE3500000,
0x1B000004,
0xE5CC3301,
0xEB000002,
0x0AFFFFFC,
0xE8BD4010,
0xE12FFF1E,
0xE3A0C301,
0xE5CC3208,
0xE15C20B8,
0xE0110002,
0x10222000,
0x114C20B8,
0xE5CC4208,
0xE12FFF1E,
0xE92D500F,
0xE3A00301,
0xE1A0E00F,
0xE510F004,
0xE8BD500F,
0xE25EF004,
0xE59FD044,
0xE92D5000,
0xE14FC000,
0xE10FE000,
0xE92D5000,
0xE3A0C302,
0xE5DCE09C,
0xE35E00A5,
0x1A000004,
0x05DCE0B4,
0x021EE080,
0xE28FE004,
0x159FF018,
0x059FF018,
0xE59FD018,
0xE8BD5000,
0xE169F00C,
0xE8BD5000,
0xE25EF004,
0x03007FF0,
0x09FE2000,
0x09FFC000,
0x03007FE0
};

variable_desc saveGameStruct[] = {
  { &graphics.DISPCNT  , sizeof(uint16_t) },
  { &graphics.DISPSTAT , sizeof(uint16_t) },
  { &VCOUNT   , sizeof(uint16_t) },
  { &BG0CNT   , sizeof(uint16_t) },
  { &BG1CNT   , sizeof(uint16_t) },
  { &BG2CNT   , sizeof(uint16_t) },
  { &BG3CNT   , sizeof(uint16_t) },
  { &BG0HOFS  , sizeof(uint16_t) },
  { &BG0VOFS  , sizeof(uint16_t) },
  { &BG1HOFS  , sizeof(uint16_t) },
  { &BG1VOFS  , sizeof(uint16_t) },
  { &BG2HOFS  , sizeof(uint16_t) },
  { &BG2VOFS  , sizeof(uint16_t) },
  { &BG3HOFS  , sizeof(uint16_t) },
  { &BG3VOFS  , sizeof(uint16_t) },
  { &BG2PA    , sizeof(uint16_t) },
  { &BG2PB    , sizeof(uint16_t) },
  { &BG2PC    , sizeof(uint16_t) },
  { &BG2PD    , sizeof(uint16_t) },
  { &BG2X_L   , sizeof(uint16_t) },
  { &BG2X_H   , sizeof(uint16_t) },
  { &BG2Y_L   , sizeof(uint16_t) },
  { &BG2Y_H   , sizeof(uint16_t) },
  { &BG3PA    , sizeof(uint16_t) },
  { &BG3PB    , sizeof(uint16_t) },
  { &BG3PC    , sizeof(uint16_t) },
  { &BG3PD    , sizeof(uint16_t) },
  { &BG3X_L   , sizeof(uint16_t) },
  { &BG3X_H   , sizeof(uint16_t) },
  { &BG3Y_L   , sizeof(uint16_t) },
  { &BG3Y_H   , sizeof(uint16_t) },
  { &WIN0H    , sizeof(uint16_t) },
  { &WIN1H    , sizeof(uint16_t) },
  { &WIN0V    , sizeof(uint16_t) },
  { &WIN1V    , sizeof(uint16_t) },
  { &WININ    , sizeof(uint16_t) },
  { &WINOUT   , sizeof(uint16_t) },
  { &MOSAIC   , sizeof(uint16_t) },
  { &BLDMOD   , sizeof(uint16_t) },
  { &COLEV    , sizeof(uint16_t) },
  { &COLY     , sizeof(uint16_t) },
  { &DM0SAD_L , sizeof(uint16_t) },
  { &DM0SAD_H , sizeof(uint16_t) },
  { &DM0DAD_L , sizeof(uint16_t) },
  { &DM0DAD_H , sizeof(uint16_t) },
  { &DM0CNT_L , sizeof(uint16_t) },
  { &DM0CNT_H , sizeof(uint16_t) },
  { &DM1SAD_L , sizeof(uint16_t) },
  { &DM1SAD_H , sizeof(uint16_t) },
  { &DM1DAD_L , sizeof(uint16_t) },
  { &DM1DAD_H , sizeof(uint16_t) },
  { &DM1CNT_L , sizeof(uint16_t) },
  { &DM1CNT_H , sizeof(uint16_t) },
  { &DM2SAD_L , sizeof(uint16_t) },
  { &DM2SAD_H , sizeof(uint16_t) },
  { &DM2DAD_L , sizeof(uint16_t) },
  { &DM2DAD_H , sizeof(uint16_t) },
  { &DM2CNT_L , sizeof(uint16_t) },
  { &DM2CNT_H , sizeof(uint16_t) },
  { &DM3SAD_L , sizeof(uint16_t) },
  { &DM3SAD_H , sizeof(uint16_t) },
  { &DM3DAD_L , sizeof(uint16_t) },
  { &DM3DAD_H , sizeof(uint16_t) },
  { &DM3CNT_L , sizeof(uint16_t) },
  { &DM3CNT_H , sizeof(uint16_t) },
  { &TM0D     , sizeof(uint16_t) },
  { &TM0CNT   , sizeof(uint16_t) },
  { &TM1D     , sizeof(uint16_t) },
  { &TM1CNT   , sizeof(uint16_t) },
  { &TM2D     , sizeof(uint16_t) },
  { &TM2CNT   , sizeof(uint16_t) },
  { &TM3D     , sizeof(uint16_t) },
  { &TM3CNT   , sizeof(uint16_t) },
  { &P1       , sizeof(uint16_t) },
  { &IE       , sizeof(uint16_t) },
  { &IF       , sizeof(uint16_t) },
  { &IME      , sizeof(uint16_t) },
  { &holdState, sizeof(bool) },
  { &holdType, sizeof(int) },
  { &graphics.lcdTicks, sizeof(int) },
  { &timer0On , sizeof(bool) },
  { &timer0Ticks , sizeof(int) },
  { &timer0Reload , sizeof(int) },
  { &timer0ClockReload  , sizeof(int) },
  { &timer1On , sizeof(bool) },
  { &timer1Ticks , sizeof(int) },
  { &timer1Reload , sizeof(int) },
  { &timer1ClockReload  , sizeof(int) },
  { &timer2On , sizeof(bool) },
  { &timer2Ticks , sizeof(int) },
  { &timer2Reload , sizeof(int) },
  { &timer2ClockReload  , sizeof(int) },
  { &timer3On , sizeof(bool) },
  { &timer3Ticks , sizeof(int) },
  { &timer3Reload , sizeof(int) },
  { &timer3ClockReload  , sizeof(int) },
  { &dma0Source , sizeof(uint32_t) },
  { &dma0Dest , sizeof(uint32_t) },
  { &dma1Source , sizeof(uint32_t) },
  { &dma1Dest , sizeof(uint32_t) },
  { &dma2Source , sizeof(uint32_t) },
  { &dma2Dest , sizeof(uint32_t) },
  { &dma3Source , sizeof(uint32_t) },
  { &dma3Dest , sizeof(uint32_t) },
  { &fxOn, sizeof(bool) },
  { &windowOn, sizeof(bool) },
  { &N_FLAG , sizeof(bool) },
  { &C_FLAG , sizeof(bool) },
  { &Z_FLAG , sizeof(bool) },
  { &V_FLAG , sizeof(bool) },
  { &armState , sizeof(bool) },
  { &armIrqEnable , sizeof(bool) },
  { &bus.armNextPC , sizeof(uint32_t) },
  { &armMode , sizeof(int) },
  { &saveType , sizeof(int) },
  { NULL, 0 }
};

INLINE int CPUUpdateTicks()
{
	int cpuLoopTicks = graphics.lcdTicks;

	if(soundTicks < cpuLoopTicks)
		cpuLoopTicks = soundTicks;

	if(timer0On && (timer0Ticks < cpuLoopTicks))
		cpuLoopTicks = timer0Ticks;

	if(timer1On && !(TM1CNT & 4) && (timer1Ticks < cpuLoopTicks))
		cpuLoopTicks = timer1Ticks;

	if(timer2On && !(TM2CNT & 4) && (timer2Ticks < cpuLoopTicks))
		cpuLoopTicks = timer2Ticks;

	if(timer3On && !(TM3CNT & 4) && (timer3Ticks < cpuLoopTicks))
		cpuLoopTicks = timer3Ticks;

#ifdef USE_SWITICKS
	if (SWITicks)
	{
		if (SWITicks < cpuLoopTicks)
			cpuLoopTicks = SWITicks;
	}
#endif

	if (IRQTicks)
	{
		if (IRQTicks < cpuLoopTicks)
			cpuLoopTicks = IRQTicks;
	}

	return cpuLoopTicks;
}

#define CPUUpdateWindow0() \
{ \
  int x00_window0 = WIN0H>>8; \
  int x01_window0 = WIN0H & 255; \
  int x00_lte_x01 = x00_window0 <= x01_window0; \
  for(int i = 0; i < 240; i++) \
      gfxInWin[0][i] = ((i >= x00_window0 && i < x01_window0) & x00_lte_x01) | ((i >= x00_window0 || i < x01_window0) & ~x00_lte_x01); \
}

#define CPUUpdateWindow1() \
{ \
  int x00_window1 = WIN1H>>8; \
  int x01_window1 = WIN1H & 255; \
  int x00_lte_x01 = x00_window1 <= x01_window1; \
  for(int i = 0; i < 240; i++) \
   gfxInWin[1][i] = ((i >= x00_window1 && i < x01_window1) & x00_lte_x01) | ((i >= x00_window1 || i < x01_window1) & ~x00_lte_x01); \
}

#define CPUCompareVCOUNT() \
  if(VCOUNT == (graphics.DISPSTAT >> 8)) \
  { \
    graphics.DISPSTAT |= 4; \
    UPDATE_REG(0x04, graphics.DISPSTAT); \
    if(graphics.DISPSTAT & 0x20) \
    { \
      IF |= 4; \
      UPDATE_REG(0x202, IF); \
    } \
  } \
  else \
  { \
    graphics.DISPSTAT &= 0xFFFB; \
    UPDATE_REG(0x4, graphics.DISPSTAT); \
  } \
  if (graphics.layerEnableDelay > 0) \
  { \
      graphics.layerEnableDelay--; \
      if (graphics.layerEnableDelay == 1) \
          graphics.layerEnable = graphics.layerSettings & graphics.DISPCNT; \
  }

#define CPUSoftwareInterrupt_() \
{ \
  uint32_t PC = bus.reg[15].I; \
  bool savedArmState = armState; \
  CPUSwitchMode(0x13, true, false); \
  bus.reg[14].I = PC - (savedArmState ? 4 : 2); \
  bus.reg[15].I = 0x08; \
  armState = true; \
  armIrqEnable = false; \
  bus.armNextPC = 0x08; \
  ARM_PREFETCH; \
  bus.reg[15].I += 4; \
}


static bool CPUWriteState(gzFile gzFile)
{
	utilWriteInt(gzFile, SAVE_GAME_VERSION);

	utilGzWrite(gzFile, &rom[0xa0], 16);

	utilWriteInt(gzFile, useBios);

	utilGzWrite(gzFile, &bus.reg[0], sizeof(bus.reg));

	utilWriteData(gzFile, saveGameStruct);

	// new to version 0.7.1
	utilWriteInt(gzFile, stopState);
	// new to version 0.8
	utilWriteInt(gzFile, IRQTicks);

	utilGzWrite(gzFile, internalRAM, 0x8000);
	utilGzWrite(gzFile, graphics.paletteRAM, 0x400);
	utilGzWrite(gzFile, workRAM, 0x40000);
	utilGzWrite(gzFile, vram, 0x20000);
	utilGzWrite(gzFile, oam, 0x400);
	utilGzWrite(gzFile, pix, 4*241*162);
	utilGzWrite(gzFile, ioMem, 0x400);

	eepromSaveGame(gzFile);
	flashSaveGame(gzFile);
	soundSaveGame(gzFile);

#ifdef USE_CHEATS
	cheatsSaveGame(gzFile);
#endif

	// version 1.5
	rtcSaveGame(gzFile);

	return true;
}

#ifdef __LIBSNES__
unsigned CPUWriteState_libgba(uint8_t* data, unsigned size)
{
	uint8_t *orig = data;

	utilWriteIntMem(data, SAVE_GAME_VERSION);
	utilWriteMem(data, &rom[0xa0], 16);
	utilWriteIntMem(data, useBios);
	utilWriteMem(data, &bus.reg[0], sizeof(bus.reg));

	utilWriteDataMem(data, saveGameStruct);

	utilWriteIntMem(data, stopState);
	utilWriteIntMem(data, IRQTicks);

	utilWriteMem(data, internalRAM, 0x8000);
	utilWriteMem(data, graphics.paletteRAM, 0x400);
	utilWriteMem(data, workRAM, 0x40000);
	utilWriteMem(data, vram, 0x20000);
	utilWriteMem(data, oam, 0x400);
	utilWriteMem(data, pix, 4 * 241 * 162);
	utilWriteMem(data, ioMem, 0x400);

	eepromSaveGameMem(data);
	flashSaveGameMem(data);
	soundSaveGameMem(data);
	rtcSaveGameMem(data);

	return (ptrdiff_t)data - (ptrdiff_t)orig;
}
#endif

bool CPUWriteState(const char *file)
{
	gzFile gzFile = utilGzOpen(file, "wb");

	if(gzFile == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), file);
#endif
		return false;
	}

	bool res = CPUWriteState(gzFile);

	utilGzClose(gzFile);

	return res;
}

bool CPUWriteMemState(char *memory, int available)
{
	gzFile gzFile = utilMemGzOpen(memory, available, "w");

	if(gzFile == NULL)
		return false;

	bool res = CPUWriteState(gzFile);

	long pos = utilGzMemTell(gzFile)+8;

	if(pos >= (available))
		res = false;

	utilGzClose(gzFile);

	return res;
}

bool CPUExportEepromFile(const char *fileName)
{
	if(eepromInUse) {
		FILE *file = fopen(fileName, "wb");

		if(!file) {
			systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"),
					fileName);
			return false;
		}

		for(int i = 0; i < eepromSize;) {
			for(int j = 0; j < 8; j++) {
				if(fwrite(&eepromData[i+7-j], 1, 1, file) != 1) {
					fclose(file);
					return false;
				}
			}
			i += 8;
		}
		fclose(file);
	}
	return true;
}

bool CPUWriteBatteryFile(const char *fileName)
{
	if(gbaSaveType == 0) {
		if(eepromInUse)
			gbaSaveType = 3;
		else switch(saveType) {
			case 1:
				gbaSaveType = 1;
				break;
			case 2:
				gbaSaveType = 2;
				break;
		}
	}

	if((gbaSaveType) && (gbaSaveType!=5)) {
		FILE *file = fopen(fileName, "wb");

		if(!file) {
			systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"),
					fileName);
			return false;
		}

		// only save if Flash/Sram in use or EEprom in use
		if(gbaSaveType != 3) {
			if(gbaSaveType == 2) {
				if(fwrite(flashSaveMemory, 1, flashSize, file) != (size_t)flashSize) {
					fclose(file);
					return false;
				}
			} else {
				if(fwrite(flashSaveMemory, 1, 0x10000, file) != 0x10000) {
					fclose(file);
					return false;
				}
			}
		} else {
			if(fwrite(eepromData, 1, eepromSize, file) != (size_t)eepromSize) {
				fclose(file);
				return false;
			}
		}
		fclose(file);
	}
	return true;
}

bool CPUReadGSASnapshot(const char *fileName)
{
	int i;
	FILE *file = fopen(fileName, "rb");

	if(!file) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
#endif
		return false;
	}

	// check file size to know what we should read
	fseek(file, 0, SEEK_END);

	// long size = ftell(file);
	fseek(file, 0x0, SEEK_SET);
	fread(&i, 1, 4, file);
	fseek(file, i, SEEK_CUR); // Skip SharkPortSave
	fseek(file, 4, SEEK_CUR); // skip some sort of flag
	fread(&i, 1, 4, file); // name length
	fseek(file, i, SEEK_CUR); // skip name
	fread(&i, 1, 4, file); // desc length
	fseek(file, i, SEEK_CUR); // skip desc
	fread(&i, 1, 4, file); // notes length
	fseek(file, i, SEEK_CUR); // skip notes
	int saveSize;
	fread(&saveSize, 1, 4, file); // read length
	saveSize -= 0x1c; // remove header size
	char buffer[17];
	char buffer2[17];
	fread(buffer, 1, 16, file);
	buffer[16] = 0;
	for(i = 0; i < 16; i++)
		if(buffer[i] < 32)
			buffer[i] = 32;
	memcpy(buffer2, &rom[0xa0], 16);
	buffer2[16] = 0;
	for(i = 0; i < 16; i++)
		if(buffer2[i] < 32)
			buffer2[i] = 32;
	if(memcmp(buffer, buffer2, 16)) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_CANNOT_IMPORT_SNAPSHOT_FOR,
				N_("Cannot import snapshot for %s. Current game is %s"),
				buffer,
				buffer2);
#endif
		fclose(file);
		return false;
	}
	fseek(file, 12, SEEK_CUR); // skip some flags
	if(saveSize >= 65536) {
		if(fread(flashSaveMemory, 1, saveSize, file) != (size_t)saveSize) {
			fclose(file);
			return false;
		}
	} else {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_UNSUPPORTED_SNAPSHOT_FILE,
				N_("Unsupported snapshot file %s"),
				fileName);
#endif
		fclose(file);
		return false;
	}
	fclose(file);
	CPUReset();
	return true;
}

bool CPUReadGSASPSnapshot(const char *fileName)
{
	const char gsvfooter[] = "xV4\x12";
	const size_t namepos=0x0c, namesz=12;
	const size_t footerpos=0x42c, footersz=4;

	char footer[footersz+1], romname[namesz+1], savename[namesz+1];;
	FILE *file = fopen(fileName, "rb");

	if(!file) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
#endif
		return false;
	}

	// read save name
	fseek(file, namepos, SEEK_SET);
	fread(savename, 1, namesz, file);
	savename[namesz] = 0;

	memcpy(romname, &rom[0xa0], namesz);
	romname[namesz] = 0;

	if(memcmp(romname, savename, namesz)) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_CANNOT_IMPORT_SNAPSHOT_FOR,
				N_("Cannot import snapshot for %s. Current game is %s"),
				savename,
				romname);
#endif
		fclose(file);
		return false;
	}

	// read footer tag
	fseek(file, footerpos, SEEK_SET);
	fread(footer, 1, footersz, file);
	footer[footersz] = 0;

	if(memcmp(footer, gsvfooter, footersz)) {
#ifdef CELL_VBA_DEBUG
		systemMessage(0,
				N_("Unsupported snapshot file %s. Footer '%s' at %u should be '%s'"),
				fileName,
				footer,
				footerpos,
				gsvfooter);
#endif
		fclose(file);
		return false;
	}

	// Read up to 128k save
	fread(flashSaveMemory, 1, FLASH_128K_SZ, file);

	fclose(file);
	CPUReset();
	return true;
}

bool CPUWriteGSASnapshot(const char *fileName, const char *title, const char *desc, const char *notes)
{
	FILE *file = fopen(fileName, "wb");

	if(!file) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
#endif
		return false;
	}

	uint8_t buffer[17];

	utilPutDword(buffer, 0x0d); // SharkPortSave length
	fwrite(buffer, 1, 4, file);
	fwrite("SharkPortSave", 1, 0x0d, file);
	utilPutDword(buffer, 0x000f0000);
	fwrite(buffer, 1, 4, file); // save type 0x000f0000 = GBA save
	utilPutDword(buffer, (uint32_t)strlen(title));
	fwrite(buffer, 1, 4, file); // title length
	fwrite(title, 1, strlen(title), file);
	utilPutDword(buffer, (uint32_t)strlen(desc));
	fwrite(buffer, 1, 4, file); // desc length
	fwrite(desc, 1, strlen(desc), file);
	utilPutDword(buffer, (uint32_t)strlen(notes));
	fwrite(buffer, 1, 4, file); // notes length
	fwrite(notes, 1, strlen(notes), file);
	int saveSize = 0x10000;
	if(gbaSaveType == 2)
		saveSize = flashSize;
	int totalSize = saveSize + 0x1c;

	utilPutDword(buffer, totalSize); // length of remainder of save - CRC
	fwrite(buffer, 1, 4, file);

	char *temp = new char[0x2001c];
	memset(temp, 0, 28);
	memcpy(temp, &rom[0xa0], 16); // copy internal name
	temp[0x10] = rom[0xbe]; // reserved area (old checksum)
	temp[0x11] = rom[0xbf]; // reserved area (old checksum)
	temp[0x12] = rom[0xbd]; // complement check
	temp[0x13] = rom[0xb0]; // maker
	temp[0x14] = 1; // 1 save ?
	memcpy(&temp[0x1c], flashSaveMemory, saveSize); // copy save
	fwrite(temp, 1, totalSize, file); // write save + header
	uint32_t crc = 0;

	for(int i = 0; i < totalSize; i++) {
		crc += ((uint32_t)temp[i] << (crc % 0x18));
	}

	utilPutDword(buffer, crc);
	fwrite(buffer, 1, 4, file); // CRC?

	fclose(file);
	delete [] temp;
	return true;
}

bool CPUImportEepromFile(const char *fileName)
{
	FILE *file = fopen(fileName, "rb");

	if(!file)
		return false;

	// check file size to know what we should read
	fseek(file, 0, SEEK_END);

	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	if(size == 512 || size == 0x2000) {
		if(fread(eepromData, 1, size, file) != (size_t)size) {
			fclose(file);
			return false;
		}
		for(int i = 0; i < size;) {
			uint8_t tmp = eepromData[i];
			eepromData[i] = eepromData[7-i];
			eepromData[7-i] = tmp;
			i++;
			tmp = eepromData[i];
			eepromData[i] = eepromData[7-i];
			eepromData[7-i] = tmp;
			i++;
			tmp = eepromData[i];
			eepromData[i] = eepromData[7-i];
			eepromData[7-i] = tmp;
			i++;
			tmp = eepromData[i];
			eepromData[i] = eepromData[7-i];
			eepromData[7-i] = tmp;
			i++;
			i += 4;
		}
	} else {
		fclose(file);
		return false;
	}
	fclose(file);
	return true;
}

bool CPUReadBatteryFile(const char *fileName)
{
	FILE *file = fopen(fileName, "rb");

	if(!file)
		return false;

	// check file size to know what we should read
	fseek(file, 0, SEEK_END);

	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	if(size == 512 || size == 0x2000) {
		if(fread(eepromData, 1, size, file) != (size_t)size) {
			fclose(file);
			return false;
		}
	} else {
		if(size == 0x20000) {
			if(fread(flashSaveMemory, 1, 0x20000, file) != 0x20000) {
				fclose(file);
				return false;
			}
			flashSetSize(0x20000);
		} else {
			if(fread(flashSaveMemory, 1, 0x10000, file) != 0x10000) {
				fclose(file);
				return false;
			}
			flashSetSize(0x10000);
		}
	}
	fclose(file);
	return true;
}

bool CPUIsGBABios(const char * file)
{
	if(strlen(file) > 4) {
		const char * p = strrchr(file,'.');

		if(p != NULL) {
			if(strcasecmp(p, ".gba") == 0)
				return true;
			if(strcasecmp(p, ".agb") == 0)
				return true;
			if(strcasecmp(p, ".bin") == 0)
				return true;
			if(strcasecmp(p, ".bios") == 0)
				return true;
			if(strcasecmp(p, ".rom") == 0)
				return true;
		}
	}

	return false;
}

bool CPUIsELF(const char *file)
{
	if(file == NULL)
		return false;

	if(strlen(file) > 4) {
		const char * p = strrchr(file,'.');

		if(p != NULL) {
			if(strcasecmp(p, ".elf") == 0)
				return true;
		}
	}
	return false;
}

static void CPUCleanUp()
{
	if(rom != NULL) {
		free(rom);
		rom = NULL;
	}

	if(vram != NULL) {
		free(vram);
		vram = NULL;
	}

	if(graphics.paletteRAM != NULL) {
		free(graphics.paletteRAM);
		graphics.paletteRAM = NULL;
	}

	if(internalRAM != NULL) {
		free(internalRAM);
		internalRAM = NULL;
	}

	if(workRAM != NULL) {
		free(workRAM);
		workRAM = NULL;
	}

	if(bios != NULL) {
		free(bios);
		bios = NULL;
	}

	if(pix != NULL) {
		free(pix);
		pix = NULL;
	}

	if(oam != NULL) {
		free(oam);
		oam = NULL;
	}

	if(ioMem != NULL) {
		free(ioMem);
		ioMem = NULL;
	}


	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	//emulating = 0;
}

int CPULoadRom(const char *szFile)
{
	romSize = 0x2000000;
	if(rom != NULL) {
		CPUCleanUp();
	}

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	rom = (uint8_t *)malloc(0x2000000);
	if(rom == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"ROM");
#endif
		return 0;
	}
	workRAM = (uint8_t *)calloc(1, 0x40000);
	if(workRAM == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"WRAM");
#endif
		return 0;
	}

	uint8_t *whereToLoad = cpuIsMultiBoot ? workRAM : rom;

		if(szFile!=NULL)
		{
			if(!utilLoad(szFile,
						utilIsGBAImage,
						whereToLoad,
						romSize)) {
				free(rom);
				rom = NULL;
				free(workRAM);
				workRAM = NULL;
				return 0;
			}
		}

	uint16_t *temp = (uint16_t *)(rom+((romSize+1)&~1));
	int i;
	for(i = (romSize+1)&~1; i < 0x2000000; i+=2) {
		WRITE16LE(temp, (i >> 1) & 0xFFFF);
		temp++;
	}

	bios = (uint8_t *)calloc(1,0x4000);
	if(bios == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"BIOS");
#endif
		CPUCleanUp();
		return 0;
	}
	internalRAM = (uint8_t *)calloc(1,0x8000);
	if(internalRAM == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"IRAM");
#endif
		CPUCleanUp();
		return 0;
	}
	graphics.paletteRAM = (uint8_t *)calloc(1,0x400);
	if(graphics.paletteRAM == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"PRAM");
#endif
		CPUCleanUp();
		return 0;
	}
	vram = (uint8_t *)calloc(1, 0x20000);
	if(vram == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"VRAM");
#endif
		CPUCleanUp();
		return 0;
	}
	oam = (uint8_t *)calloc(1, 0x400);
	if(oam == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"OAM");
#endif
		CPUCleanUp();
		return 0;
	}
	pix = (uint8_t *)calloc(1, 4 * 241 * 162);
	if(pix == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"PIX");
#endif
		CPUCleanUp();
		return 0;
	}
	ioMem = (uint8_t *)calloc(1, 0x400);
	if(ioMem == NULL) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_OUT_OF_MEMORY, N_("Failed to allocate memory for %s"),
				"IO");
#endif
		CPUCleanUp();
		return 0;
	}

	flashInit();
	eepromInit();

	memset(line[0], -1, 240 * sizeof(u32));
	memset(line[1], -1, 240 * sizeof(u32));
	memset(line[2], -1, 240 * sizeof(u32));
	memset(line[3], -1, 240 * sizeof(u32));

	return romSize;
}

void doMirroring (bool b)
{
	uint32_t mirroredRomSize = (((romSize)>>20) & 0x3F)<<20;
	uint32_t mirroredRomAddress = romSize;
	if ((mirroredRomSize <=0x800000) && (b))
	{
		mirroredRomAddress = mirroredRomSize;
		if (mirroredRomSize==0)
			mirroredRomSize=0x100000;
		while (mirroredRomAddress<0x01000000)
		{
			memcpy((uint16_t *)(rom+mirroredRomAddress), (uint16_t *)(rom), mirroredRomSize);
			mirroredRomAddress+=mirroredRomSize;
		}
	}
}

#define brightness_switch() \
      switch((BLDMOD >> 6) & 3) \
      { \
         case 2: \
               color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]); \
               break; \
         case 3: \
               color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]); \
               break; \
      }

#define alpha_blend_brightness_switch() \
      if(top2 & (BLDMOD>>8)) \
        color = gfxAlphaBlend(color, back, coeff[COLEV & 0x1F], coeff[(COLEV >> 8) & 0x1F]); \
      else if(BLDMOD & top) \
      { \
         brightness_switch(); \
      }

void mode0RenderLine(uint32_t * lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0100)
		gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line[0]);

	if(graphics.layerEnable & 0x0200)
		gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line[1]);

	if(graphics.layerEnable & 0x0400)
		gfxDrawTextScreen(BG2CNT, BG2HOFS, BG2VOFS, line[2]);

	if(graphics.layerEnable & 0x0800)
		gfxDrawTextScreen(BG3CNT, BG3HOFS, BG3VOFS, line[3]);

	gfxDrawSprites();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; x++)
	{
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		if(line[0][x] < color) {
			color = line[0][x];
			top = 0x01;
		}

		if((uint8_t)(line[1][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[1][x];
			top = 0x02;
		}

		if((uint8_t)(line[2][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[2][x];
			top = 0x04;
		}

		if((uint8_t)(line[3][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[3][x];
			top = 0x08;
		}

		if((uint8_t)(line[4][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[4][x];
			top = 0x10;
		}

		if((top & 0x10) && (color & 0x00010000)) {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			if((uint8_t)(line[0][x]>>24) < (uint8_t)(back >> 24)) {
				back = line[0][x];
				top2 = 0x01;
			}

			if((uint8_t)(line[1][x]>>24) < (uint8_t)(back >> 24)) {
				back = line[1][x];
				top2 = 0x02;
			}

			if((uint8_t)(line[2][x]>>24) < (uint8_t)(back >> 24)) {
				back = line[2][x];
				top2 = 0x04;
			}

			if((uint8_t)(line[3][x]>>24) < (uint8_t)(back >> 24)) {
				back = line[3][x];
				top2 = 0x08;
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
}

void mode0RenderLineNoWindow(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;


	if(graphics.layerEnable & 0x0100)
		gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line[0]);

	if(graphics.layerEnable & 0x0200)
		gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line[1]);

	if(graphics.layerEnable & 0x0400)
		gfxDrawTextScreen(BG2CNT, BG2HOFS, BG2VOFS, line[2]);

	if(graphics.layerEnable & 0x0800)
		gfxDrawTextScreen(BG3CNT, BG3HOFS, BG3VOFS, line[3]);

	gfxDrawSprites();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	int effect = (BLDMOD >> 6) & 3;

	for(int x = 0; x < 240; x++) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		if(line[0][x] < color) {
			color = line[0][x];
			top = 0x01;
		}

		if(line[1][x] < (color & 0xFF000000)) {
			color = line[1][x];
			top = 0x02;
		}

		if(line[2][x] < (color & 0xFF000000)) {
			color = line[2][x];
			top = 0x04;
		}

		if(line[3][x] < (color & 0xFF000000)) {
			color = line[3][x];
			top = 0x08;
		}

		if(line[4][x] < (color & 0xFF000000)) {
			color = line[4][x];
			top = 0x10;
		}

		if(!(color & 0x00010000)) {
			switch(effect) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = backdrop;
							uint8_t top2 = 0x20;
							if((line[0][x] < back) && (top != 0x01))
							{
								back = line[0][x];
								top2 = 0x01;
							}

							if((line[1][x] < (back & 0xFF000000)) && (top != 0x02))
							{
								back = line[1][x];
								top2 = 0x02;
							}

							if((line[2][x] < (back & 0xFF000000)) && (top != 0x04))
							{
								back = line[2][x];
								top2 = 0x04;
							}

							if((line[3][x] < (back & 0xFF000000)) && (top != 0x08))
							{
								back = line[3][x];
								top2 = 0x08;
							}

							if((line[4][x] < (back & 0xFF000000)) && (top != 0x10))
							{
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);

						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		} else {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			if(line[0][x] < back) {
				back = line[0][x];
				top2 = 0x01;
			}

			if(line[1][x] < (back & 0xFF000000)) {
				back = line[1][x];
				top2 = 0x02;
			}

			if(line[2][x] < (back & 0xFF000000)) {
				back = line[2][x];
				top2 = 0x04;
			}

			if(line[3][x] < (back & 0xFF000000)) {
				back = line[3][x];
				top2 = 0x08;
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
}

void mode0RenderLineAll(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	bool inWindow0 = false;
	bool inWindow1 = false;

	if(graphics.layerEnable & 0x2000) {
		uint8_t v0 = WIN0V >> 8;
		uint8_t v1 = WIN0V & 255;
		inWindow0 = ((v0 == v1) && (v0 >= 0xe8));
		if(v1 >= v0)
			inWindow0 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow0 |= (VCOUNT >= v0 || VCOUNT < v1);
	}
	if(graphics.layerEnable & 0x4000) {
		uint8_t v0 = WIN1V >> 8;
		uint8_t v1 = WIN1V & 255;
		inWindow1 = ((v0 == v1) && (v0 >= 0xe8));
		if(v1 >= v0)
			inWindow1 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow1 |= (VCOUNT >= v0 || VCOUNT < v1);
	}

	if((graphics.layerEnable & 0x0100)) {
		gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line[0]);
	}

	if((graphics.layerEnable & 0x0200)) {
		gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line[1]);
	}

	if((graphics.layerEnable & 0x0400)) {
		gfxDrawTextScreen(BG2CNT, BG2HOFS, BG2VOFS, line[2]);
	}

	if((graphics.layerEnable & 0x0800)) {
		gfxDrawTextScreen(BG3CNT, BG3HOFS, BG3VOFS, line[3]);
	}

	gfxDrawSprites();
	gfxDrawOBJWin();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	uint8_t inWin0Mask = WININ & 0xFF;
	uint8_t inWin1Mask = WININ >> 8;
	uint8_t outMask = WINOUT & 0xFF;

	for(int x = 0; x < 240; x++) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;
		uint8_t mask = outMask;

		if(!(line[5][x] & 0x80000000)) {
			mask = WINOUT >> 8;
		}

		int32_t window1_mask = ((inWindow1 & gfxInWin[1][x]) | -(inWindow1 & gfxInWin[1][x])) >> 31;
		int32_t window0_mask = ((inWindow0 & gfxInWin[0][x]) | -(inWindow0 & gfxInWin[0][x])) >> 31;
		mask = (inWin1Mask & window1_mask) | (mask & ~window1_mask);
		mask = (inWin0Mask & window0_mask) | (mask & ~window0_mask);

		if((mask & 1) && (line[0][x] < color)) {
			color = line[0][x];
			top = 0x01;
		}

		if((mask & 2) && ((uint8_t)(line[1][x]>>24) < (uint8_t)(color >> 24))) {
			color = line[1][x];
			top = 0x02;
		}

		if((mask & 4) && ((uint8_t)(line[2][x]>>24) < (uint8_t)(color >> 24))) {
			color = line[2][x];
			top = 0x04;
		}

		if((mask & 8) && ((uint8_t)(line[3][x]>>24) < (uint8_t)(color >> 24))) {
			color = line[3][x];
			top = 0x08;
		}

		if((mask & 16) && ((uint8_t)(line[4][x]>>24) < (uint8_t)(color >> 24))) {
			color = line[4][x];
			top = 0x10;
		}

		if(color & 0x00010000) {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			if((mask & 1) && ((uint8_t)(line[0][x]>>24) < (uint8_t)(back >> 24))) {
				back = line[0][x];
				top2 = 0x01;
			}

			if((mask & 2) && ((uint8_t)(line[1][x]>>24) < (uint8_t)(back >> 24))) {
				back = line[1][x];
				top2 = 0x02;
			}

			if((mask & 4) && ((uint8_t)(line[2][x]>>24) < (uint8_t)(back >> 24))) {
				back = line[2][x];
				top2 = 0x04;
			}

			if((mask & 8) && ((uint8_t)(line[3][x]>>24) < (uint8_t)(back >> 24))) {
				back = line[3][x];
				top2 = 0x08;
			}

			alpha_blend_brightness_switch();
		} else if((mask & 32) && (top & BLDMOD)) {
			// special FX on in the window
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						uint32_t back = backdrop;
						uint8_t top2 = 0x20;
						if(((mask & 1) && (uint8_t)(line[0][x]>>24) < (uint8_t)(back >> 24)) && top != 0x01)
						{
							back = line[0][x];
							top2 = 0x01;
						}

						if(((mask & 2) && (uint8_t)(line[1][x]>>24) < (uint8_t)(back >> 24)) && top != 0x02)
						{
							back = line[1][x];
							top2 = 0x02;
						}

						if(((mask & 4) && (uint8_t)(line[2][x]>>24) < (uint8_t)(back >> 24)) && top != 0x04)
						{
							back = line[2][x];
							top2 = 0x04;
						}

						if(((mask & 8) && (uint8_t)(line[3][x]>>24) < (uint8_t)(back >> 24)) && top != 0x08)
						{
							back = line[3][x];
							top2 = 0x08;
						}

						if(((mask & 16) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) && top != 0x10) {
							back = line[4][x];
							top2 = 0x10;
						}

						if(top2 & (BLDMOD>>8))
							color = gfxAlphaBlend(color, back,
									coeff[COLEV & 0x1F],
									coeff[(COLEV >> 8) & 0x1F]);
					}
					break;
				case 2:
					color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
}

/*
Mode 1 is a tiled graphics mode, but with background layer 2 supporting scaling and rotation.
There is no layer 3 in this mode.
Layers 0 and 1 can be either 16 colours (with 16 different palettes) or 256 colours. 
There are 1024 tiles available.
Layer 2 is 256 colours and allows only 256 tiles.

These routines only render a single line at a time, because of the way the GBA does events.
*/

void mode1RenderLine(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0100) {
		gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line[0]);
	}

	if(graphics.layerEnable & 0x0200) {
		gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line[1]);
	}

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif
		gfxDrawRotScreen(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed, line[2]);
	}

	gfxDrawSprites();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	for(uint32_t x = 0; x < 240u; ++x) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		uint8_t li1 = (uint8_t)(line[1][x]>>24);
		uint8_t li2 = (uint8_t)(line[2][x]>>24);
		uint8_t li4 = (uint8_t)(line[4][x]>>24);	

		uint8_t r = 	(li2 < li1) ? (li2) : (li1);

		if(li4 < r){
			r = 	(li4);
		}

		if(line[0][x] < backdrop) {
			color = line[0][x];
			top = 0x01;
		}

		if(r < (uint8_t)(color >> 24)) {
			if(r == li1){
				color = line[1][x];
				top = 0x02;
			}else if(r == li2){
				color = line[2][x];
				top = 0x04;
			}else if(r == li4){
				color = line[4][x];
				top = 0x10;
			}
		}

		if((top & 0x10) && (color & 0x00010000)) {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			uint8_t li0 = (uint8_t)(line[0][x]>>24);
			uint8_t li1 = (uint8_t)(line[1][x]>>24);
			uint8_t li2 = (uint8_t)(line[2][x]>>24);
			uint8_t r = 	(li1 < li0) ? (li1) : (li0);

			if(li2 < r) {
				r =  (li2);
			}

			if(r < (uint8_t)(back >> 24)) {
				if(r == li0){
					back = line[0][x];
					top2 = 0x01;
				}else if(r == li1){
					back = line[1][x];
					top2 = 0x02;
				}else if(r == li2){
					back = line[2][x];
					top2 = 0x04;
				}
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode1RenderLineNoWindow(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0100) {
		gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line[0]);
	}


	if(graphics.layerEnable & 0x0200) {
		gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line[1]);
	}

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif
		gfxDrawRotScreen(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed, line[2]);
	}

	gfxDrawSprites();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		uint8_t li1 = (uint8_t)(line[1][x]>>24);
		uint8_t li2 = (uint8_t)(line[2][x]>>24);
		uint8_t li4 = (uint8_t)(line[4][x]>>24);	

		uint8_t r = 	(li2 < li1) ? (li2) : (li1);

		if(li4 < r){
			r = 	(li4);
		}

		if(line[0][x] < backdrop) {
			color = line[0][x];
			top = 0x01;
		}

		if(r < (uint8_t)(color >> 24)) {
			if(r == li1){
				color = line[1][x];
				top = 0x02;
			}else if(r == li2){
				color = line[2][x];
				top = 0x04;
			}else if(r == li4){
				color = line[4][x];
				top = 0x10;
			}
		}

		if(!(color & 0x00010000)) {
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = backdrop;
							uint8_t top2 = 0x20;

							if((top != 0x01) && (uint8_t)(line[0][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[0][x];
								top2 = 0x01;
							}

							if((top != 0x02) && (uint8_t)(line[1][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[1][x];
								top2 = 0x02;
							}

							if((top != 0x04) && (uint8_t)(line[2][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);
						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		} else {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			uint8_t li0 = (uint8_t)(line[0][x]>>24);
			uint8_t li1 = (uint8_t)(line[1][x]>>24);
			uint8_t li2 = (uint8_t)(line[2][x]>>24);	

			uint8_t r = 	(li1 < li0) ? (li1) : (li0);

			if(li2 < r) {
				r =  (li2);
			}

			if(r < (uint8_t)(back >> 24))
			{
				if(r == li0)
				{
					back = line[0][x];
					top2 = 0x01;
				}
				else if(r == li1)
				{
					back = line[1][x];
					top2 = 0x02;
				}
				else if(r == li2)
				{
					back = line[2][x];
					top2 = 0x04;
				}
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode1RenderLineAll(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	bool inWindow0 = false;
	bool inWindow1 = false;

	if(graphics.layerEnable & 0x2000) {
		uint8_t v0 = WIN0V >> 8;
		uint8_t v1 = WIN0V & 255;
		inWindow0 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow0 = (((inWindow0 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow0 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow0 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow0 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}
	if(graphics.layerEnable & 0x4000) {
		uint8_t v0 = WIN1V >> 8;
		uint8_t v1 = WIN1V & 255;
		inWindow1 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow1 = (((inWindow1 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow1 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow1 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow1 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}

	if(graphics.layerEnable & 0x0100)
		gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line[0]);

	if(graphics.layerEnable & 0x0200)
		gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line[1]);

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif
		gfxDrawRotScreen(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed, line[2]);
	}

	gfxDrawSprites();
	gfxDrawOBJWin();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	uint8_t inWin0Mask = WININ & 0xFF;
	uint8_t inWin1Mask = WININ >> 8;
	uint8_t outMask = WINOUT & 0xFF;

	for(int x = 0; x < 240; ++x) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;
		uint8_t mask = outMask;

		if(!(line[5][x] & 0x80000000)) {
			mask = WINOUT >> 8;
		}

		int32_t window1_mask = ((inWindow1 & gfxInWin[1][x]) | -(inWindow1 & gfxInWin[1][x])) >> 31;
		int32_t window0_mask = ((inWindow0 & gfxInWin[0][x]) | -(inWindow0 & gfxInWin[0][x])) >> 31;
		mask = (inWin1Mask & window1_mask) | (mask & ~window1_mask);
		mask = (inWin0Mask & window0_mask) | (mask & ~window0_mask);

		// At the very least, move the inexpensive 'mask' operation up front
		if((mask & 1) && line[0][x] < backdrop) {
			color = line[0][x];
			top = 0x01;
		}

		if((mask & 2) && (uint8_t)(line[1][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[1][x];
			top = 0x02;
		}

		if((mask & 4) && (uint8_t)(line[2][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[2][x];
			top = 0x04;
		}

		if((mask & 16) && (uint8_t)(line[4][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[4][x];
			top = 0x10;
		}

		if(color & 0x00010000) {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			if((mask & 1) && (uint8_t)(line[0][x]>>24) < (uint8_t)(backdrop >> 24)) {
				back = line[0][x];
				top2 = 0x01;
			}

			if((mask & 2) && (uint8_t)(line[1][x]>>24) < (uint8_t)(back >> 24)) {
				back = line[1][x];
				top2 = 0x02;
			}

			if((mask & 4) && (uint8_t)(line[2][x]>>24) < (uint8_t)(back >> 24)) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		} else if(mask & 32) {
			// special FX on the window
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = backdrop;
							uint8_t top2 = 0x20;

							if((mask & 1) && (top != 0x01) && (uint8_t)(line[0][x]>>24) < (uint8_t)(backdrop >> 24)) {
								back = line[0][x];
								top2 = 0x01;
							}

							if((mask & 2) && (top != 0x02) && (uint8_t)(line[1][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[1][x];
								top2 = 0x02;
							}

							if((mask & 4) && (top != 0x04) && (uint8_t)(line[2][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((mask & 16) && (top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);
						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

/*
Mode 2 is a 256 colour tiled graphics mode which supports scaling and rotation.
There is no background layer 0 or 1 in this mode. Only background layers 2 and 3.
There are 256 tiles available.
It does not support flipping.

These routines only render a single line at a time, because of the way the GBA does events.
*/

void mode2RenderLine(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD, gfxBG2X, gfxBG2Y,
				changed, line[2]);
	}

	if(graphics.layerEnable & 0x0800) {
		int changed = gfxBG3Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen(BG3CNT, BG3X_L, BG3X_H, BG3Y_L, BG3Y_H,
				BG3PA, BG3PB, BG3PC, BG3PD, gfxBG3X, gfxBG3Y,
				changed, line[3]);
	}

	gfxDrawSprites();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		uint8_t li2 = (uint8_t)(line[2][x]>>24);
		uint8_t li3 = (uint8_t)(line[3][x]>>24);
		uint8_t li4 = (uint8_t)(line[4][x]>>24);	

		uint8_t r = 	(li3 < li2) ? (li3) : (li2);

		if(li4 < r){
			r = 	(li4);
		}

		if(r < (uint8_t)(color >> 24)) {
			if(r == li2){
				color = line[2][x];
				top = 0x04;
			}else if(r == li3){
				color = line[3][x];
				top = 0x08;
			}else if(r == li4){
				color = line[4][x];
				top = 0x10;
			}
		}

		if((top & 0x10) && (color & 0x00010000)) {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			uint8_t li2 = (uint8_t)(line[2][x]>>24);
			uint8_t li3 = (uint8_t)(line[3][x]>>24);
			uint8_t r = 	(li3 < li2) ? (li3) : (li2);

			if(r < (uint8_t)(back >> 24)) {
				if(r == li2){
					back = line[2][x];
					top2 = 0x04;
				}else if(r == li3){
					back = line[3][x];
					top2 = 0x08;
				}
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	gfxBG3Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode2RenderLineNoWindow(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD, gfxBG2X, gfxBG2Y,
				changed, line[2]);
	}

	if(graphics.layerEnable & 0x0800) {
		int changed = gfxBG3Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen(BG3CNT, BG3X_L, BG3X_H, BG3Y_L, BG3Y_H,
				BG3PA, BG3PB, BG3PC, BG3PD, gfxBG3X, gfxBG3Y,
				changed, line[3]);
	}

	gfxDrawSprites();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		uint8_t li2 = (uint8_t)(line[2][x]>>24);
		uint8_t li3 = (uint8_t)(line[3][x]>>24);
		uint8_t li4 = (uint8_t)(line[4][x]>>24);	

		uint8_t r = 	(li3 < li2) ? (li3) : (li2);

		if(li4 < r){
			r = 	(li4);
		}

		if(r < (uint8_t)(color >> 24)) {
			if(r == li2){
				color = line[2][x];
				top = 0x04;
			}else if(r == li3){
				color = line[3][x];
				top = 0x08;
			}else if(r == li4){
				color = line[4][x];
				top = 0x10;
			}
		}

		if(!(color & 0x00010000)) {
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = backdrop;
							uint8_t top2 = 0x20;

							if((top != 0x04) && (uint8_t)(line[2][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((top != 0x08) && (uint8_t)(line[3][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[3][x];
								top2 = 0x08;
							}

							if((top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);
						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		} else {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			uint8_t li2 = (uint8_t)(line[2][x]>>24);
			uint8_t li3 = (uint8_t)(line[3][x]>>24);
			uint8_t r = 	(li3 < li2) ? (li3) : (li2);

			if(r < (uint8_t)(back >> 24)) {
				if(r == li2){
					back = line[2][x];
					top2 = 0x04;
				}else if(r == li3){
					back = line[3][x];
					top2 = 0x08;
				}
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	gfxBG3Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode2RenderLineAll(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	bool inWindow0 = false;
	bool inWindow1 = false;

	if(graphics.layerEnable & 0x2000) {
		uint8_t v0 = WIN0V >> 8;
		uint8_t v1 = WIN0V & 255;
		inWindow0 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow0 = (((inWindow0 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow0 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow0 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow0 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}
	if(graphics.layerEnable & 0x4000) {
		uint8_t v0 = WIN1V >> 8;
		uint8_t v1 = WIN1V & 255;
		inWindow1 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow1 = (((inWindow1 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow1 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow1 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow1 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD, gfxBG2X, gfxBG2Y,
				changed, line[2]);
	}

	if(graphics.layerEnable & 0x0800) {
		int changed = gfxBG3Changed;
#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen(BG3CNT, BG3X_L, BG3X_H, BG3Y_L, BG3Y_H,
				BG3PA, BG3PB, BG3PC, BG3PD, gfxBG3X, gfxBG3Y,
				changed, line[3]);
	}

	gfxDrawSprites();
	gfxDrawOBJWin();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	uint8_t inWin0Mask = WININ & 0xFF;
	uint8_t inWin1Mask = WININ >> 8;
	uint8_t outMask = WINOUT & 0xFF;

	for(int x = 0; x < 240; x++) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;
		uint8_t mask = outMask;

		if(!(line[5][x] & 0x80000000)) {
			mask = WINOUT >> 8;
		}

		int32_t window1_mask = ((inWindow1 & gfxInWin[1][x]) | -(inWindow1 & gfxInWin[1][x])) >> 31;
		int32_t window0_mask = ((inWindow0 & gfxInWin[0][x]) | -(inWindow0 & gfxInWin[0][x])) >> 31;
		mask = (inWin1Mask & window1_mask) | (mask & ~window1_mask);
		mask = (inWin0Mask & window0_mask) | (mask & ~window0_mask);

		if((mask & 4) && line[2][x] < color) {
			color = line[2][x];
			top = 0x04;
		}

		if((mask & 8) && (uint8_t)(line[3][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[3][x];
			top = 0x08;
		}

		if((mask & 16) && (uint8_t)(line[4][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[4][x];
			top = 0x10;
		}

		if(color & 0x00010000) {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			if((mask & 4) && line[2][x] < back) {
				back = line[2][x];
				top2 = 0x04;
			}

			if((mask & 8) && (uint8_t)(line[3][x]>>24) < (uint8_t)(back >> 24)) {
				back = line[3][x];
				top2 = 0x08;
			}

			alpha_blend_brightness_switch();
		} else if(mask & 32) {
			// special FX on the window
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = backdrop;
							uint8_t top2 = 0x20;

							if((mask & 4) && (top != 0x04) && line[2][x] < back) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((mask & 8) && (top != 0x08) && (uint8_t)(line[3][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[3][x];
								top2 = 0x08;
							}

							if((mask & 16) && (top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);
						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	gfxBG3Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

/*
Mode 3 is a 15-bit (32768) colour bitmap graphics mode.
It has a single layer, background layer 2, the same size as the screen.
It doesn't support paging, scrolling, flipping, rotation or tiles.

These routines only render a single line at a time, because of the way the GBA does events.
*/

void mode3RenderLine(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen16Bit(BG2CNT, BG2X_L, BG2X_H,
				BG2Y_L, BG2Y_H, BG2PA, BG2PB,
				BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();

	uint32_t background = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = background;
		uint8_t top = 0x20;

		if(line[2][x] < color) {
			color = line[2][x];
			top = 0x04;
		}

		if((uint8_t)(line[4][x]>>24) < (uint8_t)(color >>24)) {
			color = line[4][x];
			top = 0x10;
		}

		if((top & 0x10) && (color & 0x00010000)) {
			// semi-transparent OBJ
			uint32_t back = background;
			uint8_t top2 = 0x20;

			if(line[2][x] < background) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode3RenderLineNoWindow(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen16Bit(BG2CNT, BG2X_L, BG2X_H,
				BG2Y_L, BG2Y_H, BG2PA, BG2PB,
				BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();

	uint32_t background = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = background;
		uint8_t top = 0x20;

		if(line[2][x] < background) {
			color = line[2][x];
			top = 0x04;
		}

		if((uint8_t)(line[4][x]>>24) < (uint8_t)(color >>24)) {
			color = line[4][x];
			top = 0x10;
		}

		if(!(color & 0x00010000)) {
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = background;
							uint8_t top2 = 0x20;

							if(top != 0x04 && (line[2][x] < background) ) {
								back = line[2][x];
								top2 = 0x04;
							}

							if(top != 0x10 && ((uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24))) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);

						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		} else {
			// semi-transparent OBJ
			uint32_t back = background;
			uint8_t top2 = 0x20;

			if(line[2][x] < background) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode3RenderLineAll(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	bool inWindow0 = false;
	bool inWindow1 = false;

	if(graphics.layerEnable & 0x2000) {
		uint8_t v0 = WIN0V >> 8;
		uint8_t v1 = WIN0V & 255;
		inWindow0 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow0 = (((inWindow0 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow0 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow0 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow0 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}
	if(graphics.layerEnable & 0x4000) {
		uint8_t v0 = WIN1V >> 8;
		uint8_t v1 = WIN1V & 255;
		inWindow1 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow1 = (((inWindow1 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow1 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow1 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow1 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen16Bit(BG2CNT, BG2X_L, BG2X_H,
				BG2Y_L, BG2Y_H, BG2PA, BG2PB,
				BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();
	gfxDrawOBJWin();

	uint8_t inWin0Mask = WININ & 0xFF;
	uint8_t inWin1Mask = WININ >> 8;
	uint8_t outMask = WINOUT & 0xFF;

	uint32_t background = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = background;
		uint8_t top = 0x20;
		uint8_t mask = outMask;

		if(!(line[5][x] & 0x80000000)) {
			mask = WINOUT >> 8;
		}

		int32_t window1_mask = ((inWindow1 & gfxInWin[1][x]) | -(inWindow1 & gfxInWin[1][x])) >> 31;
		int32_t window0_mask = ((inWindow0 & gfxInWin[0][x]) | -(inWindow0 & gfxInWin[0][x])) >> 31;
		mask = (inWin1Mask & window1_mask) | (mask & ~window1_mask);
		mask = (inWin0Mask & window0_mask) | (mask & ~window0_mask);

		if((mask & 4) && line[2][x] < background) {
			color = line[2][x];
			top = 0x04;
		}

		if((mask & 16) && ((uint8_t)(line[4][x]>>24) < (uint8_t)(color >>24))) {
			color = line[4][x];
			top = 0x10;
		}

		if(color & 0x00010000) {
			// semi-transparent OBJ
			uint32_t back = background;
			uint8_t top2 = 0x20;

			if((mask & 4) && line[2][x] < background) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		} else if(mask & 32) {
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = background;
							uint8_t top2 = 0x20;

							if((mask & 4) && (top != 0x04) && line[2][x] < back) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((mask & 16) && (top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);
						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

/*
Mode 4 is a 256 colour bitmap graphics mode with 2 swappable pages.
It has a single layer, background layer 2, the same size as the screen.
It doesn't support scrolling, flipping, rotation or tiles.

These routines only render a single line at a time, because of the way the GBA does events.
*/

void mode4RenderLine(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x400)
	{
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen256(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x)
	{
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		if(line[2][x] < backdrop) {
			color = line[2][x];
			top = 0x04;
		}

		if((uint8_t)(line[4][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[4][x];
			top = 0x10;
		}

		if((top & 0x10) && (color & 0x00010000)) {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			if(line[2][x] < backdrop) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode4RenderLineNoWindow(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x400)
	{
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen256(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x)
	{
		uint32_t color = backdrop;
		uint8_t top = 0x20;

		if(line[2][x] < backdrop) {
			color = line[2][x];
			top = 0x04;
		}

		if((uint8_t)(line[4][x]>>24) < (uint8_t)(color >> 24)) {
			color = line[4][x];
			top = 0x10;
		}

		if(!(color & 0x00010000)) {
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = backdrop;
							uint8_t top2 = 0x20;

							if((top != 0x04) && line[2][x] < backdrop) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);

						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		} else {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			if(line[2][x] < back) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode4RenderLineAll(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	bool inWindow0 = false;
	bool inWindow1 = false;

	if(graphics.layerEnable & 0x2000) {
		uint8_t v0 = WIN0V >> 8;
		uint8_t v1 = WIN0V & 255;
		inWindow0 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow0 = (((inWindow0 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow0 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow0 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow0 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}
	if(graphics.layerEnable & 0x4000)
	{
		uint8_t v0 = WIN1V >> 8;
		uint8_t v1 = WIN1V & 255;
		inWindow1 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow1 = (((inWindow1 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow1 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow1 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow1 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}

	if(graphics.layerEnable & 0x400)
	{
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen256(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
				BG2PA, BG2PB, BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();
	gfxDrawOBJWin();

	uint32_t backdrop = (READ16LE(&palette[0]) | 0x30000000);

	uint8_t inWin0Mask = WININ & 0xFF;
	uint8_t inWin1Mask = WININ >> 8;
	uint8_t outMask = WINOUT & 0xFF;

	for(int x = 0; x < 240; ++x) {
		uint32_t color = backdrop;
		uint8_t top = 0x20;
		uint8_t mask = outMask;

		if(!(line[5][x] & 0x80000000))
			mask = WINOUT >> 8;

		int32_t window1_mask = ((inWindow1 & gfxInWin[1][x]) | -(inWindow1 & gfxInWin[1][x])) >> 31;
		int32_t window0_mask = ((inWindow0 & gfxInWin[0][x]) | -(inWindow0 & gfxInWin[0][x])) >> 31;
		mask = (inWin1Mask & window1_mask) | (mask & ~window1_mask);
		mask = (inWin0Mask & window0_mask) | (mask & ~window0_mask);

		if((mask & 4) && (line[2][x] < backdrop))
		{
			color = line[2][x];
			top = 0x04;
		}

		if((mask & 16) && ((uint8_t)(line[4][x]>>24) < (uint8_t)(color >>24)))
		{
			color = line[4][x];
			top = 0x10;
		}

		if(color & 0x00010000) {
			// semi-transparent OBJ
			uint32_t back = backdrop;
			uint8_t top2 = 0x20;

			if((mask & 4) && line[2][x] < back) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		} else if(mask & 32) {
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = backdrop;
							uint8_t top2 = 0x20;

							if((mask & 4) && (top != 0x04) && (line[2][x] < backdrop)) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((mask & 16) && (top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);
						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

/*
Mode 5 is a low resolution (160x128) 15-bit colour bitmap graphics mode 
with 2 swappable pages!
It has a single layer, background layer 2, lower resolution than the screen.
It doesn't support scrolling, flipping, rotation or tiles.

These routines only render a single line at a time, because of the way the GBA does events.
*/

void mode5RenderLine(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen16Bit160(BG2CNT, BG2X_L, BG2X_H,
				BG2Y_L, BG2Y_H, BG2PA, BG2PB,
				BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();

	uint32_t background;
	background = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = background;
		uint8_t top = 0x20;

		if(line[2][x] < background) {
			color = line[2][x];
			top = 0x04;
		}

		if((uint8_t)(line[4][x]>>24) < (uint8_t)(color >>24)) {
			color = line[4][x];
			top = 0x10;
		}

		if((top & 0x10) && (color & 0x00010000)) {
			// semi-transparent OBJ
			uint32_t back = background;
			uint8_t top2 = 0x20;

			if(line[2][x] < back) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode5RenderLineNoWindow(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0400) {
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen16Bit160(BG2CNT, BG2X_L, BG2X_H,
				BG2Y_L, BG2Y_H, BG2PA, BG2PB,
				BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();

	uint32_t background;
	background = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = background;
		uint8_t top = 0x20;

		if(line[2][x] < background) {
			color = line[2][x];
			top = 0x04;
		}

		if((uint8_t)(line[4][x]>>24) < (uint8_t)(color >>24)) {
			color = line[4][x];
			top = 0x10;
		}

		if(!(color & 0x00010000)) {
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = background;
							uint8_t top2 = 0x20;

							if((top != 0x04) && line[2][x] < background) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);

						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		} else {
			// semi-transparent OBJ
			uint32_t back = background;
			uint8_t top2 = 0x20;

			if(line[2][x] < back) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void mode5RenderLineAll(uint32_t *lineMix)
{
	uint16_t *palette = (uint16_t *)graphics.paletteRAM;

	if(graphics.layerEnable & 0x0400)
	{
		int changed = gfxBG2Changed;

#if 0
		if(gfxLastVCOUNT > VCOUNT)
			changed = 3;
#endif

		gfxDrawRotScreen16Bit160(BG2CNT, BG2X_L, BG2X_H,
				BG2Y_L, BG2Y_H, BG2PA, BG2PB,
				BG2PC, BG2PD,
				gfxBG2X, gfxBG2Y, changed,
				line[2]);
	}

	gfxDrawSprites();
	gfxDrawOBJWin();

	bool inWindow0 = false;
	bool inWindow1 = false;

	if(graphics.layerEnable & 0x2000) {
		uint8_t v0 = WIN0V >> 8;
		uint8_t v1 = WIN0V & 255;
		inWindow0 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow0 = (((inWindow0 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow0 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow0 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow0 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}
	if(graphics.layerEnable & 0x4000) {
		uint8_t v0 = WIN1V >> 8;
		uint8_t v1 = WIN1V & 255;
		inWindow1 = ((v0 == v1) && (v0 >= 0xe8));
#ifndef ORIGINAL_BRANCHES
		uint32_t condition = v1 >= v0;
		int32_t condition_mask = ((condition) | -(condition)) >> 31;
		inWindow1 = (((inWindow1 | (VCOUNT >= v0 && VCOUNT < v1)) & condition_mask) | (((inWindow1 | (VCOUNT >= v0 || VCOUNT < v1)) & ~(condition_mask))));
#else
		if(v1 >= v0)
			inWindow1 |= (VCOUNT >= v0 && VCOUNT < v1);
		else
			inWindow1 |= (VCOUNT >= v0 || VCOUNT < v1);
#endif
	}

	uint8_t inWin0Mask = WININ & 0xFF;
	uint8_t inWin1Mask = WININ >> 8;
	uint8_t outMask = WINOUT & 0xFF;

	uint32_t background;
	background = (READ16LE(&palette[0]) | 0x30000000);

	for(int x = 0; x < 240; ++x) {
		uint32_t color = background;
		uint8_t top = 0x20;
		uint8_t mask = outMask;

		if(!(line[5][x] & 0x80000000)) {
			mask = WINOUT >> 8;
		}

		int32_t window1_mask = ((inWindow1 & gfxInWin[1][x]) | -(inWindow1 & gfxInWin[1][x])) >> 31;
		int32_t window0_mask = ((inWindow0 & gfxInWin[0][x]) | -(inWindow0 & gfxInWin[0][x])) >> 31;
		mask = (inWin1Mask & window1_mask) | (mask & ~window1_mask);
		mask = (inWin0Mask & window0_mask) | (mask & ~window0_mask);

		if((mask & 4) && (line[2][x] < background)) {
			color = line[2][x];
			top = 0x04;
		}

		if((mask & 16) && ((uint8_t)(line[4][x]>>24) < (uint8_t)(color >>24))) {
			color = line[4][x];
			top = 0x10;
		}

		if(color & 0x00010000) {
			// semi-transparent OBJ
			uint32_t back = background;
			uint8_t top2 = 0x20;

			if((mask & 4) && line[2][x] < back) {
				back = line[2][x];
				top2 = 0x04;
			}

			alpha_blend_brightness_switch();
		} else if(mask & 32) {
			switch((BLDMOD >> 6) & 3) {
				case 0:
					break;
				case 1:
					{
						if(top & BLDMOD) {
							uint32_t back = background;
							uint8_t top2 = 0x20;

							if((mask & 4) && (top != 0x04) && (line[2][x] < background)) {
								back = line[2][x];
								top2 = 0x04;
							}

							if((mask & 16) && (top != 0x10) && (uint8_t)(line[4][x]>>24) < (uint8_t)(back >> 24)) {
								back = line[4][x];
								top2 = 0x10;
							}

							if(top2 & (BLDMOD>>8))
								color = gfxAlphaBlend(color, back,
										coeff[COLEV & 0x1F],
										coeff[(COLEV >> 8) & 0x1F]);

						}
					}
					break;
				case 2:
					if(BLDMOD & top)
						color = gfxIncreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
				case 3:
					if(BLDMOD & top)
						color = gfxDecreaseBrightness(color, coeff[COLY & 0x1F]);
					break;
			}
		}

		lineMix[x] = systemColorMap32[color & 0xFFFF];
	}
	gfxBG2Changed = 0;
	//gfxLastVCOUNT = VCOUNT;
}

void (*renderLine)(uint32_t*) = mode0RenderLine;

#define CPUUpdateRender() \
  switch(graphics.DISPCNT & 7) { \
  case 0: \
    if((!fxOn && !windowOn && !(graphics.layerEnable & 0x8000))) \
      renderLine = mode0RenderLine; \
    else if(fxOn && !windowOn && !(graphics.layerEnable & 0x8000)) \
      renderLine = mode0RenderLineNoWindow; \
    else \
      renderLine = mode0RenderLineAll; \
    break; \
  case 1: \
    if((!fxOn && !windowOn && !(graphics.layerEnable & 0x8000))) \
      renderLine = mode1RenderLine; \
    else if(fxOn && !windowOn && !(graphics.layerEnable & 0x8000)) \
      renderLine = mode1RenderLineNoWindow; \
    else \
      renderLine = mode1RenderLineAll; \
    break; \
  case 2: \
    if((!fxOn && !windowOn && !(graphics.layerEnable & 0x8000))) \
      renderLine = mode2RenderLine; \
    else if(fxOn && !windowOn && !(graphics.layerEnable & 0x8000)) \
      renderLine = mode2RenderLineNoWindow; \
    else \
      renderLine = mode2RenderLineAll; \
    break; \
  case 3: \
    if((!fxOn && !windowOn && !(graphics.layerEnable & 0x8000))) \
      renderLine = mode3RenderLine; \
    else if(fxOn && !windowOn && !(graphics.layerEnable & 0x8000)) \
      renderLine = mode3RenderLineNoWindow; \
    else \
      renderLine = mode3RenderLineAll; \
    break; \
  case 4: \
    if((!fxOn && !windowOn && !(graphics.layerEnable & 0x8000))) \
      renderLine = mode4RenderLine; \
    else if(fxOn && !windowOn && !(graphics.layerEnable & 0x8000)) \
      renderLine = mode4RenderLineNoWindow; \
    else \
      renderLine = mode4RenderLineAll; \
    break; \
  case 5: \
    if((!fxOn && !windowOn && !(graphics.layerEnable & 0x8000))) \
      renderLine = mode5RenderLine; \
    else if(fxOn && !windowOn && !(graphics.layerEnable & 0x8000)) \
      renderLine = mode5RenderLineNoWindow; \
    else \
      renderLine = mode5RenderLineAll; \
  default: \
    break; \
  }

#ifdef __LIBSNES__
bool CPUReadState_libgba(const uint8_t* data, unsigned size)
{
	// Don't really care about version.
	int version = utilReadIntMem(data);
	if (version != SAVE_GAME_VERSION)
		return false;

	char romname[16];
	utilReadMem(romname, data, 16);
	if (memcmp(&rom[0xa0], romname, 16) != 0)
		return false;

	// Don't care about use bios ...
	utilReadIntMem(data);

	utilReadMem(&bus.reg[0], data, sizeof(bus.reg));

	utilReadDataMem(data, saveGameStruct);

	stopState = utilReadIntMem(data) ? true : false;

	IRQTicks = utilReadIntMem(data);
	if (IRQTicks > 0)
		intState = true;
	else
	{
		intState = false;
		IRQTicks = 0;
	}

	utilReadMem(internalRAM, data, 0x8000);
	utilReadMem(graphics.paletteRAM, data, 0x400);
	utilReadMem(workRAM, data, 0x40000);
	utilReadMem(vram, data, 0x20000);
	utilReadMem(oam, data, 0x400);
	utilReadMem(pix, data, 4*241*162);
	utilReadMem(ioMem, data, 0x400);

	eepromReadGameMem(data, version);
	flashReadGameMem(data, version);
	soundReadGameMem(data, version);
	rtcReadGameMem(data);

	//// Copypasta stuff ...
	// set pointers!
	graphics.layerEnable = graphics.layerSettings & graphics.DISPCNT;

	CPUUpdateRender();

	memset(line[0], -1, 240 * sizeof(u32));
	memset(line[1], -1, 240 * sizeof(u32));
	memset(line[2], -1, 240 * sizeof(u32));
	memset(line[3], -1, 240 * sizeof(u32));

	CPUUpdateWindow0();
	CPUUpdateWindow1();
	gbaSaveType = 0;
	switch(saveType) {
		case 0:
			cpuSaveGameFunc = flashSaveDecide;
			break;
		case 1:
			cpuSaveGameFunc = sramWrite;
			gbaSaveType = 1;
			break;
		case 2:
			cpuSaveGameFunc = flashWrite;
			gbaSaveType = 2;
			break;
		case 3:
			break;
		case 5:
			gbaSaveType = 5;
			break;
		default:
#ifdef CELL_VBA_DEBUG
			systemMessage(MSG_UNSUPPORTED_SAVE_TYPE,
					N_("Unsupported save type %d"), saveType);
#endif
			break;
	}
	if(eepromInUse)
		gbaSaveType = 3;

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
	if(armState) {
		ARM_PREFETCH;
	} else {
		THUMB_PREFETCH;
	}

	CPUUpdateRegister(0x204, CPUReadHalfWordQuick(0x4000204));

	return true;
}
#endif

static bool CPUReadState(gzFile gzFile)
{
	int version = utilReadInt(gzFile);

	if(version > SAVE_GAME_VERSION || version < SAVE_GAME_VERSION_1) {
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_UNSUPPORTED_VBA_SGM,
				N_("Unsupported VisualBoyAdvance save game version %d"),
				version);
#endif
		return false;
	}

	uint8_t romname[17];

	utilGzRead(gzFile, romname, 16);

	if(memcmp(&rom[0xa0], romname, 16) != 0) {
		romname[16]=0;
		for(int i = 0; i < 16; i++)
			if(romname[i] < 32)
				romname[i] = 32;
#ifdef CELL_VBA_DEBUG
		systemMessage(MSG_CANNOT_LOAD_SGM, N_("Cannot load save game for %s"), romname);
#endif
		return false;
	}

	bool ub = utilReadInt(gzFile) ? true : false;

	if(ub != useBios) {
#ifdef CELL_VBA_DEBUG
		if(useBios)
			systemMessage(MSG_SAVE_GAME_NOT_USING_BIOS,
					N_("Save game is not using the BIOS files"));
		else
			systemMessage(MSG_SAVE_GAME_USING_BIOS,
					N_("Save game is using the BIOS file"));
#endif
		return false;
	}

	utilGzRead(gzFile, &bus.reg[0], sizeof(bus.reg));

	utilReadData(gzFile, saveGameStruct);

	if(version < SAVE_GAME_VERSION_3)
		stopState = false;
	else
		stopState = utilReadInt(gzFile) ? true : false;

	if(version < SAVE_GAME_VERSION_4)
	{
		IRQTicks = 0;
		intState = false;
	}
	else
	{
		IRQTicks = utilReadInt(gzFile);
		if (IRQTicks>0)
			intState = true;
		else
		{
			intState = false;
			IRQTicks = 0;
		}
	}

	utilGzRead(gzFile, internalRAM, 0x8000);
	utilGzRead(gzFile, graphics.paletteRAM, 0x400);
	utilGzRead(gzFile, workRAM, 0x40000);
	utilGzRead(gzFile, vram, 0x20000);
	utilGzRead(gzFile, oam, 0x400);
	if(version < SAVE_GAME_VERSION_6)
		utilGzRead(gzFile, pix, 4*240*160);
	else
		utilGzRead(gzFile, pix, 4*241*162);
	utilGzRead(gzFile, ioMem, 0x400);

	if(skipSaveGameBattery) {
		// skip eeprom data
		eepromReadGameSkip(gzFile, version);
		// skip flash data
		flashReadGameSkip(gzFile, version);
	} else {
		eepromReadGame(gzFile, version);
		flashReadGame(gzFile, version);
	}
	soundReadGame(gzFile, version);

#ifdef USE_CHEATS
	if(version > SAVE_GAME_VERSION_1) {
		if(skipSaveGameCheats) {
			// skip cheats list data
			cheatsReadGameSkip(gzFile, version);
		} else {
			cheatsReadGame(gzFile, version);
		}
	}
#endif

	if(version > SAVE_GAME_VERSION_6) {
		rtcReadGame(gzFile);
	}

	if(version <= SAVE_GAME_VERSION_7) {
		uint32_t temp;
#define SWAP(a,b,c) \
		temp = (a);\
		(a) = (b)<<16|(c);\
		(b) = (temp) >> 16;\
		(c) = (temp) & 0xFFFF;

		SWAP(dma0Source, DM0SAD_H, DM0SAD_L);
		SWAP(dma0Dest,   DM0DAD_H, DM0DAD_L);
		SWAP(dma1Source, DM1SAD_H, DM1SAD_L);
		SWAP(dma1Dest,   DM1DAD_H, DM1DAD_L);
		SWAP(dma2Source, DM2SAD_H, DM2SAD_L);
		SWAP(dma2Dest,   DM2DAD_H, DM2DAD_L);
		SWAP(dma3Source, DM3SAD_H, DM3SAD_L);
		SWAP(dma3Dest,   DM3DAD_H, DM3DAD_L);
	}

	if(version <= SAVE_GAME_VERSION_8) {
		timer0ClockReload = TIMER_TICKS[TM0CNT & 3];
		timer1ClockReload = TIMER_TICKS[TM1CNT & 3];
		timer2ClockReload = TIMER_TICKS[TM2CNT & 3];
		timer3ClockReload = TIMER_TICKS[TM3CNT & 3];

		timer0Ticks = ((0x10000 - TM0D) << timer0ClockReload) - timer0Ticks;
		timer1Ticks = ((0x10000 - TM1D) << timer1ClockReload) - timer1Ticks;
		timer2Ticks = ((0x10000 - TM2D) << timer2ClockReload) - timer2Ticks;
		timer3Ticks = ((0x10000 - TM3D) << timer3ClockReload) - timer3Ticks;
	}

	// set pointers!
	graphics.layerEnable = graphics.layerSettings & graphics.DISPCNT;

	CPUUpdateRender();

	memset(line[0], -1, 240 * sizeof(u32));
	memset(line[1], -1, 240 * sizeof(u32));
	memset(line[2], -1, 240 * sizeof(u32));
	memset(line[3], -1, 240 * sizeof(u32));

	CPUUpdateWindow0();
	CPUUpdateWindow1();
	gbaSaveType = 0;
	switch(saveType) {
		case 0:
			cpuSaveGameFunc = flashSaveDecide;
			break;
		case 1:
			cpuSaveGameFunc = sramWrite;
			gbaSaveType = 1;
			break;
		case 2:
			cpuSaveGameFunc = flashWrite;
			gbaSaveType = 2;
			break;
		case 3:
			break;
		case 5:
			gbaSaveType = 5;
			break;
		default:
#ifdef CELL_VBA_DEBUG
			systemMessage(MSG_UNSUPPORTED_SAVE_TYPE,
					N_("Unsupported save type %d"), saveType);
#endif
			break;
	}
	if(eepromInUse)
		gbaSaveType = 3;

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
	if(armState) {
		ARM_PREFETCH;
	} else {
		THUMB_PREFETCH;
	}

	CPUUpdateRegister(0x204, CPUReadHalfWordQuick(0x4000204));

	return true;
}

bool CPUReadState(const char * file)
{
	gzFile gzFile = utilGzOpen(file, "rb");

	if(gzFile == NULL)
		return false;

	bool res = CPUReadState(gzFile);

	utilGzClose(gzFile);

	return res;
}

bool CPUReadMemState(char *memory, int available)
{
	gzFile gzFile = utilMemGzOpen(memory, available, "r");

	bool res = CPUReadState(gzFile);

	utilGzClose(gzFile);

	return res;
}


void CPUUpdateFlags(bool breakLoop)
{
	uint32_t CPSR = bus.reg[16].I;

	N_FLAG = (CPSR & 0x80000000) ? true: false;
	Z_FLAG = (CPSR & 0x40000000) ? true: false;
	C_FLAG = (CPSR & 0x20000000) ? true: false;
	V_FLAG = (CPSR & 0x10000000) ? true: false;
	armState = (CPSR & 0x20) ? false : true;
	armIrqEnable = (CPSR & 0x80) ? false : true;
	if(breakLoop)
	{
		if (armIrqEnable && (IF & IE) && (IME & 1))
			cpuNextEvent = cpuTotalTicks;
	}
}

void CPUUpdateFlags()
{
	uint32_t CPSR = bus.reg[16].I;

	N_FLAG = (CPSR & 0x80000000) ? true: false;
	Z_FLAG = (CPSR & 0x40000000) ? true: false;
	C_FLAG = (CPSR & 0x20000000) ? true: false;
	V_FLAG = (CPSR & 0x10000000) ? true: false;
	armState = (CPSR & 0x20) ? false : true;
	armIrqEnable = (CPSR & 0x80) ? false : true;
	if (armIrqEnable && (IF & IE) && (IME & 1))
		cpuNextEvent = cpuTotalTicks;
}

#define CPUSwap(a, b) \
a ^= b; \
b ^= a; \
a ^= b;

void CPUSwitchMode(int mode, bool saveState, bool breakLoop)
{
	//  if(armMode == mode)
	//    return;

	CPU_UPDATE_CPSR();

	switch(armMode) {
		case 0x10:
		case 0x1F:
			bus.reg[R13_USR].I = bus.reg[13].I;
			bus.reg[R14_USR].I = bus.reg[14].I;
			bus.reg[17].I = bus.reg[16].I;
			break;
		case 0x11:
			CPUSwap(bus.reg[R8_FIQ].I, bus.reg[8].I);
			CPUSwap(bus.reg[R9_FIQ].I, bus.reg[9].I);
			CPUSwap(bus.reg[R10_FIQ].I, bus.reg[10].I);
			CPUSwap(bus.reg[R11_FIQ].I, bus.reg[11].I);
			CPUSwap(bus.reg[R12_FIQ].I, bus.reg[12].I);
			bus.reg[R13_FIQ].I = bus.reg[13].I;
			bus.reg[R14_FIQ].I = bus.reg[14].I;
			bus.reg[SPSR_FIQ].I = bus.reg[17].I;
			break;
		case 0x12:
			bus.reg[R13_IRQ].I  = bus.reg[13].I;
			bus.reg[R14_IRQ].I  = bus.reg[14].I;
			bus.reg[SPSR_IRQ].I =  bus.reg[17].I;
			break;
		case 0x13:
			bus.reg[R13_SVC].I  = bus.reg[13].I;
			bus.reg[R14_SVC].I  = bus.reg[14].I;
			bus.reg[SPSR_SVC].I =  bus.reg[17].I;
			break;
		case 0x17:
			bus.reg[R13_ABT].I  = bus.reg[13].I;
			bus.reg[R14_ABT].I  = bus.reg[14].I;
			bus.reg[SPSR_ABT].I =  bus.reg[17].I;
			break;
		case 0x1b:
			bus.reg[R13_UND].I  = bus.reg[13].I;
			bus.reg[R14_UND].I  = bus.reg[14].I;
			bus.reg[SPSR_UND].I =  bus.reg[17].I;
			break;
	}

	uint32_t CPSR = bus.reg[16].I;
	uint32_t SPSR = bus.reg[17].I;

	switch(mode) {
		case 0x10:
		case 0x1F:
			bus.reg[13].I = bus.reg[R13_USR].I;
			bus.reg[14].I = bus.reg[R14_USR].I;
			bus.reg[16].I = SPSR;
			break;
		case 0x11:
			CPUSwap(bus.reg[8].I, bus.reg[R8_FIQ].I);
			CPUSwap(bus.reg[9].I, bus.reg[R9_FIQ].I);
			CPUSwap(bus.reg[10].I, bus.reg[R10_FIQ].I);
			CPUSwap(bus.reg[11].I, bus.reg[R11_FIQ].I);
			CPUSwap(bus.reg[12].I, bus.reg[R12_FIQ].I);
			bus.reg[13].I = bus.reg[R13_FIQ].I;
			bus.reg[14].I = bus.reg[R14_FIQ].I;
			if(saveState)
				bus.reg[17].I = CPSR; else
				bus.reg[17].I = bus.reg[SPSR_FIQ].I;
			break;
		case 0x12:
			bus.reg[13].I = bus.reg[R13_IRQ].I;
			bus.reg[14].I = bus.reg[R14_IRQ].I;
			bus.reg[16].I = SPSR;
			if(saveState)
				bus.reg[17].I = CPSR;
			else
				bus.reg[17].I = bus.reg[SPSR_IRQ].I;
			break;
		case 0x13:
			bus.reg[13].I = bus.reg[R13_SVC].I;
			bus.reg[14].I = bus.reg[R14_SVC].I;
			bus.reg[16].I = SPSR;
			if(saveState)
				bus.reg[17].I = CPSR;
			else
				bus.reg[17].I = bus.reg[SPSR_SVC].I;
			break;
		case 0x17:
			bus.reg[13].I = bus.reg[R13_ABT].I;
			bus.reg[14].I = bus.reg[R14_ABT].I;
			bus.reg[16].I = SPSR;
			if(saveState)
				bus.reg[17].I = CPSR;
			else
				bus.reg[17].I = bus.reg[SPSR_ABT].I;
			break;
		case 0x1b:
			bus.reg[13].I = bus.reg[R13_UND].I;
			bus.reg[14].I = bus.reg[R14_UND].I;
			bus.reg[16].I = SPSR;
			if(saveState)
				bus.reg[17].I = CPSR;
			else
				bus.reg[17].I = bus.reg[SPSR_UND].I;
			break;
		default:
#ifdef CELL_VBA_DEBUG
			systemMessage(MSG_UNSUPPORTED_ARM_MODE, N_("Unsupported ARM mode %02x"), mode);
#endif
			break;
	}
	armMode = mode;
	CPUUpdateFlags(breakLoop);
	CPU_UPDATE_CPSR();
}


void CPUSoftwareInterrupt(int comment)
{
	//static bool disableMessage = false;
	if(armState) comment >>= 16;
	if(useBios) {
		CPUSoftwareInterrupt_();
		return;
	}
	// This would be correct, but it causes problems if uncommented
	//  else {
	//    biosProtected = 0xe3a02004;
	//  }

	switch(comment) {
		case 0x00:
			BIOS_SoftReset();
			ARM_PREFETCH;
			break;
		case 0x01:
			BIOS_REGISTER_RAM_RESET();
			break;
		case 0x02:
			holdState = true;
			holdType = -1;
			cpuNextEvent = cpuTotalTicks;
			break;
		case 0x03:
			holdState = true;
			holdType = -1;
			stopState = true;
			cpuNextEvent = cpuTotalTicks;
			break;
		case 0x04:
			CPUSoftwareInterrupt_();
			break;
		case 0x05:
			CPUSoftwareInterrupt_();
			break;
		case 0x06:
			CPUSoftwareInterrupt_();
			break;
		case 0x07:
			CPUSoftwareInterrupt_();
			break;
		case 0x08:
			BIOS_SQRT();
			break;
		case 0x09:
			BIOS_ArcTan();
			break;
		case 0x0A:
			BIOS_ArcTan2();
			break;
		case 0x0B:
			{
#ifdef USE_SWITICKS
				int len = (bus.reg[2].I & 0x1FFFFF) >>1;
				if (!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + len) & 0xe000000) == 0))
				{
					if ((bus.reg[2].I >> 24) & 1)
					{
						if ((bus.reg[2].I >> 26) & 1)
							SWITicks = (7 + memoryWait32[(bus.reg[1].I>>24) & 0xF]) * (len>>1);
						else
							SWITicks = (8 + memoryWait[(bus.reg[1].I>>24) & 0xF]) * (len);
					}
					else
					{
						if ((bus.reg[2].I >> 26) & 1)
							SWITicks = (10 + memoryWait32[(bus.reg[0].I>>24) & 0xF] +
									memoryWait32[(bus.reg[1].I>>24) & 0xF]) * (len>>1);
						else
							SWITicks = (11 + memoryWait[(bus.reg[0].I>>24) & 0xF] +
									memoryWait[(bus.reg[1].I>>24) & 0xF]) * len;
					}
				}
#endif
			}
			BIOS_CpuSet();
			break;
		case 0x0C:
			{
#ifdef USE_SWITICKS
				int len = (bus.reg[2].I & 0x1FFFFF) >>5;
				if (!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + len) & 0xe000000) == 0))
				{
					if ((bus.reg[2].I >> 24) & 1)
						SWITicks = (6 + memoryWait32[(bus.reg[1].I>>24) & 0xF] +
								7 * (memoryWaitSeq32[(bus.reg[1].I>>24) & 0xF] + 1)) * len;
					else
						SWITicks = (9 + memoryWait32[(bus.reg[0].I>>24) & 0xF] +
								memoryWait32[(bus.reg[1].I>>24) & 0xF] +
								7 * (memoryWaitSeq32[(bus.reg[0].I>>24) & 0xF] +
									memoryWaitSeq32[(bus.reg[1].I>>24) & 0xF] + 2)) * len;
				}
#endif
			}
			BIOS_CpuFastSet();
			break;
		case 0x0D:
			BIOS_GET_BIOS_CHECKSUM();
			break;
		case 0x0E:
			BIOS_BgAffineSet();
			break;
		case 0x0F:
			BIOS_ObjAffineSet();
			break;
		case 0x10:
			{
#ifdef USE_SWITICKS
				int len = CPUReadHalfWord(bus.reg[2].I);
				if (!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + len) & 0xe000000) == 0))
					SWITicks = (32 + memoryWait[(bus.reg[0].I>>24) & 0xF]) * len;
#endif
			}
			BIOS_BitUnPack();
			break;
		case 0x11:
#ifdef USE_SWITICKS
			{
				uint32_t len = CPUReadMemory(bus.reg[0].I) >> 8;
				if(!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
					SWITicks = (9 + memoryWait[(bus.reg[1].I>>24) & 0xF]) * len;
			}
#endif
			BIOS_LZ77UnCompWram();
			break;
		case 0x12:
#ifdef USE_SWITICKS
			{
				uint32_t len = CPUReadMemory(bus.reg[0].I) >> 8;
				if(!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
					SWITicks = (19 + memoryWait[(bus.reg[1].I>>24) & 0xF]) * len;
			}
#endif
			BIOS_LZ77UnCompVram();
			break;
		case 0x13:
#ifdef USE_SWITICKS
			{
				uint32_t len = CPUReadMemory(bus.reg[0].I) >> 8;
				if(!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
					SWITicks = (29 + (memoryWait[(bus.reg[0].I>>24) & 0xF]<<1)) * len;
			}
#endif
			BIOS_HuffUnComp();
			break;
		case 0x14:
#ifdef USE_SWITICKS
			{
				uint32_t len = CPUReadMemory(bus.reg[0].I) >> 8;
				if(!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
					SWITicks = (11 + memoryWait[(bus.reg[0].I>>24) & 0xF] +
							memoryWait[(bus.reg[1].I>>24) & 0xF]) * len;
			}
#endif
			BIOS_RLUnCompWram();
			break;
		case 0x15:
#ifdef USE_SWITICKS
			{
				uint32_t len = CPUReadMemory(bus.reg[0].I) >> 9;
				if(!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
					SWITicks = (34 + (memoryWait[(bus.reg[0].I>>24) & 0xF] << 1) +
							memoryWait[(bus.reg[1].I>>24) & 0xF]) * len;
			}
#endif
			BIOS_RLUnCompVram();
			break;
		case 0x16:
#ifdef USE_SWITICKS
			{
				uint32_t len = CPUReadMemory(bus.reg[0].I) >> 8;
				if(!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
					SWITicks = (13 + memoryWait[(bus.reg[0].I>>24) & 0xF] +
							memoryWait[(bus.reg[1].I>>24) & 0xF]) * len;
			}
#endif
			BIOS_Diff8bitUnFilterWram();
			break;
		case 0x17:
#ifdef USE_SWITICKS
			{
				uint32_t len = CPUReadMemory(bus.reg[0].I) >> 9;
				if(!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
					SWITicks = (39 + (memoryWait[(bus.reg[0].I>>24) & 0xF]<<1) +
							memoryWait[(bus.reg[1].I>>24) & 0xF]) * len;
			}
#endif
			BIOS_Diff8bitUnFilterVram();
			break;
		case 0x18:
#ifdef USE_SWITICKS
			{
				uint32_t len = CPUReadMemory(bus.reg[0].I) >> 9;
				if(!(((bus.reg[0].I & 0xe000000) == 0) ||
							((bus.reg[0].I + (len & 0x1fffff)) & 0xe000000) == 0))
					SWITicks = (13 + memoryWait[(bus.reg[0].I>>24) & 0xF] +
							memoryWait[(bus.reg[1].I>>24) & 0xF]) * len;
			}
#endif
			BIOS_Diff16bitUnFilter();
			break;
		case 0x19:
			if(bus.reg[0].I)
				soundPause();
			else
				soundResume();
			break;
		case 0x1F:
			BIOS_MIDI_KEY_2_FREQ();
			break;
		case 0x2A:
			BIOS_SND_DRIVER_JMP_TABLE_COPY();
			// let it go, because we don't really emulate this function
		default:
			break;
	}
}


void doDMA(uint32_t &s, uint32_t &d, uint32_t si, uint32_t di, uint32_t c, int transfer32)
{
	int sm = s >> 24;
	int dm = d >> 24;
	int sw = 0;
	int dw = 0;
	int sc = c;

	cpuDmaCount = c;
	// This is done to get the correct waitstates.
	int32_t sm_gt_15_mask = ((sm>15) | -(sm>15)) >> 31;
	int32_t dm_gt_15_mask = ((dm>15) | -(dm>15)) >> 31;
	sm = ((((15) & sm_gt_15_mask) | ((((sm) & ~(sm_gt_15_mask))))));
	dm = ((((15) & dm_gt_15_mask) | ((((dm) & ~(dm_gt_15_mask))))));

	//if ((sm>=0x05) && (sm<=0x07) || (dm>=0x05) && (dm <=0x07))
	//    blank = (((graphics.DISPSTAT | ((graphics.DISPSTAT>>1)&1))==1) ?  true : false);

	if(transfer32)
	{
		s &= 0xFFFFFFFC;
		if(s < 0x02000000 && (bus.reg[15].I >> 24))
		{
			do
			{
				CPUWriteMemory(d, 0);
				d += di;
				c--;
			}while(c != 0);
		}
		else
		{
			do {
				CPUWriteMemory(d, CPUReadMemory(s));
				d += di;
				s += si;
				c--;
			}while(c != 0);
		}
	}
	else
	{
		s &= 0xFFFFFFFE;
		si = (int)si >> 1;
		di = (int)di >> 1;
		if(s < 0x02000000 && (bus.reg[15].I >> 24))
		{
			do {
				CPUWriteHalfWord(d, 0);
				d += di;
				c--;
			}while(c != 0);
		}
		else
		{
			do{
				CPUWriteHalfWord(d, CPUReadHalfWord(s));
				d += di;
				s += si;
				c--;
			}while(c != 0);
		}
	}

	cpuDmaCount = 0;

	int totalTicks = 0;

	int32_t transfer32_mask = ((transfer32) | -(transfer32)) >> 31;
	sw = ((((1+memoryWaitSeq32[sm & 15]) & transfer32_mask) | ((((1+memoryWaitSeq[sm & 15]))))));
	dw = ((((1+memoryWaitSeq32[dm & 15]) & transfer32_mask) | ((((1+memoryWaitSeq[dm & 15]))))));
	totalTicks = (((((sw+dw)*(sc-1) + 6 + memoryWait32[sm & 15] + memoryWaitSeq32[dm & 15]) & transfer32_mask)) | (((((sw+dw)*(sc-1) + 6 + memoryWait[sm & 15] + memoryWaitSeq[dm & 15]) & ~(transfer32_mask)))));

	cpuDmaTicksToUpdate += totalTicks;
}

void CPUCheckDMA(int reason, int dmamask)
{
	// DMA 0
	if((DM0CNT_H & 0x8000) && (dmamask & 1))
	{
		if(((DM0CNT_H >> 12) & 3) == reason)
		{
#ifdef SWITCH_TO_ARRAYVAL
			//TODO - Which is faster?
			uint32_t sourceIncrement, destIncrement;
			uint32_t arrayval[] = {4, (uint32_t)-4, 0, 4};
			uint32_t condition1 = ((DM0CNT_H >> 7) & 3);
			uint32_t condition2 = ((DM0CNT_H >> 5) & 3);
			sourceIncrement = arrayval[condition1];
			destIncrement = arrayval[condition2];
#else
			uint32_t sourceIncrement = 4;
			uint32_t destIncrement = 4;
			switch((DM0CNT_H >> 7) & 3)
			{
				case 0:
					break;
				case 1:
					sourceIncrement = (uint32_t)-4;
					break;
				case 2:
					sourceIncrement = 0;
					break;
			}
			switch((DM0CNT_H >> 5) & 3)
			{
				case 0:
					break;
				case 1:
					destIncrement = (uint32_t)-4;
					break;
				case 2:
					destIncrement = 0;
					break;
			}
#endif
			doDMA(dma0Source, dma0Dest, sourceIncrement, destIncrement,
					DM0CNT_L ? DM0CNT_L : 0x4000,
					DM0CNT_H & 0x0400);

			if(DM0CNT_H & 0x4000)
			{
				IF |= 0x0100;
				UPDATE_REG(0x202, IF);
				cpuNextEvent = cpuTotalTicks;
			}

			if(((DM0CNT_H >> 5) & 3) == 3) {
				dma0Dest = DM0DAD_L | (DM0DAD_H << 16);
			}

			if(!(DM0CNT_H & 0x0200) || (reason == 0)) {
				DM0CNT_H &= 0x7FFF;
				UPDATE_REG(0xBA, DM0CNT_H);
			}
		}
	}

	// DMA 1
	if((DM1CNT_H & 0x8000) && (dmamask & 2)) {
		if(((DM1CNT_H >> 12) & 3) == reason) {
#ifdef SWITCH_TO_ARRAYVAL
			//TODO - Which is faster?
			uint32_t sourceIncrement, destIncrement;
			uint32_t arrayval[] = {4, (uint32_t)-4, 0, 4};
			uint32_t condition1 = ((DM1CNT_H >> 7) & 3);
			uint32_t condition2 = ((DM1CNT_H >> 5) & 3);
			sourceIncrement = arrayval[condition1];
			destIncrement = arrayval[condition2];
#else
			uint32_t sourceIncrement = 4;
			uint32_t destIncrement = 4;
			switch((DM1CNT_H >> 7) & 3) {
				case 0:
					break;
				case 1:
					sourceIncrement = (uint32_t)-4;
					break;
				case 2:
					sourceIncrement = 0;
					break;
			}
			switch((DM1CNT_H >> 5) & 3) {
				case 0:
					break;
				case 1:
					destIncrement = (uint32_t)-4;
					break;
				case 2:
					destIncrement = 0;
					break;
			}
#endif
			uint32_t di_value, c_value, transfer_value;
			if(reason == 3)
			{
				di_value = 0;
				c_value = 4;
				transfer_value = 0x0400;
			}
			else
			{
				di_value = destIncrement;
				c_value = DM1CNT_L ? DM1CNT_L : 0x4000;
				transfer_value = DM1CNT_H & 0x0400;
			}
			doDMA(dma1Source, dma1Dest, sourceIncrement, di_value, c_value, transfer_value);

			if(DM1CNT_H & 0x4000) {
				IF |= 0x0200;
				UPDATE_REG(0x202, IF);
				cpuNextEvent = cpuTotalTicks;
			}

			if(((DM1CNT_H >> 5) & 3) == 3) {
				dma1Dest = DM1DAD_L | (DM1DAD_H << 16);
			}

			if(!(DM1CNT_H & 0x0200) || (reason == 0)) {
				DM1CNT_H &= 0x7FFF;
				UPDATE_REG(0xC6, DM1CNT_H);
			}
		}
	}

	// DMA 2
	if((DM2CNT_H & 0x8000) && (dmamask & 4)) {
		if(((DM2CNT_H >> 12) & 3) == reason) {
#ifdef SWITCH_TO_ARRAYVAL
			//TODO - Which is faster?
			uint32_t sourceIncrement, destIncrement;
			uint32_t arrayval[] = {4, (uint32_t)-4, 0, 4};
			uint32_t condition1 = ((DM2CNT_H >> 7) & 3);
			uint32_t condition2 = ((DM2CNT_H >> 5) & 3);
			sourceIncrement = arrayval[condition1];
			destIncrement = arrayval[condition2];
#else
			uint32_t sourceIncrement = 4;
			uint32_t destIncrement = 4;
			switch((DM2CNT_H >> 7) & 3) {
				case 0:
					break;
				case 1:
					sourceIncrement = (uint32_t)-4;
					break;
				case 2:
					sourceIncrement = 0;
					break;
			}
			switch((DM2CNT_H >> 5) & 3) {
				case 0:
					break;
				case 1:
					destIncrement = (uint32_t)-4;
					break;
				case 2:
					destIncrement = 0;
					break;
			}
#endif
			uint32_t di_value, c_value, transfer_value;
			if(reason == 3)
			{
				di_value = 0;
				c_value = 4;
				transfer_value = 0x0400;
			}
			else
			{
				di_value = destIncrement;
				c_value = DM2CNT_L ? DM2CNT_L : 0x4000;
				transfer_value = DM2CNT_H & 0x0400;
			}
			doDMA(dma2Source, dma2Dest, sourceIncrement, di_value, c_value, transfer_value);

			if(DM2CNT_H & 0x4000) {
				IF |= 0x0400;
				UPDATE_REG(0x202, IF);
				cpuNextEvent = cpuTotalTicks;
			}

			if(((DM2CNT_H >> 5) & 3) == 3) {
				dma2Dest = DM2DAD_L | (DM2DAD_H << 16);
			}

			if(!(DM2CNT_H & 0x0200) || (reason == 0)) {
				DM2CNT_H &= 0x7FFF;
				UPDATE_REG(0xD2, DM2CNT_H);
			}
		}
	}

	// DMA 3
	if((DM3CNT_H & 0x8000) && (dmamask & 8)) {
		if(((DM3CNT_H >> 12) & 3) == reason) {
#ifdef SWITCH_TO_ARRAYVAL
			//TODO - Which is faster?
			uint32_t sourceIncrement, destIncrement;
			uint32_t arrayval[] = {4, (uint32_t)-4, 0, 4};
			uint32_t condition1 = ((DM3CNT_H >> 7) & 3);
			uint32_t condition2 = ((DM3CNT_H >> 5) & 3);
			sourceIncrement = arrayval[condition1];
			destIncrement = arrayval[condition2];
#else
			uint32_t sourceIncrement = 4;
			uint32_t destIncrement = 4;
			switch((DM3CNT_H >> 7) & 3) {
				case 0:
					break;
				case 1:
					sourceIncrement = (uint32_t)-4;
					break;
				case 2:
					sourceIncrement = 0;
					break;
			}
			switch((DM3CNT_H >> 5) & 3) {
				case 0:
					break;
				case 1:
					destIncrement = (uint32_t)-4;
					break;
				case 2:
					destIncrement = 0;
					break;
			}
#endif
			doDMA(dma3Source, dma3Dest, sourceIncrement, destIncrement,
					DM3CNT_L ? DM3CNT_L : 0x10000,
					DM3CNT_H & 0x0400);
			if(DM3CNT_H & 0x4000) {
				IF |= 0x0800;
				UPDATE_REG(0x202, IF);
				cpuNextEvent = cpuTotalTicks;
			}

			if(((DM3CNT_H >> 5) & 3) == 3) {
				dma3Dest = DM3DAD_L | (DM3DAD_H << 16);
			}

			if(!(DM3CNT_H & 0x0200) || (reason == 0)) {
				DM3CNT_H &= 0x7FFF;
				UPDATE_REG(0xDE, DM3CNT_H);
			}
		}
	}
}

void CPUUpdateRegister(uint32_t address, uint16_t value)
{
	switch(address)
	{
		case 0x00:
			{ // we need to place the following code in { } because we declare & initialize variables in a case statement

				if((value & 7) > 5) // display modes above 0-5 are prohibited
					graphics.DISPCNT = (value & 7);

				bool change = (0 != ((graphics.DISPCNT ^ value) & 0x80));
				bool changeBG = (0 != ((graphics.DISPCNT ^ value) & 0x0F00));
				uint16_t changeBGon = ((~graphics.DISPCNT) & value) & 0x0F00; // these layers are being activated

				graphics.DISPCNT = (value & 0xFFF7); // bit 3 can only be accessed by the BIOS to enable GBC mode
				UPDATE_REG(0x00, graphics.DISPCNT);

				if(changeBGon) {
					graphics.layerEnableDelay = 4;
					graphics.layerEnable = graphics.layerSettings & value & (~changeBGon);
				} else {
					graphics.layerEnable = graphics.layerSettings & value;
					// CPUUpdateTicks();
				}

				windowOn = (graphics.layerEnable & 0x6000) ? true : false;
				if(change && !((value & 0x80)))
				{
					if(!(graphics.DISPSTAT & 1))
					{
						graphics.lcdTicks = 1008;
						graphics.DISPSTAT &= 0xFFFC;
						UPDATE_REG(0x04, graphics.DISPSTAT);
						CPUCompareVCOUNT();
					}
				}
				CPUUpdateRender();
				// we only care about changes in BG0-BG3
				if(changeBG) {
					// CPU Update Render Buffers set to false
					//CPUUpdateRenderBuffers(false);
					if(!(graphics.layerEnable & 0x0100))
						memset(line[0], -1, 240 * sizeof(u32));
					if(!(graphics.layerEnable & 0x0200))
						memset(line[1], -1, 240 * sizeof(u32));
					if(!(graphics.layerEnable & 0x0400))
						memset(line[2], -1, 240 * sizeof(u32));
					if(!(graphics.layerEnable & 0x0800))
						memset(line[3], -1, 240 * sizeof(u32));
				}
				break;
			}
		case 0x04:
			graphics.DISPSTAT = (value & 0xFF38) | (graphics.DISPSTAT & 7);
			UPDATE_REG(0x04, graphics.DISPSTAT);
			break;
		case 0x06:
			// not writable
			break;
		case 0x08:
			BG0CNT = (value & 0xDFCF);
			UPDATE_REG(0x08, BG0CNT);
			break;
		case 0x0A:
			BG1CNT = (value & 0xDFCF);
			UPDATE_REG(0x0A, BG1CNT);
			break;
		case 0x0C:
			BG2CNT = (value & 0xFFCF);
			UPDATE_REG(0x0C, BG2CNT);
			break;
		case 0x0E:
			BG3CNT = (value & 0xFFCF);
			UPDATE_REG(0x0E, BG3CNT);
			break;
		case 0x10:
			BG0HOFS = value & 511;
			UPDATE_REG(0x10, BG0HOFS);
			break;
		case 0x12:
			BG0VOFS = value & 511;
			UPDATE_REG(0x12, BG0VOFS);
			break;
		case 0x14:
			BG1HOFS = value & 511;
			UPDATE_REG(0x14, BG1HOFS);
			break;
		case 0x16:
			BG1VOFS = value & 511;
			UPDATE_REG(0x16, BG1VOFS);
			break;
		case 0x18:
			BG2HOFS = value & 511;
			UPDATE_REG(0x18, BG2HOFS);
			break;
		case 0x1A:
			BG2VOFS = value & 511;
			UPDATE_REG(0x1A, BG2VOFS);
			break;
		case 0x1C:
			BG3HOFS = value & 511;
			UPDATE_REG(0x1C, BG3HOFS);
			break;
		case 0x1E:
			BG3VOFS = value & 511;
			UPDATE_REG(0x1E, BG3VOFS);
			break;
		case 0x20:
			BG2PA = value;
			UPDATE_REG(0x20, BG2PA);
			break;
		case 0x22:
			BG2PB = value;
			UPDATE_REG(0x22, BG2PB);
			break;
		case 0x24:
			BG2PC = value;
			UPDATE_REG(0x24, BG2PC);
			break;
		case 0x26:
			BG2PD = value;
			UPDATE_REG(0x26, BG2PD);
			break;
		case 0x28:
			BG2X_L = value;
			UPDATE_REG(0x28, BG2X_L);
			gfxBG2Changed |= 1;
			break;
		case 0x2A:
			BG2X_H = (value & 0xFFF);
			UPDATE_REG(0x2A, BG2X_H);
			gfxBG2Changed |= 1;
			break;
		case 0x2C:
			BG2Y_L = value;
			UPDATE_REG(0x2C, BG2Y_L);
			gfxBG2Changed |= 2;
			break;
		case 0x2E:
			BG2Y_H = value & 0xFFF;
			UPDATE_REG(0x2E, BG2Y_H);
			gfxBG2Changed |= 2;
			break;
		case 0x30:
			BG3PA = value;
			UPDATE_REG(0x30, BG3PA);
			break;
		case 0x32:
			BG3PB = value;
			UPDATE_REG(0x32, BG3PB);
			break;
		case 0x34:
			BG3PC = value;
			UPDATE_REG(0x34, BG3PC);
			break;
		case 0x36:
			BG3PD = value;
			UPDATE_REG(0x36, BG3PD);
			break;
		case 0x38:
			BG3X_L = value;
			UPDATE_REG(0x38, BG3X_L);
			gfxBG3Changed |= 1;
			break;
		case 0x3A:
			BG3X_H = value & 0xFFF;
			UPDATE_REG(0x3A, BG3X_H);
			gfxBG3Changed |= 1;
			break;
		case 0x3C:
			BG3Y_L = value;
			UPDATE_REG(0x3C, BG3Y_L);
			gfxBG3Changed |= 2;
			break;
		case 0x3E:
			BG3Y_H = value & 0xFFF;
			UPDATE_REG(0x3E, BG3Y_H);
			gfxBG3Changed |= 2;
			break;
		case 0x40:
			WIN0H = value;
			UPDATE_REG(0x40, WIN0H);
			CPUUpdateWindow0();
			break;
		case 0x42:
			WIN1H = value;
			UPDATE_REG(0x42, WIN1H);
			CPUUpdateWindow1();
			break;
		case 0x44:
			WIN0V = value;
			UPDATE_REG(0x44, WIN0V);
			break;
		case 0x46:
			WIN1V = value;
			UPDATE_REG(0x46, WIN1V);
			break;
		case 0x48:
			WININ = value & 0x3F3F;
			UPDATE_REG(0x48, WININ);
			break;
		case 0x4A:
			WINOUT = value & 0x3F3F;
			UPDATE_REG(0x4A, WINOUT);
			break;
		case 0x4C:
			MOSAIC = value;
			UPDATE_REG(0x4C, MOSAIC);
			break;
		case 0x50:
			BLDMOD = value & 0x3FFF;
			UPDATE_REG(0x50, BLDMOD);
			fxOn = ((BLDMOD>>6)&3) != 0;
			CPUUpdateRender();
			break;
		case 0x52:
			COLEV = value & 0x1F1F;
			UPDATE_REG(0x52, COLEV);
			break;
		case 0x54:
			COLY = value & 0x1F;
			UPDATE_REG(0x54, COLY);
			break;
		case 0x60:
		case 0x62:
		case 0x64:
		case 0x68:
		case 0x6c:
		case 0x70:
		case 0x72:
		case 0x74:
		case 0x78:
		case 0x7c:
		case 0x80:
		case 0x84:
			{
				int gb_addr[2] = {address & 0xFF, (address & 0xFF) + 1};
				uint32_t address_array[2] = {address & 0xFF, (address&0xFF)+1};
				uint8_t data_array[2] = {(uint8_t)(value & 0xFF), (uint8_t)(value>>8)};
				gb_addr[0] = table[gb_addr[0] - 0x60];
				gb_addr[1] = table[gb_addr[1] - 0x60];
				soundEvent_u8_parallel(gb_addr, address_array, data_array);
				break;
			}
		case 0x82:
		case 0x88:
		case 0xa0:
		case 0xa2:
		case 0xa4:
		case 0xa6:
		case 0x90:
		case 0x92:
		case 0x94:
		case 0x96:
		case 0x98:
		case 0x9a:
		case 0x9c:
		case 0x9e:
			soundEvent_u16(address&0xFF, value);
			break;
		case 0xB0:
			DM0SAD_L = value;
			UPDATE_REG(0xB0, DM0SAD_L);
			break;
		case 0xB2:
			DM0SAD_H = value & 0x07FF;
			UPDATE_REG(0xB2, DM0SAD_H);
			break;
		case 0xB4:
			DM0DAD_L = value;
			UPDATE_REG(0xB4, DM0DAD_L);
			break;
		case 0xB6:
			DM0DAD_H = value & 0x07FF;
			UPDATE_REG(0xB6, DM0DAD_H);
			break;
		case 0xB8:
			DM0CNT_L = value & 0x3FFF;
			UPDATE_REG(0xB8, 0);
			break;
		case 0xBA:
			{
				bool start = ((DM0CNT_H ^ value) & 0x8000) ? true : false;
				value &= 0xF7E0;

				DM0CNT_H = value;
				UPDATE_REG(0xBA, DM0CNT_H);

				if(start && (value & 0x8000))
				{
					dma0Source = DM0SAD_L | (DM0SAD_H << 16);
					dma0Dest = DM0DAD_L | (DM0DAD_H << 16);
					CPUCheckDMA(0, 1);
				}
			}
			break;
		case 0xBC:
			DM1SAD_L = value;
			UPDATE_REG(0xBC, DM1SAD_L);
			break;
		case 0xBE:
			DM1SAD_H = value & 0x0FFF;
			UPDATE_REG(0xBE, DM1SAD_H);
			break;
		case 0xC0:
			DM1DAD_L = value;
			UPDATE_REG(0xC0, DM1DAD_L);
			break;
		case 0xC2:
			DM1DAD_H = value & 0x07FF;
			UPDATE_REG(0xC2, DM1DAD_H);
			break;
		case 0xC4:
			DM1CNT_L = value & 0x3FFF;
			UPDATE_REG(0xC4, 0);
			break;
		case 0xC6:
			{
				bool start = ((DM1CNT_H ^ value) & 0x8000) ? true : false;
				value &= 0xF7E0;

				DM1CNT_H = value;
				UPDATE_REG(0xC6, DM1CNT_H);

				if(start && (value & 0x8000))
				{
					dma1Source = DM1SAD_L | (DM1SAD_H << 16);
					dma1Dest = DM1DAD_L | (DM1DAD_H << 16);
					CPUCheckDMA(0, 2);
				}
			}
			break;
		case 0xC8:
			DM2SAD_L = value;
			UPDATE_REG(0xC8, DM2SAD_L);
			break;
		case 0xCA:
			DM2SAD_H = value & 0x0FFF;
			UPDATE_REG(0xCA, DM2SAD_H);
			break;
		case 0xCC:
			DM2DAD_L = value;
			UPDATE_REG(0xCC, DM2DAD_L);
			break;
		case 0xCE:
			DM2DAD_H = value & 0x07FF;
			UPDATE_REG(0xCE, DM2DAD_H);
			break;
		case 0xD0:
			DM2CNT_L = value & 0x3FFF;
			UPDATE_REG(0xD0, 0);
			break;
		case 0xD2:
			{
				bool start = ((DM2CNT_H ^ value) & 0x8000) ? true : false;

				value &= 0xF7E0;

				DM2CNT_H = value;
				UPDATE_REG(0xD2, DM2CNT_H);

				if(start && (value & 0x8000)) {
					dma2Source = DM2SAD_L | (DM2SAD_H << 16);
					dma2Dest = DM2DAD_L | (DM2DAD_H << 16);

					CPUCheckDMA(0, 4);
				}
			}
			break;
		case 0xD4:
			DM3SAD_L = value;
			UPDATE_REG(0xD4, DM3SAD_L);
			break;
		case 0xD6:
			DM3SAD_H = value & 0x0FFF;
			UPDATE_REG(0xD6, DM3SAD_H);
			break;
		case 0xD8:
			DM3DAD_L = value;
			UPDATE_REG(0xD8, DM3DAD_L);
			break;
		case 0xDA:
			DM3DAD_H = value & 0x0FFF;
			UPDATE_REG(0xDA, DM3DAD_H);
			break;
		case 0xDC:
			DM3CNT_L = value;
			UPDATE_REG(0xDC, 0);
			break;
		case 0xDE:
			{
				bool start = ((DM3CNT_H ^ value) & 0x8000) ? true : false;

				value &= 0xFFE0;

				DM3CNT_H = value;
				UPDATE_REG(0xDE, DM3CNT_H);

				if(start && (value & 0x8000)) {
					dma3Source = DM3SAD_L | (DM3SAD_H << 16);
					dma3Dest = DM3DAD_L | (DM3DAD_H << 16);
					CPUCheckDMA(0,8);
				}
			}
			break;
		case 0x100:
			timer0Reload = value;
			break;
		case 0x102:
			timer0Value = value;
			timerOnOffDelay|=1;
			cpuNextEvent = cpuTotalTicks;
			break;
		case 0x104:
			timer1Reload = value;
			break;
		case 0x106:
			timer1Value = value;
			timerOnOffDelay|=2;
			cpuNextEvent = cpuTotalTicks;
			break;
		case 0x108:
			timer2Reload = value;
			break;
		case 0x10A:
			timer2Value = value;
			timerOnOffDelay|=4;
			cpuNextEvent = cpuTotalTicks;
			break;
		case 0x10C:
			timer3Reload = value;
			break;
		case 0x10E:
			timer3Value = value;
			timerOnOffDelay|=8;
			cpuNextEvent = cpuTotalTicks;
			break;



		case 0x130:
			P1 |= (value & 0x3FF);
			UPDATE_REG(0x130, P1);
			break;

		case 0x132:
			UPDATE_REG(0x132, value & 0xC3FF);
			break;


		case 0x200:
			IE = value & 0x3FFF;
			UPDATE_REG(0x200, IE);
			if ((IME & 1) && (IF & IE) && armIrqEnable)
				cpuNextEvent = cpuTotalTicks;
			break;
		case 0x202:
			IF ^= (value & IF);
			UPDATE_REG(0x202, IF);
			break;
		case 0x204:
			{
				memoryWait[0x0e] = memoryWaitSeq[0x0e] = gamepakRamWaitState[value & 3];

				memoryWait[0x08] = memoryWait[0x09] = 3;
				memoryWaitSeq[0x08] = memoryWaitSeq[0x09] = 1;

				memoryWait[0x0a] = memoryWait[0x0b] = 3;
				memoryWaitSeq[0x0a] = memoryWaitSeq[0x0b] = 1;

				memoryWait[0x0c] = memoryWait[0x0d] = 3;
				memoryWaitSeq[0x0c] = memoryWaitSeq[0x0d] = 1;

				memoryWait32[8] = memoryWait[8] + memoryWaitSeq[8] + 1;
				memoryWaitSeq32[8] = memoryWaitSeq[8]*2 + 1;

				memoryWait32[9] = memoryWait[9] + memoryWaitSeq[9] + 1;
				memoryWaitSeq32[9] = memoryWaitSeq[9]*2 + 1;

				memoryWait32[10] = memoryWait[10] + memoryWaitSeq[10] + 1;
				memoryWaitSeq32[10] = memoryWaitSeq[10]*2 + 1;

				memoryWait32[11] = memoryWait[11] + memoryWaitSeq[11] + 1;
				memoryWaitSeq32[11] = memoryWaitSeq[11]*2 + 1;

				memoryWait32[12] = memoryWait[12] + memoryWaitSeq[12] + 1;
				memoryWaitSeq32[12] = memoryWaitSeq[12]*2 + 1;

				memoryWait32[13] = memoryWait[13] + memoryWaitSeq[13] + 1;
				memoryWaitSeq32[13] = memoryWaitSeq[13]*2 + 1;

				memoryWait32[14] = memoryWait[14] + memoryWaitSeq[14] + 1;
				memoryWaitSeq32[14] = memoryWaitSeq[14]*2 + 1;

				if((value & 0x4000) == 0x4000)
					bus.busPrefetchEnable = true;
				else
					bus.busPrefetchEnable = false;

				bus.busPrefetch = false;
				bus.busPrefetchCount = 0;

				UPDATE_REG(0x204, value & 0x7FFF);

			}
			break;
		case 0x208:
			IME = value & 1;
			UPDATE_REG(0x208, IME);
			if ((IME & 1) && (IF & IE) && armIrqEnable)
				cpuNextEvent = cpuTotalTicks;
			break;
		case 0x300:
			if(value != 0)
				value &= 0xFFFE;
			UPDATE_REG(0x300, value);
			break;
		default:
			UPDATE_REG(address&0x3FE, value);
			break;
	}
}

uint8_t cpuBitsSet[256];
uint8_t cpuLowestBitSet[256];

void CPUInit(const char *biosFileName, bool useBiosFile)
{
#ifndef LSB_FIRST
	if(!cpuBiosSwapped) {
		for(unsigned int i = 0; i < sizeof(myROM)/4; i++) {
			WRITE32LE(&myROM[i], myROM[i]);
		}
		cpuBiosSwapped = true;
	}
#endif
	gbaSaveType = 0;
	eepromInUse = 0;
	saveType = 0;
	useBios = false;

	if(useBiosFile)
	{
		int size = 0x4000;
		if(utilLoad(biosFileName, CPUIsGBABios, bios, size))
		{
			if(size == 0x4000)
				useBios = true;
#ifdef CELL_VBA_DEBUG
			else
				systemMessage(MSG_INVALID_BIOS_FILE_SIZE, N_("Invalid BIOS file size"));
#endif
		}
	}

	if(!useBios)
		memcpy(bios, myROM, sizeof(myROM));

	int i = 0;

	biosProtected[0] = 0x00;
	biosProtected[1] = 0xf0;
	biosProtected[2] = 0x29;
	biosProtected[3] = 0xe1;

	for(i = 0; i < 256; i++)
	{
		int count = 0;
		int j;
		for(j = 0; j < 8; j++)
			if(i & (1 << j))
				count++;
		cpuBitsSet[i] = count;

		for(j = 0; j < 8; j++)
			if(i & (1 << j))
				break;
		cpuLowestBitSet[i] = j;
	}

	for(i = 0; i < 0x400; i++)
		ioReadable[i] = true;
	for(i = 0x10; i < 0x48; i++)
		ioReadable[i] = false;
	for(i = 0x4c; i < 0x50; i++)
		ioReadable[i] = false;
	for(i = 0x54; i < 0x60; i++)
		ioReadable[i] = false;
	for(i = 0x8c; i < 0x90; i++)
		ioReadable[i] = false;
	for(i = 0xa0; i < 0xb8; i++)
		ioReadable[i] = false;
	for(i = 0xbc; i < 0xc4; i++)
		ioReadable[i] = false;
	for(i = 0xc8; i < 0xd0; i++)
		ioReadable[i] = false;
	for(i = 0xd4; i < 0xdc; i++)
		ioReadable[i] = false;
	for(i = 0xe0; i < 0x100; i++)
		ioReadable[i] = false;
	for(i = 0x110; i < 0x120; i++)
		ioReadable[i] = false;
	for(i = 0x12c; i < 0x130; i++)
		ioReadable[i] = false;
	for(i = 0x138; i < 0x140; i++)
		ioReadable[i] = false;
	for(i = 0x144; i < 0x150; i++)
		ioReadable[i] = false;
	for(i = 0x15c; i < 0x200; i++)
		ioReadable[i] = false;
	for(i = 0x20c; i < 0x300; i++)
		ioReadable[i] = false;
	for(i = 0x304; i < 0x400; i++)
		ioReadable[i] = false;

	if(romSize < 0x1fe2000) {
		*((uint16_t *)&rom[0x1fe209c]) = 0xdffa; // SWI 0xFA
		*((uint16_t *)&rom[0x1fe209e]) = 0x4770; // BX LR
	}

	graphics.layerSettings = 0xff00;
	graphics.layerEnable = 0xff00;
	graphics.layerEnableDelay = 1;
	graphics.DISPCNT = 0x0080;
	graphics.DISPSTAT = 0;
	graphics.lcdTicks = (useBios && !skipBios) ? 1008 : 208;
}

void CPUReset()
{
	if(gbaSaveType == 0)
	{
		if(eepromInUse)
			gbaSaveType = 3;
		else
			switch(saveType)
			{
				case 1:
					gbaSaveType = 1;
					break;
				case 2:
					gbaSaveType = 2;
					break;
			}
	}
	rtcReset();
	// clean registers
	memset(&bus.reg[0], 0, sizeof(bus.reg));
	// clean OAM
	memset(oam, 0, 0x400);
	// clean palette
	memset(graphics.paletteRAM, 0, 0x400);
	// clean picture
	memset(pix, 0, 4*160*240);
	// clean vram
	memset(vram, 0, 0x20000);
	// clean io memory
	memset(ioMem, 0, 0x400);

	graphics.DISPCNT  = 0x0080;
	graphics.DISPSTAT = 0x0000;
	VCOUNT   = (useBios && !skipBios) ? 0 :0x007E;
	BG0CNT   = 0x0000;
	BG1CNT   = 0x0000;
	BG2CNT   = 0x0000;
	BG3CNT   = 0x0000;
	BG0HOFS  = 0x0000;
	BG0VOFS  = 0x0000;
	BG1HOFS  = 0x0000;
	BG1VOFS  = 0x0000;
	BG2HOFS  = 0x0000;
	BG2VOFS  = 0x0000;
	BG3HOFS  = 0x0000;
	BG3VOFS  = 0x0000;
	BG2PA    = 0x0100;
	BG2PB    = 0x0000;
	BG2PC    = 0x0000;
	BG2PD    = 0x0100;
	BG2X_L   = 0x0000;
	BG2X_H   = 0x0000;
	BG2Y_L   = 0x0000;
	BG2Y_H   = 0x0000;
	BG3PA    = 0x0100;
	BG3PB    = 0x0000;
	BG3PC    = 0x0000;
	BG3PD    = 0x0100;
	BG3X_L   = 0x0000;
	BG3X_H   = 0x0000;
	BG3Y_L   = 0x0000;
	BG3Y_H   = 0x0000;
	WIN0H    = 0x0000;
	WIN1H    = 0x0000;
	WIN0V    = 0x0000;
	WIN1V    = 0x0000;
	WININ    = 0x0000;
	WINOUT   = 0x0000;
	MOSAIC   = 0x0000;
	BLDMOD   = 0x0000;
	COLEV    = 0x0000;
	COLY     = 0x0000;
	DM0SAD_L = 0x0000;
	DM0SAD_H = 0x0000;
	DM0DAD_L = 0x0000;
	DM0DAD_H = 0x0000;
	DM0CNT_L = 0x0000;
	DM0CNT_H = 0x0000;
	DM1SAD_L = 0x0000;
	DM1SAD_H = 0x0000;
	DM1DAD_L = 0x0000;
	DM1DAD_H = 0x0000;
	DM1CNT_L = 0x0000;
	DM1CNT_H = 0x0000;
	DM2SAD_L = 0x0000;
	DM2SAD_H = 0x0000;
	DM2DAD_L = 0x0000;
	DM2DAD_H = 0x0000;
	DM2CNT_L = 0x0000;
	DM2CNT_H = 0x0000;
	DM3SAD_L = 0x0000;
	DM3SAD_H = 0x0000;
	DM3DAD_L = 0x0000;
	DM3DAD_H = 0x0000;
	DM3CNT_L = 0x0000;
	DM3CNT_H = 0x0000;
	TM0D     = 0x0000;
	TM0CNT   = 0x0000;
	TM1D     = 0x0000;
	TM1CNT   = 0x0000;
	TM2D     = 0x0000;
	TM2CNT   = 0x0000;
	TM3D     = 0x0000;
	TM3CNT   = 0x0000;
	P1       = 0x03FF;
	IE       = 0x0000;
	IF       = 0x0000;
	IME      = 0x0000;

	armMode = 0x1F;

	if(cpuIsMultiBoot) {
		bus.reg[13].I = 0x03007F00;
		bus.reg[15].I = 0x02000000;
		bus.reg[16].I = 0x00000000;
		bus.reg[R13_IRQ].I = 0x03007FA0;
		bus.reg[R13_SVC].I = 0x03007FE0;
		armIrqEnable = true;
	} else {
		if(useBios && !skipBios) {
			bus.reg[15].I = 0x00000000;
			armMode = 0x13;
			armIrqEnable = false;
		} else {
			bus.reg[13].I = 0x03007F00;
			bus.reg[15].I = 0x08000000;
			bus.reg[16].I = 0x00000000;
			bus.reg[R13_IRQ].I = 0x03007FA0;
			bus.reg[R13_SVC].I = 0x03007FE0;
			armIrqEnable = true;
		}
	}
	armState = true;
	C_FLAG = V_FLAG = N_FLAG = Z_FLAG = false;
	UPDATE_REG(0x00, graphics.DISPCNT);
	UPDATE_REG(0x06, VCOUNT);
	UPDATE_REG(0x20, BG2PA);
	UPDATE_REG(0x26, BG2PD);
	UPDATE_REG(0x30, BG3PA);
	UPDATE_REG(0x36, BG3PD);
	UPDATE_REG(0x130, P1);
	UPDATE_REG(0x88, 0x200);

	// disable FIQ
	bus.reg[16].I |= 0x40;

	CPU_UPDATE_CPSR();

	bus.armNextPC = bus.reg[15].I;
	bus.reg[15].I += 4;

	// reset internal state
	holdState = false;
	holdType = 0;

	biosProtected[0] = 0x00;
	biosProtected[1] = 0xf0;
	biosProtected[2] = 0x29;
	biosProtected[3] = 0xe1;

	graphics.lcdTicks = (useBios && !skipBios) ? 1008 : 208;
	timer0On = false;
	timer0Ticks = 0;
	timer0Reload = 0;
	timer0ClockReload  = 0;
	timer1On = false;
	timer1Ticks = 0;
	timer1Reload = 0;
	timer1ClockReload  = 0;
	timer2On = false;
	timer2Ticks = 0;
	timer2Reload = 0;
	timer2ClockReload  = 0;
	timer3On = false;
	timer3Ticks = 0;
	timer3Reload = 0;
	timer3ClockReload  = 0;
	dma0Source = 0;
	dma0Dest = 0;
	dma1Source = 0;
	dma1Dest = 0;
	dma2Source = 0;
	dma2Dest = 0;
	dma3Source = 0;
	dma3Dest = 0;
	cpuSaveGameFunc = flashSaveDecide;
	renderLine = mode0RenderLine;
	fxOn = false;
	windowOn = false;
#ifdef USE_FRAMESKIP
	frameCount = 0;
#endif
	saveType = 0;
	graphics.layerEnable = graphics.DISPCNT & graphics.layerSettings;

	memset(line[0], -1, 240 * sizeof(u32));
	memset(line[1], -1, 240 * sizeof(u32));
	memset(line[2], -1, 240 * sizeof(u32));
	memset(line[3], -1, 240 * sizeof(u32));

	for(int i = 0; i < 256; i++) {
		map[i].address = 0;
		map[i].mask = 0;
	}

	map[0].address = bios;
	map[0].mask = 0x3FFF;
	map[2].address = workRAM;
	map[2].mask = 0x3FFFF;
	map[3].address = internalRAM;
	map[3].mask = 0x7FFF;
	map[4].address = ioMem;
	map[4].mask = 0x3FF;
	map[5].address = graphics.paletteRAM;
	map[5].mask = 0x3FF;
	map[6].address = vram;
	map[6].mask = 0x1FFFF;
	map[7].address = oam;
	map[7].mask = 0x3FF;
	map[8].address = rom;
	map[8].mask = 0x1FFFFFF;
	map[9].address = rom;
	map[9].mask = 0x1FFFFFF;
	map[10].address = rom;
	map[10].mask = 0x1FFFFFF;
	map[12].address = rom;
	map[12].mask = 0x1FFFFFF;
	map[14].address = flashSaveMemory;
	map[14].mask = 0xFFFF;

	eepromReset();
	flashReset();

	soundReset();

	CPUUpdateWindow0();
	CPUUpdateWindow1();

	// make sure registers are correctly initialized if not using BIOS
	if(!useBios)
	{
		if(cpuIsMultiBoot)
			BIOS_RegisterRamReset(0xfe);
		else
			BIOS_RegisterRamReset(0xff);
	}
	else
	{
		if(cpuIsMultiBoot)
			BIOS_RegisterRamReset(0xfe);
	}

	switch(cpuSaveType) {
		case 0: // automatic
			cpuSramEnabled = true;
			cpuFlashEnabled = true;
			cpuEEPROMEnabled = true;
			cpuEEPROMSensorEnabled = false;
			saveType = gbaSaveType = 0;
			break;
		case 1: // EEPROM
			cpuSramEnabled = false;
			cpuFlashEnabled = false;
			cpuEEPROMEnabled = true;
			cpuEEPROMSensorEnabled = false;
			saveType = gbaSaveType = 3;
			// EEPROM usage is automatically detected
			break;
		case 2: // SRAM
			cpuSramEnabled = true;
			cpuFlashEnabled = false;
			cpuEEPROMEnabled = false;
			cpuEEPROMSensorEnabled = false;
			cpuSaveGameFunc = sramDelayedWrite; // to insure we detect the write
			saveType = gbaSaveType = 1;
			break;
		case 3: // FLASH
			cpuSramEnabled = false;
			cpuFlashEnabled = true;
			cpuEEPROMEnabled = false;
			cpuEEPROMSensorEnabled = false;
			cpuSaveGameFunc = flashDelayedWrite; // to insure we detect the write
			saveType = gbaSaveType = 2;
			break;
		case 4: // EEPROM+Sensor
			cpuSramEnabled = false;
			cpuFlashEnabled = false;
			cpuEEPROMEnabled = true;
			cpuEEPROMSensorEnabled = true;
			// EEPROM usage is automatically detected
			saveType = gbaSaveType = 3;
			break;
		case 5: // NONE
			cpuSramEnabled = false;
			cpuFlashEnabled = false;
			cpuEEPROMEnabled = false;
			cpuEEPROMSensorEnabled = false;
			// no save at all
			saveType = gbaSaveType = 5;
			break;
	}

	ARM_PREFETCH;

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

#ifdef USE_FRAMESKIP
	lastTime = systemGetClock();
#endif

#ifdef USE_SWITICKS
	SWITicks = 0;
#endif
}

void CPUInterrupt()
{
	uint32_t PC = bus.reg[15].I;
	bool savedState = armState;
	CPUSwitchMode(0x12, true, false);
	bus.reg[14].I = PC;
	if(!savedState)
		bus.reg[14].I += 2;
	bus.reg[15].I = 0x18;
	armState = true;
	armIrqEnable = false;

	bus.armNextPC = bus.reg[15].I;
	bus.reg[15].I += 4;
	ARM_PREFETCH;

	//  if(!holdState)
	biosProtected[0] = 0x02;
	biosProtected[1] = 0xc0;
	biosProtected[2] = 0x5e;
	biosProtected[3] = 0xe5;
}

#ifdef USE_FRAMESKIP
void CPULoop(int ticks)
#else
void CPULoop()
#endif
{
	bus.busPrefetchCount = 0;
#ifdef USE_FRAMESKIP
	ticks = 250000;
#else
	int ticks = 250000;
#endif
	int timerOverflow = 0;
	// variable used by the CPU core
	cpuTotalTicks = 0;

	cpuNextEvent = CPUUpdateTicks();
	if(cpuNextEvent > ticks)
		cpuNextEvent = ticks;


	do {
		if(!holdState) {
			if(armState) {
				if (!armExecute())
					return;
			} else {
				if (!thumbExecute())
					return;
			}
			clockTicks = 0;
		} else
			clockTicks = CPUUpdateTicks();

		cpuTotalTicks += clockTicks;


		if(cpuTotalTicks >= cpuNextEvent) {
			int remainingTicks = cpuTotalTicks - cpuNextEvent;

#ifdef USE_SWITICKS
			if (SWITicks)
			{
				SWITicks-=clockTicks;
				if (SWITicks<0)
					SWITicks = 0;
			}
#endif

			clockTicks = cpuNextEvent;
			cpuTotalTicks = 0;

updateLoop:

			if (IRQTicks)
			{
				IRQTicks -= clockTicks;
				if (IRQTicks<0)
					IRQTicks = 0;
			}

			graphics.lcdTicks -= clockTicks;


			if(graphics.lcdTicks <= 0) {
				if(graphics.DISPSTAT & 1) { // V-BLANK
					// if in V-Blank mode, keep computing...
					if(graphics.DISPSTAT & 2) {
						graphics.lcdTicks += 1008;
						VCOUNT++;
						UPDATE_REG(0x06, VCOUNT);
						graphics.DISPSTAT &= 0xFFFD;
						UPDATE_REG(0x04, graphics.DISPSTAT);
						CPUCompareVCOUNT();
					} else {
						graphics.lcdTicks += 224;
						graphics.DISPSTAT |= 2;
						UPDATE_REG(0x04, graphics.DISPSTAT);
						if(graphics.DISPSTAT & 16) {
							IF |= 2;
							UPDATE_REG(0x202, IF);
						}
					}

					if(VCOUNT >= 228) { //Reaching last line
						graphics.DISPSTAT &= 0xFFFC;
						UPDATE_REG(0x04, graphics.DISPSTAT);
						VCOUNT = 0;
						UPDATE_REG(0x06, VCOUNT);
						CPUCompareVCOUNT();
					}
				} else {
#ifdef USE_FRAMESKIP
					int framesToSkip = systemFrameSkip;
					if(speedup)
						framesToSkip = 9; // try 6 FPS during speedup
#endif

					if(graphics.DISPSTAT & 2) {
						// if in H-Blank, leave it and move to drawing mode
						VCOUNT++;
						UPDATE_REG(0x06, VCOUNT);

						graphics.lcdTicks += 1008;
						graphics.DISPSTAT &= 0xFFFD;
						if(VCOUNT == 160)
						{
#ifdef USE_FRAMESKIP
							count++;
							systemFrame();

							if((count & 10) == 0)
								system10Frames(60);
							if(count == 60)
							{
								uint32_t time = systemGetClock();
								if(time != lastTime)
								{
									uint32_t t = 100000/(time - lastTime);
									systemShowSpeed(t);
								}
								else
									systemShowSpeed(0);
								lastTime = time;
								count = 0;
							}
#endif
							/* update joystick information */
							P1 = 0x03FF ^ (joy & 0x3FF);
#if 0
							if(cpuEEPROMSensorEnabled)
								systemUpdateMotionSensor();
#endif
							UPDATE_REG(0x130, P1);
							uint16_t P1CNT = READ16LE(((uint16_t *)&ioMem[0x132]));
							// this seems wrong, but there are cases where the game
							// can enter the stop state without requesting an IRQ from
							// the joypad.
							if((P1CNT & 0x4000) || stopState) {
								uint16_t p1 = (0x3FF ^ P1) & 0x3FF;
								if(P1CNT & 0x8000) {
									if(p1 == (P1CNT & 0x3FF)) {
										IF |= 0x1000;
										UPDATE_REG(0x202, IF);
									}
								} else {
									if(p1 & P1CNT) {
										IF |= 0x1000;
										UPDATE_REG(0x202, IF);
									}
								}
							}

#ifdef USE_CHEATS
							uint32_t ext = (joy >> 10);
							// If no (m) code is enabled, apply the cheats at each LCDline
							if((cheatsEnabled) && (mastercode==0))
								remainingTicks += cheatsCheckKeys(P1^0x3FF, ext);
#ifdef USE_FRAMESKIP
							speedup = (ext & 1) ? true : false;
#endif
#endif

							graphics.DISPSTAT |= 1;
							graphics.DISPSTAT &= 0xFFFD;
							UPDATE_REG(0x04, graphics.DISPSTAT);
							if(graphics.DISPSTAT & 0x0008) {
								IF |= 1;
								UPDATE_REG(0x202, IF);
							}
							CPUCheckDMA(1, 0x0f);
#ifdef USE_FRAMESKIP
							if(frameCount >= framesToSkip) {
#endif
								systemDrawScreen();
#ifdef USE_FRAMESKIP
								frameCount = 0;
							} else
								frameCount++;
							if(systemPauseOnFrame())
								ticks = 0;
#endif
						}

						UPDATE_REG(0x04, graphics.DISPSTAT);
						CPUCompareVCOUNT();

					} else {
#ifdef USE_FRAMESKIP
						if(frameCount >= framesToSkip)
						{
#endif
							//we only use 32bit color depth
							(*renderLine)((uint32_t *)pix + 241 * (VCOUNT+1));
#ifdef USE_FRAMESKIP
						}
#endif
						// entering H-Blank
						graphics.DISPSTAT |= 2;
						UPDATE_REG(0x04, graphics.DISPSTAT);
						graphics.lcdTicks += 224;
						CPUCheckDMA(2, 0x0f);
						if(graphics.DISPSTAT & 16) {
							IF |= 2;
							UPDATE_REG(0x202, IF);
						}
					}
				}
			}

			// we shouldn't be doing sound in stop state, but we lose synchronization
			// if sound is disabled, so in stop state, soundTick will just produce
			// mute sound
			soundTicks -= clockTicks;
			if(!soundTicks) {
				psoundTickfn();
				soundTicks += SOUND_CLOCK_TICKS;
			}

			if(!stopState) {
				if(timer0On) {
					timer0Ticks -= clockTicks;
					if(timer0Ticks <= 0) {
						timer0Ticks += (0x10000 - timer0Reload) << timer0ClockReload;
						timerOverflow |= 1;
						soundTimerOverflow(0);
						if(TM0CNT & 0x40) {
							IF |= 0x08;
							UPDATE_REG(0x202, IF);
						}
					}
					TM0D = 0xFFFF - (timer0Ticks >> timer0ClockReload);
					UPDATE_REG(0x100, TM0D);
				}

				if(timer1On) {
					if(TM1CNT & 4) {
						if(timerOverflow & 1) {
							TM1D++;
							if(TM1D == 0) {
								TM1D += timer1Reload;
								timerOverflow |= 2;
								soundTimerOverflow(1);
								if(TM1CNT & 0x40) {
									IF |= 0x10;
									UPDATE_REG(0x202, IF);
								}
							}
							UPDATE_REG(0x104, TM1D);
						}
					} else {
						timer1Ticks -= clockTicks;
						if(timer1Ticks <= 0) {
							timer1Ticks += (0x10000 - timer1Reload) << timer1ClockReload;
							timerOverflow |= 2;
							soundTimerOverflow(1);
							if(TM1CNT & 0x40) {
								IF |= 0x10;
								UPDATE_REG(0x202, IF);
							}
						}
						TM1D = 0xFFFF - (timer1Ticks >> timer1ClockReload);
						UPDATE_REG(0x104, TM1D);
					}
				}

				if(timer2On) {
					if(TM2CNT & 4) {
						if(timerOverflow & 2) {
							TM2D++;
							if(TM2D == 0) {
								TM2D += timer2Reload;
								timerOverflow |= 4;
								if(TM2CNT & 0x40) {
									IF |= 0x20;
									UPDATE_REG(0x202, IF);
								}
							}
							UPDATE_REG(0x108, TM2D);
						}
					} else {
						timer2Ticks -= clockTicks;
						if(timer2Ticks <= 0) {
							timer2Ticks += (0x10000 - timer2Reload) << timer2ClockReload;
							timerOverflow |= 4;
							if(TM2CNT & 0x40) {
								IF |= 0x20;
								UPDATE_REG(0x202, IF);
							}
						}
						TM2D = 0xFFFF - (timer2Ticks >> timer2ClockReload);
						UPDATE_REG(0x108, TM2D);
					}
				}

				if(timer3On) {
					if(TM3CNT & 4) {
						if(timerOverflow & 4) {
							TM3D++;
							if(TM3D == 0) {
								TM3D += timer3Reload;
								if(TM3CNT & 0x40) {
									IF |= 0x40;
									UPDATE_REG(0x202, IF);
								}
							}
							UPDATE_REG(0x10C, TM3D);
						}
					} else {
						timer3Ticks -= clockTicks;
						if(timer3Ticks <= 0) {
							timer3Ticks += (0x10000 - timer3Reload) << timer3ClockReload;
							if(TM3CNT & 0x40) {
								IF |= 0x40;
								UPDATE_REG(0x202, IF);
							}
						}
						TM3D = 0xFFFF - (timer3Ticks >> timer3ClockReload);
						UPDATE_REG(0x10C, TM3D);
					}
				}
			}

			timerOverflow = 0;
			ticks -= clockTicks;
			cpuNextEvent = CPUUpdateTicks();

			if(cpuDmaTicksToUpdate > 0) {
				if(cpuDmaTicksToUpdate > cpuNextEvent)
					clockTicks = cpuNextEvent;
				else
					clockTicks = cpuDmaTicksToUpdate;
				cpuDmaTicksToUpdate -= clockTicks;
				if(cpuDmaTicksToUpdate < 0)
					cpuDmaTicksToUpdate = 0;
				goto updateLoop;
			}

			if(IF && (IME & 1) && armIrqEnable) {
				int res = IF & IE;
				if(stopState)
					res &= 0x3080;
				if(res) {
					if (intState)
					{
						if (!IRQTicks)
						{
							CPUInterrupt();
							intState = false;
							holdState = false;
							stopState = false;
							holdType = 0;
						}
					}
					else
					{
						if (!holdState)
						{
							intState = true;
							IRQTicks=7;
							if (cpuNextEvent> IRQTicks)
								cpuNextEvent = IRQTicks;
						}
						else
						{
							CPUInterrupt();
							holdState = false;
							stopState = false;
							holdType = 0;
						}
					}

					// Stops the SWI Ticks emulation if an IRQ is executed
					//(to avoid problems with nested IRQ/SWI)
#ifdef USE_SWITICKS
					if (SWITicks)
						SWITicks = 0;
#endif
				}
			}

			if(remainingTicks > 0) {
				if(remainingTicks > cpuNextEvent)
					clockTicks = cpuNextEvent;
				else
					clockTicks = remainingTicks;
				remainingTicks -= clockTicks;
				if(remainingTicks < 0)
					remainingTicks = 0;
				goto updateLoop;
			}

			if (timerOnOffDelay)
			{
				// Apply Timer
				if (timerOnOffDelay & 1)
				{
					timer0ClockReload = TIMER_TICKS[timer0Value & 3];
					if(!timer0On && (timer0Value & 0x80)) {
						// reload the counter
						TM0D = timer0Reload;
						timer0Ticks = (0x10000 - TM0D) << timer0ClockReload;
						UPDATE_REG(0x100, TM0D);
					}
					timer0On = timer0Value & 0x80 ? true : false;
					TM0CNT = timer0Value & 0xC7;
					UPDATE_REG(0x102, TM0CNT);
					//    CPUUpdateTicks();
				}
				if (timerOnOffDelay & 2)
				{
					timer1ClockReload = TIMER_TICKS[timer1Value & 3];
					if(!timer1On && (timer1Value & 0x80)) {
						// reload the counter
						TM1D = timer1Reload;
						timer1Ticks = (0x10000 - TM1D) << timer1ClockReload;
						UPDATE_REG(0x104, TM1D);
					}
					timer1On = timer1Value & 0x80 ? true : false;
					TM1CNT = timer1Value & 0xC7;
					UPDATE_REG(0x106, TM1CNT);
				}
				if (timerOnOffDelay & 4)
				{
					timer2ClockReload = TIMER_TICKS[timer2Value & 3];
					if(!timer2On && (timer2Value & 0x80)) {
						// reload the counter
						TM2D = timer2Reload;
						timer2Ticks = (0x10000 - TM2D) << timer2ClockReload;
						UPDATE_REG(0x108, TM2D);
					}
					timer2On = timer2Value & 0x80 ? true : false;
					TM2CNT = timer2Value & 0xC7;
					UPDATE_REG(0x10A, TM2CNT);
				}
				if (timerOnOffDelay & 8)
				{
					timer3ClockReload = TIMER_TICKS[timer3Value & 3];
					if(!timer3On && (timer3Value & 0x80)) {
						// reload the counter
						TM3D = timer3Reload;
						timer3Ticks = (0x10000 - TM3D) << timer3ClockReload;
						UPDATE_REG(0x10C, TM3D);
					}
					timer3On = timer3Value & 0x80 ? true : false;
					TM3CNT = timer3Value & 0xC7;
					UPDATE_REG(0x10E, TM3CNT);
				}
				cpuNextEvent = CPUUpdateTicks();
				timerOnOffDelay = 0;
				// End of Apply Timer
			}

			if(cpuNextEvent > ticks)
				cpuNextEvent = ticks;

			if(ticks <= 0)
				break;

		}
	}while(1);
}
