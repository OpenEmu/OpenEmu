#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include "../System.h"
#include "../NLS.h"
#include "gb.h"
#include "gbCheats.h"
#include "gbGlobals.h"
#include "gbMemory.h"
#include "gbSGB.h"
#include "gbSound.h"
#include "../Util.h"

#ifdef __GNUC__
#define _stricmp strcasecmp
#endif

extern u8 *pix;
extern bool speedup;

bool gbUpdateSizes();
bool inBios = false;

// debugging
bool memorydebug = false;
char gbBuffer[2048];

extern u16 gbLineMix[160];

// mappers
void (*mapper)(u16,u8) = NULL;
void (*mapperRAM)(u16,u8) = NULL;
u8 (*mapperReadRAM)(u16) = NULL;
void (*mapperUpdateClock)() = NULL;

// registers
gbRegister PC;
gbRegister SP;
gbRegister AF;
gbRegister BC;
gbRegister DE;
gbRegister HL;
u16 IFF = 0;
// 0xff04
u8 register_DIV   = 0;
// 0xff05
u8 register_TIMA  = 0;
// 0xff06
u8 register_TMA   = 0;
// 0xff07
u8 register_TAC   = 0;
// 0xff0f
u8 register_IF    = 0;
// 0xff40
u8 register_LCDC  = 0;
// 0xff41
u8 register_STAT  = 0;
// 0xff42
u8 register_SCY   = 0;
// 0xff43
u8 register_SCX   = 0;
// 0xff44
u8 register_LY    = 0;
// 0xff45
u8 register_LYC   = 0;
// 0xff46
u8 register_DMA   = 0;
// 0xff4a
u8 register_WY    = 0;
// 0xff4b
u8 register_WX    = 0;
// 0xff4f
u8 register_VBK   = 0;
// 0xff51
u8 register_HDMA1 = 0;
// 0xff52
u8 register_HDMA2 = 0;
// 0xff53
u8 register_HDMA3 = 0;
// 0xff54
u8 register_HDMA4 = 0;
// 0xff55
u8 register_HDMA5 = 0;
// 0xff70
u8 register_SVBK  = 0;
// 0xffff
u8 register_IE    = 0;

// ticks definition
int GBDIV_CLOCK_TICKS          = 64;
int GBLCD_MODE_0_CLOCK_TICKS   = 51;
int GBLCD_MODE_1_CLOCK_TICKS   = 1140;
int GBLCD_MODE_2_CLOCK_TICKS   = 20;
int GBLCD_MODE_3_CLOCK_TICKS   = 43;
int GBLY_INCREMENT_CLOCK_TICKS = 114;
int GBTIMER_MODE_0_CLOCK_TICKS = 256;
int GBTIMER_MODE_1_CLOCK_TICKS = 4;
int GBTIMER_MODE_2_CLOCK_TICKS = 16;
int GBTIMER_MODE_3_CLOCK_TICKS = 64;
int GBSERIAL_CLOCK_TICKS       = 128;
int GBSYNCHRONIZE_CLOCK_TICKS  = 52920;

// state variables

// general
int clockTicks = 0;
bool gbSystemMessage = false;
int gbGBCColorType = 0;
int gbHardware = 0;
int gbRomType = 0;
int gbRemainingClockTicks = 0;
int gbOldClockTicks = 0;
int gbIntBreak = 0;
int gbInterruptLaunched = 0;
u8 gbCheatingDevice = 0; // 1 = GS, 2 = GG
// breakpoint
bool breakpoint = false;
// interrupt
int gbInt48Signal = 0;
int gbInterruptWait = 0;
// serial
int gbSerialOn = 0;
int gbSerialTicks = 0;
int gbSerialBits = 0;
// timer
bool gbTimerOn = false;
int gbTimerTicks = GBTIMER_MODE_0_CLOCK_TICKS;
int gbTimerClockTicks = GBTIMER_MODE_0_CLOCK_TICKS;
int gbTimerMode = 0;
bool gbIncreased = false;
// The internal timer is always active, and it is
// not reset by writing to register_TIMA/TMA, but by
// writing to register_DIV...
int gbInternalTimer = 0x55;
const u8 gbTimerMask [4] = {0xff, 0x3, 0xf, 0x3f};
const u8 gbTimerBug [8] = {0x80, 0x80, 0x02, 0x02, 0x0, 0xff, 0x0, 0xff};
bool gbTimerModeChange = false;
bool gbTimerOnChange = false;
// lcd
bool gbScreenOn = true;
int gbLcdMode = 2;
int gbLcdModeDelayed = 2;
int gbLcdTicks = GBLCD_MODE_2_CLOCK_TICKS-1;
int gbLcdTicksDelayed = GBLCD_MODE_2_CLOCK_TICKS;
int gbLcdLYIncrementTicks = 114;
int gbLcdLYIncrementTicksDelayed = 115;
int gbScreenTicks = 0;
u8 gbSCYLine[300];
u8 gbSCXLine[300];
u8 gbBgpLine[300];
u8 gbObp0Line [300];
u8 gbObp1Line [300];
u8 gbSpritesTicks [300];
u8 oldRegister_WY;
bool gbLYChangeHappened = false;
bool gbLCDChangeHappened = false;
int gbLine99Ticks = 1;
int gbRegisterLYLCDCOffOn = 0;
int inUseRegister_WY = 0;

// Used to keep track of the line that ellapse
// when screen is off
int gbWhiteScreen = 0;
bool gbBlackScreen = false;
int register_LCDCBusy = 0;

// div
int gbDivTicks = GBDIV_CLOCK_TICKS;
// cgb
int gbVramBank = 0;
int gbWramBank = 1;
//sgb
bool gbSgbResetFlag = false;
// gbHdmaDestination is 0x99d0 on startup (tested on HW)
// but I'm not sure what gbHdmaSource is...
int gbHdmaSource = 0x99d0;
int gbHdmaDestination = 0x99d0;
int gbHdmaBytes = 0x0000;
int gbHdmaOn = 0;
int gbSpeed = 0;
// frame counting
int gbFrameCount = 0;
int gbFrameSkip = 0;
int gbFrameSkipCount = 0;
// timing
u32 gbLastTime = 0;
u32 gbElapsedTime = 0;
u32 gbTimeNow = 0;
int gbSynchronizeTicks = GBSYNCHRONIZE_CLOCK_TICKS;
// emulator features
int gbBattery = 0;
bool gbBatteryError = false;
int gbCaptureNumber = 0;
bool gbCapture = false;
bool gbCapturePrevious = false;
int gbJoymask[4] = { 0, 0, 0, 0 };

u8 gbRamFill = 0xff;

int gbRomSizes[] = { 0x00008000, // 32K
                     0x00010000, // 64K
                     0x00020000, // 128K
                     0x00040000, // 256K
                     0x00080000, // 512K
                     0x00100000, // 1024K
                     0x00200000, // 2048K
                     0x00400000, // 4096K
                     0x00800000  // 8192K
};
int gbRomSizesMasks[] = { 0x00007fff,
                          0x0000ffff,
                          0x0001ffff,
                          0x0003ffff,
                          0x0007ffff,
                          0x000fffff,
                          0x001fffff,
                          0x003fffff,
                          0x007fffff
};

int gbRamSizes[6] = { 0x00000000, // 0K
                      0x00002000, // 2K  // Changed to 2000 to avoid problems with gbMemoryMap...
                      0x00002000, // 8K
                      0x00008000, // 32K
                      0x00020000, // 128K
                      0x00010000  // 64K
};

int gbRamSizesMasks[6] = { 0x00000000,
                           0x000007ff,
                           0x00001fff,
                           0x00007fff,
                           0x0001ffff,
                           0x0000ffff
};

int gbCycles[] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,  // 0
    1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,  // 1
    2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,  // 2
    2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1,  // 3
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 4
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 5
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 6
    2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,  // 7
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 8
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 9
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // a
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // b
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 2, 3, 6, 2, 4,  // c
    2, 3, 3, 1, 3, 4, 2, 4, 2, 4, 3, 1, 3, 1, 2, 4,  // d
    3, 3, 2, 1, 1, 4, 2, 4, 4, 1, 4, 1, 1, 1, 2, 4,  // e
    3, 3, 2, 1, 1, 4, 2, 4, 3, 2, 4, 1, 0, 1, 2, 4   // f
};

int gbCyclesCB[] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 0
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 1
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 2
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 3
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 4
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 5
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 6
    2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 7
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 8
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 9
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // a
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // b
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // c
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // d
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // e
    2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2   // f
};

u16 DAATable[] = {
  0x0080,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,
  0x0800,0x0900,0x1020,0x1120,0x1220,0x1320,0x1420,0x1520,
  0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,
  0x1800,0x1900,0x2020,0x2120,0x2220,0x2320,0x2420,0x2520,
  0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,
  0x2800,0x2900,0x3020,0x3120,0x3220,0x3320,0x3420,0x3520,
  0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,0x3600,0x3700,
  0x3800,0x3900,0x4020,0x4120,0x4220,0x4320,0x4420,0x4520,
  0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,0x4600,0x4700,
  0x4800,0x4900,0x5020,0x5120,0x5220,0x5320,0x5420,0x5520,
  0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,0x5600,0x5700,
  0x5800,0x5900,0x6020,0x6120,0x6220,0x6320,0x6420,0x6520,
  0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,0x6600,0x6700,
  0x6800,0x6900,0x7020,0x7120,0x7220,0x7320,0x7420,0x7520,
  0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,0x7600,0x7700,
  0x7800,0x7900,0x8020,0x8120,0x8220,0x8320,0x8420,0x8520,
  0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,0x8600,0x8700,
  0x8800,0x8900,0x9020,0x9120,0x9220,0x9320,0x9420,0x9520,
  0x9000,0x9100,0x9200,0x9300,0x9400,0x9500,0x9600,0x9700,
  0x9800,0x9900,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
  0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,0x0610,0x0710,
  0x0810,0x0910,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
  0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,0x1610,0x1710,
  0x1810,0x1910,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
  0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,0x2610,0x2710,
  0x2810,0x2910,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
  0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,0x3610,0x3710,
  0x3810,0x3910,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
  0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,0x4610,0x4710,
  0x4810,0x4910,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
  0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,0x5610,0x5710,
  0x5810,0x5910,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
  0x6010,0x6110,0x6210,0x6310,0x6410,0x6510,0x6610,0x6710,
  0x6810,0x6910,0x7030,0x7130,0x7230,0x7330,0x7430,0x7530,
  0x7010,0x7110,0x7210,0x7310,0x7410,0x7510,0x7610,0x7710,
  0x7810,0x7910,0x8030,0x8130,0x8230,0x8330,0x8430,0x8530,
  0x8010,0x8110,0x8210,0x8310,0x8410,0x8510,0x8610,0x8710,
  0x8810,0x8910,0x9030,0x9130,0x9230,0x9330,0x9430,0x9530,
  0x9010,0x9110,0x9210,0x9310,0x9410,0x9510,0x9610,0x9710,
  0x9810,0x9910,0xA030,0xA130,0xA230,0xA330,0xA430,0xA530,
  0xA010,0xA110,0xA210,0xA310,0xA410,0xA510,0xA610,0xA710,
  0xA810,0xA910,0xB030,0xB130,0xB230,0xB330,0xB430,0xB530,
  0xB010,0xB110,0xB210,0xB310,0xB410,0xB510,0xB610,0xB710,
  0xB810,0xB910,0xC030,0xC130,0xC230,0xC330,0xC430,0xC530,
  0xC010,0xC110,0xC210,0xC310,0xC410,0xC510,0xC610,0xC710,
  0xC810,0xC910,0xD030,0xD130,0xD230,0xD330,0xD430,0xD530,
  0xD010,0xD110,0xD210,0xD310,0xD410,0xD510,0xD610,0xD710,
  0xD810,0xD910,0xE030,0xE130,0xE230,0xE330,0xE430,0xE530,
  0xE010,0xE110,0xE210,0xE310,0xE410,0xE510,0xE610,0xE710,
  0xE810,0xE910,0xF030,0xF130,0xF230,0xF330,0xF430,0xF530,
  0xF010,0xF110,0xF210,0xF310,0xF410,0xF510,0xF610,0xF710,
  0xF810,0xF910,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
  0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,0x0610,0x0710,
  0x0810,0x0910,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
  0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,0x1610,0x1710,
  0x1810,0x1910,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
  0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,0x2610,0x2710,
  0x2810,0x2910,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
  0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,0x3610,0x3710,
  0x3810,0x3910,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
  0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,0x4610,0x4710,
  0x4810,0x4910,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
  0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,0x5610,0x5710,
  0x5810,0x5910,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
  0x0600,0x0700,0x0800,0x0900,0x0A00,0x0B00,0x0C00,0x0D00,
  0x0E00,0x0F00,0x1020,0x1120,0x1220,0x1320,0x1420,0x1520,
  0x1600,0x1700,0x1800,0x1900,0x1A00,0x1B00,0x1C00,0x1D00,
  0x1E00,0x1F00,0x2020,0x2120,0x2220,0x2320,0x2420,0x2520,
  0x2600,0x2700,0x2800,0x2900,0x2A00,0x2B00,0x2C00,0x2D00,
  0x2E00,0x2F00,0x3020,0x3120,0x3220,0x3320,0x3420,0x3520,
  0x3600,0x3700,0x3800,0x3900,0x3A00,0x3B00,0x3C00,0x3D00,
  0x3E00,0x3F00,0x4020,0x4120,0x4220,0x4320,0x4420,0x4520,
  0x4600,0x4700,0x4800,0x4900,0x4A00,0x4B00,0x4C00,0x4D00,
  0x4E00,0x4F00,0x5020,0x5120,0x5220,0x5320,0x5420,0x5520,
  0x5600,0x5700,0x5800,0x5900,0x5A00,0x5B00,0x5C00,0x5D00,
  0x5E00,0x5F00,0x6020,0x6120,0x6220,0x6320,0x6420,0x6520,
  0x6600,0x6700,0x6800,0x6900,0x6A00,0x6B00,0x6C00,0x6D00,
  0x6E00,0x6F00,0x7020,0x7120,0x7220,0x7320,0x7420,0x7520,
  0x7600,0x7700,0x7800,0x7900,0x7A00,0x7B00,0x7C00,0x7D00,
  0x7E00,0x7F00,0x8020,0x8120,0x8220,0x8320,0x8420,0x8520,
  0x8600,0x8700,0x8800,0x8900,0x8A00,0x8B00,0x8C00,0x8D00,
  0x8E00,0x8F00,0x9020,0x9120,0x9220,0x9320,0x9420,0x9520,
  0x9600,0x9700,0x9800,0x9900,0x9A00,0x9B00,0x9C00,0x9D00,
  0x9E00,0x9F00,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
  0x0610,0x0710,0x0810,0x0910,0x0A10,0x0B10,0x0C10,0x0D10,
  0x0E10,0x0F10,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
  0x1610,0x1710,0x1810,0x1910,0x1A10,0x1B10,0x1C10,0x1D10,
  0x1E10,0x1F10,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
  0x2610,0x2710,0x2810,0x2910,0x2A10,0x2B10,0x2C10,0x2D10,
  0x2E10,0x2F10,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
  0x3610,0x3710,0x3810,0x3910,0x3A10,0x3B10,0x3C10,0x3D10,
  0x3E10,0x3F10,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
  0x4610,0x4710,0x4810,0x4910,0x4A10,0x4B10,0x4C10,0x4D10,
  0x4E10,0x4F10,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
  0x5610,0x5710,0x5810,0x5910,0x5A10,0x5B10,0x5C10,0x5D10,
  0x5E10,0x5F10,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
  0x6610,0x6710,0x6810,0x6910,0x6A10,0x6B10,0x6C10,0x6D10,
  0x6E10,0x6F10,0x7030,0x7130,0x7230,0x7330,0x7430,0x7530,
  0x7610,0x7710,0x7810,0x7910,0x7A10,0x7B10,0x7C10,0x7D10,
  0x7E10,0x7F10,0x8030,0x8130,0x8230,0x8330,0x8430,0x8530,
  0x8610,0x8710,0x8810,0x8910,0x8A10,0x8B10,0x8C10,0x8D10,
  0x8E10,0x8F10,0x9030,0x9130,0x9230,0x9330,0x9430,0x9530,
  0x9610,0x9710,0x9810,0x9910,0x9A10,0x9B10,0x9C10,0x9D10,
  0x9E10,0x9F10,0xA030,0xA130,0xA230,0xA330,0xA430,0xA530,
  0xA610,0xA710,0xA810,0xA910,0xAA10,0xAB10,0xAC10,0xAD10,
  0xAE10,0xAF10,0xB030,0xB130,0xB230,0xB330,0xB430,0xB530,
  0xB610,0xB710,0xB810,0xB910,0xBA10,0xBB10,0xBC10,0xBD10,
  0xBE10,0xBF10,0xC030,0xC130,0xC230,0xC330,0xC430,0xC530,
  0xC610,0xC710,0xC810,0xC910,0xCA10,0xCB10,0xCC10,0xCD10,
  0xCE10,0xCF10,0xD030,0xD130,0xD230,0xD330,0xD430,0xD530,
  0xD610,0xD710,0xD810,0xD910,0xDA10,0xDB10,0xDC10,0xDD10,
  0xDE10,0xDF10,0xE030,0xE130,0xE230,0xE330,0xE430,0xE530,
  0xE610,0xE710,0xE810,0xE910,0xEA10,0xEB10,0xEC10,0xED10,
  0xEE10,0xEF10,0xF030,0xF130,0xF230,0xF330,0xF430,0xF530,
  0xF610,0xF710,0xF810,0xF910,0xFA10,0xFB10,0xFC10,0xFD10,
  0xFE10,0xFF10,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
  0x0610,0x0710,0x0810,0x0910,0x0A10,0x0B10,0x0C10,0x0D10,
  0x0E10,0x0F10,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
  0x1610,0x1710,0x1810,0x1910,0x1A10,0x1B10,0x1C10,0x1D10,
  0x1E10,0x1F10,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
  0x2610,0x2710,0x2810,0x2910,0x2A10,0x2B10,0x2C10,0x2D10,
  0x2E10,0x2F10,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
  0x3610,0x3710,0x3810,0x3910,0x3A10,0x3B10,0x3C10,0x3D10,
  0x3E10,0x3F10,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
  0x4610,0x4710,0x4810,0x4910,0x4A10,0x4B10,0x4C10,0x4D10,
  0x4E10,0x4F10,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
  0x5610,0x5710,0x5810,0x5910,0x5A10,0x5B10,0x5C10,0x5D10,
  0x5E10,0x5F10,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
  0x00C0,0x0140,0x0240,0x0340,0x0440,0x0540,0x0640,0x0740,
  0x0840,0x0940,0x0440,0x0540,0x0640,0x0740,0x0840,0x0940,
  0x1040,0x1140,0x1240,0x1340,0x1440,0x1540,0x1640,0x1740,
  0x1840,0x1940,0x1440,0x1540,0x1640,0x1740,0x1840,0x1940,
  0x2040,0x2140,0x2240,0x2340,0x2440,0x2540,0x2640,0x2740,
  0x2840,0x2940,0x2440,0x2540,0x2640,0x2740,0x2840,0x2940,
  0x3040,0x3140,0x3240,0x3340,0x3440,0x3540,0x3640,0x3740,
  0x3840,0x3940,0x3440,0x3540,0x3640,0x3740,0x3840,0x3940,
  0x4040,0x4140,0x4240,0x4340,0x4440,0x4540,0x4640,0x4740,
  0x4840,0x4940,0x4440,0x4540,0x4640,0x4740,0x4840,0x4940,
  0x5040,0x5140,0x5240,0x5340,0x5440,0x5540,0x5640,0x5740,
  0x5840,0x5940,0x5440,0x5540,0x5640,0x5740,0x5840,0x5940,
  0x6040,0x6140,0x6240,0x6340,0x6440,0x6540,0x6640,0x6740,
  0x6840,0x6940,0x6440,0x6540,0x6640,0x6740,0x6840,0x6940,
  0x7040,0x7140,0x7240,0x7340,0x7440,0x7540,0x7640,0x7740,
  0x7840,0x7940,0x7440,0x7540,0x7640,0x7740,0x7840,0x7940,
  0x8040,0x8140,0x8240,0x8340,0x8440,0x8540,0x8640,0x8740,
  0x8840,0x8940,0x8440,0x8540,0x8640,0x8740,0x8840,0x8940,
  0x9040,0x9140,0x9240,0x9340,0x9440,0x9540,0x9640,0x9740,
  0x9840,0x9940,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
  0x4050,0x4150,0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,
  0x4850,0x4950,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
  0x5050,0x5150,0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,
  0x5850,0x5950,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
  0x6050,0x6150,0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,
  0x6850,0x6950,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
  0x7050,0x7150,0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,
  0x7850,0x7950,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
  0x8050,0x8150,0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,
  0x8850,0x8950,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
  0x9050,0x9150,0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,
  0x9850,0x9950,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
  0xA050,0xA150,0xA250,0xA350,0xA450,0xA550,0xA650,0xA750,
  0xA850,0xA950,0xA450,0xA550,0xA650,0xA750,0xA850,0xA950,
  0xB050,0xB150,0xB250,0xB350,0xB450,0xB550,0xB650,0xB750,
  0xB850,0xB950,0xB450,0xB550,0xB650,0xB750,0xB850,0xB950,
  0xC050,0xC150,0xC250,0xC350,0xC450,0xC550,0xC650,0xC750,
  0xC850,0xC950,0xC450,0xC550,0xC650,0xC750,0xC850,0xC950,
  0xD050,0xD150,0xD250,0xD350,0xD450,0xD550,0xD650,0xD750,
  0xD850,0xD950,0xD450,0xD550,0xD650,0xD750,0xD850,0xD950,
  0xE050,0xE150,0xE250,0xE350,0xE450,0xE550,0xE650,0xE750,
  0xE850,0xE950,0xE450,0xE550,0xE650,0xE750,0xE850,0xE950,
  0xF050,0xF150,0xF250,0xF350,0xF450,0xF550,0xF650,0xF750,
  0xF850,0xF950,0xF450,0xF550,0xF650,0xF750,0xF850,0xF950,
  0x00D0,0x0150,0x0250,0x0350,0x0450,0x0550,0x0650,0x0750,
  0x0850,0x0950,0x0450,0x0550,0x0650,0x0750,0x0850,0x0950,
  0x1050,0x1150,0x1250,0x1350,0x1450,0x1550,0x1650,0x1750,
  0x1850,0x1950,0x1450,0x1550,0x1650,0x1750,0x1850,0x1950,
  0x2050,0x2150,0x2250,0x2350,0x2450,0x2550,0x2650,0x2750,
  0x2850,0x2950,0x2450,0x2550,0x2650,0x2750,0x2850,0x2950,
  0x3050,0x3150,0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,
  0x3850,0x3950,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
  0x4050,0x4150,0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,
  0x4850,0x4950,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
  0x5050,0x5150,0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,
  0x5850,0x5950,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
  0x6050,0x6150,0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,
  0x6850,0x6950,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
  0x7050,0x7150,0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,
  0x7850,0x7950,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
  0x8050,0x8150,0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,
  0x8850,0x8950,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
  0x9050,0x9150,0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,
  0x9850,0x9950,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
  0xFA60,0xFB60,0xFC60,0xFD60,0xFE60,0xFF60,0x00C0,0x0140,
  0x0240,0x0340,0x0440,0x0540,0x0640,0x0740,0x0840,0x0940,
  0x0A60,0x0B60,0x0C60,0x0D60,0x0E60,0x0F60,0x1040,0x1140,
  0x1240,0x1340,0x1440,0x1540,0x1640,0x1740,0x1840,0x1940,
  0x1A60,0x1B60,0x1C60,0x1D60,0x1E60,0x1F60,0x2040,0x2140,
  0x2240,0x2340,0x2440,0x2540,0x2640,0x2740,0x2840,0x2940,
  0x2A60,0x2B60,0x2C60,0x2D60,0x2E60,0x2F60,0x3040,0x3140,
  0x3240,0x3340,0x3440,0x3540,0x3640,0x3740,0x3840,0x3940,
  0x3A60,0x3B60,0x3C60,0x3D60,0x3E60,0x3F60,0x4040,0x4140,
  0x4240,0x4340,0x4440,0x4540,0x4640,0x4740,0x4840,0x4940,
  0x4A60,0x4B60,0x4C60,0x4D60,0x4E60,0x4F60,0x5040,0x5140,
  0x5240,0x5340,0x5440,0x5540,0x5640,0x5740,0x5840,0x5940,
  0x5A60,0x5B60,0x5C60,0x5D60,0x5E60,0x5F60,0x6040,0x6140,
  0x6240,0x6340,0x6440,0x6540,0x6640,0x6740,0x6840,0x6940,
  0x6A60,0x6B60,0x6C60,0x6D60,0x6E60,0x6F60,0x7040,0x7140,
  0x7240,0x7340,0x7440,0x7540,0x7640,0x7740,0x7840,0x7940,
  0x7A60,0x7B60,0x7C60,0x7D60,0x7E60,0x7F60,0x8040,0x8140,
  0x8240,0x8340,0x8440,0x8540,0x8640,0x8740,0x8840,0x8940,
  0x8A60,0x8B60,0x8C60,0x8D60,0x8E60,0x8F60,0x9040,0x9140,
  0x9240,0x9340,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
  0x3A70,0x3B70,0x3C70,0x3D70,0x3E70,0x3F70,0x4050,0x4150,
  0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
  0x4A70,0x4B70,0x4C70,0x4D70,0x4E70,0x4F70,0x5050,0x5150,
  0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
  0x5A70,0x5B70,0x5C70,0x5D70,0x5E70,0x5F70,0x6050,0x6150,
  0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
  0x6A70,0x6B70,0x6C70,0x6D70,0x6E70,0x6F70,0x7050,0x7150,
  0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
  0x7A70,0x7B70,0x7C70,0x7D70,0x7E70,0x7F70,0x8050,0x8150,
  0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
  0x8A70,0x8B70,0x8C70,0x8D70,0x8E70,0x8F70,0x9050,0x9150,
  0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
  0x9A70,0x9B70,0x9C70,0x9D70,0x9E70,0x9F70,0xA050,0xA150,
  0xA250,0xA350,0xA450,0xA550,0xA650,0xA750,0xA850,0xA950,
  0xAA70,0xAB70,0xAC70,0xAD70,0xAE70,0xAF70,0xB050,0xB150,
  0xB250,0xB350,0xB450,0xB550,0xB650,0xB750,0xB850,0xB950,
  0xBA70,0xBB70,0xBC70,0xBD70,0xBE70,0xBF70,0xC050,0xC150,
  0xC250,0xC350,0xC450,0xC550,0xC650,0xC750,0xC850,0xC950,
  0xCA70,0xCB70,0xCC70,0xCD70,0xCE70,0xCF70,0xD050,0xD150,
  0xD250,0xD350,0xD450,0xD550,0xD650,0xD750,0xD850,0xD950,
  0xDA70,0xDB70,0xDC70,0xDD70,0xDE70,0xDF70,0xE050,0xE150,
  0xE250,0xE350,0xE450,0xE550,0xE650,0xE750,0xE850,0xE950,
  0xEA70,0xEB70,0xEC70,0xED70,0xEE70,0xEF70,0xF050,0xF150,
  0xF250,0xF350,0xF450,0xF550,0xF650,0xF750,0xF850,0xF950,
  0xFA70,0xFB70,0xFC70,0xFD70,0xFE70,0xFF70,0x00D0,0x0150,
  0x0250,0x0350,0x0450,0x0550,0x0650,0x0750,0x0850,0x0950,
  0x0A70,0x0B70,0x0C70,0x0D70,0x0E70,0x0F70,0x1050,0x1150,
  0x1250,0x1350,0x1450,0x1550,0x1650,0x1750,0x1850,0x1950,
  0x1A70,0x1B70,0x1C70,0x1D70,0x1E70,0x1F70,0x2050,0x2150,
  0x2250,0x2350,0x2450,0x2550,0x2650,0x2750,0x2850,0x2950,
  0x2A70,0x2B70,0x2C70,0x2D70,0x2E70,0x2F70,0x3050,0x3150,
  0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
  0x3A70,0x3B70,0x3C70,0x3D70,0x3E70,0x3F70,0x4050,0x4150,
  0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
  0x4A70,0x4B70,0x4C70,0x4D70,0x4E70,0x4F70,0x5050,0x5150,
  0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
  0x5A70,0x5B70,0x5C70,0x5D70,0x5E70,0x5F70,0x6050,0x6150,
  0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
  0x6A70,0x6B70,0x6C70,0x6D70,0x6E70,0x6F70,0x7050,0x7150,
  0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
  0x7A70,0x7B70,0x7C70,0x7D70,0x7E70,0x7F70,0x8050,0x8150,
  0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
  0x8A70,0x8B70,0x8C70,0x8D70,0x8E70,0x8F70,0x9050,0x9150,
  0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
};

u8 ZeroTable[256] = {
  0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0
};

#define GBSAVE_GAME_VERSION_1 1
#define GBSAVE_GAME_VERSION_2 2
#define GBSAVE_GAME_VERSION_3 3
#define GBSAVE_GAME_VERSION_4 4
#define GBSAVE_GAME_VERSION_5 5
#define GBSAVE_GAME_VERSION_6 6
#define GBSAVE_GAME_VERSION_7 7
#define GBSAVE_GAME_VERSION_8 8
#define GBSAVE_GAME_VERSION_9 9
#define GBSAVE_GAME_VERSION_10 10
#define GBSAVE_GAME_VERSION_11 11
#define GBSAVE_GAME_VERSION_12 12
#define GBSAVE_GAME_VERSION GBSAVE_GAME_VERSION_12

int inline gbGetValue(int min,int max,int v)
{
  return (int)(min+(float)(max-min)*(2.0*(v/31.0)-(v/31.0)*(v/31.0)));
}

void gbGenFilter()
{
  for (int r=0;r<32;r++) {
    for (int g=0;g<32;g++) {
      for (int b=0;b<32;b++) {
        int nr=gbGetValue(gbGetValue(4,14,g),
                          gbGetValue(24,29,g),r)-4;
        int ng=gbGetValue(gbGetValue(4+gbGetValue(0,5,r),
                                     14+gbGetValue(0,3,r),b),
                          gbGetValue(24+gbGetValue(0,3,r),
                                     29+gbGetValue(0,1,r),b),g)-4;
        int nb=gbGetValue(gbGetValue(4+gbGetValue(0,5,r),
                                     14+gbGetValue(0,3,r),g),
                          gbGetValue(24+gbGetValue(0,3,r),
                                     29+gbGetValue(0,1,r),g),b)-4;
        gbColorFilter[(b<<10)|(g<<5)|r]=(nb<<10)|(ng<<5)|nr;
      }
    }
  }
}

bool gbIsGameboyRom(char * file)
{
  if(strlen(file) > 4) {
    char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".gb") == 0)
        return true;
      if(_stricmp(p, ".gbc") == 0)
        return true;
      if(_stricmp(p, ".cgb") == 0)
        return true;
      if(_stricmp(p, ".sgb") == 0)
        return true;
    }
  }

  return false;
}

void gbCopyMemory(u16 d, u16 s, int count)
{
  while(count) {
    gbMemoryMap[d>>12][d & 0x0fff] = gbMemoryMap[s>>12][s & 0x0fff];
    s++;
    d++;
    count--;
  }
}

void gbDoHdma()
{

  gbCopyMemory((gbHdmaDestination & 0x1ff0) | 0x8000,
               gbHdmaSource & 0xfff0,
               0x10);

  gbHdmaDestination += 0x10;
  if (gbHdmaDestination == 0xa000)
    gbHdmaDestination = 0x8000;

  gbHdmaSource += 0x10;
  if (gbHdmaSource == 0x8000)
    gbHdmaSource = 0xa000;

  register_HDMA2 = gbHdmaSource & 0xff;
  register_HDMA1 = gbHdmaSource>>8;

  register_HDMA4 = gbHdmaDestination & 0xff;
  register_HDMA3 = gbHdmaDestination>>8;

  gbHdmaBytes -= 0x10;
  gbMemory[0xff55] = --register_HDMA5;
  if(register_HDMA5 == 0xff)
    gbHdmaOn = 0;

// We need to add the dmaClockticks for HDMA !
  if(gbSpeed)
    gbDmaTicks = 17;
  else
    gbDmaTicks = 9;

  if (IFF & 0x80)
    gbDmaTicks++;

}

// fix for Harley and Lego Racers
void gbCompareLYToLYC()
{
  if(register_LCDC & 0x80) {
    if(register_LY == register_LYC) {

      // mark that we have a match
      register_STAT |= 4;

      // check if we need an interrupt
      if (register_STAT & 0x40)
      {
        // send LCD interrupt only if no interrupt 48h signal...
        if (!gbInt48Signal)
        {
          register_IF |=2;
        }
        gbInt48Signal |= 8;
      }
    }
    else // no match
    {
      register_STAT &= 0xfb;
      gbInt48Signal &=~8;
    }
  }
}

void  gbWriteMemory(register u16 address, register u8 value)
{

  if(address < 0x8000) {
#ifndef FINAL_VERSION
    if(memorydebug && (address>0x3fff || address < 0x2000)) {
      log("Memory register write %04x=%02x PC=%04x\n",
          address,
          value,
          PC.W);
    }

#endif
    if(mapper)
      (*mapper)(address, value);
    return;

  }

  if(address < 0xa000) {
    // No access to Vram during mode 3
    // (used to emulate the gfx differences between GB & GBC-GBA/SP in Stunt Racer)
    if ((gbLcdModeDelayed !=3) ||
    // This part is used to emulate a small difference between hardwares
    // (check 8-in-1's arrow on GBA/GBC to verify it)
       ((register_LY == 0) && ((gbHardware & 0xa) && (gbScreenOn==false) &&
       (register_LCDC & 0x80)) &&
       (gbLcdLYIncrementTicksDelayed ==(GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS))))
      gbMemoryMap[address>>12][address&0x0fff] = value;
    return;
  }

  // Used for the mirroring of 0xC000 in 0xE000
  if ((address >= 0xe000) && (address < 0xfe00))
    address &= ~0x2000;

  if(address < 0xc000) {
#ifndef FINAL_VERSION
    if(memorydebug) {
      log("Memory register write %04x=%02x PC=%04x\n",
          address,
          value,
          PC.W);
    }
#endif

    // Is that a correct fix ??? (it used to be 'if (mapper)')...
    if(mapperRAM)
        (*mapperRAM)(address, value);
    return;
  }


  if(address < 0xfe00) {
    gbMemoryMap[address>>12][address & 0x0fff] = value;
    return;
  }

  // OAM not accessible during mode 2 & 3.
  if(address < 0xfea0)
  {
      if (((gbHardware & 0xa) && ((gbLcdMode | gbLcdModeDelayed) &2)) ||
          ((gbHardware & 5) && (((gbLcdModeDelayed == 2) &&
          (gbLcdTicksDelayed<=GBLCD_MODE_2_CLOCK_TICKS)) ||
          (gbLcdModeDelayed == 3))))
      return;
    else
    {
      gbMemory[address] = value;
      return;
    }
  }



  if((address > 0xfea0) && (address < 0xff00)){ // GBC allows reading/writing to that area
    gbMemory[address] = value;
    return;
  }

  switch(address & 0x00ff) {

    case 0x00: {
      gbMemory[0xff00] = ((gbMemory[0xff00] & 0xcf) |
                        (value & 0x30) | 0xc0);
      if(gbSgbMode) {
        gbSgbDoBitTransfer(value);
      }
      return;
    }

    case 0x01: {
      gbMemory[0xff01] = value;
      return;
    }

    // serial control
    case 0x02: {
      gbSerialOn = (value & 0x80);
      gbMemory[0xff02] = value;
      if(gbSerialOn) {
        gbSerialTicks = GBSERIAL_CLOCK_TICKS;
#ifdef LINK_EMULATION
        if(linkConnected) {
          if(value & 1) {
            linkSendByte(0x100|gbMemory[0xFF01]);
            Sleep(5);
          }
        }
#endif
      }

      gbSerialBits = 0;
      return;
    }

    case 0x04: {
    // DIV register resets on any write
    // (not totally perfect, but better than nothing)
      gbMemory[0xff04] = register_DIV = 0;
      gbDivTicks = GBDIV_CLOCK_TICKS;
     // Another weird timer 'bug' :
     // Writing to DIV register resets the internal timer,
     // and can also increase TIMA/trigger an interrupt
     // in some cases...
      if (gbTimerOn && !(gbInternalTimer & (gbTimerClockTicks>>1)))
      {
        gbMemory[0xff05] = ++register_TIMA;
        if(register_TIMA == 0) {
          // timer overflow!

          // reload timer modulo
          gbMemory[0xff05] = register_TIMA = register_TMA;

          // flag interrupt
          gbMemory[0xff0f] = register_IF |= 4;
        }
      }
      gbInternalTimer = 0xff;
      return;
    }
    case 0x05:
      gbMemory[0xff05] = register_TIMA = value;
      return;

    case 0x06:
      gbMemory[0xff06] = register_TMA = value;
      return;

      // TIMER control
    case 0x07: {

      gbTimerModeChange = (((value & 3) != (register_TAC&3)) && (value & register_TAC & 4)) ? true : false;
      gbTimerOnChange = (((value ^ register_TAC) & 4) == 4) ? true : false;

      gbTimerOn = (value & 4) ? true : false;

      if (gbTimerOnChange || gbTimerModeChange)
      {
        gbTimerMode = value & 3;

        switch(gbTimerMode) {
          case 0:
            gbTimerClockTicks = GBTIMER_MODE_0_CLOCK_TICKS;
            break;
          case 1:
            gbTimerClockTicks = GBTIMER_MODE_1_CLOCK_TICKS;
            break;
          case 2:
            gbTimerClockTicks = GBTIMER_MODE_2_CLOCK_TICKS;
            break;
          case 3:
            gbTimerClockTicks = GBTIMER_MODE_3_CLOCK_TICKS;
            break;
        }
      }


      // This looks weird, but this emulates a bug in which register_TIMA
      // is increased when writing to register_TAC
      // (This fixes Korodice's long-delay between menus bug).

      if (gbTimerOnChange || gbTimerModeChange)
      {
        bool temp = false;

        if ((gbTimerOn  && !gbTimerModeChange)  && (gbTimerMode & 2) &&
            !(gbInternalTimer & 0x80) && (gbInternalTimer & (gbTimerClockTicks>>1)) &&
            !(gbInternalTimer & (gbTimerClockTicks>>5)))
          temp = true;
        else if ((!gbTimerOn && !gbTimerModeChange && gbTimerOnChange ) &&  ((gbTimerTicks-1) < (gbTimerClockTicks>>1)))
          temp = true;
        else if (gbTimerOn && gbTimerModeChange && !gbTimerOnChange)
        {
          switch(gbTimerMode & 3)
          {
            case 0x00:
              temp = false;
              break;
            case 0x01:
              if (((gbInternalTimer & 0x82) == 2) && (gbTimerTicks>(clockTicks+1)))
                temp = true;
              break;
            case 0x02:
              if (((gbInternalTimer & 0x88) == 0x8) && (gbTimerTicks>(clockTicks+1)))
                temp = true;
             break;
            case 0x03:
              if (((gbInternalTimer & 0xA0) == 0x20) && (gbTimerTicks>(clockTicks+1)))
                temp = true;
              break;
          }
        }

        if (temp)
        {
          gbMemory[0xff05] = ++register_TIMA;
          if((register_TIMA & 0xff) == 0) {
            // timer overflow!

            // reload timer modulo
            gbMemory[0xff05] = register_TIMA = register_TMA;

            // flag interrupt
            gbMemory[0xff0f] = register_IF |= 4;
          }
        }
      }
      gbMemory[0xff07] = register_TAC = value;
      return;
    }

    case 0x0f: {
      gbMemory[0xff0f] = register_IF = value;
      //gbMemory[0xff0f] = register_IE |= value;
      return;
    }

    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25: {
      if (gbMemory[NR52] & 0x80) {
        SOUND_EVENT(address,value);
        return;
      }
    }

    case 0x26: {
      SOUND_EVENT(address,value);
      return;
    }

    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3a:
    case 0x3b:
    case 0x3c:
    case 0x3d:
    case 0x3e:
    case 0x3f: {
      SOUND_EVENT(address,value);
      //gbMemory[address] = value;
      return;
    }

    case 0x40: {
      int lcdChange = (register_LCDC & 0x80) ^ (value & 0x80);

      // don't draw the window if it was not enabled and not being drawn before
      if(!(register_LCDC & 0x20) && (value & 0x20) && gbWindowLine == -1 &&
         register_LY > register_WY)
        gbWindowLine = 146;
      // 007 fix : don't draw the first window's 1st line if it's enable 'too late'
      // (ie. if register_LY == register_WY when enabling it)
      // and move it to the next line
      else if (!(register_LCDC & 0x20) && (value & 0x20) && (register_LY == register_WY))
        gbWindowLine = -2;


      gbMemory[0xff40] = register_LCDC = value;


      if(lcdChange) {
        if((value & 0x80) && (!register_LCDCBusy)) {

        //  if (!gbWhiteScreen && !gbSgbMask)

          //  systemDrawScreen();



          gbRegisterLYLCDCOffOn = (register_LY + 144) % 154;

          gbLcdTicks = GBLCD_MODE_2_CLOCK_TICKS - (gbSpeed ? 2 : 1);
          gbLcdTicksDelayed = GBLCD_MODE_2_CLOCK_TICKS - (gbSpeed ? 1 : 0);
          gbLcdLYIncrementTicks = GBLY_INCREMENT_CLOCK_TICKS - (gbSpeed ? 2 : 1);
          gbLcdLYIncrementTicksDelayed = GBLY_INCREMENT_CLOCK_TICKS - (gbSpeed ? 1 : 0);
          gbLcdMode = 2;
          gbLcdModeDelayed = 2;
          gbMemory[0xff41] = register_STAT = (register_STAT & 0xfc) | 2;
          gbMemory[0xff44] = register_LY = 0x00;
          gbInt48Signal = 0;
          gbLYChangeHappened = false;
          gbLCDChangeHappened = false;
          gbWindowLine = 146;
          oldRegister_WY = 146;

          // Fix for Namco Gallery Vol.2
          // (along with updating register_LCDC at the start of 'case 0x40') :
          if(register_STAT & 0x20)
          {
            // send LCD interrupt only if no interrupt 48h signal...
            if (!gbInt48Signal)
            {
              gbMemory[0xff0f] = register_IF |= 2;
            }
            gbInt48Signal |= 4;
          }
          gbCompareLYToLYC();

        } else {

          register_LCDCBusy = clockTicks+6;

          //used to update the screen with white lines when it's off.
          //(it looks strange, but it's pretty accurate)

          gbWhiteScreen = 0;

          gbScreenTicks = ((150-register_LY)*GBLY_INCREMENT_CLOCK_TICKS +
                          (49<<(gbSpeed ? 1 : 0)));

          // disable the screen rendering
          gbScreenOn = false;
          gbLcdTicks = 0;
          gbLcdMode = 0;
          gbLcdModeDelayed = 0;
          gbMemory[0xff41] = register_STAT &= 0xfc;
          gbInt48Signal = 0;
        }
      }
      return;
    }

    // STAT
    case 0x41: {
      //register_STAT = (register_STAT & 0x87) |
      //      (value & 0x7c);
      gbMemory[0xff41] = register_STAT = (value & 0xf8) | (register_STAT & 0x07); // fix ?
      // GB bug from Devrs FAQ
      // proper fix
       gbInt48Signal &= ((register_STAT>>3) & 0xF);

       if((register_LCDC & 0x80)) {
        if ((register_STAT & 0x08) && (gbLcdMode == 0))
        {
          if (!gbInt48Signal)
          {
            gbMemory[0xff0f] = register_IF |=2;
          }
          gbInt48Signal |= 1;
        }
        if ((register_STAT & 0x10) && (gbLcdMode == 1))
        {
          if (!gbInt48Signal)
          {
            gbMemory[0xff0f] = register_IF |=2;
          }
          gbInt48Signal |= 2;
        }
        if ((register_STAT & 0x20) && (gbLcdMode == 2))
        {
          if (!gbInt48Signal)
          {
            gbMemory[0xff0f] = register_IF |=2;
          }
          gbInt48Signal |= 4;
        }
        gbCompareLYToLYC();

        gbMemory[0xff0f] = register_IF;
        gbMemory[0xff41] = register_STAT;

      }
      return;
    }

    // SCY
    case 0x42: {
      int temp = -1;

      if ((gbLcdMode == 3) || (gbLcdModeDelayed == 3))
        temp = ((GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS) -
               gbLcdLYIncrementTicks);

      if (temp >=0)
      {
        for (int i=temp<<(gbSpeed ? 1 : 2);i<300;i++)
          if (temp < 300)
            gbSCYLine[i] = value;
      }

      else
        memset(gbSCYLine, value, sizeof(gbSCYLine));

      gbMemory[0xff42] = register_SCY = value;
      return;
    }

    // SCX
    case 0x43: {
      int temp = -1;

      if (gbLcdModeDelayed == 3)
        temp = ((GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS) -
               gbLcdLYIncrementTicksDelayed);

      if (temp >=0)
      {
        for (int i=temp<<(gbSpeed ? 1 : 2);i<300;i++)
          if (temp < 300)
            gbSCXLine[i] = value;
      }

      else
        memset(gbSCXLine, value, sizeof(gbSCXLine));

      gbMemory[0xff43] = register_SCX = value;
      return;
    }

    // LY
    case 0x44: {
      // read only
      return;
    }

    // LYC
    case 0x45: {
      if (register_LYC != value)
      {
        gbMemory[0xff45] = register_LYC = value;
        if(register_LCDC & 0x80) {
          gbCompareLYToLYC();
        }
      }
      return;
    }

    // DMA!
    case 0x46: {
      int source = value * 0x0100;

      gbCopyMemory(0xfe00,
                   source,
                   0xa0);
      gbMemory[0xff46] = register_DMA = value;
      return;
    }

    // BGP
    case 0x47: {

      int temp = -1;

      gbMemory[0xff47] = value;

      if (gbLcdModeDelayed == 3)
        temp = ((GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS) -
               gbLcdLYIncrementTicksDelayed);

      if (temp >=0)
      {
        for (int i=temp<<(gbSpeed ? 1 : 2);i<300;i++)
          if (temp < 300)
            gbBgpLine[i] = value;
      }
      else
        memset(gbBgpLine,value,sizeof(gbBgpLine));

      gbBgp[0] = value & 0x03;
      gbBgp[1] = (value & 0x0c)>>2;
      gbBgp[2] = (value & 0x30)>>4;
      gbBgp[3] = (value & 0xc0)>>6;
      break;
    }

    // OBP0
    case 0x48: {
      int temp = -1;

      gbMemory[0xff48] = value;

      if (gbLcdModeDelayed == 3)
        temp = ((GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS) -
               gbLcdLYIncrementTicksDelayed);

      if (temp >=0)
      {
        for (int i=temp<<(gbSpeed ? 1 : 2);i<300;i++)
          if (temp < 300)
            gbObp0Line[i] = value;
      }
      else
        memset(gbObp0Line,value,sizeof(gbObp0Line));

      gbObp0[0] = value & 0x03;
      gbObp0[1] = (value & 0x0c)>>2;
      gbObp0[2] = (value & 0x30)>>4;
      gbObp0[3] = (value & 0xc0)>>6;
      break;
    }

    // OBP1
    case 0x49: {
      int temp = -1;

      gbMemory[0xff49] = value;

      if (gbLcdModeDelayed == 3)
        temp = ((GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS) -
               gbLcdLYIncrementTicksDelayed);

      if (temp >=0)
      {
        for (int i=temp<<(gbSpeed ? 1 : 2);i<300;i++)
          if (temp < 300)
            gbObp1Line[i] = value;
      }
      else
        memset(gbObp1Line,value,sizeof(gbObp1Line));

      gbObp1[0] = value & 0x03;
      gbObp1[1] = (value & 0x0c)>>2;
      gbObp1[2] = (value & 0x30)>>4;
      gbObp1[3] = (value & 0xc0)>>6;
      break;
    }

    // WY
    case 0x4a:
      gbMemory[0xff4a] = register_WY = value;
      if ((register_LY <= register_WY) && ((gbWindowLine < 0) || (gbWindowLine>=144)))
      {
          gbWindowLine = -1;
          oldRegister_WY = register_WY;
      }
      return;

    // WX
    case 0x4b:
      gbMemory[0xff4b] = register_WX = value;
      return;

    // KEY1
    case 0x4d: {
      if(gbCgbMode) {
        gbMemory[0xff4d] = (gbMemory[0xff4d] & 0x80) | (value & 1) | 0x7e;
        return;
      }
    }
    break;

    // VBK
    case 0x4f: {
      if(gbCgbMode) {
        value = value & 1;
        if(value == gbVramBank)
          return;

        int vramAddress = value * 0x2000;
        gbMemoryMap[0x08] = &gbVram[vramAddress];
        gbMemoryMap[0x09] = &gbVram[vramAddress + 0x1000];

        gbVramBank = value;
        register_VBK = value;
      }
      return;
    }
    break;

    // BOOTROM disable register (also gbCgbMode enabler if value & 0x10 ?)
    case 0x50 :
    {
      if (useBios && inBios && !skipBios && (value & 1))
      {
        gbMemoryMap[0x00] = &gbRom[0x0000];
        memcpy ((u8 *)(gbRom+0x100), (u8 *)(gbMemory + 0x100), 0xF00);
        inBios = false;
      }
    }

    // HDMA1
    case 0x51: {
      if(gbCgbMode) {
        if(value > 0x7f && value < 0xa0)
          value = 0;

        gbHdmaSource = (value << 8) | (gbHdmaSource & 0xf0);

        register_HDMA1 = value;
        return;
      }
    }
    break;

    // HDMA2
    case 0x52: {
      if(gbCgbMode) {
        value = value & 0xf0;

        gbHdmaSource = (gbHdmaSource & 0xff00) | (value);

        register_HDMA2 = value;
        return;
      }
    }
    break;

    // HDMA3
    case 0x53: {
      if(gbCgbMode) {
        value = value & 0x1f;
        gbHdmaDestination = (value << 8) | (gbHdmaDestination & 0xf0);
        gbHdmaDestination |= 0x8000;
        register_HDMA3 = value;
        return;
      }
    }
    break;

    // HDMA4
    case 0x54: {
      if(gbCgbMode) {
        value = value & 0xf0;
        gbHdmaDestination = (gbHdmaDestination & 0x1f00) | value;
        gbHdmaDestination |= 0x8000;
        register_HDMA4 = value;
        return;
      }
    }
    break;

    // HDMA5
    case 0x55: {

      if(gbCgbMode) {
        gbHdmaBytes = 16 + (value & 0x7f) * 16;
        if(gbHdmaOn) {
          if(value & 0x80) {
            gbMemory[0xff55] = register_HDMA5 = (value & 0x7f);
          } else {
            register_HDMA5 = 0xff;
            gbHdmaOn = 0;
          }
        } else {
          if(value & 0x80) {
            gbHdmaOn = 1;
            gbMemory[0xff55] = register_HDMA5 = value & 0x7f;
            if(gbLcdModeDelayed == 0)
              gbDoHdma();
          } else {
            // we need to take the time it takes to complete the transfer into
            // account... according to GB DEV FAQs, the setup time is the same
            // for single and double speed, but the actual transfer takes the
            // same time
            if(gbSpeed)
              gbDmaTicks = 2+16 * ((value & 0x7f) +1);
            else
              gbDmaTicks = 1+8 * ((value & 0x7f)+1);

            gbCopyMemory((gbHdmaDestination & 0x1ff0) | 0x8000,
                         gbHdmaSource & 0xfff0,
                         gbHdmaBytes);
            gbHdmaDestination += gbHdmaBytes;
            gbHdmaSource += gbHdmaBytes;

            gbMemory[0xff51] = register_HDMA1 = 0xff;// = (gbHdmaSource >> 8) & 0xff;
            gbMemory[0xff52] = register_HDMA2 = 0xff;// = gbHdmaSource & 0xf0;
            gbMemory[0xff53] = register_HDMA3 = 0xff;// = ((gbHdmaDestination - 0x8000) >> 8) & 0x1f;
            gbMemory[0xff54] = register_HDMA4 = 0xff;// = gbHdmaDestination & 0xf0;
            gbMemory[0xff55] = register_HDMA5 = 0xff;
          }
        }
        return;
      }
    }
    break;

    // BCPS
    case 0x68: {
       if(gbCgbMode) {
        int paletteIndex = (value & 0x3f) >> 1;
        int paletteHiLo   = (value & 0x01);

        gbMemory[0xff68] = value;

        gbMemory[0xff69] = (paletteHiLo ?
                           (gbPalette[paletteIndex] >> 8) :
                           (gbPalette[paletteIndex] & 0x00ff));
        return;
      }
    }
    break;

    // BCPD
    case 0x69: {
      if(gbCgbMode) {
        int v = gbMemory[0xff68];
        int paletteIndex = (v & 0x3f) >> 1;
        int paletteHiLo  = (v & 0x01);

        // No access to gbPalette during mode 3 (Color Panel Demo)
        if (((gbLcdModeDelayed != 3) && (!((gbLcdMode == 0) && (gbLcdTicks>=(GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299]-1)))) && (!gbSpeed)) ||
           (gbSpeed && ((gbLcdMode == 1) || (gbLcdMode == 2) ||
           ((gbLcdMode == 3) && (gbLcdTicks>(GBLCD_MODE_3_CLOCK_TICKS-2))) ||
           ((gbLcdMode == 0) && (gbLcdTicks<=(GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299]-2))))))
        {
          gbMemory[0xff69] = value;
          gbPalette[paletteIndex] = (paletteHiLo ?
                                   ((value << 8) | (gbPalette[paletteIndex] & 0xff)) :
                                    ((gbPalette[paletteIndex] & 0xff00) | (value))) & 0x7fff;
        }


        if(gbMemory[0xff68] & 0x80) {
          int index = ((gbMemory[0xff68] & 0x3f) + 1) & 0x3f;

          gbMemory[0xff68] = (gbMemory[0xff68] & 0x80) | index;
          gbMemory[0xff69] = (index & 1 ?
                             (gbPalette[index>>1] >> 8) :
                             (gbPalette[index>>1] & 0x00ff));
        }
        return;
      }
    }
    break;

    // OCPS
    case 0x6a: {
      if(gbCgbMode) {
        int paletteIndex = (value & 0x3f) >> 1;
        int paletteHiLo   = (value & 0x01);

        paletteIndex += 32;

        gbMemory[0xff6a] = value;

        gbMemory[0xff6b] = (paletteHiLo ?
                           (gbPalette[paletteIndex] >> 8) :
                           (gbPalette[paletteIndex] & 0x00ff));
        return;
      }
    }
    break;

    // OCPD
    case 0x6b: {

      if(gbCgbMode) {
        int v = gbMemory[0xff6a];
        int paletteIndex = (v & 0x3f) >> 1;
        int paletteHiLo  = (v & 0x01);

        paletteIndex += 32;

        // No access to gbPalette during mode 3 (Color Panel Demo)
        if (((gbLcdModeDelayed != 3) && (!((gbLcdMode == 0) && (gbLcdTicks>=(GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299]-1)))) && (!gbSpeed)) ||
           (gbSpeed && ((gbLcdMode == 1) || (gbLcdMode == 2) ||
           ((gbLcdMode == 3) && (gbLcdTicks>(GBLCD_MODE_3_CLOCK_TICKS-2))) ||
           ((gbLcdMode == 0) && (gbLcdTicks<=(GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299]-2))))))
        {
          gbMemory[0xff6b] = value;
          gbPalette[paletteIndex] = (paletteHiLo ?
                                    ((value << 8) | (gbPalette[paletteIndex] & 0xff)) :
                                    ((gbPalette[paletteIndex] & 0xff00) | (value))) & 0x7fff;
        }

        if(gbMemory[0xff6a] & 0x80) {
          int index = ((gbMemory[0xff6a] & 0x3f) + 1) & 0x3f;

          gbMemory[0xff6a] = (gbMemory[0xff6a] & 0x80) | index;

          gbMemory[0xff6b] = (index & 1 ?
                             (gbPalette[(index>>1) + 32] >> 8) :
                             (gbPalette[(index>>1) + 32] & 0x00ff));
        }
        return;
      }
    }
    break;

    case 0x6c: {
      gbMemory[0xff6c] = 0xfe | value;
      return;
    }


    // SVBK
    case 0x70: {
      if(gbCgbMode) {
        value = value & 7;

        int bank = value;
        if(value == 0)
          bank = 1;

        if(bank == gbWramBank)
          return;

        int wramAddress = bank * 0x1000;
        gbMemoryMap[0x0d] = &gbWram[wramAddress];

        gbWramBank = bank;
        gbMemory[0xff70] = register_SVBK = value;
        return;
      }
    }

    case 0x75:{
      gbMemory[0xff75] = 0x8f | value;
      return;
    }

    case 0xff: {
      gbMemory[0xffff] = register_IE = value;
      return;
    }
  }

  if(address < 0xff80)
  {
    gbMemory[address] = value;
    return;
  }

  gbMemory[address] = value;
}

u8 gbReadOpcode(register u16 address)
{
  if(gbCheatMap[address])
    return gbCheatRead(address);

  if(address < 0x8000)
      return gbMemoryMap[address>>12][address&0x0fff];

  if(address < 0xa000)
  {
    // A lot of 'ugly' checks... But only way to emulate this particular behaviour...
    if (((gbHardware & 0xa) && ((gbLcdModeDelayed !=3) || ((register_LY == 0) &&
        (gbScreenOn==false) && (register_LCDC & 0x80)) &&
        (gbLcdLYIncrementTicksDelayed ==(GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS)))) ||
        ((gbHardware & 0x5) && (gbLcdModeDelayed !=3) &&
        ((gbLcdMode !=3) ||   ((register_LY == 0) && ((gbScreenOn==false) &&
        (register_LCDC & 0x80)) &&
        (gbLcdLYIncrementTicks ==(GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS))))))
      return gbMemoryMap[address>>12][address&0x0fff];
    return 0xff;
  }

  // Used for the mirroring of 0xC000 in 0xE000
  if ((address >= 0xe000) && (address < 0xfe00))
    address &= ~0x2000;

  switch(address & 0xf000) {
  case 0x0a:
  case 0x0b:
    if(mapperReadRAM)
      return mapperReadRAM(address);
    break;
  case 0x0f:
    if(address > 0xff00) {
      switch(address & 0x00ff) {
      case 0x02:
        return (gbMemory[0xff02]);
      case 0x03:
        return (0xff);
      case 0x04:
        return register_DIV;
      case 0x05:
        return register_TIMA;
      case 0x06:
        return register_TMA;
      case 0x07:
        return (0xf8 | register_TAC);
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
      case 0x0c:
      case 0x0d:
      case 0x0e:
        return (0xff);
      case 0x0f:
        return (0xe0 | gbMemory[0xff0f]);
      case 0x40:
        return register_LCDC;
      case 0x41:
      // This is a GB/C only bug (ie. not GBA/SP).
      if ((gbHardware & 7) && (gbLcdMode == 2) && (gbLcdModeDelayed == 1) && (!gbSpeed))
        return (0x80 | gbMemory[0xff41] & 0xFC);
      else
        return (0x80 | gbMemory[0xff41]);
      case 0x42:
        return register_SCY;
      case 0x43:
        return register_SCX;
      case 0x44:
      if (((gbHardware & 7) && ((gbLcdMode == 1) && (gbLcdTicks == 0x71))) ||
          (!(register_LCDC & 0x80)))
        return 0;
      else
        return register_LY;
      case 0x45:
        return register_LYC;
      case 0x46:
        return register_DMA;
      case 0x4a:
        return register_WY;
      case 0x4b:
        return register_WX;
      case 0x4c:
        return 0xff;
      case 0x4f:
        return (0xfe | register_VBK);
      case 0x51:
        return register_HDMA1;
      case 0x52:
        return register_HDMA2;
      case 0x53:
        return register_HDMA3;
      case 0x54:
        return register_HDMA4;
      case 0x55:
        return register_HDMA5;
      case 0x68:
      case 0x6a:
        if (gbCgbMode)
          return (0x40 | gbMemory[address]);
        else
          return 0xc0;
    case 0x69:
    case 0x6b:
      if (gbCgbMode)
      {
        // No access to gbPalette during mode 3 (Color Panel Demo)
        if (((gbLcdModeDelayed != 3) && (!((gbLcdMode == 0) && (gbLcdTicks>=(GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299]-1)))) && (!gbSpeed)) ||
           (gbSpeed && ((gbLcdMode == 1) || (gbLcdMode == 2) ||
           ((gbLcdMode == 3) && (gbLcdTicks>(GBLCD_MODE_3_CLOCK_TICKS-2))) ||
           ((gbLcdMode == 0) && (gbLcdTicks<=(GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299]-2))))))
          return (gbMemory[address]);
        else
          return 0xff;
      }
      else
        return 0xff;
      case 0x70:
        if (gbCgbMode)
          return (0xf8 | register_SVBK);
        else
          return 0xff;
      case 0xff:
        return register_IE;
      }
    }
  // OAM not accessible during mode 2 & 3.
  if(((address >= 0xfe00) && (address<0xfea0)) &&
    ((gbLcdMode | gbLcdModeDelayed) &2))
      return 0xff;
    break;
  }

  if ((address >= 0xfea0) && (address < 0xff00))
  {
    if (gbHardware & 1)
      return ((((address + ((address >> 4) - 0xfea)) >> 2) & 1) ? 0x00 : 0xff );
    else if (gbHardware & 2)
      return gbMemoryMap[address>>12][address & 0x0fff];
    else if (gbHardware & 4)
      return ((((address + ((address >> 4) - 0xfea)) >> 2) & 1) ? 0xff : 0x00 );
    else if (gbHardware & 8)
      return ((address & 0xf0) |((address & 0xf0)>>4));
  }

  return gbMemoryMap[address>>12][address & 0x0fff];
}

u8 gbReadMemory(register u16 address)
{
  if(gbCheatMap[address])
    return gbCheatRead(address);


  if(address < 0x8000)
    return gbMemoryMap[address>>12][address&0x0fff];

  if(address < 0xa000)
  {
    // A lot of 'ugly' checks... But only way to emulate this particular behaviour...
    if (((gbHardware & 0xa) && ((gbLcdModeDelayed !=3) || ((register_LY == 0) &&
        (gbScreenOn==false) && (register_LCDC & 0x80)) &&
        (gbLcdLYIncrementTicksDelayed ==(GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS)))) ||
        ((gbHardware & 0x5) && (gbLcdModeDelayed !=3) &&
        ((gbLcdMode !=3) ||   ((register_LY == 0) && ((gbScreenOn==false) &&
        (register_LCDC & 0x80)) &&
        (gbLcdLYIncrementTicks ==(GBLY_INCREMENT_CLOCK_TICKS-GBLCD_MODE_2_CLOCK_TICKS))))))
      return gbMemoryMap[address>>12][address&0x0fff];
    return 0xff;
  }

  if ((address >= 0xe000) && (address < 0xfe00))
    address &= ~0x2000;

  if(address < 0xc000) {
#ifndef FINAL_VERSION
    if(memorydebug) {
      log("Memory register read %04x PC=%04x\n",
          address,
          PC.W);
    }
#endif

  // for the 2kb ram limit (fixes crash in shawu's story
  // but now its sram test fails, as the it expects 8kb and not 2kb...
  // So use the 'genericflashcard' option to fix it).
  if (address<=(0xa000+gbRamSizeMask))
  {
    if(mapperReadRAM)
      return mapperReadRAM(address);
    return gbMemoryMap[address>>12][address & 0x0fff];
  }
  return 0xff;
  }

  if(address >= 0xff00) {
	if ( address >= 0xFF10 && address <= 0xFF3F )
		return gbSoundRead( address );

    switch(address & 0x00ff) {
    case 0x00:
      {
        if(gbSgbMode) {
          gbSgbReadingController |= 4;
          gbSgbResetPacketState();
        }

        int b = gbMemory[0xff00];

        if((b & 0x30) == 0x20) {
          b &= 0xf0;

          int joy = 0;
          if(gbSgbMode && gbSgbMultiplayer) {
            switch(gbSgbNextController) {
            case 0x0f:
              joy = 0;
              break;
            case 0x0e:
              joy = 1;
              break;
            case 0x0d:
              joy = 2;
              break;
            case 0x0c:
              joy = 3;
              break;
            default:
              joy = 0;
            }
          }
          int joystate = gbJoymask[joy];
          if(!(joystate & 128))
            b |= 0x08;
          if(!(joystate & 64))
            b |= 0x04;
          if(!(joystate & 32))
            b |= 0x02;
          if(!(joystate & 16))
            b |= 0x01;

          gbMemory[0xff00] = b;
        } else if((b & 0x30) == 0x10) {
          b &= 0xf0;

          int joy = 0;
          if(gbSgbMode && gbSgbMultiplayer) {
            switch(gbSgbNextController) {
            case 0x0f:
              joy = 0;
              break;
            case 0x0e:
              joy = 1;
              break;
            case 0x0d:
              joy = 2;
              break;
            case 0x0c:
              joy = 3;
              break;
            default:
              joy = 0;
            }
          }
          int joystate = gbJoymask[joy];
          if(!(joystate & 8))
            b |= 0x08;
          if(!(joystate & 4))
            b |= 0x04;
          if(!(joystate & 2))
            b |= 0x02;
          if(!(joystate & 1))
            b |= 0x01;

          gbMemory[0xff00] = b;
        } else {
          if(gbSgbMode && gbSgbMultiplayer) {
            gbMemory[0xff00] = 0xf0 | gbSgbNextController;
          } else {
            gbMemory[0xff00] = 0xff;
          }
        }
      }
      return gbMemory[0xff00];
      break;
    case 0x01:
      return gbMemory[0xff01];
    case 0x02:
      return (gbMemory[0xff02]);
    case 0x04:
        return register_DIV;
    case 0x05:
      return register_TIMA;
    case 0x06:
      return register_TMA;
    case 0x07:
      return (0xf8 | register_TAC);
    case 0x0f:
      return (0xe0 | gbMemory[0xff0f]);
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
      if ((gbMemory[NR30] & 0x80) && (gbMemory[NR34] & 0x80))
        return 0xFF;
      else
        return gbMemoryMap[address>>12][address & 0x0fff];
    case 0x40:
      return register_LCDC;
    case 0x41:
      // This is a GB/C only bug (ie. not GBA/SP).
      if ((gbHardware & 7) && (gbLcdMode == 2) && (gbLcdModeDelayed == 1) && (!gbSpeed))
        return (0x80 | gbMemory[0xff41] & 0xFC);
      else
        return (0x80 | gbMemory[0xff41]);
    case 0x42:
      return register_SCY;
    case 0x43:
      return register_SCX;
    case 0x44:
      if (((gbHardware & 7) && ((gbLcdMode == 1) && (gbLcdTicks == 0x71))) ||
          (!(register_LCDC & 0x80)))
        return (0);
      else
        return register_LY;
    case 0x45:
      return register_LYC;
    case 0x46:
      return register_DMA;
    case 0x4a:
      return register_WY;
    case 0x4b:
      return register_WX;
    case 0x4f:
      return (0xfe | register_VBK);
    case 0x51:
      return register_HDMA1;
    case 0x52:
      return register_HDMA2;
    case 0x53:
      return register_HDMA3;
    case 0x54:
      return register_HDMA4;
    case 0x55:
      return register_HDMA5;
    case 0x68:
    case 0x6a:
      if (gbCgbMode)
        return (0x40 | gbMemory[address]);
      else
        return 0xc0;
    case 0x69:
    case 0x6b:
      if (gbCgbMode)
      {
        // No access to gbPalette during mode 3 (Color Panel Demo)
        if (((gbLcdModeDelayed != 3) && (!((gbLcdMode == 0) && (gbLcdTicks>=(GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299]-1)))) && (!gbSpeed)) ||
           (gbSpeed && ((gbLcdMode == 1) || (gbLcdMode == 2) ||
           ((gbLcdMode == 3) && (gbLcdTicks>(GBLCD_MODE_3_CLOCK_TICKS-2))) ||
           ((gbLcdMode == 0) && (gbLcdTicks<=(GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299]-2))))))
          return (gbMemory[address]);
        else
          return 0xff;
      }
      else
        return 0xff;
    case 0x70:
      if (gbCgbMode)
        return (0xf8 | register_SVBK);
      else
        return 0xff;
    case 0xff:
      return register_IE;
    }
  }
  // OAM not accessible during mode 2 & 3.
  if(((address >= 0xfe00) && (address<0xfea0)) &&
    (((gbLcdMode | gbLcdModeDelayed) &2) &&
    (!(gbSpeed && (gbHardware & 0x2) && !(gbLcdModeDelayed & 2) && (gbLcdMode == 2))) ||
      (gbSpeed && (gbHardware & 0x2) && (gbLcdModeDelayed == 0) && (gbLcdTicksDelayed == (GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299])))))
  return 0xff;

  if ((address >= 0xfea0) && (address < 0xff00))
  {
    if (gbHardware & 1)
      return ((((address + ((address >> 4) - 0xfea)) >> 2) & 1) ? 0x00 : 0xff );
    else if (gbHardware & 2)
      return gbMemoryMap[address>>12][address & 0x0fff];
    else if (gbHardware & 4)
      return ((((address + ((address >> 4) - 0xfea)) >> 2) & 1) ? 0xff : 0x00 );
    else if (gbHardware & 8)
      return ((address & 0xf0) |((address & 0xf0)>>4));
  }

  return gbMemoryMap[address>>12][address & 0x0fff];
}

void gbVblank_interrupt()
{
  gbCheatWrite(false); // Emulates GS codes.
  gbMemory[0xff0f] = register_IF &= 0xfe;
  gbWriteMemory(--SP.W, PC.B.B1);
  gbWriteMemory(--SP.W, PC.B.B0);
  PC.W = 0x40;
}

void gbLcd_interrupt()
{
  gbMemory[0xff0f] = register_IF &= 0xfd;
  gbWriteMemory(--SP.W, PC.B.B1);
  gbWriteMemory(--SP.W, PC.B.B0);
  PC.W = 0x48;
}

void gbTimer_interrupt()
{
  gbMemory[0xff0f] = register_IF &= 0xfb;
  gbWriteMemory(--SP.W, PC.B.B1);
  gbWriteMemory(--SP.W, PC.B.B0);
  PC.W = 0x50;
}

void gbSerial_interrupt()
{
  gbMemory[0xff0f] = register_IF &= 0xf7;
  gbWriteMemory(--SP.W, PC.B.B1);
  gbWriteMemory(--SP.W, PC.B.B0);
  PC.W = 0x58;
}

void gbJoypad_interrupt()
{
  gbMemory[0xff0f] = register_IF &= 0xef;
  gbWriteMemory(--SP.W, PC.B.B1);
  gbWriteMemory(--SP.W, PC.B.B0);
  PC.W = 0x60;
}

void gbSpeedSwitch()
{
  gbBlackScreen = true;
  if(gbSpeed == 0) {
    gbSpeed = 1;
    GBLCD_MODE_0_CLOCK_TICKS = 51 * 2;
    GBLCD_MODE_1_CLOCK_TICKS = 1140 * 2;
    GBLCD_MODE_2_CLOCK_TICKS = 20 * 2;
    GBLCD_MODE_3_CLOCK_TICKS = 43 * 2;
    GBLY_INCREMENT_CLOCK_TICKS = 114 * 2;
    GBDIV_CLOCK_TICKS = 64;
    GBTIMER_MODE_0_CLOCK_TICKS = 256;
    GBTIMER_MODE_1_CLOCK_TICKS = 4;
    GBTIMER_MODE_2_CLOCK_TICKS = 16;
    GBTIMER_MODE_3_CLOCK_TICKS = 64;
    GBSERIAL_CLOCK_TICKS = 128 * 2;
    gbLcdTicks *= 2;
    gbLcdTicksDelayed *=2;
    gbLcdTicksDelayed--;
    gbLcdLYIncrementTicks *= 2;
    gbLcdLYIncrementTicksDelayed *= 2;
    gbLcdLYIncrementTicksDelayed--;
    gbSerialTicks *= 2;
    //SOUND_CLOCK_TICKS = soundQuality * 24 * 2;
    //soundTicks *= 2;
    gbLine99Ticks = 3;
  } else {
    gbSpeed = 0;
    GBLCD_MODE_0_CLOCK_TICKS = 51;
    GBLCD_MODE_1_CLOCK_TICKS = 1140;
    GBLCD_MODE_2_CLOCK_TICKS = 20;
    GBLCD_MODE_3_CLOCK_TICKS = 43;
    GBLY_INCREMENT_CLOCK_TICKS = 114;
    GBDIV_CLOCK_TICKS = 64;
    GBTIMER_MODE_0_CLOCK_TICKS = 256;
    GBTIMER_MODE_1_CLOCK_TICKS = 4;
    GBTIMER_MODE_2_CLOCK_TICKS = 16;
    GBTIMER_MODE_3_CLOCK_TICKS = 64;
    GBSERIAL_CLOCK_TICKS = 128;
    gbLcdTicks >>= 1;
    gbLcdTicksDelayed++;
    gbLcdTicksDelayed >>=1;
    gbLcdLYIncrementTicks >>= 1;
    gbLcdLYIncrementTicksDelayed++;
    gbLcdLYIncrementTicksDelayed >>= 1;
    gbSerialTicks /= 2;
    //SOUND_CLOCK_TICKS = soundQuality * 24;
    //soundTicks /= 2;
    gbLine99Ticks = 1;
    if (gbHardware & 8)
      gbLine99Ticks++;
  }
  gbDmaTicks += (134)*GBLY_INCREMENT_CLOCK_TICKS + (37<<(gbSpeed ? 1 : 0));
}

bool CPUIsGBBios(const char * file)
{
  if(strlen(file) > 4) {
    const char * p = strrchr(file,'.');

    if(p != NULL) {
      if(_stricmp(p, ".gb") == 0)
        return true;
      if(_stricmp(p, ".bin") == 0)
        return true;
      if(_stricmp(p, ".bios") == 0)
        return true;
      if(_stricmp(p, ".rom") == 0)
        return true;
    }
  }

  return false;
}

void gbCPUInit(const char *biosFileName, bool useBiosFile)
{
  useBios = false;
  if (useBiosFile)
  {
    int size = 0x100;
    if(utilLoad(biosFileName,
                CPUIsGBBios,
                bios,
                size)) {
      if(size == 0x100)
        useBios = true;
      else
        systemMessage(MSG_INVALID_BIOS_FILE_SIZE, N_("Invalid BOOTROM file size"));
    }
  }
}

void gbGetHardwareType()
{
  gbCgbMode = 0;
  gbSgbMode = 0;
  if(gbRom[0x143] & 0x80) {
    if((gbEmulatorType == 0) ||
       gbEmulatorType == 1 ||
       gbEmulatorType == 4) {
      gbCgbMode = 1;
    }
  }

  if((gbCgbMode == 0 ) && (gbRom[0x146] == 0x03)) {
    if(gbEmulatorType == 0 ||
       gbEmulatorType == 2 ||
       gbEmulatorType == 5)
      gbSgbMode = 1;
  }

  gbHardware = 1; // GB
  if (((gbCgbMode == 1) && (gbEmulatorType == 0)) || (gbEmulatorType == 1))
    gbHardware = 2; // GBC
  else if (((gbSgbMode == 1) && (gbEmulatorType == 0)) || (gbEmulatorType == 2) || (gbEmulatorType == 5))
    gbHardware = 4; // SGB(2)
  else if (gbEmulatorType == 4)
    gbHardware = 8; // GBA

  gbGBCColorType = 0;
  if (gbHardware & 8) // If GBA is selected, choose the GBA default settings.
    gbGBCColorType = 2;    // (0 = GBC, 1 = GBA, 2 = GBASP)
}

void gbReset()
{
  gbGetHardwareType();

  oldRegister_WY = 146;
  gbInterruptLaunched = 0;

  if(gbCgbMode == 1) {
      if (gbVram == NULL)
        gbVram = (u8 *)malloc(0x4000);
      if (gbWram == NULL)
        gbWram = (u8 *)malloc(0x8000);
      memset(gbVram,0,0x4000);
      memset(gbPalette,0, 2*128);
  }
  else
  {
    if(gbVram != NULL) {
      free(gbVram);
      gbVram = NULL;
    }
    if(gbWram != NULL) {
      free(gbWram);
      gbWram = NULL;
    }
  }

  gbLYChangeHappened = false;
  gbLCDChangeHappened = false;
  gbBlackScreen = false;
  gbInterruptWait = 0;
  gbDmaTicks = 0;
  clockTicks = 0;

  if(gbSpeed) {
    gbSpeedSwitch();
    gbMemory[0xff4d] = 0;
  }

  // clean Wram
  // This kinda emulates the startup state of Wram on GB/C (not very accurate,
  // but way closer to the reality than filling it with 00es or FFes).
  // On GBA/GBASP, it's kinda filled with random data.
  // In all cases, most of the 2nd bank is filled with 00s.
  // The starting data are important for some 'buggy' games, like Buster Brothers or
  // Karamuchou ha Oosawagi!.
  if (gbMemory != NULL)
  {
    memset(gbMemory,0xff, 65536);
    for (int temp = 0xC000; temp < 0xE000; temp++)
      if ((temp & 0x8) ^((temp & 0x800)>>8))
      {
          if ((gbHardware & 0x02) && (gbGBCColorType == 0))
            gbMemory[temp] = 0x0;
          else
            gbMemory[temp] = 0x0f;
      }

      else
          gbMemory[temp] = 0xff;
  }

  // GB bios set this memory area to 0
  // Fixes Pitman (J) title screen
  if (gbHardware & 0x1) {
    memset(&gbMemory[0x8000], 0x0, 0x2000);
  }

  // clean LineBuffer
  if (gbLineBuffer != NULL)
    memset(gbLineBuffer, 0, sizeof(gbLineBuffer));
  // clean Pix
  if (pix != NULL)
    memset(pix, 0, sizeof(pix));
  // clean Vram
  if (gbVram != NULL)
    memset(gbVram, 0, 0x4000);
  // clean Wram 2
  // This kinda emulates the startup state of Wram on GBC (not very accurate,
  // but way closer to the reality than filling it with 00es or FFes).
  // On GBA/GBASP, it's kinda filled with random data.
  // In all cases, most of the 2nd bank is filled with 00s.
  // The starting data are important for some 'buggy' games, like Buster Brothers or
  // Karamuchou ha Oosawagi!
  if (gbWram != NULL)
  {
      for (int i = 0; i<8; i++)
          if (i != 2)
      memcpy ((u16 *)(gbWram+i*0x1000), (u16 *)(gbMemory+0xC000), 0x1000);
  }

  memset(gbSCYLine,0,sizeof(gbSCYLine));
  memset(gbSCXLine,0,sizeof(gbSCXLine));
  memset(gbBgpLine,0xfc,sizeof(gbBgpLine));
  if (gbHardware & 5)
  {
    memset(gbObp0Line,0xff,sizeof(gbObp0Line));
    memset(gbObp1Line,0xff,sizeof(gbObp1Line));
  }
  else
  {
    memset(gbObp0Line,0x0,sizeof(gbObp0Line));
    memset(gbObp1Line,0x0,sizeof(gbObp1Line));
  }
  memset(gbSpritesTicks,0x0,sizeof(gbSpritesTicks));

  SP.W = 0xfffe;
  AF.W = 0x01b0;
  BC.W = 0x0013;
  DE.W = 0x00d8;
  HL.W = 0x014d;
  PC.W = 0x0100;
  IFF = 0;
  gbInt48Signal = 0;

  register_TIMA = 0;
  register_TMA = 0;
  register_TAC = 0;
  gbMemory[0xff0f] = register_IF = 1;
  gbMemory[0xff40] = register_LCDC = 0x91;
  gbMemory[0xff47] = 0xfc;

  if (gbCgbMode)
    gbMemory[0xff4d] = 0x7e;
  else
    gbMemory[0xff4d] = 0xff;

  if (!gbCgbMode)
    gbMemory[0xff70] = gbMemory[0xff74] = 0xff;

  if (gbCgbMode)
    gbMemory[0xff56] = 0x3e;
  else
    gbMemory[0xff56] = 0xff;

  register_SCY = 0;
  register_SCX = 0;
  register_LYC = 0;
  register_DMA = 0xff;
  register_WY = 0;
  register_WX = 0;
  register_VBK = 0;
  register_HDMA1 = 0xff;
  register_HDMA2 = 0xff;
  register_HDMA3 = 0xff;
  register_HDMA4 = 0xff;
  register_HDMA5 = 0xff;
  register_SVBK = 0;
  register_IE = 0;

  if (gbCgbMode)
    gbMemory[0xff02] = 0x7c;
  else
    gbMemory[0xff02] = 0x7e;

  gbMemory[0xff03] = 0xff;
  int i;
  for (i = 0x8; i<0xf; i++)
    gbMemory[0xff00+i] = 0xff;

  gbMemory[0xff13] = 0xff;
  gbMemory[0xff15] = 0xff;
  gbMemory[0xff18] = 0xff;
  gbMemory[0xff1d] = 0xff;
  gbMemory[0xff1f] = 0xff;

  for (i = 0x27; i<0x30; i++)
    gbMemory[0xff00+i] = 0xff;

  gbMemory[0xff4c] = 0xff;
  gbMemory[0xff4e] = 0xff;
  gbMemory[0xff50] = 0xff;

  for (i = 0x57; i<0x68; i++)
    gbMemory[0xff00+i] = 0xff;

  for (i = 0x5d; i<0x70; i++)
    gbMemory[0xff00+i] = 0xff;

  gbMemory[0xff71] = 0xff;

  for (i = 0x78; i<0x80; i++)
    gbMemory[0xff00+i] = 0xff;

  if (gbHardware & 0xa)
  {

      if (gbHardware & 2)
      {
        AF.W = 0x1180;
        BC.W = 0x0000;
      }
      else
      {
        AF.W = 0x1100;
        BC.W = 0x0100; // GBA/SP have B = 0x01 (which means GBC & GBA/SP bootrom are different !)
      }

    gbMemory[0xff26] = 0xf1;
    if (gbCgbMode)
    {

      gbMemory[0xff31] = 0xff;
      gbMemory[0xff33] = 0xff;
      gbMemory[0xff35] = 0xff;
      gbMemory[0xff37] = 0xff;
      gbMemory[0xff39] = 0xff;
      gbMemory[0xff3b] = 0xff;
      gbMemory[0xff3d] = 0xff;

      gbMemory[0xff44] = register_LY = 0x90;
      gbDivTicks = 0x19 + ((gbHardware & 2) >> 1);
      gbInternalTimer = 0x58 + ((gbHardware & 2) >> 1);
      gbLcdTicks = GBLCD_MODE_1_CLOCK_TICKS -
                   (register_LY-0x8F)*GBLY_INCREMENT_CLOCK_TICKS + 72 + ((gbHardware & 2) >> 1);
      gbLcdLYIncrementTicks = 72 + ((gbHardware & 2) >> 1);
      gbMemory[0xff04] = register_DIV = 0x1E;
    }
    else
    {
      gbMemory[0xff44] = register_LY = 0x94;
      gbDivTicks = 0x22 + ((gbHardware & 2) >> 1);
      gbInternalTimer = 0x61 + ((gbHardware & 2) >> 1);
      gbLcdTicks = GBLCD_MODE_1_CLOCK_TICKS -
                   (register_LY-0x8F)*GBLY_INCREMENT_CLOCK_TICKS + 25 + ((gbHardware & 2) >> 1);
      gbLcdLYIncrementTicks = 25 + ((gbHardware & 2) >> 1);
      gbMemory[0xff04] = register_DIV = 0x26;
    }


    DE.W = 0xff56;
    HL.W = 0x000d;

    register_HDMA5 = 0xff;
    gbMemory[0xff68] = 0xc0;
    gbMemory[0xff6a] = 0xc0;


    gbMemory[0xff41] = register_STAT = 0x81;
    gbLcdMode = 1;
  }
  else
  {
    if (gbHardware & 4)
    {
      if(gbEmulatorType == 5)
        AF.W = 0xffb0;
      else
        AF.W = 0x01b0;
      BC.W = 0x0013;
      DE.W = 0x00d8;
      HL.W = 0x014d;
    }
    gbDivTicks = 14;
    gbInternalTimer = gbDivTicks--;
    gbMemory[0xff04] = register_DIV = 0xAB;
    gbMemory[0xff41] = register_STAT = 0x85;
    gbMemory[0xff44] = register_LY = 0x00;
    gbLcdTicks = 15;
    gbLcdLYIncrementTicks = 114+gbLcdTicks;
    gbLcdMode = 1;

    // used for the handling of the gb Boot Rom
    if ((gbHardware & 5) && (bios != NULL) && useBios && !skipBios)
    {
      memcpy ((u8 *)(gbMemory), (u8 *)(gbRom), 0x1000);
      memcpy ((u8 *)(gbMemory), (u8 *)(bios), 0x100);
      gbWhiteScreen = 0;

      gbInternalTimer = 0x3e;
      gbDivTicks = 0x3f;
      gbMemory[0xff04] = register_DIV = 0x00;
      PC.W = 0x0000;
      register_LCDC = 0x11;
      gbScreenOn = false;
      gbLcdTicks = 0;
      gbLcdMode = 0;
      gbLcdModeDelayed = 0;
      gbMemory[0xff41] = register_STAT &= 0xfc;
      gbInt48Signal = 0;
      gbLcdLYIncrementTicks = GBLY_INCREMENT_CLOCK_TICKS;
    }
  }

  gbLine99Ticks = 1;
  if (gbHardware & 8)
    gbLine99Ticks++;

  gbLcdModeDelayed = gbLcdMode;
  gbLcdTicksDelayed = gbLcdTicks+1;
  gbLcdLYIncrementTicksDelayed = gbLcdLYIncrementTicks+1;


  gbTimerModeChange = false;
  gbTimerOnChange = false;
  gbTimerOn = false;

  if(gbCgbMode) {
    for (int i = 0; i<0x20; i++)
      gbPalette[i] = 0x7fff;

    // This is just to show that the starting values of the OBJ palettes are different
    // between the 3 consoles, and that they 'kinda' stay the same at each reset
    // (they can slightly change, somehow (randomly?)).
    // You can check the effects of gbGBCColorType on the "Vila Caldan Color" gbc demo.
    // Note that you could also check the Div register to check on which system the game
    // is running (GB,GBC and GBA(SP) have different startup values).
    // Unfortunatly, I don't have any SGB system, so I can't get their starting values.

    if (gbGBCColorType == 0) // GBC Hardware
    {
    gbPalette[0x20] = 0x0600;
    gbPalette[0x21] = 0xfdf3;
    gbPalette[0x22] = 0x041c;
    gbPalette[0x23] = 0xf5db;
    gbPalette[0x24] = 0x4419;
    gbPalette[0x25] = 0x57ea;
    gbPalette[0x26] = 0x2808;
    gbPalette[0x27] = 0x9b75;
    gbPalette[0x28] = 0x129b;
    gbPalette[0x29] = 0xfce0;
    gbPalette[0x2a] = 0x22da;
    gbPalette[0x2b] = 0x4ac5;
    gbPalette[0x2c] = 0x2d71;
    gbPalette[0x2d] = 0xf0c2;
    gbPalette[0x2e] = 0x5137;
    gbPalette[0x2f] = 0x2d41;
    gbPalette[0x30] = 0x6b2d;
    gbPalette[0x31] = 0x2215;
    gbPalette[0x32] = 0xbe0a;
    gbPalette[0x33] = 0xc053;
    gbPalette[0x34] = 0xfe5f;
    gbPalette[0x35] = 0xe000;
    gbPalette[0x36] = 0xbe10;
    gbPalette[0x37] = 0x914d;
    gbPalette[0x38] = 0x7f91;
    gbPalette[0x39] = 0x02b5;
    gbPalette[0x3a] = 0x77ac;
    gbPalette[0x3b] = 0x14e5;
    gbPalette[0x3c] = 0xcf89;
    gbPalette[0x3d] = 0xa03d;
    gbPalette[0x3e] = 0xfd50;
    gbPalette[0x3f] = 0x91ff;
    }
    else if (gbGBCColorType == 1) // GBA Hardware
    {
    gbPalette[0x20] = 0xbe00;
    gbPalette[0x21] = 0xfdfd;
    gbPalette[0x22] = 0xbd69;
    gbPalette[0x23] = 0x7baf;
    gbPalette[0x24] = 0xf5ff;
    gbPalette[0x25] = 0x3f8f;
    gbPalette[0x26] = 0xcee5;
    gbPalette[0x27] = 0x5bf7;
    gbPalette[0x28] = 0xb35b;
    gbPalette[0x29] = 0xef97;
    gbPalette[0x2a] = 0xef9f;
    gbPalette[0x2b] = 0x97f7;
    gbPalette[0x2c] = 0x82bf;
    gbPalette[0x2d] = 0x9f3d;
    gbPalette[0x2e] = 0xddde;
    gbPalette[0x2f] = 0xbad5;
    gbPalette[0x30] = 0x3cba;
    gbPalette[0x31] = 0xdfd7;
    gbPalette[0x32] = 0xedea;
    gbPalette[0x33] = 0xfeda;
    gbPalette[0x34] = 0xf7f9;
    gbPalette[0x35] = 0xfdee;
    gbPalette[0x36] = 0x6d2f;
    gbPalette[0x37] = 0xf0e6;
    gbPalette[0x38] = 0xf7f0;
    gbPalette[0x39] = 0xf296;
    gbPalette[0x3a] = 0x3bf1;
    gbPalette[0x3b] = 0xe211;
    gbPalette[0x3c] = 0x69ba;
    gbPalette[0x3d] = 0x3d0d;
    gbPalette[0x3e] = 0xdfd3;
    gbPalette[0x3f] = 0xa6ba;
    }
    else if (gbGBCColorType == 2) // GBASP Hardware
    {
    gbPalette[0x20] = 0x9c00;
    gbPalette[0x21] = 0x6340;
    gbPalette[0x22] = 0x10c6;
    gbPalette[0x23] = 0xdb97;
    gbPalette[0x24] = 0x7622;
    gbPalette[0x25] = 0x3e57;
    gbPalette[0x26] = 0x2e12;
    gbPalette[0x27] = 0x95c3;
    gbPalette[0x28] = 0x1095;
    gbPalette[0x29] = 0x488c;
    gbPalette[0x2a] = 0x8241;
    gbPalette[0x2b] = 0xde8c;
    gbPalette[0x2c] = 0xfabc;
    gbPalette[0x2d] = 0x0e81;
    gbPalette[0x2e] = 0x7675;
    gbPalette[0x2f] = 0xfdec;
    gbPalette[0x30] = 0xddfd;
    gbPalette[0x31] = 0x5995;
    gbPalette[0x32] = 0x066a;
    gbPalette[0x33] = 0xed1e;
    gbPalette[0x34] = 0x1e84;
    gbPalette[0x35] = 0x1d14;
    gbPalette[0x36] = 0x11c3;
    gbPalette[0x37] = 0x2749;
    gbPalette[0x38] = 0xa727;
    gbPalette[0x39] = 0x6266;
    gbPalette[0x3a] = 0xe27b;
    gbPalette[0x3b] = 0xe3fc;
    gbPalette[0x3c] = 0x1f76;
    gbPalette[0x3d] = 0xf158;
    gbPalette[0x3e] = 0x468e;
    gbPalette[0x3f] = 0xa540;
    }
  } else {
    if(gbSgbMode) {
      for(int i = 0; i < 8; i++)
        gbPalette[i] = systemGbPalette[gbPaletteOption*8+i];

    }
    for(int i = 0; i < 8; i++)
      gbPalette[i] = systemGbPalette[gbPaletteOption*8+i];
  }

  GBTIMER_MODE_0_CLOCK_TICKS = 256;
  GBTIMER_MODE_1_CLOCK_TICKS = 4;
  GBTIMER_MODE_2_CLOCK_TICKS = 16;
  GBTIMER_MODE_3_CLOCK_TICKS = 64;

  GBLY_INCREMENT_CLOCK_TICKS = 114;
  gbTimerTicks = GBTIMER_MODE_0_CLOCK_TICKS;
  gbTimerClockTicks = GBTIMER_MODE_0_CLOCK_TICKS;
  gbSerialTicks = 0;
  gbSerialBits = 0;
  gbSerialOn = 0;
  gbWindowLine = -1;
  gbTimerOn = false;
  gbTimerMode = 0;
  gbSpeed = 0;
  gbJoymask[0] = gbJoymask[1] = gbJoymask[2] = gbJoymask[3] = 0;

  if(gbCgbMode) {
    gbSpeed = 0;
    gbHdmaOn = 0;
    gbHdmaSource = 0x99d0;
    gbHdmaDestination = 0x99d0;
    gbVramBank = 0;
    gbWramBank = 1;

  }

  // used to clean the borders
  if (gbSgbMode)
  {
    gbSgbResetFlag = true;
    gbSgbReset();
    if (gbBorderOn)
      gbSgbRenderBorder();
    gbSgbResetFlag = false;
  }

  for(i = 0; i < 4; i++)
    gbBgp[i] = gbObp0[i] = gbObp1[i] = i;

  memset(&gbDataMBC1,0, sizeof(gbDataMBC1));
  gbDataMBC1.mapperROMBank = 1;

  gbDataMBC2.mapperRAMEnable = 0;
  gbDataMBC2.mapperROMBank = 1;

  memset(&gbDataMBC3,0, 6 * sizeof(int));
  gbDataMBC3.mapperROMBank = 1;

  memset(&gbDataMBC5, 0, sizeof(gbDataMBC5));
  gbDataMBC5.mapperROMBank = 1;

  memset(&gbDataHuC1, 0, sizeof(gbDataHuC1));
  gbDataHuC1.mapperROMBank = 1;

  memset(&gbDataHuC3, 0, sizeof(gbDataHuC3));
  gbDataHuC3.mapperROMBank = 1;

  memset(&gbDataTAMA5,0, 26*sizeof(int));
  gbDataTAMA5.mapperROMBank = 1;

  memset(&gbDataMMM01,0, sizeof(gbDataMMM01));
  gbDataMMM01.mapperROMBank = 1;

  if (useBios && !skipBios && (gbHardware & 5))
  {
    gbMemoryMap[0x00] = &gbMemory[0x0000];
    inBios = true;
  }
  else
  {
    gbMemoryMap[0x00] = &gbRom[0x0000];
    inBios = false;
  }

  gbMemoryMap[0x01] = &gbRom[0x1000];
  gbMemoryMap[0x02] = &gbRom[0x2000];
  gbMemoryMap[0x03] = &gbRom[0x3000];
  gbMemoryMap[0x04] = &gbRom[0x4000];
  gbMemoryMap[0x05] = &gbRom[0x5000];
  gbMemoryMap[0x06] = &gbRom[0x6000];
  gbMemoryMap[0x07] = &gbRom[0x7000];
  if(gbCgbMode) {
    gbMemoryMap[0x08] = &gbVram[0x0000];
    gbMemoryMap[0x09] = &gbVram[0x1000];
    gbMemoryMap[0x0a] = &gbMemory[0xa000];
    gbMemoryMap[0x0b] = &gbMemory[0xb000];
    gbMemoryMap[0x0c] = &gbMemory[0xc000];
    gbMemoryMap[0x0d] = &gbWram[0x1000];
    gbMemoryMap[0x0e] = &gbMemory[0xe000];
    gbMemoryMap[0x0f] = &gbMemory[0xf000];
  } else {
    gbMemoryMap[0x08] = &gbMemory[0x8000];
    gbMemoryMap[0x09] = &gbMemory[0x9000];
    gbMemoryMap[0x0a] = &gbMemory[0xa000];
    gbMemoryMap[0x0b] = &gbMemory[0xb000];
    gbMemoryMap[0x0c] = &gbMemory[0xc000];
    gbMemoryMap[0x0d] = &gbMemory[0xd000];
    gbMemoryMap[0x0e] = &gbMemory[0xe000];
    gbMemoryMap[0x0f] = &gbMemory[0xf000];
  }

  if(gbRam) {
    gbMemoryMap[0x0a] = &gbRam[0x0000];
    gbMemoryMap[0x0b] = &gbRam[0x1000];
  }

  gbSoundReset();

  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

  gbLastTime = systemGetClock();
  gbFrameCount = 0;

  gbScreenOn = true;
  gbSystemMessage = false;

  gbCheatWrite(true); // Emulates GS codes.

}

void gbWriteSaveMBC1(const char * name)
{
  if (gbRam)
  {
    FILE *gzFile = fopen(name,"wb");

    if(gzFile == NULL) {
      systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
      return;
    }

    fwrite(gbRam,
           1,
           (gbRamSizeMask+1),
           gzFile);

    fclose(gzFile);
  }
}

void gbWriteSaveMBC2(const char * name)
{
  if (gbRam)
  {
    FILE *file = fopen(name, "wb");

    if(file == NULL) {
      systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
      return;
    }

    fwrite(&gbMemory[0xa000],
           1,
           256,
           file);

    fclose(file);
  }
}

void gbWriteSaveMBC3(const char * name, bool extendedSave)
{
  if (gbRam || extendedSave)
  {
    FILE *gzFile = fopen(name,"wb");
    if (gbRam)
    {

      if(gzFile == NULL) {
        systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
        return;
      }

      fwrite(gbRam,
             1,
             (gbRamSizeMask+1),
             gzFile);
    }

    if(extendedSave)
      fwrite(&gbDataMBC3.mapperSeconds,
             1,
             10*sizeof(int) + sizeof(time_t),
             gzFile);

    fclose(gzFile);
  }
}

void gbWriteSaveMBC5(const char * name)
{
  if (gbRam)
  {
    FILE *gzFile = fopen(name,"wb");

    if(gzFile == NULL) {
      systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
      return;
    }

    fwrite(gbRam,
           1,
           (gbRamSizeMask+1),
           gzFile);

    fclose(gzFile);
  }
}

void gbWriteSaveMBC7(const char * name)
{
  if (gbRam)
  {
    FILE *file = fopen(name, "wb");

    if(file == NULL) {
      systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
      return;
    }

    fwrite(&gbMemory[0xa000],
           1,
           256,
           file);

    fclose(file);
  }
}

void gbWriteSaveTAMA5(const char * name, bool extendedSave)
{
  FILE *gzFile = fopen(name,"wb");

  if(gzFile == NULL) {
    systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
    return;
  }
  if (gbRam)
    fwrite(gbRam,
           1,
           (gbRamSizeMask+1),
           gzFile);

  fwrite(gbTAMA5ram,
         1,
         (gbTAMA5ramSize),
         gzFile);

  if(extendedSave)
    fwrite(&gbDataTAMA5.mapperSeconds,
           1,
           14*sizeof(int) + sizeof(time_t),
           gzFile);

  fclose(gzFile);
}

void gbWriteSaveMMM01(const char * name)
{
  if (gbRam)
  {
    FILE *gzFile = fopen(name,"wb");

    if(gzFile == NULL) {
      systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
      return;
    }

    fwrite(gbRam,
           1,
           (gbRamSizeMask+1),
           gzFile);

    fclose(gzFile);
  }
}


bool gbReadSaveMBC1(const char * name)
{
  if (gbRam)
  {
    gzFile gzFile = gzopen(name, "rb");

    if(gzFile == NULL) {
      return false;
    }

    int read = gzread(gzFile,
                      gbRam,
                      (gbRamSizeMask+1));

    if(read != (gbRamSizeMask+1)) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      gzclose(gzFile);
      gbBatteryError = true;
      return false;
    }

    // Also checks if the battery file it bigger than gbRamSizeMask+1 !
    u8 data[1];
    data[0] = 0;

    read = gzread(gzFile,
                  data,
                  1);
    if(read >0) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      gzclose(gzFile);
      gbBatteryError = true;
      return false;
    }

    gzclose(gzFile);
    return true;
  }
  else
    return false;
}


bool gbReadSaveMBC2(const char * name)
{
  if (gbRam)
  {
    FILE *file = fopen(name, "rb");

    if(file == NULL) {
      return false;
    }

    size_t read = fread(&gbMemory[0xa000],
                     1,
                     256,
                     file);

    if(read != 256) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      fclose(file);
      gbBatteryError = true;
      return false;
    }

    // Also checks if the battery file it bigger than gbRamSizeMask+1 !
    u8 data[1];
    data[0] = 0;

    read = fread(&data[0],
                 1,
                 1,
                 file);
    if(read > 0) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      fclose(file);
      gbBatteryError = true;
      return false;
    }

    fclose(file);
    return true;
  }
  else
    return false;
}

bool gbReadSaveMBC3(const char * name)
{
  gzFile gzFile = gzopen(name, "rb");

  if(gzFile == NULL) {
    return false;
  }

  int read = 0;

  if (gbRam)
    read = gzread(gzFile,
                      gbRam,
                      (gbRamSizeMask+1));
  else
    read = (gbRamSizeMask+1);


  bool res = true;

  if(read != (gbRamSizeMask+1)) {
    systemMessage(MSG_FAILED_TO_READ_SGM,
                  N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
    gbBatteryError = true;
    res = false;
  } else if ((gbRomType == 0xf) || (gbRomType == 0x10)){
    read = gzread(gzFile,
                  &gbDataMBC3.mapperSeconds,
                  sizeof(int)*10 + sizeof(time_t));

    if(read != (sizeof(int)*10 + sizeof(time_t)) && read != 0) {
       systemMessage(MSG_FAILED_TO_READ_RTC,N_("Failed to read RTC from save game %s (continuing)"),
                     name);
       res = false;
    }
    else if (read == 0)
    {
       systemMessage(MSG_FAILED_TO_READ_RTC,N_("Failed to read RTC from save game %s (continuing)"),
                     name);
       res = false;
    }
    else
    {
      // Also checks if the battery file it bigger than gbRamSizeMask+1+RTC !
      u8 data[1];
      data[0] = 0;

      read = gzread(gzFile,
                    data,
                    1);
      if(read >0) {
        systemMessage(MSG_FAILED_TO_READ_SGM,
                      N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
        gbBatteryError = true;
        res = false;
      }
    }
  }

  gzclose(gzFile);
  return res;
}

bool gbReadSaveMBC5(const char * name)
{
  if (gbRam)
  {
    gzFile gzFile = gzopen(name, "rb");

    if(gzFile == NULL) {
      return false;
    }

    int read = gzread(gzFile,
                      gbRam,
                      (gbRamSizeMask+1));

    if(read != (gbRamSizeMask+1)) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      gzclose(gzFile);
      gbBatteryError = true;
      return false;
    }


    // Also checks if the battery file it bigger than gbRamSizeMask+1 !
    u8 data[1];
    data[0] = 0;

    read = gzread(gzFile,
                  data,
                  1);
    if(read >0) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      gzclose(gzFile);
      gbBatteryError = true;
      return false;
    }

    gzclose(gzFile);
    return true;
  }
  else
    return false;
}

bool gbReadSaveMBC7(const char * name)
{
  if (gbRam)
  {
    FILE *file = fopen(name, "rb");

    if(file == NULL) {
      return false;
    }

    size_t read = fread(&gbMemory[0xa000],
                     1,
                     256,
                     file);

    if(read != 256) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      fclose(file);
      gbBatteryError = true;
      return false;
    }

    // Also checks if the battery file it bigger than gbRamSizeMask+1 !
    u8 data[1];
    data[0] = 0;

    read = fread(&data[0],
                 1,
                 1,
                 file);
    if(read > 0) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      fclose(file);
      gbBatteryError = true;
      return false;
    }

    fclose(file);
    return true;
  }
  else
    return false;
}

bool gbReadSaveTAMA5(const char * name)
{
  gzFile gzFile = gzopen(name, "rb");

  if(gzFile == NULL) {
    return false;
  }

  int read = 0;

  if (gbRam)
    read = gzread(gzFile,
                      gbRam,
                      (gbRamSizeMask+1));
  else
    read = gbRamSizeMask;

  read += gzread(gzFile,
                    gbTAMA5ram,
                    gbTAMA5ramSize);

  bool res = true;

  if(read != (gbRamSizeMask+gbTAMA5ramSize+1)) {
    systemMessage(MSG_FAILED_TO_READ_SGM,
                  N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
    gbBatteryError = true;
    res = false;
  } else {
    read = gzread(gzFile,
                  &gbDataTAMA5.mapperSeconds,
                  sizeof(int)*14 + sizeof(time_t));

    if(read != (sizeof(int)*14 + sizeof(time_t)) && read != 0) {
       systemMessage(MSG_FAILED_TO_READ_RTC,N_("Failed to read RTC from save game %s (continuing)"),
                     name);
       res = false;
    }
    else if (read == 0)
    {
       systemMessage(MSG_FAILED_TO_READ_RTC,N_("Failed to read RTC from save game %s (continuing)"),
                     name);
       res = false;
    }
    else
    {
      // Also checks if the battery file it bigger than gbRamSizeMask+1+RTC !
      u8 data[1];
      data[0] = 0;

      read = gzread(gzFile,
                    data,
                    1);
      if(read >0) {
        systemMessage(MSG_FAILED_TO_READ_SGM,
                      N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
        gbBatteryError = true;
        res = false;
      }
    }
  }

  gzclose(gzFile);
  return res;
}


bool gbReadSaveMMM01(const char * name)
{
  if (gbRam)
  {
    gzFile gzFile = gzopen(name, "rb");

    if(gzFile == NULL) {
      return false;
    }

    int read = gzread(gzFile,
                      gbRam,
                      (gbRamSizeMask+1));

    if(read != (gbRamSizeMask+1)) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      gzclose(gzFile);
      gbBatteryError = true;
      return false;
    }

    // Also checks if the battery file it bigger than gbRamSizeMask+1 !
    u8 data[1];
    data[0] = 0;

    read = gzread(gzFile,
                  data,
                  1);
    if(read >0) {
      systemMessage(MSG_FAILED_TO_READ_SGM,
                    N_("Battery file's size incompatible with the rom settings %s (%d).\nWarning : save of the battery file is now disabled !"), name, read);
      gzclose(gzFile);
      gbBatteryError = true;
      return false;
    }

    gzclose(gzFile);
    return true;
  }
  else
    return false;
}

void gbInit()
{
  gbGenFilter();
  gbSgbInit();

  gbMemory = (u8 *)malloc(65536);

  pix = (u8 *)calloc(1,4*257*226);

  gbLineBuffer = (u16 *)malloc(160 * sizeof(u16));
}

bool gbWriteBatteryFile(const char *file, bool extendedSave)
{
  if(gbBattery) {
    switch(gbRomType) {
    case 0x03:
      gbWriteSaveMBC1(file);
      break;
    case 0x06:
      gbWriteSaveMBC2(file);
      break;
    case 0x0d:
      gbWriteSaveMMM01(file);
      break;
    case 0x0f:
    case 0x10:
      gbWriteSaveMBC3(file, extendedSave);
      break;
    case 0x13:
    case 0xfc:
      gbWriteSaveMBC3(file, false);
    case 0x1b:
    case 0x1e:
      gbWriteSaveMBC5(file);
      break;
    case 0x22:
      gbWriteSaveMBC7(file);
      break;
    case 0xfd:
      gbWriteSaveTAMA5(file, extendedSave);
      break;
    case 0xff:
      gbWriteSaveMBC1(file);
      break;
    }
  }
  return true;
}

bool gbWriteBatteryFile(const char *file)
{
  if (!gbBatteryError)
  {
    gbWriteBatteryFile(file, true);
    return true;
  }
  else return false;
}

bool gbReadBatteryFile(const char *file)
{
  bool res = false;
  if(gbBattery) {
    switch(gbRomType) {
    case 0x03:
      res = gbReadSaveMBC1(file);
      break;
    case 0x06:
      res = gbReadSaveMBC2(file);
      break;
    case 0x0d:
      res = gbReadSaveMMM01(file);
      break;
    case 0x0f:
    case 0x10:
      if(!gbReadSaveMBC3(file)) {
        time(&gbDataMBC3.mapperLastTime);
        struct tm *lt;
        lt = localtime(&gbDataMBC3.mapperLastTime);
        gbDataMBC3.mapperSeconds = lt->tm_sec;
        gbDataMBC3.mapperMinutes = lt->tm_min;
        gbDataMBC3.mapperHours = lt->tm_hour;
        gbDataMBC3.mapperDays = lt->tm_yday & 255;
        gbDataMBC3.mapperControl = (gbDataMBC3.mapperControl & 0xfe) |
          (lt->tm_yday > 255 ? 1: 0);
        res = false;
        break;
      }
      res = true;
      break;
    case 0x13:
    case 0xfc:
      res = gbReadSaveMBC3(file);
      break;
    case 0x1b:
    case 0x1e:
      res = gbReadSaveMBC5(file);
      break;
    case 0x22:
      res = gbReadSaveMBC7(file);
      break;
    case 0xfd:
      if(!gbReadSaveTAMA5(file)) {
        u8 gbDaysinMonth [12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        time(&gbDataTAMA5.mapperLastTime);
        struct tm *lt;
        lt = localtime(&gbDataTAMA5.mapperLastTime);
        gbDataTAMA5.mapperSeconds = lt->tm_sec;
        gbDataTAMA5.mapperMinutes = lt->tm_min;
        gbDataTAMA5.mapperHours = lt->tm_hour;
        gbDataTAMA5.mapperDays = 1;
        gbDataTAMA5.mapperMonths = 1;
        gbDataTAMA5.mapperYears = 1970;
        int days = lt->tm_yday+365*3;
        while (days)
        {
          gbDataTAMA5.mapperDays++;
          days--;
          if (gbDataTAMA5.mapperDays>gbDaysinMonth[gbDataTAMA5.mapperMonths-1])
          {
            gbDataTAMA5.mapperDays = 1;
            gbDataTAMA5.mapperMonths++;
            if (gbDataTAMA5.mapperMonths>12)
            {
               gbDataTAMA5.mapperMonths = 1;
               gbDataTAMA5.mapperYears++;
               if ((gbDataTAMA5.mapperYears & 3) == 0)
                 gbDaysinMonth[1] = 29;
               else
                 gbDaysinMonth[1] = 28;
            }
          }
        }
        gbDataTAMA5.mapperControl = (gbDataTAMA5.mapperControl & 0xfe) |
          (lt->tm_yday > 255 ? 1: 0);
        res = false;
        break;
      }
      res = true;
      break;
    case 0xff:
      res = gbReadSaveMBC1(file);
      break;
    }
  }
  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
  return res;
}

bool gbReadGSASnapshot(const char *fileName)
{
  FILE *file = fopen(fileName, "rb");

  if(!file) {
    systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
    return false;
  }

  fseek(file, 0x4, SEEK_SET);
  char buffer[16];
  char buffer2[16];
  fread(buffer, 1, 15, file);
  buffer[15] = 0;
  memcpy(buffer2, &gbRom[0x134], 15);
  buffer2[15] = 0;
  if(memcmp(buffer, buffer2, 15)) {
    systemMessage(MSG_CANNOT_IMPORT_SNAPSHOT_FOR,
                  N_("Cannot import snapshot for %s. Current game is %s"),
                  buffer,
                  buffer2);
    fclose(file);
    return false;
  }
  fseek(file, 0x13, SEEK_SET);
  size_t read = 0;
  int toRead = 0;
  switch(gbRomType) {
  case 0x03:
  case 0x0f:
  case 0x10:
  case 0x13:
  case 0x1b:
  case 0x1e:
  case 0xff:
    read = fread(gbRam, 1, (gbRamSizeMask+1), file);
    toRead = (gbRamSizeMask+1);
    break;
  case 0x06:
  case 0x22:
    read = fread(&gbMemory[0xa000],1,256,file);
    toRead = 256;
    break;
  default:
    systemMessage(MSG_UNSUPPORTED_SNAPSHOT_FILE,
                  N_("Unsupported snapshot file %s"),
                  fileName);
    fclose(file);
    return false;
  }
  fclose(file);
  gbReset();
  return true;
}

variable_desc gbSaveGameStruct[] = {
  { &PC.W, sizeof(u16) },
  { &SP.W, sizeof(u16) },
  { &AF.W, sizeof(u16) },
  { &BC.W, sizeof(u16) },
  { &DE.W, sizeof(u16) },
  { &HL.W, sizeof(u16) },
  { &IFF,  sizeof(u8) },
  { &GBLCD_MODE_0_CLOCK_TICKS, sizeof(int) },
  { &GBLCD_MODE_1_CLOCK_TICKS, sizeof(int) },
  { &GBLCD_MODE_2_CLOCK_TICKS, sizeof(int) },
  { &GBLCD_MODE_3_CLOCK_TICKS, sizeof(int) },
  { &GBDIV_CLOCK_TICKS, sizeof(int) },
  { &GBLY_INCREMENT_CLOCK_TICKS, sizeof(int) },
  { &GBTIMER_MODE_0_CLOCK_TICKS, sizeof(int) },
  { &GBTIMER_MODE_1_CLOCK_TICKS, sizeof(int) },
  { &GBTIMER_MODE_2_CLOCK_TICKS, sizeof(int) },
  { &GBTIMER_MODE_3_CLOCK_TICKS, sizeof(int) },
  { &GBSERIAL_CLOCK_TICKS, sizeof(int) },
  { &GBSYNCHRONIZE_CLOCK_TICKS, sizeof(int) },
  { &gbDivTicks, sizeof(int) },
  { &gbLcdMode, sizeof(int) },
  { &gbLcdTicks, sizeof(int) },
  { &gbLcdLYIncrementTicks, sizeof(int) },
  { &gbTimerTicks, sizeof(int) },
  { &gbTimerClockTicks, sizeof(int) },
  { &gbSerialTicks, sizeof(int) },
  { &gbSerialBits, sizeof(int) },
  { &gbInt48Signal, sizeof(int) },
  { &gbInterruptWait, sizeof(int) },
  { &gbSynchronizeTicks, sizeof(int) },
  { &gbTimerOn, sizeof(int) },
  { &gbTimerMode, sizeof(int) },
  { &gbSerialOn, sizeof(int) },
  { &gbWindowLine, sizeof(int) },
  { &gbCgbMode, sizeof(int) },
  { &gbVramBank, sizeof(int) },
  { &gbWramBank, sizeof(int) },
  { &gbHdmaSource, sizeof(int) },
  { &gbHdmaDestination, sizeof(int) },
  { &gbHdmaBytes, sizeof(int) },
  { &gbHdmaOn, sizeof(int) },
  { &gbSpeed, sizeof(int) },
  { &gbSgbMode, sizeof(int) },
  { &register_DIV, sizeof(u8) },
  { &register_TIMA, sizeof(u8) },
  { &register_TMA, sizeof(u8) },
  { &register_TAC, sizeof(u8) },
  { &register_IF, sizeof(u8) },
  { &register_LCDC, sizeof(u8) },
  { &register_STAT, sizeof(u8) },
  { &register_SCY, sizeof(u8) },
  { &register_SCX, sizeof(u8) },
  { &register_LY, sizeof(u8) },
  { &register_LYC, sizeof(u8) },
  { &register_DMA, sizeof(u8) },
  { &register_WY, sizeof(u8) },
  { &register_WX, sizeof(u8) },
  { &register_VBK, sizeof(u8) },
  { &register_HDMA1, sizeof(u8) },
  { &register_HDMA2, sizeof(u8) },
  { &register_HDMA3, sizeof(u8) },
  { &register_HDMA4, sizeof(u8) },
  { &register_HDMA5, sizeof(u8) },
  { &register_SVBK, sizeof(u8) },
  { &register_IE , sizeof(u8) },
  { &gbBgp[0], sizeof(u8) },
  { &gbBgp[1], sizeof(u8) },
  { &gbBgp[2], sizeof(u8) },
  { &gbBgp[3], sizeof(u8) },
  { &gbObp0[0], sizeof(u8) },
  { &gbObp0[1], sizeof(u8) },
  { &gbObp0[2], sizeof(u8) },
  { &gbObp0[3], sizeof(u8) },
  { &gbObp1[0], sizeof(u8) },
  { &gbObp1[1], sizeof(u8) },
  { &gbObp1[2], sizeof(u8) },
  { &gbObp1[3], sizeof(u8) },
  { NULL, 0 }
};


static bool gbWriteSaveState(gzFile gzFile)
{

  utilWriteInt(gzFile, GBSAVE_GAME_VERSION);

  utilGzWrite(gzFile, &gbRom[0x134], 15);

  utilWriteInt(gzFile, useBios);
  utilWriteInt(gzFile, inBios);

  utilWriteData(gzFile, gbSaveGameStruct);

  utilGzWrite(gzFile, &IFF, 2);

  if(gbSgbMode) {
    gbSgbSaveGame(gzFile);
  }

  utilGzWrite(gzFile, &gbDataMBC1, sizeof(gbDataMBC1));
  utilGzWrite(gzFile, &gbDataMBC2, sizeof(gbDataMBC2));
  utilGzWrite(gzFile, &gbDataMBC3, sizeof(gbDataMBC3));
  utilGzWrite(gzFile, &gbDataMBC5, sizeof(gbDataMBC5));
  utilGzWrite(gzFile, &gbDataHuC1, sizeof(gbDataHuC1));
  utilGzWrite(gzFile, &gbDataHuC3, sizeof(gbDataHuC3));
  utilGzWrite(gzFile, &gbDataTAMA5, sizeof(gbDataTAMA5));
  if (gbTAMA5ram != NULL)
    utilGzWrite(gzFile, gbTAMA5ram, gbTAMA5ramSize);
  utilGzWrite(gzFile, &gbDataMMM01, sizeof(gbDataMMM01));

  utilGzWrite(gzFile, gbPalette, 128 * sizeof(u16));

  utilGzWrite(gzFile, &gbMemory[0x8000], 0x8000);

  if(gbRamSize && gbRam) {
    utilWriteInt(gzFile, gbRamSize);
    utilGzWrite(gzFile, gbRam, gbRamSize);
  }

  if(gbCgbMode) {
    utilGzWrite(gzFile, gbVram, 0x4000);
    utilGzWrite(gzFile, gbWram, 0x8000);
  }

  gbSoundSaveGame(gzFile);

  gbCheatsSaveGame(gzFile);

  utilWriteInt(gzFile, gbLcdModeDelayed);
  utilWriteInt(gzFile, gbLcdTicksDelayed);
  utilWriteInt(gzFile, gbLcdLYIncrementTicksDelayed);
  utilWriteInt(gzFile, gbSpritesTicks[299]);
  utilWriteInt(gzFile, gbTimerModeChange);
  utilWriteInt(gzFile, gbTimerOnChange);
  utilWriteInt(gzFile, gbHardware);
  utilWriteInt(gzFile, gbBlackScreen);
  utilWriteInt(gzFile, oldRegister_WY);
  utilWriteInt(gzFile, gbWindowLine);
  utilWriteInt(gzFile, inUseRegister_WY);
  utilWriteInt(gzFile, gbScreenOn);
  utilWriteInt(gzFile, 0x12345678); // end marker
  return true;
}

bool gbWriteMemSaveState(char *memory, int available)
{
  gzFile gzFile = utilMemGzOpen(memory, available, "w");

  if(gzFile == NULL) {
    return false;
  }

  bool res = gbWriteSaveState(gzFile);

  long pos = utilGzMemTell(gzFile)+8;

  if(pos >= (available))
    res = false;

  utilGzClose(gzFile);

  return res;
}

bool gbWriteSaveState(const char *name)
{
  gzFile gzFile = utilGzOpen(name,"wb");

  if(gzFile == NULL)
    return false;

  bool res = gbWriteSaveState(gzFile);

  utilGzClose(gzFile);
  return res;
}

static bool gbReadSaveState(gzFile gzFile)
{
  int version = utilReadInt(gzFile);

  if(version > GBSAVE_GAME_VERSION || version < 0) {
    systemMessage(MSG_UNSUPPORTED_VB_SGM,
                  N_("Unsupported VisualBoy save game version %d"), version);
    return false;
  }

  u8 romname[20];

  utilGzRead(gzFile, romname, 15);

  if(memcmp(&gbRom[0x134], romname, 15) != 0) {
    systemMessage(MSG_CANNOT_LOAD_SGM_FOR,
                  N_("Cannot load save game for %s. Playing %s"),
                  romname, &gbRom[0x134]);
    return false;
  }


  bool ub = false;
  bool ib = false;

  if (version >= 11)
  {
    ub = utilReadInt(gzFile) ? true : false;
    ib = utilReadInt(gzFile) ? true : false;

    if((ub != useBios) && (ib)) {
      if(useBios)
        systemMessage(MSG_SAVE_GAME_NOT_USING_BIOS,
                      N_("Save game is not using the BIOS files"));
      else
        systemMessage(MSG_SAVE_GAME_USING_BIOS,
                      N_("Save game is using the BIOS file"));
      return false;
    }
  }

  gbReset();

  inBios = ib;

  utilReadData(gzFile, gbSaveGameStruct);


  // Correct crash when loading color gameboy save in regular gameboy type.
  if (!gbCgbMode)
  {
    if(gbVram != NULL) {
      free(gbVram);
      gbVram = NULL;
    }
    if(gbWram != NULL) {
      free(gbWram);
      gbWram = NULL;
    }
  }
  else
  {
    if(gbVram == NULL)
      gbVram = (u8 *)malloc(0x4000);
    if(gbWram == NULL)
      gbWram = (u8 *)malloc(0x8000);
    memset(gbVram,0,0x4000);
    memset(gbPalette,0, 2*128);
  }



  if(version >= GBSAVE_GAME_VERSION_7) {
    utilGzRead(gzFile, &IFF, 2);
  }

  if(gbSgbMode) {
    gbSgbReadGame(gzFile, version);
  } else {
    gbSgbMask = 0; // loading a game at the wrong time causes no display
  }
  if (version<11)
    utilGzRead(gzFile, &gbDataMBC1, sizeof(gbDataMBC1) - sizeof(int));
  else
    utilGzRead(gzFile, &gbDataMBC1, sizeof(gbDataMBC1));
  utilGzRead(gzFile, &gbDataMBC2, sizeof(gbDataMBC2));
  if(version < GBSAVE_GAME_VERSION_4)
    // prior to version 4, there was no adjustment for the time the game
    // was last played, so we have less to read. This needs update if the
    // structure changes again.
    utilGzRead(gzFile, &gbDataMBC3, sizeof(gbDataMBC3)-sizeof(time_t));
  else
    utilGzRead(gzFile, &gbDataMBC3, sizeof(gbDataMBC3));
  utilGzRead(gzFile, &gbDataMBC5, sizeof(gbDataMBC5));
  utilGzRead(gzFile, &gbDataHuC1, sizeof(gbDataHuC1));
  utilGzRead(gzFile, &gbDataHuC3, sizeof(gbDataHuC3));
  if(version>=11)
  {
    utilGzRead(gzFile, &gbDataTAMA5, sizeof(gbDataTAMA5));
    if(gbTAMA5ram != NULL) {
      if(skipSaveGameBattery) {
        utilGzSeek(gzFile, gbTAMA5ramSize, SEEK_CUR);
      } else {
        utilGzRead(gzFile, gbTAMA5ram, gbTAMA5ramSize);
      }
    }
    utilGzRead(gzFile, &gbDataMMM01, sizeof(gbDataMMM01));
  }

  if(version < GBSAVE_GAME_VERSION_5) {
    utilGzRead(gzFile, pix, 256*224*sizeof(u16));
  }
  memset(pix, 0, 257*226*sizeof(u32));

  if(version < GBSAVE_GAME_VERSION_6) {
    utilGzRead(gzFile, gbPalette, 64 * sizeof(u16));
  } else
    utilGzRead(gzFile, gbPalette, 128 * sizeof(u16));

  if (version < 11)
    utilGzRead(gzFile, gbPalette, 128 * sizeof(u16));

  if(version < GBSAVE_GAME_VERSION_10) {
    if(!gbCgbMode && !gbSgbMode) {
      for(int i = 0; i < 8; i++)
        gbPalette[i] = systemGbPalette[gbPaletteOption*8+i];
    }
  }

  utilGzRead(gzFile, &gbMemory[0x8000], 0x8000);

  if(gbRamSize && gbRam) {
    if(version < 11)
      if(skipSaveGameBattery) {
        utilGzSeek(gzFile, gbRamSize, SEEK_CUR); //skip
      } else {
        utilGzRead(gzFile, gbRam, gbRamSize); //read
      }
    else
    {
      int ramSize = utilReadInt(gzFile);
      if(skipSaveGameBattery) {
        utilGzSeek(gzFile, (gbRamSize>ramSize) ? ramSize : gbRamSize, SEEK_CUR); //skip
      } else {
        utilGzRead(gzFile, gbRam, (gbRamSize>ramSize) ? ramSize : gbRamSize); //read
      }
      if(ramSize>gbRamSize)
        utilGzSeek(gzFile,ramSize-gbRamSize,SEEK_CUR);
    }
  }

  memset(gbSCYLine, register_SCY, sizeof(gbSCYLine));
  memset(gbSCXLine, register_SCX, sizeof(gbSCXLine));
  memset(gbBgpLine, (gbBgp[0] | (gbBgp[1]<<2) | (gbBgp[2]<<4) |
         (gbBgp[3]<<6)), sizeof(gbBgpLine));
  memset(gbObp0Line, (gbObp0[0] | (gbObp0[1]<<2) | (gbObp0[2]<<4) |
         (gbObp0[3]<<6)), sizeof(gbObp0Line));
  memset(gbObp1Line, (gbObp1[0] | (gbObp1[1]<<2) | (gbObp1[2]<<4) |
         (gbObp1[3]<<6)), sizeof(gbObp1Line));
  memset(gbSpritesTicks, 0x0, sizeof(gbSpritesTicks));

  if (inBios)
  {
    gbMemoryMap[0x00] = &gbMemory[0x0000];
    memcpy ((u8 *)(gbMemory), (u8 *)(gbRom), 0x1000);
    memcpy ((u8 *)(gbMemory), (u8 *)(bios), 0x100);
  }
  else
    gbMemoryMap[0x00] = &gbRom[0x0000];
  gbMemoryMap[0x01] = &gbRom[0x1000];
  gbMemoryMap[0x02] = &gbRom[0x2000];
  gbMemoryMap[0x03] = &gbRom[0x3000];
  gbMemoryMap[0x04] = &gbRom[0x4000];
  gbMemoryMap[0x05] = &gbRom[0x5000];
  gbMemoryMap[0x06] = &gbRom[0x6000];
  gbMemoryMap[0x07] = &gbRom[0x7000];
  gbMemoryMap[0x08] = &gbMemory[0x8000];
  gbMemoryMap[0x09] = &gbMemory[0x9000];
  gbMemoryMap[0x0a] = &gbMemory[0xa000];
  gbMemoryMap[0x0b] = &gbMemory[0xb000];
  gbMemoryMap[0x0c] = &gbMemory[0xc000];
  gbMemoryMap[0x0d] = &gbMemory[0xd000];
  gbMemoryMap[0x0e] = &gbMemory[0xe000];
  gbMemoryMap[0x0f] = &gbMemory[0xf000];

  switch(gbRomType) {
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
    // MBC 1
    memoryUpdateMapMBC1();
    break;
  case 0x05:
  case 0x06:
    // MBC2
    memoryUpdateMapMBC2();
    break;
  case 0x0b:
  case 0x0c:
  case 0x0d:
    // MMM01
    memoryUpdateMapMMM01();
    break;
  case 0x0f:
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
    // MBC 3
    memoryUpdateMapMBC3();
    break;
  case 0x19:
  case 0x1a:
  case 0x1b:
    // MBC5
    memoryUpdateMapMBC5();
    break;
  case 0x1c:
  case 0x1d:
  case 0x1e:
    // MBC 5 Rumble
    memoryUpdateMapMBC5();
    break;
  case 0x22:
    // MBC 7
    memoryUpdateMapMBC7();
    break;
  case 0x56:
    // GS3
    memoryUpdateMapGS3();
    break;
  case 0xfd:
    // TAMA5
    memoryUpdateMapTAMA5();
    break;
  case 0xfe:
    // HuC3
    memoryUpdateMapHuC3();
    break;
  case 0xff:
    // HuC1
    memoryUpdateMapHuC1();
    break;
  }

  if(gbCgbMode) {
    utilGzRead(gzFile, gbVram, 0x4000);
    utilGzRead(gzFile, gbWram, 0x8000);

    int value = register_SVBK;
    if(value == 0)
      value = 1;

    gbMemoryMap[0x08] = &gbVram[register_VBK * 0x2000];
    gbMemoryMap[0x09] = &gbVram[register_VBK * 0x2000 + 0x1000];
    gbMemoryMap[0x0d] = &gbWram[value * 0x1000];
  }

  gbSoundReadGame(version, gzFile);

  if (gbCgbMode && gbSgbMode) {
    gbSgbMode = 0;
  }

  if(gbBorderOn && !gbSgbMask) {
    gbSgbRenderBorder();
  }

  systemDrawScreen();

  if(version > GBSAVE_GAME_VERSION_1)
  {
    if( skipSaveGameCheats ) {
      gbCheatsReadGameSkip(gzFile, version);
    } else {
      gbCheatsReadGame(gzFile, version);
    }
  }

  if (version<11)
  {
    gbWriteMemory(0xff00, 0);
    gbMemory[0xff04] = register_DIV;
    gbMemory[0xff05] = register_TIMA;
    gbMemory[0xff06] = register_TMA;
    gbMemory[0xff07] = register_TAC;
    gbMemory[0xff40] = register_LCDC;
    gbMemory[0xff42] = register_SCY;
    gbMemory[0xff43] = register_SCX;
    gbMemory[0xff44] = register_LY;
    gbMemory[0xff45] = register_LYC;
    gbMemory[0xff46] = register_DMA;
    gbMemory[0xff4a] = register_WY;
    gbMemory[0xff4b] = register_WX;
    gbMemory[0xff4f] = register_VBK;
    gbMemory[0xff51] = register_HDMA1;
    gbMemory[0xff52] = register_HDMA2;
    gbMemory[0xff53] = register_HDMA3;
    gbMemory[0xff54] = register_HDMA4;
    gbMemory[0xff55] = register_HDMA5;
    gbMemory[0xff70] = register_SVBK;
    gbMemory[0xffff] = register_IE;
    GBDIV_CLOCK_TICKS = 64;

    if (gbSpeed)
      gbDivTicks /=2;

    if ((gbLcdMode == 0) && (register_STAT & 8))
      gbInt48Signal |= 1;
    if ((gbLcdMode == 1) && (register_STAT & 0x10))
      gbInt48Signal |= 2;
    if ((gbLcdMode == 2) && (register_STAT & 0x20))
      gbInt48Signal |= 4;
    if ((register_LY==register_LYC) && (register_STAT & 0x40))
      gbInt48Signal |= 8;

      gbLcdLYIncrementTicks = GBLY_INCREMENT_CLOCK_TICKS;

      if (gbLcdMode == 2)
        gbLcdLYIncrementTicks-=GBLCD_MODE_2_CLOCK_TICKS-gbLcdTicks;
      else if (gbLcdMode == 3)
        gbLcdLYIncrementTicks -=GBLCD_MODE_2_CLOCK_TICKS+GBLCD_MODE_3_CLOCK_TICKS-gbLcdTicks;
      else if (gbLcdMode == 0)
        gbLcdLYIncrementTicks =gbLcdTicks;
      else if (gbLcdMode == 1)
      {
         gbLcdLYIncrementTicks = gbLcdTicks % GBLY_INCREMENT_CLOCK_TICKS;
         if (register_LY == 0x99)
            gbLcdLYIncrementTicks =gbLine99Ticks;
         else if (register_LY == 0)
            gbLcdLYIncrementTicks += GBLY_INCREMENT_CLOCK_TICKS;
      }

    gbLcdModeDelayed = gbLcdMode;
    gbLcdTicksDelayed = gbLcdTicks--;
    gbLcdLYIncrementTicksDelayed = gbLcdLYIncrementTicks--;
    gbInterruptWait = 0;
    memset(gbSpritesTicks,0,sizeof(gbSpritesTicks));
  }
  else
  {
    gbLcdModeDelayed = utilReadInt(gzFile);
    gbLcdTicksDelayed = utilReadInt(gzFile);
    gbLcdLYIncrementTicksDelayed = utilReadInt(gzFile);
    gbSpritesTicks[299] = utilReadInt(gzFile) & 0xff;
    gbTimerModeChange = (utilReadInt(gzFile) ? true : false);
    gbTimerOnChange = (utilReadInt(gzFile) ? true : false);
    gbHardware = utilReadInt(gzFile);
    gbBlackScreen = (utilReadInt(gzFile) ? true : false);
    oldRegister_WY = utilReadInt(gzFile);
    gbWindowLine = utilReadInt(gzFile);
    inUseRegister_WY = utilReadInt(gzFile);
    gbScreenOn = (utilReadInt(gzFile) ? true : false);
  }

  if (gbSpeed)
    gbLine99Ticks *= 2;

  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

  if ( version >= 12 && utilReadInt( gzFile ) != 0x12345678 )
    assert( false ); // fails if something read too much/little from file

  return true;
}

bool gbReadMemSaveState(char *memory, int available)
{
  gzFile gzFile = utilMemGzOpen(memory, available, "r");

  bool res = gbReadSaveState(gzFile);

  utilGzClose(gzFile);

  return res;
}

bool gbReadSaveState(const char *name)
{
  gzFile gzFile = utilGzOpen(name,"rb");

  if(gzFile == NULL) {
    return false;
  }

  bool res = gbReadSaveState(gzFile);

  utilGzClose(gzFile);

  return res;
}

bool gbWritePNGFile(const char *fileName)
{
  if(gbBorderOn)
    return utilWritePNGFile(fileName, 256, 224, pix);
  return utilWritePNGFile(fileName, 160, 144, pix);
}

bool gbWriteBMPFile(const char *fileName)
{
  if(gbBorderOn)
    return utilWriteBMPFile(fileName, 256, 224, pix);
  return utilWriteBMPFile(fileName, 160, 144, pix);
}

void gbCleanUp()
{
  if(gbRam != NULL) {
    free(gbRam);
    gbRam = NULL;
  }

  if(gbRom != NULL) {
    free(gbRom);
    gbRom = NULL;
  }

  if(bios != NULL) {
    free(bios);
    bios = NULL;
  }

  if(gbMemory != NULL) {
    free(gbMemory);
    gbMemory = NULL;
  }

  if(gbLineBuffer != NULL) {
    free(gbLineBuffer);
    gbLineBuffer = NULL;
  }

  if(pix != NULL) {
    free(pix);
    pix = NULL;
  }

  gbSgbShutdown();

  if(gbVram != NULL) {
    free(gbVram);
    gbVram = NULL;
  }

  if(gbWram != NULL) {
    free(gbWram);
    gbWram = NULL;
  }

  if(gbTAMA5ram != NULL) {
    free(gbTAMA5ram);
    gbTAMA5ram = NULL;
  }

  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
}

bool gbLoadRom(const char *szFile)
{
  int size = 0;

  if(gbRom != NULL) {
    gbCleanUp();
  }

  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

  gbRom = utilLoad(szFile,
                   utilIsGBImage,
                   NULL,
                   size);
  if(!gbRom)
    return false;

  gbRomSize = size;

  gbBatteryError = false;

  if(bios != NULL) {
    free(bios);
    bios = NULL;
  }
  bios = (u8 *)calloc(1,0x100);

  return gbUpdateSizes();
}

bool gbUpdateSizes()
{
  if(gbRom[0x148] > 8) {
    systemMessage(MSG_UNSUPPORTED_ROM_SIZE,
                  N_("Unsupported rom size %02x"), gbRom[0x148]);
    return false;
  }

  if(gbRomSize < gbRomSizes[gbRom[0x148]]) {
    gbRom = (u8 *)realloc(gbRom, gbRomSizes[gbRom[0x148]]);
    for (int i = gbRomSize; i<gbRomSizes[gbRom[0x148]]; i++)
        gbRom[i] = 0x00; // Not sure if it's 0x00, 0xff or random data...
  }
  // (it's in the case a cart is 'lying' on its size.
  else if ((gbRomSize>gbRomSizes[gbRom[0x148]]) && (genericflashcardEnable))
  {
    gbRomSize = gbRomSize>>16;
    gbRom[0x148] = 0;
    if (gbRomSize)
    {
      while (!((gbRomSize & 1) || (gbRom[0x148] == 7)))
      {
        gbRom[0x148]++;
        gbRomSize>>=1;
      }
      gbRom[0x148]++;
    }
    gbRom = (u8 *)realloc(gbRom, gbRomSizes[gbRom[0x148]]);
  }
  gbRomSize = gbRomSizes[gbRom[0x148]];
  gbRomSizeMask = gbRomSizesMasks[gbRom[0x148]];


  // The 'genericflashcard' option allows some PD to work.
  // However, the setting is dangerous (if you let in enabled
  // and play a normal game, it might just break everything).
  // That's why it is not saved in the emulator options.
  // Also I added some checks in VBA to make sure your saves will not be
  // overwritten if you wrongly enable this option for a game
  // you already played (and vice-versa, ie. if you forgot to
  // enable the option for a game you played with it enabled, like Shawu Story).
  u8 ramsize = genericflashcardEnable ? 5 : gbRom[0x149];
  gbRom[0x149] = ramsize;

  if ((gbRom[2] == 0x6D) && (gbRom[5] == 0x47) && (gbRom[6] == 0x65) && (gbRom[7] == 0x6E) &&
      (gbRom[8] == 0x69) && (gbRom[9] == 0x65) && (gbRom[0xA] == 0x28) && (gbRom[0xB] == 0x54))
  {
    gbCheatingDevice = 1; // GameGenie
    for (int i = 0; i < 0x20; i++) // Cleans GG hardware registers
      gbRom[0x4000+i] = 0;
  }
  else if (((gbRom[0x104] == 0x44) && (gbRom[0x156] == 0xEA) && (gbRom[0x158] == 0x7F) &&
            (gbRom[0x159] == 0xEA) && (gbRom[0x15B] == 0x7F)) || ((gbRom[0x165] == 0x3E) &&
            (gbRom[0x166] == 0xD9) && (gbRom[0x16D] == 0xE1) && (gbRom[0x16E] == 0x7F)))
    gbCheatingDevice = 2; // GameShark
  else gbCheatingDevice = 0;

  if(ramsize > 5) {
    systemMessage(MSG_UNSUPPORTED_RAM_SIZE,
                  N_("Unsupported ram size %02x"), gbRom[0x149]);
    return false;
  }

  gbRamSize = gbRamSizes[ramsize];
  gbRamSizeMask = gbRamSizesMasks[ramsize];

  gbRomType = gbRom[0x147];
  if (genericflashcardEnable)
  {
    /*if (gbRomType<2)
      gbRomType =3;
    else if ((gbRomType == 0xc) || (gbRomType == 0xf) || (gbRomType == 0x12) ||
             (gbRomType == 0x16) || (gbRomType == 0x1a) || (gbRomType == 0x1d))
      gbRomType++;
    else if ((gbRomType == 0xb) || (gbRomType == 0x11) || (gbRomType == 0x15) ||
             (gbRomType == 0x19) || (gbRomType == 0x1c))
      gbRomType+=2;
    else if ((gbRomType == 0x5) || (gbRomType == 0x6))
      gbRomType = 0x1a;*/
      gbRomType = 0x1b;
  }
  else if (gbCheatingDevice == 1)
    gbRomType = 0x55;
  else  if (gbCheatingDevice == 2)
    gbRomType = 0x56;

  gbRom[0x147] = gbRomType;

  mapperReadRAM = NULL;

  switch(gbRomType) {
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x08:
  case 0x09:
    // MBC 1
    mapper = mapperMBC1ROM;
    mapperRAM = mapperMBC1RAM;
    mapperReadRAM = mapperMBC1ReadRAM;
    break;
  case 0x05:
  case 0x06:
    // MBC2
    mapper = mapperMBC2ROM;
    mapperRAM = mapperMBC2RAM;
    gbRamSize = 0x200;
    gbRamSizeMask = 0x1ff;
    break;
  case 0x0b:
  case 0x0c:
  case 0x0d:
    // MMM01
    mapper = mapperMMM01ROM;
    mapperRAM = mapperMMM01RAM;
    break;
  case 0x0f:
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
  case 0xfc:
    // MBC 3
    mapper = mapperMBC3ROM;
    mapperRAM = mapperMBC3RAM;
    mapperReadRAM = mapperMBC3ReadRAM;
    break;
  case 0x19:
  case 0x1a:
  case 0x1b:
    // MBC5
    mapper = mapperMBC5ROM;
    mapperRAM = mapperMBC5RAM;
    mapperReadRAM = mapperMBC5ReadRAM;
    break;
  case 0x1c:
  case 0x1d:
  case 0x1e:
    // MBC 5 Rumble
    mapper = mapperMBC5ROM;
    mapperRAM = mapperMBC5RAM;
    mapperReadRAM = mapperMBC5ReadRAM;
    break;
  case 0x22:
    // MBC 7
    mapper = mapperMBC7ROM;
    mapperRAM = mapperMBC7RAM;
    mapperReadRAM = mapperMBC7ReadRAM;
    gbRamSize = 0x200;
    gbRamSizeMask = 0x1ff;
    break;
    // GG (GameGenie)
  case 0x55:
    mapper = mapperGGROM;
    break;
  case 0x56:
    // GS (GameShark)
    mapper = mapperGS3ROM;
    break;
  case 0xfd:
    // TAMA5
    if (gbRam!= NULL)
    {
      free(gbRam);
      gbRam = NULL;
    }

    ramsize = 3;
    gbRamSize = gbRamSizes[3];
    gbRamSizeMask = gbRamSizesMasks[3];
    gbRamFill = 0x0;

    gbTAMA5ramSize = 0x100;

    if (gbTAMA5ram == NULL)
      gbTAMA5ram = (u8 *)malloc(gbTAMA5ramSize);
    memset(gbTAMA5ram, 0x0, gbTAMA5ramSize);

    mapperRAM = mapperTAMA5RAM;
    mapperReadRAM = mapperTAMA5ReadRAM;
    mapperUpdateClock = memoryUpdateTAMA5Clock;
    break;
  case 0xfe:
    // HuC3
    mapper = mapperHuC3ROM;
    mapperRAM = mapperHuC3RAM;
    mapperReadRAM = mapperHuC3ReadRAM;
    break;
  case 0xff:
    // HuC1
    mapper = mapperHuC1ROM;
    mapperRAM = mapperHuC1RAM;
    break;
  default:
    systemMessage(MSG_UNKNOWN_CARTRIDGE_TYPE,
                  N_("Unknown cartridge type %02x"), gbRomType);
    return false;
  }

  if(gbRamSize) {
    gbRam = (u8 *)malloc(gbRamSize);
    memset(gbRam, gbRamFill, gbRamSize);
  }

  switch(gbRomType) {
  case 0x03:
  case 0x06:
  case 0x0f:
  case 0x10:
  case 0x13:
  case 0x1b:
  case 0x1d:
  case 0x1e:
  case 0x22:
  case 0xfd:
  case 0xff:
    gbBattery = 1;
    break;
  }

  gbInit();

  //gbReset();

  switch(gbRomType) {
  case 0x1c:
  case 0x1d:
  case 0x1e:
    gbDataMBC5.isRumbleCartridge = 1;
  }

  return true;
}

int gbGetNextEvent (int clockTicks)
{
  if (register_LCDC & 0x80)
  {
    if(gbLcdTicks < clockTicks)
        clockTicks = gbLcdTicks;

    if(gbLcdTicksDelayed < clockTicks)
      clockTicks = gbLcdTicksDelayed;

    if(gbLcdLYIncrementTicksDelayed < clockTicks)
      clockTicks = gbLcdLYIncrementTicksDelayed;
  }

  if(gbLcdLYIncrementTicks < clockTicks)
    clockTicks = gbLcdLYIncrementTicks;

  if(gbSerialOn && (gbSerialTicks < clockTicks))
    clockTicks = gbSerialTicks;

  if(gbTimerOn && (((gbInternalTimer) & gbTimerMask[gbTimerMode])+1 < clockTicks))
    clockTicks = ((gbInternalTimer) & gbTimerMask[gbTimerMode])+1;

  //if(soundTicks && (soundTicks < clockTicks))
  //  clockTicks = soundTicks;

  if ((clockTicks<=0) || (gbInterruptWait))
      clockTicks = 1;

  return clockTicks;
}

void gbDrawLine()
{
  switch(systemColorDepth) {
    case 16:
    {
      u16 * dest = (u16 *)pix +
                   (gbBorderLineSkip+2) * (register_LY + gbBorderRowSkip+1)
                   + gbBorderColumnSkip;
      for(int x = 0; x < 160; ) {
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];

        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];

        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];

        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
        *dest++ = systemColorMap16[gbLineMix[x++]];
      }
      if(gbBorderOn)
        dest += gbBorderColumnSkip;
        *dest++ = 0; // for filters that read one pixel more
    }
    break;

    case 24:
    {
      u8 *dest = (u8 *)pix +
                 3*(gbBorderLineSkip * (register_LY + gbBorderRowSkip) +
                 gbBorderColumnSkip);
      for(int x = 0; x < 160;) {
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;

        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;

        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;

        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
        *((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
        dest+= 3;
      }
    }
    break;

    case 32:
    {
      u32 * dest = (u32 *)pix +
                   (gbBorderLineSkip+1) * (register_LY + gbBorderRowSkip+1)
                   + gbBorderColumnSkip;
      for(int x = 0; x < 160;) {
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];

        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];

        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];

        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
        *dest++ = systemColorMap32[gbLineMix[x++]];
      }
    }
    break;
  }
}

void gbEmulate(int ticksToStop)
{
  gbRegister tempRegister;
  u8 tempValue;
  s8 offset;

  clockTicks = 0;
  gbDmaTicks = 0;

  register int opcode = 0;

  int opcode1 = 0;
  int opcode2 = 0;
  bool execute = false;

  while(1) {
#ifndef FINAL_VERSION
    if(systemDebug) {
      if(!(IFF & 0x80)) {
        if(systemDebug > 1) {
          sprintf(gbBuffer,"PC=%04x AF=%04x BC=%04x DE=%04x HL=%04x SP=%04x I=%04x\n",
                   PC.W, AF.W, BC.W, DE.W,HL.W,SP.W,IFF);
        } else {
          sprintf(gbBuffer,"PC=%04x I=%02x\n", PC.W, IFF);
        }
        log(gbBuffer);
      }
    }
#endif

    u16 oldPCW = PC.W;

    if(IFF & 0x80) {
      if(register_LCDC & 0x80) {
          clockTicks = gbLcdTicks;
      } else
        clockTicks = 1000;

      clockTicks = gbGetNextEvent(clockTicks);

      /*if(gbLcdTicksDelayed < clockTicks)
        clockTicks = gbLcdTicksDelayed;

      if(gbLcdLYIncrementTicksDelayed < clockTicks)
        clockTicks = gbLcdLYIncrementTicksDelayed;

      if(gbLcdLYIncrementTicks < clockTicks)
        clockTicks = gbLcdLYIncrementTicks;

      if(gbSerialOn && (gbSerialTicks < clockTicks))
        clockTicks = gbSerialTicks;

      if(gbTimerOn && (((gbInternalTimer) & gbTimerMask[gbTimerMode])+1 < clockTicks))
        clockTicks = ((gbInternalTimer) & gbTimerMask[gbTimerMode])+1;

      if(soundTicks && (soundTicks < clockTicks))
        clockTicks = soundTicks;

      if ((clockTicks<=0) || (gbInterruptWait))
          clockTicks = 1;*/

    } else {

      // First we apply the clockTicks, then we execute the opcodes.
      opcode1 = 0;
      opcode2 = 0;
      execute = true;

      opcode2 = opcode1 = opcode = gbReadOpcode(PC.W++);

      // If HALT state was launched while IME = 0 and (register_IF & register_IE & 0x1F),
      // PC.W is not incremented for the first byte of the next instruction.
      if (IFF & 2)
      {
        PC.W--;
        IFF &= ~2;
      }

      clockTicks = gbCycles[opcode];

      switch(opcode) {
      case 0xCB:
        // extended opcode
        opcode2 = opcode = gbReadOpcode(PC.W++);
        clockTicks = gbCyclesCB[opcode];
        break;
      }
      gbOldClockTicks = clockTicks-1;
      gbIntBreak = 1;
    }


    if(!emulating)
      return;

    // For 'breakpoint' support (opcode 0xFC is considered as a breakpoint)
    if ((clockTicks==0) && execute)
    {
      PC.W = oldPCW;
      return;
    }


    if (!(IFF & 0x80))
      clockTicks = 1;

    gbRedoLoop:



    if (gbInterruptWait)
      gbInterruptWait = 0;
    else
      gbInterruptLaunched = 0;


    // Used for the EI/DI instruction's delay.
    if (IFF & 0x38)
    {
        int tempIFF = (IFF >> 4) & 3;

        if (tempIFF <=clockTicks)
        {
            tempIFF = 0;
            IFF |=1;
        }
        else
          tempIFF -= clockTicks;
        IFF = (IFF & 0xCF) | (tempIFF <<4);

        if (IFF & 0x08)
          IFF &= 0x82;
    }


    if (register_LCDCBusy)
    {
      register_LCDCBusy-=clockTicks;
      if (register_LCDCBusy<0)
        register_LCDCBusy = 0;
    }


    if(gbSgbMode) {
      if(gbSgbPacketTimeout) {
        gbSgbPacketTimeout -= clockTicks;

        if(gbSgbPacketTimeout <= 0)
          gbSgbResetPacketState();
      }
    }

    ticksToStop -= clockTicks;

    // DIV register emulation
    gbDivTicks -= clockTicks;
    while(gbDivTicks <= 0) {
      gbMemory[0xff04] = ++register_DIV;
      gbDivTicks += GBDIV_CLOCK_TICKS;
    }

    if(register_LCDC & 0x80) {
      // LCD stuff

      gbLcdTicks -= clockTicks;
      gbLcdTicksDelayed -= clockTicks;
      gbLcdLYIncrementTicks -= clockTicks;
      gbLcdLYIncrementTicksDelayed -= clockTicks;


      // our counters are off, see what we need to do

      // This looks (and kinda is) complicated, however this
      // is the only way I found to emulate properly the way
      // the real hardware operates...
      while(((gbLcdTicks <= 0) && (gbLCDChangeHappened == false)) ||
            ((gbLcdTicksDelayed <= 0) && (gbLCDChangeHappened == true)) ||
            ((gbLcdLYIncrementTicks <= 0) && (gbLYChangeHappened == false)) ||
            ((gbLcdLYIncrementTicksDelayed<=0) && (gbLYChangeHappened == true)))
      {

        if ((gbLcdLYIncrementTicks <= 0) && (!gbLYChangeHappened))
        {
          gbLYChangeHappened = true;
          gbMemory[0xff44] = register_LY = (register_LY + 1) % 154;

          if (register_LY == 0x91)
          {
           /* if (IFF & 0x80)
              gbScreenOn = !gbScreenOn;
            else*/ if (register_LCDC & 0x80)
              gbScreenOn = true;
          }

          gbLcdLYIncrementTicks += GBLY_INCREMENT_CLOCK_TICKS;

          if (gbLcdMode == 1)
          {

            if(register_LY == 153)
              gbLcdLYIncrementTicks -= GBLY_INCREMENT_CLOCK_TICKS - gbLine99Ticks;
            else if(register_LY == 0)
              gbLcdLYIncrementTicks += GBLY_INCREMENT_CLOCK_TICKS - gbLine99Ticks;
          }

          // GB only 'bug' : Halt state is broken one tick before LY==LYC interrupt
          // is reflected in the registers.
          if ((gbHardware & 5) && (IFF & 0x80) && (register_LY == register_LYC)
              && (register_STAT & 0x40) && (register_LY != 0))
          {
            if (!((gbLcdModeDelayed != 1) && (register_LY==0)))
            {
              gbInt48Signal &= ~9;
              gbCompareLYToLYC();
              gbLYChangeHappened = false;
              gbMemory[0xff41] = register_STAT;
              gbMemory[0xff0f] = register_IF;
            }

            gbLcdLYIncrementTicksDelayed += GBLY_INCREMENT_CLOCK_TICKS - gbLine99Ticks+1;
          }
        }


        if ((gbLcdTicks <= 0) && (!gbLCDChangeHappened))
        {
          gbLCDChangeHappened = true;

          switch(gbLcdMode)
          {
            case 0:
            {
              // H-Blank
              // check if we reached the V-Blank period
              if(register_LY == 144) {
                // Yes, V-Blank
                // set the LY increment counter
                if (gbHardware & 0x5)
                {
                  register_IF |= 1; // V-Blank interrupt
                }

                  gbInt48Signal &= ~6;
                  if(register_STAT & 0x10)
                  {
                    // send LCD interrupt only if no interrupt 48h signal...
                    if ((!(gbInt48Signal & 1)) && ((!(gbInt48Signal & 8)) || (gbHardware & 0x0a)))
                    {
                      register_IF |=2;
                      gbInterruptLaunched |= 2;
                      if (gbHardware & 0xa)
                        gbInterruptWait = 1;
                    }
                    gbInt48Signal |= 2;
                  }
                  gbInt48Signal &= ~1;

                gbLcdTicks += GBLCD_MODE_1_CLOCK_TICKS;
                gbLcdMode = 1;

              } else {
                // go the the OAM being accessed mode
                gbLcdTicks += GBLCD_MODE_2_CLOCK_TICKS;
                gbLcdMode = 2;

                gbInt48Signal &= ~6;
                if(register_STAT & 0x20)
                {
                  // send LCD interrupt only if no interrupt 48h signal...
                  if (!gbInt48Signal)
                  {
                    register_IF |= 2;
                    gbInterruptLaunched |= 2;
                  }
                  gbInt48Signal |= 4;
                }
                gbInt48Signal &= ~1;
              }
            }
            break;
            case 1:
            {
              // V-Blank
              // next mode is OAM being accessed mode
              gbInt48Signal &= ~5;
              if(register_STAT & 0x20)
              {
                // send LCD interrupt only if no interrupt 48h signal...
                if (!gbInt48Signal)
                {
                  register_IF |= 2;
                  gbInterruptLaunched |= 2;
                  if ((gbHardware & 0xa) && (IFF & 0x80))
                    gbInterruptWait = 1;
                }
                gbInt48Signal |= 4;
              }
              gbInt48Signal &= ~2;

              gbLcdTicks += GBLCD_MODE_2_CLOCK_TICKS;

              gbLcdMode = 2;
              register_LY = 0x00;

            }
            break;
            case 2:
            {

              // OAM being accessed mode
              // next mode is OAM and VRAM in use
              if ((gbScreenOn) && (register_LCDC & 0x80))
              {
                gbDrawSprites(false);
                // Used to add a one tick delay when a window line is drawn.
                //(fixes a part of Carmaggedon problem)
                if((register_LCDC & 0x01 || gbCgbMode) && (register_LCDC & 0x20) &&
                   (gbWindowLine != -2)) {

                  int inUseRegister_WY = 0;
                  int tempgbWindowLine = gbWindowLine;

                  if ((tempgbWindowLine == -1) || (tempgbWindowLine>144))
                  {
                    inUseRegister_WY = oldRegister_WY;
                    if (register_LY>oldRegister_WY)
                    tempgbWindowLine = 146;
                  }

                  if(register_LY >= inUseRegister_WY) {

                    if (tempgbWindowLine == -1)
                      tempgbWindowLine = 0;

                    int wx = register_WX;
                    wx -= 7;
                    if (wx<0)
                        wx = 0;

                    if((wx <= 159) && (tempgbWindowLine <= 143))
                      for (int i = wx; i<300; i++)
                        if (gbSpeed)
                          gbSpritesTicks[i]+=3;
                        else
                          gbSpritesTicks[i]+=1;
                  }
                }
              }

              gbInt48Signal &= ~7;

              gbLcdTicks += GBLCD_MODE_3_CLOCK_TICKS+gbSpritesTicks[299];
              gbLcdMode = 3;
            }
            break;
            case 3:
            {
              // OAM and VRAM in use
              // next mode is H-Blank


              gbInt48Signal &= ~7;
              if(register_STAT & 0x08)
              {
                // send LCD interrupt only if no interrupt 48h signal...
                if (!(gbInt48Signal & 8))
                {
                  register_IF |= 2;
                if ((gbHardware & 0xa) && (IFF & 0x80))
                  gbInterruptWait = 1;
                }
                gbInt48Signal |= 1;
              }

              gbLcdTicks += GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299];

              gbLcdMode = 0;

              // No HDMA during HALT !
              if(gbHdmaOn && (!(IFF & 0x80) || (register_IE & register_IF & 0x1f))) {
                gbDoHdma();
              }

            }
            break;
          }
        }


        if ((gbLcdTicksDelayed <= 0) && (gbLCDChangeHappened)) {
          int framesToSkip = systemFrameSkip;
          if(speedup)
            framesToSkip = 9; // try 6 FPS during speedup
          //gbLcdTicksDelayed = gbLcdTicks+1;
          gbLCDChangeHappened = false;
          switch(gbLcdModeDelayed) {
            case 0:
            {
              // H-Blank

              memset(gbSCYLine,gbSCYLine[299],sizeof(gbSCYLine));
              memset(gbSCXLine,gbSCXLine[299],sizeof(gbSCXLine));
              memset(gbBgpLine,gbBgpLine[299],sizeof(gbBgpLine));
              memset(gbObp0Line,gbObp0Line[299],sizeof(gbObp0Line));
              memset(gbObp1Line,gbObp1Line[299],sizeof(gbObp1Line));
              memset(gbSpritesTicks,gbSpritesTicks[299],sizeof(gbSpritesTicks));

              if (gbWindowLine <0)
                oldRegister_WY = register_WY;
              // check if we reached the V-Blank period
              if(register_LY == 144) {
                // Yes, V-Blank
                // set the LY increment counter

                if(register_LCDC & 0x80) {
                    if (gbHardware & 0xa)
                    {

                    register_IF |= 1; // V-Blank interrupt
                    gbInterruptLaunched |=1;
                    }


                }

                gbLcdTicksDelayed += GBLCD_MODE_1_CLOCK_TICKS;
                gbLcdModeDelayed = 1;

                gbFrameCount++;
                systemFrame();

                if((gbFrameCount % 10) == 0)
                  system10Frames(60);

                if(gbFrameCount >= 60) {
                  u32 currentTime = systemGetClock();
                  if(currentTime != gbLastTime)
                    systemShowSpeed(100000/(currentTime - gbLastTime));
                  else
                    systemShowSpeed(0);
                  gbLastTime = currentTime;
                  gbFrameCount = 0;
                }

                if(systemReadJoypads()) {
                  // read joystick
                  if(gbSgbMode && gbSgbMultiplayer) {
                    if(gbSgbFourPlayers) {
                      gbJoymask[0] = systemReadJoypad(0);
                      gbJoymask[1] = systemReadJoypad(1);
                      gbJoymask[2] = systemReadJoypad(2);
                      gbJoymask[3] = systemReadJoypad(3);
                    } else {
                      gbJoymask[0] = systemReadJoypad(0);
                      gbJoymask[1] = systemReadJoypad(1);
                    }
                  } else {
                    gbJoymask[0] = systemReadJoypad(-1);
                  }
                }
                int newmask = gbJoymask[0] & 255;

                if(gbRomType == 0x22) {
                  systemUpdateMotionSensor();
                }

                if(newmask)
                {
                  gbMemory[0xff0f] = register_IF |= 16;
                }


                newmask = (gbJoymask[0] >> 10);

                speedup = (newmask & 1) ? true : false;
                gbCapture = (newmask & 2) ? true : false;

                if(gbCapture && !gbCapturePrevious) {
                  gbCaptureNumber++;
                  systemScreenCapture(gbCaptureNumber);
                }
                gbCapturePrevious = gbCapture;

          if(gbFrameSkipCount >= framesToSkip) {

            if(!gbSgbMask)
            {
              if (gbBorderOn)
                gbSgbRenderBorder();
              //if (gbScreenOn)
                systemDrawScreen();
            }
            gbFrameSkipCount = 0;
          } else
             gbFrameSkipCount++;

              } else {
                // go the the OAM being accessed mode
                gbLcdTicksDelayed += GBLCD_MODE_2_CLOCK_TICKS;
                gbLcdModeDelayed = 2;
                gbInt48Signal &= ~3;
              }
            }
            break;
            case 1:
            {
              // V-Blank
              // next mode is OAM being accessed mode

             // gbScreenOn = true;

              oldRegister_WY = register_WY;

              gbLcdTicksDelayed += GBLCD_MODE_2_CLOCK_TICKS;
              gbLcdModeDelayed = 2;

              // reset the window line
              gbWindowLine = -1;
            }
            break;
            case 2:
            {
              // OAM being accessed mode
              // next mode is OAM and VRAM in use
              gbLcdTicksDelayed += GBLCD_MODE_3_CLOCK_TICKS+gbSpritesTicks[299];
              gbLcdModeDelayed = 3;
            }
            break;
            case 3:
            {

              // OAM and VRAM in use
              // next mode is H-Blank
              if((register_LY < 144) && (register_LCDC & 0x80) && gbScreenOn) {
                if(!gbSgbMask) {
                  if(gbFrameSkipCount >= framesToSkip) {
                    if (!gbBlackScreen)
                    {
                      gbRenderLine();
                      gbDrawSprites(true);
                    }
                    else if (gbBlackScreen)
                    {
                      u16 color = gbColorOption ? gbColorFilter[0] : 0;
                      if (!gbCgbMode)
                      color = gbColorOption ? gbColorFilter[gbPalette[3] & 0x7FFF] :
                        gbPalette[3] & 0x7FFF;
                      for(int i = 0; i < 160; i++)
                      {
                        gbLineMix[i] = color;
                        gbLineBuffer[i] = 0;
                      }
                    }
                    gbDrawLine();
                  }
                }
              }
              gbLcdTicksDelayed += GBLCD_MODE_0_CLOCK_TICKS-gbSpritesTicks[299];
              gbLcdModeDelayed = 0;
            }
            break;
          }
        }

        if ((gbLcdLYIncrementTicksDelayed <= 0) && (gbLYChangeHappened == true))
        {

          gbLYChangeHappened = false;

          if (!((gbLcdMode != 1) && (register_LY==0)))
          {
            {
              gbInt48Signal &= ~8;
              gbCompareLYToLYC();
              if ((gbInt48Signal == 8) && (!((register_LY == 0) && (gbHardware & 1))))
                gbInterruptLaunched |= 2;
              if ((gbHardware & (gbSpeed ? 8 : 2)) && (register_LY==0) && ((register_STAT & 0x44) == 0x44) && (gbLcdLYIncrementTicksDelayed==0))
              {
                 gbInterruptWait = 1;

              }
            }
          }
          gbLcdLYIncrementTicksDelayed += GBLY_INCREMENT_CLOCK_TICKS;

          if (gbLcdModeDelayed == 1)
          {

            if(register_LY == 153)
              gbLcdLYIncrementTicksDelayed -= GBLY_INCREMENT_CLOCK_TICKS - gbLine99Ticks;
            else if(register_LY == 0)
              gbLcdLYIncrementTicksDelayed += GBLY_INCREMENT_CLOCK_TICKS - gbLine99Ticks;
          }
          gbMemory[0xff0f] = register_IF;
          gbMemory[0xff41] = register_STAT;
        }
      }
      gbMemory[0xff0f] = register_IF;
      gbMemory[0xff41] = register_STAT = (register_STAT & 0xfc) | gbLcdModeDelayed;
    }
    else
    {

      // Used to update the screen with white lines when it's off.
      // (it looks strange, but it's kinda accurate :p)
      // You can try the Mario Demo Vx.x for exemple
      // (check the bottom 2 lines while moving)
     if (!gbWhiteScreen)
      {
        gbScreenTicks -= clockTicks;
        gbLcdLYIncrementTicks -= clockTicks;
        while (gbLcdLYIncrementTicks <=0)
        {
          register_LY = ((register_LY+1)%154);
          gbLcdLYIncrementTicks+=GBLY_INCREMENT_CLOCK_TICKS;
        }
        if (gbScreenTicks <= 0)
        {
          gbWhiteScreen = 1;
          u8 register_LYLcdOff = ((register_LY+154)%154);
          for (register_LY=0;register_LY <=  0x90;register_LY++)
          {
            u16 color = gbColorOption ? gbColorFilter[0x7FFF] :
                        0x7FFF;
            if (!gbCgbMode)
            color = gbColorOption ? gbColorFilter[gbPalette[0] & 0x7FFF] :
                        gbPalette[0] & 0x7FFF;
            for(int i = 0; i < 160; i++)
            {
              gbLineMix[i] = color;
              gbLineBuffer[i] = 0;
            }
            gbDrawLine();
          }
          register_LY = register_LYLcdOff;
        }
      }

      if (gbWhiteScreen)
      {
        gbLcdLYIncrementTicks -= clockTicks;

        while (gbLcdLYIncrementTicks <=0)
        {
          register_LY = ((register_LY+1)%154);
          gbLcdLYIncrementTicks+=GBLY_INCREMENT_CLOCK_TICKS;
          if (register_LY<144)
          {

            u16 color = gbColorOption ? gbColorFilter[0x7FFF] :
                        0x7FFF;
            if (!gbCgbMode)
            color = gbColorOption ? gbColorFilter[gbPalette[0] & 0x7FFF] :
                        gbPalette[0] & 0x7FFF;
            for(int i = 0; i < 160; i++)
            {
              gbLineMix[i] = color;
              gbLineBuffer[i] = 0;
            }
            gbDrawLine();
          }
          else if ((register_LY==144) && (!systemFrameSkip))
          {
            int framesToSkip = systemFrameSkip;
            if(speedup)
              framesToSkip = 9; // try 6 FPS during speedup
            if((gbFrameSkipCount >= framesToSkip) || (gbWhiteScreen == 1)) {
              gbWhiteScreen = 2;

            if(!gbSgbMask)
            {
              if (gbBorderOn)
                gbSgbRenderBorder();
                  //if (gbScreenOn)
                systemDrawScreen();
            }
            }
            if(systemReadJoypads()) {
              // read joystick
              if(gbSgbMode && gbSgbMultiplayer) {
                if(gbSgbFourPlayers) {
                  gbJoymask[0] = systemReadJoypad(0);
                  gbJoymask[1] = systemReadJoypad(1);
                  gbJoymask[2] = systemReadJoypad(2);
                  gbJoymask[3] = systemReadJoypad(3);
                } else {
                  gbJoymask[0] = systemReadJoypad(0);
                  gbJoymask[1] = systemReadJoypad(1);
                }
              } else {
                gbJoymask[0] = systemReadJoypad(-1);
              }
            }
            gbFrameCount++;

            systemFrame();

            if((gbFrameCount % 10) == 0)
              system10Frames(60);

            if(gbFrameCount >= 60) {
              u32 currentTime = systemGetClock();
              if(currentTime != gbLastTime)
                systemShowSpeed(100000/(currentTime - gbLastTime));
              else
                systemShowSpeed(0);
              gbLastTime = currentTime;
              gbFrameCount = 0;
            }
          }
        }
      }
    }

    gbMemory[0xff41] = register_STAT;

    // serial emulation
    if(gbSerialOn) {
#ifdef LINK_EMULATION
      if(linkConnected) {
        gbSerialTicks -= clockTicks;

        while(gbSerialTicks <= 0) {
          // increment number of shifted bits
          gbSerialBits++;
          linkProc();
          if(gbSerialOn && (gbMemory[0xff02] & 1)) {
            if(gbSerialBits == 8) {
              gbSerialBits = 0;
              gbMemory[0xff01] = 0xff;
              gbMemory[0xff02] &= 0x7f;
              gbSerialOn = 0;
              gbMemory[0xff0f] = register_IF |= 8;
              gbSerialTicks = 0;
            }
          }
          gbSerialTicks += GBSERIAL_CLOCK_TICKS;
        }
      } else {
#endif
        if(gbMemory[0xff02] & 1) {
          gbSerialTicks -= clockTicks;

          // overflow
          while(gbSerialTicks <= 0) {
            // shift serial byte to right and put a 1 bit in its place
            //      gbMemory[0xff01] = 0x80 | (gbMemory[0xff01]>>1);
            // increment number of shifted bits
            gbSerialBits++;
            if(gbSerialBits == 8) {
              // end of transmission
              if(gbSerialFunction) // external device
                gbMemory[0xff01] = gbSerialFunction(gbMemory[0xff01]);
              else
                gbMemory[0xff01] = 0xff;
              gbSerialTicks = 0;
              gbMemory[0xff02] &= 0x7f;
              gbSerialOn = 0;
              gbMemory[0xff0f] = register_IF |= 8;
              gbSerialBits  = 0;
            } else
              gbSerialTicks += GBSERIAL_CLOCK_TICKS;
          }
        }
#ifdef LINK_EMULATION
      }
#endif
    }


    soundTicks -= clockTicks;
	if ( !gbSpeed )
		soundTicks -= clockTicks;

    while(soundTicks < 0) {
      soundTicks += SOUND_CLOCK_TICKS;

      gbSoundTick();
    }


    // timer emulation

    if(gbTimerOn) {
      gbTimerTicks= ((gbInternalTimer) & gbTimerMask[gbTimerMode])+1-clockTicks;

      while(gbTimerTicks <= 0) {
        register_TIMA++;
        // timer overflow!
        if((register_TIMA & 0xff) == 0) {
          // reload timer modulo
          register_TIMA = register_TMA;
          // flag interrupt
          gbMemory[0xff0f] = register_IF |= 4;
        }
        gbTimerTicks += gbTimerClockTicks;
      }
      gbTimerOnChange = false;
      gbTimerModeChange = false;

      gbMemory[0xff05] = register_TIMA;

    }

    gbInternalTimer -= clockTicks;
    while (gbInternalTimer<0)
      gbInternalTimer+=0x100;

    clockTicks = 0;

    if (gbIntBreak == 1)
    {
      gbIntBreak = 0;
      if ((register_IE & register_IF & gbInterruptLaunched & 0x3) &&
         ((IFF & 0x81) == 1) && (!gbInterruptWait) && (execute))
      {
        gbIntBreak = 2;
        PC.W = oldPCW;
        execute = false;
        gbOldClockTicks = 0;
      }
      if (gbOldClockTicks)
      {
        clockTicks = gbOldClockTicks;
        gbOldClockTicks = 0;
        goto gbRedoLoop;
      }
    }

    // Executes the opcode(s), and apply the instruction's remaining clockTicks (if any).
    if (execute)
    {
      switch(opcode1) {
      case 0xCB:
        // extended opcode
        switch(opcode2) {
#include "gbCodesCB.h"
        }
        break;
#include "gbCodes.h"
      }
      execute = false;

      if (clockTicks)
      {
        gbDmaTicks += clockTicks;
        clockTicks = 0;
      }
    }

    if (gbDmaTicks)
    {
      clockTicks = gbGetNextEvent(gbDmaTicks);

      if (clockTicks<=gbDmaTicks)
        gbDmaTicks -= clockTicks;
      else
      {
        clockTicks = gbDmaTicks;
        gbDmaTicks = 0;
      }

      goto gbRedoLoop;
    }


    // Remove the 'if an IE is pending' flag if IE has finished being executed.
    if ((IFF & 0x40) && !(IFF & 0x30))
      IFF &= 0x81;



    if ((register_IE & register_IF & 0x1f) && (IFF & 0x81) && (!gbInterruptWait))
    {

      if (IFF & 1)
      {
        // Add 5 ticks for the interrupt execution time
        gbDmaTicks +=  5;

        if (gbIntBreak == 2)
        {
          gbDmaTicks--;
          gbIntBreak = 0;
        }


        if(register_IF & register_IE & 1)
          gbVblank_interrupt();
        else if(register_IF & register_IE & 2)
          gbLcd_interrupt();
        else if(register_IF & register_IE & 4)
          gbTimer_interrupt();
        else if(register_IF & register_IE & 8)
          gbSerial_interrupt();
        else if(register_IF & register_IE & 16)
          gbJoypad_interrupt();
      }

      IFF &= ~0x81;
    }

    if (IFF & 0x08)
      IFF &=~0x79;

    // Used to apply the interrupt's execution time.
    if (gbDmaTicks)
    {
      clockTicks = gbGetNextEvent(gbDmaTicks);

      if (clockTicks<=gbDmaTicks)
        gbDmaTicks -= clockTicks;
      else
      {
        clockTicks = gbDmaTicks;
        gbDmaTicks = 0;
      }
      goto gbRedoLoop;
    }


    gbBlackScreen = false;

    if((ticksToStop <= 0)) {
      if(!(register_LCDC & 0x80)) {
        if(systemReadJoypads()) {
          // read joystick
          if(gbSgbMode && gbSgbMultiplayer) {
            if(gbSgbFourPlayers) {
              gbJoymask[0] = systemReadJoypad(0);
              gbJoymask[1] = systemReadJoypad(1);
              gbJoymask[2] = systemReadJoypad(2);
              gbJoymask[3] = systemReadJoypad(3);
            } else {
              gbJoymask[0] = systemReadJoypad(0);
              gbJoymask[1] = systemReadJoypad(1);
            }
          } else {
            gbJoymask[0] = systemReadJoypad(-1);
          }
        }
      }
      return;
    }
  }
}

struct EmulatedSystem GBSystem = {
  // emuMain
  gbEmulate,
  // emuReset
  gbReset,
  // emuCleanUp
  gbCleanUp,
  // emuReadBattery
  gbReadBatteryFile,
  // emuWriteBattery
  gbWriteBatteryFile,
  // emuReadState
  gbReadSaveState,
  // emuWriteState
  gbWriteSaveState,
  // emuReadMemState
  gbReadMemSaveState,
  // emuWriteMemState
  gbWriteMemSaveState,
  // emuWritePNG
  gbWritePNGFile,
  // emuWriteBMP
  gbWriteBMPFile,
  // emuUpdateCPSR
  NULL,
  // emuHasDebugger
  false,
  // emuCount
#ifdef FINAL_VERSION
  70000/4,
#else
  1000,
#endif
};
