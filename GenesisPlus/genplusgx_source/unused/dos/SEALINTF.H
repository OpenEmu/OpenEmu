
#ifndef _SEALINTF_H_
#define _SEALINTF_H_

#define MAX_SOUND               4
#define NUMVOICES               16
#define MAX_STREAM_CHANNELS     6

/* Function prototypes */
void osd_update_audio(void);
void osd_set_mastervolume(int _attenuation);
int msdos_init_sound(int *rate, int card);
void msdos_shutdown_sound(void);
void playstreamedsample(int channel,signed char *data,int len,int freq,int volume,int pan,int bits);
void osd_play_streamed_sample_16(int channel,signed short *data,int len,int freq,int volume,int pan);

#endif /* _SEALINTF_H_ */
