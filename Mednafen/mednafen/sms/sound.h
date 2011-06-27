#ifndef __SMS_SOUND_H
#define __SMS_SOUND_H

namespace MDFN_IEN_SMS
{

/* Function prototypes */
void psg_write(int data);
void psg_stereo_w(int data);
int fmunit_detect_r(void);
void fmunit_detect_w(int data);
void fmunit_write(int offset, int data);

void SMS_SoundClose(void);
int32 SMS_SoundFlush(int16 *SoundBuf, int32 MaxSoundFrames);
void SMS_SoundReset(void);
bool SMS_SetSoundRate(uint32 rate);
int SMS_SoundStateAction(StateMem *sm, int load, int data_only);
void SMS_SoundInit(uint32 clock, bool WantFM);

}
#endif
