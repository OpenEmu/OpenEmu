#ifndef _NES_H
#define _NES_H

#include "../mednafen.h"
#include "../state.h"
#include "../movie.h"
#include "../general.h"
#include "../file.h"
#include "../md5.h"
#include "../video.h"

typedef void (*writefunc)(uint32 A, uint8 V);
typedef uint8 (*readfunc)(uint32 A);

void ResetMapping(void);
void ResetNES(void);
void PowerNES(void);

extern uint64 timestampbase;
extern uint32 MMC5HackVROMMask;
extern uint8 *MMC5HackExNTARAMPtr;
extern uint32 MMC5HackCHRBank;
extern int MMC5Hack;
extern uint8 *MMC5HackVROMPTR;
extern uint8 MMC5HackCHRMode;
extern uint8 MMC5HackSPMode;
extern uint8 MMC5HackSPScroll;
extern uint8 MMC5HackSPPage;

extern readfunc ARead[0x10000 + 0x100];
extern writefunc BWrite[0x10000 + 0x100];

extern int GameAttributes;
extern uint8 PAL;

extern int fceuindbg;
void ResetGameLoaded(void);

#define DECLFR(x) uint8 x (uint32 A)
#define DECLFW(x) void x (uint32 A, uint8 V)

DECLFR(ANull);
DECLFW(BNull);

void SetReadHandler(int32 start, int32 end, readfunc func, bool snc = 1);
void SetWriteHandler(int32 start, int32 end, writefunc func);
writefunc GetWriteHandler(int32 a);
readfunc GetReadHandler(int32 a);

typedef struct
{
 void (*Power)(void);
 void (*Reset)(void);
 void (*Close)(void);
 int (*StateAction)(StateMem *sm, int load, int data_only);
} NESGameType;


extern bool NESIsVSUni;

#endif
