#ifndef __WSWAN_INTERRUPT_H
#define __WSWAN_INTERRUPT_H

namespace MDFN_IEN_WSWAN
{



enum
{
 WSINT_SERIAL_SEND = 0,
 WSINT_KEY_PRESS,
 WSINT_RTC_ALARM,
 WSINT_SERIAL_RECV,
 WSINT_LINE_HIT,
 WSINT_VBLANK_TIMER,
 WSINT_VBLANK,
 WSINT_HBLANK_TIMER
};

void WSwan_Interrupt(int);
void WSwan_InterruptWrite(uint32 A, uint8 V);
uint8 WSwan_InterruptRead(uint32 A);
void WSwan_InterruptCheck(void);
int WSwan_InterruptStateAction(StateMem *sm, int load, int data_only);
void WSwan_InterruptReset(void);
void WSwan_InterruptDebugForce(unsigned int level);

#ifdef WANT_DEBUGGER
enum
{
 INT_GSREG_ISTATUS = 0,
 INT_GSREG_IENABLE,
 INT_GSREG_IVECTORBASE
};
uint32 WSwan_InterruptGetRegister(const unsigned int id, char *special, const uint32 special_len);
void WSwan_InterruptSetRegister(const unsigned int id, uint32 value);

#endif

}

#endif
