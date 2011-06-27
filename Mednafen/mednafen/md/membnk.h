
#ifndef _MEMBNK_H_
#define _MEMBNK_H_

namespace MDFN_IEN_MD
{

/* Function prototypes */
void z80_write_banked_memory(unsigned int address, unsigned int data);
int z80_read_banked_memory(unsigned int address);

void z80bank_vdp_w(int address, int data);
int z80bank_vdp_r(int address);

void z80bank_unused_w(int address, int data);
int z80bank_unused_r(int address);
void z80bank_lockup_w(int address, int data);
int z80bank_lockup_r(int address);

}

#endif /* _MEMBNK_H_ */
