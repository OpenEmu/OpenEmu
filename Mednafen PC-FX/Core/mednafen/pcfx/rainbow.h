#ifndef __PCFX_RAINBOW_H
#define __PCFX_RAINBOW_H

void RAINBOW_Write8(uint32 A, uint8 V);
void RAINBOW_Write16(uint32 A, uint16 V);

void RAINBOW_ForceTransferReset(void);
void RAINBOW_SwapBuffers(void);
void RAINBOW_DecodeBlock(bool arg_FirstDecode, bool Skip);

int RAINBOW_FetchRaster(uint32 *, uint32 layer_or, uint32 *palette_ptr);
int RAINBOW_StateAction(StateMem *sm, int load, int data_only);

bool RAINBOW_Init(bool arg_ChromaIP);
void RAINBOW_Close(void);
void RAINBOW_Reset(void);

#endif
