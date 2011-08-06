#ifndef __MDFN_DRIVERS_SOUND_H
#define __MDFN_DRIVERS_SOUND_H

bool InitSound(MDFNGI *gi);
void WriteSound(int16 *Buffer, int Count);
void WriteSoundSilence(int ms);
bool KillSound(void);
uint32 GetMaxSound(void);
uint32 GetWriteSound(void);
void SilenceSound(int s);

int16 *GetEmuModSoundBuffer(int32 *max_size);

double GetSoundRate(void);

#endif
