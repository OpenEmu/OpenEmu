#ifndef __MDFN_DRIVERS_SOUND_H
#define __MDFN_DRIVERS_SOUND_H

int InitSound(MDFNGI *gi);
void WriteSound(int16 *Buffer, int Count);
void WriteSoundSilence(int ms);
int KillSound(void);
uint32 GetMaxSound(void);
uint32 GetWriteSound(void);
void SilenceSound(int s);

#endif
