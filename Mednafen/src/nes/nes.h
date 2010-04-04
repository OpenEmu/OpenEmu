#ifndef _NES_H
#define _NES_H

#include "../types.h"
#include "../mednafen.h"
#include "../endian.h"
#include "../state.h"
#include "../movie.h"
#include "../general.h"
#include "../memory.h"
#include "../file.h"
#include "../md5.h"
#include "../video.h"

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

void MDFN_MemoryRand(uint8 *ptr, uint32 size);
void SetReadHandler(int32 start, int32 end, readfunc func, bool snc = 1);
void SetWriteHandler(int32 start, int32 end, writefunc func);
writefunc GetWriteHandler(int32 a);
readfunc GetReadHandler(int32 a);

int AllocGenieRW(void);
void FlushGenieRW(void);


typedef struct
{
 void (*Power)(void);
 void (*Reset)(void);
 void (*Close)(void);
 int (*StateAction)(StateMem *sm, int load, int data_only);
} NESGameType;


extern bool NESIsVSUni;

#endif
