#ifndef __SOUND_H__
#define __SOUND_H__

uint32 MDFNGBSOUND_Read(int ts, uint32_t addr);
void MDFNGBSOUND_Write(int ts, uint32 addr, uint8 val);
int16 *MDFNGBSOUND_Flush(int ts, int32 *length);
void MDFNGBSOUND_Init(bool WantMono);
void MDFNGBSOUND_Reset(void);
int MDFNGBSOUND_StateAction(StateMem *sm, int load, int data_only);

void MDFNGB_SetSoundMultiplier(double multiplier);
void MDFNGB_SetSoundVolume(uint32 volume);
void MDFNGB_Sound(int rate);

#endif


