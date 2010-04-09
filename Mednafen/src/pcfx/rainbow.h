#ifndef __PCFX_RAINBOW_H
#define __PCFX_RAINBOW_H

void RAINBOW_Write8(uint32 A, uint8 V);
void RAINBOW_Write16(uint32 A, uint16 V);

void RAINBOW_TransferStart(void);
int RAINBOW_FetchRaster(uint8 *, bool WantDecode, bool Skip);
int RAINBOW_StateAction(StateMem *sm, int load, int data_only);

bool RAINBOW_Init(void);
void RAINBOW_Close(void);
void RAINBOW_Reset(void);

#endif
