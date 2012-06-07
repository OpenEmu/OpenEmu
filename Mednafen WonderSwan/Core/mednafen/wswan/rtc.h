#ifndef __WSWAN_RTC_H
#define __WSWAN_RTC_H

namespace MDFN_IEN_WSWAN
{

void WSwan_RTCWrite(uint32 A, uint8 V);
uint8 WSwan_RTCRead(uint32 A);
void WSwan_RTCReset(void);
void WSwan_RTCClock(uint32 cycles);
int WSwan_RTCStateAction(StateMem *sm, int load, int data_only);

}

#endif
