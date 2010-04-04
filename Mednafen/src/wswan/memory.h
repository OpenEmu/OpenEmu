#ifndef __WSWAN_MEMORY_H
#define __WSWAN_MEMORY_H

extern uint8 wsRAM[65536];
extern uint8 *wsCartROM;
extern uint32 eeprom_size;
extern uint8 wsEEPROM[2048];

uint8 WSwan_readmem20(uint32);
void WSwan_writemem20(uint32 address,uint8 data);

void WSwan_MemoryInit(bool IsWSC, uint32 ssize);
void WSwan_MemoryKill(void);

void WSwan_CheckSoundDMA(void);
int WSwan_MemoryStateAction(StateMem *sm, int load, int data_only);
void WSwan_MemoryReset(void);
void WSwan_writeport(uint32 IOPort, uint8 V);
uint8 WSwan_readport(uint32 number);

#endif
