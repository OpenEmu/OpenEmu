#ifndef __MDFN_PCE_HES_H
#define __MDFN_PCE_HES_H

namespace MDFN_IEN_PCE
{

uint8 ReadIBP(unsigned int A);
int PCE_HESLoad(const uint8 *buf, uint32 size);
void HES_Reset(void);
void HES_Update(EmulateSpecStruct *espec, uint16 jp_data);
void HES_Close(void);

};

#endif
