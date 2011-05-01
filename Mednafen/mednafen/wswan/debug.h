#ifndef __WSWAN_DEBUG_H
#define __WSWAN_DEBUG_H

#ifdef WANT_DEBUGGER

void WSwanDBG_SetCPUCallback(void (*callb)(uint32 PC));
void WSwanDBG_SetBPCallback(void (*callb)(uint32 PC));

void WSwanDBG_FlushBreakPoints(int type);
void WSwanDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical);

uint32 WSwanDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical);
void WSwanDBG_Disassemble(uint32 &a, uint32 SpecialA, char *);
uint32 WSwanDBG_GetRegister(const std::string &name, std::string *special);
void WSwanDBG_SetRegister(const std::string &name, uint32 value);

void WSwanDBG_AddBranchTrace(uint16 CS, uint16 IP);
std::vector<std::string> WSwanDBG_GetBranchTrace(void);

void WSwanDBG_CheckBP(int type, uint32 address, unsigned int len);

void WSwanDBG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
void WSwanDBG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, const uint8 *Buffer);


void WSwanDBG_ToggleSyntax(void);
void WSwanDBG_IRQ(int level);

#endif

#endif
