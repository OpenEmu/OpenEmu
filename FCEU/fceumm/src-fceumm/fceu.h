#ifndef _FCEUH
#define _FCEUH

#include "types.h"

void ResetGameLoaded(void);

#define DECLFR(x) uint8 x (uint32 A)
#define DECLFW(x) void x (uint32 A, uint8 V)

void SetReadHandler(int32 start, int32 end, readfunc func);
void SetWriteHandler(int32 start, int32 end, writefunc func);
writefunc GetWriteHandler(int32 a);
readfunc GetReadHandler(int32 a);

int AllocGenieRW(void);
void FlushGenieRW(void);

void FCEU_ResetVidSys(void);

void ResetMapping(void);

extern uint64 timestampbase;
extern uint32 MMC5HackVROMMask;
extern uint8 *MMC5HackExNTARAMPtr;
extern int MMC5Hack;
extern uint8 *MMC5HackVROMPTR;
extern uint8 MMC5HackCHRMode;
extern uint8 MMC5HackSPMode;
extern uint8 MMC5HackSPScroll;
extern uint8 MMC5HackSPPage;

#ifdef COPYFAMI
extern uint8 RAM[0x4000];
#else
extern uint8 RAM[0x800];
#endif

extern uint8 GameMemBlock[131072];

extern readfunc ARead[0x10000];
extern writefunc BWrite[0x10000];

extern void (*GameInterface)(int h);
extern void (*GameStateRestore)(int version);

#define GI_RESETM2  1
#define GI_POWER  2
#define GI_CLOSE  3

#include "git.h"
extern FCEUGI *FCEUGameInfo;
extern int GameAttributes;

extern uint8 PAL;

#include "driver.h"

typedef struct {
     int PAL;
     int SoundVolume;
     int GameGenie;

     /* Current first and last rendered scanlines. */
     int FirstSLine;
     int LastSLine;

     /* Driver code(user)-specified first and last rendered scanlines.
        Usr*SLine[0] is for NTSC, Usr*SLine[1] is for PAL.
     */
     int UsrFirstSLine[2];
     int UsrLastSLine[2];
     uint32 SndRate;
} FCEUS;

extern FCEUS FSettings;

void SetNESDeemph(uint8 d, int force);

extern uint8 vsdip;

#define JOY_A   1
#define JOY_B   2
#define JOY_SELECT      4
#define JOY_START       8
#define JOY_UP  0x10
#define JOY_DOWN  0x20
#define JOY_LEFT  0x40
#define JOY_RIGHT       0x80

#endif
