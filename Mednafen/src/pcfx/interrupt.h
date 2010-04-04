#ifndef __PCFX_INTERRUPT_H
#define __PCFX_INTERRUPT_H

void PCFXIRQ_Assert(int level, bool assert);
void PCFXIRQ_Write16(uint32 A, uint16 V);
uint16 PCFXIRQ_Read16(uint32 A);
uint8 PCFXIRQ_Read8(uint32 A);
int PCFXIRQ_StateAction(StateMem *sm, int load, int data_only);


bool PCFXIRQ_GetRegister(const std::string &name, uint32 &value, std::string *special);
bool PCFXIRQ_SetRegister(const std::string &name, uint32 value);

#endif
