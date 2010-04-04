#ifndef _DEBUG_H
#define _DEBUG_H

#include "debug-driver.h"

#include "x6502struct.h"

void NESDBG_SetCPUCallback(void (*callb)(uint32 PC));
void NESDBG_SetBPCallback(void (*callb)(uint32 PC));

void NESDBG_FlushBreakPoints(int type);
void NESDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical);

uint32 NESDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical);
void NESDBG_MemPoke(uint32 A, uint32 V, unsigned int bsize, bool, bool logical);
void NESDBG_IRQ(int level);
uint32 NESDBG_GetVector(int level);
void NESDBG_Disassemble(uint32 &a, uint32 SpecialA, char *);
uint32 NESDBG_GetRegister(const std::string &name, std::string *special);
void NESDBG_SetRegister(const std::string &name, uint32 value);


void NESDBG_TestFoundBPoint(void);

void NESDBG_AddBranchTrace(uint32 PC);
std::vector<std::string> NESDBG_GetBranchTrace(void);

void NESDBG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
void NESDBG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer);

#endif
