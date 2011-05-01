#ifndef __MDFN_MDCD_INTERRUPT_H
#define __MDFN_MDCD_INTERRUPT_H

bool MDCD_InterruptGetMask(int level);
bool MDCD_InterruptGetAsserted(int level);
int MDCD_InterruptAck(int level);
void MDCD_InterruptAssert(int level, bool status);
void MDCD_InterruptReset(void);

uint8 MDCD_InterruptRead(void);
void MDCD_InterruptWrite(uint8 V);

#endif
