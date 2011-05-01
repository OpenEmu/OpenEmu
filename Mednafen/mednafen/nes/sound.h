#ifndef __NES_SOUND_H
#define __NES_SOUND_H

typedef struct __EXPSOUND {
	   void (*HiFill)(void);
	   void (*HiSync)(int32 ts);

	   void (*Kill)(void);
} EXPSOUND;

#include <vector>

extern std::vector<EXPSOUND> GameExpSound;

int FlushEmulateSound(int reverse, int16 *SoundBuf, int32 MaxSoundFrames);

extern MDFN_ALIGN(16) int16 WaveHiEx[40000];

extern uint32 soundtsoffs;
#define SOUNDTS (timestamp + soundtsoffs)

int MDFNSND_Init(bool IsPAL);
void MDFNSND_Close(void);
void MDFNSND_Power(void);
void MDFNSND_Reset(void);
void MDFNSND_SaveState(void);
void MDFNSND_LoadState(int version);

void MDFN_SoundCPUHook(int);
int MDFNSND_StateAction(StateMem *sm, int load, int data_only);
void MDFNNES_SetSoundVolume(uint32 volume);
void MDFNNES_SetSoundMultiplier(double multiplier);
bool MDFNNES_SetSoundRate(double Rate);

#endif
