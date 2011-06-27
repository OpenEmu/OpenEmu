#ifndef _MEM68K_H_
#define _MEM68K_H_

namespace MDFN_IEN_MD
{

/* Function prototypes */
unsigned int m68k_read_bus_8(unsigned int address);
unsigned int m68k_read_bus_16(unsigned int address);
void m68k_unused_w(unsigned int address, unsigned int value);

void m68k_lockup_w_8(unsigned int address, unsigned int value);
void m68k_lockup_w_16(unsigned int address, unsigned int value);
unsigned int m68k_lockup_r_8(unsigned int address);
unsigned int m68k_lockup_r_16(unsigned int address);


uint8 MD_ReadMemory8(uint32 address);
uint16 MD_ReadMemory16(uint32 address);
void MD_WriteMemory8(uint32 address, uint8 value);
void MD_WriteMemory16(uint32 address, uint16 value);

}

#endif /* _MEM68K_H_ */
