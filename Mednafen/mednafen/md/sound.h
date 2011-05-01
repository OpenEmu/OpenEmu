
#ifndef _SOUND_H_
#define _SOUND_H_

namespace MDFN_IEN_MD
{

/* Function prototypes */
void fm_write(int address, int data);
void psg_write(int data);

void MDSound_SetYM2612Reset(bool new_reset);
int MDSound_ReadFM(int address);
void MDSound_SetSoundMultiplier(double multiplier);
bool MDSound_SetSoundRate(uint32 rate);
int32 MDSound_Flush(int16 *SoundBuf, const int32 MaxSoundFrames);
int MDSound_Init(void);
void MDSound_Kill(void);
void MDSound_Power(void);

int MDSound_StateAction(StateMem *sm, int load, int data_only);

};

#endif /* _SOUND_H_ */
