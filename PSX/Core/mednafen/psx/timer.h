#ifndef __MDFN_PSX_TIMER_H
#define __MDFN_PSX_TIMER_H

namespace MDFN_IEN_PSX
{

enum
{
 TIMER_GSREG_COUNTER0 = 0x00,
 TIMER_GSREG_MODE0,
 TIMER_GSREG_TARGET0,

 TIMER_GSREG_COUNTER1 = 0x10,
 TIMER_GSREG_MODE1,
 TIMER_GSREG_TARGET1,

 TIMER_GSREG_COUNTER2 = 0x20,
 TIMER_GSREG_MODE2,
 TIMER_GSREG_TARGET2,
};

uint32 TIMER_GetRegister(unsigned int which, char *special, const uint32 special_len);
void TIMER_SetRegister(unsigned int which, uint32 value);


void TIMER_Write(const pscpu_timestamp_t timestamp, uint32 A, uint16 V);
uint16 TIMER_Read(const pscpu_timestamp_t timestamp, uint32 A);

void TIMER_AddDotClocks(uint32 count);
void TIMER_ClockHRetrace(void);
void TIMER_SetHRetrace(bool status);
void TIMER_SetVBlank(bool status);

pscpu_timestamp_t TIMER_Update(const pscpu_timestamp_t);
void TIMER_ResetTS(void);

void TIMER_Power(void);
int TIMER_StateAction(StateMem *sm, int load, int data_only);

}

#endif
