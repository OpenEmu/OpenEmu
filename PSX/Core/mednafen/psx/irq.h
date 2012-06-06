#ifndef __MDFN_PSX_IRQ_H
#define __MDFN_PSX_IRQ_H

namespace MDFN_IEN_PSX
{


enum
{
 IRQ_VSYNC = 	0,
 IRQ_GPU =	        1,
 IRQ_CD =		2,
 IRQ_DMA =		3,	// Probably
 IRQ_TIMER_0 	= 4,
 IRQ_TIMER_1 	= 5,
 IRQ_TIMER_2 	= 6,
 IRQ_SIO	      = 7,
 IRQ_SPU	      = 9,
 IRQ_PIO		= 10,	// Probably
};

void IRQ_Power(void);
void IRQ_Assert(int which, bool asserted);

void IRQ_Write(uint32 A, uint32 V);
uint32 IRQ_Read(uint32 A);


enum
{
 IRQ_GSREG_ASSERTED = 0,
 IRQ_GSREG_STATUS = 1,
 IRQ_GSREG_MASK = 2
};

uint32 IRQ_GetRegister(unsigned int which, char *special, const uint32 special_len);
void IRQ_SetRegister(unsigned int which, uint32 value);

int IRQ_StateAction(StateMem *sm, int load, int data_only);
};


#endif
