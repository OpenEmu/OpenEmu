#ifndef __WSWAN_SOUND_H
#define __WSWAN_SOUND_H

namespace MDFN_IEN_WSWAN
{

int32 WSwan_SoundFlush(int16 *SoundBuf, const int32 MaxSoundFrames);

void WSwan_SoundInit(void);
void WSwan_SetSoundMultiplier(double multiplier);
bool WSwan_SetSoundRate(uint32 rate);
int WSwan_SoundStateAction(StateMem *sm, int load, int data_only);

void WSwan_SoundWrite(uint32, uint8);
uint8 WSwan_SoundRead(uint32);
void WSwan_SoundInit(void);
void WSwan_SoundReset(void);
void WSwan_SoundCheckRAMWrite(uint32 A);

}

#endif
