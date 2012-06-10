#ifndef __PCFX_DEBUG_H
#define __PCFX_DEBUG_H

#ifdef WANT_DEBUGGER

void PCFXDBG_FlushBreakPoints(int type);
void PCFXDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical);
void PCFXDBG_Disassemble(uint32 &a, uint32 SpecialA, char *);

uint32 PCFXDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical);
uint32 PCFXDBG_GetRegister(const std::string &name, std::string *special);
void PCFXDBG_SetRegister(const std::string &name, uint32 value);

void PCFXDBG_SetCPUCallback(void (*callb)(uint32 PC));
void PCFXDBG_SetBPCallback(void (*callb)(uint32 PC));

std::vector<BranchTraceResult> PCFXDBG_GetBranchTrace(void);

void PCFXDBG_CheckBP(int type, uint32 address, unsigned int len);

void PCFXDBG_SetLogFunc(void (*func)(const char *, const char *));

void PCFXDBG_DoLog(const char *type, const char *format, ...);
char *PCFXDBG_ShiftJIS_to_UTF8(const uint16 sjc);


extern bool PCFX_LoggingOn;

#endif

#endif
