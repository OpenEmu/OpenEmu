#ifndef __WSWAN_SOUND_H
#define __WSWAN_SOUND_H

int16 *WSwan_SoundFlush(int32 *length);
void WSwan_SoundInit(bool WantMono);
void WSwan_SetSoundMultiplier(double multiplier);
void WSwan_SetSoundVolume(uint32 volume);
void WSwan_Sound(int rate);
int WSwan_SoundStateAction(StateMem *sm, int load, int data_only);

void WSwan_SoundWrite(uint32, uint8);
uint8 WSwan_SoundRead(uint32);
void WSwan_SoundInit(void);
void WSwan_SoundReset(void);
void WSwan_SoundCheckRAMWrite(uint32 A);

#endif
