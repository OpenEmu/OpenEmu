#ifndef __NES_SOUND_H
#define __NES_SOUND_H

#include "sound-driver.h"

typedef struct __EXPSOUND {
	   void (*HiFill)(void);
	   void (*HiSync)(int32 ts);

	   void (*RChange)(struct __EXPSOUND *);
	   void (*Kill)(void);
} EXPSOUND;

#include <vector>

extern std::vector<EXPSOUND> GameExpSound;

int GetSoundBuffer(float **W);
int FlushEmulateSound(int);

extern int16 WaveFinal[2048+512] __attribute__ ((aligned (16)));
extern int16 WaveHiEx[40000] __attribute__ ((aligned (16)));

extern uint32 soundtsoffs;
#define SOUNDTS (timestamp + soundtsoffs)

int MDFNSND_Init(int PAL);
void MDFNSND_Close(void);
void MDFNSND_Power(void);
void MDFNSND_Reset(void);
void MDFNSND_SaveState(void);
void MDFNSND_LoadState(int version);

void MDFN_SoundCPUHook(int);
int MDFNSND_StateAction(StateMem *sm, int load, int data_only);
void MDFNNES_SetSoundVolume(uint32 volume);
void MDFNNES_SetSoundMultiplier(double multiplier);
void MDFNNES_Sound(int Rate);

#endif
