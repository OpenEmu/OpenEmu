#ifndef __SOUND_H__
#define __SOUND_H__

uint32 MDFNGBSOUND_Read(int ts, uint32_t addr);
void MDFNGBSOUND_Write(int ts, uint32 addr, uint8 val);

int32 MDFNGBSOUND_Flush(int ts, int16 *SoundBuf, const int32 MaxSoundFrames);
void MDFNGBSOUND_Init(void);
void MDFNGBSOUND_Reset(void);
int MDFNGBSOUND_StateAction(StateMem *sm, int load, int data_only);

void MDFNGB_SetSoundVolume(uint32 volume);
bool MDFNGB_SetSoundRate(uint32 rate);

#endif


