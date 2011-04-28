#ifndef __PCFX_HUC6273_H
#define __PCFX_HUC6273_H

bool HuC6273_Init(void);

uint8 HuC6273_Read8(uint32 A);
uint16 HuC6273_Read16(uint32 A);
void HuC6273_Write16(uint32 A, uint16 V);
void HuC6273_Write8(uint32 A, uint8 V);
void HuC6273_Reset(void);


#endif
