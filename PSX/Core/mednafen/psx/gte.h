#ifndef __MDFN_PSX_GTE_H
#define __MDFN_PSX_GTE_H

namespace MDFN_IEN_PSX
{

void GTE_Power(void);
int GTE_StateAction(StateMem *sm, int load, int data_only);

int32 GTE_Instruction(uint32 instr);

void GTE_WriteCR(unsigned int which, uint32 value);
void GTE_WriteDR(unsigned int which, uint32 value);

uint32 GTE_ReadCR(unsigned int which);
uint32 GTE_ReadDR(unsigned int which);


}

#endif
