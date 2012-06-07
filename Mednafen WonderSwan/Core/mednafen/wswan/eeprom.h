#ifndef __WSWAN_EEPROM_H
#define __WSWAN_EEPROM_H

namespace MDFN_IEN_WSWAN
{

uint8 WSwan_EEPROMRead(uint32 A);
void WSwan_EEPROMWrite(uint32 A, uint8 V);
int WSwan_EEPROMStateAction(StateMem *sm, int load, int data_only);
void WSwan_EEPROMReset(void);
void WSwan_EEPROMInit(const char *Name, const uint16 BYear, const uint8 BMonth, const uint8 BDay, const uint8 Sex, const uint8 Blood);

}

#endif
