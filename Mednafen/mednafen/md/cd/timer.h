#ifndef __MDFN_MDCD_TIMER_H
#define __MDFN_MDCD_TIMER_H

void MDCD_Timer_Reset(void);
void MDCD_Timer_Run(int32 clocks);
void MDCD_Timer_Write(uint8 V);
uint8 MDCD_Timer_Read(void);

#endif
