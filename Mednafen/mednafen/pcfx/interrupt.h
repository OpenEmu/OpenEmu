#ifndef __PCFX_INTERRUPT_H
#define __PCFX_INTERRUPT_H

#define PCFXIRQ_SOURCE_TIMER	1
#define PCFXIRQ_SOURCE_EX	2
#define PCFXIRQ_SOURCE_INPUT	3
#define PCFXIRQ_SOURCE_VDCA	4
#define PCFXIRQ_SOURCE_KING	5
#define PCFXIRQ_SOURCE_VDCB	6
#define PCFXIRQ_SOURCE_HUC6273  7

void PCFXIRQ_Assert(int source, bool assert);
void PCFXIRQ_Write16(uint32 A, uint16 V);
uint16 PCFXIRQ_Read16(uint32 A);
uint8 PCFXIRQ_Read8(uint32 A);
int PCFXIRQ_StateAction(StateMem *sm, int load, int data_only);

void PCFXIRQ_Reset(void);

bool PCFXIRQ_GetRegister(const std::string &name, uint32 &value, std::string *special);
bool PCFXIRQ_SetRegister(const std::string &name, uint32 value);

#endif
