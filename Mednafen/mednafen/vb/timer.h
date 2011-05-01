#ifndef __MDFN_VB_TIMER_H
#define __MDFN_VB_TIMER_H

namespace MDFN_IEN_VB
{

v810_timestamp_t TIMER_Update(v810_timestamp_t timestamp);
void TIMER_ResetTS(void);
uint8 TIMER_Read(const v810_timestamp_t &timestamp, uint32 A);
void TIMER_Write(const v810_timestamp_t &timestamp, uint32 A, uint8 V);

void TIMER_Power(void);

int TIMER_StateAction(StateMem *sm, int load, int data_only);


enum
{
 TIMER_GSREG_TCR,
 TIMER_GSREG_DIVCOUNTER,
 TIMER_GSREG_RELOAD_VALUE,
 TIMER_GSREG_COUNTER,
};

uint32 TIMER_GetRegister(const unsigned int id, char *special, const uint32 special_len);
void TIMER_SetRegister(const unsigned int id, const uint32 value);

}

#endif
