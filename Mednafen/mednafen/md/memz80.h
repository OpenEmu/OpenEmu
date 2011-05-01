
#ifndef _MEMZ80_H_
#define _MEMZ80_H_

namespace MDFN_IEN_MD
{

/* Function prototypes */
uint8 MD_Z80_ReadByte(uint16 address);
void MD_Z80_WriteByte(uint16 address, uint8 data);
uint8 MD_Z80_ReadPort(uint16 address);
void MD_Z80_WritePort(uint16 address, uint8 data);

void z80_unused_w(int address, int data);
int z80_unused_r(int address);
void z80_lockup_w(int address, int data);
int z80_lockup_r(int address);
int z80_vdp_r(int address);
void z80_vdp_w(int address, int data);

}

#endif /* _MEMZ80_H_ */
