#ifndef _PCE_DEBUG_H
#define _PCE_DEBUG_H

#ifdef WANT_DEBUGGER

#include "pce_psg/pce_psg.h"

namespace MDFN_IEN_PCE
{

void PCEDBG_SetCPUCallback(void (*callb)(uint32 PC));
void PCEDBG_SetBPCallback(void (*callb)(uint32 PC));

void PCEDBG_FlushBreakPoints(int type);
void PCEDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical);

uint32 PCEDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical);
void PCEDBG_IRQ(int level);
uint32 PCEDBG_GetVector(int level);
void PCEDBG_Disassemble(uint32 &a, uint32 SpecialA, char *);
uint32 PCEDBG_GetRegister(const std::string &name, std::string *special);
void PCEDBG_SetRegister(const std::string &name, uint32 value);

void PCEDBG_CheckBP(int type, uint32 address, unsigned int len);

void PCEDBG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
void PCEDBG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer);


void PCEDBG_SetLogFunc(void (*func)(const char *, const char *));

void PCEDBG_DoLog(const char *type, const char *format, ...);
char *PCEDBG_ShiftJIS_to_UTF8(const uint16 sjc);

void PCEDBG_EnableUsageMap(bool);

extern bool PCE_LoggingOn;
extern bool PCE_UsageMapOn;

extern DebuggerInfoStruct PCEDBGInfo;

bool PCEDBG_Init(bool sgx, PCE_PSG *psg);

};

#endif

#endif
