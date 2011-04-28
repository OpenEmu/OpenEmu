#ifndef __MDFN_FXSCSI_H
#define __MDFN_FXSCSI_H

bool FXSCSI_Init(void);
uint8 FXSCSI_CtrlRead(uint32 A);
void FXSCSI_CtrlWrite(uint32 A, uint8 V);

#endif
