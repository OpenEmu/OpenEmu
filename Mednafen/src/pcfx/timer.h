#ifndef __PCFX_TIMER_H
#define __PCFX_TIMER_H

void FXTIMER_Write16(uint32 A, uint16 V);
uint16 FXTIMER_Read16(uint32 A);
uint8 FXTIMER_Read8(uint32 A);
void FXTIMER_Update(void);
void FXTIMER_ResetTS(void);

int FXTIMER_StateAction(StateMem *sm, int load, int data_only);
bool FXTIMER_GetRegister(const std::string &name, uint32 &value, std::string *special);

#endif
