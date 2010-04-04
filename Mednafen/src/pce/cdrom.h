#ifndef __PCE_CDROM_H
#define __PCE_CDROM_H

void PCECD_Run(void);
void PCECD_ResetTS(void);

int32 PCECD_Init();

void PCECD_Close();

void PCECD_Power(void);

uint8 PCECD_Read(uint32);

void PCECD_Write(uint32, uint8 data);

bool PCECD_IsCDROMEnabled();

bool PCECD_IsBRAMEnabled();

int PCECD_StateAction(StateMem *sm, int load, int data_only);

bool PCECD_DoADFun(uint8 *data);

#endif

