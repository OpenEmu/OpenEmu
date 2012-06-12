#ifndef __VB_DEBUG_H
#define __VB_DEBUG_H

namespace MDFN_IEN_VB
{

void VBDBG_FlushBreakPoints(int type);
void VBDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical);
void VBDBG_Disassemble(uint32 &a, uint32 SpecialA, char *);

uint32 VBDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical);
uint32 VBDBG_GetRegister(const std::string &name, std::string *special);
void VBDBG_SetRegister(const std::string &name, uint32 value);

void VBDBG_SetCPUCallback(void (*callb)(uint32 PC));
void VBDBG_SetBPCallback(void (*callb)(uint32 PC));

std::vector<BranchTraceResult> VBDBG_GetBranchTrace(void);

void VBDBG_CheckBP(int type, uint32 address, unsigned int len);

void VBDBG_SetLogFunc(void (*func)(const char *, const char *));

void VBDBG_DoLog(const char *type, const char *format, ...);


extern bool VB_LoggingOn;

bool VBDBG_Init(void);

};

#endif
