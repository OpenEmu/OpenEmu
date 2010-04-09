#ifndef __SMS_SOUND_H
#define __SMS_SOUND_H

/* Function prototypes */
void psg_write(int data);
void psg_stereo_w(int data);
int fmunit_detect_r(void);
void fmunit_detect_w(int data);
void fmunit_write(int offset, int data);

void SMS_SoundClose(void);
int16 *SMS_SoundFlush(int32 *len);
void SMS_SoundReset(void);
void SMS_SetSoundMultiplier(double multiplier);
void SMS_SetSoundVolume(uint32 volume);
void SMS_Sound(int rate);
int SMS_SoundStateAction(StateMem *sm, int load, int data_only);
void SMS_SoundInit(bool WantMono, uint32 clock, bool WantFM);

#endif
